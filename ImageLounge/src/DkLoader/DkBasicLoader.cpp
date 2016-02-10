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

#include <qmath.h>

// quazip
#ifdef WITH_QUAZIP
#include <quazip/JlCompress.h>
#endif

#ifdef WITH_WEBP
#include "webp/decode.h"
#include "webp/encode.h"
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
#include "tif_config.h"	
#endif

//#if defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)
// here we clash (typedef redefinition with different types ('long' vs 'int64_t' (aka 'long long'))) 
// so we simply define our own int64 before including tiffio
#define uint64 uint64_hack_
#define int64 int64_hack_
//#endif // defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)

#include "tiffio.h"

//#if defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)
#undef uint64
#undef int64
//#endif // defined(Q_OS_MAC) || defined(Q_OS_OPENBSD)
#endif

#else
#ifdef Q_OS_WIN
#include <olectl.h>
#pragma comment(lib, "oleaut32.lib")
#endif
#endif

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


	bool imgLoaded = false;
	
	QFileInfo fInfo(filePath);

	if (fInfo.isSymLink())
		mFile = fInfo.symLinkTarget();
	else
		mFile = filePath;
	

	fInfo = QFileInfo(mFile);	// resolved lnk
	QString newSuffix = fInfo.suffix();

	release();

	if (mPageIdxDirty)
		imgLoaded = loadPage();

	// identify raw images:
	//newSuffix.contains(QRegExp("(nef|crw|cr2|arw|rw2|mrw|dng)", Qt::CaseInsensitive)))

	int orientation = -1;

	// this fixes an issue with the new jpg loader
	// Qt considers an orientation of 0 as wrong and fails to load these jpgs
	// however, the old nomacs wrote 0 if the orientation should be cleared
	// so we simply adopt the memory here
	if (loadMetaData && mMetaData) {

		try {
			mMetaData->readMetaData(filePath, ba);

			if (!Settings::param().metaData().ignoreExifOrientation) {
				orientation = mMetaData->getOrientation();
				
				if (orientation == -1) {
					mMetaData->clearOrientation();
					mMetaData->saveMetaData(ba);
					qWarning() << "deleting illegal EXIV orientation: " << orientation;
				}
			}
		}
		catch (...) {}	// ignore if we cannot read the metadata
	}
	else if (!mMetaData) {
		qDebug() << "metaData is NULL!";
	}

	qDebug() << "ba size: " << ba;

	QList<QByteArray> qtFormats = QImageReader::supportedImageFormats();
	QString suf = fInfo.suffix().toLower();

	QImage img;

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
	if (!imgLoaded && qtFormats.contains(suf.toStdString().c_str())) {

		// if image has Indexed8 + alpha channel -> we crash... sorry for that
		if (!ba || ba->isEmpty())
			imgLoaded = img.load(mFile, suf.toStdString().c_str());
		else
			imgLoaded = img.loadFromData(*ba.data(), suf.toStdString().c_str());	// toStdString() in order get 1 byte per char

		if (imgLoaded) mLoader = qt_loader;
	}

	// PSD loader
	if (!imgLoaded) {

		imgLoaded = loadPSDFile(mFile, ba);
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
		imgLoaded = loadRawFile(mFile, ba, fast);
		if (imgLoaded) mLoader = raw_loader;
	}

	// default Qt loader
	if (!imgLoaded && !newSuffix.contains(QRegExp("(roh)", Qt::CaseInsensitive))) {

		// if we first load files to buffers, we can additionally load images with wrong extensions (rainer bugfix : )
		// TODO: add warning here
		QByteArray lba;
		loadFileToBuffer(mFile, lba);
		imgLoaded = img.loadFromData(lba);
		
		qDebug() << "lba size: " << lba.size();

		if (imgLoaded) mLoader = qt_loader;
	}  

	// this loader is a bit buggy -> be carefull
	if (!imgLoaded && newSuffix.contains(QRegExp("(roh)", Qt::CaseInsensitive))) {
		
		imgLoaded = loadRohFile(mFile, ba);
		if (imgLoaded) mLoader = roh_loader;

	} 

	// this loader is for OpenCV cascade training files
	if (!imgLoaded && newSuffix.contains(QRegExp("(vec)", Qt::CaseInsensitive))) {

		imgLoaded = loadOpenCVVecFile(mFile, ba);
		if (imgLoaded) mLoader = roh_loader;

	} 

	//if (!imgLoaded && (training || file.suffix().contains(QRegExp("(hdr)", Qt::CaseInsensitive)))) {

	//	// load hdr here...
	//	if (imgLoaded) loader = hdr_loader;
	//} 

	// tiff things
	if (imgLoaded && !mPageIdxDirty)
		indexPages(mFile);
	mPageIdxDirty = false;

	if (imgLoaded && loadMetaData && mMetaData) {
		
		try {
			mMetaData->setQtValues(img);
		
			if (orientation != -1 && !mMetaData->isTiff() && !Settings::param().metaData().ignoreExifOrientation)
				img = rotate(img, orientation);

		} catch(...) {}	// ignore if we cannot read the metadata
	}
	else if (!mMetaData) {
		qDebug() << "metaData is NULL!";
	}

	if (imgLoaded)
		setEditImage(img, tr("Original Image"));

	//qDebug() << qImg.text();

	return imgLoaded;
}

/**
 * Loads special RAW files that are generated by the Hamamatsu camera.
 * @param fileName the filename of the file to be loaded.
 * @return bool true if the file could be loaded.
 **/ 
bool DkBasicLoader::loadRohFile(const QString& filePath, QSharedPointer<QByteArray> ba) {

	if (!ba)
		ba = loadFileToBuffer(filePath);
	if (!ba || ba->isEmpty())
		return false;

	bool imgLoaded = false;

	int rohW = 4000;
	int rohH = 2672;
	unsigned char fByte;	// first byte
	unsigned char sByte;	// second byte
	QImage img;

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

	if (imgLoaded) {
		setEditImage(img, tr("Original Image"));
	}

	return imgLoaded;
}

/**
 * Loads the RAW file specified.
 * Note: nomacs needs to be compiled with OpenCV and LibRaw in
 * order to enable RAW file loading.
 * @param ba the file loaded into a bytearray.
 * @return bool true if the file could be loaded.
 **/ 
bool DkBasicLoader::loadRawFile(const QString& filePath, QSharedPointer<QByteArray> ba, bool fast) {
	
	bool imgLoaded = false;

	DkTimer dt;
	QImage img;

	try {

		// try to get preview image from exiv2
		if (mMetaData) {
			if (fast || Settings::param().resources().loadRawThumb == DkSettings::raw_thumb_always ||
				Settings::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large) {

				mMetaData->readMetaData(filePath, ba);

				int minWidth = 0;

#ifdef WITH_LIBRAW	// if nomacs has libraw - we can still hope for a fallback -> otherwise try whatever we have here
				if (Settings::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large)
					minWidth = 1920;
#endif
				img = mMetaData->getPreviewImage(minWidth);

				if (!img.isNull()) {
					
					setEditImage(img, tr("Original Image"));
					qDebug() << "[RAW] loaded with exiv2";
					return true;
				}
			}
		}
#ifdef WITH_LIBRAW

		LibRaw iProcessor;
		QImage image;

		int error = LIBRAW_DATA_ERROR;

		//use iprocessor from libraw to read the data
		// OK - so LibRaw 0.17 cannot identify iiq files in the buffer - so we load them from the file
		if (QFileInfo(filePath).suffix().contains("iiq", Qt::CaseInsensitive) || !ba || ba->isEmpty()) {
			error = iProcessor.open_file(filePath.toStdString().c_str());
		}
		else {
			// the buffer check is because:
			// libraw has an error when loading buffers if the first 4 bytes encode as 'RIFF'
			// and no data follows at all
			if (ba->isEmpty() || ba->size() < 100)
				return false;

			error = iProcessor.open_buffer((void*)ba->constData(), ba->size());
		}

		if (error != LIBRAW_SUCCESS)
			return false;

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
		int tM = qMax(iProcessor.imgdata.thumbnail.twidth, iProcessor.imgdata.thumbnail.twidth);
		// TODO: check actual screen resolution
		qDebug() << "max thumb size: " << tM;

		if (fast || Settings::param().resources().loadRawThumb == DkSettings::raw_thumb_always ||
			(Settings::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large && tM >= 1920)) {

			// crashes here if image is broken
			int err = iProcessor.unpack_thumb();
			char* tPtr = iProcessor.imgdata.thumbnail.thumb;

			if (!err && tPtr) {

				img.loadFromData((const uchar*)tPtr, iProcessor.imgdata.thumbnail.tlength);

				if (!img.isNull()) {
					imgLoaded = true;
					setEditImage(img, tr("Original Image"));
					qDebug() << "[RAW] I loaded the RAW's thumbnail";

					return imgLoaded;
				}
				else
					qDebug() << "qt could not load the thumb";
			}
			else
				qDebug() << "error unpacking the thumb...";
		}

		qDebug() << "[RAW] loading full raw file";


		//unpack the data
		error = iProcessor.unpack();
		if (std::strcmp(iProcessor.version(), "0.13.5") != 0)	// fixes a bug specific to libraw 13 - version call is UNTESTED
			iProcessor.raw2image();

		if (error != LIBRAW_SUCCESS)
			return false;

		//iProcessor.dcraw_process();
		//iProcessor.dcraw_ppm_tiff_writer("test.tiff");

		unsigned short cols = iProcessor.imgdata.sizes.width,//.raw_width,
			rows = iProcessor.imgdata.sizes.height;//.raw_height;

		cv::Mat rawMat, rgbImg;

		// modifications sequence for changing from raw to rgb:
		// 1. normalize according to black point and dynamic range
		// 2. demosaic
		// 3. white balance
		// 4. color correction
		// 5. gamma correction

		//GENERAL TODO
		//check if the corrections (black, white point gamma correction) are done in the correct order
		//check if the specific corrections are different regarding different camera models
		//find out some general specifications of the most important raw formats

		//qDebug() << "----------------";
		//qDebug() << "Bayer Pattern: " << QString::fromStdString(iProcessor.imgdata.idata.cdesc);
		//qDebug() << "Camera manufacturer: " << QString::fromStdString(iProcessor.imgdata.idata.make);
		//qDebug() << "Camera model: " << QString::fromStdString(iProcessor.imgdata.idata.model);
		//qDebug() << "canon_ev " << (float)iProcessor.imgdata.color.canon_ev;

		//debug outputs of the exif data read by libraw
		//qDebug() << "white: [%.3f %.3f %.3f %.3f]\n", iProcessor.imgdata.color.cam_mul[0],
		//	iProcessor.imgdata.color.cam_mul[1], iProcessor.imgdata.color.cam_mul[2],
		//	iProcessor.imgdata.color.cam_mul[3]);
		//qDebug() << "black: %i\n", iProcessor.imgdata.color.black);
		//qDebug() << "maximum: %.i %i\n", iProcessor.imgdata.color.maximum,
		//	iProcessor.imgdata.params.adjust_maximum_thr);
		//qDebug() << "gamma: %.3f %.3f %.3f %.3f %.3f %.3f\n",
		//	iProcessor.imgdata.params.gamm[0],
		//	iProcessor.imgdata.params.gamm[1],
		//	iProcessor.imgdata.params.gamm[2],
		//	iProcessor.imgdata.params.gamm[3],
		//	iProcessor.imgdata.params.gamm[4],
		//	iProcessor.imgdata.params.gamm[5]);

		//qDebug() << "----------------";

		if (strcmp(iProcessor.imgdata.idata.cdesc, "RGBG")) {
			qWarning() << "Wrong Bayer Pattern (not RGBG)\n";
			return false;
		}

		// 1. read raw image and normalize it according to dynamic range and black point

		//dynamic range is defined by maximum - black
		float dynamicRange = (float)(iProcessor.imgdata.color.maximum - iProcessor.imgdata.color.black);	// iProcessor.imgdata.color.channel_maximum[0]-iProcessor.imgdata.color.black;	// dynamic range

		if (iProcessor.imgdata.idata.filters) {

			rawMat = cv::Mat(rows, cols, CV_32FC1);

			for (uint row = 0; row < rows; row++) {
				float *ptrRaw = rawMat.ptr<float>(row);

				for (uint col = 0; col < cols; col++) {

					int colorIdx = iProcessor.COLOR(row, col);
					ptrRaw[col] = (float)(iProcessor.imgdata.image[cols*(row)+col][colorIdx]);

					//correct the image values according the black point defined by the camera
					ptrRaw[col] -= iProcessor.imgdata.color.black;
					//normalize according the dynamic range
					ptrRaw[col] /= dynamicRange;
					ptrRaw[col] *= 65535;  // for conversion to 16U
				}
			}

			// 2. demosaic raw image
			rawMat.convertTo(rawMat, CV_16U);

			//cvtColor(rawMat, rgbImg, CV_BayerBG2RGB);
			unsigned long type = (unsigned long)iProcessor.imgdata.idata.filters;
			type = type & 255;

			//define bayer pattern
			if (type == 180) cvtColor(rawMat, rgbImg, CV_BayerBG2RGB);      //bitmask  10 11 01 00  -> 3(G) 2(B) 1(G) 0(R) -> RG RG RG
			//												                                                                  GB GB GB
			else if (type == 30) cvtColor(rawMat, rgbImg, CV_BayerRG2RGB);		//bitmask  00 01 11 10	-> 0 1 3 2
			else if (type == 225) cvtColor(rawMat, rgbImg, CV_BayerGB2RGB);		//bitmask  11 10 00 01
			else if (type == 75) cvtColor(rawMat, rgbImg, CV_BayerGR2RGB);		//bitmask  01 00 10 11
			else {
				qWarning() << "Wrong Bayer Pattern (not BG, RG, GB, GR)\n";
				return false;
			}
		}
		else {

			rawMat = cv::Mat(rows, cols, CV_32FC3);
			rawMat.setTo(0);
			std::vector<cv::Mat> rawCh;
			split(rawMat, rawCh);

			for (unsigned int row = 0; row < rows; row++) {
				float *ptrR = rawCh[0].ptr<float>(row);
				float *ptrG = rawCh[1].ptr<float>(row);
				float *ptrB = rawCh[2].ptr<float>(row);
				//float *ptrE = rawCh[3].ptr<float>(row);

				for (unsigned int col = 0; col < cols; col++) {

					ptrR[col] = (float)(iProcessor.imgdata.image[cols*(row)+col][0]);
					ptrR[col] -= iProcessor.imgdata.color.black;
					ptrR[col] /= dynamicRange;
					ptrR[col] *= 65535;  // for conversion to 16U

					ptrG[col] = (float)(iProcessor.imgdata.image[cols*(row)+col][1]);
					ptrG[col] -= iProcessor.imgdata.color.black;
					ptrG[col] /= dynamicRange;
					ptrG[col] *= 65535;  // for conversion to 16U

					ptrB[col] = (float)(iProcessor.imgdata.image[cols*(row)+col][2]);
					ptrB[col] -= iProcessor.imgdata.color.black;
					ptrB[col] /= dynamicRange;
					ptrB[col] *= 65535;  // for conversion to 16U

				}
			}
			merge(rawCh, rgbImg);
			rgbImg.convertTo(rgbImg, CV_16U);
		}

		rawMat.release();

		// 3.. 4., 5.: apply white balance, color correction and gamma 

		// get color correction matrix
		float colorCorrMat[3][4] = {};
		for (int i = 0; i < 3; i++) for (int j = 0; j < 4; j++) colorCorrMat[i][j] = iProcessor.imgdata.color.rgb_cam[i][j];

		// get camera white balance multipliers
		float mulWhite[4];
		mulWhite[0] = iProcessor.imgdata.color.cam_mul[0];
		mulWhite[1] = iProcessor.imgdata.color.cam_mul[1];
		mulWhite[2] = iProcessor.imgdata.color.cam_mul[2];
		mulWhite[3] = iProcessor.imgdata.color.cam_mul[3];

		//read gamma value and create gamma table
		float gamma = (float)iProcessor.imgdata.params.gamm[0];///(float)iProcessor.imgdata.params.gamm[1];
		float gammaTable[65536];
		for (int i = 0; i < 65536; i++) {
			gammaTable[i] = (float)(1.099f*pow((float)i / 65535.0f, gamma) - 0.099f);
		}

		// normalize white balance multipliers
		float w = (mulWhite[0] + mulWhite[1] + mulWhite[2] + mulWhite[3]) / 4.0f;
		float maxW = 1.0f;//mulWhite[0];

		//clipping according the camera model
		//if w > 2.0 maxW is 256, otherwise 512
		//tested empirically
		//check if it can be defined by some metadata settings?
		if (w > 2.0f)
			maxW = 256.0f;
		if (w > 2.0f && QString(iProcessor.imgdata.idata.make).compare("Canon", Qt::CaseInsensitive) == 0)
			maxW = 512.0f;	// some cameras would even need ~800 - why?

		//normalize white point
		mulWhite[0] /= maxW;
		mulWhite[1] /= maxW;
		mulWhite[2] /= maxW;
		mulWhite[3] /= maxW;

		if (mulWhite[3] == 0)
			mulWhite[3] = mulWhite[1];

		//apply corrections
		std::vector<cv::Mat> corrCh;
		split(rgbImg, corrCh);

		for (uint row = 0; row < rows; row++)
		{
			unsigned short *ptrR = corrCh[0].ptr<unsigned short>(row);
			unsigned short *ptrG = corrCh[1].ptr<unsigned short>(row);
			unsigned short *ptrB = corrCh[2].ptr<unsigned short>(row);

			for (uint col = 0; col < cols; col++)
			{
				//apply white balance correction
				int tempR = qRound(ptrR[col] * mulWhite[0]);
				int tempG = qRound(ptrG[col] * mulWhite[1]);
				int tempB = qRound(ptrB[col] * mulWhite[2]);

				//apply color correction					
				int corrR = qRound(colorCorrMat[0][0] * tempR + colorCorrMat[0][1] * tempG + colorCorrMat[0][2] * tempB);
				int corrG = qRound(colorCorrMat[1][0] * tempR + colorCorrMat[1][1] * tempG + colorCorrMat[1][2] * tempB);
				int corrB = qRound(colorCorrMat[2][0] * tempR + colorCorrMat[2][1] * tempG + colorCorrMat[2][2] * tempB);
				// without color correction: change above three lines to the bottom ones
				//int corrR = tempR;
				//int corrG = tempG;
				//int corrB = tempB;

				//clipping
				ptrR[col] = (corrR > 65535) ? 65535 : (corrR < 0) ? 0 : (unsigned short)corrR;
				ptrG[col] = (corrG > 65535) ? 65535 : (corrG < 0) ? 0 : (unsigned short)corrG;
				ptrB[col] = (corrB > 65535) ? 65535 : (corrB < 0) ? 0 : (unsigned short)corrB;

				//apply gamma correction
				ptrR[col] = ptrR[col] <= 0.018f * 65535.0f ? (unsigned short)(ptrR[col] * (float)iProcessor.imgdata.params.gamm[1] / 257.0f) :
					(unsigned short)(gammaTable[ptrR[col]] * 255);
				//									(1.099f*(float)(pow((float)ptrRaw[col], gamma))-0.099f);
				ptrG[col] = ptrG[col] <= 0.018f * 65535.0f ? (unsigned short)(ptrG[col] * (float)iProcessor.imgdata.params.gamm[1] / 257.0f) :
					(unsigned short)(gammaTable[ptrG[col]] * 255);
				ptrB[col] = ptrB[col] <= 0.018f * 65535.0f ? (unsigned short)(ptrB[col] * (float)iProcessor.imgdata.params.gamm[1] / 257.0f) :
					(unsigned short)(gammaTable[ptrB[col]] * 255);
			}
		}

		merge(corrCh, rgbImg);
		rgbImg.convertTo(rgbImg, CV_8U);

		// filter color noise withe a median filter
		if (Settings::param().resources().filterRawImages) {

			float isoSpeed = iProcessor.imgdata.other.iso_speed;

			if (isoSpeed > 0) {

				int winSize;
				if (isoSpeed > 6400) winSize = 13;
				else if (isoSpeed >= 3200) winSize = 11;
				else if (isoSpeed >= 2500) winSize = 9;
				else if (isoSpeed >= 400) winSize = 7;
				else winSize = 5;

				DkTimer dMed;

				cvtColor(rgbImg, rgbImg, CV_RGB2YCrCb);
				split(rgbImg, corrCh);

				cv::medianBlur(corrCh[1], corrCh[1], winSize);
				cv::medianBlur(corrCh[2], corrCh[2], winSize);

				merge(corrCh, rgbImg);
				cvtColor(rgbImg, rgbImg, CV_YCrCb2RGB);

				qDebug() << "median blurred in: " << dMed.getTotal() << ", winSize: " << winSize;
			}
			else
				qDebug() << "median filter: unrecognizable ISO speed";

		}

		//check the pixel aspect ratio of the raw image
		if (iProcessor.imgdata.sizes.pixel_aspect != 1.0f) {
			cv::resize(rgbImg, rawMat, cv::Size(), (double)iProcessor.imgdata.sizes.pixel_aspect, 1.0f);
			rgbImg = rawMat;
		}

		//create the final image
		image = QImage(rgbImg.data, (int)rgbImg.cols, (int)rgbImg.rows, (int)rgbImg.step/*rgbImg.cols*3*/, QImage::Format_RGB888);
		img = image.copy();
		imgLoaded = true;

		iProcessor.recycle();

#else
		qDebug() << "Not compiled using OpenCV - could not load any RAW image";
#endif
	}
	catch (...) {
		qWarning() << "Exception caught during RAW loading...";
	}

	if (imgLoaded) {
		qDebug() << "[RAW] image loaded from RAW in: " << dt.getTotal();
		setEditImage(img, tr("Original Image"));
	}

	return imgLoaded;
}

#ifdef Q_OS_WIN
bool DkBasicLoader::loadPSDFile(const QString&, QSharedPointer<QByteArray>) {
#else
bool DkBasicLoader::loadPSDFile(const QString& filePath, QSharedPointer<QByteArray> ba) {

	// load from file?
	if (!ba || ba->isEmpty()) {
		QFile file(filePath);
		file.open(QIODevice::ReadOnly);

		QPsdHandler psdHandler;
		psdHandler.setDevice(&file);	// QFile is an IODevice
		//psdHandler.setFormat(fileInfo.suffix().toLocal8Bit());

		if (psdHandler.canRead(&file)) {
			QImage img;
			bool success = psdHandler.read(&img);
			setEditImage(img, tr("Original Image"));
			
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
			QImage img;
			bool success = psdHandler.read(&img);
			setEditImage(img, tr("Original Image"));

			return success;
		}
	}

#endif // !Q_OS_WIN
	return false;
}

void DkBasicLoader::setImage(const QImage & img, const QString & editName, const QString & file) {

	mFile = file;
	setEditImage(img, editName);
};

void DkBasicLoader::setEditImage(const QImage& img, const QString& editName) {

	if (img.isNull())
		return;

	// delete all hidden edit states
	for (int idx = mImages.size() - 1; idx > mImageIndex; idx--)
		mImages.pop_back();

	// compute new history size
	int historySize = 0;
	for (const DkEditImage& e : mImages) {
		historySize += e.size();
	}

	DkEditImage newImg(img, editName);

	if (historySize + newImg.size() > Settings::param().resources().historyMemory && mImages.size() > 2) {
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

int DkBasicLoader::historyIndex() const {
	return mImageIndex;
}

void DkBasicLoader::setHistoryIndex(int idx) {
	mImageIndex = idx;
}

void DkBasicLoader::loadFileToBuffer(const QString& fileInfo, QByteArray& ba) const {

#ifdef WITH_QUAZIP
	if (QFileInfo(fileInfo).dir().path().contains(DkZipContainer::zipMarker())) 
		DkZipContainer::extractImage(DkZipContainer::decodeZipFile(fileInfo), DkZipContainer::decodeImageFile(fileInfo), ba);
#endif
	
	QFile file(fileInfo);
	file.open(QIODevice::ReadOnly);

	ba = file.readAll();
}

QSharedPointer<QByteArray> DkBasicLoader::loadFileToBuffer(const QString& fileInfo) const {

#ifdef WITH_QUAZIP
	if (QFileInfo(fileInfo).dir().path().contains(DkZipContainer::zipMarker())) 
		return DkZipContainer::extractImage(DkZipContainer::decodeZipFile(fileInfo), DkZipContainer::decodeImageFile(fileInfo));
#endif

	QFile file(fileInfo);
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

void DkBasicLoader::indexPages(const QString& filePath) {

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
	TIFF* tiff = TIFFOpen(filePath.toLatin1(), "r");	// this->mFile was here before - not sure why

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

	qDebug() << dircount << " TIFF directories... " << dt.getTotal();
	TIFFClose(tiff);

	TIFFSetWarningHandler(oldWarningHandler);
	TIFFSetWarningHandler(oldErrorHandler);
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

void DkBasicLoader::convert32BitOrder(void *buffer, int width) {

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
#endif
}

QString DkBasicLoader::save(const QString& filePath, const QImage& img, int compression) {

	QSharedPointer<QByteArray> ba;

	qDebug() << "saving: " << filePath;

	if (saveToBuffer(filePath, img, ba, compression) && ba) {

		if (writeBufferToFile(filePath, ba))
			return filePath;
	}

	return QString();
}

bool DkBasicLoader::saveToBuffer(const QString& filePath, const QImage& img, QSharedPointer<QByteArray>& ba, int compression) {

	if (!ba) 
		ba = QSharedPointer<QByteArray>(new QByteArray());

	bool saved = false;

	QFileInfo fInfo(filePath);
	qDebug() << "extension: " << fInfo.suffix();


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
		if (!hasAlpha && img.colorTable().empty() && !fInfo.suffix().contains(QRegExp("(j2k|jp2|jpf|jpx|png)")))
			sImg = sImg.convertToFormat(QImage::Format_RGB888);
		else if (fInfo.suffix().contains(QRegExp("(j2k|jp2|jpf|jpx)")) && sImg.depth() != 32 && sImg.depth() != 8)
			sImg = sImg.convertToFormat(QImage::Format_RGB32);

		qDebug() << "img has alpha: " << (sImg.format() != QImage::Format_RGB888) << " img uses alpha: " << hasAlpha;

		QBuffer fileBuffer(ba.data());
		fileBuffer.open(QIODevice::WriteOnly);
		QImageWriter* imgWriter = new QImageWriter(&fileBuffer, fInfo.suffix().toStdString().c_str());
		imgWriter->setCompression(compression);
		imgWriter->setQuality(compression);
#if QT_VERSION >= 0x050500
		imgWriter->setOptimizedWrite(true);			// this saves space TODO: user option here?
		imgWriter->setProgressiveScanWrite(true);
#endif
		saved = imgWriter->write(sImg);
		delete imgWriter;
	}

	if (saved && mMetaData) {
		
		if (!mMetaData->isLoaded() || !mMetaData->hasMetaData())
			mMetaData->readMetaData(filePath, ba);

		if (mMetaData->isLoaded()) {
			try {
				mMetaData->updateImageMetaData(img);
				mMetaData->saveMetaData(ba, true);
				//metaData->printMetaData();	// debug
			} 
			catch (...) {
				// is it still throwing anything?
				qDebug() << "Sorry, I could not save the meta data...";
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

	if (ba->isEmpty() && mMetaData->isDirty())
		ba = loadFileToBuffer(filePath);

	bool saved = false;
	try {
		saved = mMetaData->saveMetaData(ba);
	} 
	catch(...) {
	}
	
	if (saved)
		writeBufferToFile(filePath, ba);

}

bool DkBasicLoader::isContainer(const QString& filePath) {

	QFileInfo fInfo(filePath);
	if (!fInfo.isFile() || !fInfo.exists())
		return false;

	QString suffix = fInfo.suffix();

	for (int idx = 0; idx < Settings::param().app().containerFilters.size(); idx++) {

		if (Settings::param().app().containerFilters[idx].contains(suffix))
			return true;
	}

	return false;
}

// image editing --------------------------------------------------------------------
/**
 * This method rotates an image.
 * @param orientation the orientation in degree.
 **/ 
QImage DkBasicLoader::rotate(const QImage& img, int orientation) {

	if (orientation == 0 || orientation == -1)
		return img;

	QTransform rotationMatrix;
	rotationMatrix.rotate((double)orientation);
	QImage rImg = img.transformed(rotationMatrix);

	return rImg;
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

#ifdef WITH_WEBP

bool DkBasicLoader::loadWebPFile(const QString& filePath, QSharedPointer<QByteArray> ba) {

	if (!ba || ba->isEmpty())
		ba = loadFileToBuffer(filePath);
	if (ba->isEmpty())
		return false;

	// retrieve the image features (size, alpha etc.)
	WebPBitstreamFeatures features;
	int error = WebPGetFeatures((const uint8_t*)ba->data(), ba->size(), &features);
	if (error) 
		return false;

	uint8_t* webData = 0;
	QImage img;

	if (features.has_alpha) {
		webData = WebPDecodeBGRA((const uint8_t*) ba->data(), ba->size(), &features.width, &features.height);
		if (!webData) return false;
		img = QImage(webData, (int)features.width, (int)features.height, QImage::Format_ARGB32);
	}
	else {
		webData = WebPDecodeRGB((const uint8_t*) ba->data(), ba->size(), &features.width, &features.height);
		if (!webData) return false;
		img = QImage(webData, (int)features.width, (int)features.height, features.width*3, QImage::Format_RGB888);
	}

	// clone the image so we own the buffer
	img = img.copy();
	if (webData) 
		free(webData);

	if (!img.isNull())
		setEditImage(img, tr("Original Image"));

	return true;
}

bool DkBasicLoader::saveWebPFile(const QString& filePath, const QImage& img, int compression) {
	
	qDebug() << "format: " << img.format();

	QSharedPointer<QByteArray> ba;

	if (saveWebPFile(img, ba, compression) && ba && !ba->isEmpty()) {

		writeBufferToFile(filePath, ba);
		return true;
	}

	return false;
}

bool DkBasicLoader::saveWebPFile(const QImage& img, QSharedPointer<QByteArray>& ba, int compression, int speed) {

	if (!ba)
		ba = QSharedPointer<QByteArray>(new QByteArray());

	QImage sImg;

	bool hasAlpha = DkImage::alphaChannelUsed(img);

	// currently, guarantee that the image is a ARGB image
	if (!hasAlpha && img.format() != QImage::Format_RGB888)
		sImg = img.convertToFormat(QImage::Format_RGB888);	// for now
	else 
		sImg = img;

	WebPConfig config;
	bool lossless = false;
	if (compression == -1) {
		compression = 100;
		lossless = true;
	}
	if (!WebPConfigPreset(&config, WEBP_PRESET_PHOTO, (float)compression)) return false;
	if (lossless) config.lossless = 1;
	config.method = speed;

	WebPPicture webImg;
	if (!WebPPictureInit(&webImg)) 
		return false;
	
	webImg.width = sImg.width();
	webImg.height = sImg.height();
	webImg.use_argb = true;		// we never use YUV
	//webImg.argb_stride = img.bytesPerLine();
	//webImg.argb = reinterpret_cast<uint32_t*>(img.bits());

	qDebug() << "speed method: " << config.method;

	int errorCode = 0;

	if (hasAlpha) 
		errorCode = WebPPictureImportBGRA(&webImg, reinterpret_cast<uint8_t*>(sImg.bits()), sImg.bytesPerLine());
	else
		errorCode = WebPPictureImportRGB(&webImg, reinterpret_cast<uint8_t*>(sImg.bits()), sImg.bytesPerLine());

	if (!errorCode)
		qDebug() << "import error: " << errorCode;

	// Set up a byte-writing method (write-to-memory, in this case):
	WebPMemoryWriter writer;
	WebPMemoryWriterInit(&writer);
	webImg.writer = WebPMemoryWrite;
	webImg.custom_ptr = &writer;

	int ok = WebPEncode(&config, &webImg);
	if (!ok || writer.size == 0) return false;

	ba = QSharedPointer<QByteArray>(new QByteArray(reinterpret_cast<const char*>(writer.mem), (int)writer.size));	// does a deep copy
	WebPPictureFree(&webImg);

	return true;
}
#endif

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

	HICON hIcon = DkImage::toWinHICON(QPixmap::fromImage(img));
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

bool DkBasicLoader::loadOpenCVVecFile(const QString& filePath, QSharedPointer<QByteArray> ba, QSize s) {

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

	QImage img = DkImage::mat2QImage(allPatches);
	img = img.convertToFormat(QImage::Format_ARGB32);

	setEditImage(img, tr("Original Image"));

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
FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) : QObject(parent) {
	QNetworkProxyQuery npq(QUrl("http://www.nomacs.org"));
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

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {

	if (pReply->error() != QNetworkReply::NoError) {
		qWarning() << "I could not download: " << mUrl;
		qWarning() << pReply->errorString();
	}

	mDownloadedData = QSharedPointer<QByteArray>(new QByteArray(pReply->readAll()));
	//emit a signal
	pReply->deleteLater();
	emit downloaded();
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

	if(encodedFilePath.contains(mZipMarker)) {
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

}
