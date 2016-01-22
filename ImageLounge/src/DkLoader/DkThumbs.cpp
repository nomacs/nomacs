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
#include "DkTimer.h"
#include "DkSettings.h"
#include "DkImageStorage.h"
#include "DkBasicLoader.h"
#include "DkMetaData.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QFileInfo>
#include <QStringList>
#include <QMutex>
#include <QImageReader>
#include <QtConcurrentRun>
#include <QTimer>
#include <QBuffer>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

/**
* Default constructor.
* @param file the corresponding file
* @param img the thumbnail image
**/ 
DkThumbNail::DkThumbNail(const QString& filePath, const QImage& img) {
	mImg = DkImage::createThumb(img);
	mFile = filePath;
	mMaxThumbSize = max_thumb_size;
	mMinThumbSize = Settings::param().display().thumbSize;
	mImgExists = true;
};

DkThumbNail::~DkThumbNail() {}

/**
 * Loads the thumbnail.
 * @param forceLoad flag for loading/saving the thumbnail from exif only.
 **/ 
void DkThumbNail::compute(int forceLoad) {
	
	// we do this that complicated to be thread-safe
	// if we use member vars in the thread and the object gets deleted during thread execution we crash...
	mImg = computeIntern(mFile, QSharedPointer<QByteArray>(), forceLoad, mMaxThumbSize, mMinThumbSize);
}

/**
 * Loads the thumbnail from the metadata.
 * If no thumbnail is embedded, the whole image
 * is loaded and downsampled in a fast manner.
 * @param file the file to be loaded
 * @param ba the file buffer (can be empty)
 * @param forceLoad the loading flag (e.g. exiv only)
 * @param maxThumbSize the maximal thumbnail size to be loaded
 * @param minThumbSize the minimal thumbnail size to be loaded
 * @return QImage the loaded image. Null if no image
 * could be loaded at all.
 **/ 
QImage DkThumbNail::computeIntern(const QString& filePath, const QSharedPointer<QByteArray> ba, 
								  int forceLoad, int maxThumbSize, int minThumbSize) {
	
	DkTimer dt;
	//qDebug() << "[thumb] file: " << file.absoluteFilePath();

	// see if we can read the thumbnail from the exif data
	QImage thumb;
	DkMetaDataT metaData;

	QSharedPointer<QByteArray> baZip = QSharedPointer<QByteArray>();
#ifdef WITH_QUAZIP
	if (QFileInfo(mFile).dir().path().contains(DkZipContainer::zipMarker())) 
		baZip = DkZipContainer::extractImage(DkZipContainer::decodeZipFile(filePath), DkZipContainer::decodeImageFile(filePath));
#endif
	try {
		if (baZip && !baZip->isEmpty())	
			metaData.readMetaData(filePath, baZip);
		else if (!ba || ba->isEmpty())
			metaData.readMetaData(filePath);
		else
			metaData.readMetaData(filePath, ba);

		// read the full image if we want to create new thumbnails
		if (forceLoad != force_save_thumb)
			thumb = metaData.getThumbnail();
	}
	catch(...) {
		// do nothing - we'll load the full file
	}
	removeBlackBorder(thumb);

	if (thumb.isNull() && forceLoad == force_exif_thumb)
		return QImage();

	bool exifThumb = !thumb.isNull();

	int orientation = metaData.getOrientation();
	int imgW = thumb.width();
	int imgH = thumb.height();
	int tS = minThumbSize;

	// as found at: http://olliwang.com/2010/01/30/creating-thumbnail-images-in-qt/
	QFileInfo fInfo(filePath);
	QString lFilePath = fInfo.isSymLink() ? fInfo.symLinkTarget() : filePath;
	fInfo = lFilePath;

	QImageReader* imageReader = 0;
	
	if (!ba || ba->isEmpty())
		imageReader = new QImageReader(lFilePath);
	else {
		QBuffer buffer;
		buffer.setData(ba->data());
		buffer.open(QIODevice::ReadOnly);
		imageReader = new QImageReader(&buffer, fInfo.suffix().toStdString().c_str());
		buffer.close();
	}

	if (thumb.isNull() || (thumb.width() < tS && thumb.height() < tS)) {

		imgW = imageReader->size().width();		// crash detected: unhandled exception at 0x66850E9A (msvcr110d.dll) in nomacs.exe: 0xC0000005: Access violation reading location 0x0000C788.
		imgH = imageReader->size().height();	// locks the file!
	}
	
	if (forceLoad != DkThumbNailT::force_exif_thumb && (imgW > maxThumbSize || imgH > maxThumbSize)) {
		if (imgW > imgH) {
			imgH = qRound((float)maxThumbSize / imgW * imgH);
			imgW = maxThumbSize;
		} 
		else if (imgW < imgH) {
			imgW = qRound((float)maxThumbSize / imgH * imgW);
			imgH = maxThumbSize;
		}
		else {
			imgW = maxThumbSize;
			imgH = maxThumbSize;
		}
	}

	bool rescale = forceLoad == force_save_thumb;

	if (forceLoad != force_exif_thumb && 
			(thumb.isNull() || 
			thumb.width() < tS && thumb.height() < tS || 
			forceLoad == force_full_thumb || 
			forceLoad == force_save_thumb)) { // braces
		
		// flip size if the image is rotated by 90°
		if (metaData.isTiff() && abs(orientation) == 90) {
			int tmpW = imgW;
			imgW = imgH;
			imgH = tmpW;
			qDebug() << "EXIF size is flipped...";
		}

		QSize initialSize = imageReader->size();

		imageReader->setScaledSize(QSize(imgW, imgH));
		thumb = imageReader->read();

		// try to read the image
		if (thumb.isNull()) {
			DkBasicLoader loader;
			
			if (baZip && !baZip->isEmpty())	{
				if (loader.loadGeneral(lFilePath, baZip, true, true))
				thumb = loader.image();
			}
			else {
				if (loader.loadGeneral(lFilePath, ba, true, true))
					thumb = loader.image();
			}
		}

		// the image is not scaled correctly yet
		if (rescale && !thumb.isNull() && (imgW == -1 || imgH == -1)) {
			imgW = thumb.width();
			imgH = thumb.height();

			if (imgW > maxThumbSize || imgH > maxThumbSize) {
				if (imgW > imgH) {
					imgH = qRound((float)maxThumbSize / imgW * imgH);
					imgW = maxThumbSize;
				} 
				else if (imgW < imgH) {
					imgW = qRound((float)maxThumbSize / imgH * imgW);
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
		imageReader->setFileName("josef");	// image reader locks the file -> but there should not be one so we just set it to another file...
	}
	else if (rescale) {
		thumb = thumb.scaled(QSize(imgW, imgH), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}

	if (imageReader)
		delete imageReader;

	if (orientation != -1 && orientation != 0 && (metaData.isJpg() || metaData.isRaw())) {
		QTransform rotationMatrix;
		rotationMatrix.rotate((double)orientation);
		thumb = thumb.transformed(rotationMatrix);
	}

	// save the thumbnail if the caller either forces it, or the save thumb is requested and the image did not have any before
	if (rescale || (forceLoad == save_thumb && !exifThumb)) {
		
		try {

			QImage sThumb = thumb.copy();
			if (orientation != -1 && orientation != 0) {
				QTransform rotationMatrix;
				rotationMatrix.rotate(-(double)orientation);
				sThumb = sThumb.transformed(rotationMatrix);
			}

			metaData.setThumbnail(sThumb);

			if (!ba || ba->isEmpty())
				metaData.saveMetaData(lFilePath);
			else
				metaData.saveMetaData(lFilePath, ba);

			qDebug() << "[thumb] saved to exif data";
		}
		catch(...) {
			qDebug() << "Sorry, I could not save the metadata";
		}
	}


	if (!thumb.isNull())
		qDebug() << "[thumb] " << fInfo.fileName() << "(" << thumb.width() << " x " << thumb.height() << ") loaded in: " << dt.getTotal() << ((exifThumb) ? " from EXIV" : " from File");

	return thumb;
}

/**
 * Removes potential black borders.
 * These borders can be found e.g. in Nikon One images (16:9 vs 4:3)
 * @param img the image whose borders are removed.
 **/ 
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

/**
 * Creates a thumbnail from the image provided and stores it internally.
 * @param img the image to be converted to a thumbnail
 **/ 
void DkThumbNail::setImage(const QImage img) {
	
	mImg = DkImage::createThumb(img);
}

/**
 * This class provides threaded access to image thumbnails.
 * @param file the thumbnail's file
 * @param img optional: a thumb image.
 **/ 
DkThumbNailT::DkThumbNailT(const QString& filePath, const QImage& img) : DkThumbNail(filePath, img) {

	mFetching = false;
	mForceLoad = do_not_force;
}

DkThumbNailT::~DkThumbNailT() {

	//if (thumbWatcher.isStarted() && !thumbWatcher.isRunning()) {
	//	qDebug() << "[WARNING]: thumb watcher is started but not running while releasing!";
	//}

	if (mFetching && Settings::param().resources().numThumbsLoading > 0)
		Settings::param().resources().numThumbsLoading--;

	thumbWatcher.blockSignals(true);
	thumbWatcher.cancel();
}

bool DkThumbNailT::fetchThumb(int forceLoad /* = false */,  QSharedPointer<QByteArray> ba) {

	if (forceLoad == force_full_thumb || forceLoad == force_save_thumb || forceLoad == save_thumb)
		mImg = QImage();

	if (!mImg.isNull() || !mImgExists || mFetching)
		return false;

	// we have to do our own bool here
	// watcher.isRunning() returns false if the thread is waiting in the pool
	mFetching = true;
	mForceLoad = forceLoad;

	connect(&thumbWatcher, SIGNAL(finished()), this, SLOT(thumbLoaded()));
	thumbWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkThumbNailT::computeCall, mFile, ba, forceLoad, mMaxThumbSize, mMinThumbSize));

	Settings::param().resources().numThumbsLoading++;

	return true;
}


QImage DkThumbNailT::computeCall(const QString& filePath, QSharedPointer<QByteArray> ba, int forceLoad, int maxThumbSize, int minThumbSize) {

	return DkThumbNail::computeIntern(filePath, ba, forceLoad, maxThumbSize, minThumbSize);
}

void DkThumbNailT::thumbLoaded() {
	
	QFuture<QImage> future = thumbWatcher.future();

	mImg = future.result();
	
	if (mImg.isNull() && mForceLoad != force_exif_thumb)
		mImgExists = false;

	mFetching = false;
	Settings::param().resources().numThumbsLoading--;
	emit thumbLoadedSignal(!mImg.isNull());
}

/**
 * Default constructor of the thumbnail loader.
 * Note: currently the init calls the getFilteredFileList which might be slow.
 * @param thumbs a pointer to an array holding the thumbnails. while
 * loading, the thumbsloader will add all images to this array. however, the
 * caller must destroy the thumbs vector.
 * @param dir the directory where thumbnails should be loaded from.
 **/ 
DkThumbsLoader::DkThumbsLoader(std::vector<DkThumbNail>* thumbs, QDir dir, QFileInfoList files) {

	mThumbs = thumbs;
	mDir = dir;
	mIsActive = true;
	mFiles = files;
	init();
}

/**
 * Initializes the thumbs loader.
 * Note: getFilteredFileList might be slow.
 **/ 
void DkThumbsLoader::init() {

	// TODO: update!
	//if (files.empty())
	//	files = DkImageLoader::getFilteredFileInfoList(dir);
	mStartIdx = -1;
	mEndIdx = -1;
	mSomethingTodo = false;
	mNumFilesLoaded = 0;
	mLoadAllThumbs = false;
	mForceSave = false;
	mForceLoad = false;

	// here comes hot stuff (for a better update policy)
	std::vector<DkThumbNail> oldThumbs = *mThumbs;
	mThumbs->clear();

	DkTimer dt;
	for (int idx = 0; idx < mFiles.size(); idx++) {
		QFileInfo cFile = mFiles[idx];

		DkThumbNail cThumb = DkThumbNail(cFile.absoluteFilePath());

		for (unsigned int i = 0; i < oldThumbs.size(); i++) {

			if (cThumb == oldThumbs[i]) {
				cThumb = oldThumbs[i];
				break;
			}
		}

		mThumbs->push_back(cThumb);
	}

	qDebug() << "thumb stubs loaded in: " << dt.getTotal();
}

/**
 * Returns the file idx of the file specified.
 * @param file the file to be queried.
 * @return int the index of the file.
 **/ 
int DkThumbsLoader::getFileIdx(const QString& filePath) const {

	if (!QFileInfo(filePath).exists() || !mThumbs)
		return -1;

	int fileIdx = 0;
	for ( ; (size_t)fileIdx < mThumbs->size(); fileIdx++) {

		if (mThumbs->at(fileIdx).getFilePath() == filePath)
			break;
	}

	if ((size_t)fileIdx == mThumbs->size()) 
		fileIdx = -1;

	return fileIdx;

}

/**
 * Thread routine.
 * Only loads thumbs if somethingTodo is true.
 **/ 
void DkThumbsLoader::run() {

	if (!mThumbs)
		return;

	for (;;) {

		if (mLoadAllThumbs && mNumFilesLoaded >= (int)mThumbs->size()) {
			qDebug() << "[thumbs] thinks he has finished...";
			break;
		}

		mMutex.lock();
		DkTimer dt;
		msleep(100);

		//QMutexLocker(&this->mutex);
		if (!mIsActive) {
			qDebug() << "thumbs loader stopped...";
			mMutex.unlock();
			break;
		}
		mMutex.unlock();

		if (mSomethingTodo)
			loadThumbs();
	}

}

/**
 * Loads thumbnails from the metadata.
 **/ 
void DkThumbsLoader::loadThumbs() {

	std::vector<DkThumbNail>::iterator thumbIter = mThumbs->begin()+mStartIdx;
	qDebug() << "start: " << mStartIdx << " end: " << mEndIdx;

	for (int idx = mStartIdx; idx < mEndIdx; idx++, thumbIter++) {

		mMutex.lock();

		// jump to new start idx
		if (mStartIdx > idx) {
			thumbIter = mThumbs->begin()+mStartIdx;
			idx = mStartIdx;
		}

		// does somebody want me to stop?
		if (!mIsActive) {
			mMutex.unlock();
			return;
		}
		
		// TODO:  he breaks here! (crash detected++)
		// at the same time, main thread in DkFilePreview indexDir() -> waiting for our loader after stopping it
		DkThumbNail* thumb = &(*thumbIter);
		if (!thumb->hasImage()) {
			thumb->compute(mForceLoad);
			if (thumb->hasImage())	// could I load the thumb?
				emit updateSignal();
			else {
				thumb->setImgExists(false);
				qDebug() << "image does NOT exist...";
			}
			
		}
		emit numFilesSignal(++mNumFilesLoaded);
		mMutex.unlock();
	}

	mSomethingTodo = false;
}

/**
 * Here you can specify which thumbnails to load.
 * Note: it is not a good idea to load all thumbnails
 * of a folder (might be a lot : )
 * @param start the start index
 * @param end the end index
 **/ 
void DkThumbsLoader::setLoadLimits(int start, int end) {

	mStartIdx = (start >= 0 && (unsigned int) start < mThumbs->size()) ? start : 0;
	mEndIdx = (end > 0 && (unsigned int) end < mThumbs->size()) ? end : (int)mThumbs->size();
}

/**
 * This function is used for batch saving.
 * If this function is called, all thumbs are saved 
 * even if save is not checked in the preferences.
 **/ 
void DkThumbsLoader::loadAll() {

	if (!mThumbs)
		return;

	// this function is used for batch saving
	mLoadAllThumbs = true;
	mForceSave = true;
	mSomethingTodo = true;
	setLoadLimits(0, (int)mThumbs->size());
}

/**
 * Stops the current loading process.
 * This method allows for stopping the thread without killing it.
 **/ 
void DkThumbsLoader::stop() {
	
	mIsActive = false;
	qDebug() << "stopping thread: " << this->thread()->currentThreadId();
}

}
