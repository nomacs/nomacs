/*******************************************************************************************************
 DkBasicLoader.cpp
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

#include "DkBasicLoader.h"

#include "DkMetaData.h"
#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkMath.h"
#include "DkUtils.h"	// just needed for qInfo() #ifdef

#pragma warning(push, 0)        
#include <QObject>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QNetworkReply>
#include <QBuffer>
#include <QNetworkProxyFactory>
#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include <QtConcurrentRun>

#include <qmath.h>
#include <assert.h>

// quazip
#ifdef WITH_QUAZIP
#ifdef WITH_QUAZIP1
#include <quazip/JlCompress.h>
#else
#include <quazip5/JlCompress.h>
#endif
#endif

// opencv
#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#ifdef WITH_LIBRAW
#include <libraw/libraw.h>
#endif

#ifdef WITH_LIBTIFF
#ifdef Q_OS_WIN
#include <tif_config.h>
#endif

//#if defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)
// here we clash (typedef redefinition with different types ('long' vs 'int64_t' (aka 'long long'))) 
// so we simply define our own int64 before including tiffio
#define uint64 uint64_hack_
#define int64 int64_hack_
//#endif // defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)

#include <tiffio.h>
#include <tiffio.hxx>		// this is needed if you want to load tiffs from the buffer

//#if defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)
#undef uint64
#undef int64
//#endif // defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)

#endif //#ifdef WITH_LIBTIFF

#endif //#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#include <olectl.h>
#pragma comment(lib, "oleaut32.lib")

#include <QtWin>
#endif //#ifdef Q_OS_WIN


#pragma warning(pop)

namespace nmc {

// DkEditImage --------------------------------------------------------------------
DkEditImage::DkEditImage(const QImage& img, const QString& editName) {
	mImg = img;
	mEditName = editName;
}

void DkEditImage::setImage(const QImage& img) {
	mImg = img;
}

QImage DkEditImage::image() const {
	return mImg;
}

QString DkEditImage::editName() const {
	return mEditName;
}

int DkEditImage::size() const {
	
	return qRound(DkImage::getBufferSizeFloat(mImg.size(), mImg.depth()));
}

// Basic loader and image edit class --------------------------------------------------------------------
DkBasicLoader::DkBasicLoader(int mode) {
	
	mMode = mode;
	mTraining = false;
	mPageIdxDirty = false;
	mNumPages = 1;
	mPageIdx = 1;
	mLoader = no_loader;

	mMetaData = QSharedPointer<DkMetaDataT>(new DkMetaDataT());
}

bool DkBasicLoader::loadGeneral(const QString& filePath, bool loadMetaData, bool fast) {

	return loadGeneral(filePath, QSharedPointer<QByteArray>(), loadMetaData, fast);
}
/**
 * This function loads the images.
 * @param file the image file that should be loaded.
 * @return bool true if the image could be loaded.
 **/ 
bool DkBasicLoader::loadGeneral(const QString& filePath, QSharedPointer<QByteArray> ba, bool loadMetaData, bool fast) {

	DkTimer dt;
	bool imgLoaded = false;
	
	mFile = DkUtils::resolveSymLink(filePath);
	QFileInfo fInfo(mFile);	// resolved lnk
	QString newSuffix = fInfo.suffix();

	release();

	if (mPageIdxDirty)
		imgLoaded = loadPage();

	// identify raw images:
	//newSuffix.contains(QRegExp("(nef|crw|cr2|arw|rw2|mrw|dng)", Qt::CaseInsensitive)))

	// this fixes an issue with the new jpg loader
	// Qt considers an orientation of 0 as wrong and fails to load these jpgs
	// however, the old nomacs wrote 0 if the orientation should be cleared
	// so we simply adopt the memory here
	if (loadMetaData && mMetaData) {

		try {
			mMetaData->readMetaData(filePath, ba);
			
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
			// this is a workaroung for old Qt5 versions where jpgs with 'illegal' orientation=0 were not loaded
			if (!DkSettingsManager::param().metaData().ignoreExifOrientation) {
				DkMetaDataT::ExifOrientationState orState = mMetaData->checkExifOrientation();
				
				if (orState == DkMetaDataT::or_illegal) {
					mMetaData->clearOrientation();
					mMetaData->saveMetaData(ba);
					qWarning() << "deleting illegal EXIV orientation...";
				}
			}
#endif
		}
		catch (...) {}	// ignore if we cannot read the metadata
	}
	else if (!mMetaData) {
		qDebug() << "metaData is NULL!";
	}

	QList<QByteArray> qtFormats = QImageReader::supportedImageFormats();
	qtFormats << "jpe";	// fixes #435 - thumbnail gets loaded in the RAW loader
	QString suf = fInfo.suffix().toLower();

	QImage img;

    // load drif file
    if (!imgLoaded && ("drif" == suf || "yuv" == suf || "raw" == suf)) 
        imgLoaded = loadDrifFile(mFile, img, ba);

	if (!imgLoaded && !fInfo.exists() && ba && !ba->isEmpty()) {
		imgLoaded = img.loadFromData(*ba.data());

		if (imgLoaded)
			mLoader = qt_loader;
	}

	// load large icons
	if (!imgLoaded && suf == "ico") {

		QIcon icon(mFile);

		if (!icon.isNull()) {
			img = icon.pixmap(QSize(256, 256)).toImage();
			imgLoaded = true;
		}
	}

	// default Qt loader
	// here we just try those formats that are officially supported
	if (!imgLoaded && qtFormats.contains(suf.toStdString().c_str()) || suf.isEmpty()) {

		// if image has Indexed8 + alpha channel -> we crash... sorry for that
		if (!ba || ba->isEmpty())
			imgLoaded = img.load(mFile, suf.toStdString().c_str());
		else
			imgLoaded = img.loadFromData(*ba.data(), suf.toStdString().c_str());	// toStdString() in order get 1 byte per char

		if (imgLoaded) mLoader = qt_loader;
	}

	// OpenCV Tiff loader - supports jpg compressed tiffs
	if (!imgLoaded && newSuffix.contains(QRegExp("(tif|tiff)", Qt::CaseInsensitive))) {

		imgLoaded = loadTIFFile(mFile, img, ba);
		
		if (imgLoaded)	mLoader = tif_loader;
	}

	// PSD loader
	if (!imgLoaded) {

		imgLoaded = loadPSDFile(mFile, img, ba);
		if (imgLoaded) mLoader = psd_loader;
	}

#if QT_VERSION < 0x050000	// >DIR: qt5 ships with webp : ) [23.4.2015 markus]
	// WEBP loader
	if (!imgLoaded) {

		imgLoaded = loadWebPFile(file, ba);
		if (imgLoaded) loader = webp_loader;
	}
#endif

	// RAW loader
	if (!imgLoaded && !qtFormats.contains(suf.toStdString().c_str())) {
		
		// TODO: sometimes (e.g. _DSC6289.tif) strange opencv errors are thrown - catch them!
		// load raw files
		imgLoaded = loadRawFile(mFile, img, ba, fast);
		if (imgLoaded) mLoader = raw_loader;
	}

	// TGA loader
	if (!imgLoaded && newSuffix.contains(QRegExp("(tga)", Qt::CaseInsensitive))) {

		imgLoaded = loadTgaFile(mFile, img, ba);

		if (imgLoaded) mLoader = tga_loader;		// TODO: add tga loader
	}

	QByteArray lba;

	// default Qt loader
	if (!imgLoaded && !newSuffix.contains(QRegExp("(roh)", Qt::CaseInsensitive))) {

		// if we first load files to buffers, we can additionally load images with wrong extensions (rainer bugfix : )
		// TODO: add warning here
		loadFileToBuffer(mFile, lba);
		imgLoaded = img.loadFromData(lba);
		
		if (imgLoaded)
			qWarning() << "The image seems to have a wrong extension";
		
		if (imgLoaded) mLoader = qt_loader;
	} 

	// add marker to fix broken panorama images from SAMSUNG
	// see: https://github.com/nomacs/nomacs/issues/254
	if (!imgLoaded && newSuffix.contains(QRegExp("(jpg|jpeg|jpe)", Qt::CaseInsensitive))) {

		// prefer external buffer
		QByteArray baf = DkImage::fixSamsungPanorama(ba && !ba->isEmpty() ? *ba : lba);

		if (!baf.isEmpty())
			imgLoaded = img.loadFromData(baf, suf.toStdString().c_str());

		if (imgLoaded) mLoader = qt_loader;
	}

	// this loader is a bit buggy -> be carefull
	if (!imgLoaded && newSuffix.contains(QRegExp("(roh)", Qt::CaseInsensitive))) {
		
		imgLoaded = loadRohFile(mFile, img, ba);
		if (imgLoaded) mLoader = roh_loader;
	} 

	// this loader is for OpenCV cascade training files
	if (!imgLoaded && newSuffix.contains(QRegExp("(vec)", Qt::CaseInsensitive))) {

		imgLoaded = loadOpenCVVecFile(mFile, img, ba);
		if (imgLoaded) mLoader = roh_loader;
	} 

	// tiff things
	if (imgLoaded && !mPageIdxDirty)
		indexPages(mFile, ba);
	mPageIdxDirty = false;

	if (imgLoaded && loadMetaData && mMetaData) {
		
		try {
			mMetaData->setQtValues(img);
			int orientation = mMetaData->getOrientationDegree();

			if (orientation != -1 && !mMetaData->isTiff() && !DkSettingsManager::param().metaData().ignoreExifOrientation)
				img = DkImage::rotate(img, orientation);

		} catch(...) {}	// ignore if we cannot read the metadata
	}
	else if (!mMetaData) {
		qDebug() << "metaData is NULL!";
	}

	if (imgLoaded)
		setEditImage(img, tr("Original Image"));

	if (imgLoaded)
		qInfo() << "[Basic Loader]" << filePath << "loaded in" << dt;
	else
		qWarning() << "[Basic Loader] could not load" << filePath;

	return imgLoaded;
}

/**
 * Loads special RAW files that are generated by the Hamamatsu camera.
 * @param fileName the filename of the file to be loaded.
 * @return bool true if the file could be loaded.
 **/ 
bool DkBasicLoader::loadRohFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba) const {

	if (!ba)
		ba = loadFileToBuffer(filePath);
	if (!ba || ba->isEmpty())
		return false;

	bool imgLoaded = false;

	int rohW = 4000;
	int rohH = 2672;
	unsigned char fByte;	// first byte
	unsigned char sByte;	// second byte

	try {
		
		const unsigned char* pData = (const unsigned char*)ba->constData();
		unsigned char* buffer = new unsigned char[rohW*rohH];

		if (!buffer)
			return imgLoaded;

		for (long long i = 0; i < (rohW*rohH); i++){
		
			fByte = pData[i*2];
			sByte = pData[i*2+1];
			fByte = fByte >> 4;
			fByte = fByte & 15;
			sByte = sByte << 4;
			sByte = sByte & 240;

			buffer[i] = (fByte | sByte);
		
		}

		img = QImage(buffer, rohW, rohH, QImage::Format_Indexed8);

		if (img.isNull())
			return imgLoaded;
		else
			imgLoaded = true;


		//img = img.copy();
		QVector<QRgb> colorTable;

		for (int i = 0; i < 256; i++)
			colorTable.push_back(QColor(i, i, i).rgb());
		
		img.setColorTable(colorTable);

	} catch(...) {
		imgLoaded = false;
	}

	//if (imgLoaded) {
	//	setEditImage(img, tr("Original Image"));
	//}

	return imgLoaded;
}

bool nmc::DkBasicLoader::loadTgaFile(const QString & filePath, QImage & img, QSharedPointer<QByteArray> ba) const {
	
	if (!ba || ba->isEmpty())
		ba = loadFileToBuffer(filePath);

	tga::DkTgaLoader tl = tga::DkTgaLoader(ba);

	bool success = tl.load();
	img = tl.image();

	return success;
}

/**
 * Loads the RAW file specified.
 * Note: nomacs needs to be compiled with OpenCV and LibRaw in
 * order to enable RAW file loading.
 * @param ba the file loaded into a bytearray.
 * @return bool true if the file could be loaded.
 **/ 
bool DkBasicLoader::loadRawFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba, bool fast) const {
	
	DkRawLoader rawLoader(filePath, mMetaData);
	rawLoader.setLoadFast(fast);

	bool success = rawLoader.load(ba);

	if (success)
		img = rawLoader.image();

	return success;
}

#ifdef Q_OS_WIN
bool DkBasicLoader::loadPSDFile(const QString&, QImage&, QSharedPointer<QByteArray>) const {
#else
bool DkBasicLoader::loadPSDFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba) const {

	// load from file?
	if (!ba || ba->isEmpty()) {
		QFile file(filePath);
		file.open(QIODevice::ReadOnly);

		QPsdHandler psdHandler;
		psdHandler.setDevice(&file);	// QFile is an IODevice
		//psdHandler.setFormat(fileInfo.suffix().toLocal8Bit());

		if (psdHandler.canRead(&file)) {
			bool success = psdHandler.read(&img);
			//setEditImage(img, tr("Original Image"));
			
			return success;
		}
	}
	else {
	
		QBuffer buffer;
		buffer.setData(*ba.data());
		buffer.open(QIODevice::ReadOnly);

		QPsdHandler psdHandler;
		psdHandler.setDevice(&buffer);	// QFile is an IODevice
		//psdHandler.setFormat(file.suffix().toLocal8Bit());

		if (psdHandler.canRead(&buffer)) {
			bool success = psdHandler.read(&img);
			//setEditImage(img, tr("Original Image"));

			return success;
		}
	}

#endif // !Q_OS_WIN
	return false;
}

#ifndef WITH_LIBTIFF
bool DkBasicLoader::loadTIFFile(const QString&, QImage&, QSharedPointer<QByteArray>) const {
#else
bool DkBasicLoader::loadTIFFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba) const {

	bool success = false;

	// first turn off nasty warning/error dialogs - (we do the GUI : )
	TIFFErrorHandler oldErrorHandler, oldWarningHandler;
	oldWarningHandler = TIFFSetWarningHandler(NULL);
	oldErrorHandler = TIFFSetErrorHandler(NULL);

	DkTimer dt;
	TIFF* tiff = 0;


// TODO: currently TIFFStreamOpen can only be linked on Windows?!
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0) && defined(Q_OS_WIN)

	std::istringstream is(ba ? ba->toStdString() : "");

	if (ba) 
		tiff = TIFFStreamOpen("MemTIFF", &is);

	// fallback to direct loading
	if (!tiff)
		tiff = TIFFOpen(filePath.toLatin1(), "r");

	// loading from buffer allows us to load files with non-latin names
	QSharedPointer<QByteArray> bal;
	if (!tiff)
		bal = loadFileToBuffer(filePath);

	std::istringstream isl(bal ? bal->toStdString() : "");

	if (bal)
		tiff = TIFFStreamOpen("MemTIFF", &isl);
#else
	tiff = TIFFOpen(filePath.toLatin1(), "r");
#endif

	if (!tiff)
		return success;

	uint32 width = 0;
	uint32 height = 0;

	TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

	// init the qImage
	img = QImage(width, height, QImage::Format_ARGB32);

	const int stopOnError = 1;
	success = TIFFReadRGBAImageOriented(tiff, width, height, reinterpret_cast<uint32 *>(img.bits()), ORIENTATION_TOPLEFT, stopOnError) != 0;

	if (success) {
		for (uint32 y = 0; y<height; ++y)
			convert32BitOrder(img.scanLine(y), width);
	}

	TIFFClose(tiff);

	TIFFSetWarningHandler(oldWarningHandler);
	TIFFSetWarningHandler(oldErrorHandler);

	return success;

#endif // !WITH_LIBTIFF
	return false;
}

#define DRIF_IMAGE_IMPL
#include "drif_image.h"

bool isQtFmtCompatible(uint32_t f)
{
    switch (f)
    {
    case DRIF_FMT_RGB888:
    case DRIF_FMT_RGBA8888:
    case DRIF_FMT_GRAY:
        return true;
    }

    return false;
}

uint32_t drif2qtfmt(uint32_t f)
{
    switch (f)
    {
    case DRIF_FMT_RGB888:  return QImage::Format_RGB888;
    case DRIF_FMT_RGBA8888:  return QImage::Format_RGBA8888;

	// grayscale 8 was added in Qt 5.4
#if QT_VERSION >= 0x050500
	case DRIF_FMT_GRAY:  return QImage::Format_Grayscale8;
#endif

    }

    return QImage::Format_Invalid;
}

bool DkBasicLoader::loadDrifFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba) const {

    bool success = false;

    uint32_t w;
    uint32_t h;
    uint32_t f;

    uint8_t* imgBytes = drifLoadImg(filePath.toLatin1(), &w, &h, &f);

    if (!imgBytes)
        return success;
    
    if (isQtFmtCompatible(f))
    {
        img = QImage((int)w, (int)h, (QImage::Format)drif2qtfmt(f));
        memcpy(reinterpret_cast<void*>(img.bits()), imgBytes, drifGetSize(w, h, f));

        success = true;
    }
#ifdef WITH_OPENCV
    else
    {
        img = QImage((int)w, (int)h, QImage::Format_RGB888);

        switch (f)
        {
        case DRIF_FMT_BGR888:
        {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC3, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_BGR2RGB);

            success = true;
        }
        break;

        case DRIF_FMT_RGB888P:
        case DRIF_FMT_RGBA8888P:
        {
            cv::Mat imgMatR = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes);
            cv::Mat imgMatG = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + w * h);
            cv::Mat imgMatB = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + 2 * w * h);
            cv::Mat rgbMat  = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            
            std::vector<cv::Mat> imgMat{ imgMatR, imgMatG, imgMatB };
            cv::merge(imgMat, rgbMat);

            success = true;
        }
        break;

        case DRIF_FMT_BGR888P:
        case DRIF_FMT_BGRA8888P:
        {
            cv::Mat imgMatB = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes);
            cv::Mat imgMatG = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + w * h);
            cv::Mat imgMatR = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + 2 * w * h);
            cv::Mat rgbMat  = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));

            std::vector<cv::Mat> imgMat{ imgMatR, imgMatG, imgMatB };
            cv::merge(imgMat, rgbMat);

            success = true;
        }
        break;

        case DRIF_FMT_BGRA8888:
        {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC4, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_BGR2RGB, 3);

            success = true;
        }
        break;

        case DRIF_FMT_RGBA8888:
        {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC4, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_RGBA2RGB, 3);

            success = true;
        }
        break;

        case DRIF_FMT_GRAY:
        {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_GRAY2RGB);

            success = true;
        }
        break;

        case DRIF_FMT_YUV420P:
        {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_I420);

            success = true;
        }
        break;

        case DRIF_FMT_YVU420P:
        {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_YV12);

            success = true;
        }
        break;

        case DRIF_FMT_NV12:
        {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_NV12);

            success = true;
        }
        break;

        case DRIF_FMT_NV21:
        {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t*>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_NV21);

            success = true;
        }
        break;

        default:
            success = false;
            break;
        }
       
    }
#endif
   
    drifFreeImg(imgBytes);

    return success;
}

void DkBasicLoader::setImage(const QImage & img, const QString & editName, const QString & file) {

	mFile = file;
	setEditImage(img, editName);
}

void DkBasicLoader::setEditImage(const QImage& img, const QString& editName) {

	if (img.isNull())
		return;

	// delete all hidden edit states
	for (int idx = mImages.size() - 1; idx > mImageIndex; idx--) {
		mImages.pop_back();
	}

	// compute new history size
	int historySize = 0;
	for (const DkEditImage& e : mImages) {
		historySize += e.size();
	}

	DkEditImage newImg(img, editName);

	if (historySize + newImg.size() > DkSettingsManager::param().resources().historyMemory && mImages.size() > mMinHistorySize) {
		mImages.removeAt(1);
		qDebug() << "removing history image because it's too large:" << historySize + newImg.size() << "MB";
	}

	mImages.append(newImg);
	mImageIndex = mImages.size() - 1;	// set the index again to the last
}

QImage DkBasicLoader::image() const {
	
	if (mImages.empty())
		return QImage();

	if (mImageIndex > mImages.size() || mImageIndex == -1) {
		qWarning() << "Illegal image index: " << mImageIndex;
		return mImages.last().image();
	}

	return mImages[mImageIndex].image();
}

void DkBasicLoader::undo() {
	
	if (mImageIndex > 0)
		mImageIndex--;
}

void DkBasicLoader::redo() {

	if (mImageIndex < mImages.size()-1)
		mImageIndex++;
}

QVector<DkEditImage>* DkBasicLoader::history() {
	return &mImages;
}

DkEditImage DkBasicLoader::lastEdit() const {
	
	assert(mImageIndex >= 0 && mImageIndex < mImages.size());
	return mImages[mImageIndex];
}

int DkBasicLoader::historyIndex() const {
	return mImageIndex;
}

void DkBasicLoader::setMinHistorySize(int size) {
	mMinHistorySize = size;
}

void DkBasicLoader::setHistoryIndex(int idx) {
	mImageIndex = idx;
}

void DkBasicLoader::loadFileToBuffer(const QString& filePath, QByteArray& ba) const {

	QFileInfo fi(filePath);

	if (!fi.exists())
		return;

#ifdef WITH_QUAZIP
	if (fi.dir().path().contains(DkZipContainer::zipMarker())) 
		DkZipContainer::extractImage(DkZipContainer::decodeZipFile(filePath), DkZipContainer::decodeImageFile(filePath), ba);
#endif
	
	QFile file(filePath);
	file.open(QIODevice::ReadOnly);

	ba = file.readAll();
}

QSharedPointer<QByteArray> DkBasicLoader::loadFileToBuffer(const QString& filePath) const {

	QFileInfo fi(filePath);

#ifdef WITH_QUAZIP
	if (fi.dir().path().contains(DkZipContainer::zipMarker())) 
		return DkZipContainer::extractImage(DkZipContainer::decodeZipFile(filePath), DkZipContainer::decodeImageFile(filePath));
#endif

	QFile file(filePath);
	file.open(QIODevice::ReadOnly);

	QSharedPointer<QByteArray> ba(new QByteArray(file.readAll()));
	file.close();

	return ba;
}

bool DkBasicLoader::writeBufferToFile(const QString& fileInfo, const QSharedPointer<QByteArray> ba) const {

	if (!ba || ba->isEmpty())
		return false;

	QFile file(fileInfo);
	file.open(QIODevice::WriteOnly);
	qint64 bytesWritten = file.write(*ba.data(), ba->size());
	file.close();
	qDebug() << "[DkBasicLoader] buffer saved, bytes written: " << bytesWritten;

	if (!bytesWritten || bytesWritten == -1)
		return false;

	return true;
}

void DkBasicLoader::indexPages(const QString& filePath, const QSharedPointer<QByteArray> ba) {

	// reset counters
	mNumPages = 1;
	mPageIdx = 1;

#ifdef WITH_LIBTIFF

	QFileInfo fInfo(filePath);

	// for now we just support tiff's
	if (!fInfo.suffix().contains(QRegExp("(tif|tiff)", Qt::CaseInsensitive)))
		return;

	// first turn off nasty warning/error dialogs - (we do the GUI : )
	TIFFErrorHandler oldErrorHandler, oldWarningHandler;
	oldWarningHandler = TIFFSetWarningHandler(NULL);
	oldErrorHandler = TIFFSetErrorHandler(NULL); 

	DkTimer dt;
	TIFF* tiff = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5,5,0) && defined(Q_OS_WIN)
	std::istringstream is(ba ? ba->toStdString() : "");

	if (ba)
		tiff = TIFFStreamOpen("MemTIFF", &is);

	// read from file
	if (!tiff)
		tiff = TIFFOpen(filePath.toLatin1(), "r");	// this->mFile was here before - not sure why

	// loading from buffer allows us to load files with non-latin names
	QSharedPointer<QByteArray> bal;
	if (!tiff)
		bal = loadFileToBuffer(filePath);;
	std::istringstream isl(bal ? bal->toStdString() : "");

	if (bal)
		tiff = TIFFStreamOpen("MemTIFF", &isl);
#else
	// read from file
	tiff = TIFFOpen(filePath.toLatin1(), "r");	// this->mFile was here before - not sure why
#endif

	if (!tiff) 
		return;

	// libtiff example
	int dircount = 0;

	do {
		dircount++;

	} while (TIFFReadDirectory(tiff));

	mNumPages = dircount;

	if (mNumPages > 1)
		mPageIdx = 1;

	qDebug() << dircount << " TIFF directories... " << dt;
	TIFFClose(tiff);

	TIFFSetWarningHandler(oldWarningHandler);
	TIFFSetWarningHandler(oldErrorHandler);
#else
	Q_UNUSED(filePath);
#endif

}

bool DkBasicLoader::loadPage(int skipIdx) {

	bool imgLoaded = false;

	mPageIdx += skipIdx;

	// <= 1 since first page is loaded using qt
	if (mPageIdx > mNumPages || mPageIdx <= 1)
		return imgLoaded;

	return loadPageAt(mPageIdx);
}

bool DkBasicLoader::loadPageAt(int pageIdx) {

	bool imgLoaded = false;

#ifdef WITH_LIBTIFF

	// <= 1 since first page is loaded using qt
	if (pageIdx > mNumPages || pageIdx < 1)
		return imgLoaded;

	// first turn off nasty warning/error dialogs - (we do the GUI : )
	TIFFErrorHandler oldErrorHandler, oldWarningHandler;
	oldWarningHandler = TIFFSetWarningHandler(NULL);
	oldErrorHandler = TIFFSetErrorHandler(NULL); 

	DkTimer dt;
	TIFF* tiff = TIFFOpen(mFile.toLatin1(), "r");

#if QT_VERSION >= QT_VERSION_CHECK(5,5,0) && defined(Q_OS_WIN)

	// loading from buffer allows us to load files with non-latin names
	QSharedPointer<QByteArray> ba;
	if (!tiff)
		ba = loadFileToBuffer(mFile);
	
	std::istringstream is(ba ? ba->toStdString() : "");
	if (ba)
		tiff = TIFFStreamOpen("MemTIFF", &is);
#endif

	if (!tiff)
		return imgLoaded;

	uint32 width = 0;
	uint32 height = 0;

	// go to current directory
	for (int idx = 1; idx < pageIdx; idx++) {

		if (!TIFFReadDirectory(tiff))
			return false;
	}
	TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

	// init the qImage
	QImage img = QImage(width, height, QImage::Format_ARGB32);

	const int stopOnError = 1;
	imgLoaded = TIFFReadRGBAImageOriented(tiff, width, height, reinterpret_cast<uint32 *>(img.bits()), ORIENTATION_TOPLEFT, stopOnError) != 0;

	if (imgLoaded) {
		for (uint32 y=0; y<height; ++y)
			convert32BitOrder(img.scanLine(y), width);
	}

	TIFFClose(tiff);

	TIFFSetWarningHandler(oldWarningHandler);
	TIFFSetWarningHandler(oldErrorHandler);

	setEditImage(img, tr("Original Image"));
#else
	Q_UNUSED(pageIdx);
#endif


	return imgLoaded;
}

bool DkBasicLoader::setPageIdx(int skipIdx) {

	// do nothing if we don't have tiff pages
	if (mNumPages <= 1)
		return false;

	mPageIdxDirty = false;

	int newPageIdx = mPageIdx + skipIdx;

	if (newPageIdx > 0 && newPageIdx <= mNumPages) {
		mPageIdxDirty = true;
		mPageIdx = newPageIdx;
	}

	return mPageIdxDirty;
}

void DkBasicLoader::resetPageIdx() {

	mPageIdxDirty = false;
	mPageIdx = 1;
}

void DkBasicLoader::convert32BitOrder(void *buffer, int width) const {

#ifdef WITH_LIBTIFF
	// code from Qt QTiffHandler
	uint32 *target = reinterpret_cast<uint32 *>(buffer);
	for (int32 x=0; x<width; ++x) {
		uint32 p = target[x];
		// convert between ARGB and ABGR
		target[x] = (p & 0xff000000)
			| ((p & 0x00ff0000) >> 16)
			| (p & 0x0000ff00)
			| ((p & 0x000000ff) << 16);
	}
#else
	Q_UNUSED(buffer);
	Q_UNUSED(width);
#endif
}

QString DkBasicLoader::save(const QString& filePath, const QImage& img, int compression) {

	QSharedPointer<QByteArray> ba;

	DkTimer dt;
	if (saveToBuffer(filePath, img, ba, compression) && ba) {

		if (writeBufferToFile(filePath, ba)) {
			qDebug() << "saving to" << filePath << "in" << dt;
			return filePath;
		}
	}

	return QString();
}

bool DkBasicLoader::saveToBuffer(const QString& filePath, const QImage& img, QSharedPointer<QByteArray>& ba, int compression) const {

	bool bufferCreated = false;

	if (!ba) {
		ba = QSharedPointer<QByteArray>(new QByteArray());
		bufferCreated = true;
	}

	bool saved = false;

	QFileInfo fInfo(filePath);

	if (fInfo.suffix().contains("ico", Qt::CaseInsensitive)) {
		saved = saveWindowsIcon(img, ba);
	}
#if QT_VERSION < 0x050000 // qt5 natively supports r/w webp
	else if (fInfo.suffix().contains("webp", Qt::CaseInsensitive)) {
		saved = saveWebPFile(img, ba, compression);
	}
#endif
	else {

		bool hasAlpha = DkImage::alphaChannelUsed(img);
		QImage sImg = img;


		// JPEG 2000 can only handle 32 or 8bit images
		if (!hasAlpha && img.colorTable().empty() && !fInfo.suffix().contains(QRegExp("(avif|j2k|jp2|jpf|jpx|jxl|png)"))) {
			sImg = sImg.convertToFormat(QImage::Format_RGB888);
		}
		else if (fInfo.suffix().contains(QRegExp("(j2k|jp2|jpf|jpx)")) && sImg.depth() != 32 && sImg.depth() != 8) {
			if (sImg.hasAlphaChannel()) {
				sImg = sImg.convertToFormat(QImage::Format_ARGB32);
			} else {
				sImg = sImg.convertToFormat(QImage::Format_RGB32);
			}
		}

		if (fInfo.suffix().contains(QRegExp("(png)")))
			compression = -1;

		QBuffer fileBuffer(ba.data());
		//size_t s = fileBuffer.size();
		fileBuffer.open(QIODevice::WriteOnly);
		QImageWriter* imgWriter = new QImageWriter(&fileBuffer, fInfo.suffix().toStdString().c_str());
		
		if (compression >= 0) {	// -1 -> use Qt's default
			imgWriter->setCompression(compression);
			imgWriter->setQuality(compression);
		}
		if (compression == -1 && imgWriter->format() == "jpg") {
			imgWriter->setQuality(DkSettingsManager::instance().settings().app().defaultJpgQuality);
		}

#if QT_VERSION >= 0x050500
		imgWriter->setOptimizedWrite(true);			// this saves space TODO: user option here?
		imgWriter->setProgressiveScanWrite(true);
#endif
		saved = imgWriter->write(sImg);
		delete imgWriter;
	}

	if (saved && mMetaData) {
		
		if (!mMetaData->isLoaded() || !mMetaData->hasMetaData()) {

			if (!bufferCreated)
				mMetaData->readMetaData(filePath, ba);
			else
				// if we created the buffere here - force loading metadata from the file
				mMetaData->readMetaData(filePath);
			}

			if (mMetaData->isLoaded()) {
			
				try {
					// be careful: here we actually lie about the constness
					mMetaData->updateImageMetaData(img);
					if (!mMetaData->saveMetaData(ba, true))
						mMetaData->clearExifState();
				} 
				catch (...) {
					// is it still throwing anything?
					qInfo() << "Sorry, I could not save the meta data...";
					// clear exif state here -> the 'dirty' flag would otherwise edit the original image (see #514)
					mMetaData->clearExifState();
				}
			}
	}

	if (!saved)
		emit errorDialogSignal(tr("Sorry, I could not save: %1").arg(fInfo.fileName()));

	return saved;
}

void DkBasicLoader::saveThumbToMetaData(const QString& filePath) {

	QSharedPointer<QByteArray> ba;	// dummy
	saveThumbToMetaData(filePath, ba);
}

void DkBasicLoader::saveMetaData(const QString& filePath) {

	QSharedPointer<QByteArray> ba;	// dummy
	saveMetaData(filePath, ba);
}

void DkBasicLoader::saveThumbToMetaData(const QString& filePath, QSharedPointer<QByteArray>& ba) {
	
	if (!hasImage())
		return;

	mMetaData->setThumbnail(DkImage::createThumb(image()));
	saveMetaData(filePath, ba);
}

void DkBasicLoader::saveMetaData(const QString& filePath, QSharedPointer<QByteArray>& ba) {

	if (!ba)
		ba = QSharedPointer<QByteArray>(new QByteArray());

	if (ba->isEmpty() && mMetaData->isDirty()) {
		ba = loadFileToBuffer(filePath);
	}

	bool saved = false;
	try {
		saved = mMetaData->saveMetaData(ba);
	} 
	catch(...) {
		qInfo() << "could not save metadata...";
	}
	
	if (saved)
		writeBufferToFile(filePath, ba);

}

bool DkBasicLoader::isContainer(const QString& filePath) {

	QFileInfo fInfo(filePath);
	if (!fInfo.isFile() || !fInfo.exists())
		return false;

	QString suffix = fInfo.suffix();

	if (suffix.isEmpty())
		return false;

	for (int idx = 0; idx < DkSettingsManager::param().app().containerFilters.size(); idx++) {

		if (DkSettingsManager::param().app().containerFilters[idx].contains(suffix))
			return true;
	}

	return false;
}

/**
 * Releases the currently loaded images.
 **/ 
void DkBasicLoader::release(bool clear) {

	// TODO: auto save routines here?
	//qDebug() << file.fileName() << " released...";
	saveMetaData(mFile);

	mImages.clear();
	//metaData.clear();
	
	// TODO: where should we clear the metadata?
	if (clear || !mMetaData->isDirty())
		mMetaData = QSharedPointer<DkMetaDataT>(new DkMetaDataT());

}

#ifdef Q_OS_WIN
bool DkBasicLoader::saveWindowsIcon(const QString& filePath, const QImage& img) const {

	QSharedPointer<QByteArray> ba;

	if (saveWindowsIcon(img, ba) && ba && !ba->isEmpty()) {

		writeBufferToFile(filePath, ba);
		return true;
	}

	return false;
}

struct ICONDIRENTRY
{
	UCHAR nWidth;
	UCHAR nHeight;
	UCHAR nNumColorsInPalette; // 0 if no palette
	UCHAR nReserved; // should be 0
	WORD nNumColorPlanes; // 0 or 1
	WORD nBitsPerPixel;
	ULONG nDataLength; // length in bytes
	ULONG nOffset; // offset of BMP or PNG data from beginning of file
};

bool DkBasicLoader::saveWindowsIcon(const QImage& img, QSharedPointer<QByteArray>& ba) const {

	// this code is an adopted version of:
	// http://stackoverflow.com/questions/2289894/how-can-i-save-hicon-to-an-ico-file

	if (!ba)
		ba = QSharedPointer<QByteArray>(new QByteArray());

	HICON hIcon = QtWin::toHICON(QPixmap::fromImage(img));
	int nColorBits = 32;

	QBuffer buffer(ba.data());
	buffer.open(QIODevice::WriteOnly);

	if (!hIcon)
		return false;

	HDC screenDevice = GetDC(0);

	// Write header:
	UCHAR icoHeader[6] = { 0, 0, 1, 0, 1, 0 }; // ICO file with 1 image
	buffer.write((const char*)(&icoHeader), sizeof(icoHeader));

	// Get information about icon:
	ICONINFO iconInfo;
	GetIconInfo(hIcon, &iconInfo);
	HGDIOBJ handle1(iconInfo.hbmColor); // free bitmaps when function ends
	BITMAPINFO bmInfo = { 0 };
	bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biBitCount = 0;    // don't get the color table     
	if (!GetDIBits(screenDevice, iconInfo.hbmColor, 0, 0, NULL, &bmInfo, DIB_RGB_COLORS))
	{
		return false;
	}

	// Allocate size of bitmap info header plus space for color table:
	int nBmInfoSize = sizeof(BITMAPINFOHEADER);
	if (nColorBits < 24)
	{
		nBmInfoSize += sizeof(RGBQUAD) * (int)(1 << nColorBits);
	}

	QSharedPointer<UCHAR> bitmapInfo(new UCHAR[nBmInfoSize]);
	BITMAPINFO* pBmInfo = (BITMAPINFO*)bitmapInfo.data();
	memcpy(pBmInfo, &bmInfo, sizeof(BITMAPINFOHEADER));

	// Get bitmap data:
	QSharedPointer<UCHAR> bits(new UCHAR[bmInfo.bmiHeader.biSizeImage]);
	pBmInfo->bmiHeader.biBitCount = (WORD)nColorBits;
	pBmInfo->bmiHeader.biCompression = BI_RGB;
	if (!GetDIBits(screenDevice, iconInfo.hbmColor, 0, bmInfo.bmiHeader.biHeight, bits.data(), pBmInfo, DIB_RGB_COLORS))
	{
		return false;
	}

	// Get mask data:
	BITMAPINFO maskInfo = { 0 };
	maskInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	maskInfo.bmiHeader.biBitCount = 0;  // don't get the color table     
	if (!GetDIBits(screenDevice, iconInfo.hbmMask, 0, 0, NULL, &maskInfo, DIB_RGB_COLORS))
	{
		return false;
	}

	QSharedPointer<UCHAR> maskBits(new UCHAR[maskInfo.bmiHeader.biSizeImage]);
	QSharedPointer<UCHAR> maskInfoBytes(new UCHAR[sizeof(BITMAPINFO) + 2 * sizeof(RGBQUAD)]);
	BITMAPINFO* pMaskInfo = (BITMAPINFO*)maskInfoBytes.data();
	memcpy(pMaskInfo, &maskInfo, sizeof(maskInfo));
	if (!GetDIBits(screenDevice, iconInfo.hbmMask, 0, maskInfo.bmiHeader.biHeight, maskBits.data(), pMaskInfo, DIB_RGB_COLORS))
	{
		return false;
	}

	// Write directory entry:
	ICONDIRENTRY dir;
	dir.nWidth = (UCHAR)pBmInfo->bmiHeader.biWidth;
	dir.nHeight = (UCHAR)pBmInfo->bmiHeader.biHeight;
	dir.nNumColorsInPalette = (nColorBits == 4 ? 16 : 0);
	dir.nReserved = 0;
	dir.nNumColorPlanes = 0;
	dir.nBitsPerPixel = pBmInfo->bmiHeader.biBitCount;
	dir.nDataLength = pBmInfo->bmiHeader.biSizeImage + pMaskInfo->bmiHeader.biSizeImage + nBmInfoSize;
	dir.nOffset = sizeof(dir) + sizeof(icoHeader);
	buffer.write((const char*)&dir, sizeof(dir));

	// Write DIB header (including color table):
	int nBitsSize = pBmInfo->bmiHeader.biSizeImage;
	pBmInfo->bmiHeader.biHeight *= 2; // because the header is for both image and mask
	pBmInfo->bmiHeader.biCompression = 0;
	pBmInfo->bmiHeader.biSizeImage += pMaskInfo->bmiHeader.biSizeImage; // because the header is for both image and mask
	buffer.write((const char*)&pBmInfo->bmiHeader, nBmInfoSize);

	// Write image data:
	buffer.write((const char*)bits.data(), nBitsSize);

	// Write mask data:
	buffer.write((const char*)maskBits.data(), pMaskInfo->bmiHeader.biSizeImage);

	buffer.close();

	DeleteObject(handle1);

	return true;
}

#endif // #ifdef Q_OS_WIN

#ifdef WITH_OPENCV

cv::Mat DkBasicLoader::getImageCv() {
	return cv::Mat();
}

bool DkBasicLoader::loadOpenCVVecFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba, QSize s) const {

	if (!ba)
		ba = QSharedPointer<QByteArray>(new QByteArray());

	// load from file?
	if (ba->isEmpty())
		ba = loadFileToBuffer(filePath);

	if (ba->isEmpty())
		return false;

	// read header & get a pointer to the first image
	int fileCount, vecSize;
	const unsigned char* imgPtr = (const unsigned char*)ba->constData();
	if (!readHeader(&imgPtr, fileCount, vecSize))
		return false;

	int guessedW = 0;
	int guessedH = 0;

	getPatchSizeFromFileName(QFileInfo(filePath).fileName(), guessedW, guessedH);

	qDebug() << "patch size from filename: " << guessedW << " x " << guessedH;

	if(vecSize > 0 && !guessedH && !guessedW) {
		guessedW = qFloor(sqrt((float) vecSize));
		if(guessedW > 0)
			guessedH = vecSize/guessedW;
	}

	if(guessedW <= 0 || guessedH <= 0 || guessedW * guessedH != vecSize) {

		// TODO: ask user
		qDebug() << "dimensions do not match, patch size: " << guessedW << " x " << guessedH << " vecSize: " << vecSize;
		return false;
	}

	int fSize = ba->size();
	int numElements = 0;

	// guess size
	if (s.isEmpty()) {
		double nEl = (fSize-64)/(vecSize*2);
		nEl = (fSize-64-qCeil(nEl))/(vecSize*2)+1;	// opencv adds one byte per image - so we take care for this here

		if (qFloor(nEl) != qCeil(nEl))
			return false;
		numElements = qRound(nEl);
	}

	double nRowsCols = sqrt(numElements);
	int numCols = qCeil(nRowsCols);
	int minusOneRow = (qFloor(nRowsCols) != qCeil(nRowsCols) && nRowsCols - qFloor(nRowsCols) < 0.5) ? 1 : 0;

	cv::Mat allPatches((numCols-minusOneRow)*guessedH, numCols*guessedW, CV_8UC1, cv::Scalar(125));

	for (int idx = 0; idx < numElements; idx++) {

		if (*imgPtr != 0) {
			qDebug() << "skipping non-empty byte - there is something seriously wrong here!";
			//return false;	// stop if the byte is non-empty -> otherwise we might read wrong memory
		}

		imgPtr++;	// there is an empty byte between images
		cv::Mat cPatch = getPatch(&imgPtr, QSize(guessedW, guessedH));
		cv::Mat cPatchAll = allPatches(cv::Rect(idx%numCols*guessedW, qFloor(idx/numCols)*guessedH, guessedW, guessedH));

		if (!cPatchAll.empty())
			cPatch.copyTo(cPatchAll);
	}

	img = DkImage::mat2QImage(allPatches);
	img = img.convertToFormat(QImage::Format_ARGB32);

	//setEditImage(img, tr("Original Image"));

	return true;
}

void DkBasicLoader::getPatchSizeFromFileName(const QString& fileName, int& width, int& height) const {

	// parse patch size from file
	QStringList sections = fileName.split(QRegExp("[-\\.]"));	

	for (int idx = 0; idx < sections.size(); idx++) {

		QString tmpSec = sections[idx];
		qDebug() << "section: " << tmpSec;

		if (tmpSec.contains("w"))
			width = tmpSec.remove("w").toInt();
		else if (tmpSec.contains("h"))
			height = tmpSec.remove("h").toInt();
	}

}

bool DkBasicLoader::readHeader(const unsigned char** dataPtr, int& fileCount, int& vecSize) const {

	const int* pData = (const int*)*dataPtr;
	fileCount = *pData; pData++;	// read file count
	vecSize = *pData;				// read vec size

	qDebug() << "vec size: " << vecSize << " fileCount " << fileCount;

	*dataPtr += 12;	// skip the first 12 (header) bytes

	return true;
}

// the double pointer is here needed to additionally increase the pointer value
cv::Mat DkBasicLoader::getPatch(const unsigned char** dataPtr, QSize patchSize) const {

	cv::Mat img8U(patchSize.height(), patchSize.width(), CV_8UC1, cv::Scalar(0));

	// ok, take just the second byte
	for (int rIdx = 0; rIdx < img8U.rows; rIdx++) {

		unsigned char* ptr8U = img8U.ptr<unsigned char>(rIdx);

		for (int cIdx = 0; cIdx < img8U.cols; cIdx++) {
			ptr8U[cIdx] = **dataPtr;
			*dataPtr += 2;	// it is strange: opencv stores vec files as 16 bit but just use the 2nd byte
		}
	}

	return img8U;
}

int DkBasicLoader::mergeVecFiles(const QStringList& vecFilePaths, QString& saveFilePath) const {

	int lastVecSize = 0;
	int totalFileCount = 0;
	int vecCount = 0;
	int pWidth = 0, pHeight = 0;
	QByteArray vecBuffer;

	for (const QString& filePath : vecFilePaths) {

		QFileInfo fInfo(filePath);
		QSharedPointer<QByteArray> ba = loadFileToBuffer(filePath);
		if (ba->isEmpty()){
			qDebug() << "could not load: " << fInfo.fileName();
			continue;
		}

		int fileCount, vecSize;
		const unsigned char* dataPtr = (const unsigned char*)ba->constData();
		if (!readHeader(&dataPtr, fileCount, vecSize)) {
			qDebug() << "could not read header, skipping: " << fInfo.fileName();
			continue;
		}

		if (lastVecSize && vecSize != lastVecSize) {
			qDebug() << "wrong vec size, skipping: " << fInfo.fileName();
			continue;
		}

		vecBuffer.append((const char*)dataPtr, vecSize*fileCount*2+fileCount);	// +fileCount accounts for the '\0' bytes between the patches

		getPatchSizeFromFileName(fInfo.fileName(), pWidth, pHeight);

		totalFileCount += fileCount;
		lastVecSize = vecSize;

		vecCount++;
	}

	// don't save if we could not merge the files
	if (!vecCount)
		return vecCount;

	unsigned int* header = new unsigned int[3];
	header[0] = totalFileCount;
	header[1] = lastVecSize;
	header[2] = 0;

	vecBuffer.prepend((const char*) header, 3*sizeof(int));

	QFileInfo saveFileInfo(saveFilePath);

	// append width, height if we don't know
	if (pWidth && pHeight) {
		QString whString = "-w" + QString::number(pWidth) + "-h" + QString::number(pHeight);
		saveFileInfo = QFileInfo(saveFileInfo.absolutePath(), saveFileInfo.baseName() + whString + "." + saveFileInfo.suffix());
	}

	QFile file(saveFileInfo.absoluteFilePath());
	file.open(QIODevice::WriteOnly);
	file.write(vecBuffer);
	file.close();

	return vecCount;
}

#endif // #ifdef WITH_OPENCV

// FileDownloader --------------------------------------------------------------------
FileDownloader::FileDownloader(const QUrl& imageUrl, const QString& filePath, QObject *parent) : QObject(parent) {
	
	mFilePath = filePath;
	
	QNetworkProxyQuery npq(QUrl("https://google.com"));
	QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
	if (!listOfProxies.empty() && listOfProxies[0].hostName() != "") {
		mWebCtrl.setProxy(listOfProxies[0]);
	}

	connect(&mWebCtrl, SIGNAL(finished(QNetworkReply*)),
		SLOT(fileDownloaded(QNetworkReply*)));

	downloadFile(imageUrl);
}

FileDownloader::~FileDownloader() {
}

void FileDownloader::downloadFile(const QUrl& url) {

	QNetworkRequest request(url);
	mWebCtrl.get(request);
	mUrl = url;
}

void FileDownloader::saved() {

	if (mSaveWatcher.result()) {
		qInfo() << "downloaded image saved to" << mFilePath;
		emit downloaded(mFilePath);
	}
	else {
		qWarning() << "could not download file to " << mFilePath;
	}
}

bool FileDownloader::save(const QString& filePath, const QSharedPointer<QByteArray> data) {

	if (!data) {
		qWarning() << "cannot save file if data is NULL";
		return false;
	}

	QFileInfo fi(filePath);

	if (!fi.absoluteDir().exists())
		QDir().mkpath(fi.absolutePath());

	QFile f(filePath);
	f.open(QIODevice::WriteOnly);

	return f.write(*data);
}

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {

	if (pReply->error() != QNetworkReply::NoError) {
		qWarning() << "I could not download: " << mUrl;
		qWarning() << pReply->errorString();
	}

	mDownloadedData = QSharedPointer<QByteArray>(new QByteArray(pReply->readAll()));
	//emit a signal
	pReply->deleteLater();

	// data only requested
	if (mFilePath.isEmpty()) {
		emit downloaded();
	}
	// ok save it
	else {
		connect(&mSaveWatcher, SIGNAL(finished()), this, SLOT(saved()), Qt::UniqueConnection);
		mSaveWatcher.setFuture(QtConcurrent::run(&nmc::FileDownloader::save, mFilePath, mDownloadedData));
	}
}

QSharedPointer<QByteArray> FileDownloader::downloadedData() const {
	return mDownloadedData;
}

QUrl FileDownloader::getUrl() const {
	return mUrl;
}

#ifdef WITH_QUAZIP

// DkZipContainer --------------------------------------------------------------------
DkZipContainer::DkZipContainer(const QString& encodedFilePath) {

	if (!encodedFilePath.isEmpty() && 
		encodedFilePath.contains(mZipMarker)) {
		mImageInZip = true;
		mEncodedFilePath = encodedFilePath;
		mZipFilePath = decodeZipFile(encodedFilePath);
		mImageFileName = decodeImageFile(encodedFilePath);
	}
	else
		mImageInZip = false;
}

QString DkZipContainer::encodeZipFile(const QString& zipFile, const QString& imageFile) {

	// if you think this code is unreadable, take a look at the old line:
	//return QFileInfo(QDir(zipFile.absoluteFilePath() + mZipMarker + imageFile.left(imageFile.lastIndexOf("/") + 1).replace("/", mZipMarker)),(imageFile.lastIndexOf("/") < 0) ? imageFile : imageFile.right(imageFile.size() - imageFile.lastIndexOf("/") - 1));

	QDir dir = QDir(zipFile + mZipMarker + imageFile.left(imageFile.lastIndexOf("/") + 1).replace("/", mZipMarker));
	QString fileName = (imageFile.lastIndexOf("/") < 0) ? imageFile : imageFile.right(imageFile.size() - imageFile.lastIndexOf("/") - 1);

	return QFileInfo(dir, fileName).absoluteFilePath();
}

QString DkZipContainer::decodeZipFile(const QString& encodedFileInfo) {

	QString encodedDir = QFileInfo(encodedFileInfo).absolutePath();

	return encodedDir.left(encodedDir.indexOf(mZipMarker));
}

QString DkZipContainer::decodeImageFile(const QString& encodedFileInfo) {

	// get relative zip path
	QString tmp = encodedFileInfo.right(encodedFileInfo.size() - encodedFileInfo.indexOf(mZipMarker) - QString(mZipMarker).size());
	tmp = tmp.replace(mZipMarker, "/");
	tmp = tmp.replace("//", "/");

	// diem: this fixes an issue with images that are in a zip's root folder
	if (tmp.startsWith("/"))
		tmp = tmp.right(tmp.length()-1);

	return tmp;
}

QSharedPointer<QByteArray> DkZipContainer::extractImage(const QString& zipFile, const QString& imageFile) {

	QuaZip zip(zipFile);		
	if(!zip.open(QuaZip::mdUnzip)) 
		return QSharedPointer<QByteArray>(new QByteArray());

	qDebug() << "DkZip::extractImage filePath: " << zipFile;
	qDebug() << "3.0 image file" << imageFile;

	zip.setCurrentFile(imageFile);
	QuaZipFile extractedFile(&zip);
	if(!extractedFile.open(QIODevice::ReadOnly) || extractedFile.getZipError() != UNZ_OK) 
		return QSharedPointer<QByteArray>(new QByteArray());

	QSharedPointer<QByteArray> ba(new QByteArray(extractedFile.readAll()));
	extractedFile.close();

	zip.close();

	return ba;
}

void DkZipContainer::extractImage(const QString& zipFile, const QString& imageFile, QByteArray& ba) {

	QuaZip zip(zipFile);		
	if(!zip.open(QuaZip::mdUnzip)) 
		return;

	zip.setCurrentFile(imageFile);
	QuaZipFile extractedFile(&zip);
	if(!extractedFile.open(QIODevice::ReadOnly) || extractedFile.getZipError() != UNZ_OK) 
		return;

	ba = QByteArray(extractedFile.readAll());
	extractedFile.close();

	zip.close();

}

bool DkZipContainer::isZip() const {

	return mImageInZip;
}

QString DkZipContainer::getZipFilePath() const {

	return mZipFilePath;
}

QString DkZipContainer::getImageFileName() const {

	return mImageFileName;
}

QString DkZipContainer::getEncodedFilePath() const {

	return mEncodedFilePath;
}

QString DkZipContainer::zipMarker() {

	return mZipMarker;
}

#endif

// DkRawLoader --------------------------------------------------------------------
DkRawLoader::DkRawLoader(const QString & filePath, const QSharedPointer<DkMetaDataT>& metaData) {
	mFilePath = filePath;
	mMetaData = metaData;
}

bool DkRawLoader::isEmpty() const {
	return mFilePath.isEmpty();
}

void DkRawLoader::setLoadFast(bool fast) {
	mLoadFast = fast;
}

bool DkRawLoader::load(const QSharedPointer<QByteArray> ba) {

	DkTimer dt;

	// try fetching the preview
	if (loadPreview(ba))
		return true;

#ifdef WITH_LIBRAW
	
	try {

		// open the buffer
		LibRaw iProcessor;

		if (!openBuffer(ba, iProcessor)) {
			qDebug() << "could not open buffer for" << mFilePath;
			return false;
		}

		// check camera models for specific hacks
		detectSpecialCamera(iProcessor);

		// try loading RAW preview
		if (mLoadFast) {
			mImg = loadPreviewRaw(iProcessor);

			// are we done already?
			if (!mImg.isNull())
				return true;
		}

		//unpack the data
		int error = iProcessor.unpack();
		if (std::strcmp(iProcessor.version(), "0.13.5") != 0)	// fixes a bug specific to libraw 13 - version call is UNTESTED
			iProcessor.raw2image();

		if (error != LIBRAW_SUCCESS)
			return false;

		// develop using libraw
		if (mCamType == camera_unknown) {
			error = iProcessor.dcraw_process();

			auto rimg = iProcessor.dcraw_make_mem_image();

			if (rimg) {

				mImg = QImage(rimg->data, rimg->width, rimg->height, rimg->width * 3, QImage::Format_RGB888);
				mImg = mImg.copy();		// make a deep copy...
				LibRaw::dcraw_clear_mem(rimg);

				return true;
			}
		}

		// demosaic image
		cv::Mat rawMat;

		if (iProcessor.imgdata.idata.filters)
			rawMat = demosaic(iProcessor);
		else
			rawMat = prepareImg(iProcessor);

		// color correction + white balance
		if (mIsChromatic) {
			whiteBalance(iProcessor, rawMat);
		}
		
		// gamma correction
		gammaCorrection(iProcessor, rawMat);

		// reduce color noise
		if (DkSettingsManager::param().resources().filterRawImages && mIsChromatic)
			reduceColorNoise(iProcessor, rawMat);

		mImg = raw2Img(iProcessor, rawMat);

		//qDebug() << "img size" << mImg.size();
		//qDebug() << "raw mat size" << rawMat.rows << "x" << rawMat.cols;
		iProcessor.recycle();
		rawMat.release();
	}
	catch (...) {
		qDebug() << "[RAW] error during processing...";
		return false;
	}

	qInfo() << "[RAW] loaded in " << dt;

#endif

	return !mImg.isNull();
}

QImage DkRawLoader::image() const {
	return mImg;
}

bool DkRawLoader::loadPreview(const QSharedPointer<QByteArray>& ba) {

	try {

		// try to get preview image from exiv2
		if (mMetaData) {
			if (mLoadFast || DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_always ||
				DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large) {

				mMetaData->readMetaData(mFilePath, ba);

				int minWidth = 0;

#ifdef WITH_LIBRAW	// if nomacs has libraw - we can still hope for a fallback -> otherwise try whatever we have here
				if (DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large)
					minWidth = 1920;
#endif
				mImg = mMetaData->getPreviewImage(minWidth);

				if (!mImg.isNull()) {
					qDebug() << "[RAW] loaded with exiv2";
					return true;
				}
			}
		}
	}
	catch (...) {
		qWarning() << "Exception caught during fetching RAW from thumbnail...";
	}

	return false;
}


#ifdef WITH_LIBRAW

// here are some hints from earlier days...
//// (-w) Use camera white balance, if possible (otherwise, fallback to auto_wb)
//iProcessor.imgdata.params.use_camera_wb = 1;
//// (-a) Use automatic white balance obtained after averaging over the entire image
//iProcessor.imgdata.params.use_auto_wb = 1;
//// (-q 3) Adaptive homogeneity-directed de-mosaicing algorithm (AHD)
//iProcessor.imgdata.params.user_qual = 3;
//iProcessor.imgdata.params.output_tiff = 1;
////iProcessor.imgdata.params.four_color_rgb = 1;
////iProcessor.imgdata.params.output_color = 1; //sRGB  (0...raw)
//// RAW data filtration mode during data unpacking and post-processing
//iProcessor.imgdata.params.filtering_mode = LIBRAW_FILTERING_AUTOMATIC;


QImage DkRawLoader::loadPreviewRaw(LibRaw & iProcessor) const {
	
	int tW = iProcessor.imgdata.thumbnail.twidth;

	if (DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_always ||
		(DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large && tW >= 1920)) {

		// crashes here if image is broken
		int err = iProcessor.unpack_thumb();
		char* tPtr = iProcessor.imgdata.thumbnail.thumb;

		if (!err && tPtr) {

			QImage img;
			img.loadFromData((const uchar*)tPtr, iProcessor.imgdata.thumbnail.tlength);

			// we're good to go
			if (!img.isNull()) {
				qDebug() << "[RAW] I loaded the RAW's thumbnail";
				return img;
			}
			else
				qDebug() << "RAW could not load the thumb";
		}
		else
			qDebug() << "error unpacking the thumb...";
	}

	// default: return nothing
	return QImage();
}

bool DkRawLoader::openBuffer(const QSharedPointer<QByteArray>& ba, LibRaw& iProcessor) const {

	int error = LIBRAW_DATA_ERROR;

	QFileInfo fi(mFilePath);

	//use iprocessor from libraw to read the data
	// OK - so LibRaw 0.17 cannot identify iiq files in the buffer - so we load them from the file
	if (fi.suffix().contains("iiq", Qt::CaseInsensitive) || !ba || ba->isEmpty()) {
		error = iProcessor.open_file(mFilePath.toStdString().c_str());
	}
	else {
		// the buffer check is because:
		// libraw has an error when loading buffers if the first 4 bytes encode as 'RIFF'
		// and no data follows at all
		if (ba->isEmpty() || ba->size() < 100)
			return false;

		error = iProcessor.open_buffer((void*)ba->constData(), ba->size());
	}

	return (error == LIBRAW_SUCCESS);
}

void DkRawLoader::detectSpecialCamera(const LibRaw & iProcessor) {

	if (QString(iProcessor.imgdata.idata.model) == "IQ260 Achromatic")
		mIsChromatic = false;
	
	if (QString(iProcessor.imgdata.idata.model).contains("IQ260"))
		mCamType = camera_iiq;
	else if (QString(iProcessor.imgdata.idata.make).compare("Canon", Qt::CaseInsensitive))
		mCamType = camera_canon;

	// add your camera flag (for hacks) here
}

cv::Mat DkRawLoader::demosaic(LibRaw & iProcessor) const {

	cv::Mat rawMat = cv::Mat(iProcessor.imgdata.sizes.height, iProcessor.imgdata.sizes.width, CV_16UC1);
	double dynamicRange = (double)(iProcessor.imgdata.color.maximum - iProcessor.imgdata.color.black);

	// normalize all image values
	for (int rIdx = 0; rIdx < rawMat.rows; rIdx++) {
		unsigned short *ptrRaw = rawMat.ptr<unsigned short>(rIdx);

		for (int cIdx = 0; cIdx < rawMat.cols; cIdx++) {

			int colIdx = iProcessor.COLOR(rIdx, cIdx);
			double val = (double)(iProcessor.imgdata.image[(rawMat.cols*rIdx) + cIdx][colIdx]);
			
			// normalize the value w.r.t the black point defined
			val = (val - iProcessor.imgdata.color.black) / dynamicRange;
			ptrRaw[cIdx] = clip<unsigned short>(val * USHRT_MAX);  // for conversion to 16U
		}
	}

	// no demosaicing
	if (mIsChromatic) {

		unsigned long type = (unsigned long)iProcessor.imgdata.idata.filters;
		type = type & 255;

		cv::Mat rgbImg;

		//define bayer pattern
		if (type == 180) {
			cvtColor(rawMat, rgbImg, CV_BayerBG2RGB);		//bitmask  10 11 01 00  -> 3(G) 2(B) 1(G) 0(R) ->	RG RG RG
															//													GB GB GB
		}
		else if (type == 30) {
			cvtColor(rawMat, rgbImg, CV_BayerRG2RGB);		//bitmask  00 01 11 10	-> 0 1 3 2
		}
		else if (type == 225) {
			cvtColor(rawMat, rgbImg, CV_BayerGB2RGB);		//bitmask  11 10 00 01
		}
		else if (type == 75) {
			cvtColor(rawMat, rgbImg, CV_BayerGR2RGB);		//bitmask  01 00 10 11
		}
		else {
			qWarning() << "Wrong Bayer Pattern (not BG, RG, GB, GR)\n";
			return cv::Mat();
		}

		rawMat = rgbImg;
	}

	// 16U (1 or 3 channeled) Mat
	return rawMat;
}

cv::Mat DkRawLoader::prepareImg(const LibRaw & iProcessor) const {

	cv::Mat rawMat = cv::Mat(iProcessor.imgdata.sizes.height, iProcessor.imgdata.sizes.width, CV_16UC3, cv::Scalar(0));
	double dynamicRange = (double)(iProcessor.imgdata.color.maximum - iProcessor.imgdata.color.black);

	// normalization function
	auto normalize = [&](double val) { 
	
		val = (val - iProcessor.imgdata.color.black) / dynamicRange;
		return clip<unsigned short>(val * USHRT_MAX);
	};

	for (int rIdx = 0; rIdx < rawMat.rows; rIdx++) {
		unsigned short *ptrI = rawMat.ptr<unsigned short>(rIdx);

		for (int cIdx = 0; cIdx < rawMat.cols; cIdx++) {

			*ptrI = normalize(iProcessor.imgdata.image[rawMat.cols*rIdx + cIdx][0]);
			ptrI++;
			*ptrI = normalize(iProcessor.imgdata.image[rawMat.cols*rIdx + cIdx][1]);
			ptrI++;
			*ptrI = normalize(iProcessor.imgdata.image[rawMat.cols*rIdx + cIdx][2]);
			ptrI++;
		}
	}

	return rawMat;
}

cv::Mat DkRawLoader::whiteMultipliers(const LibRaw & iProcessor) const {
	
	// get camera white balance multipliers
	cv::Mat wm(1, 4, CV_32FC1);
	
	float* wmp = wm.ptr<float>();

	for (int idx = 0; idx < wm.cols; idx++)
		wmp[idx] = iProcessor.imgdata.color.cam_mul[idx];

	if (wmp[3] == 0)
		wmp[3] = wmp[1];	// take green (usually its RGBG)

	// normalize white balance multipliers
	float w = (float)cv::sum(wm)[0] / 4.0f;
	float maxW = 1.0f;

	//clipping according the camera model
	//if w > 2.0 maxW is 256, otherwise 512
	//tested empirically
	//check if it can be defined by some metadata settings?
	if (w > 2.0f)
		maxW = 255.0f;
	if (w > 2.0f && mCamType == camera_canon)
		maxW = 511.0f;	// some cameras would even need ~800 - why?

	//normalize white point
	wm /= maxW;

	// 1 x 4 32FC1 white balance vector
	return wm;
}

cv::Mat DkRawLoader::gammaTable(const LibRaw & iProcessor) const {
	
	// OK this is an instance of reverse engineering:
	// we found out that the values of (at least) the PhaseOne's achromatic back have to be doubled
	// our images are no close to what their software (Capture One does) - only the gamma correction
	// seems to be slightly different... -> now we can load compressed IIQs that are not supported by PS : )
	double cameraHackMlp = (QString(iProcessor.imgdata.idata.model) == "IQ260 Achromatic") ? 2.0 : 1.0;

	//read gamma value and create gamma table	
	double gamma = (double)iProcessor.imgdata.params.gamm[0];
	
	cv::Mat gmt(1, USHRT_MAX, CV_16UC1);
	unsigned short* gmtp = gmt.ptr<unsigned short>();
	
	for (int idx = 0; idx < gmt.cols; idx++) {
		gmtp[idx] = clip<unsigned short>(qRound((1.099*std::pow((double)idx / USHRT_MAX, gamma) - 0.099) * 255 * cameraHackMlp));
	}

	// a 1 x 65535 U16 gamma table
	return gmt;
}

void DkRawLoader::whiteBalance(const LibRaw & iProcessor, cv::Mat & img) const {

	// white balance must not be empty at this point
	cv::Mat wb = whiteMultipliers(iProcessor);
	const float* wbp = wb.ptr<float>();
	assert(wb.cols == 4);

	for (int rIdx = 0; rIdx < img.rows; rIdx++) {
		
		unsigned short *ptr = img.ptr<unsigned short>(rIdx);
		
		for (int cIdx = 0; cIdx < img.cols; cIdx++) {
			
			//apply white balance correction
			unsigned short r = clip<unsigned short>(*ptr		* wbp[0]);
			unsigned short g = clip<unsigned short>(*(ptr+1)	* wbp[1]);
			unsigned short b = clip<unsigned short>(*(ptr+2)	* wbp[2]);

			//apply color correction					
			int cr = qRound(iProcessor.imgdata.color.rgb_cam[0][0] * r + 
							iProcessor.imgdata.color.rgb_cam[0][1] * g + 
							iProcessor.imgdata.color.rgb_cam[0][2] * b);
			int cg = qRound(iProcessor.imgdata.color.rgb_cam[1][0] * r + 
							iProcessor.imgdata.color.rgb_cam[1][1] * g + 
							iProcessor.imgdata.color.rgb_cam[1][2] * b);
			int cb = qRound(iProcessor.imgdata.color.rgb_cam[2][0] * r + 
							iProcessor.imgdata.color.rgb_cam[2][1] * g + 
							iProcessor.imgdata.color.rgb_cam[2][2] * b);

			// clip & save color corrected values
			*ptr = clip<unsigned short>(cr);
			ptr++;
			*ptr = clip<unsigned short>(cg);
			ptr++;
			*ptr = clip<unsigned short>(cb);
			ptr++;
		}
	}
}

void DkRawLoader::gammaCorrection(const LibRaw & iProcessor, cv::Mat& img) const {

	// white balance must not be empty at this point
	cv::Mat gt = gammaTable(iProcessor);
	const unsigned short* gammaLookup = gt.ptr<unsigned short>();
	assert(gt.cols == USHRT_MAX);
	
	for (int rIdx = 0; rIdx < img.rows; rIdx++) {

		unsigned short *ptr = img.ptr<unsigned short>(rIdx);

		for (int cIdx = 0; cIdx < img.cols * img.channels(); cIdx++) {

			// values close to 0 are treated linear
			if (ptr[cIdx] <= 5)	// 0.018 * 255
				ptr[cIdx] = (unsigned short)qRound(ptr[cIdx] * (double)iProcessor.imgdata.params.gamm[1] / 255.0);
			else
				ptr[cIdx] = gammaLookup[ptr[cIdx]];
		}
	}

}

void DkRawLoader::reduceColorNoise(const LibRaw & iProcessor, cv::Mat & img) const {

	// filter color noise with a median filter
	float isoSpeed = iProcessor.imgdata.other.iso_speed;

	if (isoSpeed > 0) {

		DkTimer dt;

		int winSize;
		if (isoSpeed > 6400) 
			winSize = 13;
		else if (isoSpeed >= 3200) 
			winSize = 11;
		else if (isoSpeed >= 2500) 
			winSize = 9;
		else if (isoSpeed >= 400) 
			winSize = 7;
		else 
			winSize = 5;

		DkTimer dMed;
		
		// revert back to 8-bit image
		img.convertTo(img, CV_8U);

		cv::cvtColor(img, img, CV_RGB2YCrCb);

		std::vector<cv::Mat> imgCh;
		cv::split(img, imgCh);
		assert(imgCh.size() == 3);

		cv::medianBlur(imgCh[1], imgCh[1], winSize);
		cv::medianBlur(imgCh[2], imgCh[2], winSize);

		cv::merge(imgCh, img);
		cv::cvtColor(img, img, CV_YCrCb2RGB);
		qDebug() << "median blur takes:" << dt;
	}

}

QImage DkRawLoader::raw2Img(const LibRaw & iProcessor, cv::Mat & img) const {
	
	//check the pixel aspect ratio of the raw image
	if (iProcessor.imgdata.sizes.pixel_aspect != 1.0f)
		cv::resize(img, img, cv::Size(), (double)iProcessor.imgdata.sizes.pixel_aspect, 1.0f);
	
	// revert back to 8-bit image
	img.convertTo(img, CV_8U);

	// TODO: for now - fix this!
	if (img.channels() == 1)
		cv::cvtColor(img, img, CV_GRAY2RGB);


	return DkImage::mat2QImage(img);
}

#endif

// -------------------------------------------------------------------- DkTgaLoader 
namespace tga {
	
	DkTgaLoader::DkTgaLoader(QSharedPointer<QByteArray> ba) {

		mBa = ba;
	}

	QImage DkTgaLoader::image() const {
		return mImg;
	}

	bool DkTgaLoader::load() {
		
		if (!mBa || mBa->isEmpty())
			return false;
		
		return load(mBa);
	}

	bool DkTgaLoader::load(QSharedPointer<QByteArray> ba) {
		

		// this code is from: http://www.paulbourke.net/dataformats/tga/
		// thanks!
		Header header;

		const char* dataC = ba->data();

		/* Display the header fields */
		header.idlength = *dataC; dataC++;
		header.colourmaptype = *dataC; dataC++;
		header.datatypecode = *dataC; dataC++;

		const short* dataS = (const short*)dataC;

		header.colourmaporigin = *dataS; dataS++;
		header.colourmaplength = *dataS; dataS++;
		dataC = (const char*)dataS;
		header.colourmapdepth = *dataC; dataC++;
		dataS = (const short*)dataC;
		header.x_origin = *dataS; dataS++;
		header.y_origin = *dataS; dataS++;
		header.width = *dataS; dataS++;
		header.height = *dataS; dataS++;
		dataC = (const char*)dataS;
		header.bitsperpixel = *dataC; dataC++;
		header.imagedescriptor = *dataC; dataC++;

#ifdef _DEBUG
		qDebug() << "TGA Header ------------------------------";
		qDebug() << "ID length:         " << (int)header.idlength;
		qDebug() << "Colourmap type:    " << (int)header.colourmaptype;
		qDebug() << "Image type:        " << (int)header.datatypecode;
		qDebug() << "Colour map offset: " << header.colourmaporigin;
		qDebug() << "Colour map length: " << header.colourmaplength;
		qDebug() << "Colour map depth:  " << (int)header.colourmapdepth;
		qDebug() << "X origin:          " << header.x_origin;
		qDebug() << "Y origin:          " << header.y_origin;
		qDebug() << "Width:             " << header.width;
		qDebug() << "Height:            " << header.height;
		qDebug() << "Bits per pixel:    " << (int)header.bitsperpixel;
		qDebug() << "Descriptor:        " << (int)header.imagedescriptor;
#endif

		/* What can we handle */
		if (header.datatypecode != 2 && header.datatypecode != 10) {
			qWarning() << "Can only handle image type 2 and 10";
			return false;
		}
		
		if (header.bitsperpixel != 16 &&
			header.bitsperpixel != 24 && 
			header.bitsperpixel != 32) {
			qWarning() << "Can only handle pixel depths of 16, 24, and 32";
			return false;
		}

		if (header.colourmaptype != 0 && header.colourmaptype != 1) {
			qWarning() << "Can only handle colour map types of 0 and 1";
			return false;
		}

		Pixel *pixels = new Pixel[header.width*header.height * sizeof(Pixel)];

		if (!pixels) {
			qWarning() << "TGA: could not allocate" << header.width*header.height * sizeof(Pixel)/1024 << "KB";
			return false;
		}

		///* Skip over unnecessary stuff */
		int skipover = header.idlength;
		skipover += header.colourmaptype * header.colourmaplength;
		dataC += skipover;
		
		/* Read the image */
		int bytes2read = header.bitsperpixel / 8;	// save?
		unsigned char p[5];
		
		for (int n = 0; n < header.width * header.height;) {
			
			if (header.datatypecode == 2) {                     /* Uncompressed */
				
				// TODO: out-of-bounds not checked here...
				for (int bi = 0; bi < bytes2read; bi++, dataC++)
					p[bi] = *dataC;
				
				mergeBytes(&(pixels[n]), p, bytes2read);
				n++;
			}
			else if (header.datatypecode == 10) {             /* Compressed */
				
				for (int bi = 0; bi < bytes2read+1; bi++, dataC++)
					p[bi] = *dataC;

				int j = p[0] & 0x7f;
				mergeBytes(&(pixels[n]), &(p[1]), bytes2read);
				n++;
				if (p[0] & 0x80) {         /* RLE chunk */
					for (int i = 0; i < j; i++) {
						mergeBytes(&(pixels[n]), &(p[1]), bytes2read);
						n++;
					}
				}
				else {                   /* Normal chunk */
					for (int i = 0; i < j; i++) {

						for (int bi = 0; bi < bytes2read; bi++, dataC++)
							p[bi] = *dataC;

						mergeBytes(&(pixels[n]), p, bytes2read);
						n++;
					}
				}
			}
		}

		mImg = QImage((uchar*)pixels, header.width, header.height, QImage::Format_ARGB32);
		mImg = mImg.copy();

		// I somehow expected the 5th bit to be 0x10 -> but Paul seems to have a 0th bit : )
		if (!(header.imagedescriptor & 0x20))
			mImg = mImg.mirrored();

		delete[] pixels;

		return true;
	}

	void DkTgaLoader::mergeBytes(Pixel * pixel, unsigned char * p, int bytes) const {
		
		if (bytes == 4) {
			pixel->r = p[0];
			pixel->g = p[1];
			pixel->b = p[2];
			pixel->a = p[3];
		}
		else if (bytes == 3) {
			pixel->r = p[0];
			pixel->g = p[1];
			pixel->b = p[2];
			pixel->a = 255;
		}
		else if (bytes == 2) {
			pixel->r = (p[0] & 0x1f) << 3;
			pixel->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
			pixel->b = (p[1] & 0x7c) << 1;
			pixel->a = 255;// (p[1] & 0x80);
		}
	}
}

}
