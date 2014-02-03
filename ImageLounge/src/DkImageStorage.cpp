/*******************************************************************************************************
 DkImageStorage.cpp
 Created on:	12.07.2013
 
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

#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkTimer.h"

namespace nmc {

// DkImage --------------------------------------------------------------------
QImage DkImage::normImage(const QImage& img) {

	QImage imgN = img.copy();
	normImage(imgN);

	return imgN;
}

bool DkImage::normImage(QImage& img) {

	uchar maxVal = 0;
	uchar minVal = 255;

	// number of used bytes pr line
	int bpl = (img.width() * img.depth() + 7) / 8;
	int pad = img.bytesPerLine() - bpl;
	uchar* mPtr = img.bits();
	for (int rIdx = 0; rIdx < img.height(); rIdx++) {
		for (int cIdx = 0; cIdx < bpl; cIdx++, mPtr++) {
			
			if (*mPtr > maxVal)
				maxVal = *mPtr;
			if (*mPtr < minVal)
				minVal = *mPtr;
			
			//*mPtr++ ^= 0xff;
		}
		mPtr += pad;
	}

	if (minVal == 0 && maxVal == 255)
		return false;

	// number of used bytes pr line
	uchar* ptr = img.bits();
	for (int rIdx = 0; rIdx < img.height(); rIdx++) {
		for (int cIdx = 0; cIdx < bpl; cIdx++) {
			*ptr++ = qRound(255.0f*(*ptr-minVal)/(maxVal-minVal));
		}
		ptr += pad;
	}

	return true;

}

QImage DkImage::autoAdjustImage(const QImage& img) {

	QImage imgA = img.copy();
	autoAdjustImage(imgA);

	return imgA;
}

bool DkImage::autoAdjustImage(QImage& img) {

	qDebug() << "[Auto Adjust] image format: " << img.format();

	// for grayscale image - normalize is the same
	if (img.format() <= QImage::Format_Indexed8) {
		qDebug() << "[Auto Adjust] Grayscale - switching to Normalize: " << img.format();
		return normImage(img);
	}
	else if (img.format() != QImage::Format_ARGB32 && img.format() != QImage::Format_ARGB32_Premultiplied && 
		img.format() != QImage::Format_RGB32 && img.format() != QImage::Format_RGB888) {
		qDebug() << "[Auto Adjust] Format not supported: " << img.format();
		return false;
	}

	int channels = (img.format() == QImage::Format_RGB888) ? 3 : 4;

	QColor maxVal(0,0,0);
	QColor minVal(255,255,255);

	// number of bytes per line used
	int bpl = (img.width() * img.depth() + 7) / 8;
	int pad = img.bytesPerLine() - bpl;

	uchar* mPtr = img.bits();

	for (int rIdx = 0; rIdx < img.height(); rIdx++) {

		for (int cIdx = 0; cIdx < bpl; ) {

			uchar r = *mPtr; mPtr++;
			uchar g = *mPtr; mPtr++;
			uchar b = *mPtr; mPtr++;

			QRgb val = qRgb(r, g, b);
			cIdx += 3;

			if (qRed(val) > maxVal.red())
				maxVal.setRed(qRed(val));
			if (qRed(val) < minVal.red())
				minVal.setRed(qRed(val));

			if (qGreen(val) > maxVal.green())
				maxVal.setGreen(qGreen(val));
			if (qGreen(val) < minVal.green())
				minVal.setGreen(qGreen(val));

			if (qBlue(val) > maxVal.blue())
				maxVal.setBlue(qBlue(val));
			if (qBlue(val) < minVal.blue())
				minVal.setBlue(qBlue(val));

			// ?? strange but I would expect the alpha channel to be the first (big endian?)
			if (channels == 4) {
				mPtr++;
				cIdx++;
			}

		}
		mPtr += pad;
	}

	QColor ignoreChannel;
	ignoreChannel.setRed(maxVal.red()-minVal.red() < 30 || maxVal.red()-minVal.red() == 255);
	ignoreChannel.setGreen(maxVal.green()-minVal.green() < 30 || maxVal.green()-minVal.green() == 255);
	ignoreChannel.setBlue(maxVal.blue()-minVal.blue() < 30 || maxVal.blue()-minVal.blue() == 255);

	if (ignoreChannel.red() && ignoreChannel.green() && ignoreChannel.blue()) {
		qDebug() << "[Auto Adjust] There is no need to adjust the image";
		return false;
	}

	// number of used bytes pr line
	uchar* ptr = img.bits();

	qDebug() << "red max: " << maxVal.red() << " min: " << minVal.red() << " ignored: " << ignoreChannel.red();
	qDebug() << "green max: " << maxVal.green() << " min: " << minVal.green() << " ignored: " << ignoreChannel.green();
	qDebug() << "blue max: " << maxVal.blue() << " min: " << minVal.blue() << " ignored: " << ignoreChannel.blue();

	for (int rIdx = 0; rIdx < img.height(); rIdx++) {

		for (int cIdx = 0; cIdx < bpl; ) {

			// don't check values - speed (but you see under-/overflows anyway)
			if (!ignoreChannel.red())
				*ptr = qRound(255.0f*((float)*ptr-minVal.red())/(maxVal.red()-minVal.red()));
			ptr++;
			cIdx++;

			if (!ignoreChannel.green())
				*ptr = qRound(255.0f*((float)*ptr-minVal.green())/(maxVal.green()-minVal.green()));
			ptr++;
			cIdx++;

			if (!ignoreChannel.blue())
				*ptr = qRound(255.0f*((float)*ptr-minVal.blue())/(maxVal.blue()-minVal.blue()));
			ptr++;
			cIdx++;

			if (channels == 4) {
				ptr++;
				cIdx++;
			}

		}
		ptr += pad;
	}

	//qDebug() << "[Auto Adjust] image adjusted: " << img.format() << " depth: " << img.depth() << " bitPerPlane: " << img.bitPlaneCount();
	return true;



}

// DkImageStorage --------------------------------------------------------------------
DkImageStorage::DkImageStorage(QImage img) {
	this->img = img;

	computeThread = new QThread;
	computeThread->start();
	moveToThread(computeThread);

	busy = false;
	stop = true;
}

void DkImageStorage::setImage(QImage img) {

	stop = true;
	imgs.clear();	// is it save (if the thread is still working?)
	this->img = img;
}

void DkImageStorage::antiAliasingChanged(bool antiAliasing) {

	DkSettings::display.antiAliasing = antiAliasing;

	if (!antiAliasing) {
		stop = true;
		imgs.clear();
	}

	emit imageUpdated();

}

QImage DkImageStorage::getImage(float factor) {

	if (factor >= 0.5f || img.isNull() || !DkSettings::display.antiAliasing)
		return img;

	// check if we have an image similar to that requested
	for (int idx = 0; idx < imgs.size(); idx++) {

		if ((float)imgs.at(idx).height()/img.height() >= factor)
			return imgs.at(idx);
	}

	// if the image does not exist - create it
	if (!busy && imgs.empty() && img.colorTable().isEmpty() && img.width() > 32 && img.height() > 32) {
		stop = false;
		// nobody is busy so start working
		QMetaObject::invokeMethod(this, "computeImage", Qt::QueuedConnection);
	}

	// currently no alternative is available
	return img;
}

void DkImageStorage::computeImage() {

	// obviously, computeImage gets called multiple times in some wired cases...
	if (!imgs.empty())
		return;

	DkTimer dt;
	busy = true;
	QImage resizedImg = img;

	// down sample the image until it is twice times full HD
	QSize iSize = img.size();
	while (iSize.width() > 2*1542 && iSize.height() > 2*1542)	// in general we need less than 200 ms for the whole downscaling if we start at 1500 x 1500
		iSize *= 0.5;

	// for extreme panorama images the Qt scaling crashes (if we have a width > 30000) so we simply 
	if (qMax(iSize.width(), iSize.height()) < 20000)
		resizedImg = resizedImg.scaled(iSize, Qt::KeepAspectRatio, Qt::FastTransformation);

	// it would be pretty strange if we needed more than 30 sub-images
	for (int idx = 0; idx < 30; idx++) {

		QSize s = resizedImg.size();
		s *= 0.5;

		if (s.width() < 32 || s.height() < 32)
			break;

#ifdef WITH_OPENCV
		cv::Mat rImgCv = DkImage::qImage2Mat(resizedImg);
		cv::Mat tmp;
		cv::resize(rImgCv, tmp, cv::Size(s.width(), s.height()), 0, 0, CV_INTER_AREA);
		resizedImg = DkImage::mat2QImage(tmp);
		//resizedImg.setColorTable(img.colorTable());
#else
		resizedImg = resizedImg.scaled(s, Qt::KeepAspectRatio, Qt::SmoothTransformation);
#endif

		// new image assigned?
		if (stop)
			break;

		mutex.lock();
		imgs.push_front(resizedImg);
		mutex.unlock();
	}

	busy = false;

	// tell my caller I did something
	emit imageUpdated();

	qDebug() << "pyramid computation took me: " << QString::fromStdString(dt.getTotal()) << " layers: " << imgs.size();

	if (imgs.size() > 6)
		qDebug() << "layer size > 6: " << img.size();

}

}
