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
#include "DkImageProc.h"
#include "DkMath.h"
#include "DkNativeImage.h"
#include "DkSettings.h"
#include "DkThumbs.h"
#include "DkTimer.h"

#include <QColorSpace>
#include <QIconEngine>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QSvgRenderer>
#include <QtConcurrentMap>
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

static QImage convertToLinear(const QImage &src)
{
    QImage img = src;

    QImage::Format format;
    if (img.pixelFormat().colorModel() == QPixelFormat::Grayscale) {
        // If we can't give a format to convertToColorSpace (Qt < 6.8),
        // it keeps pixel format unchanged, leading to invalid colorspace conversion
        format = QImage::Format_Grayscale16;
    } else {
        format = img.depth() <= 32 ? QImage::Format_RGBA64 : QImage::Format_RGBA32FPx4;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    img.convertTo(format);
    img.convertToColorSpace(QColorSpace::SRgbLinear);
#else
    img.convertToColorSpace(QColorSpace::SRgbLinear, format);
#endif
    return img;
}

static QImage convertToColorSpace(const QImage &src, QImage::Format dstFormat, const QColorSpace &dstColorSpace)
{
    QImage img = src;
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    img.convertToColorSpace(dstColorSpace);
    img.convertTo(dstFormat);
#else
    img.convertToColorSpace(dstColorSpace, dstFormat);
#endif
    return img;
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

    // FIXME: enlarging large images can easily overrun system memory

    if (correctGamma && !src.colorSpace().isValid()) {
        correctGamma = false;
        qWarning() << "[resizeImage] gamma correction disabled, source has no valid colorspace";
    }

    try {
        QImage inImg = correctGamma ? convertToLinear(src) : src;

#if WITH_OPENCV
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

        auto input = DkConstNativeImage::fromImage(inImg);
        auto output = input.allocateLike(nSize);
        cv::resize(input.mat(), output.mat(), cv::Size(nSize.width(), nSize.height()), 0, 0, ipl);
        QImage outImg = output.img();
#else
        auto ipl = interpolation == ipl_nearest ? Qt::FastTransformation : Qt::SmoothTransformation;
        QImage outImg = inImg.scaled(nSize, Qt::IgnoreAspectRatio, ipl);
#endif
        if (correctGamma) {
            outImg = convertToColorSpace(outImg, src.format(), src.colorSpace());
        } else {
            outImg.convertTo(src.format());
        }
        return outImg;
    } catch (...) {
        qWarning() << "[resizeImage] resize failed";
        return src;
    }
}

// return true if all values in a channel equal value
template<typename T>
static bool isChannelEqual(const QImage &img, int channel, int numChannels, T value)
{
    Q_ASSERT(channel >= 0 && channel < numChannels);
    Q_ASSERT(static_cast<size_t>(img.depth()) == sizeof(T) * 8 * numChannels);
    auto *channels = reinterpret_cast<const T *>(img.constBits());
    size_t stride = img.bytesPerLine() / sizeof(T);
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
    bool hasAlpha = img.hasAlphaChannel();
    if (!hasAlpha) {
        return false;
    }

    switch (img.format()) {
    case QImage::Format_Indexed8:
        return hasAlpha; // QImage::hasAlphaChannel already scanned colortable
    case QImage::Format_Alpha8:
        return !isChannelEqual<uint8_t>(img, 0, 1, 0xFF);
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied: {
        return !isChannelEqual<uint8_t>(img, 3, 4, 0xFF);
    }
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied: {
        return !isChannelEqual<uint16_t>(img, 3, 4, 0xFFFF);
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

#if WITH_OPENCV

// rgb->grayscale conversion in linear light
class DkGrayScaleKernel : public DkKernelBase<DkGrayScaleKernel>
{
    friend class DkKernelBase<DkGrayScaleKernel>;

public:
    Q_DISABLE_COPY(DkGrayScaleKernel)
    DkGrayScaleKernel() = delete;
    ~DkGrayScaleKernel() override = default;

    DkGrayScaleKernel(const QImage &img)
        : mSrc{DkConstNativeImage::fromImage(img)}
    {
    }

protected:
    const DkConstNativeImage mSrc;
    DkNativeImage mDst{};
    QColorTransform mSrcToLinear{};

    template<typename SrcFmt, typename DstFmt>
    static bool kernel(DkGrayScaleKernel &self, const DkWorkRange &range)
    {
        using SrcType = typename SrcFmt::ChannelType;
        using DstType = typename DstFmt::ChannelType;

        const auto &src = self.mSrc.mat();
        auto &dst = self.mDst.mat();
        auto &srcToLinear = self.mSrcToLinear;

        Q_ASSERT(SrcFmt::Channels > 1); // input rgb or argb
        Q_ASSERT(DstFmt::Channels == 1 || DstFmt::Channels == 4); // output gray or argb

        Q_ASSERT(SrcFmt::isCompatibleWith(src));
        Q_ASSERT(DstFmt::isCompatibleWith(dst));
        Q_ASSERT(src.size == dst.size);
        Q_ASSERT(range.isWithin(0, src.rows));

        const int srcChannelsPerRow = src.cols * SrcFmt::Channels;

        // Qt color conversion is very slow per-pixel so load up a row and convert it all at once
        // TODO: optimize this so buffer fits in L1 or L2 cache (no read/write to main memory!)
        QImage scratchBuf(src.cols, 1, QImage::Format_RGBA32FPx4);
        auto *scratchPtr = reinterpret_cast<float *>(scratchBuf.bits());

        for (int row = range.begin; row < range.end; ++row) {
            const auto *srcPtr = src.ptr<SrcType>(row);

            // get normalized input values [0,1] for this entire row
            float *normPtr = scratchPtr;
            for (int col = 0; col < srcChannelsPerRow; col += SrcFmt::Channels, normPtr += 4) {
                const SrcType *pixel = srcPtr + col;
                auto [r, g, b, a] = SrcFmt::loadFloat(pixel);
                normPtr[0] = r, normPtr[1] = g, normPtr[2] = b, normPtr[3] = a;
            }

            // transform source colorspace to srgb linear; might reallocate image (not in new versions)
            // Qt prior to 6.4.2 steps on the alpha channel of float images, but that is barely in use anymore
            scratchBuf.applyColorTransform(srcToLinear);
            scratchPtr = reinterpret_cast<float *>(scratchBuf.bits());

            const float *linPtr = scratchPtr;
            auto *dstPtr = dst.ptr<DstType>(row);
            for (int col = 0; col < src.cols; col++, linPtr += 4, dstPtr += DstFmt::Channels) {
                float r = linPtr[0], g = linPtr[1], b = linPtr[2], a = linPtr[3];

                // D65 srgb linear to XYZ. We only care about the luminance (Y) for grayscale conversion
                // there is no normalization step as Y=1.0 is reference white
                float y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;

                // Let's convert to srgb-ish gamma 2.2, close enough for this and OK without a color profile
                // for float formats we are expected to output linear values (and colorspace)
                float ys;
                if constexpr (std::is_same_v<DstType, float>) {
                    ys = y;
                } else {
                    y = qBound(0.0, y, 1.0f);
                    ys = std::pow(y, 1.0f / 2.2f);
                }

                DstFmt::store(dstPtr, ys, ys, ys, a);
            }
        }
        return true;
    }

    static constexpr FmtMap kMapOpaque = {{{ImgFmt::BGR888, ImgFmt::Gray16}, // output to wide Grayscale
                                           {ImgFmt::RGB888, ImgFmt::Gray16},
                                           {ImgFmt::ARGB32, ImgFmt::Gray16},
                                           {ImgFmt::RGBA8888, ImgFmt::Gray16},
                                           {ImgFmt::RGBA64, ImgFmt::Gray16},
                                           {ImgFmt::RGBAFP32, ImgFmt::Gray16}}};

    static constexpr FmtMap kMapAlpha = {{{ImgFmt::ARGB32, ImgFmt::RGBA64}, // output to wider RGBA type
                                          {ImgFmt::RGBA8888, ImgFmt::RGBA64},
                                          {ImgFmt::RGBA64, ImgFmt::RGBAFP32},
                                          {ImgFmt::RGBAFP32, ImgFmt::RGBAFP32}}};

    static constexpr DispatchTable kTableOpaque = makeTable(kMapOpaque);
    static constexpr DispatchTable kTableAlpha = makeTable(kMapAlpha);

public:
    bool run() override
    {
        QColorSpace srcColorSpace = mSrc.img().colorSpace();
        if (!srcColorSpace.isValid()) {
            srcColorSpace = QColorSpace{QColorSpace::SRgb}; // FIXME: DkImage::defaultColorSpace(img)
        }

        mSrcToLinear = srcColorSpace.transformationToColorSpace(QColorSpace::SRgbLinear);

        bool usesAlpha = DkImage::alphaChannelUsed(mSrc.img());

        auto map = usesAlpha ? kMapAlpha : kMapOpaque;
        auto table = usesAlpha ? kTableAlpha : kTableOpaque;

        auto srcFormat = qtImageFormatToNative(mSrc.img().format());
        auto dstFormat = findFormat(map, srcFormat);
        if (dstFormat == ImgFmt::Invalid) {
            return false;
        }

        auto qtFormat = nativeFormatToQtFormat(dstFormat);
        QImage dst{mSrc.img().size(), qtFormat};

        QColorSpace dstColorSpace;
        if (usesAlpha) {
            if (qtFormat == QImage::Format_RGBA32FPx4) {
                dstColorSpace = QColorSpace{QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::Linear};
                dstColorSpace.setDescription("sRGB Linear");
            } else {
                dstColorSpace = QColorSpace{QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::Gamma, 2.2f};
                dstColorSpace.setDescription("sRGB Gamma 2.2");
            }
        } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
            // Qt < 6.8 does not support gray colorspaces, best we can do is not assign one, which gives us ~sRGB
            dstColorSpace = {};
#else
            dstColorSpace = QColorSpace{QPointF{0.3127, 0.3290}, QColorSpace::TransferFunction::Gamma, 2.2f};
            dstColorSpace.setDescription("D65 Gamma 2.2 Grayscale");
#endif
        }
        dst.setColorSpace(dstColorSpace);

        mDst = DkNativeImage::fromImage(dst);

        return dispatch(table, mSrc.img().format(), *this, {0, mSrc.img().height()});
    }

    QImage result() const override
    {
        return mDst.img();
    }
};

#endif // WITH_OPENCV

QImage DkImage::grayscaleImage(const QImage &src)
{
#if WITH_OPENCV
    DkGrayScaleKernel kernel{src};
    return kernel.run() ? kernel.result() : QImage{};
#else
    if (src.pixelFormat().colorModel() == QPixelFormat::ColorModel::Grayscale) {
        return src;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    // Qt <6.8 does not support gray colorspaces.
    return src.convertedTo(QImage::Format_Grayscale16);
#else
    // This works similarly but we can't preserve alpha channel
    QColorSpace dstColorSpace{QPointF{0.3127, 0.3290}, QColorSpace::TransferFunction::Gamma, 2.2};
    dstColorSpace.setDescription("D65 Gamma 2.2 Grayscale");
    return src.convertedToColorSpace(dstColorSpace);
#endif
#endif
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

// get the best format for adding alpha channel to image
static QImage::Format alphaFormat(const QImage &img)
{
    QImage::Format format = img.format();
    QPixelFormat pf = img.pixelFormat();
    if (pf.alphaUsage() == QPixelFormat::UsesAlpha)
        return format;

    switch (format) {
    case QImage::Format_RGBX64:
        format = QImage::Format_RGBA64;
        break;
    case QImage::Format_RGBX16FPx4:
        format = QImage::Format_RGBA16FPx4;
        break;
    case QImage::Format_RGBX32FPx4:
        format = QImage::Format_RGBA32FPx4;
        break;
    default:
        format = QImage::Format_ARGB32;
    }

    return format;
}

QImage DkImage::cropToImage(const QImage &src, const DkRotatingRect &rect, const QColor &fillColor)
{
    QTransform tForm;
    QPointF cImgSize;
    rect.getTransform(tForm, cImgSize);

    // illegal?
    if (cImgSize.x() < 0.5f || cImgSize.y() < 0.5f)
        return src;

    // If the rotation angle is a right angle we disable interpolation.
    // We can also use QImage::copy() to keep the format identical

    // FIXME: getAngle() is imprecise for tiny crops (1x1, 2x2 etc), so we need large epsilon
    // This works, but requires angle has limited precision (in degrees, 2 decimal places)
    const double epsilon = 1e-5;

    double radians = rect.getAngle();
    double rightAngle = radians / (CV_PI * 0.5);
    double error = std::abs(std::round(rightAngle) - rightAngle);
    bool rotated = error > epsilon;

    if (!rotated) {
        QRect cropRect = tForm.inverted().mapRect(QRectF{0.0, 0.0, cImgSize.x(), cImgSize.y()}).toRect();
        if (src.rect().contains(cropRect)) {
            return src.copy(cropRect);
        }
    }

    // try to keep the pixel format; add alpha channel if fill color is transparent
    QImage::Format outFormat = src.format();
    if (fillColor.alpha() < 255) {
        outFormat = alphaFormat(src);
    }

    // QPainter segfaults if the target is indexed
    if (outFormat == QImage::Format_Indexed8) {
        outFormat = QImage::Format_ARGB32;
    }

    QImage img = QImage(qRound(cImgSize.x()), qRound(cImgSize.y()), outFormat);
    img.setColorSpace(src.colorSpace());

    if (outFormat == QImage::Format_Mono || outFormat == QImage::Format_MonoLSB) {
        img.fill(fillColor.lightness() < 127 ? 0 : 1);
    } else {
        img.fill(fillColor);
    }

    QPainter painter(&img);
    painter.setWorldTransform(tForm);

    if (rotated) {
        painter.setRenderHints(QPainter::SmoothPixmapTransform);
    }

    painter.setCompositionMode(QPainter::CompositionMode_Source); // do not blend with fill color
    painter.drawImage(QPoint{}, src);

    return img;
}

#ifdef WITH_OPENCV

class DkHsvKernel : public DkKernelBase<DkHsvKernel>
{
    friend class DkKernelBase<DkHsvKernel>;

public:
    DkHsvKernel() = delete;
    Q_DISABLE_COPY(DkHsvKernel)
    DkHsvKernel(QImage &img, float hue, float saturation, float brightness)
        : mImg(DkNativeImage::fromImage(img, DkNativeImage::map_anyrgb))
        , mHue(hue)
        , mSaturation(saturation)
        , mBrightness(brightness)
    {
    }
    ~DkHsvKernel() override = default;

protected:
    DkNativeImage mImg;
    const float mHue, mSaturation, mBrightness;

    // float for hsv->rgb avoids most division and has the precision we need for wide formats
    // this performs quite well, OpenCVs 8-bit hsv is only about 20% faster and only for
    // small images.
    static std::tuple<float, float, float> rgbToHsv(float r, float g, float b)
    {
        const float cmax = std::max(r, std::max(g, b));
        const float cmin = std::min(r, std::min(g, b));
        const float delta = cmax - cmin;

        float h, s, v;
        v = cmax;

        if (delta > 0.0f) {
            const float invDelta = 1.0f / delta;
            if (r >= g && r >= b) {
                float t = (g - b) * invDelta;
                if (t < 0.0f) {
                    t += 6.0f;
                }
                h = t;
            } else if (g >= b) {
                h = ((b - r) * invDelta + 2.0f);
            } else {
                h = ((r - g) * invDelta + 4.0f);
            }
            h *= 60.0f;
            s = delta * (1.0f / cmax); // cmax > 0 here
        } else {
            h = 0.0f; // grayscale
            s = 0.0f;
        }
        return {h, s, v};
    }

    static std::tuple<float, float, float> hsvToRgb(float h, float s, float v)
    {
        Q_ASSERT(h >= 0.0f && h <= 360.0f);
        Q_ASSERT(s >= 0.0f && s <= 1.0f);
        Q_ASSERT(v >= 0.0f && v <= 1.0f);

        const float invSectorSize = 1.0f / 60.0f;
        float hh = h * invSectorSize;
        int sector = (int)hh;
        float frac = hh - sector;

        float p = v * (1.0f - s);
        float q = v * (1.0f - s * frac);
        float t = v * (1.0f - s * (1.0f - frac));

        // clang-format off
        float r, g, b;
        switch (sector) {
        case 0:  r = v, g = t, b = p; break;
        case 1:  r = q, g = v, b = p; break;
        case 2:  r = p, g = v, b = t; break;
        case 3:  r = p, g = q, b = v; break;
        case 4:  r = t, g = p, b = v; break;
        default: r = v, g = p, b = q; break;
        }
        // clang-format on
        return {r, g, b};
    }

    template<typename Format>
    static bool kernel(DkHsvKernel &self, const DkWorkRange &range)
    {
        using ChannelType = typename Format::ChannelType;

        auto &mat = self.mImg.mat();
        const float hueAdd = self.mHue; // [-180,180]
        const float satScale = self.mSaturation / 100.0 + 1.0; // [-100,100] => [0,2]
        const float valAdd = self.mBrightness / 100.0; // [-100,100] => [-1,1]

        forEachPixel<Format>(mat, range, [&](ChannelType *pixel) {
            auto [r, g, b, /* unused */ _a] = Format::loadFloat(pixel);

            auto [h, s, v] = rgbToHsv(r, g, b); // h:[0,360] s:[0,1] v:[0,1]

            h += hueAdd, s *= satScale, v += valAdd;

            if (h < 0.0f) {
                h += 360.0f;
            } else if (h > 360.0f) {
                h -= 360.0f;
            }
            s = qBound(0.0f, s, 1.0f);
            v = qBound(0.0f, v, 1.0f);

            auto [rr, gg, bb] = hsvToRgb(h, s, v);

            Format::store(pixel, rr, gg, bb);
        });

        return true;
    }

    static constexpr int kCaps = cap_gray | cap_bgr | cap_rgb;
    static constexpr FmtList kFormats = listForKernelCaps(kCaps);
    static constexpr DispatchTable kTable = makeTable(kFormats);

public:
    bool run() override
    {
        return dispatch(kTable, mImg.img().format(), *this, {0, mImg.img().height()});
    }

    QImage result() const override
    {
        return mImg.img();
    }
};
#endif // WITH_OPENCV

bool DkImage::hueSaturation(QImage &img, float hue, float sat, float brightness)
{
    if (hue == 0 && sat == 0 && brightness == 0) {
        return false;
    }

#ifdef WITH_OPENCV
    DkHsvKernel kernel{img, hue, sat, brightness};
    if (kernel.run()) {
        img = kernel.result();
        return true;
    }
#else
    Q_UNUSED(img)
#endif
    return false;
}

#if WITH_OPENCV

static cv::Mat exposureLut(double exposure)
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
    return lut;
}

static cv::Mat gammaLut(double gamma)
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

    return lut;
}

// kernel to apply a 16-bit LUT equally to all channels
class DkLutKernel : DkKernelBase<DkLutKernel>
{
    friend class DkKernelBase<DkLutKernel>; // needs visibility to kernel()
public:
    Q_DISABLE_COPY(DkLutKernel)
    DkLutKernel() = delete;
    DkLutKernel(QImage &img, cv::Mat &lut, float offset)
        : mImg(DkNativeImage::fromImage(img, DkNativeImage::map_anyrgb))
        , mLut{lut}
        , mOffset(offset)
    {
    }
    ~DkLutKernel() override = default;

protected:
    DkNativeImage mImg; // input/output
    const cv::Mat &mLut; // 16-bit lookup table
    float mOffset; // offset added before lookup [-1,1]

    template<typename Format>
    static bool kernel(DkLutKernel &self, const DkWorkRange &range)
    {
        using ChannelType = typename Format::ChannelType;

        constexpr int U16_Max = std::numeric_limits<uint16_t>::max();

        auto &mat = self.mImg.mat();
        const auto &lut = self.mLut;
        const auto offset = self.mOffset;

        Q_ASSERT(lut.rows == 1 && lut.cols >= U16_Max && lut.depth() == CV_16UC1);

        const auto *lutPtr = lut.ptr<uint16_t>();

        // scale factor from src value to 16-bit
        constexpr int64_t srcScale = U16_Max / Format::Scale;

        forEachChannel<Format>(mat, range, [&](ChannelType &channel, int /* unused */) {
            int value = channel * srcScale; // int16 too small for U16_Max*2 (offset [-1,1])
            value = value + (offset * U16_Max);
            value = qBound(0, value, U16_Max); // TODO: clipping solution for HDR (tonemap to [0,1] before LUT?)
            value = lutPtr[value];
            if constexpr (std::is_same_v<ChannelType, float>) {
                channel = value * (1.0f / srcScale); // fp math required (output [0.0,1.0])
            } else {
                channel = value / srcScale;
            }
        });

        return true;
    }

    static constexpr FmtList formats = listForKernelCaps(cap_gray | cap_rgb_invariant);
    static constexpr DispatchTable table = makeTable(formats);

public:
    bool run() override
    {
        return dispatch(table, mImg.img().format(), *this, {0, mImg.img().height()});
    }

    QImage result() const override
    {
        return mImg.img();
    }
};
#endif // WITH_OPENCV

bool DkImage::exposure(QImage &img, double exposure, double offset, double gamma)
{
    if (exposure == 0.0 && offset == 0.0 && gamma == 1.0) {
        return false;
    }

#ifdef WITH_OPENCV
    cv::Mat expTable = exposureLut(exposure);
    cv::Mat gammaTable = gammaLut(gamma);
    const auto *expPtr = expTable.ptr<unsigned short>();
    const auto *gammaPtr = gammaTable.ptr<unsigned short>();

    cv::Mat combined = expTable;
    auto *ptr = combined.ptr<unsigned short>();
    for (int i = 0; i < combined.cols; ++i) {
        ptr[i] = gammaPtr[expPtr[i]];
    }

    DkLutKernel lutKernel{img, combined, (float)offset};
    if (lutKernel.run()) {
        img = lutKernel.result();
        return true;
    }
#else
    Q_UNUSED(img)
#endif // WITH_OPENCV

    return false;
}

// blend two colors with the standard source-over operator
static QColor compositeOver(const QColor &dst, const QColor &src)
{
    float sa = src.alphaF();
    float da = dst.alphaF();
    float outA = sa + da * (1.0f - sa);
    if (outA == 0.0f) {
        return Qt::transparent;
    }

    float outR = (src.redF() * sa + dst.redF() * da * (1.0f - sa)) / outA;
    float outG = (src.greenF() * sa + dst.greenF() * da * (1.0f - sa)) / outA;
    float outB = (src.blueF() * sa + dst.blueF() * da * (1.0f - sa)) / outA;
    return QColor::fromRgbF(outR, outG, outB, outA);
}

QImage DkImage::bgColor(const QImage &src, const QColor &col)
{
    if (!DkImage::alphaChannelUsed(src)) {
        qWarning() << "[bgColor] no alpha channel or alpha channel is fully opaque";
        return src;
    }

    QImage::Format opaqueFormat = QImage::Format_Invalid;

    switch (src.format()) {
    case QImage::Format_Indexed8: {
        QList<QRgb> colorTable = src.colorTable();
        for (QRgb &rgba : colorTable) {
            if (qAlpha(rgba) != 255) {
                rgba = compositeOver(col, QColor::fromRgba(rgba)).rgba();
            }
        }
        QImage dst = src;
        dst.setColorTable(colorTable);
        return dst;
    }
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
        opaqueFormat = QImage::Format_RGBX64;
        break;
    case QImage::Format_RGBA16FPx4:
    case QImage::Format_RGBA16FPx4_Premultiplied:
        opaqueFormat = QImage::Format_RGBX16FPx4;
        break;
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBA32FPx4_Premultiplied:
        opaqueFormat = QImage::Format_RGBX32FPx4;
        break;
    default:
        opaqueFormat = QImage::Format_RGB32;
    }

    QImage dst(src.size(), opaqueFormat);
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

QImage::Format DkImage::renderFormat(QImage::Format imageFormat)
{
    QImage::Format format = imageFormat;
    switch (imageFormat) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_RGBX32FPx4:
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBX64:
    case QImage::Format_RGBA64:
    case QImage::Format_Grayscale8:
    case QImage::Format_Grayscale16:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    case QImage::Format_CMYK8888:
#endif
        // native formats for colorspace conversion - see: QImage::colorTransformed
        // premultiplied formats excluded since it saves some processing when color converting
        break;
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        format = QImage::Format_Grayscale8;
        break;
    case QImage::Format_Indexed8:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
    case QImage::Format_RGB16:
    case QImage::Format_RGB444:
    case QImage::Format_RGB555:
    case QImage::Format_RGB666:
    case QImage::Format_RGB888:
    case QImage::Format_BGR888:
    case QImage::Format_RGBX8888:
    case QImage::Format_Alpha8:
    case QImage::Format_ARGB32_Premultiplied:
        format = QImage::Format_ARGB32;
        break;
    case QImage::Format_BGR30:
    case QImage::Format_RGB30:
        format = QImage::Format_RGBX64;
        break;
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_A2RGB30_Premultiplied:
    case QImage::Format_RGBA64_Premultiplied:
        format = QImage::Format_RGBA64;
        break;
    case QImage::Format_RGBX16FPx4:
    case QImage::Format_RGBA16FPx4:
    case QImage::Format_RGBA16FPx4_Premultiplied:
    case QImage::Format_RGBA32FPx4_Premultiplied:
        format = QImage::Format_RGBA32FPx4;
        break;
    case QImage::Format_Invalid:
    case QImage::NImageFormats:
        Q_UNREACHABLE();
        break;
    }
    return format;
}

QImage DkImage::convertToColorSpaceInPlace(const QWidget *target, QImage &img)
{
    return convertToColorSpaceInPlace(targetColorSpace(target), img);
}

QImage DkImage::convertToColorSpaceInPlace(const QColorSpace &target, QImage &img)
{
    if (target == QColorSpace{}) {
        return img; // colorspace conversion is disabled
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    if (!target.isValid()) {
#else
    if (!target.isValidTarget()) {
#endif
        qWarning() << "[convertToColorSpace] invalid target" << target;
        return img;
    }

    QColorSpace source = img.colorSpace();
    if (!source.isValid()) {
        qWarning() << "[convertToColorSpace] invalid source" << source;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // The output pixel format must have the same color model,
    // otherwise grayscale and cmyk will not always be correct
    if (target.colorModel() == QColorSpace::ColorModel::Rgb //
        && img.pixelFormat().colorModel() != QPixelFormat::ColorModel::RGB) {
        return img.convertedToColorSpace(target, targetFormat());
    }
#endif

    img.convertToColorSpace(target);

    return img;
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

cv::Mat DkImage::qImage2Mat(const QImage &img)
{
    return DkConstNativeImage::fromImage(img, DkNativeImage::map_bgr).mat().clone();
}

QImage DkImage::mat2QImage(cv::Mat mat, const QImage &srcImg)
{
    return DkNativeImage::fromMat(mat, srcImg, DkNativeImage::map_bgr).img().copy();
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

QImage DkImage::tinyPlanet(const QImage &img, double scaleLog, double angle, const QSize &size, bool invert)
{
    QTransform rotationMatrix;
    rotationMatrix.rotate((invert) ? (double)-90 : (double)90);
    QImage tmp = img.transformed(rotationMatrix);

    tmp = tmp.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    auto view = DkNativeImage::fromImage(tmp);
    logPolar(view.mat(), view.mat(), cv::Point2d(view.mat().cols * 0.5, view.mat().rows * 0.5), scaleLog, angle);
    return view.img();
}

#endif

bool DkImage::gaussianBlur(QImage &img, float sigma)
{
    if (sigma <= 0.0f) {
        return false; // kernel would be 1x1 => no-op
    }

#ifdef WITH_OPENCV
    auto native = DkNativeImage::fromImage(img);
    cv::GaussianBlur(native.mat(), native.mat(), cv::Size{}, sigma);
    img = native.img();
    return true;
#else
    Q_UNUSED(img);
    Q_UNUSED(sigma);
#endif
    return false;
}

bool DkImage::unsharpMask(QImage &img, float sigma, float weight)
{
    if (sigma <= 0.0f) {
        return false; // kernel would be 1x1 => no-op
    }

#ifdef WITH_OPENCV
    auto native = DkNativeImage::fromImage(img);
    cv::Mat blurred;
    cv::GaussianBlur(native.mat(), blurred, cv::Size{}, sigma);
    cv::addWeighted(native.mat(), weight, blurred, 1.0f - weight, 0.0, native.mat());
    img = native.img();
    return true;
#else
    Q_UNUSED(img);
    Q_UNUSED(sigma);
    Q_UNUSED(weight);
#endif
    return false;
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
DkImageStorage::DkImageStorage()
{
    connect(&mWorker, &QFutureWatcher<QImage>::finished, this, &DkImageStorage::workerFinished);

    connect(DkActionManager::instance().action(DkActionManager::menu_view_anti_aliasing),
            &QAction::toggled,
            this,
            &DkImageStorage::antiAliasingChanged);
}

DkImageStorage::~DkImageStorage()
{
    // If we destruct while a worker is active, and then immediately construct a new one,
    // workers may pile up in the thread pool.
    if (mWorkerPending) {
        qWarning() << "[ImageStorage] destructing with active worker";
    }
}

bool DkImageStorage::alphaChannelUsed()
{
    if (mAlphaState == alpha_unknown) {
        mAlphaState = DkImage::alphaChannelUsed(mOriginal) ? alpha_used : alpha_unused;
    }

    return mAlphaState == alpha_used;
}

void DkImageStorage::setImage(const QImage &img)
{
    mOriginal = img;
    mScaled = {};
    mAlphaState = alpha_unknown;

    cancelWorker();
}

void DkImageStorage::antiAliasingChanged(bool antiAliasing)
{
    DkSettingsManager::param().display().antiAliasing = antiAliasing;
    if (!antiAliasing) {
        mScaled = {};
        cancelWorker();
    }

    emit infoSignal((antiAliasing) ? tr("Anti Aliasing Enabled") : tr("Anti Aliasing Disabled"));
    emit imageUpdated();
}

QImage DkImageStorage::downsampled(const QSize &size, const QWidget *target, int options) &
{
    if (size.isEmpty() //
        || mOriginal.isNull() //
        || mOriginal.size().width() <= size.width() // scale factor >= 1 always handled by QPainter
    ) {
        return mOriginal;
    }

    bool antialias = DkSettingsManager::param().display().antiAliasing;

    ScaleFilter filter = antialias ? ScaleFilter::area : ScaleFilter::nearest;
    QColorSpace colorSpace = DkImage::targetColorSpace(target);
    QImage::Format format = DkImage::targetFormat();

    if (mScaled.image.size() == size && mScaled.filter == filter && mScaled.colorSpace == colorSpace
        && mScaled.image.format() == format) {
        return mScaled.image;
    }

    if (options & process_sync) {
        cancelWorker();
        if (options & process_fallback) {
            filter = ScaleFilter::nearest;
        }
        mScaled = scaleImage(mOriginal, size, filter, colorSpace, format);
        return mScaled.image;
    }

    if (mWorkerPending) {
        return mOriginal;
    }

    mScaled = {};
    mDiscardResult = false;
    mWorkerPending = true;
    mWorker.setFuture(QtConcurrent::run(scaleImage, mOriginal, size, filter, colorSpace, format));

    if (options & process_fallback) {
        return scaleImage(mOriginal, size, ScaleFilter::nearest, colorSpace, format).image;
    }

    return mOriginal;
}

void DkImageStorage::cancelWorker()
{
    // QFuture from QtConcurrent::run() cannot be cancelled so set this flag
    if (mWorkerPending) {
        mDiscardResult = true;
    }
}

void DkImageStorage::workerFinished()
{
    Q_ASSERT(mWorkerPending);
    mWorkerPending = false;

    if (mDiscardResult) {
        return;
    }

    mScaled = mWorker.result();
    if (!mScaled.image.isNull()) {
        emit imageUpdated();
    } else {
        qWarning() << "[ImageStorage] interpolation failed";
    }
}

DkImageStorage::ScaledImage DkImageStorage::scaleImage(const QImage &src,
                                                       const QSize &size,
                                                       ScaleFilter filter,
                                                       const QColorSpace &colorSpace,
                                                       QImage::Format format)
{
    Q_ASSERT(size.width() < src.width()); // we only downsample

    auto mode = filter == ScaleFilter::area ? Qt::SmoothTransformation : Qt::FastTransformation;
    QImage scaled;
#ifdef WITH_OPENCV
    if (mode == Qt::SmoothTransformation) {
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

        // FIXME: changing size breaks caching
        if (s.height() == 0)
            s.setHeight(1);
        if (s.width() == 0)
            s.setWidth(1);
        try {
            const auto input = DkConstNativeImage::fromImage(resizedImg);
            auto output = input.allocateLike(s);
            cv::resize(input.mat(), output.mat(), cv::Size(s.width(), s.height()), 0, 0, CV_INTER_AREA);
            scaled = output.img();
        } catch (...) {
            qWarning() << "[ImageStorage]: OpenCV exception while resizing";
        }
    } else {
        scaled = src.scaled(size, Qt::IgnoreAspectRatio, mode);
    }
#else
    scaled = src.scaled(size, Qt::IgnoreAspectRatio, mode);
#endif
    Q_ASSERT(scaled.size() == size);
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    if (colorSpace.isValid()) {
#else
    if (colorSpace.isValidTarget()) {
#endif
        scaled = DkImage::convertToColorSpaceInPlace(colorSpace, scaled);
        // Q_ASSERT(scaled.colorSpace() == colorSpace); // FIXME: sometimes fails due to qImage2Mat
    }
    scaled.convertTo(format);
    return {scaled, filter, colorSpace};
}
}
