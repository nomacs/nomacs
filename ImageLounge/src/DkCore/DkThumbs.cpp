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
    mMaxThumbSize = qRound(max_thumb_size * DkSettingsManager::param().dpiScaleFactor());
    mImgExists = true;
}

DkThumbNail::~DkThumbNail()
{
}

/**
 * Loads the thumbnail.
 * @param forceLoad flag for loading/saving the thumbnail from exif only.
 **/
void DkThumbNail::compute(int forceLoad)
{
    // this is so complicated to be thread-safe
    // if we use member vars in the thread and the object gets deleted during thread execution we crash...
    mImg = computeIntern(mFile, QSharedPointer<QByteArray>(), forceLoad, mMaxThumbSize);
    mImg = DkImage::createThumb(mImg);
}

/**
 * Loads the thumbnail from the metadata.
 * If no thumbnail is embedded, the whole image
 * is loaded and downsampled in a fast manner.
 * @param file the file to be loaded
 * @param ba the file buffer (can be empty)
 * @param forceLoad the loading flag (e.g. exiv only)
 * @param maxThumbSize the maximal thumbnail size to be loaded
 * @param minThumbSize the minimal thumbnail size to be loaded
 * @return QImage the loaded image. Null if no image
 * could be loaded at all.
 * @reentrant all parameters must be copies or thread-safe shared pointers,
 *            no class members are allowed
 **/
QImage DkThumbNail::computeIntern(const QString &filePath, QSharedPointer<QByteArray> ba, int forceLoad, int maxThumbSize)
{
    DkTimer dt;
    // qDebug() << "[thumb] file: " << filePath;

    // see if we can read the thumbnail from the exif data
    QImage thumb;
    DkMetaDataT metaData;

    QSharedPointer<QByteArray> baZip = QSharedPointer<QByteArray>();
#ifdef WITH_QUAZIP
    if (QFileInfo(filePath).dir().path().contains(DkZipContainer::zipMarker()))
        baZip = DkZipContainer::extractImage(DkZipContainer::decodeZipFile(filePath), DkZipContainer::decodeImageFile(filePath));
#endif
    try {
        // [DIEM] READ  build crashed here 09.06.2016
        if (baZip && !baZip->isEmpty())
            metaData.readMetaData(filePath, baZip);
        else if (!ba || ba->isEmpty())
            metaData.readMetaData(filePath);
        else
            metaData.readMetaData(filePath, ba);

        // read the full image if we want to create new thumbnails
        if (forceLoad != force_save_thumb)
            thumb = metaData.getThumbnail();
    } catch (...) {
        // do nothing - we'll load the full file
    }
    removeBlackBorder(thumb);

    bool exifThumb = !thumb.isNull();
    int orientation = metaData.getOrientationDegree();

    if (exifThumb && (metaData.isAVIF() || metaData.isHEIF() || metaData.isJXL()) && orientation != -1 && orientation != 0) {
        // do not rotate together with full image but rotate Exif thumb only
        QTransform rotationMatrix;
        rotationMatrix.rotate((double)orientation);
        thumb = thumb.transformed(rotationMatrix);
    }

    QFileInfo fInfo(filePath);
    QString lFilePath = fInfo.isSymLink() ? fInfo.symLinkTarget() : filePath;
    fInfo = QFileInfo(lFilePath);

    // diem: do_not_force is the generic load - so also rescale these
    bool rescale = forceLoad == do_not_force;

    if ((forceLoad != force_exif_thumb || fInfo.size() < 1e5) && (thumb.isNull() || forceLoad == force_full_thumb || forceLoad == force_save_thumb)) { // braces

        // try to read the image
        DkBasicLoader loader;

        if (baZip && !baZip->isEmpty()) {
            if (loader.loadGeneral(lFilePath, baZip, true, true))
                thumb = loader.image();
        } else {
            if (loader.loadGeneral(lFilePath, ba, true, true))
                thumb = loader.image();
        }
    }

    if (thumb.isNull() && forceLoad == force_exif_thumb)
        return QImage();

    // the image is not scaled correctly yet
    if (rescale && !thumb.isNull()) {
        int w = thumb.width();
        int h = thumb.height();

        if (w > maxThumbSize || h > maxThumbSize) {
            if (w > h) {
                h = qRound((double)maxThumbSize / w * h);
                w = maxThumbSize;
            } else if (w < h) {
                w = qRound((double)maxThumbSize / h * w);
                h = maxThumbSize;
            } else {
                w = maxThumbSize;
                h = maxThumbSize;
            }
        }

        // scale
        thumb = thumb.scaled(QSize(w * 2, h * 2), Qt::KeepAspectRatio, Qt::FastTransformation);
        thumb = thumb.scaled(QSize(w, h), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (orientation != -1 && orientation != 0 && (metaData.isJpg() || metaData.isRaw())) {
        QTransform rotationMatrix;
        rotationMatrix.rotate((double)orientation);
        thumb = thumb.transformed(rotationMatrix);
    }

    // save the thumbnail if the caller either forces it, or the save thumb is requested and the image did not have any before
    if (forceLoad == force_save_thumb || (forceLoad == save_thumb && !exifThumb)) {
        try {
            QImage sThumb = thumb.copy();
            if (orientation != -1 && orientation != 0) {
                QTransform rotationMatrix;
                rotationMatrix.rotate(-(double)orientation);
                sThumb = sThumb.transformed(rotationMatrix);
            }

            metaData.updateImageMetaData(sThumb);

            if (!ba || ba->isEmpty())
                metaData.saveMetaData(lFilePath);
            else
                metaData.saveMetaData(ba);

            qDebug() << "[thumb] saved to exif data";
        } catch (...) {
            qWarning() << "Sorry, I could not save the metadata";
        }
    }
    // if (!thumb.isNull())
    // 	qInfoClean() << "[thumb] " << fInfo.fileName() << " (" << thumb.width() << " x " << thumb.height() << ") loaded in " << dt << ((exifThumb) ? " from
    // EXIV" : " from File");

    return thumb;
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

bool DkThumbNailT::fetchThumb(int forceLoad /* = false */, QSharedPointer<QByteArray> ba)
{
    if (forceLoad == force_full_thumb || forceLoad == force_save_thumb || forceLoad == save_thumb)
        mImg = QImage();

    if (!mImg.isNull() || !mImgExists || mFetching)
        return false;

    // check if we can load the file
    // though if it might seem over engineered: it is much faster cascading it here
    if (!DkUtils::hasValidSuffix(getFilePath()) && !QFileInfo(getFilePath()).suffix().isEmpty() && !DkUtils::isValid(QFileInfo(getFilePath())))
        return false;

    // we have to do our own bool here
    // watcher.isRunning() returns false if the thread is waiting in the pool
    mFetching = true;
    mForceLoad = forceLoad;

    connect(&mThumbWatcher, SIGNAL(finished()), this, SLOT(thumbLoaded()), Qt::UniqueConnection);

    // add work to the thread pool
    // note: arguments to lambda must be thread-safe or copies (no "&", "this") to prevent race conditions
    QString filePath = getFilePath(); // not a copy, but will detach (COW) if string is modified
    int maxThumbSize = mMaxThumbSize;
    mThumbWatcher.setFuture(QtConcurrent::run(DkThumbsThreadPool::pool(), // load thumbnails on their dedicated pool
                                              [filePath, ba, forceLoad, maxThumbSize] {
                                                  QImage thumb = DkThumbNail::computeIntern(filePath, ba, forceLoad, maxThumbSize);
                                                  return DkImage::createThumb(thumb);
                                              }));

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
