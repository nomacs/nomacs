/*******************************************************************************************************
 DkThumbs.h
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

#pragma once

#include <QCache>
#include <QFutureWatcher>
#include <QImage>

#include <optional>
#include <queue>

#include "DkMetaData.h"

class QThreadPool;

namespace nmc
{

#define max_thumb_size 400

class DkThumbsThreadPool
{
public:
    static DkThumbsThreadPool &instance();

    static QThreadPool *pool();
    static void clear();

private:
    DkThumbsThreadPool();
    DkThumbsThreadPool(const DkThumbsThreadPool &) = delete; // NOLINT

    QThreadPool *mPool;
};

enum class LoadThumbnailOption {
    // Try to load EXIF thumbnail first, and fall back to full image if not exist.
    none,

    // Only load EXIF thumbnail.
    force_exif,

    // Return requested size if smaller than full image
    force_size,

    // Only load full image.
    force_full,
};

using ThumbnailId = size_t;

struct LoadThumbnailRequest {
    ThumbnailId id{};
    QString filePath{};
    LoadThumbnailOption option{};
    int size{};

    LoadThumbnailRequest() = default;

    explicit LoadThumbnailRequest(const QString &filePath_,
                                  LoadThumbnailOption option_ = {},
                                  int size_ = max_thumb_size);

    size_t sizeInBytes()
    {
        return sizeof(this) + filePath.size() * 2;
    }
};

struct LoadThumbnailResult {
    QImage thumb{};
    QString filePath{};
    std::unique_ptr<DkMetaDataT> metaData{};
    bool fromExif{};
    bool transformed{};
};

std::optional<LoadThumbnailResult> loadThumbnail(const LoadThumbnailRequest &request);

struct ThumbnailFromMetadata {
    QImage thumb{};
    bool transformed{};
};

std::optional<ThumbnailFromMetadata> loadThumbnailFromMetadata(const DkMetaDataT &metaData);

class DkThumbLoader : public QObject
{
    Q_OBJECT

    struct LoadThumbnailResultLocal {
        LoadThumbnailRequest request;
        QImage thumb{};
        bool valid{};
        bool fromExif{};
        size_t sizeInBytes()
        {
            return sizeof(this) + request.sizeInBytes() + request.sizeInBytes() + thumb.sizeInBytes();
        }
    };

    QCache<ThumbnailId, LoadThumbnailResultLocal> mThumbnailCache{100000000}; // 100 MB
    std::vector<QFutureWatcher<LoadThumbnailResultLocal>> mWatchers{};
    std::vector<QFutureWatcher<LoadThumbnailResultLocal> *> mIdleWatchers{};
    std::queue<LoadThumbnailRequest> mQueue{};
    std::queue<LoadThumbnailResultLocal> mFullImageQueue{};
    QHash<ThumbnailId, int> mCounts{};

public:
    DkThumbLoader();
    void requestThumbnail(const LoadThumbnailRequest &request);
    void cancelThumbnailRequest(const LoadThumbnailRequest &request);

    // When we have full image loaded in the viewport,
    // create a side effect to update the thumbnail.
    void dispatchFullImage(const LoadThumbnailRequest &request, const QImage &img);

signals:
    // TODO: signals should return request object or id
    void thumbnailLoaded(const QString &filePath, const QImage &thumb, bool fromExif);
    void thumbnailLoadFailed(const QString &filePath);

private:
    void onThumbnailLoadFinished();
    static LoadThumbnailResultLocal loadThumbnailLocal(const LoadThumbnailRequest &request);
    static LoadThumbnailResultLocal scaleFullThumbnail(const LoadThumbnailRequest &request, const QImage &img);
    void handleFinishedWatcher(QFutureWatcher<LoadThumbnailResultLocal> *w);
};
}
