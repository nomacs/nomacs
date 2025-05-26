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

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

class QuaZipFileInfo64;

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
 * porting notes:
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

    // todo: clean path before giving fileInfo
    // remove trailing '/' : breaks fileName()
    //
    DkFileInfo(const QString &path = "");

    // provide type conversions for porting; prints a warning if there might be a problem
    explicit DkFileInfo(const QFileInfo &info);
    operator QFileInfo() const;

    bool operator==(const DkFileInfo &other) const;

    // these will be made private when they are no longer necessary
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

    // absolute file path
    // this is also the uuid of file; e.g. DkFileInfo(info.path()) == info;
    // for VFS this will not be an ordinary path, so avoid QFileInfo(dkInfo.path())
    QString path() const;

    // absolute directory path
    // if it's a file, return parent
    // if it's a dir, return the dir (container files are considered a dir)
    QString dirPath() const;

    // todo: move certain DkUtils methods here
    // static bool isValid(const QFileInfo &fileInfo);
    // static bool isSavable(const QString &fileName);
    // static bool hasValidSuffix(const QString &fileName);

    // ---- QFileInfo methods --------------------------------
    // where possible keep the semantics of QFileInfo

    void refresh();

    bool exists() const;
    bool isFile() const;
    bool isDir() const;

    bool isReadable() const;

    QString fileName() const;
    QString suffix() const;
    QString baseName() const;

    QDateTime birthTime() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;

    QString owner() const;
    uint ownerId() const;
    QString group() const;
    QFileDevice::Permissions permissions() const;

    bool permission(QFile::Permissions flags) const;

    bool isShortcut() const;
    bool resolveShortcut();

    bool isSymLink() const;
    QString symLinkTarget() const;

    qint64 size() const;

private:
    // info of the container, if it is a local file, otherwise invalid
    const QFileInfo &containerInfo() const;

    static DkFileInfoList readZipArchive(const QString &zipPath);

    class ZipData
    {
    public:
        ZipData(const QString &encodedFilePath);
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
        // clang-format on

    private:
        void setMetaData(const QuaZipFileInfo64 &info);
        void readMetaData();

        QString mZipFilePath;
        QString mZipMemberPath;

        QDateTime mModified;
        QDateTime mCreated;

        qint64 mDecompressedSize = 0;
        bool mIsMember = false;
        bool mIsCached = false;
    };

    class SharedData : public QSharedData
    {
        friend class DkFileInfo;

    private:
        SharedData(const QString &path);
        SharedData(const QFileInfo &info);
        SharedData(const QString &zipPath, const QuaZipFileInfo64 &info);

        QFileInfo mFileInfo; // zip: constructed from encoded path in the zipfile
#ifdef WITH_QUAZIP
        const ZipData mZipData;
        QFileInfo mContainerInfo; // zip: info of the .zip file itself
#endif
    };

    QSharedDataPointer<SharedData> d;

    DkFileInfo(DkFileInfo::SharedData *shared);
};
}
