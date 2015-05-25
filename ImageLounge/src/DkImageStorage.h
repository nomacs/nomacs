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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QImage>
#include <QMutex>
#include <QVector>
#include <QObject>

// opencv
#ifdef WITH_OPENCV
#ifdef DISABLE_LANCZOS // opencv 2.1.0 is used, does not have opencv2 includes
#include "opencv/cv.h"
#else
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#endif
#endif
#pragma warning(pop)		// no warnings from includes - end

#ifdef WIN32
#pragma warning(disable: 4251)	// TODO: remove
#endif

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

// Qt defines
class QPixmap;
class QString;
class QSize;
class QColor;

namespace nmc {

/**
 * DkImage holds some basic image processing
 * methods that are generally needed.
 **/ 
class DllExport DkImage {

public:

	/**< interpolation mapping OpenCV -> Qt */
	enum{ipl_nearest, ipl_area, ipl_linear, ipl_cubic, ipl_lanczos, ipl_end};

#ifdef WIN32
	static QImage fromWinHBITMAP(HDC hdc, HBITMAP bitmap, int w, int h);
	static QPixmap fromWinHICON(HICON icon);
#endif

#ifdef WITH_OPENCV
	static cv::Mat qImage2Mat(const QImage& img);
	static QImage mat2QImage(cv::Mat img);
	static cv::Mat get1DGauss(double sigma);
	static void mapGammaTable(cv::Mat& img, const QVector<unsigned short>& gammaTable);
	static void gammaToLinear(cv::Mat& img);
	static void linearToGamma(cv::Mat& img);
	static void logPolar(const cv::Mat& src, cv::Mat& dst, CvPoint2D32f center, double scaleLog, double scale, double angle, int flags);
	static void tinyPlanet(QImage& img, double scaleLog, double scale, double angle, QSize s, bool invert = false);
#endif

	static QString getBufferSize(const QImage& img);
	static QString getBufferSize(const QSize& imgSize, const int depth);
	static float getBufferSizeFloat(const QSize& imgSize, const int depth);
	static QImage resizeImage(const QImage& img, const QSize& newSize, float factor = 1.0f, int interpolation = ipl_cubic, bool correctGamma = true);

	template <typename numFmt>
	static QVector<numFmt> getGamma2LinearTable(int maxVal = USHRT_MAX);
	template <typename numFmt>
	static QVector<numFmt> getLinear2GammaTable(int maxVal = USHRT_MAX);
	static void gammaToLinear(QImage& img);
	static void linearToGamma(QImage& img);
	static void mapGammaTable(QImage& img, const QVector<uchar>& gammaTable);
	static QImage normImage(const QImage& img);
	static bool normImage(QImage& img);
	static QImage autoAdjustImage(const QImage& img);
	static bool autoAdjustImage(QImage& img);
	static bool unsharpMask(QImage& img, float sigma = 20.0f, float weight = 1.5f);
	static bool alphaChannelUsed(const QImage& img);
	static QPixmap colorizePixmap(const QPixmap& icon, const QColor& col, float opacity = 1.0f);
	static QImage createThumb(const QImage& img);
	static bool addToImage(QImage& img, unsigned char val = 1);
	static QColor getMeanColor(const QImage& img);
	static uchar findHistPeak(const int* hist, float quantile = 0.005f);
};

class DllExport DkImageStorage : public QObject {
	Q_OBJECT

public:
	DkImageStorage(QImage img = QImage());

	void setImage(QImage img);
	QImage getImageConst() const;
	QImage getImage(float factor = 1.0f);
	bool hasImage() const {
		return !img.isNull();
	}

public slots:
	void computeImage();
	void antiAliasingChanged(bool antiAliasing);

signals:
	void imageUpdated();
	void infoSignal(QString msg);

protected:
	QImage img;
	QVector<QImage> imgs;

	QMutex mutex;
	QThread* computeThread;
	bool busy;
	bool stop;
};

};
