/*******************************************************************************************************
 DkInstagramPlugin.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>

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


#include <QObject>
#include <QWidget>
#include <QtPlugin>
#include <QImage>
#include <QStringList>
#include <QString>
#include <QMessageBox>


#include "DkPluginInterface.h"

// OpenCV
#ifdef WITH_OPENCV

#ifdef Q_WS_WIN
	#pragma warning(disable: 4996)
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
#endif

namespace nmc {

class DkInstagramPlugin : public QObject, DkPluginInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkInstagramPlugin/3.0" FILE "DkInstagramPlugin.json")

public:

	DkInstagramPlugin(QObject* parent = 0);
	~DkInstagramPlugin();

	QString id() const override;
	QImage image() const override;
	QString version() const override;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<nmc::DkImageContainer> imgC = QSharedPointer<nmc::DkImageContainer>()) const override;
	static QImage applyImageFilter(QImage inImg);

	enum {
		ID_ACTION1,
		// add actions here

		id_end
	};


protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;

#ifdef WITH_OPENCV

	static void nashville(const Mat& myImage, Mat& Result);
/**
	 * Converts a QImage to a Mat
	 * @param img formats supported: ARGB32 | RGB32 | RGB888 | Indexed8
	 * @return cv::Mat the corresponding Mat
	 **/ 
	static Mat qImage2Mat(const QImage img) {

		Mat mat2;
		QImage cImg;	// must be initialized here!	(otherwise the data is lost before clone())

		if (img.format() == QImage::Format_ARGB32 || img.format() == QImage::Format_RGB32 ) {
			mat2 = Mat(img.height(), img.width(), CV_8UC4, (uchar*)img.bits(), img.bytesPerLine());
			//qDebug() << "ARGB32 or RGB32";
		}
		else if (img.format() == QImage::Format_RGB888) {
			mat2 = Mat(img.height(), img.width(), CV_8UC3, (uchar*)img.bits(), img.bytesPerLine());
			//qDebug() << "RGB888";
		}
		else if (img.format() == QImage::Format_Indexed8) {
			mat2 = Mat(img.height(), img.width(), CV_8UC1, (uchar*)img.bits(), img.bytesPerLine());
			//qDebug() << "indexed...";
		}
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


};

};
