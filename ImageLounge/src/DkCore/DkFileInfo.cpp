/*******************************************************************************************************
 DkFileInfo.cpp
 Created on: 04.28.2025

nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

Copyright (C) 2025 Scrubs <scrubbbbs@gmail.com>

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
#include "DkFileInfo.h"

#ifdef WITH_QUAZIP
#include "DkBasicLoader.h"

#ifdef WITH_QUAZIP1
#include <quazip/JlCompress.h>
#else
#include <quazip5/JlCompress.h>
#endif
#endif

namespace nmc
{

DkFileInfo::DkFileInfo(const QString &path)
{
    d = new PrivateData(path);
    setFile(path);
}

DkFileInfo::DkFileInfo(const QFileInfo &info)
{
    d = new PrivateData(info.absoluteFilePath());
    d->mFileInfo = info;
}

DkFileInfo::operator QFileInfo() const
{
#if WITH_QUAZIP
    if (isFromZip())
        qWarning() << "[FileInfo] cast to QFileInfo breaks zip files";
#endif
    return d->mFileInfo;
}

#ifdef WITH_QUAZIP
bool DkFileInfo::isFromZip() const
{
    return d->mZipData.isZip();
}

bool DkFileInfo::isZipFile() const
{
    return d->mFileInfo.isFile() && DkBasicLoader::isContainer(path());
}
#endif

QSharedPointer<QIODevice> DkFileInfo::getIoDevice() const
{
    QSharedPointer<QIODevice> io;

#ifdef WITH_QUAZIP
    if (isFromZip())
        io.reset(new QuaZipFile(d->mZipData.getZipFilePath(), d->mZipData.getImageFileName()));
#endif

    if (!io)
        io.reset(new QFile(path()));

    if (!io->open(QIODevice::ReadOnly)) {
        qWarning() << "[FileInfo] failed to open i/o" << path() << io->errorString();
        io.clear();
    }

    return io;
}

QString DkFileInfo::path() const
{
    return d->mFileInfo.absoluteFilePath();
}

QString DkFileInfo::dirPath() const
{
    if (isFromZip())
        return containerInfo().absoluteFilePath();
    if (isZipFile())
        return path();
    return d->mFileInfo.absolutePath();
}

void DkFileInfo::refresh()
{
    if (isFromZip())
        d->mContainerInfo.refresh();
    else
        d->mFileInfo.refresh();
}

bool DkFileInfo::exists() const
{
    return containerInfo().exists();
}

bool DkFileInfo::isFile() const
{
    return !isDir();
}

bool DkFileInfo::isDir() const
{
    return d->mFileInfo.isDir() || isZipFile();
}

bool DkFileInfo::isReadable() const
{
    return containerInfo().isReadable();
}

QString DkFileInfo::fileName() const
{
    return d->mFileInfo.fileName();
}

QString DkFileInfo::suffix() const
{
    return d->mFileInfo.suffix();
}

QDateTime DkFileInfo::lastModified() const
{
    // TODO: use zip file internal modification date
    return containerInfo().lastModified();
}

bool DkFileInfo::permission(QFileDevice::Permissions flags) const
{
    return containerInfo().permission(flags);
}

bool DkFileInfo::isSymLink() const
{
    return isFromZip() ? false : d->mFileInfo.isSymLink();
}

QString DkFileInfo::symLinkTarget() const
{
    if (isFromZip()) {
        qWarning() << "[FileInfo] ignoring zip symlink resolution";
        return path();
    }
    return d->mFileInfo.symLinkTarget();
}

qint64 DkFileInfo::size() const
{
    if (isFromZip()) {
#ifdef WITH_QUAZIP
        return d->mZipData.getZipFileSize();
#endif
    }
    return d->mFileInfo.size();
}

void DkFileInfo::setFile(const QString &newPath)
{
    d->mFileInfo.setFile(newPath);
#ifdef WITH_QUAZIP
    if (isFromZip())
        d->mContainerInfo.setFile(d->mZipData.getZipFilePath());
#endif
}

const QFileInfo &DkFileInfo::containerInfo() const
{
    if (isFromZip())
        return d->mContainerInfo;
    return d->mFileInfo;
}
}
