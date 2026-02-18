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
#include <QColorSpace>
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

    /**
     * @brief deep copy of QImage as cv::Mat
     */
    static cv::Mat qImage2Mat(const QImage &img);

    /**
     * @brief deep copy of cv::Mat as QImage
     */
    static QImage mat2QImage(cv::Mat mat, const QImage &srcImg);

    static void mapGammaTable(cv::Mat &img, const QVector<uint16_t> &gammaTable);
    static void gammaToLinear(cv::Mat &img);
    static void linearToGamma(cv::Mat &img);
    static void logPolar(const cv::Mat &src,
                         cv::Mat &dst,
                         cv::Point2d center,
                         double scaleLog,
                         double angle,
                         double scale = 1.0);
    static QImage tinyPlanet(const QImage &img, double scaleLog, double angle, const QSize &size, bool invert = false);
#endif

    static QString getBufferSize(const QSize &imgSize, const int depth);
    static float getBufferSizeFloat(const QSize &imgSize, const int depth);
    static QImage resizeImage(const QImage &src,
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
    static QImage flipImage(const QImage &image, Qt::Orientations flags);

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
    static bool exposure(QImage &img, double exposure, double offset, double gamma);
    static QImage bgColor(const QImage &src, const QColor &col);
    static QByteArray extractImageFromDataStream(const QByteArray &ba,
                                                 const QByteArray &beginSignature = "‰PNG",
                                                 const QByteArray &endSignature = "END®B`‚",
                                                 bool debugOutput = false);
    static bool fixSamsungPanorama(QByteArray &ba);
    static int intFromByteArray(const QByteArray &ba, int pos);

    static QColorSpace targetColorSpace(const QWidget *widget);
    static QColorSpace loadIccProfile(const QString &filePath);
    static QColorSpace profileForId(int id);
    static QVector<std::pair<int, QColorSpace>> builtinProfiles();

    static QImage::Format targetFormat()
    {
        // Qt backing store is usually this format, the memory savings of 24-bit are not worthwhile
        return QImage::Format_ARGB32_Premultiplied;
    }

    /**
     * @brief get a target render format suitable for image pixel format
     * @param imageFormat QImage.format() usually
     * @note This is for optimizing offscreen rendering and colorspace conversion
     *       - The returned format can usually be colorspace converted in-place
     *       - The returned format has sufficient precision
     * @return
     */
    static QImage::Format renderFormat(QImage::Format imageFormat);

    /**
     * @brief convert image color space while avoiding memory allocations
     * @param target use this widget's screen's colorspace
     * @param img input, may be modified, hence non-const
     * @return converted image
     */
    static QImage convertToColorSpaceInPlace(const QWidget *target, QImage &img);
    static QImage convertToColorSpaceInPlace(const QColorSpace &target, QImage &img);
};

class DllCoreExport DkImageStorage : public QObject
{
    Q_OBJECT

public:
    DkImageStorage();
    virtual ~DkImageStorage();

    bool isEmpty() const
    {
        return mOriginal.isNull();
    }

    QSize size() const
    {
        return mOriginal.size();
    }

    QImage image() const
    {
        return mOriginal;
    }

    // compute and cache DkImage::alphaChannelUsed()
    bool alphaChannelUsed();

    /**
     * @brief change current image, discard cached result
     * @param img
     * @note We cannot simply construct a new DkImageStorage because
     *       the future watcher will detach from the future and the future
     *       will not be removed from the thread pool. Which would
     *       cause threads to pile up.
     */
    void setImage(const QImage &img);

    enum class ScaleFilter {
        invalid, // uninitialized
        nearest, // nearest neighbor/QImage::FastTransformation
        area, // average/CV_AREA/QImage::SmoothTransformation, no samples limit
        // area_x16, // avarage/CV_AREA but limit samples per pixel to 16 with nearest filter
        // algo_area_sharp, // area + sharpen a little
        // lanczos2
        // lanczos3
    };

    enum {
        process_async = 1, // background processing, emit imageUpdated() on completion
        process_sync = 2, // foreground processing
        process_fallback = 4, // return fastest possible scaled image instead of original
    };

    /**
     * @brief downsample image in background for screen painting
     * @param size size of the scaled image which must be < image().size()
     * @param target the intended paint target
     * @param options change behavior
     * @note when scaling is completed, emit imageUpdated(), at
     *       which time downsampled() may be called again to get the result
     * @return cached result or original image (see ProcessOption)
     */
    QImage downsampled(const QSize &size, const QWidget *target, int options = process_async) &;

signals:
    // emit after image() if result is non-null, call image() again to retrieve it
    void imageUpdated() const;

    // send a message when toggling AA
    void infoSignal(const QString &msg) const;

protected slots:
    void antiAliasingChanged(bool antiAliasing);
    void workerFinished();

protected:
    struct ScaledImage {
        QImage image;
        ScaleFilter filter = ScaleFilter::invalid;
        QColorSpace colorSpace; // requested value, may be different from result
    };

    static ScaledImage scaleImage(const QImage &src,
                                  const QSize &size,
                                  ScaleFilter filter,
                                  const QColorSpace &colorSpace,
                                  QImage::Format format);

    void cancelWorker();
    void startWorker(const QSize &size);

    QImage mOriginal{};
    ScaledImage mScaled{};

    QFutureWatcher<ScaledImage> mWorker{};
    bool mDiscardResult = false;
    bool mWorkerPending = false;

    enum {
        alpha_unknown = 0,
        alpha_unused = 1,
        alpha_used = 2
    } mAlphaState = alpha_unknown;
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
