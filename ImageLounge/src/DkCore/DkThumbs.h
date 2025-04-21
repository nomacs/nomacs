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

#include <queue>
#pragma warning(push, 0) // no warnings from includes - begin
#include <QFutureWatcher>
#include <QImage>
#include <QSharedPointer>
#pragma warning(pop) // no warnings from includes - end
#include "DkMetaData.h"
#include <QThread>
#include <optional>

#pragma warning(disable : 4251) // TODO: remove

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

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
    DkThumbsThreadPool(const DkThumbsThreadPool &);

    QThreadPool *mPool;
};

struct LoadThumbnailResult {
    QImage thumb{};
    QString filePath{};
    std::unique_ptr<DkMetaDataT> metaData{};
    bool fromExif{};
    bool transformed{};
};

enum class LoadThumbnailOption {
    // Try to load EXIF thumbnail first, and fall back to full image if not exist.
    none,

    // Only load EXIF thumbnail.
    force_exif,

    // Only load full image.
    force_full,
};

std::optional<LoadThumbnailResult> loadThumbnail(const QString &filePath, LoadThumbnailOption opt);

struct ThumbnailFromMetadata {
    QImage thumb{};
    bool transformed{};
};

std::optional<ThumbnailFromMetadata> loadThumbnailFromMetadata(const DkMetaDataT &metaData);

class DkThumbLoader : public QObject
{
    Q_OBJECT

    struct LoadThumbnailResultLocal {
        QImage thumb{};
        QString filePath{};
        bool valid{};
        bool fromExif{};
    };
    std::vector<QFutureWatcher<LoadThumbnailResultLocal>> mWatchers{};
    std::vector<QFutureWatcher<LoadThumbnailResultLocal> *> mIdleWatchers{};
    std::queue<QString> mQueue{};
    std::queue<LoadThumbnailResultLocal> mFullImageQueue{};
    QHash<QString, int> mCounts{};

public:
    DkThumbLoader();
    void requestThumbnail(const QString &filePath);
    void cancelThumbnailRequest(const QString &filePath);

    // When we have full image loaded in the viewport,
    // create a side effect to update the thumbnail.
    void dispatchFullImage(const QString &filePath, const QImage &img);

signals:
    void thumbnailLoaded(const QString &filePath, const QImage &thumb, bool fromExif);
    void thumbnailLoadFailed(const QString &filePath);
    void thumbnailRequested(const QString &filePath, LoadThumbnailOption opt = LoadThumbnailOption::force_exif);

private:
    void onThumbnailLoadFinished();
    static LoadThumbnailResultLocal loadThumbnailLocal(const QString &filePath);
    static LoadThumbnailResultLocal scaleFullThumbnail(const QString &filePath, const QImage &img);
    void handleFinishedWatcher(QFutureWatcher<LoadThumbnailResultLocal> *w);
};
}
