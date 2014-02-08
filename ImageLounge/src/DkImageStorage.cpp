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
bool DkImage::alphaChannelUsed(const QImage& img) {

	if (img.format() != QImage::Format_ARGB32 && img.format() != QImage::Format_ARGB32_Premultiplied)
		return false;

	// number of used bytes per line
	int bpl = (img.width() * img.depth() + 7) / 8;
	int pad = img.bytesPerLine() - bpl;
	const uchar* ptr = img.bits();

	for (int rIdx = 0; rIdx < img.height(); rIdx++) {

		for (int cIdx = 0; cIdx < bpl; cIdx++, ptr++) {

			if (cIdx % 4 == 3 && *ptr != 255)
				return true;
		}

		ptr += pad;
	}

	return false;
}
	
QImage DkImage::normImage(const QImage& img) {

	QImage imgN = img.copy();
	normImage(imgN);

	return imgN;
}

bool DkImage::normImage(QImage& img) {

	uchar maxVal = 0;
	uchar minVal = 255;

	// number of used bytes per line
	int bpl = (img.width() * img.depth() + 7) / 8;
	int pad = img.bytesPerLine() - bpl;
	uchar* mPtr = img.bits();
	bool hasAlpha = img.hasAlphaChannel() || img.format() == QImage::Format_RGB32;

	for (int rIdx = 0; rIdx < img.height(); rIdx++) {
		
		for (int cIdx = 0; cIdx < bpl; cIdx++, mPtr++) {
			
			if (hasAlpha && cIdx % 4 == 3)
				continue;

			if (*mPtr > maxVal)
				maxVal = *mPtr;
			if (*mPtr < minVal)
				minVal = *mPtr;
		}
		
		mPtr += pad;
	}

	if (minVal == 0 && maxVal == 255)
		return false;

	uchar* ptr = img.bits();
	
	for (int rIdx = 0; rIdx < img.height(); rIdx++) {
	
		for (int cIdx = 0; cIdx < bpl; cIdx++, ptr++) {

			if (hasAlpha && cIdx % 4 == 3)
				continue;

			*ptr = qRound(255.0f*(*ptr-minVal)/(maxVal-minVal));
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

	DkTimer dt;
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

	int channels = (img.hasAlphaChannel() || img.format() == QImage::Format_RGB32) ? 4 : 3;

	uchar maxR = 0,		maxG = 0,	maxB = 0;
	uchar minR = 255,	minG = 255, minB = 255;

	// number of bytes per line used
	int bpl = (img.width() * img.depth() + 7) / 8;
	int pad = img.bytesPerLine() - bpl;

	uchar* mPtr = img.bits();
	uchar r,g,b;

	for (int rIdx = 0; rIdx < img.height(); rIdx++) {

		for (int cIdx = 0; cIdx < bpl; ) {

			r = *mPtr; mPtr++;
			g = *mPtr; mPtr++;
			b = *mPtr; mPtr++;
			cIdx += 3;

			if (r > maxR)	maxR = r;
			if (r < minR)	minR = r;

			if (g > maxG)	maxG = g;
			if (g < minG)	minG = g;

			if (b > maxB)	maxB = b;
			if (b < minB)	minB = b;


			// ?? strange but I would expect the alpha channel to be the first (big endian?)
			if (channels == 4) {
				mPtr++;
				cIdx++;
			}

		}
		mPtr += pad;
	}

	QColor ignoreChannel;
	bool ignoreR = maxR-minR == 255;
	bool ignoreG = maxG-minG == 255;
	bool ignoreB = maxB-minB == 255;

	uchar* ptr = img.bits();

	//qDebug() << "red max: " << maxR << " min: " << minR << " ignored: " << ignoreR;
	//qDebug() << "green max: " << maxG << " min: " << minG << " ignored: " << ignoreG;
	//qDebug() << "blue max: " << maxB << " min: " << minB << " ignored: " << ignoreB;
	//qDebug() << "computed in: " << QString::fromStdString(dt.getTotal());

	if (ignoreR && ignoreG && ignoreB) {
		qDebug() << "[Auto Adjust] There is no need to adjust the image";
		return false;
	}

	for (int rIdx = 0; rIdx < img.height(); rIdx++) {

		for (int cIdx = 0; cIdx < bpl; ) {

			// don't check values - speed (but you see under-/overflows anyway)
			if (!ignoreR)
				*ptr = qRound(255.0f*((float)*ptr-minR)/(maxR-minR));
			ptr++;
			cIdx++;

			if (!ignoreG)
				*ptr = qRound(255.0f*((float)*ptr-minG)/(maxG-minG));
			ptr++;
			cIdx++;

			if (!ignoreB)
				*ptr = qRound(255.0f*((float)*ptr-minB)/(maxB-minB));
			ptr++;
			cIdx++;

			if (channels == 4) {
				ptr++;
				cIdx++;
			}

		}
		ptr += pad;
	}

	qDebug() << "[Auto Adjust] image adjusted in: " << QString::fromStdString(dt.getTotal());
	
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

	qDebug() << "empty color table: " << img.colorTable().isEmpty();

	// if the image does not exist - create it
	if (!busy && imgs.empty() && /*img.colorTable().isEmpty() &&*/ img.width() > 32 && img.height() > 32) {
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
		resizedImg.setColorTable(img.colorTable());		// Not sure why we turned the color tables off
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
