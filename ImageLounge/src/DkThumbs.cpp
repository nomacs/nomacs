/*******************************************************************************************************
 DkThumbs.cpp
 Created on:	19.04.2013
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

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

#include "DkThumbs.h"

#include "DkImage.h"

namespace nmc {


int DkThumbsLoader::maxThumbSize = 160;

/**
* Default constructor.
* @param file the corresponding file
* @param img the thumbnail image
**/ 
DkThumbNail::DkThumbNail(QFileInfo file, QImage img) {
	this->img = img;
	this->file = file;
	this->maxThumbSize = 160;
	this->minThumbSize = DkSettings::display.thumbSize;
	this->rescale = true;
	imgExists = true;
	s = qMax(img.width(), img.height());
};

void DkThumbNail::compute(bool forceLoad /* = false */, bool forceSave /* = false */) {
	
	// we do this that complicated to be thread-safe
	// if we use member vars in the thread and the object gets deleted during thread execution we crash...
	this->img = computeIntern(file, forceLoad, forceSave, maxThumbSize, minThumbSize, rescale);
}

/**
 * Loads the thumbnail from the metadata.
 * If no thumbnail is embedded, the whole image
 * is loaded and downsampled in a fast manner.
 * @param file the file to be loaded
 * @return QImage the loaded image. Null if no image
 * could be loaded at all.
 **/ 
QImage DkThumbNail::computeIntern(QFileInfo file, bool forceLoad, bool forceSave, int maxThumbSize, int minThumbSize, bool rescale) {
	
	DkTimer dt;
	qDebug() << "[thumb] file: " << file.absoluteFilePath();
	//if (file.fileName().contains("__doris-gray.jp2"))
	//	qDebug() << "the wrong file...";

	//// see if we can read the thumbnail from the exif data
	DkMetaData dataExif(file);
	QImage thumb = dataExif.getThumbnail();
	removeBlackBorder(thumb);
	int orientation = dataExif.getOrientation();
	int imgW = thumb.width();
	int imgH = thumb.height();
	int tS = minThumbSize;

	// as found at: http://olliwang.com/2010/01/30/creating-thumbnail-images-in-qt/
	QString filePath = (file.isSymLink()) ? file.symLinkTarget() : file.absoluteFilePath();
	QImageReader imageReader(filePath);

	if (thumb.isNull() || thumb.width() < tS && thumb.height() < tS) {

		imgW = imageReader.size().width();
		imgH = imageReader.size().height();	// locks the file!
	}
	//else if (!thumb.isNull())
	//	qDebug() << "EXIV thumb loaded: " << thumb.width() << " x " << thumb.height();
	
	if (rescale && (imgW > maxThumbSize || imgH > maxThumbSize)) {
		if (imgW > imgH) {
			imgH = (float)maxThumbSize / imgW * imgH;
			imgW = maxThumbSize;
		} 
		else if (imgW < imgH) {
			imgW = (float)maxThumbSize / imgH * imgW;
			imgH = maxThumbSize;
		}
		else {
			imgW = maxThumbSize;
			imgH = maxThumbSize;
		}
	}

	if (thumb.isNull() || thumb.width() < tS && thumb.height() < tS || forceLoad) {
		
		// flip size if the image is rotated by 90°
		if (dataExif.isTiff() && abs(orientation) == 90) {
			int tmpW = imgW;
			imgW = imgH;
			imgH = tmpW;
		}

		QSize initialSize = imageReader.size();

		imageReader.setScaledSize(QSize(imgW, imgH));
		thumb = imageReader.read();

		// try to read the image
		if (thumb.isNull()) {
			DkBasicLoader loader;
			
			if (loader.loadGeneral(file, true))
				thumb = loader.image();
		}

		// the image is not scaled correctly yet
		if (rescale && !thumb.isNull() && (imgW == -1 || imgH == -1)) {
			imgW = thumb.width();
			imgH = thumb.height();

			if (imgW > maxThumbSize || imgH > maxThumbSize) {
				if (imgW > imgH) {
					imgH = (float)maxThumbSize / imgW * imgH;
					imgW = maxThumbSize;
				} 
				else if (imgW < imgH) {
					imgW = (float)maxThumbSize / imgH * imgW;
					imgH = maxThumbSize;
				}
				else {
					imgW = maxThumbSize;
					imgH = maxThumbSize;
				}
			}

			thumb = thumb.scaled(QSize(imgW*2, imgH*2), Qt::KeepAspectRatio, Qt::FastTransformation);
			thumb = thumb.scaled(QSize(imgW, imgH), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		// is there a nice solution to do so??
		imageReader.setFileName("josef");	// image reader locks the file -> but there should not be one so we just set it to another file...

		// there seems to be a bug in exiv2
		if ((initialSize.width() > 400 || initialSize.height() > 400) && (forceSave || DkSettings::display.saveThumb)) {
			
			try {
				dataExif.saveThumbnail(thumb, QFileInfo(filePath));
			} catch (DkException de) {
				// do nothing -> the file type does not support meta data
			}
			catch (...) {

				if (!DkImageLoader::restoreFile(QFileInfo(filePath)))
					qDebug() << "could not save thumbnail for: " << filePath;
			}
		}

	}
	else if (rescale) {
		thumb = thumb.scaled(QSize(imgW, imgH), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		//qDebug() << "thumb loaded from exif...";
	}

	if (orientation != -1 && !dataExif.isTiff()) {
		QTransform rotationMatrix;
		rotationMatrix.rotate((double)orientation);
		thumb = thumb.transformed(rotationMatrix);
	}

	//qDebug() << "[thumb] " << file.fileName() << " loaded in: " << QString::fromStdString(dt.getTotal());

	//if (!thumb.isNull())
	//	qDebug() << "thumb: " << thumb.width() << " x " << thumb.height();


	return thumb;
}

void DkThumbNail::removeBlackBorder(QImage& img) {

	int rIdx = 0;
	bool nonblack = false;
	
	for ( ; rIdx < qRound(img.height()*0.1); rIdx++) {

		const QRgb* pixel = (QRgb*)(img.constScanLine(rIdx));

		for (int cIdx = 0; cIdx < img.width(); cIdx++, pixel++) {

			// > 50 due to jpeg (normally we would want it to be != 0)
			if (qRed(*pixel) > 50 || qBlue(*pixel) > 50 || qGreen(*pixel) > 50) {
				nonblack = true;
				break;
			}
		}

		if (nonblack)
			break;
	}

	// non black border?
	if (rIdx == -1 || rIdx > 15)
		return;

	int rIdxB = img.height()-1;
	nonblack = false;

	for ( ; rIdxB >= qRound(img.height()*0.9f); rIdxB--) {

		const QRgb* pixel = (QRgb*)(img.constScanLine(rIdxB));

		for (int cIdx = 0; cIdx < img.width(); cIdx++, pixel++) {

			if (qRed(*pixel) > 50 || qBlue(*pixel) > 50 || qGreen(*pixel) > 50) {
				nonblack = true;
				break;
			}
		}

		if (nonblack) {
			rIdxB--;
			break;
		}
	}

	// remove black borders
	if (rIdx < rIdxB)
		img = img.copy(0, rIdx, img.width(), rIdxB-rIdx);

}

DkThumbNailT::DkThumbNailT(QFileInfo file, QImage img) : DkThumbNail(file, img) {

	connect(&watcher, SIGNAL(finished()), this, SLOT(thumbLoaded()));
}

DkThumbNailT::~DkThumbNailT() {
	watcher.blockSignals(true);
	watcher.cancel();
}

void DkThumbNailT::fetchThumb(bool forceLoad /* = false */, bool forceSave /* = false */) {

	
	if (!img.isNull() || !imgExists || watcher.isRunning())
		return;

	QFuture<QImage> future = QtConcurrent::run(this, 
		&nmc::DkThumbNailT::computeCall, forceLoad, forceSave);

	watcher.setFuture(future);
}

QImage DkThumbNailT::computeCall(bool forceLoad, bool forceSave) {

	return DkThumbNail::computeIntern(file, forceLoad, forceSave, maxThumbSize, minThumbSize, rescale);
}

void DkThumbNailT::thumbLoaded() {
	
	QFuture<QImage> future = watcher.future();

	this->img = future.result();
	
	if (!img.isNull())
		emit thumbUpdated();
	else
		imgExists = false;
}

// DkThumbPool --------------------------------------------------------------------
DkThumbPool::DkThumbPool(QFileInfo file /* = QFileInfo */, QObject* parent /* = 0 */) : QObject(parent) {
	this->currentFile = file;
}

void DkThumbPool::setFile(const QFileInfo& file, int force) {

	if (!listenerList.empty() && (force == DkThumbsLoader::user_updated || dir(currentFile) != dir(file)))
		indexDir(file);
	else if (!listenerList.empty() && force == DkThumbsLoader::dir_updated)
		updateDir(file);

	if (currentFile != file || force != DkThumbsLoader::not_forced)
		emit newFileIdxSignal(fileIdx(file));

	currentFile = file;
	qDebug() << "[thumbpool] current file: " << currentFile.absoluteFilePath();
}

QFileInfo DkThumbPool::getCurrentFile() {
	return currentFile;
}

QDir DkThumbPool::dir(const QFileInfo& file) const {

	return (file.isDir()) ? QDir(file.absoluteFilePath()) : file.absoluteDir();
}

int DkThumbPool::fileIdx(const QFileInfo& file) {

	int tIdx = -1;
	
	for (int idx = 0; idx < thumbs.size(); idx++) {
		if (file == thumbs.at(idx)->getFile()) {
			tIdx = idx;
			break;
		}
	}

	return tIdx;
}

int DkThumbPool::getCurrentFileIdx() {

	if (thumbs.empty())
		indexDir(currentFile);
	
	return fileIdx(currentFile);
}

QVector<QSharedPointer<DkThumbNailT> > DkThumbPool::getThumbs() {

	if (thumbs.empty())
		indexDir(currentFile);
	
	emit newFileIdxSignal(getCurrentFileIdx());

	return thumbs;
}

void DkThumbPool::getUpdates(QObject* obj, bool isActive) {

	bool registered = false;
	for (int idx = 0; idx < listenerList.size(); idx++) {

		if (!isActive && listenerList.at(idx) == obj) {
			listenerList.remove(idx);
			break;
		}
		else if (isActive && listenerList.at(idx) == obj) {
			registered = true;
			break;
		}
	}

	if (!registered && isActive)
		listenerList.append(obj);

}

void DkThumbPool::indexDir(const QFileInfo& currentFile) {

	thumbs.clear();

	// imho this is a Qt bug
	QDir cDir = dir(currentFile);

	files = DkImageLoader::getFilteredFileList(cDir);

	for (int idx = 0; idx < files.size(); idx++) {
		QSharedPointer<DkThumbNailT> t = createThumb(QFileInfo(cDir, files.at(idx)));
		thumbs.append(t);
	}
	
	if (!thumbs.empty())
		emit numThumbChangedSignal();

}

void DkThumbPool::updateDir(const QFileInfo& currentFile) {

	QVector<QSharedPointer<DkThumbNailT> > newThumbs;

	QDir cDir = dir(currentFile);
	files = DkImageLoader::getFilteredFileList(cDir);

	for (int idx = 0; idx < files.size(); idx++) {

		int fIdx = fileIdx(QFileInfo(cDir, files.at(idx)));

		if (fIdx != -1)
			newThumbs.append(thumbs.at(fIdx));
		else {
			QSharedPointer<DkThumbNailT> t = createThumb(QFileInfo(cDir, files.at(idx)));
			newThumbs.append(t);
		}
	}
	
	if (!thumbs.empty() && thumbs.size() != newThumbs.size())
		emit numThumbChangedSignal();

	thumbs = newThumbs;
}

QSharedPointer<DkThumbNailT> DkThumbPool::createThumb(const QFileInfo& file) {

	QSharedPointer<DkThumbNailT> thumb(new DkThumbNailT(file));
	connect(thumb.data(), SIGNAL(thumbUpdated()), this, SLOT(thumbUpdated()));
	return thumb;
}

void DkThumbPool::thumbUpdated() {

	// maybe we have to add a timer here to ignore too many calls at the same time
	emit thumbUpdatedSignal();
}


/**
 * Default constructor of the thumbnail loader.
 * Note: currently the init calls the getFilteredFileList which might be slow.
 * @param thumbs a pointer to an array holding the thumbnails. while
 * loading, the thumbsloader will add all images to this array. however, the
 * caller must destroy the thumbs vector.
 * @param dir the directory where thumbnails should be loaded from.
 **/ 
DkThumbsLoader::DkThumbsLoader(std::vector<DkThumbNail>* thumbs, QDir dir, QStringList files) {

	this->thumbs = thumbs;
	this->dir = dir;
	this->isActive = true;
	this->files = files;
	init();
}

/**
 * Initializes the thumbs loader.
 * Note: getFilteredFileList might be slow.
 **/ 
void DkThumbsLoader::init() {

	if (files.empty())
		files = DkImageLoader::getFilteredFileList(dir);
	startIdx = -1;
	endIdx = -1;
	somethingTodo = false;
	numFilesLoaded = 0;
	loadAllThumbs = false;
	forceSave = false;
	forceLoad = false;

	// here comes hot stuff (for a better update policy)
	std::vector<DkThumbNail> oldThumbs = *thumbs;
	thumbs->clear();

	DkTimer dt;
	for (int idx = 0; idx < files.size(); idx++) {
		QFileInfo cFile = QFileInfo(dir, files[idx]);

		DkThumbNail cThumb = DkThumbNail(cFile);

		for (unsigned int idx = 0; idx < oldThumbs.size(); idx++) {

			if (cThumb == oldThumbs[idx]) {
				cThumb = oldThumbs[idx];
				break;
			}
		}

		thumbs->push_back(cThumb);
	}

	qDebug() << "thumb stubs loaded in: " << QString::fromStdString(dt.getTotal());
}

/**
 * Returns the file idx of the file specified.
 * @param file the file to be queried.
 * @return int the index of the file.
 **/ 
int DkThumbsLoader::getFileIdx(QFileInfo& file) {

	//mutex.lock();

	if (!file.exists() || !thumbs)
		return -1;

	QString cFilePath = file.absoluteFilePath();
	unsigned int fileIdx = 0;
	for ( ; fileIdx < thumbs->size(); fileIdx++) {

		if (thumbs->at(fileIdx).getFile().absoluteFilePath() == cFilePath)
			break;
	}

	if (fileIdx == thumbs->size()) fileIdx = -1;

	//mutex.unlock();

	return fileIdx;

}

/**
 * Thread routine.
 * Only loads thumbs if somethingTodo is true.
 **/ 
void DkThumbsLoader::run() {

	if (!thumbs)
		return;

	while (true) {

		if (loadAllThumbs && numFilesLoaded >= (int)thumbs->size()) {
			qDebug() << "[thumbs] thinks he has finished...";
			break;
		}

		mutex.lock();
		DkTimer dt;
		msleep(100);

		//QMutexLocker(&this->mutex);
		if (!isActive) {
			qDebug() << "thumbs loader stopped...";
			mutex.unlock();
			break;
		}
		mutex.unlock();

		if (somethingTodo)
			loadThumbs();
	}

	//// locate the current file
	//QStringList files = dir.entryList(DkImageLoader::fileFilters);

	//DkTimer dtt;

	//for (int idx = 0; idx < files.size(); idx++) {

	//	QMutexLocker(&this->mutex);
	//	if (!isActive) {
	//		break;
	//	}

	//	QFileInfo cFile = QFileInfo(dir, files[idx]);

	//	if (!cFile.exists() || !cFile.isReadable())
	//		continue;

	//	QImage img = getThumbNailQt(cFile);
	//	//QImage img = getThumbNailWin(cFile);
	//	thumbs->push_back(DkThumbNail(cFile, img));
	//}

}

/**
 * Loads thumbnails from the metadata.
 **/ 
void DkThumbsLoader::loadThumbs() {

	std::vector<DkThumbNail>::iterator thumbIter = thumbs->begin()+startIdx;
	qDebug() << "start: " << startIdx << " end: " << endIdx;

	for (int idx = startIdx; idx < endIdx; idx++, thumbIter++) {

		mutex.lock();

		// jump to new start idx
		if (startIdx > idx) {
			thumbIter = thumbs->begin()+startIdx;
			idx = startIdx;
		}

		// does somebody want me to stop?
		if (!isActive ) {
			mutex.unlock();
			return;
		}
		
		// TODO:  he breaks here! (crash detected)
		DkThumbNail* thumb = &(*thumbIter);
		if (!thumb->hasImage()) {
			thumb->compute(forceLoad, forceSave);
			if (thumb->hasImage())	// could I load the thumb?
				emit updateSignal();
			else {
				thumb->setImgExists(false);
				qDebug() << "image does NOT exist...";
			}
			
		}
		emit numFilesSignal(++numFilesLoaded);
		mutex.unlock();
	}

	somethingTodo = false;
}

/**
 * Here you can specify which thumbnails to load.
 * Note: it is not a good idea to load all thumbnails
 * of a folder (might be a lot : )
 * @param start the start index
 * @param end the end index
 **/ 
void DkThumbsLoader::setLoadLimits(int start, int end) {

	//QMutexLocker(&this->mutex);
	//if (start < startIdx || startIdx == -1)	startIdx = (start >= 0 && start < thumbs->size()) ? start : 0;
	//if (end > endIdx || endIdx == -1)		endIdx = (end > 0 && end < thumbs->size()) ? end : thumbs->size();
	startIdx = (start >= 0 && (unsigned int) start < thumbs->size()) ? start : 0;
	endIdx = (end > 0 && (unsigned int) end < thumbs->size()) ? end : (int)thumbs->size();



	//somethingTodo = true;
}

/**
 * This function is used for batch saving.
 * If this function is called, all thumbs are saved 
 * even if save is not checked in the preferences.
 **/ 
void DkThumbsLoader::loadAll() {

	if (!thumbs)
		return;

	// this function is used for batch saving
	loadAllThumbs = true;
	forceSave = true;
	setLoadLimits(0, (int)thumbs->size());
}

//QImage DkThumbsLoader::getThumbNailWin(QFileInfo file) {
//
//	CoInitialize(NULL);
//
//	DkTimer dt;
//
//	QImage thumb;
//
//	// allocate some unmanaged memory for our strings and divide the file name
//	// into a folder path and file name.
//	//String* fileName = file.absoluteFilePath();
//	//IntPtr dirPtr = Marshal::StringToHGlobalUni(Path::GetDirectoryName(fileName));
//	//IntPtr filePtr = Marshal::StringToHGlobalUni(Path::GetFileName(fileName));
//
//	QString winPath = QDir::toNativeSeparators(file.absolutePath());
//	QString winFile = QDir::toNativeSeparators(file.fileName());
//	winPath.append("\\");	
//
//	WCHAR* wDirName = new WCHAR[winPath.length()];
//	WCHAR* wFileName = new WCHAR[winFile.length()];
//
//	int dirLength = winPath.toWCharArray(wDirName);
//	int fileLength = winFile.toWCharArray(wFileName);
//
//	wDirName[dirLength] = L'\0';
//	wFileName[fileLength] = L'\0';
//
//	IShellFolder* pDesktop = NULL;
//	IShellFolder* pSub = NULL;
//	IExtractImage* pIExtract = NULL;
//	LPITEMIDLIST pList = NULL;
//
//	// get the desktop directory
//	if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
//	{   
//		// get the pidl for the directory
//		HRESULT hr = pDesktop->ParseDisplayName(NULL, NULL, wDirName, NULL, &pList, NULL);
//		if (FAILED(hr)) {
//			//throw new Exception(S"Failed to parse the directory name");
//
//			return thumb;
//		}
//
//		// get the directory IShellFolder interface
//		hr = pDesktop->BindToObject(pList, NULL, IID_IShellFolder, (void**)&pSub);
//		if (FAILED(hr))	{
//			//throw new Exception(S"Failed to bind to the directory");
//			return thumb;
//		}
//
//		// get the file's pidl
//		hr = pSub->ParseDisplayName(NULL, NULL, wFileName, NULL, &pList, NULL);
//		if (FAILED(hr))	{
//			//throw new Exception(S"Failed to parse the file name");
//			return thumb;
//		}
//
//		// get the IExtractImage interface
//		LPCITEMIDLIST pidl = pList;
//		hr = pSub->GetUIObjectOf(NULL, 1, &pidl, IID_IExtractImage,
//			NULL, (void**)&pIExtract);
//
//		// set our desired image size
//		SIZE size;
//		size.cx = maxThumbSize;
//		size.cy = maxThumbSize;      
//
//		if(pIExtract == NULL) {
//			return thumb;
//		}         
//
//		HBITMAP hBmp = NULL;
//
//		// The IEIFLAG_ORIGSIZE flag tells it to use the original aspect
//		// ratio for the image size. The IEIFLAG_QUALITY flag tells the 
//		// interface we want the image to be the best possible quality.
//		DWORD dwFlags = IEIFLAG_ORIGSIZE | IEIFLAG_QUALITY;      
//
//		OLECHAR pathBuffer[MAX_PATH];
//		hr = pIExtract->GetLocation(pathBuffer, MAX_PATH, NULL, &size, 4, &dwFlags);         // TODO: color depth!! (1)
//		if (FAILED(hr)) {
//			//throw new Exception(S"The call to GetLocation failed");
//			return thumb;
//		}
//
//		hr = pIExtract->Extract(&hBmp);
//
//		// It is possible for Extract to fail if there is no thumbnail image
//		// so we won't check for success here
//
//		pIExtract->Release();
//
//		if (hBmp != NULL) {
//			thumb = QPixmap::fromWinHBITMAP(hBmp, QPixmap::Alpha).toImage();
//		}      
//	}
//
//	// Release the COM objects we have a reference to.
//	pDesktop->Release();
//	pSub->Release(); 
//
//	// delete the unmanaged memory we allocated
//	//Marshal::FreeCoTaskMem(dirPtr);
//	//Marshal::FreeCoTaskMem(filePtr);
//	//delete[] wDirName;
//	//delete[] wFileName;
//
//
//	return thumb;
//}

/**
 * Stops the current loading process.
 * This method allows for stopping the thread without killing it.
 **/ 
void DkThumbsLoader::stop() {
	
	//QMutexLocker(&this->mutex);
	isActive = false;
	qDebug() << "stopping thread: " << this->thread()->currentThreadId();
}

QImage DkThumbsLoader::createThumb(const QImage& image) {

	int imgW = image.width();
	int imgH = image.height();

	if (imgW > maxThumbSize || imgH > maxThumbSize) {
		if (imgW > imgH) {
			imgH = (float)maxThumbSize / imgW * imgH;
			imgW = maxThumbSize;
		} 
		else if (imgW < imgH) {
			imgW = (float)maxThumbSize / imgH * imgW;
			imgH = maxThumbSize;
		}
		else {
			imgW = maxThumbSize;
			imgH = maxThumbSize;
		}
	}

	// fast downscaling
	QImage thumb = image.scaled(QSize(imgW*2, imgH*2), Qt::KeepAspectRatio, Qt::FastTransformation);
	thumb = thumb.scaled(QSize(imgW, imgH), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	return thumb;

}


// DkColorLoader --------------------------------------------------------------------
DkColorLoader::DkColorLoader(QDir dir, QStringList files) {

	this->dir = dir;
	this->files = files;

	moveToThread(this);

	init();
}

void DkColorLoader::init() {

	if (files.empty())
		files = DkImageLoader::getFilteredFileList(dir);

	isActive = true;
	maxThumbs = 800;
}

void DkColorLoader::run() {
	
	int updateIvl = 30;

	// max full HD
	for (int idx = 0; idx <= maxThumbs && idx < files.size(); idx++) {

		//mutex.lock();
		if (!isActive) {
			qDebug() << "color loader stopped...";
			//mutex.unlock();
			break;
		}

		loadColor(idx);

		if ((idx % updateIvl) == 0)
			emit updateSignal(cols, indexes);
	}

	emit updateSignal(cols, indexes);
	
}

void DkColorLoader::loadColor(int fileIdx) {

	if (files.size() > maxThumbs)
		fileIdx = qRound((float)fileIdx/maxThumbs*(files.size()-1));
	
	QFileInfo file(dir, files[fileIdx]);
	
	//// see if we can read the thumbnail from the exif data
	DkMetaData dataExif(file);
	QImage thumb = dataExif.getThumbnail();

	if (!thumb.isNull()) {
		
		cols.append(computeColor(thumb));	// TODO: compute most significant color
		indexes.append(fileIdx);
	}
}

QColor DkColorLoader::computeColor(QImage& thumb) {

	////int r = 0, g = 0, b = 0;

	//int nC = qRound(thumb.depth()/8.0f);
	//int rStep = qRound(thumb.height()/100.0f)+1;
	//int cStep = qRound(thumb.width()/100.0f)+1;

	//QVector<int> rHist; rHist.resize(100);
	//QVector<int> gHist; gHist.resize(100);
	//QVector<int> bHist; bHist.resize(100);

	//for (int idx = 0; idx < rHist.size(); idx++) {
	//	rHist[idx] = 0;
	//	gHist[idx] = 0;
	//	bHist[idx] = 0;
	//}

	//int offset = (nC > 1) ? 1 : 0;	// no offset for grayscale images

	//for (int rIdx = 0; rIdx < thumb.height(); rIdx += rStep) {

	//	const unsigned char* pixel = thumb.constScanLine(rIdx);

	//	for (int cIdx = 0; cIdx < thumb.width()*nC; cIdx += cStep*nC) {

	//		rHist[qRound(pixel[cIdx+2*offset]/255.0f*rHist.size())]++;
	//		gHist[qRound(pixel[cIdx+offset]/255.0f*gHist.size())]++;
	//		bHist[qRound(pixel[cIdx]/255.0f*bHist.size())]++;
	//	}
	//}

	//int rMaxVal = 0, gMaxVal = 0, bMaxVal = 0;
	//int rMaxIdx = 0, gMaxIdx = 0, bMaxIdx = 0;

	//for (int idx = 0; idx < rHist.size(); idx++) {

	//	if (rHist[idx] > rMaxVal) {
	//		rMaxVal = rHist[idx];
	//		rMaxIdx = idx;
	//	}
	//	if (gHist[idx] > gMaxVal) {
	//		gMaxVal = gHist[idx];
	//		gMaxIdx = idx;
	//	}
	//	if (bHist[idx] > bMaxVal) {
	//		bMaxVal = bHist[idx];
	//		bMaxIdx = idx;
	//	}
	//}

	//qDebug() << fileIdx;

	//cols.append(QColor((float)rMaxIdx/rHist.size()*255, (float)gMaxIdx/gHist.size()*255, (float)bMaxIdx/bHist.size()*255));	// TODO: compute most significant color

	//// compute mean color
	//int r = 0, g = 0, b = 0;

	//int nC = qRound(thumb.depth()/8.0f);
	//int rStep = qRound(thumb.height()/100.0f)+1;
	//int cStep = qRound(thumb.width()/100.0f)+1;
	//int n = 0;

	//int offset = (nC > 1) ? 1 : 0;	// no offset for grayscale images

	//for (int rIdx = 0; rIdx < thumb.height(); rIdx += rStep) {

	//	const unsigned char* pixel = thumb.constScanLine(rIdx);

	//	for (int cIdx = 0; cIdx < thumb.width()*nC; cIdx += cStep*nC) {

	//		r += pixel[cIdx+2*offset];
	//		g += pixel[cIdx+offset];
	//		b += pixel[cIdx];
	//		n++;
	//	}
	//}

	//return QColor((float)r/n, g/n, b/n);


	// compute most common color with a lookup table
	//int r = 0, g = 0, b = 0;

	// some speed-up params
	int nC = qRound(thumb.depth()/8.0f);
	int rStep = qRound(thumb.height()/100.0f)+1;
	int cStep = qRound(thumb.width()/100.0f)+1;
	int numCols = 42;

	int offset = (nC > 1) ? 1 : 0;	// no offset for grayscale images
	QMap<QRgb, int> colLookup;
	int maxColCount = 0;
	QRgb maxCol;

	for (int rIdx = 0; rIdx < thumb.height(); rIdx += rStep) {

		const unsigned char* pixel = thumb.constScanLine(rIdx);

		for (int cIdx = 0; cIdx < thumb.width()*nC; cIdx += cStep*nC) {

			QColor cColC(qRound(pixel[cIdx+2*offset]/255.0f*numCols), 
				qRound(pixel[cIdx+offset]/255.0f*numCols), 
				qRound(pixel[cIdx]/255.0f*numCols));
			QRgb cCol = cColC.rgb();

			//// skip black
			//if (cColC.saturation() < 10)
			//	continue;
			if (qRed(cCol) < 3 && qGreen(cCol) < 3 && qBlue(cCol) < 3)
				continue;
			if (qRed(cCol) > numCols-3 && qGreen(cCol) > numCols-3 && qBlue(cCol) > numCols-3)
				continue;


			if (colLookup.contains(cCol)) {
				colLookup[cCol]++;
			}
			else
				colLookup[cCol] = 1;

			if (colLookup[cCol] > maxColCount) {
				maxCol = cCol;
				maxColCount = colLookup[cCol];
			}
		}
	}

	if (maxColCount > 0)
		return QColor((float)qRed(maxCol)/numCols*255, (float)qGreen(maxCol)/numCols*255, (float)qBlue(maxCol)/numCols*255);
	else
		return DkSettings::display.bgColorWidget;
}

void DkColorLoader::stop() {

	isActive = false;
}

}
