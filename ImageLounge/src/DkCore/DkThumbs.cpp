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
#include "qpainter.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QStringBuilder>
#include <QStringList>
#include <QTimer>
#include <QtConcurrentRun>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

void removeBlackBorder(QImage &img);

std::optional<ThumbnailFromMetadata> loadThumbnailFromMetadata(const DkMetaDataT &metaData)
{
    QImage thumb = metaData.getThumbnail();
    if (thumb.isNull()) {
        return std::nullopt;
    }

    removeBlackBorder(thumb);

    int orientation = metaData.getOrientationDegrees();

    const bool disableTransform = DkSettingsManager::param().metaData().ignoreExifOrientation; // match loadGeneral()
    const bool shouldTransform = !disableTransform && orientation != DkMetaDataT::or_invalid && orientation != DkMetaDataT::or_not_set;
    if (shouldTransform) {
        if (orientation != 0) {
            QTransform rotationMatrix;
            rotationMatrix.rotate(orientation);
            thumb = thumb.transformed(rotationMatrix);
        }
        if (metaData.isOrientationMirrored()) {
            thumb = thumb.mirrored(true, false);
        }
    }

    return ThumbnailFromMetadata{thumb, shouldTransform};
}

std::optional<QImage> loadThumbnailFromFullImage(const QString &filePath, QSharedPointer<QByteArray> baZip)
{
    DkBasicLoader loader;
    if (loader.loadGeneral(filePath, baZip, true, true)) {
        return loader.image();
    } else {
        return std::nullopt;
    }
}

std::optional<LoadThumbnailResult> loadThumbnail(const QString &filePath, LoadThumbnailOption opt)
{
    DkTimer dt{};

    auto metaData = std::make_unique<DkMetaDataT>();
    QSharedPointer<QByteArray> ba{};
#ifdef WITH_QUAZIP
    if (QFileInfo(filePath).dir().path().contains(DkZipContainer::zipMarker()))
        ba = DkZipContainer::extractImage(DkZipContainer::decodeZipFile(filePath), DkZipContainer::decodeImageFile(filePath));
#endif

    // read the thumbnail from the exif data
    try {
        if (!ba || ba->isEmpty()) {
            metaData->readMetaData(filePath);
        } else {
            metaData->readMetaData(filePath, ba);
        }
    } catch (...) {
        // this should never happen since we handle exceptions in metaData
        qWarning() << "[Thumbnail] unexpected exception when reading exif thumbnail";
    }

    // FIXME: why do we need link resolution here?? won't links be followed by default??
    QFileInfo fileInfo(filePath);
    QString linkFilePath = fileInfo.isSymLink() ? fileInfo.symLinkTarget() : filePath;
    fileInfo = QFileInfo(linkFilePath);

    std::optional<ThumbnailFromMetadata> exifThumb{};
    if (opt != LoadThumbnailOption::force_full) {
        exifThumb = loadThumbnailFromMetadata(*metaData);
    }

    std::optional<QImage> fullThumb{};
    if (opt != LoadThumbnailOption::force_exif && !exifThumb) {
        fullThumb = loadThumbnailFromFullImage(linkFilePath, ba);
    }

    if (!fullThumb && !exifThumb) {
        return std::nullopt;
    }

    LoadThumbnailResult res = {
        exifThumb ? exifThumb.value().thumb : fullThumb.value(),
        linkFilePath,
        std::move(metaData),
        exifThumb.has_value(),
        exifThumb && exifThumb->transformed,
    };

    QString info = QString("[Thumbnail] %1 exif=%2 size=%3x%4 %8ms")
                       .arg(linkFilePath)
                       .arg(exifThumb ? "yes" : "no")
                       .arg(res.thumb.width())
                       .arg(res.thumb.height())
                       .arg(dt.elapsed());
    qInfo().noquote() << info;
    return res;
}

void removeBlackBorder(QImage &img)
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

DkThumbLoaderWorker::DkThumbLoaderWorker()
{
    connect(this, &DkThumbLoaderWorker::requestFullThumbnail, this, &DkThumbLoaderWorker::requestThumbnail, Qt::QueuedConnection);
}

void DkThumbLoaderWorker::requestThumbnail(const QString &filePath, LoadThumbnailOption opt)
{
    const std::optional<LoadThumbnailResult> res = loadThumbnail(filePath, opt);
    if (res) {
        emit thumbnailLoaded(filePath, res->thumb, res->fromExif);
        return;
    }

    if (opt == LoadThumbnailOption::force_exif) {
        // By default we can use force_exif to try loading those that have EXIF thumbnails first,
        // which should be fast.
        // If this failed, push the filePath to the back of the queue, via the QueuedConnection,
        // so we prioritize loading all the EXIF thumbnails.
        emit requestFullThumbnail(filePath, LoadThumbnailOption::force_full);
    } else {
        // We have tried loading the full thumbnail
        emit thumbnailLoadFailed(filePath);
    }
}

DkThumbLoader::DkThumbLoader()
{
    auto *worker = new DkThumbLoaderWorker;
    worker->moveToThread(&mWorkerThread);
    connect(&mWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &DkThumbLoader::thumbnailRequested, worker, &DkThumbLoaderWorker::requestThumbnail);
    connect(worker, &DkThumbLoaderWorker::thumbnailLoaded, this, &DkThumbLoader::thumbnailLoaded);
    connect(worker, &DkThumbLoaderWorker::thumbnailLoadFailed, this, &DkThumbLoader::thumbnailLoadFailed);
    mWorkerThread.start();
}

DkThumbLoader::~DkThumbLoader()
{
    mWorkerThread.quit();
    mWorkerThread.wait();
}

void DkThumbLoader::requestThumbnail(const QString &filePath)
{
    emit thumbnailRequested(filePath);
}
}
