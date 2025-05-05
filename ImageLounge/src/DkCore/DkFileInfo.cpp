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
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#ifdef WITH_QUAZIP
#include "DkBasicLoader.h" // TODO: move isContainer() here
#include <quazip/JlCompress.h>
#endif

#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace nmc
{

#ifdef WITH_QUAZIP

// this is the same marker as KIO
const QString DkFileInfo::ZipData::mZipMarker = "#/";

DkFileInfo::ZipData::ZipData(const QString &encodedFilePath)
{
    qsizetype index = encodedFilePath.indexOf(mZipMarker);
    if (index > 0) {
        mIsMember = true;
        mZipFilePath = encodedFilePath.mid(0, index);
        mZipMemberPath = encodedFilePath.mid(index + mZipMarker.length());
    }
}

DkFileInfo::ZipData::ZipData(const QString &zipFile, const QuaZipFileInfo64 &info)
{
    mIsMember = true;
    mZipFilePath = zipFile;
    mZipMemberPath = info.name;
    mDecompressedSize = info.uncompressedSize;
    mModified = info.dateTime;
    mCreated = info.getExtTime(info.extra, 4);
    if (!mCreated.isValid())
        mCreated = info.getNTFScTime();
    mAccessed = info.getExtTime(info.extra, 2);
    if (!mAccessed.isValid())
        mAccessed = info.getNTFSaTime();
}

QString DkFileInfo::ZipData::encodePath(const QString &zipFilePath, const QString &memberPath)
{
    Q_ASSERT(!memberPath.startsWith('/'));
    Q_ASSERT(QFileInfo(zipFilePath).absoluteFilePath() == zipFilePath);

    return zipFilePath + mZipMarker + memberPath;
}

#endif

DkFileInfo::SharedData::SharedData(const QFileInfo &info)
    : mFileInfo(info)
#if WITH_QUAZIP
    , mZipData(info.absoluteFilePath())
#endif
{
#ifdef WITH_QUAZIP
    if (mZipData.isZipMember())
        mContainerInfo.setFile(mZipData.zipFilePath());
#endif
}

DkFileInfo::SharedData::SharedData(const QString &zipPath, const QuaZipFileInfo64 &info)
    : mFileInfo(ZipData::encodePath(zipPath, info.name))
    ,
#ifdef WITH_QUAZIP
    mZipData(zipPath, info)
#endif
{
#ifdef WITH_QUAZIP
    if (mZipData.isZipMember())
        mContainerInfo.setFile(mZipData.zipFilePath());
#endif
}

DkFileInfo::DkFileInfo(const QString &path)
{
    d = new SharedData(QFileInfo(path));
}

DkFileInfo::DkFileInfo(const QFileInfo &info)
{
    d = new SharedData(info);
}

DkFileInfo::DkFileInfo(SharedData *shared)
{
    d = shared;
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
    return d->mZipData.isZipMember();
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
        io.reset(new QuaZipFile(d->mZipData.zipFilePath(), d->mZipData.zipMemberPath()));
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

QDateTime DkFileInfo::birthTime() const
{
#if WITH_QUAZIP
    if (isFromZip())
        return d->mZipData.birthTime();
#endif
    return d->mFileInfo.birthTime();
}

QDateTime DkFileInfo::lastModified() const
{
#if WITH_QUAZIP
    if (isFromZip())
        return d->mZipData.lastModified();
#endif
    return d->mFileInfo.lastModified();
}

QDateTime DkFileInfo::lastRead() const
{
#if WITH_QUAZIP
    if (isFromZip())
        return d->mZipData.lastRead();
#endif
    return d->mFileInfo.lastRead();
}

QString DkFileInfo::owner() const
{
    return containerInfo().owner();
}

uint DkFileInfo::ownerId() const
{
    return containerInfo().ownerId();
}

QString DkFileInfo::group() const
{
    return containerInfo().group();
}

QFile::Permissions DkFileInfo::permissions() const
{
    return containerInfo().permissions();
}

bool DkFileInfo::permission(QFileDevice::Permissions flags) const
{
    return containerInfo().permission(flags);
}

bool DkFileInfo::isShortcut() const
{
    if (isFromZip())
        return false;

    bool shortcut = false, alias = false;
#if defined(Q_OS_WIN)
    shortcut = d->mFileInfo.isShortcut();
#elif defined(Q_OS_DARWIN)
    alias = d->mFileInfo.isAlias();
#endif
    return shortcut || alias;
}

bool DkFileInfo::resolveShortcut()
{
    if (!isShortcut())
        return false;

    *this = DkFileInfo(d->mFileInfo.symLinkTarget());
    return exists();
}

bool DkFileInfo::isSymLink() const
{
    return containerInfo().isSymLink();
}

QString DkFileInfo::symLinkTarget() const
{
    return containerInfo().symLinkTarget();
}

qint64 DkFileInfo::size() const
{
    if (isFromZip())
        return d->mZipData.size();
    return d->mFileInfo.size();
}

const QFileInfo &DkFileInfo::containerInfo() const
{
    if (isFromZip())
        return d->mContainerInfo;
    return d->mFileInfo;
}

#ifdef WITH_QUAZIP

DkFileInfoList DkFileInfo::readZipArchive(const QString &zipPath)
{
    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "[FileInfo] zip: open failed:" << zipPath << zip.getZipError();
        zip.getZipError();
        return {};
    }

    // seems better to use a hashtable here; ~50 extensions are possible without kimageformats
    const QStringList &fileFilters = DkSettingsManager::param().app().browseFilters;
    QSet<QString> suffixes;
    for (const QString &filter : fileFilters)
        suffixes.insert(QString(filter).replace('*', ""));

    DkFileInfoList fileInfoList;
    QuaZipFileInfo64 info;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        if (!zip.getCurrentFileInfo(&info)) {
            qWarning() << "[FileInfo] zip: getCurrentFile failed:" << zipPath << zip.getZipError();
            return {};
        }

        QString suffix = info.name.mid(info.name.lastIndexOf('.')).toLower();
        if (!suffixes.contains(suffix))
            continue;

        fileInfoList += DkFileInfo(new SharedData(zipPath, info));
    }

    return fileInfoList;
}
#endif

DkFileInfoList DkFileInfo::readDirectory(const QString &dirPath, QString folderKeywords)
{
    DkTimer dt;

    if (dirPath.isEmpty())
        return {};

#if WITH_QUAZIP
    if (DkFileInfo(dirPath).isZipFile())
        return readZipArchive(dirPath);
#endif

#ifdef Q_OS_WIN

    QString winPath = QDir::toNativeSeparators(dirPath) + "\\*.*";

    const wchar_t *fname = reinterpret_cast<const wchar_t *>(winPath.utf16());

    WIN32_FIND_DATAW findFileData;
    HANDLE MyHandle = FindFirstFileW(fname, &findFileData);

    std::vector<std::wstring> fileNameList;
    std::wstring fileName;

    if (MyHandle != INVALID_HANDLE_VALUE) {
        do {
            fileName = findFileData.cFileName;
            fileNameList.push_back(fileName); // TODO: sort correct according to numbers
        } while (FindNextFileW(MyHandle, &findFileData) != 0);
    }

    FindClose(MyHandle);

    // remove the * in fileFilters
    QStringList fileFiltersClean = DkSettingsManager::param().app().browseFilters;
    for (QString &filter : fileFiltersClean)
        filter.replace("*", "");

    // qDebug() << "browse filters: " << DkSettingsManager::param().app().browseFilters;

    QStringList fileList;
    std::vector<std::wstring>::iterator lIter = fileNameList.begin();

    // convert to QStringList
    for (unsigned int idx = 0; idx < fileNameList.size(); idx++, lIter++) {
        QString qFilename = DkUtils::stdWStringToQString(*lIter);

        // believe it or not, but this is 10 times faster than QRegExp
        // drawback: we also get files that contain *.jpg*
        for (int i = 0; i < fileFiltersClean.size(); i++) {
            if (qFilename.contains(fileFiltersClean[i], Qt::CaseInsensitive)) {
                fileList.append(qFilename);
                break;
            }
        }
    }

    qInfoClean() << "WinAPI, indexed (" << fileList.size() << ") files in: " << dt;
#else
    // true file list
    QDir tmpDir(dirPath);
    tmpDir.setSorting(QDir::LocaleAware);
    QStringList fileList = tmpDir.entryList(DkSettingsManager::param().app().browseFilters);

#endif

    // append files with no suffix
    QDir cDir(dirPath);
    QStringList allFiles = cDir.entryList();
    QStringList noSuffixFiles;

    for (const QString &name : allFiles) {
        if (!name.contains(".") && DkUtils::isValid(QFileInfo(dirPath, name))) {
            fileList << name;
        }
    }

    if (folderKeywords != "") {
        QStringList filterList = fileList;
        fileList = DkUtils::filterStringList(folderKeywords, filterList);
    }

    if (DkSettingsManager::param().resources().filterDuplicats) {
        QString preferredExtension = DkSettingsManager::param().resources().preferredExtension;
        preferredExtension = preferredExtension.replace("*.", "");
        qDebug() << "preferred extension: " << preferredExtension;

        QStringList resultList = fileList;
        fileList.clear();

        for (int idx = 0; idx < resultList.size(); idx++) {
            QFileInfo cFName = QFileInfo(resultList.at(idx));

            if (preferredExtension.compare(cFName.suffix(), Qt::CaseInsensitive) == 0) {
                fileList.append(resultList.at(idx));
                continue;
            }

            QString cFBase = cFName.baseName();
            bool remove = false;

            for (int cIdx = 0; cIdx < resultList.size(); cIdx++) {
                QString ccBase = QFileInfo(resultList.at(cIdx)).baseName();

                if (cIdx != idx && ccBase == cFBase && resultList.at(cIdx).contains(preferredExtension, Qt::CaseInsensitive)) {
                    remove = true;
                    break;
                }
            }

            if (!remove)
                fileList.append(resultList.at(idx));
        }
    }

    // fileList = sort(fileList, dir);

    DkFileInfoList fileInfoList;
    for (int idx = 0; idx < fileList.size(); idx++)
        fileInfoList.append(DkFileInfo(QFileInfo(dirPath, fileList.at(idx))));

    return fileInfoList;
}
}
