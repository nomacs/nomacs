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

#include <QColor>
#include <QFutureWatcher>
#include <QImage>
#include <QObject>

#ifdef WITH_OPENCV
#include "opencv2/core/core.hpp"
#endif

#include "nmc_config.h"

class QPixmap;
class QString;
class QSize;
class QColor;

namespace nmc
{
class DkRotatingRect;

/**
 * DkImage holds some basic image processing
 * methods that are generally needed.
 **/
class DllCoreExport DkImage
{
public:
    /**< interpolation mapping OpenCV -> Qt */
    enum {
        ipl_nearest,
        ipl_area,
        ipl_linear,
        ipl_cubic,
        ipl_lanczos,
        ipl_end
    };

#ifdef WITH_OPENCV
    static cv::Mat qImage2Mat(const QImage &img);
    static QImage mat2QImage(cv::Mat img);
    static void mapGammaTable(cv::Mat &img, const QVector<uint16_t> &gammaTable);
    static void gammaToLinear(cv::Mat &img);
    static void linearToGamma(cv::Mat &img);
    static void logPolar(const cv::Mat &src,
                         cv::Mat &dst,
                         cv::Point2d center,
                         double scaleLog,
                         double angle,
                         double scale = 1.0);
    static void tinyPlanet(QImage &img, double scaleLog, double angle, QSize s, bool invert = false);
#endif

    static QString getBufferSize(const QSize &imgSize, const int depth);
    static float getBufferSizeFloat(const QSize &imgSize, const int depth);
    static QImage resizeImage(const QImage &img,
                              const QSize &newSize,
                              double factor = 1.0,
                              int interpolation = ipl_cubic,
                              bool correctGamma = true);

    template<typename numFmt>
    static QVector<numFmt> getGamma2LinearTable(int maxVal = USHRT_MAX);
    template<typename numFmt>
    static QVector<numFmt> getLinear2GammaTable(int maxVal = USHRT_MAX);
    static void gammaToLinear(QImage &img);
    static void linearToGamma(QImage &img);
    static void mapGammaTable(QImage &img, const QVector<uchar> &gammaTable);
    static bool normImage(QImage &img);
    static bool autoAdjustImage(QImage &img);
    static bool gaussianBlur(QImage &img, float sigma = 20.0f);
    static bool unsharpMask(QImage &img, float sigma = 20.0f, float weight = 1.5f);
    static bool alphaChannelUsed(const QImage &img);
    static QImage thresholdImage(const QImage &img, double thr, bool color = false);

    /**
     * Rotates the image clockwise by angle. See rotateImageFast().
     */
    static QImage rotateImage(const QImage &img, double angle);
    static QImage grayscaleImage(const QImage &img);
    static QPixmap colorizePixmap(const QPixmap &icon, const QColor &col, float opacity = 1.0f);
    static QPixmap loadIcon(const QString &filePath, const QSize &size, const QColor &col);
    static QPixmap loadIcon(const QString &filePath, const QColor &col, const QSize &size);
    static QPixmap loadFromSvg(const QString &filePath, const QSize &size);

    /**
     * Load icon with colorization for the default QIcon mode and state
     *
     * @param filePath svg file
     * @param color if unset, use the default ui/user icon color
     * @note to add icons for other states you must use QIcon::addFile(), addPixMap() is unsupported
     * @note size passed through QIcon::addFile is ignored; we always render the requested size
     */
    static QIcon loadIcon(const QString &filePath, const QColor &color = {});
    static QImage createThumb(const QImage &img, int maxSize = -1);
    static uchar findHistPeak(const int *hist, float quantile = 0.005f);
    static QPixmap makeSquare(const QPixmap &pm);
    static QPixmap merge(const QVector<QImage> &imgs);
    static QImage cropToImage(const QImage &src, const DkRotatingRect &rect, const QColor &fillColor = QColor());
    static QImage hueSaturation(const QImage &src, int hue, int sat, int brightness);
    static QImage exposure(const QImage &src, double exposure, double offset, double gamma);
    static QImage bgColor(const QImage &src, const QColor &col);
    static QByteArray extractImageFromDataStream(const QByteArray &ba,
                                                 const QByteArray &beginSignature = "‰PNG",
                                                 const QByteArray &endSignature = "END®B`‚",
                                                 bool debugOutput = false);
    static bool fixSamsungPanorama(QByteArray &ba);
    static int intFromByteArray(const QByteArray &ba, int pos);

#ifdef WITH_OPENCV
    static cv::Mat exposureMat(const cv::Mat &src, double exposure);
    static cv::Mat gammaMat(const cv::Mat &src, double gmma);
    static cv::Mat applyLUT(const cv::Mat &src, const cv::Mat &lut);
#endif // WITH_OPENCV
};

class DllCoreExport DkImageStorage : public QObject
{
    Q_OBJECT

public:
    explicit DkImageStorage(const QImage &img = QImage());

    enum ComputeState {
        l_not_computed,
        l_computing,
        l_computed,
        l_empty,
        l_cancelled,

        l_end
    };

    bool isEmpty() const
    {
        return mImg.isNull();
    };

    QSize size() const
    {
        return mImg.size();
    };

    void setImage(const QImage &img);
    QImage imageConst() const;
    QImage image(const QSize &size = QSize());

public slots:
    void antiAliasingChanged(bool antiAliasing);
    void imageComputed();

signals:
    void imageUpdated() const;
    void infoSignal(const QString &msg) const;

protected:
    QImage mImg;
    QImage mScaledImg;

    QFutureWatcher<QImage> mFutureWatcher;

    ComputeState mComputeState = l_not_computed;

    void init();
    void compute(const QSize &size);
};

/**
 * Rotates the image clockwise by angle.
 *
 * Returns the original image if angle is 0.
 * Do matrix operations for angles 90, 180, or 270
 * to speed up the process.
 * Calls rotateImage() for other angles.
 *
 * The returned image might have a different format.
 */
QImage rotateImageFast(const QImage &img, double angle);

/**
 * Rotates the image clockwise by angle.
 *
 * Rotate the input image by painting on a new RGBA8888 QImage.
 */
QImage rotateImage(const QImage &img, double angle);
}
