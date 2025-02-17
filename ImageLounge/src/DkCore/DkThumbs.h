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

/**
 * This class holds thumbnails.
 **/
class DllCoreExport DkThumbNail
{
public:
    enum Status {
        loading = -2,
        exists_not = -1,
        not_loaded,
        loaded,
    };

    enum FetchMode {
        prefer_exif, // try exif first, then load full image, always rescale
        require_exif, // fail if exif is not present, never rescale, never load full image
        // never_exif, // unused; always load full image
        write_exif, // write a new thumbnail to file if there isn't one, load full image
        write_exif_always // load full image; overwrite existing thumbnails
    };

    /**
     * Default constructor.
     * @param filePath the corresponding file
     * @param img the thumbnail image
     **/
    DkThumbNail(const QString &filePath = QString(), const QImage &img = QImage());

    /**
     * Default destructor.
     * @return
     **/
    virtual ~DkThumbNail();

    friend bool operator==(const DkThumbNail &lt, const DkThumbNail &rt)
    {
        return lt.mFile == rt.mFile;
    };

    /**
     * Creates a thumbnail from the image provided and stores it internally.
     * @param img the image to be converted to a thumbnail
     **/
    virtual void setImage(const QImage &img);

    /**
     * Removes potential black borders.
     * These borders can be found e.g. in Nikon One images (16:9 vs 4:3)
     * @param img the image whose borders are removed.
     **/
    static void removeBlackBorder(QImage &img);

    /**
     * Returns the thumbnail.
     * @return QImage the thumbnail.
     **/
    QImage getImage() const
    {
        return mImg;
    };

    /**
     * Returns the file information.
     * @return the thumbnail file
     **/
    QString getFilePath() const
    {
        return mFile;
    };

    /**
     * Returns whether the thumbnail was loaded, or does not exist.
     * @return int a status (loaded | not loaded | exists not)
     **/
    int hasImage() const
    {
        if (!mImg.isNull())
            return loaded;
        else if (mImg.isNull() && mImgExists)
            return not_loaded;
        else
            return exists_not;
    };

    QString toolTip() const;

protected:
    /**
     * Loads the thumbnail from the metadata.
     * If no thumbnail is embedded, the whole image
     * is loaded and downsampled in a fast manner.
     * @param file the file to be loaded
     * @param ba the file buffer (can be empty)
     * @param mode the loading flag (e.g. exif only)
     * @param maxThumbSize the maximal thumbnail size to be loaded
     * @return QImage the loaded image, or null image
     * @reentrant all parameters must be copies or thread-safe shared pointers
     **/
    static QImage computeIntern(const QString &filePath, const int mode);

    QImage mImg;
    QString mFile;
    bool mImgExists;
};

class DllCoreExport DkThumbNailT : public QObject, public DkThumbNail
{
    Q_OBJECT

public:
    DkThumbNailT(const QString &mFile = QString(), const QImage &mImg = QImage());
    ~DkThumbNailT();

    bool fetchThumb(DkThumbNail::FetchMode mode = prefer_exif);

    /**
     * Returns whether the thumbnail was loaded, or does not exist.
     * @return int a status (loaded | not loaded | exists not | loading)
     **/
    int hasImage() const
    {
        if (mThumbWatcher.isRunning())
            return loading;
        else
            return DkThumbNail::hasImage();
    };

    void setImage(const QImage img)
    {
        DkThumbNail::setImage(img);
        emit thumbLoadedSignal(true);
    };

signals:
    void thumbLoadedSignal(bool loaded = true);

protected slots:
    void thumbLoaded();

protected:
    QFutureWatcher<QImage> mThumbWatcher;
    bool mFetching = false;
    int mFetchMode = prefer_exif;
};

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
    none,
    force_exif,
    force_full,
};

std::optional<LoadThumbnailResult> loadThumbnail(const QString &filePath, LoadThumbnailOption opt);

class DkThumbLoaderWorker : public QObject
{
    Q_OBJECT
public:
    DkThumbLoaderWorker();
    void requestThumbnail(const QString &filePath, LoadThumbnailOption opt);
signals:
    void thumbnailLoaded(const QString &filePath, const QImage &thumb);
    void thumbnailLoadFailed(const QString &filePath);
    void requestFullThumbnail(const QString &filePath, LoadThumbnailOption opt);
};

class DkThumbLoader : public QObject
{
    Q_OBJECT
    QThread mWorkerThread{};

public:
    DkThumbLoader();
    ~DkThumbLoader();
    void requestThumbnail(const QString &filePath);

signals:
    void thumbnailLoaded(const QString &filePath, const QImage &thumb);
    void thumbnailLoadFailed(const QString &filePath);
    void thumbnailRequested(const QString &filePath, LoadThumbnailOption opt = LoadThumbnailOption::force_exif);
};
}
