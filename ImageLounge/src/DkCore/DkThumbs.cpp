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

DkThumbNail::DkThumbNail(const QString &filePath, const QImage &img)
{
    mImg = DkImage::createThumb(img);
    mFile = filePath;
    mImgExists = true;
}

DkThumbNail::~DkThumbNail()
{
}

void DkThumbNail::compute()
{
    if (!DkUtils::isValid(QFileInfo(mFile))) {
        qWarning() << "[Thumbnail] compute: file does not exist or is not readable" << mFile;
        return;
    }

    mImg = computeIntern(mFile, prefer_exif);
}

QString DkThumbNail::toolTip() const
{
    const QFileInfo fileInfo(getFilePath());

    // clang-format off
    QString str =
        QObject::tr("Name: ") % fileInfo.fileName() % "\n" %
        QObject::tr("Size: ") % DkUtils::readableByte((float)fileInfo.size()) % "\n" %
        QObject::tr("Created: ") % fileInfo.birthTime().toString();
    if (hasImage()) {
        str = str % "\n" %
            QObject::tr("Thumb: ") %
            QString::number(mImg.size().width()) % "x" % QString::number(mImg.size().height()) % " " %
            (mImg.text("Thumb.IsExif") == "yes" ? QObject::tr("Embedded ") : "");
    }
    // clang-format on

    return str;
}

QImage DkThumbNail::computeIntern(const QString &filePath, const int mode)
{
    DkTimer dt;

    QImage thumb;
    DkMetaDataT metaData;
    QSize origSize;

    QSharedPointer<QByteArray> ba{};
#ifdef WITH_QUAZIP
    if (QFileInfo(filePath).dir().path().contains(DkZipContainer::zipMarker()))
        ba = DkZipContainer::extractImage(DkZipContainer::decodeZipFile(filePath), DkZipContainer::decodeImageFile(filePath));
#endif

    // read the thumbnail from the exif data
    try {
        if (!ba || ba->isEmpty()) {
            metaData.readMetaData(filePath);
        } else {
            metaData.readMetaData(filePath, ba);
        }

        // read the full image if we want to recreate thumbnails
        if (mode != write_exif_always) {
            thumb = metaData.getThumbnail();
            origSize = thumb.size();
        }

        // debug tool: force loading full image
        if (qEnvironmentVariableIsSet("NOMACS_THUMB_NO_EXIF"))
            thumb = QImage();
    } catch (...) {
        // this should never happen since we handle exceptions in metaData
        qWarning() << "[Thumbnail] unexpected exception when reading exif thumbnail";
    }

    removeBlackBorder(thumb);

    const bool isExif = !thumb.isNull();
    const int rotation = metaData.getOrientationDegrees();
    const bool disableTransform = DkSettingsManager::param().metaData().ignoreExifOrientation; // match loadGeneral()

    // debug tool: tint embedded thumbs
    if (isExif && qEnvironmentVariableIsSet("NOMACS_THUMB_TINT")) {
        QPainter p(&thumb);
        p.setOpacity(0.5);
        p.fillRect(thumb.rect(), Qt::cyan);
    }

    // FIXME: why do we need link resolution here?? won't links be followed by default??
    QFileInfo fileInfo(filePath);
    QString linkFilePath = fileInfo.isSymLink() ? fileInfo.symLinkTarget() : filePath;
    fileInfo = QFileInfo(linkFilePath);

    bool transformed = false;

    // transform the exif thumbnail; do not attempt later as loadGeneral() *should* take care of that
    if (!disableTransform && isExif && rotation != DkMetaDataT::or_invalid && rotation != DkMetaDataT::or_not_set) {
        if (rotation != 0) {
            // TODO: use DkUtils rotation as in loadGeneral()
            QTransform rotationMatrix;
            rotationMatrix.rotate((double)rotation);
            thumb = thumb.transformed(rotationMatrix);
        }
        if (metaData.isOrientationMirrored())
            thumb = thumb.mirrored(true, false);

        transformed = true;
    }

    if (mode == require_exif && thumb.isNull())
        return thumb;

    // read the full image
    if ((mode != require_exif /*|| fInfo.size() < 1e5*/) && (thumb.isNull() || mode == write_exif_always)) {
        DkBasicLoader loader;
        if (loader.loadGeneral(linkFilePath, ba, true, true)) {
            thumb = loader.image();
            origSize = thumb.size();
        }
    }

    // rescale only in the default mode
    bool isScaled = false;
    if (mode == prefer_exif && !thumb.isNull()) {
        // simple antialasing but could use a lot of memory
        // FIXME: use opencv area scaler
        // FIXME: this runs even if w/h did not change
        thumb = DkImage::createThumb(thumb);
        isScaled = true;
    }

    // this was a bug; if the JPG does not have a thumbnail, but was rotated, we get a double rotation
    // if (orientation != -1 && orientation != 0 && (metaData.isJpg() || metaData.isRaw())) {
    //     QTransform rotationMatrix;
    //     rotationMatrix.rotate((double)orientation);
    //     thumb = thumb.transformed(rotationMatrix);
    //     isRotated = true;
    // }

    // save the thumbnail
    if (mode == write_exif_always || (mode == write_exif && !isExif)) {
        try {
            QImage rotatedThumb = thumb;
            if (rotation != DkMetaDataT::or_invalid && rotation != DkMetaDataT::or_not_set && rotation != 0) {
                // TODO: Use DkUtils rotation
                QTransform rotationMatrix;
                rotationMatrix.rotate(-(double)rotation);
                rotatedThumb = rotatedThumb.transformed(rotationMatrix);
                transformed = true;
            }

            metaData.updateImageMetaData(rotatedThumb);

            if (!ba || ba->isEmpty())
                metaData.saveMetaData(linkFilePath);
            else
                qWarning() << "[Thumbnail] I cannot update exif thumbnail without a file";

        } catch (...) {
            qWarning() << "Sorry, I could not save the metadata";
        }
    }

    // NOTE: setText() should not be used, since values could end up in saved metadata
    // It is OK here since it isn't something a user would save; if it was used
    // for the EXIF thumb we explicitly strip all metadata there
    // thumb.setText("Thumb.IsScaled", isScaled ? "yes" : "no");
    thumb.setText("Thumb.IsExif", isExif ? "yes" : "no");
    thumb.setText("Thumb.Size", QString("%1x%2").arg(origSize.width()).arg(origSize.height()));
    thumb.setText("Thumb.Transformed", transformed ? "yes" : "no");

    QString info = QString("[Thumbnail] %1 exif=%2 size=%3x%4 scaled=%5x%6 %8ms")
                       .arg(fileInfo.fileName())
                       .arg(isExif ? "yes" : "no")
                       .arg(origSize.width())
                       .arg(origSize.height())
                       .arg(thumb.width())
                       .arg(thumb.height())
                       .arg(dt.elapsed());
    qInfo().noquote() << info;

    return DkImage::createThumb(thumb);
}

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

void DkThumbNail::setImage(const QImage &img)
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
}

DkThumbNailT::~DkThumbNailT()
{
    mThumbWatcher.blockSignals(true);
    mThumbWatcher.cancel();
}

bool DkThumbNailT::fetchThumb(DkThumbNail::FetchMode mode)
{
    if (mode == write_exif_always || mode == write_exif)
        mImg = QImage();

    if (!mImg.isNull() || !mImgExists || mFetching)
        return false;

    // check if we can load the file
    // though if it might seem over engineered: it is much faster cascading it here
    // FIXME: hasValidSuffix() bypasses exists/readable check presumably to workaround ZIP support issues
    if (!DkUtils::hasValidSuffix(getFilePath()) && !DkUtils::isValid(QFileInfo(getFilePath())))
        return false;

    // we have to do our own bool here
    // watcher.isRunning() returns false if the thread is waiting in the pool
    mFetching = true;
    mFetchMode = mode;
    connect(&mThumbWatcher, &QFutureWatcherBase::finished, this, &DkThumbNailT::thumbLoaded, Qt::UniqueConnection);

    // Load thumbnails on their dedicated thread pool
    mThumbWatcher.setFuture(QtConcurrent::run(DkThumbsThreadPool::pool(), DkThumbNail::computeIntern, mFile, mode));

    return true;
}

void DkThumbNailT::thumbLoaded()
{
    QFuture<QImage> future = mThumbWatcher.future();

    mImg = future.result();

    if (mImg.isNull() && mFetchMode != require_exif)
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
