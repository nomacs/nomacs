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
#include <QColor>
#include <QDir>
#include <QFutureWatcher>
#include <QImage>
#include <QSharedPointer>
#include <QThread>
#pragma warning(pop) // no warnings from includes - end

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
    enum {
        loading = -2,
        exists_not = -1,
        not_loaded,
        loaded,
    };

    /**
     * Default constructor.
     * @param file the corresponding file
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
     * Sets the thumbnail image.
     * @param img the thumbnail
     **/
    virtual void setImage(const QImage img);

    void removeBlackBorder(QImage &img);

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
     * @return QFileInfo the thumbnail file
     **/
    QString getFilePath() const
    {
        return mFile;
    };

    void compute(int forceLoad = do_not_force);

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

    void setMaxThumbSize(int maxSize)
    {
        mMaxThumbSize = maxSize;
    };

    int getMaxThumbSize() const
    {
        return mMaxThumbSize;
    };

    /**
     * Manipulates the file loaded status.
     * @param exists a status (loaded | not loaded | exists not)
     **/
    void setImgExists(bool exists)
    {
        mImgExists = exists;
    };

    enum {
        do_not_force,
        force_exif_thumb,
        force_full_thumb,
        save_thumb,
        force_save_thumb,
    };

protected:
    QImage computeIntern(const QString &file, QSharedPointer<QByteArray> ba, int forceLoad, int maxThumbSize);

    QImage mImg;
    QString mFile;
    // int s;
    bool mImgExists;
    int mMaxThumbSize;
};

class DllCoreExport DkThumbNailT : public QObject, public DkThumbNail
{
    Q_OBJECT

public:
    DkThumbNailT(const QString &mFile = QString(), const QImage &mImg = QImage());
    ~DkThumbNailT();

    bool fetchThumb(int forceLoad = do_not_force, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());

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
    QImage computeCall(const QString &filePath, QSharedPointer<QByteArray> ba, int forceLoad, int maxThumbSize);

    QFutureWatcher<QImage> mThumbWatcher;
    bool mFetching;
    int mForceLoad;
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

}
