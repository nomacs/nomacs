/*******************************************************************************************************
 DkFileInfo.h
 Created on: 04.06.2024

nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

Copyright (C) 2024 Scrubbbbs <esceruebbebbes@gemeaeiel.com =~ s/e//g>

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

#include <QDateTime>
#include <QFileInfo>

#include <memory>

#include "nmc_config.h"

struct QuaZipFileInfo64;

namespace nmc
{

class DkFileInfo;
typedef QList<DkFileInfo> DkFileInfoList;

/**
 * Replaces QFileInfo to support VFS and normalize filesystem calls throughout
 * nomacs.
 *
 * The API should match QFileInfo semantics for the most part.
 *
 * Porting notes:
 *   isFile/isDir imply exists() so don't combine them
 *
 *   absoluteFilePath() => path() -- we always use absolute file paths in nomacs
 *   absolutePath() => dirPath() -- ditto
 *
 *   QDir().isDir => DkFileInfo().isDir()
 *   QDir().exists() => DkFileInfo().isDir()
 *
 */
class DllCoreExport DkFileInfo
{
public:
    /**
     * Returns a file list of supported file types, non-recursive.
     * @param dirPath directory or container path
     * @param nameFilter additional filter on file name
     * @return list of presumed supported files in no particular order
     *
     * @note checks the header of files with no suffix, which may get slow
     *       for large directories.
     **/
    static DkFileInfoList readDirectory(const QString &dirPath, const QString &nameFilter = {});

    // fast check if file has supported suffix
    static bool isContainer(const QFileInfo &fileInfo);

    // construct from vfs-supported path; allow type conversions here
    DkFileInfo(const QString &path = QString()); // NOLINT(google-explicit-constructor)

    // construct from ordinary file info (non-zip, non-vfs etc)
    explicit DkFileInfo(const QFileInfo &info);

    bool operator==(const DkFileInfo &other) const;

    // avoid these if possible as they break the abstraction
#ifdef WITH_QUAZIP
    bool isFromZip() const;
    bool isZipFile() const;
    QString pathInZip() const;
#else
    // clang-format off
    bool isFromZip() const { return false; }
    bool isZipFile() const { return false; }
    QString pathInZip() const { return {}; }
    // clang-format on
#endif

    // return i/o open for reading, or nullptr
    std::unique_ptr<QIODevice> getIODevice() const;

    // absolute file path (like QFileInfo::absoluteFilePath())
    // this is also the uuid of file; e.g. DkFileInfo(info.path()) == info;
    // for VFS this will not be an ordinary path, so avoid QFileInfo(dkInfo.path())
    QString path() const;

    // absolute directory path not including entry itself (like QFileInfo::absolutePath())
    // note: returns parent directory path for directories
    QString dirPath() const;

    // check for file or metadata modification and refresh internal state
    // return true if no longer exists or modified
    bool isModified();

    // switch to the link target if isSymLink() is true
    // return false if target does not exist
    bool resolveSymLink();

    // ---- wrapped methods of QFileInfo --------------------------------
    // where noted changes semantics of QFileInfo

    bool exists() const;
    bool isFile() const;
    bool isDir() const; // also true for containers

    bool isReadable() const;

    QString fileName() const;
    QString suffix() const;
    QString baseName() const;

    QDateTime birthTime() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const; // from container, not member

    QString owner() const; // owner/group/perm from container, not member
    uint ownerId() const;
    QString group() const;
    QFileDevice::Permissions permissions() const;
    bool permission(QFile::Permissions flags) const;

    bool isSymLink() const;
    QString symLinkTarget() const;

    qint64 size() const;

    void stat();

private:
    // info of the container if isFromZip(), otherwise info of file/dir,
    const QFileInfo &containerInfo() const;

#ifdef WITH_QUAZIP
    void readZipMetaData() const;
#else
    void readZipMetaData() const {};
#endif

    static DkFileInfoList readZipArchive(const QString &zipPath);

    class ZipData
    {
    public:
        explicit ZipData(const QString &encodedFilePath);
        ZipData(const QString &zipFile, const QuaZipFileInfo64 &info);

        static QString encodePath(const QString &zipFilePath, const QString &memberPath);

        // clang-format off
#ifdef WITH_QUAZIP
        bool isZipMember() const { return mIsMember; }
#else
        bool isZipMember() const { return false; }
#endif
        QString zipFilePath() const    { return mZipFilePath; }
        QString zipMemberPath() const  { return mZipMemberPath; }
        qint64 size() const            { return mDecompressedSize; }
        QDateTime lastModified() const { return mModified; }
        QDateTime birthTime() const    { return mCreated; }
        bool hasMetaData() const       { return mHasMetaData; }
        void zipFileModified()         { mHasMetaData = false; }
        // clang-format on

        void readMetaData();

    private:
        void setMetaData(const QuaZipFileInfo64 &info);

        QString mZipFilePath;
        QString mZipMemberPath;

        QDateTime mModified;
        QDateTime mCreated;

        qint64 mDecompressedSize = 0;
        bool mIsMember = false;
        bool mHasMetaData = false;
    };

    class SharedData : public QSharedData
    {
        friend class DkFileInfo;

    private:
        explicit SharedData(const QString &path);
        explicit SharedData(const QFileInfo &info);
        SharedData(const QString &zipPath, const QuaZipFileInfo64 &info);

        QFileInfo mFileInfo; // isFromZip() => constructed from encodedPath()-formatted path
#ifdef WITH_QUAZIP
        ZipData mZipData;
        QFileInfo mContainerInfo; // isFromZip() => info of the .zip file itself
#endif
    };

    QSharedDataPointer<SharedData> d;

    explicit DkFileInfo(DkFileInfo::SharedData *shared);
};
}
