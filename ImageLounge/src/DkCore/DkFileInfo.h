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

#include <QFileInfo>

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

namespace nmc
{

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
     * Returns the file list of the directory dir.
     * Note: this function might get slow if lots of files (> 10000) are in the
     * directory or if the directory is in the net.
     * Currently the file list is sorted according to the system specification.
     * @param dir the directory to load the file list from.
     * @return QStringList all filtered files of the current directory.
     **/
    static QFileInfoList readDirectory(const QString &dirPath, QString folderKeywords = {});

    // todo: clean path before giving fileInfo
    // remove trailing '/' : breaks fileName()
    //
    DkFileInfo(const QString &path = "");

    // provide type conversions for porting; prints a warning if there might be a problem
    explicit DkFileInfo(const QFileInfo &info);
    operator QFileInfo() const;

    // these will be made private when they are no longer necessary
#ifdef WITH_QUAZIP
    bool isFromZip() const;
    bool isZipFile() const;
#else
    bool isFromZip() const
    {
        return false;
    }

    bool isZipFile() const
    {
        return false;
    }
#endif

    // return i/o open for reading
    QSharedPointer<QIODevice> getIoDevice() const;

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

    QDateTime lastModified() const;
    bool permission(QFile::Permissions flags) const;

    bool isShortcut() const;
    bool resolveShortcut();

    qint64 size() const;

private:
    // info of the container, if it is a local file, otherwise invalid
    const QFileInfo &containerInfo() const;

#ifdef WITH_QUAZIP

    static QFileInfoList readZipArchive(const QString &zipPath);

    class ZipData
    {
    public:
        ZipData(const QString &fileName);

        bool isZip() const;
        QString getZipFilePath() const;
        QString getImageFileName() const;
        static QString encodeZipFile(const QString &zipFile, const QString &imageFile);

    private:
        static QString zipMarker();
        static QString decodeZipFile(const QString &encodedFileInfo);
        static QString decodeImageFile(const QString &encodedFileInfo);

        static QString mZipMarker;

        QString mEncodedFilePath;
        QString mZipFilePath;
        QString mImageFileName;
        bool mImageInZip;
    };
#endif

    class SharedData : public QSharedData
    {
        friend class DkFileInfo;

    private:
        SharedData(const QFileInfo &info);
        QFileInfo mFileInfo; // only valid if path() is an ordinary file
        QFileInfo mContainerInfo; // only valid if container is an ordinary file
#if WITH_QUAZIP
        ZipData mZipData;
#endif
    };

    QSharedDataPointer<SharedData> d;
};

typedef QList<DkFileInfo> DkFileInfoList;

}
