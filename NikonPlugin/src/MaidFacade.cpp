#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

#include <QDebug>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QDir>
#include <QFileDialog>

#include "MaidFacade.h"
#include "MaidUtil.h"
#include "MaidError.h"
#include "DkSettings.h"

using nmc::MaidFacade;
using Maid::MaidUtil;
using Maid::MaidObject;

MaidFacade::MaidFacade() 
	: lensAttached(false), prevFileNumber(0), captureCount(0), allItemsAcquired(false), 
	currentlyAcquiringObjects(false), initialized(false) {
}

/*!
 * throws InitError, MaidError
 */
bool MaidFacade::init() {
	
	try {
		MaidUtil::getInstance().loadLibrary();
		qDebug() << "Loaded MAID library";
		MaidUtil::getInstance().initMAID();
		qDebug() << "Initialized MAID";

		// create module object
		moduleObject.reset(MaidObject::create(0, nullptr));
		moduleObject->enumCaps();
		qDebug() << "MAID Module Object created";
	} catch (...) {
		qDebug() << "Could not initialize MAID (whatever that is)";
		return false;
	}
	
	// set callbacks
	moduleObject->setEventCallback(this, eventProc);

	// connect future watchers
	connect(&shootFutureWatcher, SIGNAL(finished()), this, SLOT(shootFinished()));
	connect(&acquireFutureWatcher, SIGNAL(finished()), this, SLOT(acquireItemObjectsFinished()));

	initialized = true;
	return true;
}

bool MaidFacade::isInitialized() {
	return initialized;
}

void MaidFacade::setCapValueChangeCallback(std::function<void(uint32_t)> capValueChangeCallback) {
	this->capValueChangeCallback = capValueChangeCallback;
}

/*!
 * throws MaidError
 */
std::set<uint32_t> MaidFacade::listDevices() {
	auto& devicesV = moduleObject->getChildren();

	return std::set<uint32_t>(devicesV.begin(), devicesV.end());
}

/*!
 * throws OpenCloseObjectError
 */
void MaidFacade::openSource(ULONG id) {
	sourceObject.reset(MaidObject::create(id, moduleObject.get()));
	sourceObject->setEventCallback(this, eventProc);
	//sourceObject->setProgressCallback(progressProc);
}

/*!
 * throws MaidError
 */
bool MaidFacade::checkCameraType() {
	// the only currently supported source is a Nikon D4
	// read the camera type
	ULONG cameraType = 0;
	sourceObject->capGet(kNkMAIDCapability_CameraType, kNkMAIDDataType_UnsignedPtr, (NKPARAM) &cameraType);
	return cameraType == kNkMAIDCameraType_D4;
}

/*!
 * Reads a packed string value from the source and returns it
 * throws MaidError
 */
MaidFacade::MaybeStringValues MaidFacade::readPackedStringCap(ULONG capId) {
	MaybeStringValues mv;
	StringValues& v = mv.first;
	mv.second = false;
	
	if (!sourceObject) {
		return mv;
	}
	
	std::pair<NkMAIDEnum, bool> mEnum = MaidUtil::getInstance().fillEnum<char>(sourceObject.get(), capId);
	if (!mEnum.second) {
		return mv;
	}

	NkMAIDEnum* en = &mEnum.first;
	v.values = MaidUtil::getInstance().packedStringEnumToVector(en);
	v.currentValue = en->ulValue;
	delete[] en->pData;

	mv.second = true;
	return mv;
}

MaidFacade::MaybeUnsignedValues MaidFacade::readUnsignedEnumCap(ULONG capId) {
	MaybeUnsignedValues mv;
	UnsignedValues& v = mv.first;
	mv.second = false;

	if (!sourceObject) {
		return mv;
	}

	std::pair<NkMAIDEnum, bool> mEnum = MaidUtil::getInstance().fillEnum<ULONG>(sourceObject.get(), capId);
	if (!mEnum.second) {
		return mv;
	}
	
	NkMAIDEnum* en = &mEnum.first;
	v.values = MaidUtil::getInstance().unsignedEnumToVector(en);
	v.currentValue = en->ulValue;
	delete[] en->pData;

	mv.second = true;
	return mv;
}

/*!
 * Reads the aperture value from the source and returns it
 * throws MaidError
 */
MaidFacade::MaybeStringValues MaidFacade::readAperture() {
	MaybeStringValues v = readPackedStringCap(kNkMAIDCapability_Aperture);
	// maid module returns "--" if there is no lens attached and F0Manual is not set
	if (!v.second || v.first.values.size() == 0 || v.first.values.at(0) == "--") {
		v.second = false;
		return v;
	}

	aperture = v;

	return aperture;
}

/*!
 * Reads the sensitivity value from the source and returns it
 * throws MaidError
 */
MaidFacade::MaybeStringValues MaidFacade::readSensitivity() {
	sensitivity = readPackedStringCap(kNkMAIDCapability_Sensitivity);
	return sensitivity;
}

/*!
 * Reads the shutter speed value from the source and returns it
 * throws MaidError
 */
MaidFacade::MaybeStringValues MaidFacade::readShutterSpeed() {
	shutterSpeed = readPackedStringCap(kNkMAIDCapability_ShutterSpeed);
	return shutterSpeed;
}

/*!
 * Reads the exposure mode from the source and returns it
 * throws MaidError
 */
MaidFacade::MaybeUnsignedValues MaidFacade::readExposureMode() {
	exposureMode = readUnsignedEnumCap(kNkMAIDCapability_ExposureMode);
	if (!exposureMode.second) {
		return exposureMode;
	}

	// lens attached: 4 values
	// not attached: 2 values
	lensAttached = exposureMode.first.values.size() == 4;

	return exposureMode;
}

MaidFacade::MaybeStringValues MaidFacade::getAperture() {
	return aperture;
}

MaidFacade::MaybeStringValues MaidFacade::getSensitivity() {
	return sensitivity;
}

MaidFacade::MaybeStringValues MaidFacade::getShutterSpeed() {
	return shutterSpeed;
}

MaidFacade::MaybeUnsignedValues MaidFacade::getExposureMode() {
	return exposureMode;
}

bool MaidFacade::writeEnumCap(ULONG capId, size_t newValue) {
	try {
		if (!sourceObject->hasCapOperation(capId, kNkMAIDCapOperation_Set)) {
			return false;
		}

		std::pair<NkMAIDEnum, bool> mEnum = MaidUtil::getInstance().fillEnum<char>(sourceObject.get(), capId);
		if (!mEnum.second) {
			return false;
		}

		mEnum.first.ulValue = newValue;
		sourceObject->capSet(capId, kNkMAIDDataType_EnumPtr, (NKPARAM) &mEnum.first);
		return true;
	} catch (Maid::MaidError) {
		return false; // we don't care about what specifically went wrong
	}
}

bool MaidFacade::setMaybeStringEnumValue(std::pair<StringValues, bool>& theMaybeValue, ULONG capId, size_t newValue) {
	if (theMaybeValue.second) {
		bool r = writeEnumCap(capId, newValue);
		if (r) {
			theMaybeValue.first.currentValue = newValue;
		}
		return r;
	} else {
		return false;
	}
}

bool MaidFacade::setMaybeUnsignedEnumValue(std::pair<UnsignedValues, bool>& theMaybeValue, ULONG capId, size_t newValue) {
	if (theMaybeValue.second) {
		bool r = writeEnumCap(capId, newValue);
		if (r) {
			theMaybeValue.first.currentValue = newValue;
		}
		return r;
	} else {
		return false;
	}
}

bool MaidFacade::setAperture(size_t newValue) {
	return setMaybeStringEnumValue(aperture, kNkMAIDCapability_Aperture, newValue);
}

bool MaidFacade::setSensitivity(size_t newValue) {
	return setMaybeStringEnumValue(sensitivity, kNkMAIDCapability_Sensitivity, newValue);
}

bool MaidFacade::setShutterSpeed(size_t newValue) {
	return setMaybeStringEnumValue(shutterSpeed, kNkMAIDCapability_ShutterSpeed, newValue);
}

bool MaidFacade::setExposureMode(size_t newValue) {
	return setMaybeUnsignedEnumValue(exposureMode, kNkMAIDCapability_ExposureMode, newValue);
}

bool MaidFacade::isLensAttached() {
	return lensAttached;
}

/*!
 * throws MaidError
 */
bool MaidFacade::isAutoIso() {
	bool autoIso;
	sourceObject->capGet(kNkMAIDCapability_IsoControl, kNkMAIDDataType_BooleanPtr, (NKPARAM) &autoIso);
	return autoIso;
}

void MaidFacade::closeModule() {
	if (moduleObject) {
		moduleObject->closeObject();
	}
	moduleObject.reset();
}
	
void MaidFacade::closeSource() {
	if (sourceObject) {
		try {
			if (isLiveViewActive()) {
				toggleLiveView();
			}

			sourceObject->closeObject();
		} catch (Maid::MaidError) {
			// live view not active or object was already closed
		}
	}
	sourceObject.reset();
}

void MaidFacade::closeEverything() {
	closeSource();
	closeModule();
}

/*!
 * throws MaidError
 */
bool MaidFacade::isSourceAlive() {
	if (sourceObject) {
		return sourceObject->isAlive();
	}
	return false;
}

void MaidFacade::sourceIdleLoop(ULONG* count) {
	// wait until the operation is completed (when completionProc is called)
	do {
		sourceObject->async();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	} while (*count <= 0);
	sourceObject->async();
}

/**
 * throws MaidError
 */
bool MaidFacade::shoot(bool withAf) {
	captureCount = 0;
	NkMAIDCapInfo capInfo;
	sourceObject->getCapInfo(kNkMAIDCapability_Capture, &capInfo);

	CompletionProcData* complData = new CompletionProcData();
	complData->count = &captureCount;

	unsigned long cap = kNkMAIDCapability_Capture;
	if (withAf) {
		cap = kNkMAIDCapability_AFCapture;
	}

	if (!shootFutureWatcher.isRunning()) {
		// start shooting (threaded)
		QFuture<int> shootFuture = QtConcurrent::run(sourceObject.get(), &MaidObject::capStart, cap, (LPNKFUNC) completionProc, (NKREF) complData);
		shootFutureWatcher.setFuture(shootFuture);

		return true;
	}

	return false;
}

/**
 * AutoFocus is treated like shoot because it does the same kind of operation
 * throws MaidError
 */
bool MaidFacade::autoFocus() {
	captureCount = 0;
	unsigned long cap = kNkMAIDCapability_AutoFocus;

	CompletionProcData* complData = new CompletionProcData();
	complData->count = &captureCount;

	if (!shootFutureWatcher.isRunning()) {
		if (!isLiveViewActive()) {
			// start shooting (threaded)
			QFuture<int> shootFuture = QtConcurrent::run(sourceObject.get(), &MaidObject::capStart, cap, (LPNKFUNC) completionProc, (NKREF) complData);
			shootFutureWatcher.setFuture(shootFuture);

			return true;
		} else { // do contrast af in live view
			sourceObject->capSet(kNkMAIDCapability_ContrastAF, kNkMAIDDataType_Unsigned, (NKPARAM) kNkMAIDContrastAF_Start);
		}
	}

	return false;
}

void MaidFacade::shootFinished() {
	int opRet = shootFutureWatcher.result();
	if (opRet != kNkMAIDResult_NoError && opRet != kNkMAIDResult_Pending) {
		qDebug() << "Error executing capture or autofocus capability";
		return;// return false;
	}

	// start acquiring the pictures (threaded)
	//sourceIdleLoop(&captureCount);
}

bool MaidFacade::acquireItemObjects() {
	CompletionProcData* complData;
	NkMAIDCapInfo capInfo;

	allItemsAcquired = false;

	// acquire the _next_ item object

	try {
		std::vector<ULONG> itemIds = sourceObject->getChildren();
		if (itemIds.size() <= 0) {
			qDebug() << "No item objects left";

			emit shootAndAcquireFinished();
			allItemsAcquired = true;

			return true;
		}

		// open the item object
		std::unique_ptr<MaidObject> itemObject(MaidObject::create(itemIds.at(0), sourceObject.get())); // we always choose the one at pos 0
		if (!itemObject) {
			qDebug() << "Item object #0 could not be opened!";
			return false;
		}

		itemObject->getCapInfo(kNkMAIDCapability_DataTypes, &capInfo);
		ULONG dataTypes;
		itemObject->capGet(kNkMAIDCapability_DataTypes, kNkMAIDDataType_UnsignedPtr, (NKPARAM) &dataTypes);

		std::unique_ptr<MaidObject> dataObject;

		if (dataTypes & kNkMAIDDataObjType_Image) {
			dataObject.reset(MaidObject::create(kNkMAIDDataObjType_Image, itemObject.get()));
		} else if (dataTypes & kNkMAIDDataObjType_File) {
			dataObject.reset(MaidObject::create(kNkMAIDDataObjType_File, itemObject.get()));
		} else {
			return false;
		}

		if (!dataObject) {
			qDebug() << "Data object could not be opened!";
			return false;
		}

		DataProcData* dataRef = new DataProcData(this);
		dataRef->id = dataObject->getID();

		ProgressProcData* progressRef = new ProgressProcData(this);

		ULONG acquireCount = 0;
		complData = new CompletionProcData();
		complData->count = &acquireCount;
		complData->data = dataRef;

		dataObject->setDataCallback((NKREF) dataRef, dataProc);
		dataObject->setProgressCallback((NKREF) progressRef, progressProc);
		int opRet = dataObject->capStart(kNkMAIDCapability_Acquire, completionProc, (NKREF) complData);
		if (opRet != kNkMAIDResult_NoError && opRet != kNkMAIDResult_Pending) {
			qDebug() << "Error acquiring data";
			return false;
		}

		sourceIdleLoop(&acquireCount);

		dataObject->setDataCallback((NKREF) nullptr, (LPMAIDDataProc) nullptr);
	} catch (Maid::MaidError) {
		qDebug() << "something went wrong in acquireItemObjects";
	}

	return true;
}

void MaidFacade::startAcquireItemObjects() {
	if (!currentlyAcquiringObjects) {
		currentlyAcquiringObjects = true;
		emit acquireStart();
		QFuture<bool> acquireFuture = QtConcurrent::run(this, &MaidFacade::acquireItemObjects);
		acquireFutureWatcher.setFuture(acquireFuture);
	}
}

void MaidFacade::setAutoSaveNaming(bool a) {
	autoSaveNaming = a;
}

void MaidFacade::acquireItemObjectsFinished() {
	if (allItemsAcquired) {
		allItemsAcquired = false;
		currentlyAcquiringObjects = false;
		return;
	}

	[&] () {

		QFileInfo fileInfo(QDir(DkSettings::global.lastDir), makePictureFilename());
		QString filename = getCapturedFileName(fileInfo);

		QFileInfo firstFilenameInfo = QFileInfo(firstFilename);

		// if it is the first picture or the file type has changed or not auto save naming
		if (firstFilename.isEmpty() || !autoSaveNaming) {
		
			if (filename.isEmpty()) {
				//return kNkMAIDResult_NoError;
				qDebug() << "filename was empty, file will be discarded";
				return;
			}

			firstFilename = filename;

		} else {
			// save 
			QFileInfo newFilenameInfo = QFileInfo(firstFilenameInfo.canonicalPath() + "/" + firstFilenameInfo.baseName() + "." + fileInfo.suffix());
			filename = increaseFilenameNumber(newFilenameInfo);
			qDebug() << "I tried to increase the file number...";
		}
		qDebug() << "saving file: " << filename;

		// using Qt for I/O allows for Unicode filenames!
		QFile file(filename);
		file.open(QFile::WriteOnly);
		
		if (!file.isOpen()) {
			QMessageBox::critical(0, tr("Save Error"), tr("Sorry, I could not write to:\n %1").arg(filename));
			return;
		}
		
		file.write(currentFileData->buffer, currentFileFileInfo.ulTotalLength);
		file.close();


		//std::ofstream outFile;

		//outFile.open(filename.toStdString(), std::ios::out | std::ios::binary);
		//if (!outFile.good() || !outFile.is_open()) {
		//	//return kNkMAIDResult_UnexpectedError;
		//	qDebug() << "could not open file for writing!";
		//	return;
		//}

		//qDebug() << "writing " << currentFileFileInfo.ulTotalLength << " bytes";
		//outFile.write(currentFileData->buffer, currentFileFileInfo.ulTotalLength);
		//outFile.close();

		lastFileInfo = QFileInfo(filename);

	}();

	delete[] currentFileData->buffer;
	delete currentFileData;
	currentFileData = nullptr;

	// acquire next item object
	QFuture<bool> acquireFuture = QtConcurrent::run(this, &MaidFacade::acquireItemObjects);
	acquireFutureWatcher.setFuture(acquireFuture);
}

QString MaidFacade::getCapturedFileName(const QFileInfo& saveFile) {

	//if (!viewport)
	//	return saveFile.fileName();

	qDebug() << "saving captured image...";

	//DkImageLoader* loader = viewport->getImageLoader();

	QString selectedFilter;
	QString saveName;
	saveName = saveFile.fileName();

	int filterIdx = -1;

	// do not use the nomacs saveFilters here, the file format is already set
	QStringList sF;
	sF.append("Nikon (*.nef)");
	sF.append("JPEG (*.jpg *.jpeg)");
	//qDebug() << sF;

	QRegExp exp = QRegExp("*." + saveFile.suffix() + "*", Qt::CaseInsensitive);
	exp.setPatternSyntax(QRegExp::Wildcard);
	//qDebug() << "exp: " << exp;

	for (int idx = 0; idx < sF.size(); idx++) {
		if (exp.exactMatch(sF.at(idx))) {
			selectedFilter = sF.at(idx);
			filterIdx = idx;
			break;
		}
	}

	if (filterIdx == -1)
		saveName.remove("." + saveFile.suffix());

	QString fileName;

	//int answer = QDialog::Rejected;

	//if (!selectedFilter.isEmpty() && viewport()->getImageLoader()->isEdited()) {
	//	fileName = loader->getCurrentImage()->file().absoluteFilePath();
	//	DkMessageBox* msg = new DkMessageBox(QMessageBox::Question, tr("Overwrite File"), 
	//		tr("Do you want to overwrite:\n%1?").arg(fileName), 
	//		(QMessageBox::Yes | QMessageBox::No), this);
	//	msg->setObjectName("overwriteDialog");

	//	//msg->show();
	//	answer = msg->exec();

	//}

	//if (answer == QDialog::Rejected || answer == QMessageBox::No) {
		// note: basename removes the whole file name from the first dot...
	if (firstFilename.isEmpty() || !autoSaveNaming) {	
		QString savePath = (!selectedFilter.isEmpty()) ? saveFile.absoluteFilePath() : QFileInfo(saveFile.absoluteDir(), saveName).absoluteFilePath();

		// TODO: set the main window here...
		fileName = QFileDialog::getSaveFileName(0, tr("Save File %1").arg(saveName),
			savePath, selectedFilter, &selectedFilter);
	}

	return fileName;
}


/**
 * For image0.jpg, this will return image1.jpg, etc.
 */
QString MaidFacade::increaseFilenameNumber(const QFileInfo& fileInfo) {
	
	qDebug() << "file info before increasing: " << fileInfo.absoluteFilePath();
	
	std::ifstream testFileIn;
	QString basePath = fileInfo.canonicalPath() + "/" + fileInfo.baseName();
	QString filename = "";
	// test file names
	while (true) {
		filename = basePath + "_" + QString::number(++prevFileNumber) + "." + fileInfo.completeSuffix();
		testFileIn.open(filename.toStdString());
		if (!testFileIn.good()) {
			testFileIn.close();
			break;
		}

		testFileIn.close();
	}

	qDebug() << "increasing filename to: " << filename;

	return filename;
}

QString MaidFacade::getCurrentSavePath() {
	if (firstFilename.isEmpty()) {
		return QString();
	} else {
		return QFileInfo(firstFilename).canonicalPath();
	}
}

bool MaidFacade::toggleLiveView() {
	int32_t lvStatus = 0;

	if (isLiveViewActive()) {
		lvStatus = 0;
	} else {
		lvStatus = 1;
	}

	try {
		sourceObject->capSet(kNkMAIDCapability_LiveViewStatus, kNkMAIDDataType_Unsigned, (NKPARAM) lvStatus);

		//if (isLiveViewActive()) {
		//	getLiveViewImage();
		//}
	} catch (Maid::MaidError) {
		return false;
	}

	return true;
}

/**
 * throws MaidError
 */
bool MaidFacade::isLiveViewActive() {
	int32_t lvStatus = 0;
	sourceObject->capGet(kNkMAIDCapability_LiveViewStatus, kNkMAIDDataType_UnsignedPtr, (NKPARAM) &lvStatus);
	return lvStatus == 1;
}

/**
 * throws MaidError
 */
QImage MaidFacade::getLiveViewImage() {
	unsigned int headerSize = 0;
	NkMAIDArray dataArray;
	dataArray.pData = nullptr;
	int i = 0;
	std::unique_ptr<unsigned char[]> data;
	bool r = true;
	QImage empty = QImage();

	headerSize = 384;

	memset(&dataArray, 0, sizeof(NkMAIDArray));

	// check if everything is supported

	NkMAIDCapInfo capInfo;
	r = sourceObject->getCapInfo(kNkMAIDCapability_GetLiveViewImage, &capInfo);
	if (!r) {
		return empty;
	}

	r = sourceObject->hasCapOperation(kNkMAIDCapability_GetLiveViewImage, kNkMAIDCapOperation_Get);
	r = r && sourceObject->hasCapOperation(kNkMAIDCapability_GetLiveViewImage, kNkMAIDCapOperation_GetArray);
	if (!r) {
		return empty;
	}

	try {
		// get info about image, allocate memory
		sourceObject->capGet(kNkMAIDCapability_GetLiveViewImage, kNkMAIDDataType_ArrayPtr, (NKPARAM) &dataArray);
		dataArray.pData = new unsigned char[dataArray.ulElements * dataArray.wPhysicalBytes];
		data.reset((unsigned char*) dataArray.pData); // just for RAII/automatic deletion

		// get data
		sourceObject->capGetArray(kNkMAIDCapability_GetLiveViewImage, kNkMAIDDataType_ArrayPtr, (NKPARAM) &dataArray);
	} catch (Maid::MaidError) {
		return empty;
	}

	// construct a QImage out of the data and return it
	return QImage::fromData(((unsigned char*) dataArray.pData) + headerSize, dataArray.ulElements - headerSize, "JPEG");
}

std::pair<QStringList, size_t> MaidFacade::toQStringList(const StringValues& values) {
	QStringList list;
	for (auto& s : values.values) {
		list.append(QString::fromStdString(s));
	}
	return std::make_pair(list, values.currentValue);
}

NKERROR MaidFacade::processMaidData(NKREF ref, LPVOID info, LPVOID data) {
	NkMAIDDataInfo* dataInfo = static_cast<NkMAIDDataInfo*>(info);
	NkMAIDFileInfo* fileInfo = static_cast<NkMAIDFileInfo*>(info);
	NkMAIDImageInfo* imageInfo = static_cast<NkMAIDImageInfo*>(info);
	auto* r = static_cast<MaidFacade::DataProcData*>(ref);
	
	if (dataInfo->ulType & kNkMAIDDataObjType_File) {
		// initialize buffer
		if (r->offset == 0 && r->buffer == nullptr) {
			r->buffer = new char[fileInfo->ulTotalLength];
		}

		memcpy(r->buffer + r->offset, data, fileInfo->ulLength);
		r->offset += fileInfo->ulLength;

		if (r->offset >= fileInfo->ulTotalLength) {
			// file delivery is finished
			// the file will be written to disk in acquireItemObjectsFinished

			setCurrentFileData(r, info);
		}
	} else { // image
		return kNkMAIDResult_UnexpectedError;

		//unsigned long totalSize = imageInfo->ulRowBytes * imageInfo->szTotalPixels.h;
		//if (r->offset == 0 && r->buffer == nullptr) {
		//	r->buffer = new char[totalSize];
		//}
		//unsigned long byte = imageInfo->ulRowBytes * imageInfo->rData.h;
		//memcpy(r->buffer + r->offset, data, byte);
		//r->offset += byte;

		//if (r->offset >= totalSize) {
		//	std::string filename = makePictureFilename(dataInfo, nullptr);
		//	std::ofstream outFile;

		//	outFile.open(filename, std::ios::out | std::ios::binary);
		//	if (!outFile.good() || !outFile.is_open()) {
		//		return kNkMAIDResult_UnexpectedError;
		//	}

		//	outFile.write(r->buffer, totalSize);
		//	delete[] r->buffer;
		//	r->buffer = nullptr;
		//	r->offset = 0;
		//	outFile.close();
		//}
	}

	return kNkMAIDResult_NoError;
}

QString MaidFacade::makePictureFilename() {
	std::string prefix;
	std::string ext;

	if (currentFileDataInfo.ulType & kNkMAIDDataObjType_Image) {
		prefix = "Image";
	} else if (currentFileDataInfo.ulType & kNkMAIDDataObjType_Thumbnail) {
		prefix = "Thumb";
	} else {
		prefix = "Unknown";
	}

	switch (currentFileFileInfo.ulFileDataType) {
	case kNkMAIDFileDataType_JPEG:
		ext = "jpg";
		break;
	case kNkMAIDFileDataType_TIFF:
		ext = "tif";
		break;
	case kNkMAIDFileDataType_NIF:
		ext = "nef";
		break;
	//case kNkMAIDFileDataType_NDF:
	//	ext = "ndf";
	//	break;
	default:
		ext = "dat";
	}

	std::stringstream filenameStream;
	filenameStream << prefix << "." << ext;

	return QString::fromStdString(filenameStream.str());	// >DIR: std::strings are not too good with unicode - so our interfaces use QStrings [4.7.2014 markus]
}

QFileInfo MaidFacade::getLastFileInfo() {
	return lastFileInfo;
}

void MaidFacade::setCurrentFileData(DataProcData* fileData, void* info) {
	currentFileData = fileData;
	currentFileDataInfo = *static_cast<NkMAIDDataInfo*>(info);
	currentFileFileInfo = *static_cast<NkMAIDFileInfo*>(info);
}

void MaidFacade::progressCallbackUpdate(ULONG command, ULONG param, ULONG done, ULONG total) {
	if (command == kNkMAIDCommand_CapStart && param == kNkMAIDCapability_Acquire) {
		emit updateAcquireProgress(done, total);
	}
}

void MaidFacade::enumerateCapsAll() {
	moduleObject->enumCaps();
	sourceObject->enumCaps();
}

void CALLPASCAL CALLBACK eventProc(NKREF ref, ULONG eventType, NKPARAM data) {
	MaidFacade* maidFacade = (MaidFacade*) ref;

	switch (eventType) {
	case kNkMAIDEvent_AddChild:
		qDebug() << "A MAID child was added: " << data;
		maidFacade->startAcquireItemObjects();
		break;
	case kNkMAIDEvent_RemoveChild:
		qDebug() << "A MAID child was removed: " << data;
		break;
	case kNkMAIDEvent_WarmingUp:
		// The Type0007 Module does not use this event.
		break;
	case kNkMAIDEvent_WarmedUp:
		// The Type0007 Module does not use this event.
		break;
	case kNkMAIDEvent_CapChange:
		maidFacade->enumerateCapsAll();
		maidFacade->capValueChangeCallback(data);
		break;
	case kNkMAIDEvent_CapChangeValueOnly:
		maidFacade->capValueChangeCallback(data);
		break;
	case kNkMAIDEvent_OrphanedChildren:
		// TODO close children (source objects)
		break;
	default:
		qDebug() << "Unknown event in a MaidObject.";
	}

}

NKERROR CALLPASCAL CALLBACK dataProc(NKREF ref, LPVOID info, LPVOID data) {
	return static_cast<MaidFacade::DataProcData*>(ref)->maidFacade->processMaidData(ref, info, data);
}

void CALLPASCAL CALLBACK completionProc(
		LPNkMAIDObject object,
		ULONG command,
		ULONG param,
		ULONG dataType,
		NKPARAM data,
		NKREF refComplete,
		NKERROR result) {

	auto complData = static_cast<MaidFacade::CompletionProcData*>(refComplete);
	complData->result = result;
	(*complData->count)++;

	// if the operation is aquire, free the memory
	if(command == kNkMAIDCommand_CapStart && param == kNkMAIDCapability_Acquire) {
		// here, complData->data would be deleted
		// because shoot (Acquire) is threaded, this is now done in acquireItemObjectsFinished
	}
	
	if (complData != nullptr) {
		delete complData;
	}
}

void CALLPASCAL CALLBACK progressProc(
		ULONG command,
		ULONG param, 
		NKREF ref, 
		ULONG done, 
		ULONG total) {

	static_cast<MaidFacade::ProgressProcData*>(ref)->maidFacade->progressCallbackUpdate(command, param, done, total);
}