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
#include <QVector>
#include <QObject>
#include <QColor>
#include <QFutureWatcher>

// opencv
#ifdef WITH_OPENCV
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#endif
#pragma warning(pop)		// no warnings from includes - end

#ifdef Q_OS_WIN
#pragma warning(disable: 4251)	// TODO: remove
#pragma warning(disable: 4714)	// Qt's force inline
#endif

#if defined(__GNUC__) && defined(Q_OS_MAC) || defined(__clang__)
#pragma GCC diagnostic ignored "-Winvalid-source-encoding"
#endif

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QPixmap;
class QString;
class QSize;
class QColor;
class QTimer;

namespace nmc {

class DkRotatingRect;

/**
 * DkImage holds some basic image processing
 * methods that are generally needed.
 **/ 
class DllCoreExport DkImage {

public:

	/**< interpolation mapping OpenCV -> Qt */
	enum {
		ipl_nearest, 
		ipl_area, 
		ipl_linear, 
		ipl_cubic, 
		ipl_lanczos, 
		ipl_end};

#ifdef WITH_OPENCV
	static cv::Mat qImage2Mat(const QImage& img);
	static QImage mat2QImage(cv::Mat img);
	static cv::Mat get1DGauss(double sigma);
	static void mapGammaTable(cv::Mat& img, const QVector<unsigned short>& gammaTable);
	static void gammaToLinear(cv::Mat& img);
	static void linearToGamma(cv::Mat& img);
	static void logPolar(const cv::Mat& src, cv::Mat& dst, cv::Point2d center, double scaleLog, double angle, double scale = 1.0);
	static void tinyPlanet(QImage& img, double scaleLog, double angle, QSize s, bool invert = false);
#endif

	static QString getBufferSize(const QImage& img);
	static QString getBufferSize(const QSize& imgSize, const int depth);
	static float getBufferSizeFloat(const QSize& imgSize, const int depth);
	static QImage resizeImage(const QImage& img, const QSize& newSize, double factor = 1.0, int interpolation = ipl_cubic, bool correctGamma = true);

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
	static bool gaussianBlur(QImage& img, float sigma = 20.0f);
	static bool unsharpMask(QImage& img, float sigma = 20.0f, float weight = 1.5f);
	static bool alphaChannelUsed(const QImage& img);
	static QImage thresholdImage(const QImage& img, double thr, bool color = false);
	static QImage rotate(const QImage& img, double angle);
	static QImage grayscaleImage(const QImage& img);
	static QPixmap colorizePixmap(const QPixmap& icon, const QColor& col, float opacity = 1.0f);
	static QPixmap loadIcon(const QString& filePath = QString(), const QSize& size = QSize(), const QColor& col = QColor());
	static QPixmap loadIcon(const QString& filePath, const QColor& col, const QSize& size = QSize());
	static QPixmap loadFromSvg(const QString& filePath, const QSize& size);
	static QImage createThumb(const QImage& img, const int maxSize = -1);
	static bool addToImage(QImage& img, unsigned char val = 1);
	static QColor getMeanColor(const QImage& img);
	static uchar findHistPeak(const int* hist, float quantile = 0.005f);
	static QPixmap makeSquare(const QPixmap& pm);
	static QPixmap merge(const QVector<QImage>& imgs);
	static QImage cropToImage(const QImage& src, const QRect& cropRect, const QTransform& t, const QColor& fillColor = QColor());
	static QImage cropToImage(const QImage& src, const DkRotatingRect& rect, const QColor& fillColor = QColor());
	static QImage hueSaturation(const QImage& src, int hue, int sat, int brightness);
	static QImage exposure(const QImage& src, double exposure, double offset, double gamma);
	static QImage bgColor(const QImage& src, const QColor& col);
	static QByteArray extractImageFromDataStream(const QByteArray& ba, const QByteArray& beginSignature = "‰PNG", const QByteArray& endSignature = "END®B`‚", bool debugOutput = false);
	static QByteArray fixSamsungPanorama(QByteArray& ba);
	static int intFromByteArray(const QByteArray& ba, int pos);
	
#ifdef WITH_OPENCV
	static cv::Mat exposureMat(const cv::Mat& src, double exposure);
	static cv::Mat gammaMat(const cv::Mat& src, double gmma);
	static cv::Mat applyLUT(const cv::Mat& src, const cv::Mat& lut);
#endif // WITH_OPENCV

private:
	static QImage rotateSimple(const QImage& img, double angle);
	
};

class DllCoreExport DkImageStorage : public QObject {
	Q_OBJECT

public:
	DkImageStorage(const QImage& img = QImage());

	enum ComputeState {
		l_not_computed,
		l_computing,
		l_computed,
		l_empty,
		l_cancelled,

		l_end
	};

	bool isEmpty() const {
		return mImg.isNull();
	};

	QSize size() const {
		return mImg.size();
	};

	void setImage(const QImage& img);
	QImage imageConst() const;
	QImage image(const QSize& size = QSize());
	void cancel();

public slots:
	void antiAliasingChanged(bool antiAliasing);
	void imageComputed();
	void compute();

signals:
	void imageUpdated() const;
	void infoSignal(const QString& msg) const;

protected:
	QImage mImg;
	QImage mScaledImg;
	QSize mSize;

	QTimer* mWaitTimer = 0;
	QFutureWatcher<QImage> mFutureWatcher;

	ComputeState mComputeState = l_not_computed;

	QImage computeIntern(const QImage& src, const QSize& size);
	void init();

};
//
//class DllCoreExport DkImageStorage : public QObject {
//	Q_OBJECT
//
//public:
//	DkImageStorage(const QImage& img = QImage());
//
//	void setImage(const QImage& img);
//	QImage getImageConst() const;
//	QImage getImage(float factor = 1.0f);
//	bool hasImage() const {
//		return !mImg.isNull();
//	}
//
//public slots:
//	void antiAliasingChanged(bool antiAliasing);
//	void computeImage(double scale);
//
//signals:
//	void imageUpdated() const;
//	void infoSignal(const QString& msg) const;
//
//protected:
//	QImage mImg;
//	QVector<QImage> mImgs;
//
//	QMutex mMutex;
//	QThread* mComputeThread = 0;
//	bool mBusy = false;
//	bool mStop = true;
//
//	QTimer* mWaitTimer = 0;
//
//	void compute();
//
//	double mScaling = 1.0;
//};

}
