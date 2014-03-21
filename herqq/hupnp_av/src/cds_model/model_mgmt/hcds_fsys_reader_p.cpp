/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hcds_fsys_reader_p.h"
#include "hcdsobjectdata_p.h"

#include "../datasource/hrootdir.h"

#include "../cds_objects/hphoto.h"
#include "../cds_objects/htextitem.h"
#include "../cds_objects/hvideoitem.h"
#include "../cds_objects/hmusictrack.h"
#include "../cds_objects/hstoragefolder.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QFileInfo>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

namespace
{

HItem* createMusicTrack(const QFileInfo& fileInfo, const QString& parentId)
{
    return new HMusicTrack(fileInfo.fileName(), parentId);
}

HItem* createPhotoItem(const QFileInfo& fileInfo, const QString& parentId)
{
    return new HPhoto(fileInfo.fileName(), parentId);
}

HItem* createVideoItem(const QFileInfo& fileInfo, const QString& parentId)
{
    return new HVideoItem(fileInfo.fileName(), parentId);
}

HItem* createTextItem(const QFileInfo& fileInfo, const QString& parentId)
{
    return new HTextItem(fileInfo.fileName(), parentId);
}

typedef HItem* (*HItemCreator)(const QFileInfo& fileInfo, const QString& parentId);

typedef QPair<const char*, HItemCreator> MimeAndItemCreator;

QHash<QString, MimeAndItemCreator> initializeCreatorFunctions()
{
    QHash<QString, MimeAndItemCreator> retVal;

    retVal["mp3"] = MimeAndItemCreator("audio/mpeg", createMusicTrack);
    retVal["wav"] = MimeAndItemCreator("audio/x-wav", createMusicTrack);
    retVal["ogg"] = MimeAndItemCreator("audio/ogg", createMusicTrack);
    retVal["oga"] = MimeAndItemCreator("audio/ogg", createMusicTrack);
    retVal["flac"] = MimeAndItemCreator("audio/flac", createMusicTrack);

    retVal["mp2"] = MimeAndItemCreator("video/mpeg", createVideoItem);
    retVal["mp4"] = MimeAndItemCreator("video/mpeg", createVideoItem);
    retVal["mpa"] = MimeAndItemCreator("video/mpeg", createVideoItem);
    retVal["mpe"] = MimeAndItemCreator("video/mpeg", createVideoItem);
    retVal["mpeg"] = MimeAndItemCreator("video/mpeg", createVideoItem);
    retVal["wmv"] = MimeAndItemCreator("video/mpeg", createVideoItem);
    retVal["mpg"] = MimeAndItemCreator("video/mpeg", createVideoItem);
    retVal["mov"] = MimeAndItemCreator("video/quicktime", createVideoItem);
    retVal["qt"] = MimeAndItemCreator("video/quicktime", createVideoItem);
    retVal["avi"] = MimeAndItemCreator("video/x-msvideo", createVideoItem);
    retVal["asf"] = MimeAndItemCreator("video/x-ms-asf", createVideoItem);
    retVal["asr"] = MimeAndItemCreator("video/x-ms-asf", createVideoItem);

    retVal["bmp"] = MimeAndItemCreator("image/bmp", createPhotoItem);
    retVal["gif"] = MimeAndItemCreator("image/gif", createPhotoItem);
    retVal["jpe"] = MimeAndItemCreator("image/jpeg", createPhotoItem);
    retVal["jpg"] = MimeAndItemCreator("image/jpeg", createPhotoItem);
    retVal["jpeg"] = MimeAndItemCreator("image/jpeg", createPhotoItem);
    retVal["png"] = MimeAndItemCreator("image/png", createPhotoItem);
    retVal["tif"] = MimeAndItemCreator("image/tiff", createPhotoItem);
    retVal["tiff"] = MimeAndItemCreator("image/tiff", createPhotoItem);

    retVal["css"] = MimeAndItemCreator("text/css", createTextItem);
    retVal["html"] = MimeAndItemCreator("text/htm", createTextItem);
    retVal["html"] = MimeAndItemCreator("text/html", createTextItem);
    retVal["cs"] = MimeAndItemCreator("text/plain", createTextItem);
    retVal["hpp"] = MimeAndItemCreator("text/plain", createTextItem);
    retVal["cpp"] = MimeAndItemCreator("text/plain", createTextItem);
    retVal["c"] = MimeAndItemCreator("text/plain", createTextItem);
    retVal["h"] = MimeAndItemCreator("text/plain", createTextItem);
    retVal["txt"] = MimeAndItemCreator("text/plain", createTextItem);
    retVal["xml"] = MimeAndItemCreator("text/xml", createTextItem);

    return retVal;
}

QHash<QString, MimeAndItemCreator> creatorFunctions = initializeCreatorFunctions();

}

/*******************************************************************************
 * HCdsFileSystemReaderPrivate
 ******************************************************************************/
class HCdsFileSystemReaderPrivate
{
friend class HCdsFileSystemReader;

private:

    HCdsObjectData* indexFile(
        const QFileInfo& fileInfo, const QString& parentId);

    HContainer* indexDir (
        const QDir& dir, const QString& parentId, const QString& id,
        const QList<HObject*>& children);

    HCdsFileSystemReaderPrivate();
    virtual ~HCdsFileSystemReaderPrivate();

    HCdsObjectData* scan(
        const HRootDir& dir, const QString& parentId, QList<HCdsObjectData*>*);
};

HCdsFileSystemReaderPrivate::HCdsFileSystemReaderPrivate()
{
}

HCdsFileSystemReaderPrivate::~HCdsFileSystemReaderPrivate()
{
}

HCdsObjectData* HCdsFileSystemReaderPrivate::indexFile(
    const QFileInfo& file, const QString& parentId)
{
    HLOG(H_AT, H_FUN);

    QString sufx = file.suffix().toLower();

    MimeAndItemCreator creator = creatorFunctions[sufx];
    if (!creator.second)
    {
        HLOG_WARN(QString("File type [%1] is not supported.").arg(sufx));
        return 0;
    }

    HItem* item = creator.second(file, parentId);
    Q_ASSERT(item);
    item->setContentFormat(creator.first);

    return new HCdsObjectData(item, file.absoluteFilePath());
}

HCdsObjectData* HCdsFileSystemReaderPrivate::scan(
    const HRootDir& rdir, const QString& parentId, QList<HCdsObjectData*>* result)
{
    HLOG(H_AT, H_FUN);

    QDir dir = rdir.dir();
    HLOG_DBG(QString("Entering directory %1").arg(dir.absolutePath()));

    HStorageFolder* folder = new HStorageFolder(dir.dirName(), parentId);
    QString id = folder->id();

    HCdsObjectData* item = new HCdsObjectData(folder, dir.absolutePath());
    result->append(item);

    QSet<QString> childIds;
    QFileInfoList infoList =
        dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);

    for(qint32 i = 0; i < infoList.size(); ++i)
    {
        QFileInfo finfo = infoList[i];
        if (finfo.isDir() && rdir.scanMode() == HRootDir::RecursiveScan)
        {
            if (QDir(finfo.absoluteFilePath()) == dir)
            {
                continue;
            }

            HRootDir subdir(QDir(finfo.absoluteFilePath()), HRootDir::RecursiveScan);

            HCdsObjectData* child = scan(subdir, id, result);
            Q_ASSERT(child);

            childIds.insert(child->object()->id());

            continue;
        }

        HCdsObjectData* child = indexFile(finfo, id);
        if (child)
        {
            result->append(child);
            childIds.insert(child->object()->id());
        }
    }

    folder->setChildIds(childIds);
    return item;
}

/*******************************************************************************
 * HCdsFileSystemReader
 ******************************************************************************/
HCdsFileSystemReader::HCdsFileSystemReader() :
    h_ptr(new HCdsFileSystemReaderPrivate())
{
}

HCdsFileSystemReader::~HCdsFileSystemReader()
{
    delete h_ptr;
}

bool HCdsFileSystemReader::scan(
    const HRootDir& rootDir, const QString& parentId,
    QList<HCdsObjectData*>* result)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(result);

    if (!rootDir.dir().exists())
    {
        return false;
    }

    QList<HCdsObjectData*> tmp;
    h_ptr->scan(rootDir, parentId, &tmp);
    result->append(tmp);

    return true;
}

QString HCdsFileSystemReader::deduceMimeType(const QString& filename)
{
    QString fileSuffix = filename.mid(filename.lastIndexOf('.')+1).toLower();

    MimeAndItemCreator creator = creatorFunctions[fileSuffix];
    if (!creator.second)
    {
        return "";
    }

    return creator.first;
}

HItem* HCdsFileSystemReader::createItem(
    const QString& filename, const QString& parentId)
{
    QString fileSuffix = filename.mid(filename.lastIndexOf('.')+1).toLower();

    MimeAndItemCreator creator = creatorFunctions[fileSuffix];
    if (!creator.second)
    {
        return 0;
    }

    return creator.second(QFileInfo(filename), parentId);
}

}
}
}
