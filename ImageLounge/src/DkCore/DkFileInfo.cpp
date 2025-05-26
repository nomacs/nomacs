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
#include <quazip/JlCompress.h>
#endif

#include <QDir>
#include <QStringBuilder>

// macro to avoid #ifdef spaghetti
#ifdef WITH_QUAZIP
#define IF_FROM_ZIP(x, y) (isFromZip() ? (x) : (y))
#else
#define IF_FROM_ZIP(x, y) (y)
#endif

namespace nmc
{
#ifdef WITH_QUAZIP

// Delimiter for zipfile+member encoded path
// - this is the same marker as KIO
// - the slash at the end makes QFileInfo::fileName(),
//   etc compatible with encoded paths
// - note: would prefer to scope to class but breaks plugins linking on Qt5/gcc
static constexpr QStringView ZipMarker = u"#/";

DkFileInfo::ZipData::ZipData(const QString &encodedFilePath)
{
    qsizetype index = encodedFilePath.indexOf(ZipMarker);
    if (index > 0) {
        mIsMember = true;
        mZipFilePath = encodedFilePath.mid(0, index);
        mZipMemberPath = encodedFilePath.mid(index + ZipMarker.length());
        readMetaData();
    }
}

DkFileInfo::ZipData::ZipData(const QString &zipFile, const QuaZipFileInfo64 &info)
{
    mIsMember = true;
    mIsCached = true;
    mZipFilePath = zipFile;
    mZipMemberPath = info.name;
    setMetaData(info);
}

void DkFileInfo::ZipData::setMetaData(const QuaZipFileInfo64 &info)
{
    mDecompressedSize = info.uncompressedSize;

    // support for these dates is mixed so we will try the various options
    // the extra data fields first for modtime since they have higher resolution
    mModified = info.getNTFSmTime().toLocalTime();
    if (!mModified.isValid())
        mModified = info.getExtTime(info.extra, QUAZIP_EXTRA_EXT_MOD_TIME_FLAG).toLocalTime();
    if (!mModified.isValid())
        mModified = info.dateTime;

    mCreated = info.getNTFScTime().toLocalTime();
    if (!mCreated.isValid())
        mCreated = info.getExtTime(info.extra, QUAZIP_EXTRA_EXT_CR_TIME_FLAG).toLocalTime();
}

QString DkFileInfo::ZipData::encodePath(const QString &zipFilePath, const QString &memberPath)
{
    Q_ASSERT(!memberPath.startsWith('/'));
    Q_ASSERT(QFileInfo(zipFilePath).absoluteFilePath() == zipFilePath);

    return zipFilePath % ZipMarker % memberPath;
}

void DkFileInfo::ZipData::readMetaData()
{
    if (mIsCached)
        return;

    // FIXME: this is a bit slow on large zipfiles, readZipArchive() could cache it
    QuaZip zip(mZipFilePath);
    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "[FileInfo] zip: open failed:" << mZipFilePath << zip.getZipError();
        return;
    }

    if (!zip.setCurrentFile(mZipMemberPath)) {
        qWarning() << "[FileInfo] zip: locate failed:" << mZipFilePath << zip.getZipError();
        return;
    }

    QuaZipFileInfo64 info;
    if (!zip.getCurrentFileInfo(&info)) {
        qWarning() << "[FileInfo] zip: decompress failed:" << mZipFilePath << zip.getZipError();
        return;
    }

    setMetaData(info);
    mIsCached = true;
}
#endif

#ifdef WITH_QUAZIP

DkFileInfo::SharedData::SharedData(const QString &path)
    : mFileInfo(path)
    , mZipData(mFileInfo.absoluteFilePath())
    , mContainerInfo(mZipData.isZipMember() ? mZipData.zipFilePath() : QString{})
{
}

DkFileInfo::SharedData::SharedData(const QFileInfo &info)
    : mFileInfo(info)
    , mZipData(mFileInfo.absoluteFilePath())
    , mContainerInfo(mZipData.isZipMember() ? mZipData.zipFilePath() : QString{})
{
}

DkFileInfo::SharedData::SharedData(const QString &zipPath, const QuaZipFileInfo64 &info)
    : mFileInfo(ZipData::encodePath(zipPath, info.name))
    , mZipData(zipPath, info)
    , mContainerInfo(zipPath)
{
}

#else

DkFileInfo::SharedData::SharedData(const QString &path)
    : mFileInfo(path)
{
}

DkFileInfo::SharedData::SharedData(const QFileInfo &info)
    : mFileInfo(info)
{
}

#endif

DkFileInfo::DkFileInfo(const QString &path)
    : d(new SharedData(path))
{
}

DkFileInfo::DkFileInfo(const QFileInfo &info)
    : d(new SharedData(info))
{
}

DkFileInfo::DkFileInfo(SharedData *shared)
    : d(shared)
{
}

DkFileInfo::operator QFileInfo() const
{
    if (isFromZip())
        qWarning() << "[FileInfo] cast to QFileInfo breaks zip files";
    return d->mFileInfo;
}

bool DkFileInfo::operator==(const DkFileInfo &other) const
{
    return path() == other.path();
}

bool DkFileInfo::isContainer(const QFileInfo &fileInfo)
{
    if (!fileInfo.isFile())
        return false;

    const QString &rawFilters = DkSettingsManager::param().app().containerRawFilters; // "*.zip *.cbz *.docx"
    if (rawFilters.contains(fileInfo.suffix(), Qt::CaseInsensitive))
        return true;

    return false;
}

#ifdef WITH_QUAZIP

bool DkFileInfo::isFromZip() const
{
    return d->mZipData.isZipMember();
}

bool DkFileInfo::isZipFile() const
{
    return isContainer(d->mFileInfo);
}

QString DkFileInfo::pathInZip() const
{
    return d->mZipData.zipMemberPath();
}
#endif

std::unique_ptr<QIODevice> DkFileInfo::getIODevice() const
{
    std::unique_ptr<QIODevice> io;

    if (isFromZip()) {
#ifdef WITH_QUAZIP
        io = std::make_unique<QuaZipFile>(d->mZipData.zipFilePath(), d->mZipData.zipMemberPath());
#endif
    } else {
        io = std::make_unique<QFile>(path());
    }

    Q_ASSERT(io != nullptr); // isFromZip()==false if !WITH_QUAZIP

    if (!io->open(QIODevice::ReadOnly)) {
        qWarning() << "[FileInfo] failed to open i/o" << path() << io->errorString();
        io = {};
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
    else if (isZipFile())
        return path();
    else
        return d->mFileInfo.absolutePath();
}

void DkFileInfo::refresh()
{
    IF_FROM_ZIP(d->mContainerInfo.refresh(), d->mFileInfo.refresh());
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
    return isZipFile() || d->mFileInfo.isDir();
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

QString DkFileInfo::baseName() const
{
    return d->mFileInfo.baseName();
}

QDateTime DkFileInfo::birthTime() const
{
    return IF_FROM_ZIP(d->mZipData.birthTime(), d->mFileInfo.birthTime());
}

QDateTime DkFileInfo::lastModified() const
{
    return IF_FROM_ZIP(d->mZipData.lastModified(), d->mFileInfo.lastModified());
}

QDateTime DkFileInfo::lastRead() const
{
    return containerInfo().lastRead();
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
    return IF_FROM_ZIP(d->mZipData.size(), d->mFileInfo.size());
}

const QFileInfo &DkFileInfo::containerInfo() const
{
    return IF_FROM_ZIP(d->mContainerInfo, d->mFileInfo);
}

// remove files that have the same name but different extension
// - assume list came from a single directory (no recursive scan)
// - output is sorted randomly
DkFileInfoList filterDuplicateNames(const DkFileInfoList &list)
{
    // extension we will prefer to choose if there are duplicate file names
    QString preferredExtension = DkSettingsManager::param().resources().preferredExtension;
    preferredExtension = preferredExtension.replace("*.", "");

    QHash<QString, DkFileInfo> dups;
    for (auto &fi : list) {
        const QString name = fi.baseName();
        const QString suffix = fi.suffix().toLower();

        auto it = dups.find(name);
        if (it == dups.end())
            dups.insert(name, fi);
        else if (it->suffix().compare(preferredExtension, Qt::CaseInsensitive) != 0 && //
                 suffix.compare(preferredExtension, Qt::CaseInsensitive) == 0)
            dups[name] = fi;
    }

    return dups.values();
}

// filter file name matching expression
DkFileInfoList filterFileName(const QRegularExpression &regExp, const DkFileInfoList &list)
{
    DkFileInfoList result;
    for (auto &fi : list)
        if (fi.fileName().contains(regExp))
            result.append(fi);
    return result;
}

// filter file name containing word
DkFileInfoList filterFileName(const QString &word, const DkFileInfoList &list)
{
    DkFileInfoList result;
    for (auto &fi : list)
        if (fi.fileName().contains(word, Qt::CaseInsensitive))
            result.append(fi);
    return result;
}

// filter list with query string
// - keywords separated by space " " (case-insensitive)
// - regular expression (case-sensitive)
// - glob pattern (anchored, case-insensitive)
DkFileInfoList filterInfoList(const QString &query, const DkFileInfoList &list)
{
    QStringList keywords = query.split(" ");
    DkFileInfoList result = list;

    for (int idx = 0; idx < keywords.size(); idx++) {
        // Detect and correct special case where a space is leading or trailing the search term - this should be significant
        if (idx == 0 && keywords.size() > 1 && keywords[idx].size() == 0)
            keywords[idx] = " " + keywords[idx + 1];
        if (idx == keywords.size() - 1 && keywords.size() > 2 && keywords[idx].size() == 0)
            keywords[idx] = keywords[idx - 1] + " ";
        // The queries will be repeated, but this is okay - it will just be matched both with and without the space.
        // result = result.filter(queries[idx], Qt::CaseInsensitive);
        result = filterFileName(keywords[idx], result);
    }

    if (result.empty()) {
        // regexp, case sensitive
        QRegularExpression regExp(query);
        if (regExp.isValid())
            result = filterFileName(regExp, list);

        // globbing, anchored, case-insensitive
        if (result.isEmpty()) {
            regExp = QRegularExpression(QRegularExpression::wildcardToRegularExpression(query), QRegularExpression::CaseInsensitiveOption);
            if (regExp.isValid())
                result = filterFileName(regExp, list);
        }
    }

    return result;
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

    DkFileInfoList fileInfoList;
    QuaZipFileInfo64 info;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        if (!zip.getCurrentFileInfo(&info)) {
            qWarning() << "[FileInfo] zip: getCurrentFile failed:" << zipPath << zip.getZipError();
            return {};
        }

        // ignore MacOS metadata, could be parsed with adouble interface from netatalk
        if (info.name.startsWith("__MACOSX/._"))
            continue;

        fileInfoList += DkFileInfo(new SharedData(zipPath, info));
    }

    return fileInfoList;
}
#endif

DkFileInfoList DkFileInfo::readDirectory(const QString &dirPath, const QString &nameFilter)
{
    DkTimer dt;

    if (dirPath.isEmpty())
        return {};

    DkFileInfoList unfiltered;

#if WITH_QUAZIP
    if (DkFileInfo(dirPath).isZipFile()) {
        unfiltered = readZipArchive(dirPath);
    } else
#endif
    {
        // all files, unfiltered, unsorted
        const QFileInfoList list = QDir(dirPath).entryInfoList(QDir::Files, QDir::NoSort);
        unfiltered.reserve(list.count());
        for (auto &fileInfo : list)
            unfiltered.append(DkFileInfo(fileInfo));
    }

    // seems better to use a hashtable here; ~50 extensions are possible without kimageformats
    const QStringList &fileFilters = DkSettingsManager::param().app().browseFilters;
    QSet<QString> suffixes;
    for (const QString &filter : fileFilters)
        suffixes.insert(QString(filter).replace("*.", ""));

    DkFileInfoList filtered;

    // filter by suffix
    for (auto &fileInfo : qAsConst(unfiltered)) {
        DkFileInfo fi = fileInfo;
        if (fi.isShortcut() && !fi.resolveShortcut())
            continue;

        const QString suffix = fileInfo.suffix().toLower();
        if (suffix.isEmpty() && !DkUtils::isValidByContent(fi)) // reads file header, maybe slow
            continue;
        else if (!suffixes.contains(suffix))
            continue;

        filtered.append(fi);
    }

    // filter with keywords, regexp, or glob
    if (!nameFilter.isEmpty())
        filtered = filterInfoList(nameFilter, filtered);

    // filter duplicate basenames
    if (DkSettingsManager::param().resources().filterDuplicats)
        filtered = filterDuplicateNames(filtered);

    qDebug() << "[readDirectory] list directory:" << dirPath << dt;

    return filtered;
}
}
