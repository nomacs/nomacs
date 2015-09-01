/*******************************************************************************************************
 DkImageContainer.cpp
 Created on:	21.02.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "DkMetaData.h"
#include "DkThumbs.h"
#include "DkBasicLoader.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkTimer.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QObject>
#include <QImage>
#include <QtConcurrentRun>

// quazip
#ifdef WITH_QUAZIP
#include <quazip/JlCompress.h>
#endif
#pragma warning(pop)		// no warnings from includes - end

#pragma warning(disable: 4251)	// TODO: remove


namespace nmc {

#ifdef WITH_QUAZIP
QString DkZipContainer::mZipMarker = "dIrChAr";
#endif

// DkImageContainer --------------------------------------------------------------------
/**
 * Creates a DkImageContainer.
 * This class is the basic image management class.
 * @param fileInfo the file of the given 
 **/ 
DkImageContainer::DkImageContainer(const QString& filePath) {
	
	setFilePath(filePath);
	loadState = not_loaded;
	init();
}

DkImageContainer::~DkImageContainer() {

}

void DkImageContainer::init() {

	edited = false;
	selected = false;
	
	// always keep in mind that a file does not exist
	if (!edited && loadState != exists_not)
		loadState = not_loaded;

}

bool DkImageContainer::operator==(const DkImageContainer& ric) const {
	return mFilePath == ric.filePath();
}

bool DkImageContainer::operator<=(const DkImageContainer& o) const {

	if (*this == o)
		return true;

	return *this < o;
}

bool DkImageContainer::operator<(const DkImageContainer& o) const {

	return imageContainerLessThan(*this, o);
}

bool DkImageContainer::operator>(const DkImageContainer& o) const {

	return !imageContainerLessThan(*this, o);
}

bool DkImageContainer::operator>=(const DkImageContainer& o) const {

	if (*this == o)
		return true;

	return !imageContainerLessThan(*this, o);
}


void DkImageContainer::clear() {

	if (loader)
		loader->release();
	if (fileBuffer)
		fileBuffer->clear();
	init();
}

QFileInfo DkImageContainer::fileInfo() const {
	return mFileInfo;
}

QString DkImageContainer::filePath() const {

	return mFilePath;
}

QString DkImageContainer::dirPath() const {
	
	return mFileInfo.absolutePath();
}

QString DkImageContainer::fileName() const {

	return mFileInfo.fileName();
}

bool DkImageContainer::isFromZip() {

#ifdef WITH_QUAZIP
	return getZipData() && getZipData()->isZip();
#else
	return false;
#endif
}

bool DkImageContainer::exists() {

#ifdef WITH_QUAZIP

	if (isFromZip())
		return true;
#endif

	return QFileInfo(mFilePath).exists();
}

QString DkImageContainer::getTitleAttribute() const {

	if (!loader || loader->getNumPages() <= 1)
		return QString();

	QString attr = "[" + QString::number(loader->getPageIdx()) + "/" + 
		QString::number(loader->getNumPages()) + "]";

	return attr;
}

QSharedPointer<DkBasicLoader> DkImageContainer::getLoader() {

	if (!loader) {
		this->loader = QSharedPointer<DkBasicLoader>(new DkBasicLoader());
	}

	return loader;
}

QSharedPointer<DkMetaDataT> DkImageContainer::getMetaData() {

	return getLoader()->getMetaData();
}

QSharedPointer<DkThumbNailT> DkImageContainer::getThumb() {

	if (!thumb) {
#ifdef WITH_QUAZIP	
		if(isFromZip()) 
			thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(getZipData()->getEncodedFileInfo()));
		else
			thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(mFilePath));
#else
		thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(mFilePath));
#endif	
	}

	return thumb;
}

QSharedPointer<QByteArray> DkImageContainer::getFileBuffer() {

	if (!fileBuffer) {
		fileBuffer = QSharedPointer<QByteArray>(new QByteArray());
	}

	return fileBuffer;
}

float DkImageContainer::getMemoryUsage() const {

	if (!loader)
		return 0;

	float memSize = fileBuffer ? fileBuffer->size()/(1024.0f*1024.0f) : 0;
	memSize += DkImage::getBufferSizeFloat(loader->image().size(), loader->image().depth());

	return memSize;
}

float DkImageContainer::getFileSize() const {

	return QFileInfo(mFilePath).size()/(1024.0f*1024.0f);
}


QImage DkImageContainer::image() {

	if (getLoader()->image().isNull() && getLoadState() == not_loaded)
		loadImage();

	return loader->image();
}

void DkImageContainer::setImage(const QImage& img) {

	setImage(img, mFilePath);
}

void DkImageContainer::setImage(const QImage& img, const QString& filePath) {

	setFilePath(mFilePath);
	getLoader()->setImage(img, filePath);
	edited = true;
}

void DkImageContainer::setFilePath(const QString& filePath) {

	mFilePath = filePath;
	mFileInfo = filePath;

#ifdef WIN32
#if QT_VERSION < 0x050000
	mFileNameStr = fileName().toStdWString();
#else
	mFileNameStr = DkUtils::qStringToStdWString(fileName());
#endif
#endif

}

bool DkImageContainer::hasImage() const {

	if (!loader)
		return false;

	return loader->hasImage();
}

int DkImageContainer::getLoadState() const {

	return loadState;
}

bool DkImageContainer::loadImage() {

	if (getFileBuffer()->isEmpty())
		fileBuffer = loadFileToBuffer(mFilePath);

	loader = loadImageIntern(mFilePath, getLoader(), fileBuffer);

	return loader->hasImage();
}

bool DkImageContainer::saveImage(const QString& filePath, int compression /* = -1 */) {
	return saveImage(filePath, getLoader()->image(), compression);
}

bool DkImageContainer::saveImage(const QString& filePath, const QImage saveImg, int compression /* = -1 */) {

	QFileInfo saveFile = saveImageIntern(filePath, getLoader(), saveImg, compression);

	saveFile.refresh();
	qDebug() << "save file: " << saveFile.absoluteFilePath();

	return saveFile.exists() && saveFile.isFile();
}

QSharedPointer<QByteArray> DkImageContainer::loadFileToBuffer(const QString& filePath) {

	QFileInfo fInfo = filePath;

	if (fInfo.isSymLink())
		fInfo = fInfo.symLinkTarget();

#ifdef WITH_QUAZIP
	if (isFromZip()) 
		return getZipData()->extractImage(getZipData()->getZipFileInfo(), getZipData()->getImageFileInfo());
#endif

	if (fInfo.suffix().contains("psd")) {	// for now just psd's are not cached because their file might be way larger than the part we need to read
		return QSharedPointer<QByteArray>(new QByteArray());
	}

	QFile file(fInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	QSharedPointer<QByteArray> ba(new QByteArray(file.readAll()));
	file.close();

	return ba;
}


QSharedPointer<DkBasicLoader> DkImageContainer::loadImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, const QSharedPointer<QByteArray> fileBuffer) {

	try {
		loader->loadGeneral(filePath, fileBuffer, true);
	} catch(...) {}

	return loader;
}

QString DkImageContainer::saveImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QImage saveImg, int compression) {

	return loader->save(filePath, saveImg, compression);
}

void DkImageContainer::saveMetaData() {

	if (!loader)
		return;

	saveMetaDataIntern(mFilePath, loader, fileBuffer);
}

void DkImageContainer::saveMetaDataIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QSharedPointer<QByteArray> fileBuffer) {

	loader->saveMetaData(filePath, fileBuffer);
}

void DkImageContainer::setEdited(bool edited) {
	this->edited = edited;
}

bool DkImageContainer::isEdited() const {

	return edited;
}

bool DkImageContainer::isSelected() const {

	return selected;
}

bool DkImageContainer::setPageIdx(int skipIdx) {

	return getLoader()->setPageIdx(skipIdx);
}


#ifdef WITH_QUAZIP
QSharedPointer<DkZipContainer> DkImageContainer::getZipData() {

	if (!zipData) {
		this->zipData = QSharedPointer<DkZipContainer>(new DkZipContainer(mFilePath));
		if (zipData->isZip())
			setFilePath(zipData->getImageFileInfo());
	}

	return zipData;
}
#endif
#ifdef WIN32
std::wstring DkImageContainer::getFileNameWStr() const {
	
	return mFileNameStr;
}
#endif

bool imageContainerLessThanPtr(const QSharedPointer<DkImageContainer> l, const QSharedPointer<DkImageContainer> r) {

	if (!l || !r)
		return false;

	return imageContainerLessThan(*l, *r);
}

bool imageContainerLessThan(const DkImageContainer& l, const DkImageContainer& r) {

	switch(DkSettings::global.sortMode) {

	case DkSettings::sort_filename:
#ifdef WIN32
		// not beautiful if you take a look at the code, but:
		// time on Win8 with compFilename:
		//		WinAPI, indexed ( 73872 ) files in:  " 92 ms"
		//		[DkImageLoader]  73872  containers created in  " 1.825 sec"
		//		[DkImageLoader] after sorting:  " 52.246 sec"
		// time on Win8 with direct wCompLogic:
		//		WinAPI, indexed ( 73872 ) files in:  " 63 ms"
		//		[DkImageLoader]  73872  containers created in  " 1.203 sec"
		//		[DkImageLoader] after sorting:  " 14.407 sec"
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::wCompLogic(l.getFileNameWStr(), r.getFileNameWStr());
		else
			return !DkUtils::wCompLogic(l.getFileNameWStr(), r.getFileNameWStr());
		break;
#else
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compFilename(l.fileInfo(), r.fileInfo());
		else
			return DkUtils::compFilenameInv(l.fileInfo(), r.fileInfo());
		break;
#endif

	case DkSettings::sort_date_created:
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compDateCreated(l.fileInfo(), r.fileInfo());
		else
			return DkUtils::compDateCreatedInv(l.fileInfo(), r.fileInfo());
		break;

	case DkSettings::sort_date_modified:
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compDateModified(l.fileInfo(), r.fileInfo());
		else
			return DkUtils::compDateModifiedInv(l.fileInfo(), r.fileInfo());

	case DkSettings::sort_random:
		return DkUtils::compRandom(l.fileInfo(), r.fileInfo());

	default:
		// filename
		return DkUtils::compFilename(l.fileInfo(), r.fileInfo());
	}
	
}

// DkImageContainerT --------------------------------------------------------------------
DkImageContainerT::DkImageContainerT(const QString& filePath) : DkImageContainer(filePath) {
	
	fetchingImage = false;
	fetchingBuffer = false;
	
	// our file watcher
	fileUpdateTimer.setSingleShot(false);
	fileUpdateTimer.setInterval(500);
	waitForUpdate = false;
	downloaded = false;

	connect(&fileUpdateTimer, SIGNAL(timeout()), this, SLOT(checkForFileUpdates()), Qt::UniqueConnection);
	//connect(&metaDataWatcher, SIGNAL(finished()), this, SLOT(metaDataLoaded()));
}

DkImageContainerT::~DkImageContainerT() {
	
	bufferWatcher.blockSignals(true);
	bufferWatcher.cancel();
	imageWatcher.blockSignals(true);
	imageWatcher.cancel();

	saveMetaData();

	// we have to wait here
	saveMetaDataWatcher.blockSignals(true);
	saveImageWatcher.blockSignals(true);
}

void DkImageContainerT::clear() {

	cancel();

	if (fetchingImage || fetchingBuffer)
		return;

	DkImageContainer::clear();
}

void DkImageContainerT::checkForFileUpdates() {

#ifdef WITH_QUAZIP
	if(isFromZip()) 
		setFilePath(getZipData()->getZipFileInfo());
#endif

	QDateTime modifiedBefore = fileInfo().lastModified();
	mFileInfo.refresh();
	
	bool changed = false;

	// if image exists_not don't do this
	if (!mFileInfo.exists() && loadState == loaded) {
		changed = true;
	}

	if (mFileInfo.lastModified() != modifiedBefore)
		waitForUpdate = true;

#ifdef WITH_QUAZIP
	if(isFromZip()) 
		setFilePath(getZipData()->getImageFileInfo());
#endif

	if (changed) {
		fileUpdateTimer.stop();
		if (DkSettings::global.askToSaveDeletedFiles) {
			edited = changed;
			emit fileLoadedSignal(true);
		}
		return;
	}

	// we use our own file watcher, since the qt watcher
	// uses locks to check for updates. this might
	// be more accurate. however, the locks are pretty nasty
	// if the user e.g. wants to delete the file while watching
	// it in nomacs
	if (waitForUpdate && mFileInfo.isReadable()) {
		waitForUpdate = false;
		getThumb()->setImage(QImage());
		loadImageThreaded(true);
	}

}

bool DkImageContainerT::loadImageThreaded(bool force) {

#ifdef WITH_QUAZIP
	//zip archives: get zip file fileInfo for checks
	if(isFromZip()) 
		setFilePath(getZipData()->getZipFileInfo());
#endif
	
	// check file for updates
	QFileInfo fileInfo = filePath();
	QDateTime modifiedBefore = fileInfo.lastModified();
	fileInfo.refresh();

	if (force || fileInfo.lastModified() != modifiedBefore || getLoader()->isDirty()) {
		qDebug() << "updating image...";
		getThumb()->setImage(QImage());
		clear();
	}

	// null file?
	if (fileInfo.fileName().isEmpty() || !fileInfo.exists()) {

		QString msg = tr("Sorry, the file: %1 does not exist... ").arg(fileName());
		emit showInfoSignal(msg);
		loadState = exists_not;
		return false;
	}
	else if (!fileInfo.permission(QFile::ReadUser)) {

		QString msg = tr("Sorry, you are not allowed to read: %1").arg(fileName());
		emit showInfoSignal(msg);
		loadState = exists_not;
		return false;
	}

#ifdef WITH_QUAZIP
	//zip archives: use the image file info from now on
	if(isFromZip()) 
		setFilePath(getZipData()->getImageFileInfo());
#endif
	
	loadState = loading;
	fetchFile();
	return true;
}

void DkImageContainerT::fetchFile() {
	
	if (fetchingBuffer && getLoadState() == loading_canceled) {
		loadState = loading;	// uncancel loading - we had another call
		return;
	}
	if (fetchingImage)
		imageWatcher.waitForFinished();
	// I think we missed to return here
	if (fetchingBuffer)
		return;

	// ignore doubled calls
	if (fileBuffer && !fileBuffer->isEmpty()) {
		bufferLoaded();
		return;
	}

	fetchingBuffer = true;	// saves the threaded call
	connect(&bufferWatcher, SIGNAL(finished()), this, SLOT(bufferLoaded()), Qt::UniqueConnection);

	bufferWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadFileToBuffer, filePath()));
}

void DkImageContainerT::bufferLoaded() {

	fetchingBuffer = false;

	if (!bufferWatcher.isCanceled())
		fileBuffer = bufferWatcher.result();

	if (getLoadState() == loading)
		fetchImage();
	else if (getLoadState() == loading_canceled) {
		loadState = not_loaded;
		clear();
		return;
	}
}

void DkImageContainerT::fetchImage() {

	if (fetchingBuffer)
		bufferWatcher.waitForFinished();

	if (fetchingImage) {
		loadState = loading;
		return;
	}

	if (getLoader()->hasImage() || /*!fileBuffer || fileBuffer->isEmpty() ||*/ loadState == exists_not) {
		loadingFinished();
		return;
	}
	
	qDebug() << "fetching: " << filePath();
	fetchingImage = true;

	connect(&imageWatcher, SIGNAL(finished()), this, SLOT(imageLoaded()), Qt::UniqueConnection);

	imageWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadImageIntern, filePath(), loader, fileBuffer));
}

void DkImageContainerT::imageLoaded() {

	fetchingImage = false;

	if (getLoadState() == loading_canceled) {
		loadState = not_loaded;
		clear();
		return;
	}

	// deliver image
	loader = imageWatcher.result();

	loadingFinished();
}

void DkImageContainerT::loadingFinished() {

	DkTimer dt;

	if (getLoadState() == loading_canceled) {
		loadState = not_loaded;
		clear();
		return;
	}

	if (!getLoader()->hasImage()) {
		fileUpdateTimer.stop();
		edited = false;
		QString msg = tr("Sorry, I could not load: %1").arg(fileName());
		emit showInfoSignal(msg);
		emit fileLoadedSignal(false);
		loadState = exists_not;
		return;
	}
	else if (!getThumb()->hasImage()) {
		getThumb()->setImage(getLoader()->image());
	}

	// clear file buffer if it exceeds a certain size?! e.g. psd files
	if (fileBuffer && fileBuffer->size()/(1024.0f*1024.0f) > DkSettings::resources.cacheMemory*0.5f)
		fileBuffer->clear();
	
	loadState = loaded;
	emit fileLoadedSignal(true);
}

void DkImageContainerT::downloadFile(const QUrl& url) {

	if (!fileDownloader) {
		fileDownloader = QSharedPointer<FileDownloader>(new FileDownloader(url, this));
		connect(fileDownloader.data(), SIGNAL(downloaded()), this, SLOT(fileDownloaded()), Qt::UniqueConnection);
		qDebug() << "trying to download: " << url;
	}
	else
		fileDownloader->downloadFile(url);
}

void DkImageContainerT::fileDownloaded() {

	if (!fileDownloader) {
		qDebug() << "empty fileDownloader, where it should not be";
		emit fileLoadedSignal(false);
		return;
	}

	fileBuffer = fileDownloader->downloadedData();

	if (!fileBuffer || fileBuffer->isEmpty()) {
		qDebug() << fileDownloader->getUrl() << " not downloaded...";
		edited = false;
		emit showInfoSignal(tr("Sorry, I could not download:\n%1").arg(fileDownloader->getUrl().toString()));
		emit fileLoadedSignal(false);
		loadState = exists_not;
		return;
	}

	downloaded = true;
	fetchImage();
}

void DkImageContainerT::cancel() {

	if (loadState != loading)
		return;

	loadState = loading_canceled;
}

void DkImageContainerT::receiveUpdates(QObject* obj, bool connectSignals /* = true */) {

	// !selected - do not connect twice
	if (connectSignals && !selected) {
		connect(this, SIGNAL(errorDialogSignal(const QString&)), obj, SLOT(errorDialog(const QString&)), Qt::UniqueConnection);
		connect(this, SIGNAL(fileLoadedSignal(bool)), obj, SLOT(imageLoaded(bool)), Qt::UniqueConnection);
		connect(this, SIGNAL(showInfoSignal(QString, int, int)), obj, SIGNAL(showInfoSignal(QString, int, int)), Qt::UniqueConnection);
		connect(this, SIGNAL(fileSavedSignal(const QString&, bool)), obj, SLOT(imageSaved(const QString&, bool)), Qt::UniqueConnection);
		fileUpdateTimer.start();
	}
	else if (!connectSignals) {
		disconnect(this, SIGNAL(errorDialogSignal(const QString&)), obj, SLOT(errorDialog(const QString&)));
		disconnect(this, SIGNAL(fileLoadedSignal(bool)), obj, SLOT(imageLoaded(bool)));
		disconnect(this, SIGNAL(showInfoSignal(QString, int, int)), obj, SIGNAL(showInfoSignal(QString, int, int)));
		disconnect(this, SIGNAL(fileSavedSignal(const QString&, bool)), obj, SLOT(imageSaved(const QString&, bool)));
		fileUpdateTimer.stop();
	}

	selected = connectSignals;

}

void DkImageContainerT::saveMetaDataThreaded() {

	if (!exists() || getLoader()->getMetaData() && !getLoader()->getMetaData()->isDirty())
		return;

	fileUpdateTimer.stop();
	QFuture<void> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::saveMetaDataIntern, filePath(), getLoader(), getFileBuffer());

}

bool DkImageContainerT::saveImageThreaded(const QString& filePath, int compression /* = -1 */) {

	return saveImageThreaded(filePath, getLoader()->image(), compression);
}


bool DkImageContainerT::saveImageThreaded(const QString& filePath, const QImage saveImg, int compression /* = -1 */) {

	saveImageWatcher.waitForFinished();

	QFileInfo fInfo = filePath;

	if (saveImg.isNull()) {
		QString msg = tr("I can't save an empty file, sorry...\n");
		emit errorDialogSignal(msg);
		return false;
	}
	if (!fInfo.absoluteDir().exists()) {
		QString msg = tr("Sorry, the directory: %1  does not exist\n").arg(filePath);
		emit errorDialogSignal(msg);
		return false;
	}
	if (fInfo.exists() && !fInfo.isWritable()) {
		QString msg = tr("Sorry, I can't write to the file: %1").arg(fInfo.fileName());
		emit errorDialogSignal(msg);
		return false;
	}

	qDebug() << "attempting to save: " << filePath;

	fileUpdateTimer.stop();
	connect(&saveImageWatcher, SIGNAL(finished()), this, SLOT(savingFinished()), Qt::UniqueConnection);

	saveImageWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkImageContainerT::saveImageIntern, filePath, loader, saveImg, compression));

	return true;
}

void DkImageContainerT::savingFinished() {

	QString savePath = saveImageWatcher.result();
	
	QFileInfo sInfo = savePath;
	sInfo.refresh();
	qDebug() << "save file: " << savePath;
	
	if (!sInfo.exists() || !sInfo.isFile())
		emit fileSavedSignal(savePath, false);
	else {
		//// reset thumb - loadImageThreaded should do it anyway
		//thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(saveFile, loader->image()));

		if (fileBuffer)
			fileBuffer->clear();	// do a complete clear?
		setFilePath(savePath);
		edited = false;
		downloaded = false;
		if (selected) {
			loadImageThreaded(true);	// force a reload
			fileUpdateTimer.start();
		}
		emit fileSavedSignal(savePath);
	}
}

QSharedPointer<QByteArray> DkImageContainerT::loadFileToBuffer(const QString& filePath) {

	return DkImageContainer::loadFileToBuffer(filePath);
}

QSharedPointer<DkBasicLoader> DkImageContainerT::loadImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, const QSharedPointer<QByteArray> fileBuffer) {

	return DkImageContainer::loadImageIntern(filePath, loader, fileBuffer);
}

QString DkImageContainerT::saveImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QImage saveImg, int compression) {

	qDebug() << "saveImage in T: " << filePath;

	return DkImageContainer::saveImageIntern(filePath, loader, saveImg, compression);
}

void DkImageContainerT::saveMetaDataIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QSharedPointer<QByteArray> fileBuffer) {

	return DkImageContainer::saveMetaDataIntern(filePath, loader, fileBuffer);
}

QSharedPointer<DkBasicLoader> DkImageContainerT::getLoader() {

	if (!loader) {
		DkImageContainer::getLoader();
		connect(loader.data(), SIGNAL(errorDialogSignal(const QString&)), this, SIGNAL(errorDialogSignal(const QString&)));
	}

	return loader;
}

QSharedPointer<DkThumbNailT> DkImageContainerT::getThumb() {

	if (!thumb) {
		DkImageContainer::getThumb();
		connect(thumb.data(), SIGNAL(thumbLoadedSignal(bool)), this, SIGNAL(thumbLoadedSignal(bool)));
	}

	return thumb;
}

bool DkImageContainerT::isFileDownloaded() const {

	return downloaded;
}

};