/*******************************************************************************************************
 DkBasicLoader.cpp
 Created on:	21.02.2014

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include "DkBasicLoader.h"

#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "DkMath.h"
#include "DkMetaData.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h" // just needed for qInfo() #ifdef
#include <utility>

#pragma warning(push, 0)
#include <QBuffer>
#include <QColorSpace>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QImageWriter>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QObject>
#include <QPixmap>
#include <QRegularExpression>
#include <QtConcurrentRun>

#include <assert.h>
#include <qmath.h>

// opencv
#ifdef WITH_OPENCV

#ifdef Q_CC_MSVC
#pragma warning(disable : 4996)
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#ifdef WITH_LIBRAW
#include <libraw/libraw.h>
#endif

#ifdef WITH_LIBTIFF
#ifdef Q_CC_MSVC
#include <tif_config.h>
#endif

//  here we clash (typedef redefinition with different types ('long' vs 'int64_t' (aka 'long long')))
//  so we simply define our own int64 before including tiffio
#define uint64 uint64_hack_
#define int64 int64_hack_

#include <tiffio.h>
#include <tiffio.hxx> // this is needed if you want to load tiffs from the buffer

#undef uint64
#undef int64

#endif // WITH_LIBTIFF

#endif // #ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#include <olectl.h>
#pragma comment(lib, "oleaut32.lib")

#endif // #ifdef Q_OS_WIN

#pragma warning(pop)

namespace nmc
{
// DkEditImage --------------------------------------------------------------------

DkEditImage::DkEditImage()
    : mNewImg(false)
    , mNewMetaData(false)
{
}
DkEditImage::DkEditImage(const QImage &img, const QSharedPointer<DkMetaDataT> &metaData, const QString &editName)
    : mImg(img)
    , mMetaData(metaData)
    , mEditName(editName)
    , mNewImg(true)
    , mNewMetaData(false)
{
    // history edit item with modified image
}

DkEditImage::DkEditImage(const QSharedPointer<DkMetaDataT> &metaData, const QImage &img, const QString &editName)
    : mImg(img)
    , mMetaData(metaData)
    , mEditName(editName)
    , mNewImg(false)
    , mNewMetaData(true)
{
    // history edit item with modified metadata
}

bool DkEditImage::hasImage() const
{
    // Every edit item has an image, but it may be the old/original one if only metadata has been edited
    return !mImg.isNull();
}

bool DkEditImage::hasMetaData() const
{
    if (mMetaData) {
        return !mMetaData->isNull();
    }
    return false;
}

bool DkEditImage::hasNewImage() const
{
    return hasImage() && mNewImg;
}

bool DkEditImage::hasNewMetaData() const
{
    return hasMetaData() && mNewMetaData;
}

void DkEditImage::setImage(const QImage &img)
{
    mImg = img;
}

QImage DkEditImage::image() const
{
    return mImg;
}

QSharedPointer<DkMetaDataT> DkEditImage::metaData() const
{
    return mMetaData;
}

QString DkEditImage::editName() const
{
    return mEditName;
}

int DkEditImage::size() const
{
    return qRound(DkImage::getBufferSizeFloat(mImg.size(), mImg.depth()));
}

// Basic loader and image edit class --------------------------------------------------------------------
DkBasicLoader::DkBasicLoader()
{
    // mTraining = false;
    mPageIdxDirty = false;
    mNumPages = 1;
    mPageIdx = 1;
    mMetaData = QSharedPointer<DkMetaDataT>(new DkMetaDataT());
}

int DkBasicLoader::getOrientationDegrees(const QImageIOHandler::Transformations transform)
{
    switch (transform) {
    case QImageIOHandler::TransformationNone:
        return 0;
    case QImageIOHandler::TransformationRotate180:
        return 180;
    case QImageIOHandler::TransformationRotate90:
        return 90;
    case QImageIOHandler::TransformationRotate270:
        return -90;
    case QImageIOHandler::TransformationMirrorAndRotate90:
        return -90;
    case QImageIOHandler::TransformationFlipAndRotate90:
        return 90;
    case QImageIOHandler::TransformationFlip:
        return 180;
    case QImageIOHandler::TransformationMirror:
        return 0;
    }
    return DkMetaDataT::or_invalid;
}

bool DkBasicLoader::isOrientationMirrored(const QImageIOHandler::Transformations transform)
{
    switch (transform) {
    case QImageIOHandler::TransformationNone:
    case QImageIOHandler::TransformationRotate180:
    case QImageIOHandler::TransformationRotate90:
    case QImageIOHandler::TransformationRotate270:
        return false;
    case QImageIOHandler::TransformationMirrorAndRotate90:
    case QImageIOHandler::TransformationFlipAndRotate90:
    case QImageIOHandler::TransformationFlip:
    case QImageIOHandler::TransformationMirror:
        return true;
    }
    return false;
}

bool DkBasicLoader::loadGeneral(const QString &filePath, bool loadMetaData, bool fast)
{
    return loadGeneral(filePath, QSharedPointer<QByteArray>(), loadMetaData, fast);
}

bool DkBasicLoader::loadGeneral(const QString &filePath, QSharedPointer<QByteArray> ba, bool loadMetaData, bool fast)
{
    DkTimer dt;

    DkFileInfo fileInfo(filePath);
    if (fileInfo.isShortcut() && !fileInfo.resolveShortcut()) {
        qWarning() << "[Loader] broken shortcut:" << fileInfo.fileName();
        return false;
    }
    mFile = fileInfo.path();

    const QByteArray suffix = fileInfo.suffix().toLower().toLatin1();

    // name of the load method for tracing and also indicates if we loaded successfully
    QString loader;

    // reset edit history and metadata
    release();

    // tiff page handler
    if (mPageIdxDirty)
        if (loadPage())
            loader = "page";

    // mMetaData can never be null due to release()
    Q_ASSERT(mMetaData);

    // this fixes an issue with the new jpg loader
    // Qt considers an orientation of 0 as wrong and fails to load these jpgs
    // however, the old nomacs wrote 0 if the orientation should be cleared
    // so we simply adopt the memory here
    if (loadMetaData)
        mMetaData->readMetaData(fileInfo, ba);

    static const QList<QByteArray> qtFormats = QImageReader::supportedImageFormats();
    static const QList<QByteArray> drifFormats{"drif", "yuv", "raw"};
    static const QList<QByteArray>
        rawFormats{"nef", "nrw", "crw", "cr2", "cr3", "arw", "dng", "raw", "rw2", "mrw", "srw", "orf", "3fr", "x3f", "mos", "pef", "iiq", "raf"};
    static const QList<QByteArray> tiffFormats{"tif", "tiff"};
    static const QList<QByteArray> psdFormats{"psb", "psd"};
    static const QList<QByteArray> jpegFormats{"jpg", "jpeg"};

    //
    // Loader precedence
    //
    // - RAW must precede TIFF (most RAW formats use TIFF container)
    // - Qt should get first attempt as it is more actively maintained (in case of future CVEs etc)
    // - Qt 5.15 TGA plugin cannot read some TGAs correctly, and won't report an error, so try ours first
    // - tiff after Qt as that also has support
    // - psd after Qt as KImageFormats has support
    // - roh/vec go last since they are rarely used, I can't source a test file for either
    //
    // We prefer our RAW loader over KImageFormats
    // - We have an option to disable/enable jpeg preview
    // - We have other options for raw loading (denoise etc)
    //
    QImage img;

    // "Developers Raw Image File" -> https://github.com/ovidiuvio/drif_image
    if (loader.isNull() && drifFormats.contains(suffix)) {
        if (loadDRIF(mFile, img, ba))
            loader = "drif";
    }

    // what is this for...loaders *always* prefer byte array over file
    // if (!imgLoaded && !fInfo.exists() && ba && !ba->isEmpty()) {
    //     imgLoaded = img.loadFromData(*ba.data());
    //     if (imgLoaded)
    //         mLoader = qt_loader;
    // }

    // RAW tries early otherwise Qt's TIFF plugin is used
#ifdef WITH_LIBRAW
    bool libRawUsed = false;
    if (loader.isNull() && rawFormats.contains(suffix)) {
        libRawUsed = true;
        if (loadRAW(mFile, img, ba, fast))
            loader = "raw";
    }
#endif

    // TGA loader adds variants unsupported in QT
    // ideally, this comes after Qt loader fails, however 5.15 will
    // fail silently and produce a bad image in certain cases
    if (loader.isNull() && suffix == "tga") {
        if (loadTGA(mFile, img, ba))
            loader = "tga";
    }

    // Qt loader (by file extension match or by content (no suffix))
    // - if the suffix has no match in Qt, this will fail
    // - if the suffix is empty, plugins will check the file header
    LoaderResult result;
    if (loader.isNull() && (qtFormats.contains(suffix) || suffix.isEmpty())) {
        result = loadQt(mFile, ba, suffix);
        if (result.ok) {
            loader = "qt";
            img = result.img;
        }
    }

    // Tiff loader - supports jpg compressed tiffs
    if (loader.isNull() && tiffFormats.contains(suffix)) {
        if (loadTIFF(mFile, img, ba))
            loader = "tiff";
    }

    // PSD loader
    if (loader.isNull() && psdFormats.contains(suffix)) {
        if (loadPSD(mFile, img, ba))
            loader = "psd";
    }

#ifdef WITH_LIBRAW
    // try RAW again, for unknown extensions
    // - we didn't try with libraw yet
    // - kimageformats-plugins doesn't know it either
    // - "image/jpeg" fixes #435 - thumbnail gets loaded in the RAW loader
    if (loader.isNull() && !libRawUsed && !qtFormats.contains(suffix) && mMetaData->getMimeType() != "image/jpeg") {
        // TODO: sometimes (e.g. _DSC6289.tif) strange opencv errors are thrown - catch them!
        if (loadRAW(mFile, img, ba, fast))
            loader = "raw-unknown-suffix";
    }
#endif

    // Qt loader, unknown/wrong file extension
    if (loader.isNull() && suffix != "roh" && suffix != "vec") {
        result = loadQt(mFile, ba);
        if (result.ok) {
            loader = "qt-unknown-suffix";
            img = result.img;
            qWarning().noquote() << "[Loader]" << fileInfo.fileName() << "seems to have the wrong extension, the content type is" << mMetaData->getMimeType();
        }
    }

    // fix broken samsung panorama images (see #254,#263)
    if (loader.isNull() && jpegFormats.contains(suffix)) {
        if (!ba || ba->isEmpty())
            ba = loadFileToBuffer(filePath);
        if (ba && !ba->isEmpty()) {
            if (DkImage::fixSamsungPanorama(*ba)) {
                result = loadQt(mFile, ba, suffix);
                if (result.ok) {
                    loader = "qt-samsung-panorama";
                    img = result.img;
                }
            }
        }
    }

    // this loader is a bit buggy -> be careful
    if (loader.isNull() && suffix == "roh") {
        if (loadROH(mFile, img, ba))
            loader = "roh";
    }

    // this loader is for OpenCV cascade training files created with opencv_createsamples
    if (loader.isNull() && suffix == "vec") {
        if (loadOpenCVVecFile(mFile, img, ba))
            loader = "vec";
    }

    // tiff things
    if (!loader.isNull() && !mPageIdxDirty)
        indexPages(mFile, ba);
    mPageIdxDirty = false;

    // copy QImage::text() data to DkMetaData
    if (!loader.isNull())
        mMetaData->setQtValues(img);

    int rotation = DkMetaDataT::or_invalid;
    bool mirrored = false;

    // if the loader published a transform, it takes priority
    // The theory is that either this is some non-exif transform (JXL/AVIF/HEIC), or
    // it *is* the exif transform (JPG/TIFF); either way, the image loader is in control
    // of that decision.
    if (loader.startsWith("qt") && result.supportsTransform) {
        auto transform = result.transform;
        rotation = getOrientationDegrees(transform);
        mirrored = isOrientationMirrored(transform);
    }

    if (rotation == DkMetaDataT::or_invalid && loadMetaData && mMetaData->hasMetaData()) {
        rotation = mMetaData->getOrientationDegrees();
        mirrored = mMetaData->isOrientationMirrored();
    }

    bool maybeTransformed = false;
    if (!result.supportsTransform) {
        // JXL, HEIC, and AVIF Qt plugins (from kimageformats), depending on version,
        // may have already rotated the image, even though we asked QIR not to.
        // In this case, do not rotate again using the exif orientation
        static const QList<QByteArray> transformingFormats{"heic", "heif", "avif", "avifs", "jxl"};

        if (transformingFormats.contains(suffix))
            maybeTransformed = true;

        // #1174 kif_raw silently transforms raw files (and does not set TransformedByDefault)
        if (rawFormats.contains(suffix) && QString(loader).startsWith("qt"))
            maybeTransformed = true;
    }

    bool disableTransform = DkSettingsManager::param().metaData().ignoreExifOrientation;
    if (maybeTransformed && disableTransform)
        qWarning() << "[Loader] the plugin for" << suffix << "does not support disabling orientation/transform";

    bool transformed = false;
    if (!loader.isNull() && !disableTransform && !maybeTransformed && rotation != DkMetaDataT::or_invalid && rotation != DkMetaDataT::or_not_set) {
        if (rotation != 0) {
            img = DkImage::rotateImage(img, rotation);
            transformed = true;
        }
        if (mirrored) {
            img = img.mirrored(true, false);
            transformed = true;
        }
    }

    if (!loader.isNull()) {
        setEditImage(img, tr("Original Image"));

        // log some details, this is formatted to make parsing easier
        // e.g. nomacs 2>&1 | grep Loader::  | column -t
        QString formatString;
        {
            QDebug fmt(&formatString); // convert format enum to string
            fmt = fmt.nospace().noquote();
            fmt << img.format();
            if (img.colorCount() > 0)
                fmt << ':' + QString::number(img.colorCount());

            fmt << ' ' << '"' << img.colorSpace().description().replace(' ', '_') << '"';
        }

        QString transformType = "none";
        if (maybeTransformed)
            transformType = "forced"; // image loader passively transforms
        else if (disableTransform)
            transformType = "disabled";
        else if (transformed) {
            if (result.supportsTransform)
                transformType = "loader"; // transform defined by image loader
            else
                transformType = "exif"; // transform from our exif parser

            transformType += ':' + QString::number(rotation);
            transformType += ':' + QString::number(mirrored);
        }

        // animation /  loop count
        QString info = QStringLiteral("[Loader::%1] %2 \"%3\" %4 transform:%5 %6ms")
                           .arg(loader)
                           .arg(fileInfo.fileName())
                           .arg(mMetaData->getMimeType())
                           .arg(formatString)
                           .arg(transformType)
                           .arg(dt.elapsed());
        qInfo().noquote() << info;
    } else
        qWarning().noquote() << "[Loader]" << fileInfo.fileName() << mMetaData->getMimeType() << "failed to load";

    return !loader.isNull();
}

DkBasicLoader::LoaderResult DkBasicLoader::loadQt(const QString &filePath, QSharedPointer<QByteArray> ba, const QByteArray &format)
{
    LoaderResult result;

    std::unique_ptr<QIODevice> device;
    if (ba && !ba->isEmpty())
        device.reset(new QBuffer(ba.get()));
    else
        device.reset(new QFile(filePath));

    if (!device->open(QIODevice::ReadOnly)) {
        qWarning() << "[loadQt] failed to  open file:" << device->errorString();
        return result;
    }

    QImageReader qir(device.get());

    qir.setAutoTransform(false);
    qir.setAutoDetectImageFormat(format.isEmpty());
    qir.setFormat(format);

    // load the largest icon (height*depth)
    int index = -1;
    if (format == "ico" || format == "icns") {
        int maxIndex = 0, maxSize = 0;
        index = 0;
        const uchar data[32] = {0}; // enough for 1x1 64-bit pixels, and some padding for safety
        do {
            QImage img;
            // we can avoid decompression of all sizes, but we need to construct a temporary image
            int size = qir.size().height() * QImage(data, 1, 1, qir.imageFormat()).depth();
            if (size <= 0) {
                if (!qir.read(&img))
                    break;
                size = img.size().height() * img.depth();
            }

            if (size > maxSize) {
                maxIndex = index;
                maxSize = size;
            }
            index++;
        } while (qir.jumpToNextImage());
        qir.jumpToImage(maxIndex);
    }

    result.ok = qir.read(&result.img);

    if (result.ok) {
        result.supportsTransform = qir.supportsOption(QImageIOHandler::ImageTransformation);
        result.transform = qir.transformation();
    }

    if (!result.ok)
        qWarning() << "[loadQt]" << QFileInfo(filePath).fileName() << qir.errorString();

    return result;
}

bool DkBasicLoader::loadROH(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba) const
{
    if (!ba)
        ba = loadFileToBuffer(filePath);
    if (!ba || ba->isEmpty())
        return false;

    bool imgLoaded = false;

    int rohW = 4000;
    int rohH = 2672;
    unsigned char fByte; // first byte
    unsigned char sByte; // second byte

    try {
        const unsigned char *pData = (const unsigned char *)ba->constData();
        unsigned char *buffer = new unsigned char[rohW * rohH];

        if (!buffer)
            return imgLoaded;

        for (long long i = 0; i < (rohW * rohH); i++) {
            fByte = pData[i * 2];
            sByte = pData[i * 2 + 1];
            fByte = fByte >> 4;
            fByte = fByte & 15;
            sByte = sByte << 4;
            sByte = sByte & 240;

            buffer[i] = (fByte | sByte);
        }

        img = QImage(buffer, rohW, rohH, QImage::Format_Indexed8);

        if (img.isNull())
            return imgLoaded;
        else
            imgLoaded = true;

        // img = img.copy();
        QVector<QRgb> colorTable;

        for (int i = 0; i < 256; i++)
            colorTable.push_back(QColor(i, i, i).rgb());

        img.setColorTable(colorTable);

    } catch (...) {
        imgLoaded = false;
    }

    return imgLoaded;
}

bool DkBasicLoader::loadTGA(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba) const
{
    if (!ba || ba->isEmpty())
        ba = loadFileToBuffer(filePath);

    tga::DkTgaLoader tl = tga::DkTgaLoader(ba);

    bool success = tl.load();
    img = tl.image();

    return success;
}

bool DkBasicLoader::loadRAW(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba, bool fast) const
{
    DkRawLoader rawLoader(filePath, mMetaData);
    rawLoader.setLoadFast(fast);

    bool success = rawLoader.load(ba);

    if (success)
        img = rawLoader.image();

    return success;
}

#ifdef Q_OS_WIN
bool DkBasicLoader::loadPSD(const QString &, QImage &, QSharedPointer<QByteArray>) const
{
    qWarning() << "built-in PSD loader unsupported on Windows, you will need a Qt plugin";
#else
bool DkBasicLoader::loadPSD(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba) const
{
    // load from file?
    if (!ba || ba->isEmpty()) {
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);

        QPsdHandler psdHandler;
        psdHandler.setDevice(&file); // QFile is an IODevice
        // psdHandler.setFormat(fileInfo.suffix().toLocal8Bit());

        if (psdHandler.canRead(&file)) {
            bool success = psdHandler.read(&img);
            // setEditImage(img, tr("Original Image"));

            return success;
        }
    } else {
        QBuffer buffer;
        buffer.setData(*ba.data());
        buffer.open(QIODevice::ReadOnly);

        QPsdHandler psdHandler;
        psdHandler.setDevice(&buffer); // QFile is an IODevice
        // psdHandler.setFormat(file.suffix().toLocal8Bit());

        if (psdHandler.canRead(&buffer)) {
            bool success = psdHandler.read(&img);
            // setEditImage(img, tr("Original Image"));

            return success;
        }
    }

#endif // !Q_OS_WIN
    return false;
}

#ifndef WITH_LIBTIFF
bool DkBasicLoader::loadTIFF(const QString &, QImage &, QSharedPointer<QByteArray>) const
{
    qWarning() << "built-in TIFF loader is not included in this build and may be able to load this file";
#else
bool DkBasicLoader::loadTIFF(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba) const
{
    bool success = false;

    // first turn off nasty warning/error dialogs - (we do the GUI : )
    TIFFErrorHandler oldErrorHandler, oldWarningHandler;
    oldWarningHandler = TIFFSetWarningHandler(NULL);
    oldErrorHandler = TIFFSetErrorHandler(NULL);

    DkTimer dt;
    TIFF *tiff = 0;

// TODO: currently TIFFStreamOpen can only be linked on Windows?!
#if defined(Q_OS_WIN)

    std::istringstream is(ba ? ba->toStdString() : "");

    if (ba)
        tiff = TIFFStreamOpen("MemTIFF", &is);

    // fallback to direct loading
    if (!tiff)
        tiff = TIFFOpen(filePath.toLatin1(), "r");

    // FIXME: we could load without buffer if toLatin1()=>toUtf8() and on
    // windows use wchar_t* version of TIFFOpen();

    // loading from buffer allows us to load files with non-latin names
    QSharedPointer<QByteArray> bal;
    if (!tiff)
        bal = loadFileToBuffer(filePath);

    std::istringstream isl(bal ? bal->toStdString() : "");

    if (bal)
        tiff = TIFFStreamOpen("MemTIFF", &isl);
#else
    tiff = TIFFOpen(filePath.toLatin1(), "r");
#endif

    if (!tiff)
        return success;

    uint32_t width = 0;
    uint32_t height = 0;

    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

    // init the qImage
    img = QImage(width, height, QImage::Format_ARGB32);

    const int stopOnError = 1;
    success = TIFFReadRGBAImageOriented(tiff, width, height, reinterpret_cast<uint32_t *>(img.bits()), ORIENTATION_TOPLEFT, stopOnError) != 0;

    if (success) {
        for (uint32_t y = 0; y < height; ++y)
            convert32BitOrder(img.scanLine(y), width);
    }

    TIFFClose(tiff);

    TIFFSetWarningHandler(oldWarningHandler);
    TIFFSetWarningHandler(oldErrorHandler);

    return success;

#endif // !WITH_LIBTIFF
    return false;
}

#define DRIF_IMAGE_IMPL
#include "drif_image.h"

bool isQtFmtCompatible(uint32_t f)
{
    switch (f) {
    case DRIF_FMT_RGB888:
    case DRIF_FMT_RGBA8888:
    case DRIF_FMT_GRAY:
        return true;
    }

    return false;
}

uint32_t drif2qtfmt(uint32_t f)
{
    switch (f) {
    case DRIF_FMT_RGB888:
        return QImage::Format_RGB888;
    case DRIF_FMT_RGBA8888:
        return QImage::Format_RGBA8888;
    case DRIF_FMT_GRAY:
        return QImage::Format_Grayscale8;
    }

    return QImage::Format_Invalid;
}

bool DkBasicLoader::loadDRIF(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba) const
{
    bool success = false;

    uint32_t w;
    uint32_t h;
    uint32_t f;

    uint8_t *imgBytes = drifLoadImg(filePath.toLatin1(), &w, &h, &f);

    if (!imgBytes)
        return success;

    if (isQtFmtCompatible(f)) {
        img = QImage((int)w, (int)h, (QImage::Format)drif2qtfmt(f));
        memcpy(reinterpret_cast<void *>(img.bits()), imgBytes, drifGetSize(w, h, f));

        success = true;
    }
#ifdef WITH_OPENCV
    else {
        img = QImage((int)w, (int)h, QImage::Format_RGB888);

        switch (f) {
        case DRIF_FMT_BGR888: {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC3, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_BGR2RGB);

            success = true;
        } break;

        case DRIF_FMT_RGB888P:
        case DRIF_FMT_RGBA8888P: {
            cv::Mat imgMatR = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes);
            cv::Mat imgMatG = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + w * h);
            cv::Mat imgMatB = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + 2 * w * h);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));

            std::vector<cv::Mat> imgMat{imgMatR, imgMatG, imgMatB};
            cv::merge(imgMat, rgbMat);

            success = true;
        } break;

        case DRIF_FMT_BGR888P:
        case DRIF_FMT_BGRA8888P: {
            cv::Mat imgMatB = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes);
            cv::Mat imgMatG = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + w * h);
            cv::Mat imgMatR = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes + 2 * w * h);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));

            std::vector<cv::Mat> imgMat{imgMatR, imgMatG, imgMatB};
            cv::merge(imgMat, rgbMat);

            success = true;
        } break;

        case DRIF_FMT_BGRA8888: {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC4, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_BGR2RGB, 3);

            success = true;
        } break;

        case DRIF_FMT_RGBA8888: {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC4, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_RGBA2RGB, 3);

            success = true;
        } break;

        case DRIF_FMT_GRAY: {
            cv::Mat imgMat = cv::Mat((int)h, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_GRAY2RGB);

            success = true;
        } break;

        case DRIF_FMT_YUV420P: {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_I420);

            success = true;
        } break;

        case DRIF_FMT_YVU420P: {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_YV12);

            success = true;
        } break;

        case DRIF_FMT_NV12: {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_NV12);

            success = true;
        } break;

        case DRIF_FMT_NV21: {
            cv::Mat imgMat = cv::Mat((int)h + h / 2, (int)w, CV_8UC1, imgBytes);
            cv::Mat rgbMat = cv::Mat((int)h, (int)w, CV_8UC3, reinterpret_cast<uint8_t *>(img.bits()));
            cv::cvtColor(imgMat, rgbMat, CV_YUV2RGB_NV21);

            success = true;
        } break;

        default:
            success = false;
            break;
        }
    }
#endif

    drifFreeImg(imgBytes);

    return success;
}

void DkBasicLoader::setImage(const QImage &img, const QString &editName, const QString &file)
{
    mFile = file;
    setEditImage(img, editName);
}

void DkBasicLoader::pruneEditHistory()
{
    // delete all hidden edit states
    for (int idx = mImages.size() - 1; idx > mImageIndex; idx--) {
        mImages.pop_back();
    }
}

void DkBasicLoader::setEditImage(const QImage &img, const QString &editName)
{
    if (img.isNull())
        return;

    // delete all hidden edit states
    pruneEditHistory();

    // compute new history size
    int historySize = 0;
    for (const DkEditImage &e : mImages) {
        historySize += e.size();
    }

    // reset exif orientation after image edit
    if (!mImages.isEmpty())
        mMetaData->clearOrientation();
    // new history item with new pixmap (and old or original metadata)
    DkEditImage newImg(img, mMetaData->copy(), editName); // new image, old/unchanged metadata

    if (historySize + newImg.size() > DkSettingsManager::param().resources().historyMemory && mImages.size() > mMinHistorySize) {
        mImages.removeAt(1);
        qWarning() << "removing history image because it's too large:" << historySize + newImg.size() << "MB";
    }

    mImages.append(newImg);
    mImageIndex = mImages.size() - 1; // set the index again to the last
}

void DkBasicLoader::setEditMetaData(const QSharedPointer<DkMetaDataT> &metaData, const QImage &img, const QString &editName)
{
    // delete all hidden edit states
    pruneEditHistory();

    // not removing second history item if oversized (see setEditImage())

    // new history item with new metadata (and image, but hasNewImage() will be false)
    DkEditImage newImg(metaData->copy(), img, editName); // new metadata, old/unchanged image

    mImages.append(newImg);
    mImageIndex = mImages.size() - 1; // set the index again to the last
}

void DkBasicLoader::setEditMetaData(const QSharedPointer<DkMetaDataT> &metaData, const QString &editName)
{
    // Add history edit with new metadata (hasMetaData()), copying last or original image
    QImage lastImg = image(); // copy last edit of pixmap (if any) to new history item
    setEditMetaData(metaData, lastImg, editName);
}

void DkBasicLoader::setEditMetaData(const QString &editName)
{
    // Add history edit with edited metadata (hasMetaData()), copying last or original image
    setEditMetaData(mMetaData, image(), editName);
}

QImage DkBasicLoader::lastImage() const
{
    // Find and return the last/current version of the image (ready to be saved to disk)
    // This is initially the first item (the original image) or the last one,
    // excluding history items with images that only have modified metadata,
    // for example, after rotating there'd be a history item with the rotated image
    // but this rotated pixmap is for the gui only, it should not be saved.
    for (int idx = mImageIndex; idx >= 0; idx--) {
        if (mImages[idx].hasNewImage()) {
            return mImages[idx].image();
        }
    }

    return QImage();
}

QImage DkBasicLoader::pixmap() const
{
    // This is sometimes called with an invalid index, for example, after navigating back and forth
    // if the history has been edited (i.e., > 1 entry). Sometimes, the index is -1 and the history is empty,
    // which means we have nothing to return (image requested but nothing loaded). (via ViewPort?)
    if (mImageIndex < 0 || mImageIndex >= mImages.size()) {
        if (mImages.isEmpty())
            return QImage();
        else
            return mImages.last().image();
    }
    // Return current pixmap, which may contain modification from metadata changes like rotation
    // This should not be used to write the image to disk, use image() instead.
    return mImages.at(mImageIndex).image();
}

/**
 * @brief Returns the pointer to the current metadata object which belongs to the loaded image.
 *
 * Note that this is a pointer, not a copy. After changing the metadata, it's necessary
 * to call setEditMetaData(), passing an appropriate edit name, to add a history item (will be copied).
 *
 * @return QSharedPointer<DkMetaDataT>
 */
QSharedPointer<DkMetaDataT> DkBasicLoader::getMetaData() const
{
    QSharedPointer<DkMetaDataT> metaData(mMetaData);
    return metaData;
};

QSharedPointer<DkMetaDataT> DkBasicLoader::lastMetaDataEdit(bool return_nullptr, bool return_orig) const
{
    QSharedPointer<DkMetaDataT> lastEdit; // null edit
    if (return_orig) {
        // Return original metadata only if requested (otherwise only return modified metadata)
        lastEdit = mImages.first().metaData();
    } else if (!return_nullptr) {
        // Empty null object will be returned if no history item (with edited metadata) could be found
        lastEdit = QSharedPointer<DkMetaDataT>(new DkMetaDataT());
    }

    // Get latest modified metadata item from history (or null)
    for (int idx = mImageIndex; idx > 0; idx--) {
        if (mImages[idx].hasNewMetaData()) {
            lastEdit = mImages[idx].metaData();
            break;
        }
    }

    return lastEdit;
}

bool DkBasicLoader::isImageEdited()
{
    for (int i = 1, ii = mImageIndex; i <= ii; i++) {
        if (mImages[i].hasNewImage()) {
            return true;
        }
    }
    return false;
}

bool DkBasicLoader::isMetaDataEdited()
{
    for (int i = 1, ii = mImageIndex; i <= ii; i++) {
        if (mImages[i].hasNewMetaData()) {
            return true;
        }
    }
    return false;
}

void DkBasicLoader::undo()
{
    // Change history index (for image()...)
    if (mImageIndex > 0)
        mImageIndex--;

    // Get last history item with modified metadata (up until new history index)
    QSharedPointer<DkMetaDataT> metaData(mMetaData);
    metaData = lastMetaDataEdit(false, true);
    // Update our current metadata object, which is also used elsewhere (pointer)
    // for example, see DkMetaDataWidgets/DkMetaDataHUD - or DkCommentWidget
    mMetaData->update(metaData);

    // Notify listeners about changed metadata
    emit undoSignal();
    emit resetMetaDataSignal();
}

void DkBasicLoader::redo()
{
    // Change history index (for image()...)
    if (mImageIndex < mImages.size() - 1)
        mImageIndex++;

    // Get last history item with modified metadata (up until new history index)
    QSharedPointer<DkMetaDataT> metaData(mMetaData);
    metaData = lastMetaDataEdit(false, true);
    // Update our current metadata object, which is also used elsewhere (pointer)
    // for example, see DkMetaDataWidgets/DkMetaDataHUD - or DkCommentWidget
    mMetaData->update(metaData);

    // Notify listeners about changed metadata
    emit redoSignal();
    emit resetMetaDataSignal();
}

QVector<DkEditImage> *DkBasicLoader::history()
{
    return &mImages;
}

DkEditImage DkBasicLoader::lastEdit() const
{
    assert(mImageIndex >= 0 && mImageIndex < mImages.size());
    return mImages[mImageIndex];
}

int DkBasicLoader::historyIndex() const
{
    return mImageIndex;
}

void DkBasicLoader::setMinHistorySize(int size)
{
    mMinHistorySize = size;
}

void DkBasicLoader::setHistoryIndex(int idx)
{
    mImageIndex = idx;
    // TODO update mMetaData, see undo()
}

QSharedPointer<QByteArray> DkBasicLoader::loadFileToBuffer(const QString &filePath)
{
    DkFileInfo file(filePath);
    Q_ASSERT(file.isFile());

    std::unique_ptr<QIODevice> io = file.getIODevice();
    if (!io)
        return {};

    return QSharedPointer<QByteArray>(new QByteArray(io->readAll()));
}

/**
 * @brief writeBufferToFile() writes the passed in file buffer to the specified file.
 *
 * It's called by the save() routine, which saves the image to that file buffer
 * and updates that file buffer to also contain exif data.
 *
 * @param fileInfo path to file to be written
 * @param ba raw content to be written to file
 */
bool DkBasicLoader::writeBufferToFile(const QString &fileInfo, const QSharedPointer<QByteArray> ba) const
{
    if (!ba || ba->isEmpty())
        return false;

    QFile file(fileInfo);
    file.open(QIODevice::WriteOnly);
    qint64 bytesWritten = file.write(*ba.data(), ba->size());
    file.close();
    qDebug() << "[DkBasicLoader] buffer saved, bytes written: " << bytesWritten;

    if (!bytesWritten || bytesWritten == -1)
        return false;

    return true;
}

void DkBasicLoader::indexPages(const QString &filePath, const QSharedPointer<QByteArray> ba)
{
    // reset counters
    mNumPages = 1;
    mPageIdx = 1;

#ifdef WITH_LIBTIFF

    QFileInfo fInfo(filePath);

    // for now we just support tiff's
    if (!fInfo.suffix().contains(QRegularExpression("(tif|tiff)", QRegularExpression::CaseInsensitiveOption)))
        return;

    // first turn off nasty warning/error dialogs - (we do the GUI : )
    TIFFErrorHandler oldErrorHandler, oldWarningHandler;
    oldWarningHandler = TIFFSetWarningHandler(NULL);
    oldErrorHandler = TIFFSetErrorHandler(NULL);

    DkTimer dt;
    TIFF *tiff = 0;

#if defined(Q_OS_WIN)
    std::istringstream is(ba ? ba->toStdString() : "");

    if (ba)
        tiff = TIFFStreamOpen("MemTIFF", &is);

    // read from file
    if (!tiff)
        tiff = TIFFOpen(filePath.toLatin1(), "r"); // this->mFile was here before - not sure why

    // loading from buffer allows us to load files with non-latin names
    QSharedPointer<QByteArray> bal;
    if (!tiff)
        bal = loadFileToBuffer(filePath);
    ;
    std::istringstream isl(bal ? bal->toStdString() : "");

    if (bal)
        tiff = TIFFStreamOpen("MemTIFF", &isl);
#else
    // read from file
    tiff = TIFFOpen(filePath.toLatin1(), "r"); // this->mFile was here before - not sure why
#endif

    if (!tiff)
        return;

    // libtiff example
    int dircount = 0;

    do {
        dircount++;

    } while (TIFFReadDirectory(tiff));

    mNumPages = dircount;

    if (mNumPages > 1)
        mPageIdx = 1;

    qDebug() << dircount << " TIFF directories... " << dt;
    TIFFClose(tiff);

    TIFFSetWarningHandler(oldWarningHandler);
    TIFFSetWarningHandler(oldErrorHandler);
#else
    Q_UNUSED(filePath);
#endif
}

bool DkBasicLoader::loadPage(int skipIdx)
{
    bool imgLoaded = false;

    mPageIdx += skipIdx;

    // <= 1 since first page is loaded using qt
    if (mPageIdx > mNumPages || mPageIdx <= 1)
        return imgLoaded;

    return loadPageAt(mPageIdx);
}

bool DkBasicLoader::loadPageAt(int pageIdx)
{
    bool imgLoaded = false;

#ifdef WITH_LIBTIFF

    // <= 1 since first page is loaded using qt
    if (pageIdx > mNumPages || pageIdx < 1)
        return imgLoaded;

    // first turn off nasty warning/error dialogs - (we do the GUI : )
    TIFFErrorHandler oldErrorHandler, oldWarningHandler;
    oldWarningHandler = TIFFSetWarningHandler(NULL);
    oldErrorHandler = TIFFSetErrorHandler(NULL);

    DkTimer dt;
    TIFF *tiff = TIFFOpen(mFile.toLatin1(), "r");

#if defined(Q_OS_WIN)

    // loading from buffer allows us to load files with non-latin names
    QSharedPointer<QByteArray> ba;
    if (!tiff)
        ba = loadFileToBuffer(mFile);

    std::istringstream is(ba ? ba->toStdString() : "");
    if (ba)
        tiff = TIFFStreamOpen("MemTIFF", &is);
#endif

    if (!tiff)
        return imgLoaded;

    uint32_t width = 0;
    uint32_t height = 0;

    // go to current directory
    for (int idx = 1; idx < pageIdx; idx++) {
        if (!TIFFReadDirectory(tiff))
            return false;
    }
    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

    // init the qImage
    QImage img = QImage(width, height, QImage::Format_ARGB32);

    const int stopOnError = 1;
    imgLoaded = TIFFReadRGBAImageOriented(tiff, width, height, reinterpret_cast<uint32_t *>(img.bits()), ORIENTATION_TOPLEFT, stopOnError) != 0;

    if (imgLoaded) {
        for (uint32_t y = 0; y < height; ++y)
            convert32BitOrder(img.scanLine(y), width);
    }

    TIFFClose(tiff);

    TIFFSetWarningHandler(oldWarningHandler);
    TIFFSetWarningHandler(oldErrorHandler);

    setEditImage(img, tr("Original Image"));
#else
    Q_UNUSED(pageIdx);
#endif

    return imgLoaded;
}

bool DkBasicLoader::setPageIdx(int skipIdx)
{
    // do nothing if we don't have tiff pages
    if (mNumPages <= 1)
        return false;

    mPageIdxDirty = false;

    int newPageIdx = mPageIdx + skipIdx;

    if (newPageIdx > 0 && newPageIdx <= mNumPages) {
        mPageIdxDirty = true;
        mPageIdx = newPageIdx;
    }

    return mPageIdxDirty;
}

void DkBasicLoader::resetPageIdx()
{
    mPageIdxDirty = false;
    mPageIdx = 1;
}

void DkBasicLoader::convert32BitOrder(void *buffer, int width) const
{
#ifdef WITH_LIBTIFF
    // code from Qt QTiffHandler
    uint32_t *target = reinterpret_cast<uint32_t *>(buffer);
    for (uint32_t x = 0; x < width; ++x) {
        uint32_t p = target[x];
        // convert between ARGB and ABGR
        target[x] = (p & 0xff000000) | ((p & 0x00ff0000) >> 16) | (p & 0x0000ff00) | ((p & 0x000000ff) << 16);
    }
#else
    Q_UNUSED(buffer);
    Q_UNUSED(width);
#endif
}

/**
 * @brief saves the image and its metadata to the specified file.
 *
 * It writes the image to a file buffer and then writes that buffer back to the original file.
 * Modified metadata is saved afterwards.
 *
 * @param filePath target path to image file
 * @param img source image to be written to file (may be converted along the way)
 * @param compression compression flag for QImageWriter
 */
QString DkBasicLoader::save(const QString &filePath, const QImage &img, int compression)
{
    QSharedPointer<QByteArray> ba;

    DkTimer dt;
    if (saveToBuffer(filePath, img, ba, compression) && ba) {
        if (writeBufferToFile(filePath, ba)) {
            qInfo() << "saved to" << filePath << "in" << dt;
            return filePath;
        }
    }

    return QString();
}

/**
 * @brief saveToBuffer() writes the image matrix img to the file buffer.
 *
 * The file path is used to convert the image based on the file suffix.
 *
 * @param filePath path to file to which this image will later be written, the suffix is relevant
 * @param img image to be written to file buffer
 * @param ba in-memory file buffer containing resulting file
 * @param compression compression flag for QImageWriter
 */
bool DkBasicLoader::saveToBuffer(const QString &filePath, const QImage &img, QSharedPointer<QByteArray> &ba, int compression) const
{
    bool bufferCreated = false;

    if (!ba) {
        ba = QSharedPointer<QByteArray>(new QByteArray());
        bufferCreated = true;
    }
    // copy current metadata object: mMetaData pointer may be reset in the background in the process
    // and then it won't be saved because !isLoaded()... [2022-08, pse]
    QSharedPointer<DkMetaDataT> metaData = mMetaData;

    bool saved = false;

    QFileInfo fInfo(filePath);

#ifdef Q_OS_WIN
    if (0 == fInfo.suffix().compare("ico", Qt::CaseInsensitive)) {
        saved = saveWindowsIcon(img, ba);
    } else
#endif
    {
        bool hasAlpha = DkImage::alphaChannelUsed(img);

        QImage sImg = img;

        // JPEG 2000 can only handle 32 or 8bit images
        if (!hasAlpha && img.colorTable().empty() && !fInfo.suffix().contains(QRegularExpression("(avif|j2k|jp2|jpf|jpx|jxl|png)"))) {
            sImg = sImg.convertToFormat(QImage::Format_RGB888);
        } else if (fInfo.suffix().contains(QRegularExpression("(j2k|jp2|jpf|jpx)")) && sImg.depth() != 32 && sImg.depth() != 8) {
            if (sImg.hasAlphaChannel()) {
                sImg = sImg.convertToFormat(QImage::Format_ARGB32);
            } else {
                sImg = sImg.convertToFormat(QImage::Format_RGB32);
            }
        }

        if (fInfo.suffix().contains(QRegularExpression("(png)")))
            compression = -1;

        QBuffer fileBuffer(ba.data());
        // size_t s = fileBuffer.size();
        fileBuffer.open(QIODevice::WriteOnly);
        QImageWriter *imgWriter = new QImageWriter(&fileBuffer, fInfo.suffix().toStdString().c_str());

        if (compression >= 0) { // -1 -> use Qt's default
            imgWriter->setCompression(compression);
            imgWriter->setQuality(compression);
        }
        if (compression == -1 && imgWriter->format() == "jpg") {
            imgWriter->setQuality(DkSettingsManager::instance().settings().app().defaultJpgQuality);
        }

        imgWriter->setOptimizedWrite(true); // this saves space TODO: user option here?
        imgWriter->setProgressiveScanWrite(true);

        saved = imgWriter->write(sImg); // hint: release() might run now, resetting mMetaData which is used below [2022-08, pse]
        delete imgWriter;
    }

    if (saved && metaData) {
        if (!metaData->isLoaded() || !metaData->hasMetaData()) {
            if (!bufferCreated)
                metaData->readMetaData(filePath, ba);
            else
                // if we created the buffere here - force loading metadata from the file
                metaData->readMetaData(filePath);
        }

        // If we have metadata for the image, save it
        // If your images are saved without metadata, check if the metadata object is discarded or reset
        // causing isLoaded() to return false (glitch on reload) - pse
        if (metaData->isLoaded()) {
            try {
                // be careful: here we actually lie about the constness
                metaData->updateImageMetaData(img, false); // set dimensions in exif (do not reset exif orientation)
                if (!metaData->saveMetaData(ba, true))
                    metaData->clearExifState();
            } catch (...) {
                // is it still throwing anything?
                qInfo() << "Sorry, I could not save the meta data...";
                // clear exif state here -> the 'dirty' flag would otherwise edit the original image (see #514)
                metaData->clearExifState();
            }
        }
    }

    if (!saved)
        emit errorDialogSignal(tr("Sorry, I could not save: %1").arg(fInfo.fileName()));

    return saved;
}

void DkBasicLoader::saveThumbToMetaData(const QString &filePath)
{
    QSharedPointer<QByteArray> ba; // dummy
    saveThumbToMetaData(filePath, ba);
}

void DkBasicLoader::saveThumbToMetaData(const QString &filePath, QSharedPointer<QByteArray> &ba)
{
    if (!hasImage())
        return;

    mMetaData->setThumbnail(DkImage::createThumb(image()));
    saveMetaData(filePath, ba);
}

/**
 * @brief this will write the current exif/metadata to the loaded file.
 *
 * It calls the other overload passing an empty buffer,
 * so it'll load the buffer, save the exif data to the buffer
 * and write the buffer back to the file.
 *
 * @param filePath path to current file to be updated
 */
void DkBasicLoader::saveMetaData(const QString &filePath)
{
    QSharedPointer<QByteArray> ba; // dummy
    saveMetaData(filePath, ba);
}

/**
 * @brief writes metadata to the file on disk, if it's marked as dirty
 *
 * This routine will write new metadata to the file on disk if metadata is marked dirty.
 * It does this by first loading the file into a buffer (unless a non-empty buffer is passed),
 * then it calls the MetaData module to save the exif data to that buffer
 * and finally, it writes the modified buffer to the file on disk.
 * The MetaData module has an overload which does basically the same thing.
 *
 * See ImageLoader (regular workflow starts there) and ImageContainer.
 *
 * @param filePath path to image file
 * @param ba file buffer to be saved (leave empty to work on the specified file as it is on disk)
 */
void DkBasicLoader::saveMetaData(const QString &filePath, QSharedPointer<QByteArray> &ba)
{
    if (!ba)
        ba = QSharedPointer<QByteArray>(new QByteArray());

    if (ba->isEmpty() && mMetaData->isDirty()) {
        ba = loadFileToBuffer(filePath);
    }

    // Update in-memory copy of file (ba) with new meta data
    bool saved = false;
    try {
        saved = mMetaData->saveMetaData(ba);
    } catch (...) {
        qInfo() << "could not save metadata...";
    }

    // Write in-memory copy to specified file - use this overload only if you really need it
    if (saved)
        writeBufferToFile(filePath, ba);
}

bool DkBasicLoader::isContainer(const QString &filePath)
{
    QFileInfo fInfo(filePath);
    if (!fInfo.isFile() || !fInfo.exists())
        return false;

    QString suffix = fInfo.suffix();

    if (suffix.isEmpty())
        return false;

    for (int idx = 0; idx < DkSettingsManager::param().app().containerFilters.size(); idx++) {
        if (DkSettingsManager::param().app().containerFilters[idx].contains(suffix))
            return true;
    }

    return false;
}

/**
 * @brief releases the currently loaded images.
 *
 * Clears the history.
 * Called by loadGeneral() and ImageContainer::clear().
 *
 * @note This will *not* silently auto-save your beautiful images.
 * It was apparently intended to be used that way (it called saveMetaData(), like ~DkImageContainerT()).
 * All changes should be explicitly committed, including exif notes.
 * If you think this is wrong, a comment would be appreciated. See issue #799. PSE, 2022.
 *
 **/
void DkBasicLoader::release()
{
    // TODO: auto save routines here?
    // answer: no.

    mImages.clear(); // clear history
    mImageIndex = -1;

    // Unload metadata
    mMetaData = QSharedPointer<DkMetaDataT>(new DkMetaDataT());
}

#ifdef Q_OS_WIN
// bool DkBasicLoader::saveWindowsIcon(const QString &filePath, const QImage &img) const
// {
//     QSharedPointer<QByteArray> ba;

//     if (saveWindowsIcon(img, ba) && ba && !ba->isEmpty()) {
//         writeBufferToFile(filePath, ba);
//         return true;
//     }

//     return false;
// }

struct ICONDIRENTRY {
    UCHAR nWidth;
    UCHAR nHeight;
    UCHAR nNumColorsInPalette; // 0 if no palette
    UCHAR nReserved; // should be 0
    WORD nNumColorPlanes; // 0 or 1
    WORD nBitsPerPixel;
    ULONG nDataLength; // length in bytes
    ULONG nOffset; // offset of BMP or PNG data from beginning of file
};

bool DkBasicLoader::saveWindowsIcon(const QImage &img, QSharedPointer<QByteArray> &ba) const
{
    // this code is an adopted version of:
    // http://stackoverflow.com/questions/2289894/how-can-i-save-hicon-to-an-ico-file

    if (!ba)
        ba = QSharedPointer<QByteArray>(new QByteArray());

    HICON hIcon = img.toHICON();
    int nColorBits = 32;

    QBuffer buffer(ba.data());
    buffer.open(QIODevice::WriteOnly);

    if (!hIcon)
        return false;

    HDC screenDevice = GetDC(0);

    // Write header:
    UCHAR icoHeader[6] = {0, 0, 1, 0, 1, 0}; // ICO file with 1 image
    buffer.write((const char *)(&icoHeader), sizeof(icoHeader));

    // Get information about icon:
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);
    HGDIOBJ handle1(iconInfo.hbmColor); // free bitmaps when function ends
    BITMAPINFO bmInfo = {0};
    bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo.bmiHeader.biBitCount = 0; // don't get the color table
    if (!GetDIBits(screenDevice, iconInfo.hbmColor, 0, 0, NULL, &bmInfo, DIB_RGB_COLORS)) {
        return false;
    }

    // Allocate size of bitmap info header plus space for color table:
    int nBmInfoSize = sizeof(BITMAPINFOHEADER);
    if (nColorBits < 24) {
        nBmInfoSize += sizeof(RGBQUAD) * (int)(1 << nColorBits);
    }

    QSharedPointer<UCHAR> bitmapInfo(new UCHAR[nBmInfoSize]);
    BITMAPINFO *pBmInfo = (BITMAPINFO *)bitmapInfo.data();
    memcpy(pBmInfo, &bmInfo, sizeof(BITMAPINFOHEADER));

    // Get bitmap data:
    QSharedPointer<UCHAR> bits(new UCHAR[bmInfo.bmiHeader.biSizeImage]);
    pBmInfo->bmiHeader.biBitCount = (WORD)nColorBits;
    pBmInfo->bmiHeader.biCompression = BI_RGB;
    if (!GetDIBits(screenDevice, iconInfo.hbmColor, 0, bmInfo.bmiHeader.biHeight, bits.data(), pBmInfo, DIB_RGB_COLORS)) {
        return false;
    }

    // Get mask data:
    BITMAPINFO maskInfo = {0};
    maskInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    maskInfo.bmiHeader.biBitCount = 0; // don't get the color table
    if (!GetDIBits(screenDevice, iconInfo.hbmMask, 0, 0, NULL, &maskInfo, DIB_RGB_COLORS)) {
        return false;
    }

    QSharedPointer<UCHAR> maskBits(new UCHAR[maskInfo.bmiHeader.biSizeImage]);
    QSharedPointer<UCHAR> maskInfoBytes(new UCHAR[sizeof(BITMAPINFO) + 2 * sizeof(RGBQUAD)]);
    BITMAPINFO *pMaskInfo = (BITMAPINFO *)maskInfoBytes.data();
    memcpy(pMaskInfo, &maskInfo, sizeof(maskInfo));
    if (!GetDIBits(screenDevice, iconInfo.hbmMask, 0, maskInfo.bmiHeader.biHeight, maskBits.data(), pMaskInfo, DIB_RGB_COLORS)) {
        return false;
    }

    // Write directory entry:
    ICONDIRENTRY dir;
    dir.nWidth = (UCHAR)pBmInfo->bmiHeader.biWidth;
    dir.nHeight = (UCHAR)pBmInfo->bmiHeader.biHeight;
    dir.nNumColorsInPalette = (nColorBits == 4 ? 16 : 0);
    dir.nReserved = 0;
    dir.nNumColorPlanes = 0;
    dir.nBitsPerPixel = pBmInfo->bmiHeader.biBitCount;
    dir.nDataLength = pBmInfo->bmiHeader.biSizeImage + pMaskInfo->bmiHeader.biSizeImage + nBmInfoSize;
    dir.nOffset = sizeof(dir) + sizeof(icoHeader);
    buffer.write((const char *)&dir, sizeof(dir));

    // Write DIB header (including color table):
    int nBitsSize = pBmInfo->bmiHeader.biSizeImage;
    pBmInfo->bmiHeader.biHeight *= 2; // because the header is for both image and mask
    pBmInfo->bmiHeader.biCompression = 0;
    pBmInfo->bmiHeader.biSizeImage += pMaskInfo->bmiHeader.biSizeImage; // because the header is for both image and mask
    buffer.write((const char *)&pBmInfo->bmiHeader, nBmInfoSize);

    // Write image data:
    buffer.write((const char *)bits.data(), nBitsSize);

    // Write mask data:
    buffer.write((const char *)maskBits.data(), pMaskInfo->bmiHeader.biSizeImage);

    buffer.close();

    DeleteObject(handle1);

    return true;
}

#endif // #ifdef Q_OS_WIN

#ifdef WITH_OPENCV

// cv::Mat DkBasicLoader::getImageCv()
// {
//     return cv::Mat();
// }

bool DkBasicLoader::loadOpenCVVecFile(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba, QSize s) const
{
    if (!ba)
        ba = QSharedPointer<QByteArray>(new QByteArray());

    // load from file?
    if (ba->isEmpty())
        ba = loadFileToBuffer(filePath);

    if (ba->isEmpty())
        return false;

    // read header & get a pointer to the first image
    int fileCount, vecSize;
    const unsigned char *imgPtr = (const unsigned char *)ba->constData();
    if (!readHeader(&imgPtr, fileCount, vecSize))
        return false;

    int guessedW = 0;
    int guessedH = 0;

    getPatchSizeFromFileName(QFileInfo(filePath).fileName(), guessedW, guessedH);

    qDebug() << "patch size from filename: " << guessedW << " x " << guessedH;

    if (vecSize > 0 && !guessedH && !guessedW) {
        guessedW = qFloor(sqrt((float)vecSize));
        if (guessedW > 0)
            guessedH = vecSize / guessedW;
    }

    if (guessedW <= 0 || guessedH <= 0 || guessedW * guessedH != vecSize) {
        // TODO: ask user
        qDebug() << "dimensions do not match, patch size: " << guessedW << " x " << guessedH << " vecSize: " << vecSize;
        return false;
    }

    int fSize = ba->size();
    int numElements = 0;

    // guess size
    if (s.isEmpty()) {
        double nEl = (fSize - 64) / (vecSize * 2);
        nEl = (fSize - 64 - qCeil(nEl)) / (vecSize * 2) + 1; // opencv adds one byte per image - so we take care for this here

        if (qFloor(nEl) != qCeil(nEl))
            return false;
        numElements = qRound(nEl);
    }

    double nRowsCols = sqrt(numElements);
    int numCols = qCeil(nRowsCols);
    int minusOneRow = (qFloor(nRowsCols) != qCeil(nRowsCols) && nRowsCols - qFloor(nRowsCols) < 0.5) ? 1 : 0;

    cv::Mat allPatches((numCols - minusOneRow) * guessedH, numCols * guessedW, CV_8UC1, cv::Scalar(125));

    for (int idx = 0; idx < numElements; idx++) {
        if (*imgPtr != 0) {
            qDebug() << "skipping non-empty byte - there is something seriously wrong here!";
            // return false;	// stop if the byte is non-empty -> otherwise we might read wrong memory
        }

        imgPtr++; // there is an empty byte between images
        cv::Mat cPatch = getPatch(&imgPtr, QSize(guessedW, guessedH));
        cv::Mat cPatchAll = allPatches(cv::Rect(idx % numCols * guessedW, qFloor(idx / numCols) * guessedH, guessedW, guessedH));

        if (!cPatchAll.empty())
            cPatch.copyTo(cPatchAll);
    }

    img = DkImage::mat2QImage(allPatches);
    img = img.convertToFormat(QImage::Format_ARGB32);

    // setEditImage(img, tr("Original Image"));

    return true;
}

void DkBasicLoader::getPatchSizeFromFileName(const QString &fileName, int &width, int &height) const
{
    // parse patch size from file
    QStringList sections = fileName.split(QRegularExpression("[-\\.]"));

    for (int idx = 0; idx < sections.size(); idx++) {
        QString tmpSec = sections[idx];
        qDebug() << "section: " << tmpSec;

        if (tmpSec.contains("w"))
            width = tmpSec.remove("w").toInt();
        else if (tmpSec.contains("h"))
            height = tmpSec.remove("h").toInt();
    }
}

bool DkBasicLoader::readHeader(const unsigned char **dataPtr, int &fileCount, int &vecSize) const
{
    const int *pData = (const int *)*dataPtr;
    fileCount = *pData;
    pData++; // read file count
    vecSize = *pData; // read vec size

    qDebug() << "vec size: " << vecSize << " fileCount " << fileCount;

    *dataPtr += 12; // skip the first 12 (header) bytes

    return true;
}

// the double pointer is here needed to additionally increase the pointer value
cv::Mat DkBasicLoader::getPatch(const unsigned char **dataPtr, QSize patchSize) const
{
    cv::Mat img8U(patchSize.height(), patchSize.width(), CV_8UC1, cv::Scalar(0));

    // ok, take just the second byte
    for (int rIdx = 0; rIdx < img8U.rows; rIdx++) {
        unsigned char *ptr8U = img8U.ptr<unsigned char>(rIdx);

        for (int cIdx = 0; cIdx < img8U.cols; cIdx++) {
            ptr8U[cIdx] = **dataPtr;
            *dataPtr += 2; // it is strange: opencv stores vec files as 16 bit but just use the 2nd byte
        }
    }

    return img8U;
}

int DkBasicLoader::mergeVecFiles(const QStringList &vecFilePaths, QString &saveFilePath) const
{
    int lastVecSize = 0;
    int totalFileCount = 0;
    int vecCount = 0;
    int pWidth = 0, pHeight = 0;
    QByteArray vecBuffer;

    for (const QString &filePath : vecFilePaths) {
        QFileInfo fInfo(filePath);
        QSharedPointer<QByteArray> ba = loadFileToBuffer(filePath);
        if (ba->isEmpty()) {
            qDebug() << "could not load: " << fInfo.fileName();
            continue;
        }

        int fileCount, vecSize;
        const unsigned char *dataPtr = (const unsigned char *)ba->constData();
        if (!readHeader(&dataPtr, fileCount, vecSize)) {
            qDebug() << "could not read header, skipping: " << fInfo.fileName();
            continue;
        }

        if (lastVecSize && vecSize != lastVecSize) {
            qDebug() << "wrong vec size, skipping: " << fInfo.fileName();
            continue;
        }

        vecBuffer.append((const char *)dataPtr, vecSize * fileCount * 2 + fileCount); // +fileCount accounts for the '\0' bytes between the patches

        getPatchSizeFromFileName(fInfo.fileName(), pWidth, pHeight);

        totalFileCount += fileCount;
        lastVecSize = vecSize;

        vecCount++;
    }

    // don't save if we could not merge the files
    if (!vecCount)
        return vecCount;

    unsigned int *header = new unsigned int[3];
    header[0] = totalFileCount;
    header[1] = lastVecSize;
    header[2] = 0;

    vecBuffer.prepend((const char *)header, 3 * sizeof(int));

    QFileInfo saveFileInfo(saveFilePath);

    // append width, height if we don't know
    if (pWidth && pHeight) {
        QString whString = "-w" + QString::number(pWidth) + "-h" + QString::number(pHeight);
        saveFileInfo = QFileInfo(saveFileInfo.absolutePath(), saveFileInfo.baseName() + whString + "." + saveFileInfo.suffix());
    }

    QFile file(saveFileInfo.absoluteFilePath());
    file.open(QIODevice::WriteOnly);
    file.write(vecBuffer);
    file.close();

    return vecCount;
}

#endif // #ifdef WITH_OPENCV

// FileDownloader --------------------------------------------------------------------
FileDownloader::FileDownloader(const QUrl &imageUrl, const QString &filePath, QObject *parent)
    : QObject(parent)
{
    mFilePath = filePath;

    QNetworkProxyQuery npq(QUrl("https://google.com"));
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (!listOfProxies.empty() && listOfProxies[0].hostName() != "") {
        mWebCtrl.setProxy(listOfProxies[0]);
    }

    connect(&mWebCtrl, &QNetworkAccessManager::finished, this, &FileDownloader::fileDownloaded);

    downloadFile(imageUrl);
}

FileDownloader::~FileDownloader()
{
}

void FileDownloader::downloadFile(const QUrl &url)
{
    QNetworkRequest request(url);
    mWebCtrl.get(request);
    mUrl = url;
}

void FileDownloader::saved()
{
    if (mSaveWatcher.result()) {
        qInfo() << "downloaded image saved to" << mFilePath;
        emit downloaded(mFilePath);
    } else {
        qWarning() << "could not download file to " << mFilePath;
    }
}

bool FileDownloader::save(const QString &filePath, const QSharedPointer<QByteArray> data)
{
    if (!data) {
        qWarning() << "cannot save file if data is NULL";
        return false;
    }

    QFileInfo fi(filePath);

    if (!fi.absoluteDir().exists())
        QDir().mkpath(fi.absolutePath());

    QFile f(filePath);
    f.open(QIODevice::WriteOnly);

    return f.write(*data);
}

void FileDownloader::fileDownloaded(QNetworkReply *pReply)
{
    if (pReply->error() != QNetworkReply::NoError) {
        qWarning() << "I could not download: " << mUrl;
        qWarning() << pReply->errorString();
    }

    mDownloadedData = QSharedPointer<QByteArray>(new QByteArray(pReply->readAll()));
    // emit a signal
    pReply->deleteLater();

    // data only requested
    if (mFilePath.isEmpty()) {
        emit downloaded();
    }
    // ok save it
    else {
        connect(&mSaveWatcher, &QFutureWatcherBase::finished, this, &FileDownloader::saved, Qt::UniqueConnection);
        mSaveWatcher.setFuture(QtConcurrent::run([&] {
            return save(mFilePath, mDownloadedData);
        }));
    }
}

QSharedPointer<QByteArray> FileDownloader::downloadedData() const
{
    return mDownloadedData;
}

QUrl FileDownloader::getUrl() const
{
    return mUrl;
}

// DkRawLoader --------------------------------------------------------------------
DkRawLoader::DkRawLoader(const QString &filePath, const QSharedPointer<DkMetaDataT> &metaData)
{
    mFilePath = filePath;
    mMetaData = metaData;
}

bool DkRawLoader::isEmpty() const
{
    return mFilePath.isEmpty();
}

void DkRawLoader::setLoadFast(bool fast)
{
    mLoadFast = fast;
}

bool DkRawLoader::load(const QSharedPointer<QByteArray> ba)
{
    DkTimer dt;

    // try fetching the preview
    if (loadPreview(ba))
        return true;

#ifdef WITH_LIBRAW

    try {
        // open the buffer
        auto libRawPtr = std::make_unique<LibRaw>(); // huge stack allocation crashes MacOS
        auto &iProcessor = *libRawPtr;

        iProcessor.imgdata.params.use_camera_wb = 1;
        iProcessor.imgdata.params.output_color = 1;
        iProcessor.imgdata.params.output_bps = 8;
        iProcessor.imgdata.params.four_color_rgb = 1;
        iProcessor.imgdata.params.user_flip = 0;

        if (DkSettingsManager::param().resources().filterRawImages) {
            iProcessor.imgdata.params.user_qual = 3;
            iProcessor.imgdata.params.dcb_enhance_fl = 1;
            iProcessor.imgdata.params.fbdd_noiserd = 2;
        } else {
            iProcessor.imgdata.params.user_qual = 0;
            iProcessor.imgdata.params.dcb_enhance_fl = 0;
            iProcessor.imgdata.params.fbdd_noiserd = 0;
        }

        if (!openBuffer(ba, iProcessor)) {
            qDebug() << "could not open buffer for" << mFilePath;
            return false;
        }

        // check camera models for specific hacks
        detectSpecialCamera(iProcessor);

        // try loading RAW preview
        if (mLoadFast) {
            mImg = loadPreviewRaw(iProcessor);
            mImg.setText("RAW.IsPreview", mImg.isNull() ? "no" : "yes");

            // are we done already?
            if (!mImg.isNull())
                return true;
        }

        // unpack the data
        int error = iProcessor.unpack();
        if (std::strcmp(iProcessor.version(), "0.13.5") != 0) // fixes a bug specific to libraw 13 - version call is UNTESTED
            iProcessor.raw2image();

        if (error != LIBRAW_SUCCESS)
            return false;

        // develop using libraw
        error = iProcessor.dcraw_process();

        auto rimg = iProcessor.dcraw_make_mem_image();

        if (rimg) {
            mImg = QImage(rimg->data, rimg->width, rimg->height, rimg->width * 3, QImage::Format_RGB888);
            mImg = mImg.copy(); // make a deep copy...
            mImg.setColorSpace(QColorSpace(QColorSpace::SRgb));
            LibRaw::dcraw_clear_mem(rimg);
            mImg.setText("RAW.Loader", "Default");
            mImg.setText("RAW.IsPreview", "no");
            return true;
        }

        // demosaic image
        cv::Mat rawMat;

        QHash<QString, QString> info; // info for mImg.setText()
        info.insert("RAW.Loader", "Nomacs");
        info.insert("RAW.IsPreview", "no");

        if (iProcessor.imgdata.idata.filters) {
            rawMat = demosaic(iProcessor);
            info.insert("RAW.Processing", "Demosaic");
        } else {
            rawMat = prepareImg(iProcessor);
            info.insert("RAW.Processing", "Copy");
        }

        // color correction + white balance
        if (mIsChromatic)
            whiteBalance(iProcessor, rawMat);

        info.insert("RAW.ColorCorrection", mIsChromatic ? "yes" : "no");

        // gamma correction
        gammaCorrection(iProcessor, rawMat);

        // reduce color noise
        bool noiseReduced = false;
        if (DkSettingsManager::param().resources().filterRawImages && mIsChromatic) {
            reduceColorNoise(iProcessor, rawMat);
            noiseReduced = true;
        }
        info.insert("RAW.NoiseReduction", noiseReduced ? "yes" : "no");

        mImg = raw2Img(iProcessor, rawMat);

        for (auto &key : std::as_const(info).keys())
            mImg.setText(key, info.value(key));

        // qDebug() << "img size" << mImg.size();
        // qDebug() << "raw mat size" << rawMat.rows << "x" << rawMat.cols;
        iProcessor.recycle();
        rawMat.release();
    } catch (...) {
        qDebug() << "[RAW] error during processing...";
        return false;
    }

    qInfo() << "[RAW] loaded in " << dt;

#endif

    return !mImg.isNull();
}

QImage DkRawLoader::image() const
{
    return mImg;
}

bool DkRawLoader::loadPreview(const QSharedPointer<QByteArray> &ba)
{
    try {
        // try to get preview image from exiv2
        if (mMetaData) {
            if (mLoadFast || DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_always
                || DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large) {
                mMetaData->readMetaData(mFilePath, ba);

                int minWidth = 0;

#ifdef WITH_LIBRAW // if nomacs has libraw - we can still hope for a fallback -> otherwise try whatever we have here
                if (DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large)
                    minWidth = 1920;
#endif
                mImg = mMetaData->getPreviewImage(minWidth);

                if (!mImg.isNull()) {
                    qDebug() << "[RAW] loaded with exiv2";
                    return true;
                }
            }
        }
    } catch (...) {
        qWarning() << "Exception caught during fetching RAW from thumbnail...";
    }

    return false;
}

#ifdef WITH_LIBRAW

// here are some hints from earlier days...
//// (-w) Use camera white balance, if possible (otherwise, fallback to auto_wb)
// iProcessor.imgdata.params.use_camera_wb = 1;
//// (-a) Use automatic white balance obtained after averaging over the entire image
// iProcessor.imgdata.params.use_auto_wb = 1;
//// (-q 3) Adaptive homogeneity-directed de-mosaicing algorithm (AHD)
// iProcessor.imgdata.params.user_qual = 3;
// iProcessor.imgdata.params.output_tiff = 1;
////iProcessor.imgdata.params.four_color_rgb = 1;
////iProcessor.imgdata.params.output_color = 1; //sRGB  (0...raw)
//// RAW data filtration mode during data unpacking and post-processing
// iProcessor.imgdata.params.filtering_mode = LIBRAW_FILTERING_AUTOMATIC;

QImage DkRawLoader::loadPreviewRaw(LibRaw &iProcessor) const
{
    int tW = iProcessor.imgdata.thumbnail.twidth;

    if (DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_always
        || (DkSettingsManager::param().resources().loadRawThumb == DkSettings::raw_thumb_if_large && tW >= 1920)) {
        // crashes here if image is broken
        int err = iProcessor.unpack_thumb();
        char *tPtr = iProcessor.imgdata.thumbnail.thumb;

        if (!err && tPtr) {
            QImage img;
            img.loadFromData((const uchar *)tPtr, iProcessor.imgdata.thumbnail.tlength);

            // we're good to go
            if (!img.isNull()) {
                qDebug() << "[RAW] I loaded the RAW's thumbnail";
                return img;
            } else
                qDebug() << "RAW could not load the thumb";
        } else
            qDebug() << "error unpacking the thumb...";
    }

    // default: return nothing
    return QImage();
}

bool DkRawLoader::openBuffer(const QSharedPointer<QByteArray> &ba, LibRaw &iProcessor) const
{
    int error = LIBRAW_DATA_ERROR;

    QFileInfo fi(mFilePath);

    // use iprocessor from libraw to read the data
    // OK - so LibRaw 0.17 cannot identify iiq files in the buffer - so we load them from the file
    if (fi.suffix().contains("iiq", Qt::CaseInsensitive) || !ba || ba->isEmpty()) {
        error = iProcessor.open_file(mFilePath.toStdString().c_str());
    } else {
        // the buffer check is because:
        // libraw has an error when loading buffers if the first 4 bytes encode as 'RIFF'
        // and no data follows at all
        if (ba->isEmpty() || ba->size() < 100)
            return false;

        error = iProcessor.open_buffer((void *)ba->constData(), ba->size());
    }

    return (error == LIBRAW_SUCCESS);
}

void DkRawLoader::detectSpecialCamera(const LibRaw &iProcessor)
{
    if (QString(iProcessor.imgdata.idata.model) == "IQ260 Achromatic")
        mIsChromatic = false;

    if (QString(iProcessor.imgdata.idata.model).contains("IQ260"))
        mCamType = camera_iiq;
    else if (QString(iProcessor.imgdata.idata.make).compare("Canon", Qt::CaseInsensitive))
        mCamType = camera_canon;

    // add your camera flag (for hacks) here
}

cv::Mat DkRawLoader::demosaic(LibRaw &iProcessor) const
{
    cv::Mat rawMat = cv::Mat(iProcessor.imgdata.sizes.height, iProcessor.imgdata.sizes.width, CV_16UC1);
    double dynamicRange = (double)(iProcessor.imgdata.color.maximum - iProcessor.imgdata.color.black);

    // normalize all image values
    for (int rIdx = 0; rIdx < rawMat.rows; rIdx++) {
        unsigned short *ptrRaw = rawMat.ptr<unsigned short>(rIdx);

        for (int cIdx = 0; cIdx < rawMat.cols; cIdx++) {
            int colIdx = iProcessor.COLOR(rIdx, cIdx);
            double val = (double)(iProcessor.imgdata.image[(rawMat.cols * rIdx) + cIdx][colIdx]);

            // normalize the value w.r.t the black point defined
            val = (val - iProcessor.imgdata.color.black) / dynamicRange;
            ptrRaw[cIdx] = clip<unsigned short>(val * USHRT_MAX); // for conversion to 16U
        }
    }

    // no demosaicing
    if (mIsChromatic) {
        unsigned long type = (unsigned long)iProcessor.imgdata.idata.filters;
        type = type & 255;

        cv::Mat rgbImg;

        // define bayer pattern
        if (type == 180) {
            cvtColor(rawMat, rgbImg, CV_BayerBG2RGB); // bitmask  10 11 01 00  -> 3(G) 2(B) 1(G) 0(R) ->	RG RG RG
                                                      //													GB GB GB
        } else if (type == 30) {
            cvtColor(rawMat, rgbImg, CV_BayerRG2RGB); // bitmask  00 01 11 10	-> 0 1 3 2
        } else if (type == 225) {
            cvtColor(rawMat, rgbImg, CV_BayerGB2RGB); // bitmask  11 10 00 01
        } else if (type == 75) {
            cvtColor(rawMat, rgbImg, CV_BayerGR2RGB); // bitmask  01 00 10 11
        } else {
            qWarning() << "Wrong Bayer Pattern (not BG, RG, GB, GR)\n";
            return cv::Mat();
        }

        rawMat = rgbImg;
    }

    // 16U (1 or 3 channeled) Mat
    return rawMat;
}

cv::Mat DkRawLoader::prepareImg(const LibRaw &iProcessor) const
{
    cv::Mat rawMat = cv::Mat(iProcessor.imgdata.sizes.height, iProcessor.imgdata.sizes.width, CV_16UC3, cv::Scalar(0));
    double dynamicRange = (double)(iProcessor.imgdata.color.maximum - iProcessor.imgdata.color.black);

    // normalization function
    auto normalize = [&](double val) {
        val = (val - iProcessor.imgdata.color.black) / dynamicRange;
        return clip<unsigned short>(val * USHRT_MAX);
    };

    for (int rIdx = 0; rIdx < rawMat.rows; rIdx++) {
        unsigned short *ptrI = rawMat.ptr<unsigned short>(rIdx);

        for (int cIdx = 0; cIdx < rawMat.cols; cIdx++) {
            *ptrI = normalize(iProcessor.imgdata.image[rawMat.cols * rIdx + cIdx][0]);
            ptrI++;
            *ptrI = normalize(iProcessor.imgdata.image[rawMat.cols * rIdx + cIdx][1]);
            ptrI++;
            *ptrI = normalize(iProcessor.imgdata.image[rawMat.cols * rIdx + cIdx][2]);
            ptrI++;
        }
    }

    return rawMat;
}

cv::Mat DkRawLoader::whiteMultipliers(const LibRaw &iProcessor) const
{
    // get camera white balance multipliers
    cv::Mat wm(1, 4, CV_32FC1);

    float *wmp = wm.ptr<float>();

    for (int idx = 0; idx < wm.cols; idx++)
        wmp[idx] = iProcessor.imgdata.color.cam_mul[idx];

    if (wmp[3] == 0)
        wmp[3] = wmp[1]; // take green (usually its RGBG)

    // normalize white balance multipliers
    float w = (float)cv::sum(wm)[0] / 4.0f;
    float maxW = 1.0f;

    // clipping according the camera model
    // if w > 2.0 maxW is 256, otherwise 512
    // tested empirically
    // check if it can be defined by some metadata settings?
    if (w > 2.0f)
        maxW = 255.0f;
    if (w > 2.0f && mCamType == camera_canon)
        maxW = 511.0f; // some cameras would even need ~800 - why?

    // normalize white point
    wm /= maxW;

    // 1 x 4 32FC1 white balance vector
    return wm;
}

cv::Mat DkRawLoader::gammaTable(const LibRaw &iProcessor) const
{
    // OK this is an instance of reverse engineering:
    // we found out that the values of (at least) the PhaseOne's achromatic back have to be doubled
    // our images are no close to what their software (Capture One does) - only the gamma correction
    // seems to be slightly different... -> now we can load compressed IIQs that are not supported by PS : )
    double cameraHackMlp = (QString(iProcessor.imgdata.idata.model) == "IQ260 Achromatic") ? 2.0 : 1.0;

    // read gamma value and create gamma table
    double gamma = (double)iProcessor.imgdata.params.gamm[0];

    cv::Mat gmt(1, USHRT_MAX, CV_16UC1);
    unsigned short *gmtp = gmt.ptr<unsigned short>();

    for (int idx = 0; idx < gmt.cols; idx++) {
        gmtp[idx] = clip<unsigned short>(qRound((1.099 * std::pow((double)idx / USHRT_MAX, gamma) - 0.099) * 255 * cameraHackMlp));
    }

    // a 1 x 65535 U16 gamma table
    return gmt;
}

void DkRawLoader::whiteBalance(const LibRaw &iProcessor, cv::Mat &img) const
{
    // white balance must not be empty at this point
    cv::Mat wb = whiteMultipliers(iProcessor);
    const float *wbp = wb.ptr<float>();
    assert(wb.cols == 4);

    for (int rIdx = 0; rIdx < img.rows; rIdx++) {
        unsigned short *ptr = img.ptr<unsigned short>(rIdx);

        for (int cIdx = 0; cIdx < img.cols; cIdx++) {
            // apply white balance correction
            unsigned short r = clip<unsigned short>(*ptr * wbp[0]);
            unsigned short g = clip<unsigned short>(*(ptr + 1) * wbp[1]);
            unsigned short b = clip<unsigned short>(*(ptr + 2) * wbp[2]);

            // apply color correction
            int cr =
                qRound(iProcessor.imgdata.color.rgb_cam[0][0] * r + iProcessor.imgdata.color.rgb_cam[0][1] * g + iProcessor.imgdata.color.rgb_cam[0][2] * b);
            int cg =
                qRound(iProcessor.imgdata.color.rgb_cam[1][0] * r + iProcessor.imgdata.color.rgb_cam[1][1] * g + iProcessor.imgdata.color.rgb_cam[1][2] * b);
            int cb =
                qRound(iProcessor.imgdata.color.rgb_cam[2][0] * r + iProcessor.imgdata.color.rgb_cam[2][1] * g + iProcessor.imgdata.color.rgb_cam[2][2] * b);

            // clip & save color corrected values
            *ptr = clip<unsigned short>(cr);
            ptr++;
            *ptr = clip<unsigned short>(cg);
            ptr++;
            *ptr = clip<unsigned short>(cb);
            ptr++;
        }
    }
}

void DkRawLoader::gammaCorrection(const LibRaw &iProcessor, cv::Mat &img) const
{
    // white balance must not be empty at this point
    cv::Mat gt = gammaTable(iProcessor);
    const unsigned short *gammaLookup = gt.ptr<unsigned short>();
    assert(gt.cols == USHRT_MAX);

    for (int rIdx = 0; rIdx < img.rows; rIdx++) {
        unsigned short *ptr = img.ptr<unsigned short>(rIdx);

        for (int cIdx = 0; cIdx < img.cols * img.channels(); cIdx++) {
            // values close to 0 are treated linear
            if (ptr[cIdx] <= 5) // 0.018 * 255
                ptr[cIdx] = (unsigned short)qRound(ptr[cIdx] * (double)iProcessor.imgdata.params.gamm[1] / 255.0);
            else
                ptr[cIdx] = gammaLookup[ptr[cIdx]];
        }
    }
}

void DkRawLoader::reduceColorNoise(const LibRaw &iProcessor, cv::Mat &img) const
{
    // filter color noise with a median filter
    float isoSpeed = iProcessor.imgdata.other.iso_speed;

    if (isoSpeed > 0) {
        DkTimer dt;

        int winSize;
        if (isoSpeed > 6400)
            winSize = 13;
        else if (isoSpeed >= 3200)
            winSize = 11;
        else if (isoSpeed >= 2500)
            winSize = 9;
        else if (isoSpeed >= 400)
            winSize = 7;
        else
            winSize = 5;

        DkTimer dMed;

        // revert back to 8-bit image
        img.convertTo(img, CV_8U);

        cv::cvtColor(img, img, CV_RGB2YCrCb);

        std::vector<cv::Mat> imgCh;
        cv::split(img, imgCh);
        assert(imgCh.size() == 3);

        cv::medianBlur(imgCh[1], imgCh[1], winSize);
        cv::medianBlur(imgCh[2], imgCh[2], winSize);

        cv::merge(imgCh, img);
        cv::cvtColor(img, img, CV_YCrCb2RGB);
        qDebug() << "median blur takes:" << dt;
    }
}

QImage DkRawLoader::raw2Img(const LibRaw &iProcessor, cv::Mat &img) const
{
    // check the pixel aspect ratio of the raw image
    if (iProcessor.imgdata.sizes.pixel_aspect != 1.0f)
        cv::resize(img, img, cv::Size(), (double)iProcessor.imgdata.sizes.pixel_aspect, 1.0f);

    // revert back to 8-bit image
    img.convertTo(img, CV_8U);

    // TODO: for now - fix this!
    if (img.channels() == 1)
        cv::cvtColor(img, img, CV_GRAY2RGB);

    return DkImage::mat2QImage(img);
}

#endif

// -------------------------------------------------------------------- DkTgaLoader
namespace tga
{
DkTgaLoader::DkTgaLoader(QSharedPointer<QByteArray> ba)
{
    mBa = ba;
}

QImage DkTgaLoader::image() const
{
    return mImg;
}

bool DkTgaLoader::load()
{
    if (!mBa || mBa->isEmpty())
        return false;

    return load(mBa);
}

bool DkTgaLoader::load(QSharedPointer<QByteArray> ba)
{
    // this code is from: http://www.paulbourke.net/dataformats/tga/
    // thanks!
    Header header;

    const char *dataC = ba->data();

    /* Display the header fields */
    header.idlength = *dataC;
    dataC++;
    header.colourmaptype = *dataC;
    dataC++;
    header.datatypecode = *dataC;
    dataC++;

    const short *dataS = (const short *)dataC;

    header.colourmaporigin = *dataS;
    dataS++;
    header.colourmaplength = *dataS;
    dataS++;
    dataC = (const char *)dataS;
    header.colourmapdepth = *dataC;
    dataC++;
    dataS = (const short *)dataC;
    header.x_origin = *dataS;
    dataS++;
    header.y_origin = *dataS;
    dataS++;
    header.width = *dataS;
    dataS++;
    header.height = *dataS;
    dataS++;
    dataC = (const char *)dataS;
    header.bitsperpixel = *dataC;
    dataC++;
    header.imagedescriptor = *dataC;
    dataC++;

#ifdef _DEBUG
    qDebug() << "TGA Header ------------------------------";
    qDebug() << "ID length:         " << (int)header.idlength;
    qDebug() << "Colourmap type:    " << (int)header.colourmaptype;
    qDebug() << "Image type:        " << (int)header.datatypecode;
    qDebug() << "Colour map offset: " << header.colourmaporigin;
    qDebug() << "Colour map length: " << header.colourmaplength;
    qDebug() << "Colour map depth:  " << (int)header.colourmapdepth;
    qDebug() << "X origin:          " << header.x_origin;
    qDebug() << "Y origin:          " << header.y_origin;
    qDebug() << "Width:             " << header.width;
    qDebug() << "Height:            " << header.height;
    qDebug() << "Bits per pixel:    " << (int)header.bitsperpixel;
    qDebug() << "Descriptor:        " << (int)header.imagedescriptor;
#endif

    /* What can we handle */
    if (header.datatypecode != 2 && header.datatypecode != 10) {
        qWarning() << "[TGA] Can only handle image type 2 and 10";
        return false;
    }

    if (header.bitsperpixel != 16 && header.bitsperpixel != 24 && header.bitsperpixel != 32) {
        qWarning() << "[TGA] Can only handle pixel depths of 16, 24, and 32";
        return false;
    }

    if (header.colourmaptype != 0 && header.colourmaptype != 1) {
        qWarning() << "[TGA] Can only handle colour map types of 0 and 1";
        return false;
    }

    Pixel *pixels = new Pixel[header.width * header.height * sizeof(Pixel)];

    if (!pixels) {
        qWarning() << "[TGA] could not allocate" << header.width * header.height * sizeof(Pixel) / 1024 << "KB";
        return false;
    }

    ///* Skip over unnecessary stuff */
    int skipover = header.idlength;
    skipover += header.colourmaptype * header.colourmaplength;
    dataC += skipover;

    /* Read the image */
    int bytes2read = header.bitsperpixel / 8; // save?
    unsigned char p[5];

    for (int n = 0; n < header.width * header.height;) {
        if (header.datatypecode == 2) { /* Uncompressed */

            // TODO: out-of-bounds not checked here...
            for (int bi = 0; bi < bytes2read; bi++, dataC++)
                p[bi] = *dataC;

            mergeBytes(&(pixels[n]), p, bytes2read);
            n++;
        } else if (header.datatypecode == 10) { /* Compressed */

            for (int bi = 0; bi < bytes2read + 1; bi++, dataC++)
                p[bi] = *dataC;

            int j = p[0] & 0x7f;
            mergeBytes(&(pixels[n]), &(p[1]), bytes2read);
            n++;
            if (p[0] & 0x80) { /* RLE chunk */
                for (int i = 0; i < j; i++) {
                    mergeBytes(&(pixels[n]), &(p[1]), bytes2read);
                    n++;
                }
            } else { /* Normal chunk */
                for (int i = 0; i < j; i++) {
                    for (int bi = 0; bi < bytes2read; bi++, dataC++)
                        p[bi] = *dataC;

                    mergeBytes(&(pixels[n]), p, bytes2read);
                    n++;
                }
            }
        }
    }

    mImg = QImage((uchar *)pixels, header.width, header.height, QImage::Format_ARGB32);
    mImg = mImg.copy();

    // I somehow expected the 5th bit to be 0x10 -> but Paul seems to have a 0th bit : )
    if (!(header.imagedescriptor & 0x20))
        mImg = mImg.mirrored();

    delete[] pixels;

    return true;
}

void DkTgaLoader::mergeBytes(Pixel *pixel, unsigned char *p, int bytes) const
{
    if (bytes == 4) {
        pixel->r = p[0];
        pixel->g = p[1];
        pixel->b = p[2];
        pixel->a = p[3];
    } else if (bytes == 3) {
        pixel->r = p[0];
        pixel->g = p[1];
        pixel->b = p[2];
        pixel->a = 255;
    } else if (bytes == 2) {
        pixel->r = (p[0] & 0x1f) << 3;
        pixel->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
        pixel->b = (p[1] & 0x7c) << 1;
        pixel->a = 255; // (p[1] & 0x80);
    }
}
}

}
