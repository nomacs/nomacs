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
#include "DkCachedThumb.h"
#include "DkFileInfo.h"
#include "DkMetaData.h"
#include "DkSettings.h"
#include "DkTimer.h"

#include <QtConcurrentRun>

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
    const bool shouldTransform = !disableTransform && orientation != DkMetaDataT::or_invalid
        && orientation != DkMetaDataT::or_not_set;
    if (shouldTransform) {
        if (orientation != 0) {
            QTransform rotationMatrix;
            rotationMatrix.rotate(orientation);
            thumb = thumb.transformed(rotationMatrix);
        }
        if (metaData.isOrientationMirrored()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
            thumb = thumb.flipped(Qt::Horizontal);
#else
            thumb = thumb.mirrored(true, false);
#endif
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

std::optional<LoadThumbnailResult> loadThumbnail(const LoadThumbnailRequest &request)
{
    DkTimer dt{};

    auto metaData = std::make_unique<DkMetaDataT>();
    QSharedPointer<QByteArray> ba{};
    DkFileInfo fileInfo(request.filePath);

    if (fileInfo.isSymLink() && !fileInfo.resolveSymLink()) {
        qWarning() << "[Thumbnail] broken link:" << request.filePath;
        return std::nullopt;
    }

    const QString thumbPath = fileInfo.path(); // resolved path (shortcuts/aliases)

    std::optional<ThumbnailFromMetadata> exifThumb{};
    std::optional<QImage> fullThumb{};
    std::unique_ptr<DkCachedThumb> cachedThumb{};

    if (request.option != LoadThumbnailOption::force_full
        && DkSettingsManager::param().resources().thumbDiskSpace > 0) {
        cachedThumb = std::make_unique<DkCachedThumb>(fileInfo, request.size, request.constraint);
        QImage thumb = cachedThumb->load();
        if (!thumb.isNull()) {
            fullThumb = thumb;
        }
    }

    if (!fullThumb) {
        if (fileInfo.isFromZip()) {
            std::unique_ptr<QIODevice> io = fileInfo.getIODevice();
            if (io)
                ba.reset(new QByteArray(io->readAll()));
        }

        // read the thumbnail from the exif data
        try {
            if (!ba || ba->isEmpty()) {
                metaData->readMetaData(thumbPath);
            } else {
                metaData->readMetaData(thumbPath, ba);
            }
        } catch (...) {
            // this should never happen since we handle exceptions in metaData
            qWarning() << "[Thumbnail] unexpected exception when reading exif thumbnail";
        }

        if (request.option != LoadThumbnailOption::force_full) {
            exifThumb = loadThumbnailFromMetadata(*metaData);
        }

        bool loadFull = request.option != LoadThumbnailOption::force_exif && !exifThumb;
        loadFull |= request.option == LoadThumbnailOption::force_size && exifThumb
            && qMax(exifThumb->thumb.height(), exifThumb->thumb.width()) < request.size;
        if (loadFull) {
            exifThumb = {};
            fullThumb = loadThumbnailFromFullImage(thumbPath, ba);
        }

        if (!fullThumb && !exifThumb) {
            return std::nullopt;
        }
    }

    QImage thumb = exifThumb ? exifThumb.value().thumb : fullThumb.value();

    if (!thumb.isNull() && cachedThumb) {
        if (dt.elapsed() >= 10) { // skip save if it loaded quickly
            cachedThumb->save(thumb);
        }
    }

    LoadThumbnailResult res = {
        thumb,
        thumbPath,
        std::move(metaData),
        exifThumb.has_value(),
        exifThumb && exifThumb->transformed,
    };

    QString info = QString("[Thumbnail] %1 exif=%2 req=%3 size=%4x%5 %6ms")
                       .arg(fileInfo.fileName())
                       .arg(exifThumb ? "yes" : "no")
                       .arg(request.size)
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
    const auto &res = DkSettingsManager::param().resources();
    int numThreads = qBound(1, res.thumbThreads, QThread::idealThreadCount() - 2);
    mPool = new QThreadPool();
    mPool->setMaxThreadCount(numThreads);
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

DkThumbLoader::DkThumbLoader()
{
    unsigned numThreads = DkThumbsThreadPool::pool()->maxThreadCount();

    const auto &res = DkSettingsManager::param().resources();
    auto maxBytes = qsizetype(res.thumbCacheMemory) * 1024 * 1024;

    mThumbnailCache.setMaxCost(maxBytes);
    mWatchers = decltype(mWatchers){numThreads};

    mIdleWatchers.reserve(numThreads);
    for (auto &ele : mWatchers) {
        mIdleWatchers.push_back(&ele);
        connect(&ele,
                &QFutureWatcher<LoadThumbnailResultLocal>::finished,
                this,
                &DkThumbLoader::onThumbnailLoadFinished);
    }
}

DkThumbLoader::LoadThumbnailResultLocal DkThumbLoader::loadThumbnailLocal(const LoadThumbnailRequest &request)
{
    const auto res = loadThumbnail(request);
    if (!res) {
        return {request, QImage(), false, false};
    }
    return {request, DkImage::createThumb(res->thumb, request.size, request.constraint), true, res->fromExif};
}

DkThumbLoader::LoadThumbnailResultLocal DkThumbLoader::scaleFullThumbnail(const LoadThumbnailRequest &request,
                                                                          const QImage &img)
{
    return {request, DkImage::createThumb(img, request.size, request.constraint), true, false};
}

void DkThumbLoader::requestThumbnail(const LoadThumbnailRequest &request)
{
    const LoadThumbnailResultLocal *cached = mThumbnailCache.object(request.id);
    if (cached) {
        if (!cached->valid) {
            emit thumbnailLoadFailed(cached->request.id, cached->request.filePath);
        } else {
            emit thumbnailLoaded(cached->request.id, cached->request.filePath, cached->thumb, cached->fromExif);
        }
        return;
    }

    if (mIdleWatchers.size() == 0) {
        // We may have multiple widgets requesting the same thumbnail. With cancellation, we
        // must ensure if only one cancels the others will not; and so we count the requests.
        auto it = mCounts.find(request.id);
        if (it == mCounts.end()) {
            mQueue.push(request);
            mCounts.insert(request.id, 1);
        } else {
            it.value() += 1;
            Q_ASSERT(it.value() > 0);
        }
        return;
    }

    auto *w = mIdleWatchers.back();
    mIdleWatchers.pop_back();
    w->setFuture(QtConcurrent::run(loadThumbnailLocal, request));
}

void DkThumbLoader::cancelThumbnailRequest(const LoadThumbnailRequest &request)
{
    auto it = mCounts.find(request.id);
    if (it == mCounts.end()) {
        return;
    }
    it.value() -= 1;
    Q_ASSERT(it.value() >= 0);
    if (it.value() == 0) {
        mCounts.remove(it.key());
    }
}

void DkThumbLoader::dispatchFullImage(const LoadThumbnailRequest &request, const QImage &img)
{
    if (mIdleWatchers.size() == 0) {
        // Full image takes priority, so we can skip the pending requests
        mCounts.remove(request.id);
        mFullImageQueue.push({request, img, true, false});
        return;
    }

    auto *w = mIdleWatchers.back();
    mIdleWatchers.pop_back();
    w->setFuture(QtConcurrent::run(scaleFullThumbnail, request, img));
}

void DkThumbLoader::onThumbnailLoadFinished()
{
    const auto w = dynamic_cast<QFutureWatcher<LoadThumbnailResultLocal> *>(sender());
    Q_ASSERT(w != nullptr);

    auto *res = new LoadThumbnailResultLocal{w->result()};
    const size_t resSize = sizeof(*res) + res->sizeInBytes();

    auto it = mCounts.find(res->request.id);
    if (it != mCounts.end()) {
        // We have finished the request, the count might be gone due to cancellations
        mCounts.remove(it.key());
    }

    handleFinishedWatcher(w);

    if (!res->valid) { // NOLINT(clang-analyzer-core.uninitialized.Branch) -- false positive
        emit thumbnailLoadFailed(res->request.id, res->request.filePath);
        mThumbnailCache.insert(res->request.id, res, resSize);
        return;
    }

    emit thumbnailLoaded(res->request.id, res->request.filePath, res->thumb, res->fromExif);

    // Add cache after finished using res because the cache takes ownership.
    mThumbnailCache.insert(res->request.id, res, resSize);
}

void DkThumbLoader::handleFinishedWatcher(QFutureWatcher<LoadThumbnailResultLocal> *w)
{
    if (mFullImageQueue.size() > 0) {
        const LoadThumbnailResultLocal &item = mFullImageQueue.front();
        w->setFuture(QtConcurrent::run(scaleFullThumbnail, item.request, item.thumb));
        mFullImageQueue.pop();
        return;
    }

    while (!mQueue.empty()) {
        // Remove next request from the queue; if it has no refcount, all requests were cancelled
        const LoadThumbnailRequest request = std::move(mQueue.front());
        mQueue.pop();
        auto it = mCounts.find(request.id);
        if (it != mCounts.end()) {
            // Do not drop refcount here, we need to keep count while the request is processed
            w->setFuture(QtConcurrent::run(loadThumbnailLocal, request));
            return;
        }
    }
    mIdleWatchers.push_back(w);
}

LoadThumbnailRequest::LoadThumbnailRequest(const QString &filePath_,
                                           LoadThumbnailOption option_,
                                           int size_,
                                           ScaleConstraint constraint_)
    : filePath(filePath_)
    , option(option_)
    , size(size_)
    , constraint(constraint_)
{
    QString key = filePath;
    key += QString::number(static_cast<int>(option));
    key += QString::number(static_cast<int>(constraint_));
    key += QString::number(size);

    id = qHash(key);
}
}
