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

#include "DkActionManager.h"
#include "DkMath.h"
#include "DkSettings.h"
#include "DkThumbs.h"
#include "DkTimer.h"

#include <QColorSpace>
#include <QIconEngine>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QSvgRenderer>
#include <QtConcurrentRun>
#include <qmath.h>

#ifdef WITH_OPENCV
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#endif

#include <cmath>

#if defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h> // needed since libraw 0.16
#endif

namespace nmc
{
// DkImage --------------------------------------------------------------------

/**
 * Returns a string with the buffer size of an image.
 * @param imgSize the image size
 * @param depth the image depth
 * @return QString a human readable string containing the buffer size
 **/
QString DkImage::getBufferSize(const QSize &imgSize, const int depth)
{
    double size = (double)imgSize.width() * (double)imgSize.height() * (double)(depth / 8.0f);
    QString sizeStr;
    qDebug() << "dimension: " << size;

    if (size >= 1024 * 1024 * 1024) {
        return QString::number(size / (1024.0f * 1024.0f * 1024.0f), 'f', 2) + " GB";
    } else if (size >= 1024 * 1024) {
        return QString::number(size / (1024.0f * 1024.0f), 'f', 2) + " MB";
    } else if (size >= 1024) {
        return QString::number(size / 1024.0f, 'f', 2) + " KB";
    } else {
        return QString::number(size, 'f', 2) + " B";
    }
}

/**
 * Returns a the buffer size of an image.
 * @param imgSize the image size
 * @param depth the image depth
 * @return buffer size in MB
 **/
float DkImage::getBufferSizeFloat(const QSize &imgSize, const int depth)
{
    double size = (double)imgSize.width() * (double)imgSize.height() * (double)(depth / 8.0f);
    QString sizeStr;

    return (float)size / (1024.0f * 1024.0f);
}

/**
 * This function resizes an image according to the interpolation method specified.
 * @param src the image to resize
 * @param newSize the new size
 * @param factor the resize factor
 * @param interpolation the interpolation method
 * @return QImage the resized image
 **/
QImage DkImage::resizeImage(const QImage &src,
                            const QSize &newSize,
                            double factor /* = 1.0 */,
                            int interpolation /* = ipl_cubic */,
                            bool correctGamma /* = true */)
{
    QSize nSize = newSize;

    // nothing to do
    if (src.size() == nSize && factor == 1.0)
        return src;

    if (factor != 1.0)
        nSize = QSize(qRound(src.width() * factor), qRound(src.height() * factor));

    if (nSize.width() < 1 || nSize.height() < 1) {
        return QImage();
    }

    Qt::TransformationMode iplQt = Qt::FastTransformation;
    switch (interpolation) {
    case ipl_nearest:
    case ipl_area:
        iplQt = Qt::FastTransformation;
        break;
    case ipl_linear:
    case ipl_cubic:
    case ipl_lanczos:
        iplQt = Qt::SmoothTransformation;
        break;
    }
#ifdef WITH_OPENCV

    int ipl = CV_INTER_CUBIC;
    switch (interpolation) {
    case ipl_nearest:
        ipl = CV_INTER_NN;
        break;
    case ipl_area:
        ipl = CV_INTER_AREA;
        break;
    case ipl_linear:
        ipl = CV_INTER_LINEAR;
        break;
    case ipl_cubic:
        ipl = CV_INTER_CUBIC;
        break;
    case ipl_lanczos:
        ipl = CV_INTER_LANCZOS4;
        break;
    }

    try {
        QImage qImg;
        cv::Mat resizeImage = DkImage::qImage2Mat(src);

        if (correctGamma) {
            resizeImage.convertTo(resizeImage, CV_16U, USHRT_MAX / 255.0f);
            DkImage::gammaToLinear(resizeImage);
        }

        // is the image convertible?
        if (resizeImage.empty()) {
            qImg = src.scaled(newSize, Qt::IgnoreAspectRatio, iplQt);
        } else {
            cv::Mat tmp;
            cv::resize(resizeImage, tmp, cv::Size(nSize.width(), nSize.height()), 0, 0, ipl);
            resizeImage = tmp;

            if (correctGamma) {
                DkImage::linearToGamma(resizeImage);
                resizeImage.convertTo(resizeImage, CV_8U, 255.0f / USHRT_MAX);
            }

            qImg = DkImage::mat2QImage(resizeImage, src);
        }

        return qImg;

    } catch (...) {
        return QImage();
    }

#else
    QImage qImg = src.copy();

    if (correctGamma)
        DkImage::gammaToLinear(qImg);

    qImg = qImg.scaled(nSize, Qt::IgnoreAspectRatio, iplQt);

    if (correctGamma)
        DkImage::linearToGamma(qImg);
    return qImg;
#endif
}

// return true if all values masked with mask == value
// optimal version for packed pixel formats
template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
static bool isMaskedEqual(const QImage &img, T mask, T value)
{
    Q_ASSERT(static_cast<size_t>(img.depth()) == sizeof(T) * 8);
    auto *bits = reinterpret_cast<const T *>(img.constBits());
    int stride = img.bytesPerLine() / sizeof(T);
    const int height = img.height();
    const int width = img.width();
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            if ((bits[w] & mask) != value) {
                return false;
            }
        }
        bits += stride;
    }
    return true;
}

// return true if all values in a channel equal value
template<typename T>
static bool isChannelEqual(const QImage &img, int channel, int numChannels, T value)
{
    Q_ASSERT(channel >= 0 && channel < numChannels);
    Q_ASSERT(static_cast<size_t>(img.depth()) == sizeof(T) * 8 * numChannels);
    auto *channels = reinterpret_cast<const T *>(img.constBits());
    int stride = img.bytesPerLine() / sizeof(T);
    const int height = img.height();
    const int width = img.width();
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            if (channels[w * numChannels + channel] != value) {
                return false;
            }
        }
        channels += stride;
    }
    return true;
}

bool DkImage::alphaChannelUsed(const QImage &img)
{
    if (!img.hasAlphaChannel()) {
        return false;
    }

    switch (img.format()) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        return !isMaskedEqual<uint32_t>(img, 0xFF000000, 0xFF000000);
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied: {
        constexpr bool littleEndian = Q_BYTE_ORDER != Q_BIG_ENDIAN;
        constexpr uint32_t mask = littleEndian ? 0xFF : 0xFF000000;
        return !isMaskedEqual<uint32_t>(img, mask, mask);
    }
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
        return !isMaskedEqual<uint64_t>(img, 0xFFFF, 0xFFFF);
    case QImage::Format_Indexed8: {
        const QList<QRgb> colorTable = img.colorTable();
        for (auto &rgb : colorTable) {
            if (qAlpha(rgb) != 0xFF) {
                return true;
            }
        }
        return false;
    }
    case QImage::Format_RGBA16FPx4:
    case QImage::Format_RGBA16FPx4_Premultiplied: {
        return !isChannelEqual<uint16_t>(img, 3, 4, 0x3C00);
    }
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBA32FPx4_Premultiplied: {
        return !isChannelEqual<float>(img, 3, 4, 1.0f);
    } break;
    default:
        // supposedly Qt image loaders will never give 16-bit or 24-bit images w/alpha channel,
        // although there is format support for it
        qWarning() << "[alphaChannelUsed] unsupported format" << img.format();
    }

    return false;
}

QImage DkImage::thresholdImage(const QImage &img, double thr, bool color)
{
    if (img.isNull())
        return img;

    DkTimer dt;

    QImage tImg = color ? img.copy() : grayscaleImage(img);

    // number of bytes per line used
    int bpl = (tImg.width() * tImg.depth() + 7) / 8;
    int pad = tImg.bytesPerLine() - bpl;

    uchar *mPtr = tImg.bits();

    for (int rIdx = 0; rIdx < tImg.height(); rIdx++) {
        for (int cIdx = 0; cIdx < bpl; cIdx++, mPtr++) {
            *mPtr = *mPtr > thr ? 255 : 0;
        }
        mPtr += pad;
    }

    qDebug() << "thresholding takes: " << dt;

    return tImg;
}

QImage DkImage::flipImage(const QImage &image, Qt::Orientations flags)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    return image.flipped(flags);
#else
    return image.mirrored(flags & Qt::Horizontal, flags & Qt::Vertical);
#endif
}

QImage DkImage::rotateImage(const QImage &img, double angle)
{
    return rotateImageFast(img, angle);
}

QImage rotateImage(const QImage &img, double angle)
{
    // compute new image size
    DkVector nSl((float)img.width(), (float)img.height());
    DkVector nSr = nSl;
    double angleRad = angle * DK_DEG2RAD;

    // size left
    nSl.rotate(angleRad);
    nSl.abs();

    // size right
    nSr.swap();
    nSr.rotate(angleRad);
    nSr.abs();
    nSr.swap();

    DkVector ns = nSl.maxVec(nSr);
    QSize newSize((int)ns.width, (int)ns.height);

    // create image
    QImage imgR(newSize, QImage::Format_RGBA8888);
    imgR.setColorSpace(img.colorSpace());
    imgR.fill(Qt::transparent);

    // create transformation
    QTransform trans;
    trans.translate(imgR.width() / 2, imgR.height() / 2);
    trans.rotate(angle);
    trans.translate(-img.width() / 2, -img.height() / 2);

    // render
    QPainter p(&imgR);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setTransform(trans);
    p.drawImage(QPoint(), img);

    return imgR;
}

template<typename T>
QImage transposeImage(const QImage &imgIn)
{
    QImage imgOut = QImage(imgIn.size().transposed(), imgIn.format());
    imgOut.setColorTable(imgIn.colorTable());
    imgOut.setColorSpace(imgIn.colorSpace());

    const int h = imgIn.height();
    const int w = imgIn.width();
    const uchar *ptrIn = imgIn.constBits();
    const int outBPL = imgOut.bytesPerLine();
    const int inBPL = imgIn.bytesPerLine();
    for (int i = 0; i < h; i++) {
        const T *lineIn = reinterpret_cast<const T *>(ptrIn);
        auto *ptrOut = const_cast<uchar *>(imgOut.constBits() + i * sizeof(T));
        for (int j = 0; j < w; j++) {
            T *out = reinterpret_cast<T *>(ptrOut);
            *out = lineIn[j];
            ptrOut += outBPL;
        }
        ptrIn += inBPL;
    }

    return imgOut;
}

QImage transposeImage24(const QImage &imgIn)
{
    // NOTE: this implementation is more general (any depth that is multiple of 8 can be handled),
    // but somehow several times slower than the above transposeImage.

    QImage imgOut = QImage(imgIn.size().transposed(), imgIn.format());
    imgOut.setColorTable(imgIn.colorTable());
    imgOut.setColorSpace(imgIn.colorSpace());

    const int h = imgIn.height();
    const int w = imgIn.width();
    const int outBPL = imgOut.bytesPerLine();
    uchar *outBits = imgOut.bits();
    const int charPerPixel = imgIn.depth() / CHAR_BIT;
    for (int i = 0; i < h; i++) {
        const uchar *ptrIn = imgIn.constScanLine(i);
        uchar *ptrOut = outBits + i * charPerPixel;
        for (int j = 0; j < w; j++) {
            memcpy(ptrOut + outBPL * j, ptrIn + charPerPixel * j, charPerPixel);
        }
    }

    return imgOut;
}

#ifdef WITH_OPENCV
QImage rotateImageCVMat(const QImage &imgIn, cv::RotateFlags rot, int type)
{
    QSize size = rot == cv::ROTATE_180 ? imgIn.size() : imgIn.size().transposed();
    QImage imgOut = QImage(size, imgIn.format());
    imgOut.setColorTable(imgIn.colorTable());
    imgOut.setColorSpace(imgIn.colorSpace());
    imgOut.setDotsPerMeterX(imgIn.dotsPerMeterX());
    imgOut.setDotsPerMeterY(imgIn.dotsPerMeterY());
    imgOut.setDevicePixelRatio(imgIn.devicePixelRatio());
    for (auto &key : imgIn.textKeys())
        imgOut.setText(key, imgIn.text(key));

    const cv::Mat matIn = cv::Mat(imgIn.height(),
                                  imgIn.width(),
                                  type,
                                  (uchar *)imgIn.constBits(),
                                  imgIn.bytesPerLine());
    cv::Mat matOut = cv::Mat(imgOut.height(), imgOut.width(), type, imgOut.bits(), imgOut.bytesPerLine());

    cv::rotate(matIn, matOut, rot);
    return imgOut;
}
#endif

QImage rotateImageFast(const QImage &img, double angle)
{
    angle = std::fmod(angle, 360);
    if (angle < 0) {
        angle += 360;
    }

    if (angle == 0) {
        return img;
    }

    if (angle == 90 || angle == 180 || angle == 270) {
        // Fast path

#ifdef WITH_OPENCV
        cv::RotateFlags rot{};
        if (angle == 180) {
            rot = cv::ROTATE_180;
        } else if (angle == 90) {
            rot = cv::ROTATE_90_CLOCKWISE;
        } else {
            rot = cv::ROTATE_90_COUNTERCLOCKWISE;
        }

        // The pixel depth matters, but the color interpretation does not.
        switch (img.depth()) {
        case 1:
            return rotateImageCVMat(img.convertToFormat(QImage::Format_Indexed8), rot, CV_8U);
        case 8:
            return rotateImageCVMat(img, rot, CV_8U);
        case 16:
            return rotateImageCVMat(img, rot, CV_16U);
        case 24:
            return rotateImageCVMat(img, rot, CV_8UC3);
        case 32:
            return rotateImageCVMat(img, rot, CV_32F);
        case 64:
            return rotateImageCVMat(img, rot, CV_64F);
        default:
            return rotateImageCVMat(img.convertToFormat(QImage::Format_ARGB32), rot, CV_32F);
        }
#else
        if (angle == 180) {
            return DkImage::flipImage(img, Qt::Horizontal | Qt::Vertical);
        }

        QImage imgIn = img;

        if (angle == 90) {
            switch (imgIn.depth()) {
            case 1:
                imgIn = imgIn.convertToFormat(QImage::Format_Indexed8);
                imgIn = transposeImage<uint8_t>(imgIn);
                break;
            case 8:
                imgIn = transposeImage<uint8_t>(imgIn);
                break;
            case 16:
                imgIn = transposeImage<uint16_t>(imgIn);
                break;
            case 24:
                imgIn = transposeImage24(imgIn);
                break;
            case 32:
                imgIn = transposeImage<uint32_t>(imgIn);
                break;
            case 64:
                imgIn = transposeImage<uint64_t>(imgIn);
                break;
            default:
                imgIn = imgIn.convertToFormat(QImage::Format_ARGB32);
                return transposeImage<uint32_t>(imgIn);
            }

            return DkImage::flipImage(imgIn, Qt::Horizontal);
        }

        if (angle == 270) {
            imgIn = DkImage::flipImage(imgIn, Qt::Horizontal);

            switch (imgIn.depth()) {
            case 1:
                imgIn = imgIn.convertToFormat(QImage::Format_Indexed8);
                return transposeImage<uint8_t>(imgIn);
            case 8:
                return transposeImage<uint8_t>(imgIn);
            case 16:
                return transposeImage<uint16_t>(imgIn);
            case 24:
                return transposeImage24(imgIn);
            case 32:
                return transposeImage<uint32_t>(imgIn);
            case 64:
                return transposeImage<uint64_t>(imgIn);
            default:
                imgIn = imgIn.convertToFormat(QImage::Format_ARGB32);
                return transposeImage<uint32_t>(imgIn);
            }
        }
#endif
    }

    return rotateImage(img, angle);
}

QImage DkImage::grayscaleImage(const QImage &img)
{
    QImage imgR;

#ifdef WITH_OPENCV

    cv::Mat cvImg = DkImage::qImage2Mat(img);
    cv::cvtColor(cvImg, cvImg, CV_RGB2Lab);

    std::vector<cv::Mat> imgs;
    cv::split(cvImg, imgs);

    // get the luminance channel
    if (!imgs.empty())
        cvImg = imgs[0];

    // convert it back for the painter
    cv::cvtColor(cvImg, cvImg, CV_GRAY2RGB);

    imgR = DkImage::mat2QImage(cvImg, img);
#else

    QVector<QRgb> table(256);
    for (int i = 0; i < 256; ++i)
        table[i] = qRgb(i, i, i);

    imgR = img.convertToFormat(QImage::Format_Indexed8, table);
#endif

    return imgR;
}

template<typename numFmt>
QVector<numFmt> DkImage::getLinear2GammaTable(int maxVal)
{
    QVector<numFmt> gammaTable;
    double a = 0.055;

    for (int idx = 0; idx <= maxVal; idx++) {
        double i = idx / (double)maxVal;
        if (i <= 0.0031308) {
            gammaTable.append((numFmt)(qRound(i * 12.92 * (double)maxVal)));
        } else {
            gammaTable.append((numFmt)(qRound(((1 + a) * pow(i, 1 / 2.4) - a) * (double)maxVal)));
        }
    }

    return gammaTable;
}

template<typename numFmt>
QVector<numFmt> DkImage::getGamma2LinearTable(int maxVal)
{
    // the formula should be:
    // i = px/255
    // i <= 0.04045 -> i/12.92
    // i > 0.04045 -> (i+0.055)/(1+0.055)^2.4
    QVector<numFmt> gammaTable;
    double a = 0.055;

    for (int idx = 0; idx <= maxVal; idx++) {
        double i = idx / (double)maxVal;
        if (i <= 0.04045) {
            gammaTable.append((numFmt)(qRound(i / 12.92 * maxVal)));
        } else {
            gammaTable.append(pow((i + a) / (1 + a), 2.4) * maxVal > 0 ? (numFmt)(pow((i + a) / (1 + a), 2.4) * maxVal)
                                                                       : 0);
        }
    }

    return gammaTable;
}

void DkImage::gammaToLinear(QImage &img)
{
    QVector<uchar> gt = getGamma2LinearTable<uchar>(255);
    mapGammaTable(img, gt);
}

void DkImage::linearToGamma(QImage &img)
{
    QVector<uchar> gt = getLinear2GammaTable<uchar>(255);
    mapGammaTable(img, gt);
}

void DkImage::mapGammaTable(QImage &img, const QVector<uchar> &gammaTable)
{
    if (gammaTable.size() != 1 << 8) {
        qCritical() << "invalid 8-bit gamma table";
        return;
    }

    // number of bytes per line used
    int bpl = (img.width() * img.depth() + 7) / 8;
    int pad = img.bytesPerLine() - bpl;

    // int channels = (img.hasAlphaChannel() || img.format() == QImage::Format_RGB32) ? 4 : 3;

    uchar *mPtr = img.bits();

    for (int rIdx = 0; rIdx < img.height(); rIdx++) {
        for (int cIdx = 0; cIdx < bpl; cIdx++, mPtr++) {
            *mPtr = gammaTable[*mPtr];
        }
        mPtr += pad;
    }
}

bool DkImage::normImage(QImage &img)
{
    uchar maxVal = 0;
    uchar minVal = 255;

    // number of used bytes per line
    int bpl = (img.width() * img.depth() + 7) / 8;
    int pad = img.bytesPerLine() - bpl;
    uchar *mPtr = img.bits();
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

    if ((minVal == 0 && maxVal == 255) || maxVal - minVal == 0)
        return false;

    uchar *ptr = img.bits();

    for (int rIdx = 0; rIdx < img.height(); rIdx++) {
        for (int cIdx = 0; cIdx < bpl; cIdx++, ptr++) {
            if (hasAlpha && cIdx % 4 == 3)
                continue;

            *ptr = (uchar)qRound(255.0f * (*ptr - minVal) / (maxVal - minVal));
        }

        ptr += pad;
    }

    return true;
}

bool DkImage::autoAdjustImage(QImage &img)
{
    // return DkImage::unsharpMask(img, 30.0f, 1.5f);

    DkTimer dt;
    qDebug() << "[Auto Adjust] image format: " << img.format();

    // for grayscale image - normalize is the same
    if (img.format() <= QImage::Format_Indexed8) {
        qDebug() << "[Auto Adjust] Grayscale - switching to Normalize: " << img.format();
        return normImage(img);
    } else if (img.format() != QImage::Format_ARGB32 && img.format() != QImage::Format_RGB32
               && img.format() != QImage::Format_RGB888) {
        qDebug() << "[Auto Adjust] Format not supported: " << img.format();
        return false;
    }

    int channels = (img.hasAlphaChannel() || img.format() == QImage::Format_RGB32) ? 4 : 3;

    uchar maxR = 0, maxG = 0, maxB = 0;
    uchar minR = 255, minG = 255, minB = 255;

    // number of bytes per line used
    int bpl = (img.width() * img.depth() + 7) / 8;
    int pad = img.bytesPerLine() - bpl;

    uchar *mPtr = img.bits();
    uchar r, g, b;

    int histR[256] = {0};
    int histG[256] = {0};
    int histB[256] = {0};

    for (int rIdx = 0; rIdx < img.height(); rIdx++) {
        for (int cIdx = 0; cIdx < bpl;) {
            r = *mPtr;
            mPtr++;
            g = *mPtr;
            mPtr++;
            b = *mPtr;
            mPtr++;
            cIdx += 3;

            if (r > maxR)
                maxR = r;
            if (r < minR)
                minR = r;

            if (g > maxG)
                maxG = g;
            if (g < minG)
                minG = g;

            if (b > maxB)
                maxB = b;
            if (b < minB)
                minB = b;

            histR[r]++;
            histG[g]++;
            histB[b]++;

            // ?? strange but I would expect the alpha channel to be the first (big endian?)
            if (channels == 4) {
                mPtr++;
                cIdx++;
            }
        }
        mPtr += pad;
    }

    bool ignoreR = maxR - minR == 0 || maxR - minR == 255;
    bool ignoreG = maxR - minR == 0 || maxG - minG == 255;
    bool ignoreB = maxR - minR == 0 || maxB - minB == 255;

    uchar *ptr = img.bits();

    if (ignoreR) {
        maxR = findHistPeak(histR);
        ignoreR = maxR - minR == 0 || maxR - minR == 255;
    }
    if (ignoreG) {
        maxG = findHistPeak(histG);
        ignoreG = maxG - minG == 0 || maxG - minG == 255;
    }
    if (ignoreB) {
        maxB = findHistPeak(histB);
        ignoreB = maxB - minB == 0 || maxB - minB == 255;
    }

    // qDebug() << "red max: " << maxR << " min: " << minR << " ignored: " << ignoreR;
    // qDebug() << "green max: " << maxG << " min: " << minG << " ignored: " << ignoreG;
    // qDebug() << "blue max: " << maxB << " min: " << minB << " ignored: " << ignoreB;
    // qDebug() << "computed in: " << dt;

    if (ignoreR && ignoreG && ignoreB) {
        qDebug() << "[Auto Adjust] There is no need to adjust the image";
        return false;
    }

    for (int rIdx = 0; rIdx < img.height(); rIdx++) {
        for (int cIdx = 0; cIdx < bpl;) {
            // don't check values - speed (but you see under-/overflows anyway)
            if (!ignoreR && *ptr < maxR)
                *ptr = (uchar)qRound(255.0f * ((float)*ptr - minR) / (maxR - minR));
            else if (!ignoreR)
                *ptr = 255;

            ptr++;
            cIdx++;

            if (!ignoreG && *ptr < maxG)
                *ptr = (uchar)qRound(255.0f * ((float)*ptr - minG) / (maxG - minG));
            else if (!ignoreG)
                *ptr = 255;

            ptr++;
            cIdx++;

            if (!ignoreB && *ptr < maxB)
                *ptr = (uchar)qRound(255.0f * ((float)*ptr - minB) / (maxB - minB));
            else if (!ignoreB)
                *ptr = 255;
            ptr++;
            cIdx++;

            if (channels == 4) {
                ptr++;
                cIdx++;
            }
        }
        ptr += pad;
    }

    qDebug() << "[Auto Adjust] image adjusted in: " << dt;

    return true;
}

uchar DkImage::findHistPeak(const int *hist, float quantile)
{
    int histArea = 0;

    for (int idx = 0; idx < 256; idx++)
        histArea += hist[idx];

    int sumBins = 0;

    for (int idx = 255; idx >= 0; idx--) {
        sumBins += hist[idx];

        if (sumBins / (float)histArea > quantile) {
            qDebug() << "max bin: " << idx;
            return (uchar)idx;
        }
    }

    qDebug() << "no max bin found... sum: " << sumBins;

    return 255;
}

QPixmap DkImage::makeSquare(const QPixmap &pm)
{
    QRect r(QPoint(), pm.size());

    if (r.width() > r.height()) {
        r.setX(qFloor((r.width() - r.height()) * 0.5f));
        r.setWidth(r.height());
    } else {
        r.setY(qFloor((r.height() - r.width()) * 0.5f));
        r.setHeight(r.width());
    }

    return pm.copy(r);
}

QPixmap DkImage::merge(const QVector<QImage> &imgs)
{
    if (imgs.size() > 10) {
        qWarning() << "DkImage::merge is built for a small amount of images, you gave me: " << imgs.size();
    }

    QPixmap pm;
    int margin = 10;
    int x = 0;
    QPainter p;

    for (const QImage &img : imgs) {
        // init on first
        if (pm.isNull()) {
            pm = QPixmap(img.height() * imgs.size() + margin * (imgs.size() - 1), img.height());
            pm.fill(QColor(0, 0, 0, 0));

            p.begin(&pm);
        }

        QPixmap cpm = DkImage::makeSquare(QPixmap::fromImage(img));
        QRect r(QPoint(x, 0), QSize(pm.height(), pm.height()));
        p.drawPixmap(r, cpm);
        x += r.width() + margin;
    }

    return pm;
}

QImage DkImage::cropToImage(const QImage &src, const DkRotatingRect &rect, const QColor &fillColor)
{
    QTransform tForm;
    QPointF cImgSize;
    rect.getTransform(tForm, cImgSize);

    // illegal?
    if (cImgSize.x() < 0.5f || cImgSize.y() < 0.5f)
        return src;

    double angle = DkMath::normAngleRad(rect.getAngle(), 0, CV_PI * 0.5);
    double minD = qMin(std::abs(angle), std::abs(angle - CV_PI * 0.5));

    QImage img = QImage(qRound(cImgSize.x()), qRound(cImgSize.y()), QImage::Format_ARGB32);
    img.setColorSpace(src.colorSpace());
    img.fill(fillColor.rgba());

    // render the image into the new coordinate system
    QPainter painter(&img);
    painter.setWorldTransform(tForm);

    // for rotated rects we want perfect anti-aliasing
    if (minD > FLT_EPSILON)
        painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

    painter.drawImage(QRect(QPoint(), src.size()), src, QRect(QPoint(), src.size()));
    painter.end();

    return img;
}

QImage DkImage::hueSaturation(const QImage &src, int hue, int sat, int brightness)
{
    // nothing to do?
    if (hue == 0 && sat == 0 && brightness == 0)
        return src;

    QImage imgR;

#ifdef WITH_OPENCV

    // normalize brightness/saturation
    int brightnessN = qRound(brightness / 100.0 * 255.0);
    double satN = sat / 100.0 + 1.0;

    cv::Mat hsvImg = DkImage::qImage2Mat(src);

    if (hsvImg.channels() > 3)
        cv::cvtColor(hsvImg, hsvImg, CV_RGBA2BGR);

    cv::cvtColor(hsvImg, hsvImg, CV_BGR2HSV);

    // apply hue/saturation changes
    for (int rIdx = 0; rIdx < hsvImg.rows; rIdx++) {
        auto *iPtr = hsvImg.ptr<unsigned char>(rIdx);

        for (int cIdx = 0; cIdx < hsvImg.cols * 3; cIdx += 3) {
            // adopt hue
            int h = iPtr[cIdx] + hue;
            if (h < 0)
                h += 180;
            if (h >= 180)
                h -= 180;

            iPtr[cIdx] = (unsigned char)h;

            // adopt value
            int v = iPtr[cIdx + 2] + brightnessN;
            if (v < 0)
                v = 0;
            if (v > 255)
                v = 255;
            iPtr[cIdx + 2] = (unsigned char)v;

            // adopt saturation
            int s = qRound(iPtr[cIdx + 1] * satN);
            if (s < 0)
                s = 0;
            if (s > 255)
                s = 255;
            iPtr[cIdx + 1] = (unsigned char)s;
        }
    }

    cv::cvtColor(hsvImg, hsvImg, CV_HSV2BGR);
    imgR = DkImage::mat2QImage(hsvImg, src);

#endif // WITH_OPENCV

    return imgR;
}

QImage DkImage::exposure(const QImage &src, double exposure, double offset, double gamma)
{
    if (exposure == 0.0 && offset == 0.0 && gamma == 1.0)
        return src;

    QImage imgR;
#ifdef WITH_OPENCV

    cv::Mat rgbImg = DkImage::qImage2Mat(src);
    rgbImg.convertTo(rgbImg, CV_16U, 256, offset * std::numeric_limits<unsigned short>::max());

    if (rgbImg.channels() > 3)
        cv::cvtColor(rgbImg, rgbImg, CV_RGBA2BGR);

    if (exposure != 0.0)
        rgbImg = exposureMat(rgbImg, exposure);

    if (gamma != 1.0)
        rgbImg = gammaMat(rgbImg, gamma);

    rgbImg.convertTo(rgbImg, CV_8U, 1.0 / 256.0);
    imgR = DkImage::mat2QImage(rgbImg, src);

#endif // WITH_OPENCV

    return imgR;
}

QImage DkImage::bgColor(const QImage &src, const QColor &col)
{
    QImage dst(src.size(), QImage::Format_RGB32);
    dst.setColorSpace(src.colorSpace());
    dst.fill(col);

    QPainter p(&dst);
    p.drawImage(QPoint(0, 0), src);

    return dst;
}

QByteArray DkImage::extractImageFromDataStream(const QByteArray &ba,
                                               const QByteArray &beginSignature,
                                               const QByteArray &endSignature,
                                               bool debugOutput)
{
    int bIdx = ba.indexOf(beginSignature);

    if (bIdx == -1) {
        qDebug() << "[ExtractImage] could not locate" << beginSignature;
        return QByteArray();
    }

    int eIdx = ba.indexOf(endSignature, bIdx);

    if (eIdx == -1) {
        qDebug() << "[ExtractImage] could not locate" << endSignature;
        return QByteArray();
    }

    QByteArray bac = ba.mid(bIdx, eIdx + endSignature.size() - bIdx);

    if (debugOutput) {
        qDebug() << "extracting image from stream...";
        qDebug() << "cropping: [" << bIdx << eIdx << "]";
        qDebug() << "original size: " << ba.size() / 1024.0 << "KB" << "new size: " << bac.size() / 1024.0 << "KB"
                 << "difference:" << (ba.size() - bac.size()) / 1024 << "KB";
    }

    return bac;
}

bool DkImage::fixSamsungPanorama(QByteArray &ba)
{
    // this code is based on python code from bcyrill
    // see: https://gist.github.com/bcyrill/e59fda6c7ffe23c7c4b08a990804b269
    // it fixes SAMSUNG panorama images that are not standard conformant by adding an EOI marker to the QByteArray
    // see also: https://github.com/nomacs/nomacs/issues/254

    if (ba.size() < 8)
        return false;

    QByteArray trailer = ba.right(4);

    // is it a samsung panorama jpg?
    if (trailer != QByteArray("SEFT"))
        return false;

    // TODO saveify:
    int sefhPos = intFromByteArray(ba, ba.size() - 8) + 8;
    trailer = ba.right(sefhPos);

    // trailer starts with "SEFH"?
    if (trailer.left(4) != QByteArray("SEFH"))
        return false;

    int endPos = ba.size();
    int dirPos = endPos - sefhPos;

    int count = intFromByteArray(trailer, 8);

    int firstBlock = 0;
    bool isPano = false;

    for (int idx = 0; idx < count; idx++) {
        int e = 12 + 12 * idx;

        int noff = intFromByteArray(trailer, e + 4);
        int size = intFromByteArray(trailer, e + 8);

        if (firstBlock < noff)
            firstBlock = noff;

        QByteArray cdata = ba.mid(dirPos - noff, size);

        int eoff = intFromByteArray(cdata, 4);
        QString pi = cdata.mid(8, eoff);

        if (pi == "Panorama_Shot_Info")
            isPano = true;
    }

    if (!isPano)
        return false;

    int dataPos = dirPos - firstBlock;

    // ok, append the missing marker
    QByteArray nb;
    nb.append(ba.left(dataPos));
    nb.append(QByteArray("\xff\xd9"));
    nb.append(ba.right(dataPos));
    qDebug() << "SAMSUNG panorma fix: EOI marker injected";

    ba = nb;
    return true;
}

int DkImage::intFromByteArray(const QByteArray &ba, int pos)
{
    // TODO saveify:
    QByteArray tmp = ba.mid(pos, 4);
    const int *val = (const int *)(tmp.constData());

    return *val;
}

#ifdef WITH_OPENCV
cv::Mat DkImage::exposureMat(const cv::Mat &src, double exposure)
{
    int maxVal = std::numeric_limits<unsigned short>::max();
    cv::Mat lut(1, maxVal + 1, CV_16UC1);

    double smooth = 0.5;
    double cStops = std::log(exposure) / std::log(2.0);
    double range = cStops * 2.0;
    double linRange = std::pow(2.0, range);
    double x1 = (maxVal + 1.0) / linRange - 1.0;
    double y1 = x1 * exposure;
    double y2 = maxVal * (1.0 + (1.0 - smooth) * (exposure - 1.0));
    double sq3x = std::pow(x1 * x1 * maxVal, 1.0 / 3.0);
    double B = (y2 - y1 + exposure * (3.0 * x1 - 3.0 * sq3x)) / (maxVal + 2.0 * x1 - 3.0 * sq3x);
    double A = (exposure - B) * 3.0 * std::pow(x1 * x1, 1.0 / 3.0);
    double CC = y2 - A * std::pow(maxVal, 1.0 / 3.0) - B * maxVal;

    for (int rIdx = 0; rIdx < lut.rows; rIdx++) {
        auto *ptrLut = lut.ptr<unsigned short>(rIdx);

        for (int cIdx = 0; cIdx < lut.cols; cIdx++) {
            double val = cIdx;
            double valE = 0.0;

            if (exposure < 1.0) {
                valE = val * std::exp(exposure / 10.0); // /10 - make it slower -> we go down till -20
            } else if (cIdx < x1) {
                valE = val * exposure;
            } else {
                valE = A * std::pow(val, 1.0 / 3.0) + B * val + CC;
            }

            if (valE < 0)
                ptrLut[cIdx] = 0;
            else if (valE > maxVal)
                ptrLut[cIdx] = (unsigned short)maxVal;
            else
                ptrLut[cIdx] = (unsigned short)qRound(valE);
        }
    }

    return applyLUT(src, lut);
}

cv::Mat DkImage::gammaMat(const cv::Mat &src, double gamma)
{
    int maxVal = std::numeric_limits<unsigned short>::max();
    cv::Mat lut(1, maxVal + 1, CV_16UC1);

    for (int rIdx = 0; rIdx < lut.rows; rIdx++) {
        auto *ptrLut = lut.ptr<unsigned short>(rIdx);

        for (int cIdx = 0; cIdx < lut.cols; cIdx++) {
            double val = std::pow((double)cIdx / maxVal, 1.0 / gamma) * maxVal;
            ptrLut[cIdx] = (unsigned short)qRound(val);
        }
    }

    return applyLUT(src, lut);
}

cv::Mat DkImage::applyLUT(const cv::Mat &src, const cv::Mat &lut)
{
    if (src.depth() != lut.depth()) {
        qCritical() << "cannot apply LUT!";
        return cv::Mat();
    }

    cv::Mat dst = src.clone();
    const auto *lutPtr = lut.ptr<unsigned short>();

    for (int rIdx = 0; rIdx < src.rows; rIdx++) {
        auto *dPtr = dst.ptr<unsigned short>(rIdx);

        for (int cIdx = 0; cIdx < src.cols * src.channels(); cIdx++) {
            assert(dPtr[cIdx] >= 0 && dPtr[cIdx] < lut.cols);
            dPtr[cIdx] = lutPtr[dPtr[cIdx]];
        }
    }

    return dst;
}
#endif // WITH_OPENCV

QColorSpace DkImage::targetColorSpace(const QWidget *widget)
{
    // TODO: add an "Auto" setting and use platform-specific APIs or wait for Qt to get one
    Q_UNUSED(widget)

    // This could be slow to read from disk so keep a cache
    static int targetId = 0;
    static QColorSpace colorSpace;

    const auto &dpy = DkSettingsManager::param().display();

    if (targetId != dpy.targetColorSpace) {
        targetId = dpy.targetColorSpace;
        colorSpace = profileForId(targetId);
    }

    return colorSpace;
}

QColorSpace DkImage::loadIccProfile(const QString &filePath)
{
    if (filePath.isEmpty()) {
        qWarning() << "[loadIccProfile] empty file path";
        return {};
    }

    QFile profile(filePath);
    if (!profile.open(QFile::ReadOnly)) {
        qWarning() << "[loadIccProfile] open failed" << profile.error() << profile.errorString() << filePath;
        return {};
    }

    auto colorSpace = QColorSpace::fromIccProfile(profile.readAll());
    if (!colorSpace.isValid()) {
        qWarning() << "[loadIccProfile] invalid color profile" << filePath;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    else if (!colorSpace.isValidTarget()) {
        qWarning() << "[loadIccProfile] unsupported for output" << filePath;
    }
#endif

    return colorSpace;
}

QColorSpace DkImage::profileForId(int id)
{
    QColorSpace colorSpace;
    if (id >= 0 && id < 100) {
        const auto profiles = builtinProfiles();
        auto it = std::find_if(profiles.begin(), profiles.end(), [id](auto &pair) {
            return pair.first == id;
        });
        if (it != profiles.end()) {
            colorSpace = it->second;
        }
    } else if (id < 1000) {
        int iccIndex = id - 100;
        colorSpace = loadIccProfile(DkSettingsManager::param().display().iccProfiles.value(iccIndex));
    }
    return colorSpace;
}

QVector<std::pair<int, QColorSpace>> DkImage::builtinProfiles()
{
    QColorSpace unmanaged{};
    unmanaged.setDescription(QObject::tr("Unmanaged"));

    // sRGB with true gamma as most displays do not implement sRGB transfer function
    QColorSpace srgbGamma22{QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::Gamma, 2.2f};
    srgbGamma22.setDescription("sRGB (Gamma 2.2)");

    return {{0, unmanaged},
            {QColorSpace::SRgb, QColorSpace{QColorSpace::SRgb}},
            {50, srgbGamma22},
            {QColorSpace::DisplayP3, QColorSpace{QColorSpace::DisplayP3}},
            {QColorSpace::AdobeRgb, QColorSpace{QColorSpace::AdobeRgb}}};
}

QPixmap DkImage::colorizePixmap(const QPixmap &icon, const QColor &col, float opacity)
{
    if (icon.isNull())
        return icon;

    QPixmap glow = icon.copy();
    QPixmap sGlow = glow.copy();
    sGlow.fill(col);

    QPainter painter(&glow);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn); // check if this is the right composition mode
    painter.setOpacity(opacity);
    painter.drawPixmap(glow.rect(), sGlow);

    return glow;
}

QPixmap DkImage::loadIcon(const QString &filePath, const QSize &size, const QColor &col)
{
    if (filePath.isEmpty())
        return QPixmap();

    QSize s = size * DkSettingsManager::param().dpiScaleFactor();
    if (size.isEmpty()) {
        int eis = DkSettingsManager::param().effectiveIconSize();
        s = QSize(eis, eis);
    }

    QPixmap icon = loadFromSvg(filePath, s);

    QColor c = (col.isValid()) ? col : DkSettingsManager::param().display().iconColor;

    if (c.alpha() != 0)
        icon = colorizePixmap(icon, c);

    return icon;
}

QPixmap DkImage::loadIcon(const QString &filePath, const QColor &col, const QSize &size)
{
    QSize is = size;

    if (is.isNull()) {
        int s = DkSettingsManager::param().effectiveIconSize();
        is = QSize(s, s);
    }

    QPixmap icon = loadFromSvg(filePath, is);
    icon = colorizePixmap(icon, col);

    return icon;
}

QPixmap DkImage::loadFromSvg(const QString &filePath, const QSize &size)
{
    QSharedPointer<QSvgRenderer> svg(new QSvgRenderer(filePath));

    QPixmap pm(size);
    pm.fill(QColor(0, 0, 0, 0)); // clear background

    QPainter p(&pm);
    svg->render(&p);

    return pm;
}

class DkSvgIconEngine : public QIconEngine
{
public:
    DkSvgIconEngine(const QString &filePath, const QColor &color)
        : mColor(color)
    {
        static_assert(QIcon::State::Off + QIcon::State::On == 1);
        mFiles.resize(2); // On/Off state only; 0 or 1
        mFiles[QIcon::State::Off] = filePath;
    }

private:
    DkSvgIconEngine() = default;

    QString iconName() override
    {
        // return the default icon file path via QIcon::name()
        return mFiles[QIcon::State::Off];
    }

    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        Q_UNUSED(size) // not supported; we always render the requested size
        Q_UNUSED(mode) // not supported; modes besides normal are automagically generated
        mFiles[state & 1] = fileName;
    }

    void paint(QPainter *painter, const QRect &bounds, QIcon::Mode mode, QIcon::State state) override
    {
        QPixmap pix = pixmap(bounds.size(), mode, state);
        painter->drawPixmap(bounds, pix);
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        //  we don't support these modes yet, so don't render or cache them
        if (mode == QIcon::Mode::Active || mode == QIcon::Mode::Selected)
            mode = QIcon::Mode::Normal;

        // we could have two svg files via addFile, select the right one
        // if we don't have the requested file, fallback to normal/off (the default)
        QString filePath = mFiles[state & 1];
        if (filePath.isEmpty())
            filePath = mFiles[QIcon::State::Off];
        if (filePath.isEmpty())
            return {};

        QColor color = mColor;
        if (!mColor.isValid())
            color = DkSettingsManager::param().display().iconColor;
        if (mode == QIcon::Mode::Disabled)
            color = Qt::gray; // TODO: pull from theme

        QString key = QStringLiteral("icon.%1.%2.%3.%4.%5.%6")
                          .arg(filePath)
                          .arg(color.name())
                          .arg(size.width())
                          .arg(size.height())
                          .arg(mode)
                          .arg(state);

        QPixmap pix;
        if (QPixmapCache::find(key, &pix))
            return pix;

        pix = QPixmap(size);
        pix.fill(Qt::transparent);

        {
            QPainter painter(&pix);
            QSvgRenderer(filePath).render(&painter, pix.rect());

            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pix.rect(), color);
        }

        QPixmapCache::insert(key, pix);

        // qDebug() << "rendered" << mode << state << size << pix << filePath;
        return pix;
    }

    QIconEngine *clone() const override
    {
        auto *copy = new DkSvgIconEngine;
        copy->mFiles = mFiles;
        copy->mColor = mColor;
        return copy;
    }

    QStringList mFiles;

    QColor mColor;
};

QIcon DkImage::loadIcon(const QString &filePath, const QColor &color)
{
    return QIcon(new DkSvgIconEngine(filePath, color));
}

#ifdef WITH_OPENCV

/**
 * Converts a QImage to a Mat
 * @param img formats supported: ARGB32 | RGB32 | RGB888 | Indexed8
 * @return cv::Mat the corresponding Mat
 **/
cv::Mat DkImage::qImage2Mat(const QImage &img)
{
    cv::Mat mat2;
    QImage cImg; // must be initialized here!	(otherwise the data is lost before clone())

    try {
        // if (img.format() == QImage::Format_RGB32)
        //	qDebug() << "we have an RGB32 in memory...";

        if (img.format() == QImage::Format_ARGB32 || img.format() == QImage::Format_RGB32) {
            mat2 = cv::Mat(img.height(), img.width(), CV_8UC4, (uchar *)img.bits(), img.bytesPerLine());
            // qDebug() << "ARGB32 or RGB32";
        } else if (img.format() == QImage::Format_RGB888) {
            mat2 = cv::Mat(img.height(), img.width(), CV_8UC3, (uchar *)img.bits(), img.bytesPerLine());
            // qDebug() << "RGB888";
        }
        //// converting to indexed8 causes bugs in the qpainter
        //// see: http://qt-project.org/doc/qt-4.8/qimage.html
        // else if (img.format() == QImage::Format_Indexed8) {
        //	mat2 = Mat(img.height(), img.width(), CV_8UC1, (uchar*)img.bits(), img.bytesPerLine());
        //	//qDebug() << "indexed...";
        // }
        else {
            // qDebug() << "image flag: " << img.format();
            cImg = img.convertToFormat(QImage::Format_ARGB32);
            mat2 = cv::Mat(cImg.height(), cImg.width(), CV_8UC4, (uchar *)cImg.bits(), cImg.bytesPerLine());
            // qDebug() << "I need to convert the QImage to ARGB32";
        }

        mat2 = mat2.clone(); // we need to own the pointer
    } catch (...) { // something went seriously wrong (e.g. out of memory)
        // DkNoMacs::dialog(QObject::tr("Sorry, could not convert image."));
        qDebug() << "[DkImage::qImage2Mat] could not convert image - something is seriously wrong down here...";
    }

    return mat2;
}

/**
 * Converts a cv::Mat to a QImage.
 * @param img supported formats CV8UC1 | CV_8UC3 | CV_8UC4
 * @return QImage the corresponding QImage
 **/
QImage DkImage::mat2QImage(cv::Mat img, const QImage &srcImg)
{
    QImage qImg;

    // since Mat header is copied, a new buffer should be allocated (check this!)
    if (img.depth() == CV_32F)
        img.convertTo(img, CV_8U, 255);

    if (img.type() == CV_8UC1) {
        qImg = QImage(img.data,
                      (int)img.cols,
                      (int)img.rows,
                      (int)img.step,
                      QImage::Format_Indexed8); // opencv uses size_t for scaling in x64 applications
        // Mat tmp;
        // cvtColor(img, tmp, CV_GRAY2RGB);	// Qt does not support writing to index8 images
        // img = tmp;
        if (!srcImg.colorTable().isEmpty())
            qImg.setColorTable(srcImg.colorTable());
    }
    if (img.type() == CV_8UC3) {
        // cv::cvtColor(img, img, CV_RGB2BGR);
        qImg = QImage(img.data, (int)img.cols, (int)img.rows, (int)img.step, QImage::Format_RGB888);
    }
    if (img.type() == CV_8UC4) {
        // do not add back an empty alpha channel
        // TODO: if the manipulator adds/removes alpha channel, pass as an argument
        auto format = !srcImg.hasAlphaChannel() ? QImage::Format_RGB32 : QImage::Format_ARGB32;
        qImg = QImage(img.data, (int)img.cols, (int)img.rows, (int)img.step, format);
    }

    qImg = qImg.copy();
    qImg.setColorSpace(srcImg.colorSpace());

    return qImg;
}

void DkImage::linearToGamma(cv::Mat &img)
{
    auto gammaTable = getLinear2GammaTable<uint16_t>();
    mapGammaTable(img, gammaTable);
}

void DkImage::gammaToLinear(cv::Mat &img)
{
    auto gammaTable = getGamma2LinearTable<uint16_t>();
    mapGammaTable(img, gammaTable);
}

void DkImage::mapGammaTable(cv::Mat &img, const QVector<uint16_t> &gammaTable)
{
    if (gammaTable.size() != 1 << 16) {
        qCritical() << "invalid 16-bit gamma table";
        return;
    }

    for (int rIdx = 0; rIdx < img.rows; rIdx++) {
        auto *mPtr = img.ptr<uint16_t>(rIdx);

        for (int cIdx = 0; cIdx < img.cols; cIdx++) {
            for (int channelIdx = 0; channelIdx < img.channels(); channelIdx++, mPtr++) {
                *mPtr = gammaTable[*mPtr];
            }
        }
    }
}

void DkImage::logPolar(const cv::Mat &src,
                       cv::Mat &dst,
                       cv::Point2d center,
                       double scaleLog,
                       double angle,
                       double scale)
{
    cv::Mat mapx, mapy;

    cv::Size ssize, dsize;
    ssize = src.size();
    dsize = dst.size();

    mapx = cv::Mat(dsize.height, dsize.width, CV_32F);
    mapy = cv::Mat(dsize.height, dsize.width, CV_32F);

    double xDist = dst.cols - center.x;
    double yDist = dst.rows - center.y;

    double radius = std::sqrt(xDist * xDist + yDist * yDist);

    scale *= src.cols / std::log(radius / scaleLog + 1.0);

    int x, y;
    cv::Mat bufx, bufy, bufp, bufa;
    double ascale = ssize.height / (2 * CV_PI);
    cv::AutoBuffer<float> _buf(4 * dsize.width);
    float *buf = _buf;

    bufx = cv::Mat(1, dsize.width, CV_32F, buf);
    bufy = cv::Mat(1, dsize.width, CV_32F, buf + dsize.width);
    bufp = cv::Mat(1, dsize.width, CV_32F, buf + dsize.width * 2);
    bufa = cv::Mat(1, dsize.width, CV_32F, buf + dsize.width * 3);

    for (x = 0; x < dsize.width; x++)
        bufx.ptr<float>()[x] = (float)(x - center.x);

    for (y = 0; y < dsize.height; y++) {
        auto *mx = mapx.ptr<float>(y);
        auto *my = mapy.ptr<float>(y);

        for (x = 0; x < dsize.width; x++)
            bufy.ptr<float>()[x] = (float)(y - center.y);

        cv::cartToPolar(bufx, bufy, bufp, bufa);

        for (x = 0; x < dsize.width; x++) {
            bufp.ptr<float>()[x] /= (float)scaleLog;
            bufp.ptr<float>()[x] += 1.0f;
        }

        cv::log(bufp, bufp);

        for (x = 0; x < dsize.width; x++) {
            double rho = bufp.ptr<float>()[x] * scale;
            double phi = bufa.ptr<float>()[x] + angle;

            if (phi < 0)
                phi += 2 * CV_PI;
            else if (phi > 2 * CV_PI)
                phi -= 2 * CV_PI;

            phi *= ascale;

            // qDebug() << "phi: " << bufa.data.fl[x];

            mx[x] = (float)rho;
            my[x] = (float)phi;
        }
    }

    cv::remap(src, dst, mapx, mapy, CV_INTER_AREA, IPL_BORDER_REPLICATE);
}

void DkImage::tinyPlanet(QImage &img, double scaleLog, double angle, QSize s, bool invert /* = false */)
{
    QTransform rotationMatrix;
    rotationMatrix.rotate((invert) ? (double)-90 : (double)90);
    img = img.transformed(rotationMatrix);

    // make square
    img = img.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    cv::Mat mImg = DkImage::qImage2Mat(img);

    qDebug() << "scale log: " << scaleLog << " inverted: " << invert;
    logPolar(mImg, mImg, cv::Point2d(mImg.cols * 0.5, mImg.rows * 0.5), scaleLog, angle);

    img = DkImage::mat2QImage(mImg, img);
}

#endif

bool DkImage::gaussianBlur(QImage &img, float sigma)
{
#ifdef WITH_OPENCV
    DkTimer dt;
    cv::Mat imgCv = DkImage::qImage2Mat(img);

    cv::Mat imgG;
    cv::Mat gx = cv::getGaussianKernel(qRound(4 * sigma + 1), sigma);
    cv::Mat gy = gx.t();
    cv::sepFilter2D(imgCv, imgG, CV_8U, gx, gy);
    img = DkImage::mat2QImage(imgG, img);

    qDebug() << "gaussian blur takes: " << dt;
#else
    Q_UNUSED(img);
    Q_UNUSED(sigma);
#endif

    return true;
}

bool DkImage::unsharpMask(QImage &img, float sigma, float weight)
{
#ifdef WITH_OPENCV
    DkTimer dt;
    // DkImage::gammaToLinear(img);
    cv::Mat imgCv = DkImage::qImage2Mat(img);

    cv::Mat imgG;
    cv::Mat gx = cv::getGaussianKernel(qRound(4 * sigma + 1), sigma);
    cv::Mat gy = gx.t();
    cv::sepFilter2D(imgCv, imgG, CV_8U, gx, gy);
    // cv::GaussianBlur(imgCv, imgG, cv::Size(4*sigma+1, 4*sigma+1), sigma);		// this is awesomely slow
    cv::addWeighted(imgCv, weight, imgG, 1 - weight, 0, imgCv);
    img = DkImage::mat2QImage(imgCv, img);

    qDebug() << "unsharp mask takes: " << dt;
    // DkImage::linearToGamma(img);
#else
    Q_UNUSED(img);
    Q_UNUSED(sigma);
    Q_UNUSED(weight);
#endif

    return true;
}

QImage DkImage::createThumb(const QImage &image, int maxSize)
{
    if (image.isNull()) {
        return image;
    }
    const double maxThumbSize = maxSize == -1 ? max_thumb_size * DkSettingsManager::param().dpiScaleFactor() : maxSize;
    int imgW = image.width();
    int imgH = image.height();

    if (imgW <= maxThumbSize && imgH <= maxThumbSize) {
        return image;
    }

    if (imgW > imgH) {
        imgH = qRound(maxThumbSize / imgW * imgH);
        imgW = maxThumbSize;
    } else if (imgW < imgH) {
        imgW = qRound(maxThumbSize / imgH * imgW);
        imgH = maxThumbSize;
    } else {
        imgW = maxThumbSize;
        imgH = maxThumbSize;
    }

    // fast downscaling
    QImage thumb = image.scaled(QSize(imgW * 2, imgH * 2), Qt::KeepAspectRatio, Qt::FastTransformation);
    thumb = thumb.scaled(QSize(imgW, imgH), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    thumb.convertToColorSpace(QColorSpace{QColorSpace::SRgb});

    // qDebug() << "thumb size in createThumb: " << thumb.size() << " format: " << thumb.format();

    return thumb;
}

// DkImageStorage --------------------------------------------------------------------
DkImageStorage::DkImageStorage(const QImage &img)
{
    mImg = img;

    init();

    connect(&mFutureWatcher,
            &QFutureWatcher<QImage>::finished,
            this,
            &DkImageStorage::imageComputed,
            Qt::UniqueConnection);
    connect(DkActionManager::instance().action(DkActionManager::menu_view_anti_aliasing),
            &QAction::toggled,
            this,
            &DkImageStorage::antiAliasingChanged,
            Qt::UniqueConnection);
}

bool DkImageStorage::alphaChannelUsed()
{
    if (mAlphaState == alpha_unknown) {
        mAlphaState = DkImage::alphaChannelUsed(mImg) ? alpha_used : alpha_unused;
    }

    return mAlphaState == alpha_used;
}

void DkImageStorage::init()
{
    mComputeState = l_not_computed;
    mScaledImg = QImage();
}

void DkImageStorage::setImage(const QImage &img)
{
    mScaledImg = QImage();
    mImg = img;
    mComputeState = l_cancelled;
    mAlphaState = alpha_unknown;
}

void DkImageStorage::antiAliasingChanged(bool antiAliasing)
{
    DkSettingsManager::param().display().antiAliasing = antiAliasing;

    if (!antiAliasing)
        init();

    emit infoSignal((antiAliasing) ? tr("Anti Aliasing Enabled") : tr("Anti Aliasing Disabled"));
    emit imageUpdated();
}

QImage DkImageStorage::imageConst() const
{
    return mImg;
}

QImage DkImageStorage::image(const QSize &size)
{
    if (size.isEmpty() || mImg.isNull() //
        || !DkSettingsManager::param().display().antiAliasing // if AA is disabled QPainter handles all scaling
        || mImg.size().width() <= size.width() // scale factor >= 1 always handled by QPainter
    ) {
        return mImg;
    }

    if (mScaledImg.size() == size) {
        return mScaledImg;
    }

    // start downsampling/antialiasing in a thread
    compute(size);

    // downSampling is async, return original image as a placeholder
    return mImg;
}

QImage imageStorageScaleToSize(const QImage &src, const QSize &size);

void DkImageStorage::compute(const QSize &size)
{
    // don't compute twice
    if (mComputeState == l_computing) {
        return;
    }

    // Reset state
    mScaledImg = QImage();
    mComputeState = l_computing;

    mFutureWatcher.setFuture(QtConcurrent::run(imageStorageScaleToSize, mImg, size));
}

QImage imageStorageScaleToSize(const QImage &src, const QSize &size)
{
    // should not happen
    if (size.width() >= src.width()) {
        qWarning() << "imageStorageScaleToSize was called without a need...";
        return src;
    }

    QImage resizedImg = src;

    if (!DkSettingsManager::param().display().highQualityAntiAliasing) {
        QSize cs = src.size();

        // fast down sampling until the image is twice times full HD
        while (qMin(cs.width(), cs.height()) > 2 * 4000) {
            cs *= 0.5;
        }

        // for extreme panorama images the Qt scaling crashes (if we have a width > 30000) so we simply
        if (cs != src.size()) {
            resizedImg = resizedImg.scaled(cs, Qt::KeepAspectRatio, Qt::FastTransformation);
        }
    }

    QSize s = size;

    if (s.height() == 0)
        s.setHeight(1);
    if (s.width() == 0)
        s.setWidth(1);

#ifdef WITH_OPENCV
    try {
        cv::Mat rImgCv = DkImage::qImage2Mat(resizedImg);
        cv::Mat tmp;
        cv::resize(rImgCv, tmp, cv::Size(s.width(), s.height()), 0, 0, CV_INTER_AREA);
        resizedImg = DkImage::mat2QImage(tmp, resizedImg);
    } catch (...) {
        qWarning() << "imageStorageScaleToSize: OpenCV exception caught while resizing...";
    }
#else
    resizedImg = resizedImg.scaled(s, Qt::KeepAspectRatio, Qt::SmoothTransformation);
#endif

    return resizedImg;
}

void DkImageStorage::imageComputed()
{
    if (mComputeState == l_cancelled) {
        mComputeState = l_not_computed;
        return;
    }

    mScaledImg = mFutureWatcher.result();

    mComputeState = (mScaledImg.isNull()) ? l_empty : l_computed;

    if (mComputeState == l_computed)
        emit imageUpdated();
    else
        qWarning() << "could not compute interpolated image...";
}
}
