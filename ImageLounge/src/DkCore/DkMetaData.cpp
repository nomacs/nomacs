/*******************************************************************************************************
 DkMetaData.cpp
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

#include "DkMetaData.h"

#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#include <QApplication>
#include <QBuffer>
#include <QImage>
#include <QObject>
#include <QRegularExpression>
#include <QVector2D>

#include <iostream>

namespace nmc
{
// DkMetaDataT --------------------------------------------------------------------
DkMetaDataT::DkMetaDataT()
{
    mExifState = not_loaded;
}

bool DkMetaDataT::isNull()
{
    return !mExifImg.get();
}

QSharedPointer<DkMetaDataT> DkMetaDataT::copy() const
{
    // Copy Exiv2::Image object
    QSharedPointer<DkMetaDataT> metaDataN(new DkMetaDataT());
    metaDataN->mFileInfo = mFileInfo;
    metaDataN->mExifState = mExifState;

    if (mExifImg.get() != nullptr) {
        // ImageFactory::create(type) may crash even if old Image object has that type
        try {
            // Load new Exiv2::Image object
            metaDataN->mExifImg = Exiv2::ImageFactory::create(mExifImg->imageType());
            // Copy exif data from old object into new object
            Exiv2::ExifData data = mExifImg->exifData();
            metaDataN->mExifImg->setExifData(data); // explicit copy of list<Exifdatum>
            metaDataN->mExifState = dirty;
        } catch (...) {
            metaDataN->mExifState = no_data;
        }
    }

    return metaDataN;
}

/**
 * @brief Updates exif data to match that of other.
 *
 * @param other
 */
void DkMetaDataT::update(const QSharedPointer<DkMetaDataT> &other)
{
    QSharedPointer<DkMetaDataT> src(other);
    // Copy exif data (to this instance), reading from src
    if (src->isNull())
        return;
    mExifImg->setExifData(src->mExifImg->exifData()); // explicit copy of list<Exifdatum>
}

void DkMetaDataT::readMetaData(const DkFileInfo &file, QSharedPointer<QByteArray> ba)
{
    mExifState = no_data;

    if (mUseSidecar) {
        loadSidecar(file.path());
        return;
    }

    mFileInfo = file;

    try {
        if (!ba || ba->isEmpty()) {
            DkFileInfo tmpFileInfo = file;
            if (tmpFileInfo.isSymLink() && !tmpFileInfo.resolveSymLink()) {
                qWarning() << "[DkMetaDataT] broken link" << file.path();
                return;
            }

            // FIXME: this does not support unicode file names on windows
            // we could use DkFileInfo::getIoDevice() and subclass Exiv2::BasicIo
            mExifImg = Exiv2::ImageFactory::open(qUtf8Printable(tmpFileInfo.path()));
        } else {
            mExifImg = Exiv2::ImageFactory::open(reinterpret_cast<const byte *>(ba->constData()), ba->size());
        }

    } catch (...) {
        // TODO: check crashes here
        // qDebug() << "[Exiv2] could not open file for exif data";
        qInfo() << "[Exiv2] could not load Exif data from file:" << file.fileName();
        return;
    }

    if (mExifImg.get() == nullptr) {
        qDebug() << "[Exiv2] image could not be opened for exif data extraction";
        return;
    }

    try {
        mExifImg->readMetadata();

        if (!mExifImg->good()) {
            qDebug() << "[Exiv2] metadata could not be read";
            return;
        }

        if (mExifImg->exifData().empty() && mExifImg->xmpData().empty() && mExifImg->iptcData().empty()
            && mExifImg->iptcData().empty()) {
            qDebug() << "[Exiv2] metadata is empty";
            return;
        }
        qDebug() << "[Exiv2] metadata loaded" << mExifImg->mimeType().c_str() << mExifImg->pixelWidth()
                 << mExifImg->pixelHeight();

    } catch (...) {
        qDebug() << "[Exiv2] could not read metadata";
        return;
    }

    mExifState = loaded;

    // printMetaData();
}

/**
 * @brief saveMetaData() reloads the specified file and updates that file with the modified exif data.
 *
 * Note that this changes the file (again).
 * It's a wrapper around the real saveMetaData() function, which works with an in-memory copy.
 *
 * @param fileInfo file to be read and updated
 * @param force update file even if no exif data has been changed
 */
bool DkMetaDataT::saveMetaData(const DkFileInfo &fileInfo, bool force)
{
    if (mExifState != loaded && mExifState != dirty)
        return false;

    QSharedPointer<QByteArray> ba;
    {
        std::unique_ptr<QIODevice> io = fileInfo.getIODevice();
        if (!io) {
            qWarning() << "[DkMetaDataT] could not open for reading:" << fileInfo.fileName();
            return false;
        }
        ba.reset(new QByteArray(io->readAll()));
    }

    // Write modified metadata (from this instance) to mExifImg and then to ba (in-memory image file)
    bool saved = saveMetaData(ba, force);
    if (!saved) {
        qDebug() << "[DkMetaDataT] could not save: " << fileInfo.fileName();
        return saved;
    } else if (ba->isEmpty()) {
        qDebug() << "[DkMetaDataT] could not save: " << fileInfo.fileName() << " empty Buffer!";
        return false;
    }

    // FIXME: zip: test if file is writable here

    // Open image file for WRITING, save it with modified metadata
    QFile file(fileInfo.path());
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "[DkMetaDataT] could not open for writing:" << fileInfo.fileName() << file.error()
                   << file.errorString();
        return false;
    }

    if (ba->size() != file.write(ba->data(), ba->size())) {
        qWarning() << "[DkMetaDataT] write failed:" << fileInfo.fileName() << file.error() << file.errorString();
        return false;
    }

    qInfo() << "[DkMetaDataT] " << fileInfo.fileName() << "wrote" << ba->size() << " bytes";

    return true;
}

/**
 * @brief saveMetaData() updates the exif record of an in-memory copy of the image file
 *
 * It takes a copy of the file, as a byte array reference, and creates a new copy
 * with the new exif data. The reference pointer is changed to point to this new copy.
 * The caller is expected to write this new copy to disk or whatever.
 *
 * @param ba in-memory copy of the file to be updated
 * @param force update file even if no exif data has been changed
 */
bool DkMetaDataT::saveMetaData(QSharedPointer<QByteArray> &ba, bool force)
{
    if (!ba)
        return false;

    if (!force && mExifState != dirty)
        return false;
    else if (mExifState == not_loaded || mExifState == no_data)
        return false;

    // Get exif sections from currently loaded image (old exif object)
    Exiv2::ExifData &exifData = mExifImg->exifData();
    Exiv2::XmpData &xmpData = mExifImg->xmpData();
    Exiv2::IptcData &iptcData = mExifImg->iptcData();

    std::unique_ptr<Exiv2::Image> exifImgN;

    try {
        // Load new exif object (based on byte array of raw image file, see overload)
        exifImgN = Exiv2::ImageFactory::open(reinterpret_cast<const byte *>(ba->constData()), ba->size());
    } catch (...) {
        qDebug() << "could not open image for exif data";
        return false;
    }

    if (exifImgN.get() == nullptr) {
        qDebug() << "image could not be opened for exif data extraction";
        return false;
    }

    exifImgN->readMetadata();

    // Update new exif object, copy changes made to the old exif object (loaded image)
    exifImgN->setExifData(exifData);
    exifImgN->setXmpData(xmpData);
    exifImgN->setIptcData(iptcData);

    exifImgN->writeMetadata(); // TODO: CIMG6206.jpg crashes here...

    // Copy image + new exif and return temporary object as byte array
    // The calling function should then write it back to the file
    Exiv2::DataBuf exifBuf = exifImgN->io().read((long)exifImgN->io().size());

#if ((((EXIV2_MAJOR_VERSION) << 16) + ((EXIV2_MINOR_VERSION) << 8) + (EXIV2_PATCH_VERSION)) >= (28 << 8))
    if (!exifBuf.empty()) {
        QSharedPointer<QByteArray> tmp = QSharedPointer<QByteArray>(
            new QByteArray(reinterpret_cast<const char *>(exifBuf.c_data()), exifBuf.size()));
#else
    if (exifBuf.pData_) {
        QSharedPointer<QByteArray> tmp = QSharedPointer<QByteArray>(
            new QByteArray(reinterpret_cast<const char *>(exifBuf.pData_), exifBuf.size_));
#endif
        if (tmp->size() > qRound(ba->size() * 0.5f))
            ba = tmp;
        else
            return false; // catch exif bug - observed e.g. for hasselblad RAW (3fr) files - see: Bug #995
                          // (http://dev.exiv2.org/issues/995)
    } else
        return false;

    // Replace old exif object with new one and clear "dirty" flag
    mExifImg.swap(exifImgN);
    mExifState = loaded;

    return true;
}

QString DkMetaDataT::getDescription() const
{
    QString description;

    if (mExifState != loaded && mExifState != dirty)
        return description;

    try {
        const Exiv2::ExifData &exifData = mExifImg->exifData();

        if (!exifData.empty()) {
            const auto key = Exiv2::ExifKey("Exif.Image.ImageDescription");
            const auto pos = exifData.findKey(key);

            if (pos != exifData.end() && pos->count() != 0) {
                description = exiv2ToQString(pos->toString());
            }
        }
    } catch (...) {
        qDebug() << "[DkMetaDataT] Error: could not load description";
        return description;
    }

    return description;
}

bool DkMetaDataT::isOrientationMirrored() const
{
    QString value = getExifValue("Orientation");

    bool ok;
    int orientation = value.toInt(&ok);
    if (value.isEmpty() || !ok)
        return false;

    if (orientation < 1 || orientation > 8) {
        qWarning() << "[EXIF] Bogus orientation:" << orientation;
        return false;
    }

    switch (orientation) {
    case 1:
    case 3:
    case 6:
    case 8:
        return false;
    case 2:
    case 4:
    case 5:
    case 7:
        return true;
    }
    return false;
}

int DkMetaDataT::getOrientationDegrees() const
{
    QString value = getExifValue("Orientation");
    if (value.isEmpty())
        return or_not_set;

    bool ok;
    int orientation = value.toInt(&ok);

    if (!ok || orientation < 1 || orientation > 8) {
        qWarning() << "[EXIF] Bogus orientation:" << orientation;
        return or_invalid;
    }

    switch (orientation) {
    case 1:
        return 0;
    case 6:
        return 90;
    case 3:
        return 180;
    case 8:
        return -90;
    case 2:
        return 0;
    case 4:
        return 180;
    case 5:
        return 90;
    case 7:
        return -90;
    default:
        return or_invalid;
    }
}

int DkMetaDataT::getRating() const
{
    if (mExifState != loaded && mExifState != dirty)
        return -1;

    float exifRating = -1;
    float xmpRating = -1;
    float fRating = 0;

    const Exiv2::ExifData &exifData = mExifImg->exifData(); // Exif.Image.Rating  - short
    const Exiv2::XmpData &xmpData = mExifImg->xmpData(); // Xmp.xmp.Rating - text

    // get Rating of Exif Tag
    if (!exifData.empty()) {
        const auto key = Exiv2::ExifKey("Exif.Image.Rating");
        const auto pos = exifData.findKey(key);

        if (pos != exifData.end() && pos->count() != 0) {
            auto v = pos->getValue();
            exifRating = v->toFloat();
        }
    }

    // get Rating of Xmp Tag
    if (!xmpData.empty()) {
        auto key = Exiv2::XmpKey("Xmp.xmp.Rating");
        auto pos = xmpData.findKey(key);

        // xmp Rating tag
        if (pos != xmpData.end() && pos->count() != 0) {
            auto v = pos->getValue();
            xmpRating = v->toFloat();
        }

        // if xmpRating not found, try to find MicrosoftPhoto Rating tag
        if (xmpRating == -1) {
            key = Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating");
            pos = xmpData.findKey(key);
            if (pos != xmpData.end() && pos->count() != 0) {
                auto v = pos->getValue();
                xmpRating = v->toFloat();
            }
        }
    }

    if (xmpRating == -1.0f && exifRating != -1.0f)
        fRating = exifRating;
    else if (xmpRating != -1.0f && exifRating == -1.0f)
        fRating = xmpRating;
    else
        fRating = exifRating;

    return qRound(fRating);
}

QSize DkMetaDataT::getImageSize() const
{
    QSize size;

    if (mExifState != loaded && mExifState != dirty)
        return size;

    bool ok = false;
    int width = getNativeExifValue("Exif.Photo.PixelXDimension", false).toInt(&ok);

    if (!ok)
        return size;

    int height = getNativeExifValue("Exif.Photo.PixelYDimension", false).toInt(&ok);

    if (!ok)
        return size;

    return QSize(width, height);
}

QString DkMetaDataT::getNativeExifValue(const QString &key, bool humanReadable) const
{
    QString info;

    if (mExifState != loaded && mExifState != dirty)
        return info;

    const Exiv2::ExifData &exifData = mExifImg->exifData();

    if (!exifData.empty()) {
        Exiv2::ExifData::const_iterator pos;

        try {
            auto ekey = Exiv2::ExifKey(key.toStdString());
            pos = exifData.findKey(ekey);

        } catch (...) {
            return info;
        }

        if (pos != exifData.end() && pos->count() != 0) {
            if (pos->count()
                < 2000) { // diem: this is about performance - adobe obviously embeds whole images into tiff exiv data

                // qDebug() << "pos count: " << pos->count();
                // Exiv2::Value::AutoPtr v = pos->getValue();
                if (key == QLatin1String("Exif.Photo.UserComment")) {
                    info = QString::fromStdString(static_cast<const Exiv2::CommentValue &>(pos->value()).comment());
                } else if (humanReadable) {
                    std::stringstream ss;
                    ss << *pos;
                    info = exiv2ToQString(ss.str());
                } else {
                    info = exiv2ToQString(pos->toString());
                }

            } else {
                info = QObject::tr("<data too large to display>");
            }
        }
    }

    return info;
}

QString DkMetaDataT::getXmpValue(const QString &key) const
{
    QString info;

    if (mExifState != loaded && mExifState != dirty)
        return info;

    const Exiv2::XmpData &xmpData = mExifImg->xmpData();

    if (!xmpData.empty()) {
        Exiv2::XmpData::const_iterator pos;

        try {
            auto ekey = Exiv2::XmpKey(key.toStdString());
            pos = xmpData.findKey(ekey);

        } catch (...) {
            return info;
        }

        if (pos != xmpData.end() && pos->count() != 0) {
            info = exiv2ToQString(pos->toString());
        }
    }

    return info;
}

QString DkMetaDataT::getExifValue(const QString &key) const
{
    QString info;

    if (mExifState != loaded && mExifState != dirty)
        return info;

    const Exiv2::ExifData &exifData = mExifImg->exifData();
    std::string sKey = key.toStdString();

    if (!exifData.empty()) {
        Exiv2::ExifData::const_iterator pos;

        try {
            auto ekey = Exiv2::ExifKey("Exif.Image." + sKey);
            pos = exifData.findKey(ekey);

            if (pos == exifData.end() || pos->count() == 0) {
                Exiv2::ExifKey lEkey = Exiv2::ExifKey("Exif.Photo." + sKey);
                pos = exifData.findKey(lEkey);
            }
        } catch (...) {
            try {
                sKey = "Exif.Photo." + sKey;
                Exiv2::ExifKey ekey = Exiv2::ExifKey(sKey);
                pos = exifData.findKey(ekey);
            } catch (...) {
                return "";
            }
        }

        if (pos != exifData.end() && pos->count() != 0) {
            // Exiv2::Value::AutoPtr v = pos->getValue();
            info = exiv2ToQString(pos->toString());
        }
    }

    return info;
}

QString DkMetaDataT::getIptcValue(const QString &key) const
{
    QString info;

    if (mExifState != loaded && mExifState != dirty)
        return info;

    const Exiv2::IptcData &iptcData = mExifImg->iptcData();

    if (!iptcData.empty()) {
        Exiv2::IptcData::const_iterator pos;

        try {
            auto ekey = Exiv2::IptcKey(key.toStdString());
            pos = iptcData.findKey(ekey);
        } catch (...) {
            return info;
        }

        if (pos != iptcData.end() && pos->count() != 0) {
            auto v = pos->getValue();
            info = exiv2ToQString(pos->toString());
        }
    }

    return info;
}

void DkMetaDataT::getFileMetaData(QStringList &fileKeys, QStringList &fileValues) const
{
    fileKeys.append(QObject::tr("Filename"));
    fileValues.append(mFileInfo.fileName());

    fileKeys.append(QObject::tr("Path"));
    fileValues.append(mFileInfo.path());

    if (mFileInfo.isSymLink()) {
        fileKeys.append(QObject::tr("Target"));
        fileValues.append(mFileInfo.symLinkTarget());
    }

    fileKeys.append(QObject::tr("Size"));
    fileValues.append(DkUtils::readableByte((float)mFileInfo.size()));

    // date group
    fileKeys.append(QObject::tr("Date") + "." + QObject::tr("Created"));
    fileValues.append(mFileInfo.birthTime().toString());

    fileKeys.append(QObject::tr("Date") + "." + QObject::tr("Last Modified"));
    fileValues.append(mFileInfo.lastModified().toString());

    fileKeys.append(QObject::tr("Date") + "." + QObject::tr("Last Read"));
    fileValues.append(mFileInfo.lastRead().toString());

    if (!mFileInfo.owner().isEmpty()) {
        fileKeys.append(QObject::tr("Owner"));
        fileValues.append(mFileInfo.owner());
    }

    fileKeys.append(QObject::tr("OwnerID"));
    fileValues.append(QString::number(mFileInfo.ownerId()));

    if (!mFileInfo.group().isEmpty()) {
        fileKeys.append(QObject::tr("Group"));
        fileValues.append(mFileInfo.group());
    }

    QString permissionString;
    fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("Owner"));
    permissionString += mFileInfo.permissions() & QFile::ReadOwner ? "r" : "-";
    permissionString += mFileInfo.permissions() & QFile::WriteOwner ? "w" : "-";
    permissionString += mFileInfo.permissions() & QFile::ExeOwner ? "x" : "-";
    fileValues.append(permissionString);

    permissionString = "";
    fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("User"));
    permissionString += mFileInfo.permissions() & QFile::ReadUser ? "r" : "-";
    permissionString += mFileInfo.permissions() & QFile::WriteUser ? "w" : "-";
    permissionString += mFileInfo.permissions() & QFile::ExeUser ? "x" : "-";
    fileValues.append(permissionString);

    permissionString = "";
    fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("Group"));
    permissionString += mFileInfo.permissions() & QFile::ReadGroup ? "r" : "-";
    permissionString += mFileInfo.permissions() & QFile::WriteGroup ? "w" : "-";
    permissionString += mFileInfo.permissions() & QFile::ExeGroup ? "x" : "-";
    fileValues.append(permissionString);

    permissionString = "";
    fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("Other"));
    permissionString += mFileInfo.permissions() & QFile::ReadOther ? "r" : "-";
    permissionString += mFileInfo.permissions() & QFile::WriteOther ? "w" : "-";
    permissionString += mFileInfo.permissions() & QFile::ExeOther ? "x" : "-";
    fileValues.append(permissionString);

    QStringList tmpKeys;

    // full file keys are needed to create the hierarchy
    for (int idx = 0; idx < fileKeys.size(); idx++) {
        tmpKeys.append(QObject::tr("File") + "." + fileKeys.at(idx));
    }

    fileKeys = tmpKeys;
}

void DkMetaDataT::getAllMetaData(QStringList &keys, QStringList &values) const
{
    QStringList exifKeys = getExifKeys();

    for (int idx = 0; idx < exifKeys.size(); idx++) {
        QString cKey = exifKeys.at(idx);
        QString exifValue = getNativeExifValue(cKey, true);

        keys.append(cKey);
        values.append(exifValue);
    }

    QStringList iptcKeys = getIptcKeys();

    for (int idx = 0; idx < iptcKeys.size(); idx++) {
        QString cKey = iptcKeys.at(idx);
        QString exifValue = getIptcValue(iptcKeys.at(idx));

        keys.append(cKey);
        values.append(exifValue);
    }

    QStringList xmpKeys = getXmpKeys();

    for (int idx = 0; idx < xmpKeys.size(); idx++) {
        QString cKey = xmpKeys.at(idx);
        QString exifValue = getXmpValue(xmpKeys.at(idx));

        keys.append(cKey);
        values.append(exifValue);
    }

    QStringList qtKeys = getQtKeys();

    for (QString cKey : qtKeys) {
        keys.append(cKey);
        values.append(getQtValue(cKey));
    }
}

QImage DkMetaDataT::getThumbnail() const
{
    QImage qThumb;

    if (mExifState != loaded && mExifState != dirty)
        return qThumb;

    Exiv2::ExifData &exifData = mExifImg->exifData();

    if (exifData.empty())
        return qThumb;

    try {
        Exiv2::ExifThumb thumb(exifData);
        Exiv2::DataBuf buffer = thumb.copy();

#if ((((EXIV2_MAJOR_VERSION) << 16) + ((EXIV2_MINOR_VERSION) << 8) + (EXIV2_PATCH_VERSION)) >= (28 << 8))
        QByteArray ba = QByteArray(reinterpret_cast<const char *>(buffer.c_data()), buffer.size());
#else
        QByteArray ba = QByteArray(reinterpret_cast<const char *>(buffer.pData_), buffer.size_);
#endif
        qThumb.loadFromData(ba);
    } catch (...) {
        qDebug() << "Sorry, I could not load the thumb from the exif data...";
    }

    return qThumb;
}

QString DkMetaDataT::getMimeType() const
{
    QString type;
    try {
        if (mExifImg)
            type = mExifImg->mimeType().c_str();
    } catch (...) { // NOLINT nothing else we can do in catch
    }
    return type;
}

QImage DkMetaDataT::getPreviewImage(int minPreviewWidth) const
{
    QImage qImg;

    if (mExifState != loaded && mExifState != dirty)
        return qImg;

    Exiv2::ExifData &exifData = mExifImg->exifData();

    if (exifData.empty())
        return qImg;

    try {
        Exiv2::PreviewManager loader(*mExifImg);
        Exiv2::PreviewPropertiesList pList = loader.getPreviewProperties();

        int maxWidth = 0;
        int mIdx = -1;

        // select the largest preview image
        for (size_t idx = 0; idx < pList.size(); idx++) {
            if (pList[idx].width_ > (uint32_t)maxWidth && pList[idx].width_ > (uint32_t)minPreviewWidth) {
                mIdx = static_cast<int>(idx);
                maxWidth = static_cast<int>(pList[idx].width_);
            }
        }

        if (mIdx == -1)
            return qImg;

        // Get the selected preview image
        Exiv2::PreviewImage preview = loader.getPreviewImage(pList[mIdx]);

        QByteArray ba((const char *)preview.pData(), preview.size());
        if (!qImg.loadFromData(ba))
            return QImage();
    } catch (...) {
        qDebug() << "Sorry, I could not load the thumb from the exif data...";
    }

    return qImg;
}

void DkMetaDataT::setUseSidecar(bool useSidecar)
{
    mUseSidecar = useSidecar;
}

bool DkMetaDataT::useSidecar() const
{
    return mUseSidecar;
}

bool DkMetaDataT::hasMetaData() const
{
    return !(mExifState == no_data || mExifState == not_loaded);
}

bool DkMetaDataT::isLoaded() const
{
    return mExifState == loaded || mExifState == dirty || mExifState == no_data;
}

bool DkMetaDataT::isTiff() const
{
    return mFileInfo.suffix().contains(QRegularExpression("(tif|tiff)", QRegularExpression::CaseInsensitiveOption))
        != 0;
}

bool DkMetaDataT::isJpg() const
{
    return mFileInfo.suffix().contains(QRegularExpression("(jpg|jpeg)", QRegularExpression::CaseInsensitiveOption))
        != 0;
}

bool DkMetaDataT::isRaw() const
{
    return mFileInfo.suffix().contains(
               QRegularExpression("(nef|crw|cr2|arw)", QRegularExpression::CaseInsensitiveOption))
        != 0;
}

bool DkMetaDataT::isAVIF() const
{
    return mFileInfo.suffix().contains(QRegularExpression("(avif)", QRegularExpression::CaseInsensitiveOption)) != 0;
}

bool DkMetaDataT::isHEIF() const
{
    return mFileInfo.suffix().contains(QRegularExpression("(heic|heif)", QRegularExpression::CaseInsensitiveOption))
        != 0;
}

bool DkMetaDataT::isJXL() const
{
    return mFileInfo.suffix().contains(QRegularExpression("(jxl)", QRegularExpression::CaseInsensitiveOption)) != 0;
}

bool DkMetaDataT::isDirty() const
{
    return mExifState == dirty;
}

QStringList DkMetaDataT::getExifKeys() const
{
    QStringList exifKeys;

    if (mExifState != loaded && mExifState != dirty)
        return exifKeys;

    const Exiv2::ExifData &exifData = mExifImg->exifData();

    if (exifData.empty()) {
        return exifKeys;
    } else {
        for (Exiv2::Exifdatum i : exifData) {
            std::string tmp = i.key();
            exifKeys << QString::fromStdString(tmp);

            // qDebug() << QString::fromStdString(tmp);
        }
    }

    return exifKeys;
}

QStringList DkMetaDataT::getXmpKeys() const
{
    QStringList xmpKeys;

    if (mExifState != loaded && mExifState != dirty)
        return xmpKeys;

    const Exiv2::XmpData &xmpData = mExifImg->xmpData();
    const auto end = xmpData.end();

    if (xmpData.empty()) {
        return xmpKeys;

    } else {
        for (auto i = xmpData.begin(); i != end; ++i) {
            std::string tmp = i->key();
            xmpKeys << QString::fromStdString(tmp);
        }
    }

    return xmpKeys;
}

QStringList DkMetaDataT::getIptcKeys() const
{
    QStringList iptcKeys;

    if (mExifState != loaded && mExifState != dirty)
        return iptcKeys;

    const Exiv2::IptcData &iptcData = mExifImg->iptcData();
    const auto endI = iptcData.end();

    if (iptcData.empty())
        return iptcKeys;

    for (auto md = iptcData.begin(); md != endI; ++md) {
        std::string tmp = md->key();
        iptcKeys << QString::fromStdString(tmp);
    }

    return iptcKeys;
}

QStringList DkMetaDataT::getExifValues() const
{
    QStringList exifValues;

    if (mExifState != loaded && mExifState != dirty)
        return QStringList();

    const Exiv2::ExifData &exifData = mExifImg->exifData();
    const auto end = exifData.end();

    if (exifData.empty())
        return exifValues;

    for (auto i = exifData.begin(); i != end; ++i) {
        std::string tmp = i->value().toString();
        QString info = exiv2ToQString(tmp);
        exifValues << info;
    }

    return exifValues;
}

QStringList DkMetaDataT::getIptcValues() const
{
    QStringList iptcValues;

    if (mExifState != loaded && mExifState != dirty)
        return iptcValues;

    const Exiv2::IptcData &iptcData = mExifImg->iptcData();
    const auto endI = iptcData.end();

    if (iptcData.empty())
        return iptcValues;
    for (auto md = iptcData.begin(); md != endI; ++md) {
        std::string tmp = md->value().toString();
        iptcValues << exiv2ToQString(tmp);
    }

    return iptcValues;
}

void DkMetaDataT::setQtValues(const QImage &img)
{
    // Omit "Raw profile type exif" set by ???, presumably it would conflict when saving file
    // This is a known unregistered tag in PNG: https://exiftool.org/TagNames/PNG.html
    const QStringList keys = img.textKeys();
    for (const auto &key : keys) {
        if (key.isEmpty() || key == "Raw profile type exif")
            continue;
        QString value = img.text(key);
        if (value.length() >= 5000)
            value = QObject::tr("<data too large to display>");

        if (!value.isEmpty()) {
            mQtValues.append(value);
            mQtKeys.append(key);
        }
    }
}

QString DkMetaDataT::getQtValue(const QString &key) const
{
    int idx = mQtKeys.indexOf(key);

    if (idx >= 0 && idx < mQtValues.size())
        return mQtValues.at(idx);

    return QString();
}

QStringList DkMetaDataT::getQtKeys() const
{
    return mQtKeys;
}

QStringList DkMetaDataT::getQtValues() const
{
    return mQtValues;
}

void DkMetaDataT::setThumbnail(QImage thumb)
{
    if (mExifState == not_loaded || mExifState == no_data)
        return;

    try {
        Exiv2::ExifData exifData = mExifImg->exifData();

        if (exifData.empty())
            exifData = Exiv2::ExifData();

        // ok, let's try to save the thumbnail...
        Exiv2::ExifThumb eThumb(exifData);

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        thumb.save(&buffer, "JPEG"); // here we destroy the alpha channel of thumbnails

        try {
            // whipe all exif data of the thumbnail
            auto exifImgThumb = Exiv2::ImageFactory::open(reinterpret_cast<const byte *>(ba.constData()), ba.size());

            if (exifImgThumb.get() != nullptr && exifImgThumb->good())
                exifImgThumb->clearExifData();
        } catch (...) {
            qDebug() << "could not clear the thumbnail exif info";
        }

        eThumb.erase(); // erase all thumbnails
        eThumb.setJpegThumbnail((Exiv2::byte *)ba.data(), ba.size());

        mExifImg->setExifData(exifData);
        mExifState = dirty;

    } catch (...) {
        qDebug() << "I could not save the thumbnail...";
    }
}

QVector2D DkMetaDataT::getResolution() const
{
    QVector2D resV = QVector2D(72, 72);
    QString xRes, yRes;

    try {
        if (hasMetaData()) {
            xRes = getExifValue("XResolution");
            QStringList res;
            res = xRes.split("/");

            if (res.size() != 2)
                return resV;

            if (res.at(0).toFloat() != 0 && res.at(1).toFloat() != 0)
                resV.setX(res.at(0).toFloat() / res.at(1).toFloat());

            yRes = getExifValue("YResolution");
            res = yRes.split("/");

            // qDebug() << "Resolution"  << xRes << " " << yRes;
            if (res.size() != 2)
                return resV;

            if (res.at(0).toFloat() != 0 && res.at(1).toFloat() != 0)
                resV.setY(res.at(0).toFloat() / res.at(1).toFloat());
        }
    } catch (...) {
        qDebug() << "could not load Exif resolution, set to 72dpi";
    }

    return resV;
}

void DkMetaDataT::setResolution(const QVector2D &res)
{
    if (getResolution() == res)
        return;

    QString x, y;
    x.setNum(res.x());
    y.setNum(res.y());
    x = x + "/1";
    y = y + "/1";

    setExifValue("Exif.Image.XResolution", x);
    setExifValue("Exif.Image.YResolution", y);
}

void DkMetaDataT::clearOrientation()
{
    if (mExifState == not_loaded || mExifState == no_data)
        return;

    setExifValue("Exif.Image.Orientation", "1"); // we wrote "0" here - that was against the standard!
}

void DkMetaDataT::clearExifState()
{
    if (mExifState == dirty)
        mExifState = loaded;
}

void DkMetaDataT::setOrientation(int o)
{
    if (mExifState == not_loaded || mExifState == no_data)
        return;

    if (o != 90 && o != -90 && o != 180 && o != 0 && o != 270)
        return;

    if (o == -180)
        o = 180;
    if (o == 270)
        o = -90;

    int orientation = 1;

    Exiv2::ExifData &exifData = mExifImg->exifData();
    const auto key = Exiv2::ExifKey("Exif.Image.Orientation");

    // this does not really work -> *.bmp images
    if (exifData.empty())
        exifData["Exif.Image.Orientation"] = uint16_t(1);

    auto pos = exifData.findKey(key);

    if (pos == exifData.end() || pos->count() == 0) {
        exifData["Exif.Image.Orientation"] = uint16_t(1);
        pos = exifData.findKey(key);
    }

    auto v = pos->getValue();
    auto *prv = dynamic_cast<Exiv2::UShortValue *>(v.release());
    if (!prv)
        return;

    std::unique_ptr<Exiv2::UShortValue> rv(prv);
    if (rv->value_.empty())
        return;

    orientation = (int)rv->value_[0];
    if (orientation <= 0 || orientation > 8)
        orientation = 1;

    switch (orientation) {
    case 1:
        if (o != 0)
            orientation = (o == -90) ? 8 : (o == 90 ? 6 : 3);
        break;
    case 2:
        if (o != 0)
            orientation = (o == -90) ? 5 : (o == 90 ? 7 : 4);
        break;
    case 3:
        if (o != 0)
            orientation = (o == -90) ? 6 : (o == 90 ? 8 : 1);
        break;
    case 4:
        if (o != 0)
            orientation = (o == -90) ? 7 : (o == 90 ? 5 : 2);
        break;
    case 5:
        if (o != 0)
            orientation = (o == -90) ? 4 : (o == 90 ? 2 : 7);
        break;
    case 6:
        if (o != 0)
            orientation = (o == -90) ? 1 : (o == 90 ? 3 : 8);
        break;
    case 7:
        if (o != 0)
            orientation = (o == -90) ? 2 : (o == 90 ? 4 : 5);
        break;
    case 8:
        if (o != 0)
            orientation = (o == -90) ? 3 : (o == 90 ? 1 : 6);
        break;
    }
    rv->value_[0] = (unsigned short)orientation;
    pos->setValue(rv.get());

    mExifImg->setExifData(exifData);

    mExifState = dirty;
}

bool DkMetaDataT::setDescription(const QString &description)
{
    if (mExifState == not_loaded || mExifState == no_data)
        return false;

    return setExifValue("Exif.Image.ImageDescription", description.toUtf8());
}

void DkMetaDataT::setRating(int r)
{
    if (mExifState == not_loaded || mExifState == no_data || getRating() == r)
        return;

    int16_t ratingPercent = 0;
    switch (r) {
    case 1:
        ratingPercent = 1;
        break;
    case 2:
        ratingPercent = 25;
        break;
    case 3:
        ratingPercent = 50;
        break;
    case 4:
        ratingPercent = 75;
        break;
    case 5:
        ratingPercent = 99;
        break;
    default:
        r = 0;
    }

    Exiv2::ExifData &exifData = mExifImg->exifData(); // Exif.Image.Rating  - short
    Exiv2::XmpData &xmpData = mExifImg->xmpData(); // Xmp.xmp.Rating - text

    if (r > 0) {
        exifData["Exif.Image.Rating"] = uint16_t(r);
        exifData["Exif.Image.RatingPercent"] = ratingPercent;

        auto v = Exiv2::Value::create(Exiv2::xmpText);
        v->read(std::to_string(r));
        xmpData.add(Exiv2::XmpKey("Xmp.xmp.Rating"), v.get());
        v->read(std::to_string(ratingPercent));
        xmpData.add(Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating"), v.get());
    } else {
        Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Rating");
        auto pos = exifData.findKey(key);
        if (pos != exifData.end())
            exifData.erase(pos);

        key = Exiv2::ExifKey("Exif.Image.RatingPercent");
        pos = exifData.findKey(key);
        if (pos != exifData.end())
            exifData.erase(pos);

        Exiv2::XmpKey key2 = Exiv2::XmpKey("Xmp.xmp.Rating");
        auto pos2 = xmpData.findKey(key2);
        if (pos2 != xmpData.end())
            xmpData.erase(pos2);

        key2 = Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating");
        pos2 = xmpData.findKey(key2);
        if (pos2 != xmpData.end())
            xmpData.erase(pos2);
    }

    try {
        mExifImg->setExifData(exifData);
        mExifImg->setXmpData(xmpData);

        mExifState = dirty;
    } catch (...) {
        qDebug() << "[WARNING] I could not set the exif data for this image format...";
    }
}

bool DkMetaDataT::updateImageMetaData(const QImage &img, bool reset_orientation)
{
    bool success = true;

    success &= setExifValue("Exif.Image.ImageWidth", QString::number(img.width()));
    success &= setExifValue("Exif.Image.ImageLength", QString::number(img.height()));
    success &= setExifValue("Exif.Image.ProcessingSoftware",
                            qApp->organizationName() + " - " + qApp->applicationName() + " "
                                + qApp->applicationVersion());

    // TODO: convert Date Time to Date Time Original and set new Date Time

    if (reset_orientation)
        clearOrientation();

    // NOTE: exiv crashes for some images (i.e. \exif-crash\0125-results.png)
    // if the thumbnail's max size is > 200px
    setThumbnail(DkImage::createThumb(img, 200));

    return success;
}

bool DkMetaDataT::setExifValue(QString key, QString taginfo)
{
    bool setExifSuccessfull = false;

    if (mExifState == not_loaded || mExifState == no_data)
        return false;

    try {
        if (mExifImg->checkMode(Exiv2::mdExif) != Exiv2::amReadWrite
            && mExifImg->checkMode(Exiv2::mdExif) != Exiv2::amWrite)
            return false;

        Exiv2::ExifData &exifData = mExifImg->exifData();

        if (!exifData.empty() && getExifKeys().contains(key)) {
            Exiv2::Exifdatum &tag = exifData[key.toStdString()];

            // TODO: save utf8 strings
            // QByteArray ba = taginfo.toUtf8();
            // Exiv2::DataValue val((const byte*)ba.data(), taginfo.size(), Exiv2::ByteOrder::bigEndian,
            // Exiv2::TypeId::unsignedByte);

            // tag.setValue(&val);
            if (!tag.setValue(taginfo.toStdString())) {
                mExifState = dirty;
                setExifSuccessfull = true;
            }
        } else {
            Exiv2::ExifKey exivKey(key.toStdString());
            Exiv2::Exifdatum tag(exivKey);
            if (!tag.setValue(taginfo.toStdString())) {
                mExifState = dirty;
                setExifSuccessfull = true;
            }

            exifData.add(tag);
        }
    } catch (...) {
        setExifSuccessfull = false;
    }

    return setExifSuccessfull;
}

QString DkMetaDataT::exiv2ToQString(std::string exifString)
{
    const char *prefix_ascii1 = "charset=\"ASCII\" ";
    const char *prefix_ascii2 = "charset=Ascii ";
    const char *prefix_Unicode = "charset=Unicode ";
    QString info;

    const size_t input_size = exifString.size();

    if (QString::fromStdString(exifString).startsWith(prefix_ascii1, Qt::CaseInsensitive)) {
        const size_t prefix1_size = strlen(prefix_ascii1);
        if (input_size > prefix1_size) {
            info = QString::fromLocal8Bit(exifString.c_str() + prefix1_size, int(input_size - prefix1_size));
        }
    } else if (QString::fromStdString(exifString).startsWith(prefix_ascii2, Qt::CaseInsensitive)) {
        const size_t prefix2_size = strlen(prefix_ascii2);
        if (input_size > prefix2_size) {
            info = QString::fromLocal8Bit(exifString.c_str() + prefix2_size, int(input_size - prefix2_size));
        }
    } else if (QString::fromStdString(exifString).startsWith(prefix_Unicode, Qt::CaseInsensitive)) {
        const size_t prefixunicode_size = strlen(prefix_Unicode);
        if (input_size > prefixunicode_size) {
            info = QString::fromUtf8(exifString.c_str() + prefixunicode_size, int(input_size - prefixunicode_size));
        }
    } else if (input_size > 0) {
        info = QString::fromUtf8(exifString.c_str(), (int)input_size);
    }

    return info;
}

void DkMetaDataT::printMetaData() const
{
    if (mExifState != loaded && mExifState != dirty)
        return;

    const Exiv2::XmpData &xmpData = mExifImg->xmpData();

    qDebug() << "Exif------------------------------------------------------------------";

    QStringList exifKeys = getExifKeys();

    for (int idx = 0; idx < exifKeys.size(); idx++)
        qDebug() << exifKeys.at(idx) << " is " << getNativeExifValue(exifKeys.at(idx), true);

    qDebug() << "IPTC------------------------------------------------------------------";

    QStringList iptcKeys = getIptcKeys();

    for (int idx = 0; idx < iptcKeys.size(); idx++)
        qDebug() << iptcKeys.at(idx) << " is " << getIptcValue(iptcKeys.at(idx));

    qDebug() << "XMP------------------------------------------------------------------";

    const auto endI3 = xmpData.end();
    for (auto md = xmpData.begin(); md != endI3; ++md) {
        std::cout << std::setw(44) << std::setfill(' ') << std::left << md->key() << " " << "0x" << std::setw(4)
                  << std::setfill('0') << std::right << std::hex << md->tag() << " " << std::setw(9)
                  << std::setfill(' ') << std::left << md->typeName() << " " << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right << md->count() << "  " << std::dec << md->value() << std::endl;
    }

    std::string xmpPacket;
    if (0 != Exiv2::XmpParser::encode(xmpPacket, xmpData)) {
        qCritical() << "Failed to serialize XMP data";
    }
    std::cout << xmpPacket << "\n";
}

bool DkMetaDataT::saveRectToXMP(const DkRotatingRect &rect, const QSize &size)
{
    if (mExifState != loaded && mExifState != dirty)
        return false;

    Exiv2::XmpData xmpData = mExifImg->xmpData();

    QRectF r = rect.toExifRect(size);

    double angle = rect.getAngle();
    angle = DkMath::normAngleRad(angle, -CV_PI * 0.25, CV_PI * 0.25) * DK_RAD2DEG;

    // Set the cropping coordinates here in percentage:
    setXMPValue(xmpData, "Xmp.crs.CropTop", QString::number(r.top()));
    setXMPValue(xmpData, "Xmp.crs.CropLeft", QString::number(r.left()));
    setXMPValue(xmpData, "Xmp.crs.CropBottom", QString::number(r.bottom()));
    setXMPValue(xmpData, "Xmp.crs.CropRight", QString::number(r.right()));

    setXMPValue(xmpData, "Xmp.crs.CropAngle", QString::number(angle));

    setXMPValue(xmpData, "Xmp.crs.HasCrop", "True");
    // These key values are set by camera raw automatically, but I have found no documentation for them:
    setXMPValue(xmpData, "Xmp.crs.CropConstrainToWarp", "1");
    setXMPValue(xmpData, "Xmp.crs.crs:AlreadyApplied", "False"); // is this crs.crs: correct??

    // Save the crop coordinates to the sidecar file:
    try {
        mExifImg->setXmpData(xmpData);
        mExifState = dirty;

        qInfo() << r << "written to XMP";

    } catch (...) {
        qWarning() << "[WARNING] I could not set the exif data for this image format...";
    }

    return true;
}

bool DkMetaDataT::clearXMPRect()
{
    if (mExifState != loaded && mExifState != dirty)
        return false;

    try {
        Exiv2::XmpData xmpData = mExifImg->xmpData();
        setXMPValue(xmpData, "Xmp.crs.HasCrop", "False");
        mExifImg->setXmpData(xmpData);
        mExifState = dirty;
    } catch (...) {
        return false;
    }

    return true;
}

DkRotatingRect DkMetaDataT::getXMPRect(const QSize &size) const
{
    if (mExifState != loaded && mExifState != dirty)
        return DkRotatingRect();

    // pretend it's not here if it is already applied
    QString applied = getXmpValue("Xmp.crs.AlreadyApplied");
    QString hasCrop = getXmpValue("Xmp.crs.HasCrop");
    if (applied.compare("true", Qt::CaseInsensitive) == 0 || // compare is 0 if the strings are the same
        hasCrop.compare("true", Qt::CaseInsensitive) != 0)
        return DkRotatingRect();

    Exiv2::XmpData xmpData = mExifImg->xmpData();
    double top = getXmpValue("Xmp.crs.CropTop").toDouble();
    double bottom = getXmpValue("Xmp.crs.CropBottom").toDouble();
    double left = getXmpValue("Xmp.crs.CropLeft").toDouble();
    double right = getXmpValue("Xmp.crs.CropRight").toDouble();

    double angle = getXmpValue("Xmp.crs.CropAngle").toDouble();

    QRectF r(left, top, right - left, bottom - top);
    DkRotatingRect rr = DkRotatingRect::fromExifRect(r, size, angle * DK_DEG2RAD);

    return DkRotatingRect(rr);
}

std::unique_ptr<Exiv2::Image> DkMetaDataT::loadSidecar(const QString &filePath) const
{
    std::unique_ptr<Exiv2::Image> xmpImg;

    // TODO: check if the file type supports xmp

    // Create the path to the XMP file:
    QString dir = filePath;
    QString ext = QFileInfo(filePath).suffix();
    QString xmpPath = dir.left(dir.length() - ext.length() - 1);
    QString xmpExt = ".xmp";
    QString xmpFilePath = xmpPath + xmpExt;

    QFileInfo xmpFileInfo = QFileInfo(xmpFilePath);

    qDebug() << "XMP sidecar path: " << xmpFilePath;

    if (xmpFileInfo.exists()) {
        try {
            xmpImg = Exiv2::ImageFactory::open(xmpFilePath.toStdString());
            xmpImg->readMetadata();
        } catch (...) {
            qWarning() << "Could not read xmp from: " << xmpFilePath;
        }
    }
    if (!xmpImg.get()) {
        // We can only load sidecar files whose paths use std::string literals (no unicode here)
        // Create a new XMP sidecar, unfortunately this one has fewer attributes than the adobe version:
        xmpImg = Exiv2::ImageFactory::create(Exiv2::ImageType::xmp, xmpFilePath.toStdString());

        xmpImg->setMetadata(*mExifImg);
        xmpImg->writeMetadata(); // we need that to add xmp afterwards - but why?
    }

    return xmpImg;
}

bool DkMetaDataT::setXMPValue(Exiv2::XmpData &xmpData, QString xmpKey, QString xmpValue)
{
    bool setXMPValueSuccessful = false;

    // if (!xmpData.empty()) {

    Exiv2::XmpKey key = Exiv2::XmpKey(xmpKey.toStdString());
    auto pos = xmpData.findKey(key);

    // Update the tag if it is set:
    if (pos != xmpData.end() && pos->count() != 0) {
        // sidecarXmpData.erase(pos);
        if (!pos->setValue(xmpValue.toStdString()))
            setXMPValueSuccessful = true;
    } else {
        auto v = Exiv2::Value::create(Exiv2::xmpText);
        if (!v->read(xmpValue.toStdString())) {
            if (!xmpData.add(Exiv2::XmpKey(key), v.get()))
                setXMPValueSuccessful = true;
        }
    }
    //}

    return setXMPValueSuccessful;
}

// DkMetaDataHelper --------------------------------------------------------------------
void DkMetaDataHelper::init()
{
    mCamSearchTags.append("ImageSize");
    mCamSearchTags.append("Orientation");
    mCamSearchTags.append("Make");
    mCamSearchTags.append("Model");
    mCamSearchTags.append("ApertureValue");
    mCamSearchTags.append("ISOSpeedRatings");
    mCamSearchTags.append("Flash");
    mCamSearchTags.append("FocalLength");
    mCamSearchTags.append("ExposureMode");
    mCamSearchTags.append("ExposureTime");
    mCamSearchTags.append("Compression");

    mDescSearchTags.append("Rating");
    mDescSearchTags.append("UserComment");
    mDescSearchTags.append("DateTime");
    mDescSearchTags.append("DateTimeOriginal");
    mDescSearchTags.append("ImageDescription");
    mDescSearchTags.append("Byline");
    mDescSearchTags.append("BylineTitle");
    mDescSearchTags.append("City");
    mDescSearchTags.append("Country");
    mDescSearchTags.append("Headline");
    mDescSearchTags.append("Caption");
    mDescSearchTags.append("CopyRight");
    mDescSearchTags.append("Keywords");
    mDescSearchTags.append("Path");
    mDescSearchTags.append("FileSize");

    for (int i = 0; i < DkSettingsManager::param().translatedCamData().size(); i++)
        mTranslatedCamTags << qApp->translate("nmc::DkMetaData",
                                              DkSettingsManager::param().translatedCamData().at(i).toLatin1());

    for (int i = 0; i < DkSettingsManager::param().translatedDescriptionData().size(); i++)
        mTranslatedDescTags << qApp->translate("nmc::DkMetaData",
                                               DkSettingsManager::param().translatedDescriptionData().at(i).toLatin1());

    mExposureModes.append(QObject::tr("not defined"));
    mExposureModes.append(QObject::tr("manual"));
    mExposureModes.append(QObject::tr("normal"));
    mExposureModes.append(QObject::tr("aperture priority"));
    mExposureModes.append(QObject::tr("shutter priority"));
    mExposureModes.append(QObject::tr("program creative"));
    mExposureModes.append(QObject::tr("high-speed program"));
    mExposureModes.append(QObject::tr("portrait mode"));
    mExposureModes.append(QObject::tr("landscape mode"));

    // flash mapping is taken from: http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/EXIF.html#Flash
    mFlashModes.insert(0x0, QObject::tr("No Flash"));
    mFlashModes.insert(0x1, QObject::tr("Fired"));
    mFlashModes.insert(0x5, QObject::tr("Fired, Return not detected"));
    mFlashModes.insert(0x7, QObject::tr("Fired, Return detected"));
    mFlashModes.insert(0x8, QObject::tr("On, Did not fire"));
    mFlashModes.insert(0x9, QObject::tr("On, Fired"));
    mFlashModes.insert(0xd, QObject::tr("On, Return not detected"));
    mFlashModes.insert(0xf, QObject::tr("On, Return detected"));
    mFlashModes.insert(0x10, QObject::tr("Off, Did not fire"));
    mFlashModes.insert(0x14, QObject::tr("Off, Did not fire, Return not detected"));
    mFlashModes.insert(0x18, QObject::tr("Auto, Did not fire"));
    mFlashModes.insert(0x19, QObject::tr("Auto, Fired"));
    mFlashModes.insert(0x1d, QObject::tr("Auto, Fired, Return not detected"));
    mFlashModes.insert(0x1f, QObject::tr("Auto, Fired, Return detected"));
    mFlashModes.insert(0x20, QObject::tr("No flash function"));
    mFlashModes.insert(0x30, QObject::tr("Off, No flash function"));
    mFlashModes.insert(0x41, QObject::tr("Fired, Red-eye reduction"));
    mFlashModes.insert(0x45, QObject::tr("Fired, Red-eye reduction, Return not detected"));
    mFlashModes.insert(0x47, QObject::tr("Fired, Red-eye reduction, Return detected"));
    mFlashModes.insert(0x49, QObject::tr("On, Red-eye reduction"));
    mFlashModes.insert(0x4d, QObject::tr("On, Red-eye reduction, Return not detected"));
    mFlashModes.insert(0x4f, QObject::tr("On, Red-eye reduction, Return detected"));
    mFlashModes.insert(0x50, QObject::tr("Off, Red-eye reduction"));
    mFlashModes.insert(0x58, QObject::tr("Auto, Did not fire, Red-eye reduction"));
    mFlashModes.insert(0x59, QObject::tr("Auto, Fired, Red-eye reduction"));
    mFlashModes.insert(0x5d, QObject::tr("Auto, Fired, Red-eye reduction, Return not detected"));
    mFlashModes.insert(0x5f, QObject::tr("Auto, Fired, Red-eye reduction, Return detected"));

    // compression mapping taken from: https://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/EXIF.html#Compression
    mCompressionModes.insert(1, QObject::tr("Uncompressed"));
    mCompressionModes.insert(2, QObject::tr("CCITT 1D"));
    mCompressionModes.insert(3, QObject::tr("T4/Group 3 Fax"));
    mCompressionModes.insert(4, QObject::tr("T6/Group 4 Fax"));
    mCompressionModes.insert(5, QObject::tr("LZW"));
    mCompressionModes.insert(6, QObject::tr("JPEG (old-style)"));
    mCompressionModes.insert(7, QObject::tr("JPEG"));
    mCompressionModes.insert(8, QObject::tr("Adobe Deflate"));
    mCompressionModes.insert(9, QObject::tr("JBIG B&W"));
    mCompressionModes.insert(10, QObject::tr("JBIG Color"));
    mCompressionModes.insert(99, QObject::tr("JPEG"));
    mCompressionModes.insert(262, QObject::tr("Kodak 262"));
    mCompressionModes.insert(32766, QObject::tr("Next"));
    mCompressionModes.insert(32767, QObject::tr("Sony ARW Compressed"));
    mCompressionModes.insert(32769, QObject::tr("Packed RAW"));
    mCompressionModes.insert(32770, QObject::tr("Samsung SRW Compressed"));
    mCompressionModes.insert(32771, QObject::tr("CCIRLEW"));
    mCompressionModes.insert(32772, QObject::tr("Samsung SRW Compressed 2"));
    mCompressionModes.insert(32773, QObject::tr("PackBits"));
    mCompressionModes.insert(32809, QObject::tr("Thunderscan"));
    mCompressionModes.insert(32867, QObject::tr("Kodak KDC Compressed"));
    mCompressionModes.insert(32895, QObject::tr("IT8CTPAD"));
    mCompressionModes.insert(32896, QObject::tr("IT8LW"));
    mCompressionModes.insert(32897, QObject::tr("IT8MP"));
    mCompressionModes.insert(32898, QObject::tr("IT8BL"));
    mCompressionModes.insert(32908, QObject::tr("PixarFilm"));
    mCompressionModes.insert(32909, QObject::tr("PixarLog"));
    mCompressionModes.insert(32946, QObject::tr("Deflate"));
    mCompressionModes.insert(32947, QObject::tr("DCS"));
    mCompressionModes.insert(33003, QObject::tr("Aperio JPEG 2000 YCbCr"));
    mCompressionModes.insert(33005, QObject::tr("Aperio JPEG 2000 RGB"));
    mCompressionModes.insert(34661, QObject::tr("JBIG"));
    mCompressionModes.insert(34676, QObject::tr("SGILog"));
    mCompressionModes.insert(34677, QObject::tr("SGILog24"));
    mCompressionModes.insert(34712, QObject::tr("JPEG 2000"));
    mCompressionModes.insert(34713, QObject::tr("Nikon NEF Compressed"));
    mCompressionModes.insert(34715, QObject::tr("JBIG2 TIFF FX"));
    mCompressionModes.insert(34718, QObject::tr("Microsoft Document Imaging(MDI) Binary Level Codec"));
    mCompressionModes.insert(34719, QObject::tr("Microsoft Document Imaging(MDI) Progressive Transform Codec"));
    mCompressionModes.insert(34720, QObject::tr("Microsoft Document Imaging(MDI) Vector"));
    mCompressionModes.insert(34887, QObject::tr("ESRI Lerc"));
    mCompressionModes.insert(34892, QObject::tr("Lossy JPEG"));
    mCompressionModes.insert(34925, QObject::tr("LZMA2"));
    mCompressionModes.insert(34926, QObject::tr("Zstd"));
    mCompressionModes.insert(34927, QObject::tr("WebP"));
    mCompressionModes.insert(34933, QObject::tr("PNG"));
    mCompressionModes.insert(34934, QObject::tr("JPEG XR"));
    mCompressionModes.insert(65000, QObject::tr("Kodak DCR Compressed"));
    mCompressionModes.insert(65535, QObject::tr("Pentax PEF Compressed"));
}

QString DkMetaDataHelper::getApertureValue(QSharedPointer<DkMetaDataT> metaData) const
{
    QString key = mCamSearchTags.at(key_aperture);

    QString value = metaData->getExifValue(key);
    QStringList sList = value.split('/');

    if (sList.size() == 2) {
        double val = pow(1.4142,
                         sList[0].toDouble()
                             / sList[1].toDouble()); // see the exif documentation (e.g.
                                                     // http://www.media.mit.edu/pia/Research/deepview/exif.html)
        value = QString::fromStdString(DkUtils::stringify(val, 1));
    }

    // just divide the fnumber
    if (value.isEmpty()) {
        value = metaData->getExifValue("FNumber"); // try alternative tag
        value = DkUtils::resolveFraction(value);
    }

    return value;
}

QString DkMetaDataHelper::getFocalLength(QSharedPointer<DkMetaDataT> metaData) const
{
    // focal length
    QString key = mCamSearchTags.at(key_focal_length);

    QString value = metaData->getExifValue(key);

    float v = convertRational(value);

    if (v != -1)
        value = QString::number(v) + " mm";

    return value;
}

QString DkMetaDataHelper::getExposureTime(QSharedPointer<DkMetaDataT> metaData) const
{
    QString key = mCamSearchTags.at(key_exposure_time);
    QString value = metaData->getExifValue(key);
    QStringList sList = value.split('/');

    if (sList.size() == 2) {
        int nom = sList[0].toInt(); // nominator
        int denom = sList[1].toInt(); // denominator

        // if exposure time is less than a second -> normalize to get nice values (1/500 instead of 2/1000)
        if (nom <= denom && nom != 0) {
            // fixes #496
            double nd = (double)denom / nom;
            value = QString("1/") + QString::number(qRound(nd));
        } else
            value = QString::fromStdString(DkUtils::stringify((float)nom / (float)denom, 1));

        value += " sec";
    }

    return value;
}

QString DkMetaDataHelper::getExposureMode(QSharedPointer<DkMetaDataT> metaData) const
{
    QString key = mCamSearchTags.at(key_exposure_mode);
    QString value = metaData->getExifValue(key);
    int mode = value.toInt();

    if (mode >= 0 && mode < mExposureModes.size())
        value = mExposureModes[mode];

    return value;
}

QString DkMetaDataHelper::getFlashMode(QSharedPointer<DkMetaDataT> metaData) const
{
    QString key = mCamSearchTags.at(key_flash);
    QString value = metaData->getExifValue(key);
    unsigned int mode = value.toUInt();

    if (mode < (unsigned int)mFlashModes.size())
        value = mFlashModes[mode];
    else {
        value = mFlashModes.first(); // assuming no flash to be first
        qWarning() << "illegal flash mode dected: " << mode;
    }

    return value;
}

QString DkMetaDataHelper::getCompression(QSharedPointer<DkMetaDataT> metaData) const
{
    int cmpKey = metaData->getExifValue(mCamSearchTags[key_compression]).toInt();
    QString value = mCompressionModes.value(cmpKey, "");

    // show raw data if we can't map it
    if (value.isEmpty())
        value = QString::number(cmpKey);

    return value;
}

QString DkMetaDataHelper::getGpsAltitude(const QString &val) const
{
    QString rVal = val;
    float v = convertRational(val);

    if (v != -1)
        rVal = QString::number(v) + " m";

    return rVal;
}

QString DkMetaDataHelper::getGpsCoordinates(QSharedPointer<DkMetaDataT> metaData) const
{
    QString Lat, LatRef, Lon, LonRef, gpsInfo;
    QStringList help;

    try {
        if (metaData->hasMetaData()) {
            // metaData = DkImageLoader::imgMetaData;
            Lat = metaData->getNativeExifValue("Exif.GPSInfo.GPSLatitude", false);
            LatRef = metaData->getNativeExifValue("Exif.GPSInfo.GPSLatitudeRef", false);
            Lon = metaData->getNativeExifValue("Exif.GPSInfo.GPSLongitude", false);
            LonRef = metaData->getNativeExifValue("Exif.GPSInfo.GPSLongitudeRef", false);
            // example url
            // http://maps.google.com/maps?q=N48+8'+31.940001''+E16+15'+35.009998''

            gpsInfo = "https://maps.google.com/maps?q=";

            QString latStr = convertGpsCoordinates(Lat).join("+");
            QString lonStr = convertGpsCoordinates(Lon).join("+");
            if (latStr.isEmpty() || lonStr.isEmpty())
                return "";
            gpsInfo += "+" + LatRef + "+" + latStr;
            gpsInfo += "+" + LonRef + "+" + lonStr;
        }

    } catch (...) {
        gpsInfo = "";
        // qDebug() << "could not load Exif GPS information";
    }

    return gpsInfo;
}

QStringList DkMetaDataHelper::convertGpsCoordinates(const QString &coordString) const
{
    QStringList gpsInfo;
    QStringList entries = coordString.split(" ");

    for (int i = 0; i < entries.size(); i++) {
        float val1, val2;
        QString valS;
        QStringList coordP;

        valS = entries.at(i);
        coordP = valS.split("/");
        if (coordP.size() != 2)
            return QStringList();

        val1 = coordP.at(0).toFloat();
        val2 = coordP.at(1).toFloat();
        val1 = val2 != 0 ? val1 / val2 : val1;

        if (i == 0) {
            valS.setNum((int)val1);
            gpsInfo.append(valS + dk_degree_str);
        }
        if (i == 1) {
            if (val2 > 1)
                valS.setNum(val1, 'f', 6);
            else
                valS.setNum((int)val1);
            gpsInfo.append(valS + "'");
        }
        if (i == 2) {
            if (val1 != 0) {
                valS.setNum(val1, 'f', 6);
                gpsInfo.append(valS + "''");
            }
        }
    }

    return gpsInfo;
}

float DkMetaDataHelper::convertRational(const QString &val) const
{
    float rVal = -1;
    QStringList sList = val.split('/');

    if (sList.size() == 2) {
        bool ok1 = false;
        bool ok2 = false;

        rVal = sList[0].toFloat(&ok1) / sList[1].toFloat(&ok2);

        if (!ok1 || !ok2)
            rVal = -1;
    }

    return rVal;
}

QString DkMetaDataHelper::translateKey(const QString &key) const
{
    QString translatedKey = key;

    int keyIdx = mCamSearchTags.indexOf(key);
    if (keyIdx != -1)
        translatedKey = mTranslatedCamTags.at(keyIdx);

    keyIdx = mDescSearchTags.indexOf(key);
    if (keyIdx != -1)
        translatedKey = mTranslatedDescTags.at(keyIdx);

    return translatedKey;
}

QString DkMetaDataHelper::resolveSpecialValue(QSharedPointer<DkMetaDataT> metaData,
                                              const QString &key,
                                              const QString &value) const
{
    QString rValue = value;

    if (key == mCamSearchTags[key_aperture] || key == "FNumber") {
        rValue = getApertureValue(metaData);
    } else if (key == mCamSearchTags[key_focal_length]) {
        rValue = getFocalLength(metaData);
    } else if (key == mCamSearchTags[key_exposure_time]) {
        rValue = getExposureTime(metaData);
    } else if (key == mCamSearchTags[key_exposure_mode]) {
        rValue = getExposureMode(metaData);
    } else if (key == mCamSearchTags[key_flash]) {
        rValue = getFlashMode(metaData);
    } else if (key == mCamSearchTags[key_compression]) {
        rValue = getCompression(metaData);
    } else if (key == "GPSLatitude" || key == "GPSLongitude") {
        rValue = convertGpsCoordinates(value).join(" ");
    } else if (key == "GPSAltitude") {
        rValue = getGpsAltitude(value);
    } else if (value.contains("charset=")) {
        if (value.contains("charset=\"unicode\"", Qt::CaseInsensitive)) {
            rValue = rValue.replace("charset=\"unicode\" ", "", Qt::CaseInsensitive);

            //// try to set the BOM ourselves (Note the string would not be released yet
            // ushort* utfStr = new ushort[rValue.size()+2];
            // utfStr[0] = 0xFF;
            // utfStr[1] = 0xFE;
            // utfStr = utfStr+2;
            //
            // utfStr = (ushort*)(rValue.data());

            qDebug() << "UNICODE conversion started...";
            // TODO: does this actually have any effect?
            // QStrings should already be UTF-16
            rValue = QString::fromUtf16((char16_t *)(rValue.data()), rValue.size());
        }
    } else {
        rValue = DkUtils::resolveFraction(rValue); // resolve fractions
    }

    return rValue;
}

bool DkMetaDataHelper::hasGPS(QSharedPointer<DkMetaDataT> metaData) const
{
    return !getGpsCoordinates(metaData).isEmpty();
}

QStringList DkMetaDataHelper::getCamSearchTags() const
{
    return mCamSearchTags;
}

QStringList DkMetaDataHelper::getDescSearchTags() const
{
    return mDescSearchTags;
}

QStringList DkMetaDataHelper::getTranslatedCamTags() const
{
    return mTranslatedCamTags;
}

QStringList DkMetaDataHelper::getTranslatedDescTags() const
{
    return mTranslatedDescTags;
}

QStringList DkMetaDataHelper::getAllExposureModes() const
{
    return mExposureModes;
}

QMap<int, QString> DkMetaDataHelper::getAllFlashModes() const
{
    return mFlashModes;
}

// make XmpParser thread-save and enable support for ISO BMFF formats (AVIF, HEIF, JXL)
// see http://exiv2.org/doc/classExiv2_1_1XmpParser.html#aea661a7039adb5a748eb7639c8ce9294
void DkMetaDataHelper::initialize()
{
    DkTimer dt;

    // struct XmpLock {
    //	CRITICAL_SECTION cs;
    //	XmpLock() { InitializeCriticalSection(&cs); }
    //	~XmpLock() { DeleteCriticalSection(&cs); }
    //	static void LockUnlock(void* pData, bool fLock) {
    //		XmpLock* pThis = reinterpret_cast<XmpLock*>(pData);
    //		if (pThis) {

    //			if (fLock) {
    //				EnterCriticalSection(&pThis->cs);
    //				qDebug() << "locking -------------------------";
    //			}
    //			else {
    //				LeaveCriticalSection(&pThis->cs);
    //				qDebug() << "unlocking ------------------------";
    //			}
    //		}
    //	}
    //} xmpLock;

    // Exiv2::XmpParser::initialize(XmpLock::LockUnlock, &xmpLock);
    Exiv2::XmpParser::initialize();
    qDebug() << "initializing the xmp parser takes" << dt;

#ifdef EXV_ENABLE_BMFF
    if (Exiv2::enableBMFF(true)) {
        qInfo() << "Metadata support for BMFF formats is active.";
    } else {
        qInfo() << "Exiv2 was built without metadata support for BMFF formats.";
    }
#else
    qInfo() << "Metadata support for AVIF, HEIF and JPEG XL formats is not available.";
#endif
}

}
