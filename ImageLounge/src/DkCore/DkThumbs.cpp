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

struct ThumbnailFromMetadata {
    QImage thumb;
    bool transformed;
};

std::optional<ThumbnailFromMetadata> loadThumbnailFromMetadata(const DkMetaDataT &metaData)
{
    QImage thumb = metaData.getThumbnail();
    if (thumb.isNull()) {
        return std::nullopt;
    }

    DkThumbNail::removeBlackBorder(thumb);

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

    return LoadThumbnailResult{
        exifThumb ? exifThumb.value().thumb : fullThumb.value(),
        linkFilePath,
        std::move(metaData),
        exifThumb.has_value(),
        exifThumb && exifThumb->transformed,
    };
}

QImage DkThumbNail::computeIntern(const QString &filePath, const int mode)
{
    DkTimer dt{};

    LoadThumbnailOption opt = LoadThumbnailOption::none;
    if (mode == write_exif || mode == write_exif_always) {
        opt = LoadThumbnailOption::force_full;
    }
    if (mode == require_exif) {
        opt = LoadThumbnailOption::force_exif;
    }
    std::optional<LoadThumbnailResult> res = loadThumbnail(filePath, opt);
    if (!res) {
        return {};
    }

    // save the thumbnail
    if (mode == write_exif_always || (mode == write_exif && !res->fromExif)) {
        try {
            int orientation = res->metaData->getOrientationDegrees();
            QImage rotatedThumb = res->thumb;
            if (orientation != DkMetaDataT::or_invalid && orientation != DkMetaDataT::or_not_set && orientation != 0) {
                // TODO: Use DkUtils rotation
                QTransform rotationMatrix;
                rotationMatrix.rotate(-(double)orientation);
                rotatedThumb = rotatedThumb.transformed(rotationMatrix);
            }

            res->metaData->updateImageMetaData(rotatedThumb);
            res->metaData->saveMetaData(res->filePath);
        } catch (...) {
            qWarning() << "Sorry, I could not save the metadata";
        }
    }

    QSize origSize = res->thumb.size();

    QImage scaled = DkImage::createThumb(res->thumb);

    QString info = QString("[Thumbnail] %1 exif=%2 size=%3x%4 scaled=%5x%6 %8ms")
                       .arg(res->filePath)
                       .arg(res->fromExif ? "yes" : "no")
                       .arg(origSize.width())
                       .arg(origSize.height())
                       .arg(scaled.width())
                       .arg(scaled.height())
                       .arg(dt.elapsed());
    qInfo().noquote() << info;

    // TODO: replace these with passed struct
    // NOTE: setText() should not be used, since values could end up in saved metadata
    // It is OK here since it isn't something a user would save; if it was used
    // for the EXIF thumb we explicitly strip all metadata there
    // thumb.setText("Thumb.IsScaled", isScaled ? "yes" : "no");
    scaled.setText("Thumb.IsExif", res->fromExif ? "yes" : "no");
    scaled.setText("Thumb.Transformed", res->transformed ? "yes" : "no");

    return scaled;
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
