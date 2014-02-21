/*******************************************************************************************************
 DkImageStorage.h
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

#pragma once

#include <QDebug>
#include <QMutex>
#include <QImage>
#include <QThread>

#include "DkMetaData.h"

// opencv
#ifdef WITH_OPENCV

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#include <libraw/libraw.h>
#ifdef DISABLE_LANCZOS // opencv 2.1.0 is used, does not have opencv2 includes
#include "opencv/cv.h"
#else
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

using namespace cv;
#endif

namespace nmc {

/**
 * DkImage holds some basic image processing
 * methods that are generally needed.
 **/ 
class DkImage {

public:

	/**< interpolation mapping OpenCV -> Qt */
	enum{ipl_nearest, ipl_area, ipl_linear, ipl_cubic, ipl_lanczos, ipl_end};

#ifdef WIN32
	static QImage fromWinHBITMAP(HDC hdc, HBITMAP bitmap, int w, int h);
	static QPixmap fromWinHICON(HICON icon);
#endif

#ifdef WITH_OPENCV
	
	/**
	 * Converts a QImage to a Mat
	 * @param img formats supported: ARGB32 | RGB32 | RGB888 | Indexed8
	 * @return cv::Mat the corresponding Mat
	 **/ 
	static Mat qImage2Mat(const QImage img) {

		Mat mat2;
		QImage cImg;	// must be initialized here!	(otherwise the data is lost before clone())

		if (img.format() == QImage::Format_RGB32)
			qDebug() << "we have an RGB32 in memory...";

		if (img.format() == QImage::Format_ARGB32 || img.format() == QImage::Format_RGB32) {
			mat2 = Mat(img.height(), img.width(), CV_8UC4, (uchar*)img.bits(), img.bytesPerLine());
			//qDebug() << "ARGB32 or RGB32";
		}
		else if (img.format() == QImage::Format_RGB888) {
			mat2 = Mat(img.height(), img.width(), CV_8UC3, (uchar*)img.bits(), img.bytesPerLine());
			//qDebug() << "RGB888";
		}
		//// converting to indexed8 causes bugs in the qpainter
		//// see: http://qt-project.org/doc/qt-4.8/qimage.html
		//else if (img.format() == QImage::Format_Indexed8) {
		//	mat2 = Mat(img.height(), img.width(), CV_8UC1, (uchar*)img.bits(), img.bytesPerLine());
		//	//qDebug() << "indexed...";
		//}
		else {
			//qDebug() << "image flag: " << img.format();
			cImg = img.convertToFormat(QImage::Format_ARGB32);
			mat2 = Mat(cImg.height(), cImg.width(), CV_8UC4, (uchar*)cImg.bits(), cImg.bytesPerLine());
			//qDebug() << "I need to convert the QImage to ARGB32";
		}

		mat2 = mat2.clone();	// we need to own the pointer

		return mat2; 
	}

	/**
	 * Converts a cv::Mat to a QImage.
	 * @param img supported formats CV8UC1 | CV_8UC3 | CV_8UC4
	 * @return QImage the corresponding QImage
	 **/ 
	static QImage mat2QImage(Mat img) {

		QImage qImg;

		// since Mat header is copied, a new buffer should be allocated (check this!)
		if (img.depth() == CV_32F)
			img.convertTo(img, CV_8U, 255);

		if (img.type() == CV_8UC1) {
			qImg = QImage(img.data, (int)img.cols, (int)img.rows, (int)img.step, QImage::Format_Indexed8);	// opencv uses size_t if for scaling in x64 applications
			//Mat tmp;
			//cvtColor(img, tmp, CV_GRAY2RGB);	// Qt does not support writing to index8 images
			//img = tmp;
		}
		if (img.type() == CV_8UC3) {
			
			//cv::cvtColor(img, img, CV_RGB2BGR);
			qImg = QImage(img.data, (int)img.cols, (int)img.rows, (int)img.step, QImage::Format_RGB888);
		}
		if (img.type() == CV_8UC4) {
			qImg = QImage(img.data, (int)img.cols, (int)img.rows, (int)img.step, QImage::Format_ARGB32);
		}

		qImg = qImg.copy();

		return qImg;
	}
#endif

	/**
	 * Returns a string with the buffer size of an image.
	 * @param img a QImage
	 * @return QString a human readable string containing the buffer size
	 **/ 
	static QString getBufferSize(const QImage img) {

		return getBufferSize(img.size(), img.depth());
	}

	/**
	 * Returns a string with the buffer size of an image.
	 * @param imgSize the image size
	 * @param depth the image depth
	 * @return QString a human readable string containing the buffer size
	 **/ 
	static QString getBufferSize(const QSize imgSize, const int depth) {

		double size = (double)imgSize.width() * (double)imgSize.height() * (double)(depth/8.0f);
		QString sizeStr;
		qDebug() << "dimension: " << size;

		if (size >= 1024*1024*1024) {
			return QString::number(size/(1024.0f*1024.0f*1024.0f), 'f', 2) + " GB";
		}
		else if (size >= 1024*1024) {
			return QString::number(size/(1024.0f*1024.0f), 'f', 2) + " MB";
		}
		else if (size >= 1024) {
			return QString::number(size/1024.0f, 'f', 2) + " KB";
		}
		else {
			return QString::number(size, 'f', 2) + " B";
		}
	}

	/**
	 * Returns a the buffer size of an image.
	 * @param imgSize the image size
	 * @param depth the image depth
	 * @return buffer size in MB
	 **/ 
	static float getBufferSizeFloat(const QSize imgSize, const int depth) {

		double size = (double)imgSize.width() * (double)imgSize.height() * (double)(depth/8.0f);
		QString sizeStr;
		//qDebug() << "dimension: " << size;

		return size/(1024.0f*1024.0f);
	}


	/**
	 * This function resizes an image according to the interpolation method specified.
	 * @param img the image to resize
	 * @param newSize the new size
	 * @param factor the resize factor
	 * @param interpolation the interpolation method
	 * @return QImage the resized image
	 **/ 
	static QImage resizeImage(const QImage img, const QSize& newSize, float factor = 1.0f, int interpolation = ipl_cubic) {
		
		QSize nSize = newSize;

		// nothing to do
		if (img.size() == nSize && factor == 1.0f)
			return img;

		if (factor != 1.0f)
			nSize = QSize(img.width()*factor, img.height()*factor);

		if (nSize.width() < 1 || nSize.height() < 1) {
			return QImage();
		}

		Qt::TransformationMode iplQt;
		switch(interpolation) {
		case ipl_nearest:	
		case ipl_area:		iplQt = Qt::FastTransformation; break;
		case ipl_linear:	
		case ipl_cubic:		
		case ipl_lanczos:	iplQt = Qt::SmoothTransformation; break;
		}
#ifdef WITH_OPENCV

		int ipl = CV_INTER_CUBIC;
		switch(interpolation) {
		case ipl_nearest:	ipl = CV_INTER_NN; break;
		case ipl_area:		ipl = CV_INTER_AREA; break;
		case ipl_linear:	ipl = CV_INTER_LINEAR; break;
		case ipl_cubic:		ipl = CV_INTER_CUBIC; break;
#ifdef DISABLE_LANCZOS
		case ipl_lanczos:	ipl = CV_INTER_CUBIC; break;
#else
		case ipl_lanczos:	ipl = CV_INTER_LANCZOS4; break;
#endif
		}


		try {
			Mat resizeImage = DkImage::qImage2Mat(img);

			// is the image convertible?
			if (resizeImage.empty()) {
				return img.scaled(newSize, Qt::IgnoreAspectRatio, iplQt);
			}
			else {

				Mat tmp;
				cv::resize(resizeImage, tmp, cv::Size(nSize.width(), nSize.height()), 0, 0, ipl);
				resizeImage = tmp;
				return DkImage::mat2QImage(resizeImage);
			}

		}catch (std::exception se) {

			return QImage();
		}

#else

		return img.scaled(nSize, Qt::IgnoreAspectRatio, iplQt);

#endif
	}

	static QImage normImage(const QImage& img);

	static bool normImage(QImage& img);

	static QImage autoAdjustImage(const QImage& img);

	static bool autoAdjustImage(QImage& img);

	static bool alphaChannelUsed(const QImage& img);

	static QPixmap colorizePixmap(const QPixmap& icon, const QColor& col, float opacity = 0.5f);

};

class DkImageStorage : public QObject {
	Q_OBJECT

public:
	DkImageStorage(QImage img = QImage());

	void setImage(QImage img);
	QImage getImage(float factor = 1.0f);
	bool hasImage() {
		return !img.isNull();
	}

public slots:
	void computeImage();
	void antiAliasingChanged(bool antiAliasing);

signals:
	void imageUpdated();

protected:
	QImage img;
	QVector<QImage> imgs;

	QMutex mutex;
	QThread* computeThread;
	bool busy;
	bool stop;
};

};
