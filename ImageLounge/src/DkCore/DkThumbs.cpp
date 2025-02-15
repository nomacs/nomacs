/*******************************************************************************************************
 DkThumbs.cpp
 Created on:	19.04.2013

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

#include "DkThumbs.h"
#include "DkBasicLoader.h"
#include "DkImageStorage.h"
#include "DkMetaData.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QBuffer>
#include <QFileInfo>
#include <QImageReader>
#include <QMutex>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>
#include <QtConcurrentRun>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

/**
 * Default constructor.
 * @param file the corresponding file
 * @param img the thumbnail image
 **/
DkThumbNail::DkThumbNail(const QString &filePath, const QImage &img)
{
    mImg = DkImage::createThumb(img);
    mFile = filePath;
    mImgExists = true;
}

DkThumbNail::~DkThumbNail()
{
}

/**
 * Loads the thumbnail.
 **/
void DkThumbNail::compute()
{
    std::optional<LoadThumbnailResult> res = loadThumbnail(mFile, LoadThumbnailOption::none);
    if (!res) {
        mImg = {};
        return;
    }
    mImg = DkImage::createThumb(res->thumb);
}

std::optional<QImage> loadThumbnailFromMetadata(const DkMetaDataT &metaData)
{
    QImage thumb = metaData.getThumbnail();
    if (thumb.isNull()) {
        return std::nullopt;
    }

    DkThumbNail::removeBlackBorder(thumb);

    int orientation = metaData.getOrientationDegree();

    bool shouldRotate = metaData.isAVIF() || metaData.isHEIF() || metaData.isJXL() || metaData.isJpg() || metaData.isRaw();
    if (shouldRotate && orientation != -1 && orientation != 0) {
        // do not rotate together with full image but rotate Exif thumb only
        QTransform rotationMatrix;
        rotationMatrix.rotate(orientation);
        thumb = thumb.transformed(rotationMatrix);
    }

    return thumb;
}

std::optional<QImage> loadThumbnailFromFullImage(const QString &filePath, const DkMetaDataT &metaData, QSharedPointer<QByteArray> baZip)
{
    DkBasicLoader loader;
    QImage thumb;

    if (baZip && !baZip->isEmpty()) {
        if (loader.loadGeneral(filePath, baZip, true, true)) {
            thumb = loader.image();
        } else {
            return std::nullopt;
        }
    } else {
        if (loader.loadGeneral(filePath, true, true)) {
            thumb = loader.image();
        } else {
            return std::nullopt;
        }
    }

    int orientation = metaData.getOrientationDegree();

    if (orientation != -1 && orientation != 0 && (metaData.isJpg() || metaData.isRaw())) {
        QTransform rotationMatrix;
        rotationMatrix.rotate(orientation);
        thumb = thumb.transformed(rotationMatrix);
    }

    return thumb;
}

std::optional<LoadThumbnailResult> loadThumbnail(const QString &filePath, LoadThumbnailOption opt)
{
    auto metaData = std::make_unique<DkMetaDataT>();

    QSharedPointer<QByteArray> baZip = QSharedPointer<QByteArray>();
#ifdef WITH_QUAZIP
    if (QFileInfo(filePath).dir().path().contains(DkZipContainer::zipMarker()))
        baZip = DkZipContainer::extractImage(DkZipContainer::decodeZipFile(filePath), DkZipContainer::decodeImageFile(filePath));
#endif
    try {
        // [DIEM] READ  build crashed here 09.06.2016
        if (baZip && !baZip->isEmpty()) {
            metaData->readMetaData(filePath, baZip);
        } else {
            metaData->readMetaData(filePath);
        }
    } catch (...) {
        // do nothing - we'll load the full file
    }

    QFileInfo fInfo(filePath);
    QString lFilePath = fInfo.isSymLink() ? fInfo.symLinkTarget() : filePath;
    fInfo = QFileInfo(lFilePath);

    std::optional<QImage> exifThumb{};
    if (opt != LoadThumbnailOption::force_full) {
        exifThumb = loadThumbnailFromMetadata(*metaData);
    }

    std::optional<QImage> fullThumb{};
    if ((opt != LoadThumbnailOption::force_exif || fInfo.size() < 1e5) && !exifThumb) { // braces
        fullThumb = loadThumbnailFromFullImage(lFilePath, *metaData, baZip);
    }

    if (!fullThumb && !exifThumb) {
        return std::nullopt;
    }

    return LoadThumbnailResult{exifThumb ? exifThumb.value() : fullThumb.value(), !exifThumb->isNull(), std::move(metaData), lFilePath};
}

/**
 * Loads the thumbnail from the metadata.
 * If no thumbnail is embedded, the whole image
 * is loaded and downsampled in a fast manner.
 * @param file the file to be loaded
 * @param forceLoad the loading flag (e.g. exiv only)
 * @return QImage the loaded image. Null if no image
 * could be loaded at all.
 * @reentrant all parameters must be copies or thread-safe shared pointers,
 *            no class members are allowed
 **/
QImage DkThumbNail::computeIntern(const QString &filePath, int forceLoad)
{
    LoadThumbnailOption opt = LoadThumbnailOption::none;
    if (forceLoad == force_save_thumb || forceLoad == force_full_thumb) {
        opt = LoadThumbnailOption::force_full;
    }
    if (forceLoad == force_exif_thumb) {
        opt = LoadThumbnailOption::force_exif;
    }
    std::optional<LoadThumbnailResult> res = loadThumbnail(filePath, opt);

    if (!res) {
        return {};
    }

    // save the thumbnail if the caller either forces it, or the save thumb is requested and the image did not have any before
    if (forceLoad == force_save_thumb || (forceLoad == save_thumb && !res->fromExif)) {
        try {
            int orientation = res->metaData->getOrientationDegree();
            QImage sThumb = res->thumb;
            if (orientation != -1 && orientation != 0) {
                QTransform rotationMatrix;
                rotationMatrix.rotate(-(double)orientation);
                sThumb = sThumb.transformed(rotationMatrix);
            }

            res->metaData->updateImageMetaData(sThumb);
            res->metaData->saveMetaData(res->filePath);

            qDebug() << "[thumb] saved to exif data";
        } catch (...) {
            qWarning() << "Sorry, I could not save the metadata";
        }
    }
    // if (!thumb.isNull())
    // 	qInfoClean() << "[thumb] " << fInfo.fileName() << " (" << thumb.width() << " x " << thumb.height() << ") loaded in " << dt << ((exifThumb) ? " from
    // EXIV" : " from File");

    return DkImage::createThumb(res->thumb);
}

/**
 * Removes potential black borders.
 * These borders can be found e.g. in Nikon One images (16:9 vs 4:3)
 * @param img the image whose borders are removed.
 **/
void DkThumbNail::removeBlackBorder(QImage &img)
{
    int rIdx = 0;
    bool nonblack = false;

    for (; rIdx < qRound(img.height() * 0.1); rIdx++) {
        const QRgb *pixel = (QRgb *)(img.constScanLine(rIdx));

        for (int cIdx = 0; cIdx < img.width(); cIdx++, pixel++) {
            // > 50 due to jpeg (normally we would want it to be != 0)
            if (qRed(*pixel) > 50 || qBlue(*pixel) > 50 || qGreen(*pixel) > 50) {
                nonblack = true;
                break;
            }
        }

        if (nonblack)
            break;
    }

    // non black border?
    if (rIdx == -1 || rIdx > 15)
        return;

    int rIdxB = img.height() - 1;
    nonblack = false;

    for (; rIdxB >= qRound(img.height() * 0.9f); rIdxB--) {
        const QRgb *pixel = (QRgb *)(img.constScanLine(rIdxB));

        for (int cIdx = 0; cIdx < img.width(); cIdx++, pixel++) {
            if (qRed(*pixel) > 50 || qBlue(*pixel) > 50 || qGreen(*pixel) > 50) {
                nonblack = true;
                break;
            }
        }

        if (nonblack) {
            rIdxB--;
            break;
        }
    }

    // remove black borders
    if (rIdx < rIdxB)
        img = img.copy(0, rIdx, img.width(), rIdxB - rIdx);
}

/**
 * Creates a thumbnail from the image provided and stores it internally.
 * @param img the image to be converted to a thumbnail
 **/
void DkThumbNail::setImage(const QImage img)
{
    mImg = DkImage::createThumb(img);
}

/**
 * This class provides threaded access to image thumbnails.
 * @param file the thumbnail's file
 * @param img optional: a thumb image.
 **/
DkThumbNailT::DkThumbNailT(const QString &filePath, const QImage &img)
    : DkThumbNail(filePath, img)
{
    mFetching = false;
    mForceLoad = do_not_force;
}

DkThumbNailT::~DkThumbNailT()
{
    mThumbWatcher.blockSignals(true);
    mThumbWatcher.cancel();
}

bool DkThumbNailT::fetchThumb(int forceLoad /* = false */)
{
    if (forceLoad == force_full_thumb || forceLoad == force_save_thumb || forceLoad == save_thumb)
        mImg = QImage();

    if (!mImg.isNull() || !mImgExists || mFetching)
        return false;

    // check if we can load the file
    // though if it might seem over engineered: it is much faster cascading it here
    if (!DkUtils::hasValidSuffix(getFilePath()) && !DkUtils::isValid(QFileInfo(getFilePath())))
        return false;

    // we have to do our own bool here
    // watcher.isRunning() returns false if the thread is waiting in the pool
    mFetching = true;
    mForceLoad = forceLoad;

    connect(&mThumbWatcher, &QFutureWatcherBase::finished, this, &DkThumbNailT::thumbLoaded, Qt::UniqueConnection);

    // Load thumbnails on their dedicated thread pool
    mThumbWatcher.setFuture(QtConcurrent::run(DkThumbsThreadPool::pool(), DkThumbNail::computeIntern, mFile, forceLoad));

    return true;
}

void DkThumbNailT::thumbLoaded()
{
    QFuture<QImage> future = mThumbWatcher.future();

    mImg = future.result();

    if (mImg.isNull() && mForceLoad != force_exif_thumb)
        mImgExists = false;

    mFetching = false;
    emit thumbLoadedSignal(!mImg.isNull());
}

// DkThumbsThreadPool --------------------------------------------------------------------
DkThumbsThreadPool::DkThumbsThreadPool()
{
    mPool = new QThreadPool();
    mPool->setMaxThreadCount(qMax(mPool->maxThreadCount() - 2, 1));
}

DkThumbsThreadPool &DkThumbsThreadPool::instance()
{
    static DkThumbsThreadPool inst;
    return inst;
}

QThreadPool *DkThumbsThreadPool::pool()
{
    return instance().mPool;
}

void DkThumbsThreadPool::clear()
{
    pool()->clear();
}
}
