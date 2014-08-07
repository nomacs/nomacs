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

namespace nmc {

// Basic loader and image edit class --------------------------------------------------------------------
DkBasicLoader::DkBasicLoader(int mode) {
	this->mode = mode;
	training = false;
	pageIdxDirty = false;
	numPages = 1;
	pageIdx = 1;
	loader = no_loader;

	this->metaData = QSharedPointer<DkMetaDataT>(new DkMetaDataT());
}

bool DkBasicLoader::loadGeneral(const QFileInfo& fileInfo, bool loadMetaData, bool fast) {

	return loadGeneral(fileInfo, QSharedPointer<QByteArray>(), loadMetaData, fast);
}
/**
 * This function loads the images.
 * @param file the image file that should be loaded.
 * @return bool true if the image could be loaded.
 **/ 
bool DkBasicLoader::loadGeneral(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba, bool loadMetaData, bool fast) {

	bool imgLoaded = false;
	
	if (fileInfo.isSymLink())
		file = fileInfo.symLinkTarget();
	else
		file = fileInfo;
	
	QString newSuffix = file.suffix();

	QImage oldImg = qImg;
#ifdef WITH_OPENCV
	cv::Mat oldMat = cvImg;
#endif
	release();

	if (pageIdxDirty)
		imgLoaded = loadPage();

	// identify raw images:
	//newSuffix.contains(QRegExp("(nef|crw|cr2|arw|rw2|mrw|dng)", Qt::CaseInsensitive)))

	QList<QByteArray> qtFormats = QImageReader::supportedImageFormats();
	QString suf = file.suffix().toLower();

	if (!imgLoaded && !file.exists() && ba && !ba->isEmpty()) {
		imgLoaded = qImg.loadFromData(*ba.data());

		if (imgLoaded)
			loader = qt_loader;
	}

	// default Qt loader
	// here we just try those formats that are officially supported
	if (!imgLoaded && qtFormats.contains(suf.toStdString().c_str())) {

		// if image has Indexed8 + alpha channel -> we crash... sorry for that
		if (!ba || ba->isEmpty())
			imgLoaded = qImg.load(file.absoluteFilePath());
		else
			imgLoaded = qImg.loadFromData(*ba.data());

		if (imgLoaded) loader = qt_loader;
	}

	// PSD loader
	if (!imgLoaded) {

		imgLoaded = loadPSDFile(file, ba);
		if (imgLoaded) loader = psd_loader;
	}
	// WEBP loader
	if (!imgLoaded) {

		imgLoaded = loadWebPFile(file, ba);
		if (imgLoaded) loader = webp_loader;
	}

	// RAW loader
	if (!imgLoaded && !qtFormats.contains(suf.toStdString().c_str())) {
		
		// TODO: sometimes (e.g. _DSC6289.tif) strange opencv errors are thrown - catch them!
		// load raw files
		imgLoaded = loadRawFile(file, ba, fast);
		if (imgLoaded) loader = raw_loader;
	}

	// default Qt loader
	if (!imgLoaded && !newSuffix.contains(QRegExp("(roh)", Qt::CaseInsensitive))) {

		// if we first load files to buffers, we can additionally load images with wrong extensions (rainer bugfix : )
		// TODO: add warning here
		QByteArray ba;
		loadFileToBuffer(file, ba);
		imgLoaded = qImg.loadFromData(ba);
		
		if (imgLoaded) loader = qt_loader;
	}  

	// this loader is a bit buggy -> be carefull
	if (!imgLoaded && newSuffix.contains(QRegExp("(roh)", Qt::CaseInsensitive))) {
		
		imgLoaded = loadRohFile(file, ba);
		if (imgLoaded) loader = roh_loader;

	} 

	// this loader is for OpenCV cascade training files
	if (!imgLoaded && newSuffix.contains(QRegExp("(vec)", Qt::CaseInsensitive))) {

		imgLoaded = loadOpenCVVecFile(file, ba);
		if (imgLoaded) loader = roh_loader;

	} 

	//if (!imgLoaded && (training || file.suffix().contains(QRegExp("(hdr)", Qt::CaseInsensitive)))) {

	//	// load hdr here...
	//	if (imgLoaded) loader = hdr_loader;
	//} 

	// ok, play back the old images
	if (!imgLoaded) {
		qImg = oldImg;
#ifdef WITH_OPENCV
		cvImg = oldMat;
#endif
	}

	// tiff things
	if (imgLoaded && !pageIdxDirty)
		indexPages(file);
	pageIdxDirty = false;

	if (imgLoaded && loadMetaData && metaData) {
		
		try {
			metaData->readMetaData(fileInfo, ba);
		
			if (!DkSettings::metaData.ignoreExifOrientation) {
				int orientation = metaData->getOrientation();

				if (!metaData->isTiff() && !DkSettings::metaData.ignoreExifOrientation)
					rotate(orientation);
			}
		} catch(...) {}	// ignore if we cannot read the metadata
	}
	else if (!metaData) {
		qDebug() << "metaData is NULL!";
	}

	return imgLoaded;
}

/**
 * Loads special RAW files that are generated by the Hamamatsu camera.
 * @param fileName the filename of the file to be loaded.
 * @return bool true if the file could be loaded.
 **/ 
bool DkBasicLoader::loadRohFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba) {

	if (!ba)
		ba = loadFileToBuffer(fileInfo);
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

		qImg = QImage(buffer, rohW, rohH, QImage::Format_Indexed8);

		if (qImg.isNull())
			return imgLoaded;
		else
			imgLoaded = true;


		//img = img.copy();
		QVector<QRgb> colorTable;

		for (int i = 0; i < 256; i++)
			colorTable.push_back(QColor(i, i, i).rgb());
		
		qImg.setColorTable(colorTable);

	} catch(...) {
		imgLoaded = false;
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
bool DkBasicLoader::loadRawFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba, bool fast) {
	
	bool imgLoaded = false;

	try {

#ifdef WITH_LIBRAW

		LibRaw iProcessor;
		QImage image;
		int orientation = 0;

		int error = LIBRAW_DATA_ERROR;

		//use iprocessor from libraw to read the data
		if (!ba || ba->isEmpty()) {
			error = iProcessor.open_file(fileInfo.absoluteFilePath().toStdString().c_str());
		}
		else {
			// the buffer check is because:
			// libraw has an error when loading buffers if the first 4 bytes encode as 'RIFF'
			// and no data follows at all
			if (ba->isEmpty() || ba->size() < 100)	
				return false;

			error = iProcessor.open_buffer((void*) ba->constData(), ba->size());
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
				
		if (fast || DkSettings::resources.loadRawThumb == DkSettings::raw_thumb_always ||
			(DkSettings::resources.loadRawThumb == DkSettings::raw_thumb_if_large && tM >= 1920)) {
			
			// crashes here if image is broken
			int err = iProcessor.unpack_thumb();
			char* tPtr = iProcessor.imgdata.thumbnail.thumb;

			if (!err && tPtr) {

				QImage tmp;
				tmp.loadFromData((const uchar*) tPtr, iProcessor.imgdata.thumbnail.tlength);

				if (!tmp.isNull()) {
					imgLoaded = true;
					qImg = tmp;
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
		if (iProcessor.version() != "0.13.5")	// fixes a bug specific to libraw 13 - version call is UNTESTED
			iProcessor.raw2image();

		if (error != LIBRAW_SUCCESS)
			return false;

		//iProcessor.dcraw_process();
		//iProcessor.dcraw_ppm_tiff_writer("test.tiff");

		unsigned short cols = iProcessor.imgdata.sizes.width,//.raw_width,
			rows = iProcessor.imgdata.sizes.height;//.raw_height;

		Mat rawMat, rgbImg;

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

		if (strcmp(iProcessor.imgdata.idata.cdesc, "RGBG")) throw DkException("Wrong Bayer Pattern (not RGBG)\n", __LINE__, __FILE__);

		// 1. read raw image and normalize it according to dynamic range and black point
		
		//dynamic range is defined by maximum - black
		float dynamicRange = iProcessor.imgdata.color.maximum-iProcessor.imgdata.color.black;	// iProcessor.imgdata.color.channel_maximum[0]-iProcessor.imgdata.color.black;	// dynamic range

		if (iProcessor.imgdata.idata.filters) {

			rawMat = Mat(rows, cols, CV_32FC1);

			for (uint row = 0; row < rows; row++) {
				float *ptrRaw = rawMat.ptr<float>(row);

				for (uint col = 0; col < cols; col++) {

					int colorIdx = iProcessor.COLOR(row, col);
					ptrRaw[col] = (float)(iProcessor.imgdata.image[cols*(row) + col][colorIdx]);

					//correct the image values according the black point defined by the camera
					ptrRaw[col] -= iProcessor.imgdata.color.black;
					//normalize according the dynamic range
					ptrRaw[col] /= dynamicRange;
					ptrRaw[col] *= 65535;  // for conversion to 16U
				}
			}
			
			// 2. demosaic raw image
			rawMat.convertTo(rawMat,CV_16U);

			//cvtColor(rawMat, rgbImg, CV_BayerBG2RGB);
			unsigned long type = (unsigned long)iProcessor.imgdata.idata.filters;
			type = type & 255;

			//define bayer pattern
			if (type == 180) cvtColor(rawMat, rgbImg, CV_BayerBG2RGB);      //bitmask  10 11 01 00  -> 3(G) 2(B) 1(G) 0(R) -> RG RG RG
			//												                                                                  GB GB GB
			else if (type == 30) cvtColor(rawMat, rgbImg, CV_BayerRG2RGB);		//bitmask  00 01 11 10	-> 0 1 3 2
			else if (type == 225) cvtColor(rawMat, rgbImg, CV_BayerGB2RGB);		//bitmask  11 10 00 01
			else if (type == 75) cvtColor(rawMat, rgbImg, CV_BayerGR2RGB);		//bitmask  01 00 10 11
			else throw DkException("Wrong Bayer Pattern (not BG, RG, GB, GR)\n", __LINE__, __FILE__);
		}
		else {

			rawMat = Mat(rows, cols, CV_32FC3);
			rawMat.setTo(0);
			std::vector<Mat> rawCh;
			split(rawMat, rawCh);

			for (unsigned int row = 0; row < rows; row++) {
				float *ptrR = rawCh[0].ptr<float>(row);
				float *ptrG = rawCh[1].ptr<float>(row);
				float *ptrB = rawCh[2].ptr<float>(row);
				//float *ptrE = rawCh[3].ptr<float>(row);

				for (unsigned int col = 0; col < cols; col++) {

					int colorIdx = iProcessor.COLOR(row, col);
					ptrR[col] = (float)(iProcessor.imgdata.image[cols*(row) + col][0]);
					ptrR[col] -= iProcessor.imgdata.color.black;
					ptrR[col] /= dynamicRange;
					ptrR[col] *= 65535;  // for conversion to 16U

					ptrG[col] = (float)(iProcessor.imgdata.image[cols*(row) + col][1]);
					ptrG[col] -= iProcessor.imgdata.color.black;
					ptrG[col] /= dynamicRange;
					ptrG[col] *= 65535;  // for conversion to 16U

					ptrB[col] = (float)(iProcessor.imgdata.image[cols*(row) + col][2]);
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
		for(int i=0;i<3;i++) for(int j=0;j<4;j++) colorCorrMat[i][j] = iProcessor.imgdata.color.rgb_cam[i][j];

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
			gammaTable[i] = (float)(1.099f*pow((float)i/65535.0f, gamma)-0.099f);
		}

		// normalize white balance multipliers
		float w = (mulWhite[0] + mulWhite[1] + mulWhite[2] + mulWhite[3])/4.0f;
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

		////DkUtils::printDebug(DK_MODULE, "----------------\n", (float)iProcessor.imgdata.color.maximum);
		////DkUtils::printDebug(DK_MODULE, "Bayer Pattern: %s\n", iProcessor.imgdata.idata.cdesc);
		////DkUtils::printDebug(DK_MODULE, "Camera manufacturer: %s\n", iProcessor.imgdata.idata.make);
		////DkUtils::printDebug(DK_MODULE, "Camera model: %s\n", iProcessor.imgdata.idata.model);
		////DkUtils::printDebug(DK_MODULE, "canon_ev %f\n", (float)iProcessor.imgdata.color.canon_ev);

		////DkUtils::printDebug(DK_MODULE, "white: [%.3f %.3f %.3f %.3f]\n", iProcessor.imgdata.color.cam_mul[0],
		////	iProcessor.imgdata.color.cam_mul[1], iProcessor.imgdata.color.cam_mul[2],
		////	iProcessor.imgdata.color.cam_mul[3]);
		////DkUtils::printDebug(DK_MODULE, "white (processing): [%.3f %.3f %.3f %.3f]\n", mulWhite[0],
		////	mulWhite[1], mulWhite[2],
		////	mulWhite[3]);
		////DkUtils::printDebug(DK_MODULE, "black: %i\n", iProcessor.imgdata.color.black);
		////DkUtils::printDebug(DK_MODULE, "maximum: %.i %i\n", iProcessor.imgdata.color.maximum,
		////	iProcessor.imgdata.params.adjust_maximum_thr);
		////DkUtils::printDebug(DK_MODULE, "----------------\n", (float)iProcessor.imgdata.color.maximum);



		//apply corrections
		std::vector<Mat> corrCh;
		split(rgbImg, corrCh);

		for (uint row = 0; row < rows; row++)
		{
			unsigned short *ptrR = corrCh[0].ptr<unsigned short>(row);
			unsigned short *ptrG = corrCh[1].ptr<unsigned short>(row);
			unsigned short *ptrB = corrCh[2].ptr<unsigned short>(row);

			for (uint col = 0; col < cols; col++)
			{
				//apply white balance correction
				int tempR = ptrR[col] * mulWhite[0];
				int tempG = ptrG[col] * mulWhite[1];
				int tempB = ptrB[col] * mulWhite[2];

				//apply color correction					
				int corrR = colorCorrMat[0][0] * tempR + colorCorrMat[0][1] * tempG  + colorCorrMat[0][2] * tempB;
				int corrG = colorCorrMat[1][0] * tempR + colorCorrMat[1][1] * tempG  + colorCorrMat[1][2] * tempB;
				int corrB = colorCorrMat[2][0] * tempR + colorCorrMat[2][1] * tempG  + colorCorrMat[2][2] * tempB;
				// without color correction: change above three lines to the bottom ones
				//int corrR = tempR;
				//int corrG = tempG;
				//int corrB = tempB;

				//clipping
				ptrR[col] = (corrR > 65535) ? 65535 : (corrR < 0) ? 0 : corrR;
				ptrG[col] = (corrG > 65535) ? 65535 : (corrG < 0) ? 0 : corrG;
				ptrB[col] = (corrB > 65535) ? 65535 : (corrB < 0) ? 0 : corrB;

				//apply gamma correction
				ptrR[col] = ptrR[col] <= 0.018f * 65535.0f ? (unsigned short)(ptrR[col]*(float)iProcessor.imgdata.params.gamm[1]/257.0f) :
					gammaTable[ptrR[col]] * 255;
				//									(1.099f*(float)(pow((float)ptrRaw[col], gamma))-0.099f);
				ptrG[col] = ptrG[col] <= 0.018f * 65535.0f ? (unsigned short)(ptrG[col]*(float)iProcessor.imgdata.params.gamm[1]/257.0f) :
					gammaTable[ptrG[col]] * 255;
				ptrB[col] = ptrB[col] <= 0.018f * 65535.0f ? (unsigned short)(ptrB[col]*(float)iProcessor.imgdata.params.gamm[1]/257.0f) :
					gammaTable[ptrB[col]] * 255;
			}
		}

		merge(corrCh, rgbImg);
		rgbImg.convertTo(rgbImg, CV_8U);
			
		// filter color noise withe a median filter
		if (DkSettings::resources.filterRawImages) {

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
			else qDebug() << "median filter: unrecognizable ISO speed";
			
		}


		//Mat cropMat(rawMat, Rect(1, 1, rawMat.cols-1, rawMat.rows-1));
		//rawMat.release();
		//rawMat = cropMat;
		//rawMat.setTo(0);

		//normalize(rawMat, rawMat, 255, 0, NORM_MINMAX);

		//check the pixel aspect ratio of the raw image
		if (iProcessor.imgdata.sizes.pixel_aspect != 1.0f) {
			cv::resize(rgbImg, rawMat, Size(), (double)iProcessor.imgdata.sizes.pixel_aspect, 1.0f);
			rgbImg = rawMat;
		}

		//create the final image
		image = QImage(rgbImg.data, (int)rgbImg.cols, (int)rgbImg.rows, (int)rgbImg.step/*rgbImg.cols*3*/, QImage::Format_RGB888);

		//orientation is done in loadGeneral with libExiv
		//orientation = iProcessor.imgdata.sizes.flip;
		//switch (orientation) {
		//case 0: orientation = 0; break;
		//case 3: orientation = 180; break;
		//case 5:	orientation = -90; break;
		//case 6: orientation = 90; break;
		//}

		qImg = image.copy();
		//if (orientation!=0) {
		//	QTransform rotationMatrix;
		//	rotationMatrix.rotate((double)orientation);
		//	img = img.transformed(rotationMatrix);
		//}
		imgLoaded = true;

		iProcessor.recycle();

#else
		throw DkException("Not compiled using OpenCV - could not load any RAW image", __LINE__, __FILE__);
		qDebug() << "Not compiled using OpenCV - could not load any RAW image";
#endif
	} catch (...) {
		//// silently ignore, maybe it's not a raw image
		//qWarning() << "failed to load raw image...";
	}

	return imgLoaded;
}

bool DkBasicLoader::loadPSDFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba) {

	// load from file?
	if (!ba || ba->isEmpty()) {
		QFile file(fileInfo.absoluteFilePath());
		file.open(QIODevice::ReadOnly);

		QPsdHandler psdHandler;
		psdHandler.setDevice(&file);	// QFile is an IODevice
		//psdHandler.setFormat(fileInfo.suffix().toLocal8Bit());

		if (psdHandler.canRead(&file))
			return psdHandler.read(&this->qImg);
	}
	else {
	
		QBuffer buffer;
		buffer.setData(*ba.data());
		buffer.open(QIODevice::ReadOnly);

		QPsdHandler psdHandler;
		psdHandler.setDevice(&buffer);	// QFile is an IODevice
		//psdHandler.setFormat(file.suffix().toLocal8Bit());

		if (psdHandler.canRead(&buffer))
			return psdHandler.read(&this->qImg);
	}

	return false;
}

#ifdef WITH_OPENCV

bool DkBasicLoader::loadOpenCVVecFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba, QSize s, int skipHeader) {

	if (!ba)
		ba = QSharedPointer<QByteArray>(new QByteArray());

	// load from file?
	if (ba->isEmpty())
		ba = loadFileToBuffer(fileInfo);

	if (ba->isEmpty())
		return false;


	const int* pData = (const int*)ba->constData();
	int rIdx = 0;
	int fileCount = pData[rIdx]; rIdx++;
	int vecSize = pData[rIdx]; rIdx++;
	rIdx++; // skip two shorts

	int guessedW = 0;
	int guessedH = 0;

	// parse patch size from file
	QString fName = fileInfo.fileName();
	QStringList sections = fName.split(QRegExp("[-\\.]"));	

	for (int idx = 0; idx < sections.size(); idx++) {

		QString tmpSec = sections[idx];
		qDebug() << "section: " << tmpSec;

		int sIdx = tmpSec.indexOf("w");
		if (tmpSec.contains("w"))
			guessedW = tmpSec.remove("w").toInt();
		else if (tmpSec.contains("h"))
			guessedH = tmpSec.remove("h").toInt();
	}

	qDebug() << "patch size from filename: " << guessedW << " x " << guessedH;

	if(vecSize > 0 && !guessedH && !guessedW) {
		guessedW = cvFloor(sqrt((float) vecSize));
		if(guessedW > 0)
			guessedH = vecSize/guessedW;
	}

	if(guessedW <= 0 || guessedH <= 0 || guessedW * guessedH != vecSize) {
		
		// TODO: ask user
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

	const int* dataPtr = (const int*)ba->constData();
	dataPtr += rIdx;
	const unsigned char* imgPtr = (const unsigned char*)dataPtr;

	double nRowsCols = sqrt(numElements);
	int numCols = qCeil(nRowsCols);
	int minusOneRow = (qFloor(nRowsCols) != qCeil(nRowsCols) && nRowsCols - qFloor(nRowsCols) < 0.5) ? 1 : 0;

	cv::Mat allPatches((numCols-minusOneRow)*guessedH, numCols*guessedW, CV_8UC1, Scalar(125));

	for (int idx = 0; idx < numElements; idx++) {

		cv::Mat cPatch = getPatch(imgPtr + vecSize*idx*2+idx+1, QSize(guessedW, guessedH), 0);
		cv::Mat cPatchAll = allPatches(cv::Rect(idx%numCols*guessedW, qFloor(idx/numCols)*guessedH, guessedW, guessedH));

		if (!cPatchAll.empty())
			cPatch.copyTo(cPatchAll);
	}

	this->qImg = DkImage::mat2QImage(allPatches);
	this->qImg = this->qImg.convertToFormat(QImage::Format_ARGB32);

	return true;
}

Mat DkBasicLoader::getPatch(const unsigned char* dataPtr, QSize patchSize, int mod2) const {
	
	cv::Mat img(patchSize.height(), patchSize.width(), CV_16UC1, (void*)dataPtr);
	cv::Mat img8U(patchSize.height(), patchSize.width(), CV_8UC1, Scalar(0));

	// ok, take just the second byte
	for (int rIdx = 0; rIdx < img8U.rows; rIdx++) {

		unsigned char* ptr8U = img8U.ptr<unsigned char>(rIdx);
		unsigned char* ptr16U = img.ptr<unsigned char>(rIdx);

		for (int cIdx = 0; cIdx < img8U.cols; cIdx++) {
			ptr8U[cIdx] = ptr16U[cIdx*2];
		}
	}

	return img8U;
}
#endif

void DkBasicLoader::loadFileToBuffer(const QFileInfo& fileInfo, QByteArray& ba) {

	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	ba = file.readAll();
}

QSharedPointer<QByteArray> DkBasicLoader::loadFileToBuffer(const QFileInfo& fileInfo) {

	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	QSharedPointer<QByteArray> ba(new QByteArray(file.readAll()));
	file.close();

	return ba;
}

bool DkBasicLoader::writeBufferToFile(const QFileInfo& fileInfo, const QSharedPointer<QByteArray> ba) const {

	if (!ba || ba->isEmpty())
		return false;

	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::WriteOnly);
	qint64 bytesWritten = file.write(*ba.data(), ba->size());
	file.close();
	qDebug() << "[DkBasicLoader] buffer saved, bytes written: " << bytesWritten;

	if (!bytesWritten || bytesWritten == -1)
		return false;

	return true;
}

void DkBasicLoader::indexPages(const QFileInfo& fileInfo) {

	// reset counters
	numPages = 1;
	pageIdx = 1;

#ifdef WITH_LIBTIFF

	// for now we just support tiff's
	if (!fileInfo.suffix().contains(QRegExp("(tif|tiff)", Qt::CaseInsensitive)))
		return;

	// first turn off nasty warning/error dialogs - (we do the GUI : )
	TIFFErrorHandler oldErrorHandler, oldWarningHandler;
	oldWarningHandler = TIFFSetWarningHandler(NULL);
	oldErrorHandler = TIFFSetErrorHandler(NULL); 

	DkTimer dt;
	TIFF* tiff = TIFFOpen(this->file.absoluteFilePath().toLatin1(), "r");

	if (!tiff) 
		return;

	// libtiff example
	int dircount = 0;

	do {
		dircount++;

	} while (TIFFReadDirectory(tiff));

	numPages = dircount;

	if (numPages > 1)
		pageIdx = 1;

	qDebug() << dircount << " TIFF directories... " << dt.getTotal();
	TIFFClose(tiff);

	TIFFSetWarningHandler(oldWarningHandler);
	TIFFSetWarningHandler(oldErrorHandler);
#endif

}

bool DkBasicLoader::loadPage(int skipIdx) {

	bool imgLoaded = false;

#ifdef WITH_LIBTIFF
	pageIdx += skipIdx;

	// <= 1 since first page is loaded using qt
	if (pageIdx > numPages || pageIdx <= 1)
		return imgLoaded;

	// first turn off nasty warning/error dialogs - (we do the GUI : )
	TIFFErrorHandler oldErrorHandler, oldWarningHandler;
	oldWarningHandler = TIFFSetWarningHandler(NULL);
	oldErrorHandler = TIFFSetErrorHandler(NULL); 

	DkTimer dt;
	TIFF* tiff = TIFFOpen(this->file.absoluteFilePath().toLatin1(), "r");

	if (!tiff)
		return imgLoaded;

	uint32 width = 0;
	uint32 height = 0;
	TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

	// go to current directory
	for (int idx = 1; idx < pageIdx; idx++) {

		if (!TIFFReadDirectory(tiff))
			return false;
	}

	// init the qImage
	qImg = QImage(width, height, QImage::Format_ARGB32);

	const int stopOnError = 1;
	imgLoaded = TIFFReadRGBAImageOriented(tiff, width, height, reinterpret_cast<uint32 *>(qImg.bits()), ORIENTATION_TOPLEFT, stopOnError);

	if (imgLoaded) {
		for (uint32 y=0; y<height; ++y)
			convert32BitOrder(qImg.scanLine(y), width);
	}

	TIFFClose(tiff);

	TIFFSetWarningHandler(oldWarningHandler);
	TIFFSetWarningHandler(oldErrorHandler);
#endif

	return imgLoaded;
}

bool DkBasicLoader::setPageIdx(int skipIdx) {

	// do nothing if we don't have tiff pages
	if (numPages <= 1)
		return false;

	pageIdxDirty = false;

	int newPageIdx = pageIdx + skipIdx;

	if (newPageIdx > 0 && newPageIdx <= numPages) {
		pageIdxDirty = true;
		pageIdx = newPageIdx;
	}

	return pageIdxDirty;
}

void DkBasicLoader::resetPageIdx() {

	pageIdxDirty = false;
	pageIdx = 1;
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

QFileInfo DkBasicLoader::save(const QFileInfo& fileInfo, const QImage& img, int compression) {

	QSharedPointer<QByteArray> ba;

	qDebug() << "saving: " << fileInfo.absoluteFilePath();

	if (saveToBuffer(fileInfo, img, ba, compression) && ba) {

		if (writeBufferToFile(fileInfo, ba))
			return fileInfo;
	}

	return QFileInfo();
}

bool DkBasicLoader::saveToBuffer(const QFileInfo& fileInfo, const QImage& img, QSharedPointer<QByteArray>& ba, int compression) {

	if (!ba) 
		ba = QSharedPointer<QByteArray>(new QByteArray());

	bool saved = false;

	qDebug() << "extension: " << fileInfo.suffix();

	if (fileInfo.suffix().contains("webp", Qt::CaseInsensitive)) {
		saved = saveWebPFile(img, ba, compression);
	}
	else {

		bool hasAlpha = DkImage::alphaChannelUsed(img);
		QImage sImg = img;

		// JPEG 2000 can only handle 32 or 8bit images
		if (!hasAlpha && !fileInfo.suffix().contains(QRegExp("(j2k|jp2|jpf|jpx)")))
			sImg = sImg.convertToFormat(QImage::Format_RGB888);
		else if (fileInfo.suffix().contains(QRegExp("(j2k|jp2|jpf|jpx)")) && sImg.depth() != 32 && sImg.depth() != 8)
			sImg = sImg.convertToFormat(QImage::Format_RGB32);

		qDebug() << "img has alpha: " << (sImg.format() != QImage::Format_RGB888) << " img uses alpha: " << hasAlpha;

		QBuffer fileBuffer(ba.data());
		fileBuffer.open(QIODevice::WriteOnly);
		QImageWriter* imgWriter = new QImageWriter(&fileBuffer, fileInfo.suffix().toStdString().c_str());
		imgWriter->setCompression(compression);
		imgWriter->setQuality(compression);
		saved = imgWriter->write(sImg);		// TODO: J2K crash detected
		delete imgWriter;
	}

	if (saved && metaData->isLoaded()) {
		try {
			metaData->setExifValue("Exif.Image.ImageWidth", QString::number(img.width()));
			metaData->setExifValue("Exif.Image.ImageLength", QString::number(img.height()));
			metaData->clearOrientation();
			metaData->setThumbnail(DkImage::createThumb(img));
			metaData->saveMetaData(ba, true);
			metaData->printMetaData();
		} 
		catch (...) {
			// is it still throwing anything?
			qDebug() << "Sorry, I could not save the meta data...";
		}
	}
	
	if (!saved)
		emit errorDialogSignal(tr("Sorry, I could not save: %1").arg(fileInfo.fileName()));

	return saved;
}

void DkBasicLoader::saveThumbToMetaData(const QFileInfo& fileInfo) {

	QSharedPointer<QByteArray> ba;	// dummy
	saveThumbToMetaData(fileInfo, ba);
}

void DkBasicLoader::saveMetaData(const QFileInfo& fileInfo) {

	QSharedPointer<QByteArray> ba;	// dummy
	saveMetaData(fileInfo, ba);
}

void DkBasicLoader::saveThumbToMetaData(const QFileInfo& fileInfo, QSharedPointer<QByteArray>& ba) {
	
	if (qImg.isNull())
		return;

	metaData->setThumbnail(DkImage::createThumb(qImg));
	saveMetaData(fileInfo, ba);
}

void DkBasicLoader::saveMetaData(const QFileInfo& fileInfo, QSharedPointer<QByteArray>& ba) {

	if (!ba)
		ba = QSharedPointer<QByteArray>(new QByteArray());

	if (ba->isEmpty() && metaData->isDirty())
		ba = loadFileToBuffer(fileInfo);

	bool saved = false;
	try {
		saved = metaData->saveMetaData(ba);
	} 
	catch(...) {
	}
	
	if (saved)
		writeBufferToFile(fileInfo, ba);

}

// image editing --------------------------------------------------------------------
/**
 * This method rotates an image.
 * @param orientation the orientation in degree.
 **/ 
void DkBasicLoader::rotate(int orientation) {

	if (orientation == 0 || orientation == -1)
		return;

	QTransform rotationMatrix;
	rotationMatrix.rotate((double)orientation);
	qImg = qImg.transformed(rotationMatrix);

// TODO: test without OpenCV
#ifdef WITH_OPENCV

	if (!cvImg.empty()) {

		DkVector nSz = DkVector(cvImg.size());	// *0.5f?
		DkVector nSl = nSz;
		DkVector nSr = nSz;

		double angleRad = orientation*DK_RAD2DEG;
		int interpolation = (orientation % 90 == 0) ? INTER_NEAREST : INTER_CUBIC;

		// compute
		nSl.rotate(angleRad);
		nSl.abs();

		nSr.swap();
		nSr.rotate(angleRad);
		nSr.abs();
		nSr.swap();

		nSl = nSl.maxVec(nSr);

		DkVector center = nSl * 0.5f;

		Mat rotMat = getRotationMatrix2D(center.getCvPoint32f(), DK_RAD2DEG*angleRad, 1.0);

		// add a shift towards new center
		DkVector cDiff = center - (nSz * 0.5f);
		cDiff.rotate(angleRad);

		double *transl = rotMat.ptr<double>();
		transl[2] += (double)cDiff.x;
		transl[5] += (double)cDiff.y;

		// img in wrapAffine must not be overwritten
		Mat rImg = Mat(nSl.getCvSize(), cvImg.type());
		warpAffine(cvImg, rImg, rotMat, rImg.size(), interpolation, BORDER_CONSTANT/*, borderValue*/);
		cvImg = rImg;
	} 

#endif

}

/**
 * Releases the currently loaded images.
 **/ 
void DkBasicLoader::release() {

	// TODO: auto save routines here?
	//qDebug() << file.fileName() << " released...";

	qImg = QImage();
	//metaData.clear();
	
	// TODO: where should we clear the metadata?
	if (!metaData->isDirty())
		metaData = QSharedPointer<DkMetaDataT>(new DkMetaDataT());
	
#ifdef WITH_OPENCV
	cvImg.release();
#endif

}

#ifdef WITH_WEBP

bool DkBasicLoader::loadWebPFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba) {

	if (!ba || ba->isEmpty())
		ba = loadFileToBuffer(fileInfo);
	if (ba->isEmpty())
		return false;

	// retrieve the image features (size, alpha etc.)
	WebPBitstreamFeatures features;
	int error = WebPGetFeatures((const uint8_t*)ba->data(), ba->size(), &features);
	if (error) return false;

	uint8_t* webData = 0;

	if (features.has_alpha) {
		webData = WebPDecodeBGRA((const uint8_t*) ba->data(), ba->size(), &features.width, &features.height);
		if (!webData) return false;
		qImg = QImage(webData, (int)features.width, (int)features.height, QImage::Format_ARGB32);
	}
	else {
		webData = WebPDecodeRGB((const uint8_t*) ba->data(), ba->size(), &features.width, &features.height);
		if (!webData) return false;
		qImg = QImage(webData, (int)features.width, (int)features.height, features.width*3, QImage::Format_RGB888);
	}

	// clone the image so we own the buffer
	qImg = qImg.copy();
	if (webData) free(webData);

	return true;
}

bool DkBasicLoader::saveWebPFile(const QFileInfo& fileInfo, const QImage img, int compression) {
	
	qDebug() << "format: " << img.format();

	QSharedPointer<QByteArray> ba;

	if (saveWebPFile(img, ba, compression) && ba && !ba->isEmpty()) {

		writeBufferToFile(fileInfo, ba);
		return true;
	}

	return false;
}

bool DkBasicLoader::saveWebPFile(const QImage img, QSharedPointer<QByteArray>& ba, int compression, int speed) {

	if (!ba)
		ba = QSharedPointer<QByteArray>(new QByteArray());

	QImage sImg;

	bool hasAlpha = DkImage::alphaChannelUsed(img);

	// currently, guarantee that the image is a ARGB image
	if (!hasAlpha && img.format() != QImage::Format_RGB888)
		sImg = img.convertToFormat(QImage::Format_RGB888);	// for now
	else 
		sImg = img;
	//char* buffer;
	//size_t bufSize;

	//if (compression < 0) {

	//	if (!img.hasAlphaChannel())
	//		qDebug() << "no alpha...";


	//	if (img.hasAlphaChannel())
	//		bufSize = WebPEncodeLosslessBGRA(reinterpret_cast<const uint8_t*>(img.constBits()), img.width(), img.height(), img.bytesPerLine(), reinterpret_cast<uint8_t**>(&buffer));
	//	// // without alpha there is something wrong...
	//	else
	//		bufSize = WebPEncodeLosslessRGB(reinterpret_cast<const uint8_t*>(img.constBits()), img.width(), img.height(), img.bytesPerLine(), reinterpret_cast<uint8_t**>(&buffer));
	//}
	//else {
	//	
	//	if (img.hasAlphaChannel())
	//		bufSize = WebPEncodeBGRA(reinterpret_cast<const uint8_t*>(img.constBits()), img.width(), img.height(), img.bytesPerLine(), compression, reinterpret_cast<uint8_t**>(&buffer));
	//	else
	//		bufSize = WebPEncodeRGB(reinterpret_cast<const uint8_t*>(img.constBits()), img.width(), img.height(), img.bytesPerLine(), compression, reinterpret_cast<uint8_t**>(&buffer));
	//}

	//if (!bufSize) return false;

	//QFile file(fileInfo.absoluteFilePath());
	//file.open(QIODevice::WriteOnly);
	//file.write(buffer, bufSize);
	//free(buffer);

	WebPConfig config;
	bool lossless = false;
	if (compression == -1) {
		compression = 100;
		lossless = true;
	}
	if (!WebPConfigPreset(&config, WEBP_PRESET_PHOTO, compression)) return false;
	if (lossless) config.lossless = 1;
	config.method = speed;

	WebPPicture webImg;
	if (!WebPPictureInit(&webImg)) return false;
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


}