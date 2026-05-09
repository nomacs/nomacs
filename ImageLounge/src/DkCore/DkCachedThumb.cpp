
#include "DkCachedThumb.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#include <QBuffer>
#include <QByteArrayView>
#include <QCryptographicHash>
#include <QDir>
#include <QSaveFile>
#include <QStorageInfo>
#include <QtConcurrentRun>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace nmc
{
#ifdef Q_OS_WIN
static void disableContentIndexing(const QString &path)
{
    std::wstring standardPath = path.toStdWString();
    DWORD attributes = GetFileAttributesW(standardPath.c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return;
    }
    (void)SetFileAttributesW(standardPath.c_str(), attributes | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
}
#else
static void disableContentIndexing(const QString &path)
{
    Q_UNUSED(path)
}
#endif

// check if cached thumb was written by nomacs, without decoding PNG image
static bool isNomacsThumb(const QString &filename)
{
    // PNG uses big-endian, swap to little
    static constexpr auto readUint32 = [](QFile &file) {
        QByteArray buffer = file.read(4);
        if (buffer.size() != 4) {
            return 0u;
        }
        static_assert(Q_BYTE_ORDER == Q_LITTLE_ENDIAN);
        return (static_cast<uint32_t>(buffer[0]) << 24) | (static_cast<uint32_t>(buffer[1]) << 16)
            | (static_cast<uint32_t>(buffer[2]) << 8) | static_cast<uint32_t>(buffer[3]);
    };

    struct Comment {
        QString key, value;
    };

    // read a chunk from the PNG file and return comment
    static constexpr auto readChunk = [](QFile &file) -> std::optional<Comment> {
        const uint32_t length = readUint32(file);
        const QByteArray type = file.read(4);

        if (type.length() != 4) {
            return std::nullopt;
        }

        const QByteArray data = file.read(length);
        (void)readUint32(file); // crc of chunk type

        if (data.length() == length && type == "tEXt") {
            // the key and value are separated by \0
            QString comment = QString::fromLatin1(data);
            int delim = comment.indexOf(QChar{'\0'});
            if (delim >= 0) {
                QString key = comment.mid(0, delim);
                QString value = comment.mid(delim + 1);
                return Comment{key, value};
            }
        }
        return std::nullopt;
    };

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file!";
        return false;
    }

    QByteArray signature = file.read(8);

    static constexpr std::array<uint8_t, 8> kPngHeader = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    if (signature != QByteArrayView(kPngHeader.data(), kPngHeader.size())) {
        return false;
    }

    while (!file.atEnd()) {
        auto comment = readChunk(file);
        if (comment && comment->key == "Software") {
            return comment->value == "nomacs";
        }
    }

    return false;
}

void DkCachedThumb::cleanup(bool deleteAll)
{
    const bool isSharedCache = isXdgCompliant();
    const auto maxUsedBytes = deleteAll
        ? 0
        : static_cast<qint64>(DkSettingsManager::param().resources().thumbDiskSpace) * 1024 * 1024;

    DkTimer dt{};
    bool haveAtimeCheck = false;
    bool haveAtime = false;

    QFileInfoList allFiles;
    qint64 usedBytes = 0;

    // Ignore non-xdg folders and "fail" folder
    QStringList dirNameFilters;
    for (auto xdgBin : kXdgBins) {
        dirNameFilters += xdgBin.name;
    }

    const QFileInfoList dirs = QDir(cacheHome()).entryInfoList(dirNameFilters, QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto &d : dirs) {
        const QFileInfoList files = QDir(d.absoluteFilePath()).entryInfoList(QDir::Files);
        for (QFileInfo f : files) {
            // Don't delete thumbs created by other software
            if (isSharedCache && !isNomacsThumb(f.absoluteFilePath())) {
                continue;
            }

            // Try using atime, but not all filesystems support it
            // Fallback to last modified which should be the same as created/birthTime
            if (!haveAtimeCheck) {
                haveAtimeCheck = true;
                QDateTime aTime = f.lastRead();
                haveAtime = aTime.isValid();
            }

            usedBytes += f.size();
            allFiles.emplaceBack(f);
        }
    }
    qint64 freedBytes = 0;
    if (usedBytes > maxUsedBytes) {
        // sort in reverse, slightly faster removal from array
        std::sort(allFiles.begin(), allFiles.end(), [&](QFileInfo &a, QFileInfo &b) {
            QDateTime at = haveAtime ? a.lastRead() : a.lastModified();
            QDateTime bt = haveAtime ? b.lastRead() : b.lastModified();
            return bt < at;
        });

        qint64 toReclaim = usedBytes - maxUsedBytes;
        while (toReclaim > 0 && !allFiles.isEmpty()) {
            const QFileInfo f = allFiles.takeLast();
            qint64 size = f.size();

            // Note there is no synchronization with thumb loader/saver; but there should not be
            // as a system service could also be managing the cache.
            if (!QFile::remove(f.absoluteFilePath())) {
                qWarning() << "[DkCachedThumb] failed to remove file:" << f.absoluteFilePath();
            } else {
                freedBytes += size;
            }
            toReclaim -= size;
        }
    }

    // cleanup empty directories
    const QFileInfoList emptyDirs = QDir(cacheHome()).entryInfoList(dirNameFilters, QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto &d : dirs) {
        QDir().rmdir(d.absoluteFilePath());
    }
    QDir().rmdir(cacheHome());

    qInfo().noquote() << "[DkCachedThumb] cache cleanup:" << cacheHome() << DkUtils::readableByte(usedBytes) << "used"
                      << DkUtils::readableByte(freedBytes) << "reclaimed" << dt;
}

QFuture<void> &DkCachedThumb::cleanupJob()
{
    static QFuture<void> future{};
    return future;
}

void DkCachedThumb::cleanupAsync()
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (cleanupJob().isRunning()) {
        qWarning() << "[CachedThumb] cleanup in progress, try again later";
        return;
    }

    cleanupJob() = QtConcurrent::run(&DkCachedThumb::cleanup, false);
}

void DkCachedThumb::cleanupSync(bool deleteAll)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    while (cleanupJob().isRunning()) {
        qInfo() << "[CachedThumb] waiting for previous cleanup to finish...";
        QThread::msleep(1000);
    }

    cleanup(deleteAll);
}

DkCachedThumb::DkCachedThumb(DkFileInfo &fileInfo, int size, ScaleConstraint constraint)
    : mFileInfo(fileInfo)
    , mSize(size)
    , mConstraint(constraint)
{
    //
    // Different viewers/thumbnailers do not agree on what to do with the path/URI.
    // This is a problem because if URIs don't match, the cache doesn't work between programs.
    // The XDG spec wants a "canonical, minimally-encoded URI", but the majority
    // use fully-encoded, non-canonical URI. So we'll do that as well
    //
    // KDE Thumbnails v2 (kio-extras): fully encoded URI, not canonical
    // GNOME::Thumbnailfactory: fully encoded, not canonical
    // Mate::ThumbnailFactory: fully encoded, not canonical
    // geeqie: fully encoded, not canonical
    // thunar: fully encoded, not canonical
    //
    // The outliers:
    // gwenview: pretty encoded, not canonical
    // photoqt: pretty encoded, canonical (symlinks resolved)
    //
    mUri = QUrl::fromLocalFile(fileInfo.path()).toEncoded();
    QByteArray hash = QCryptographicHash::hash(mUri, QCryptographicHash::Md5).toHex();
    const char *suffix = isXdgCompliant() ? ".png" : ".dat";
    mCacheFileName = hash + suffix;
}

bool DkCachedThumb::isXdgCompliant()
{
    // Always use non-compliance mode on Windows and macOS as there are no
    // other programs to share thumbnails with
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
    return DkSettingsManager::param().resources().sharedThumbs;
#else
    return false;
#endif
}

QImage DkCachedThumb::load()
{
    DkTimer dt{};

    // Find cached thumbnail satisfying scale constraint. If it is larger than we need, we'll
    // use it and also save the smaller size for later.
    XdgBin bin{};
    QImageReader reader{};
    int maxSize = DkSettingsManager::param().resources().maxThumbSize;
    if (DkSettingsManager::param().display().highQualityThumbs) {
        // In hq mode allow the next size up to be used, if needed to meet scale constraint
        // This prevents blur of square thumbnails except very narrow images
        maxSize = qMin(1024, maxSize * 2);
    }

    for (auto &xdgBin : kXdgBins) {
        QString cacheFilePath = cacheHome() + u'/' + xdgBin.name + u'/' + mCacheFileName;

        if (!QFile::exists(cacheFilePath)) {
            // qInfo() << "[CachedThumb] no file" << cacheFilePath;
            continue;
        }

        // Read png header without fully decoding, to get the size of the thumb
        reader.setFileName(cacheFilePath);

        QSize sz = reader.size();
        if (!sz.isValid()) {
            // qInfo() << "[CachedThumb] no SIZE" << cacheFilePath;
            // another nomacs instance or system daemon may have deleted this out from under us
            continue;
        }

        if (xdgBin.size >= maxSize || isLargeEnough(sz)) {
            // qInfo() << "[CachedThumb] FOUND" << mFileInfo.fileName() << sz << "for" << mSize << (int)mConstraint <<
            // dt;
            bin = xdgBin;
            break;
        }

        // qInfo() << "[CachedThumb] cache REJECT" << mFileInfo.fileName() << sz << "for" << mSize << (int)mConstraint;
    }

    if (bin.size <= 0) {
        // qInfo() << "[CachedThumb] cache MISS" << mFileInfo.fileName() << mSize << (int)mConstraint << mUri
        // << mCacheFileName << dt;
        reader.setDevice(nullptr); // Release open file, if any
        return {};
    }

    QImage th = reader.read();
    if (!th.isNull()) {
        // Check the freshness of the thumb; no reason to delete it here since we will regenerate shortly
        QString uri = th.text(QStringLiteral("Thumb::URI"));
        qint64 modTime = th.text(QStringLiteral("Thumb::MTime")).toLongLong();
        qint64 size = th.text(QStringLiteral("Thumb::Size")).toLongLong();
        if (uri == mUri && modTime == lastModified().toSecsSinceEpoch() && size == mFileInfo.size()) {
            // qDebug() << "[CachedThumb] loaded" << mFileInfo.fileName() << th.size() << "for size" << mSize
            // << "constraint" << (int)mConstraint;

            // If thumb is larger than necessary, we can save a smaller thumb to the cache for next time
            save(th, bin.size);

            return th;
        }
    }

    return {};
}

void DkCachedThumb::save(const QImage &img, int loadedBinSize)
{
    static bool cacheDisabled = false;
    static QMutex jobMutex{}, dirMutex{}, countMutex{};
    static int saveCount = 0;
    static QSet<QString> jobs{};

    DkTimer dt{};

    Q_ASSERT(!img.isNull());

    if (img.isNull()) {
        return;
    }

    // If there is some filesystem problem we disable cache until restarting nomacs
    if (cacheDisabled) {
        return;
    }

    // Saving disabled in private mode
    if (DkSettingsManager::param().app().privateMode) {
        return;
    }

    // Prevent recursive caching
    const QString cacheRoot = cacheHome();
    if (mFileInfo.path().startsWith(cacheRoot)) {
        return;
    }

    const QSize imgSize = img.size();

    // Find the first xdg bin large enough for scale constraint
    XdgBin bin{};
    QString cacheDirPath{}, cacheFilePath{};
    int maxSize = DkSettingsManager::param().resources().maxThumbSize;
    if (DkSettingsManager::param().display().highQualityThumbs) {
        maxSize = qMin(1024, maxSize * 2);
    }

    for (auto &xdgBin : kXdgBins) {
        bin = xdgBin;
        cacheDirPath = cacheRoot + u'/' + bin.name;
        cacheFilePath = cacheDirPath + u'/' + mCacheFileName;

        // get the xdg thumb size at this level - longest-side scaling
        int xdgW, xdgH;
        if (imgSize.width() > imgSize.height()) {
            xdgW = bin.size;
            xdgH = xdgW * imgSize.height() / imgSize.width();
        } else {
            xdgH = bin.size;
            xdgW = xdgH * imgSize.width() / imgSize.height();
        }

        QSize sz{xdgW, xdgH};
        if (bin.size >= maxSize || isLargeEnough(sz)) {
            break;
        }
    }

    // The image itself may be smaller than this bin, no need to cache it
    int imgDim = qMax(imgSize.width(), imgSize.height());
    if (imgDim < bin.size) {
        // qInfo() << "[CachedThumb] SKIP save, too small" << mFileInfo.fileName() << imgSize << "bin:" << bin.size
        // << "req:" << mSize << (int)mConstraint;
        return;
    }

    // Allow caching a smaller thumbnail from a larger thumbnail.
    // Since it is at least 2x larger, blur is minimal
    if (loadedBinSize > 0 && bin.size >= loadedBinSize) {
        return;
    }

    // Mod time is required for freshness check
    const QDateTime modTime = lastModified();
    if (!modTime.isValid()) {
        qWarning() << "[CachedThumb] missing modtime, will not cache" << mFileInfo.path();
        return;
    }

    // We are now entering the slow part of the process
    // Multiple threads can request a thumbnail to the same bin; this should be rare, but in nomacs we
    // have multiple views with thumbnails. There is no threat of corrupted files due to atomic swap,
    // but this will waste cpu cycles.
    {
        QMutexLocker locker(&jobMutex);
        if (jobs.contains(cacheFilePath)) {
            return;
        }
        jobs.insert(cacheFilePath);
    }

    // Release the job when we return
    struct OnReturn {
        const QString mCacheFilePath;
        ~OnReturn()
        {
            QMutexLocker locker(&jobMutex);
            jobs.remove(mCacheFilePath);
        }
    } removeJob{cacheFilePath};

    // Create cache folder and any parent directories required
    if (!QFileInfo::exists(cacheDirPath)) {
        QMutexLocker locker(&dirMutex);
        if (!QFileInfo::exists(cacheDirPath)) {
            if (!QDir().mkpath(cacheDirPath)) {
                cacheDisabled = true;
                qWarning() << "[CachedThumb] cache disabled, failed to create cache dir" << cacheDirPath;
                return;
            } else {
                disableContentIndexing(cacheDirPath);
                qInfo() << "[CachedThumb] cache directory created at" << cacheDirPath;
            }
        }
    }

    // Check free space, only once in a while since it can be slow
    {
        QMutexLocker locker(&countMutex);
        if (saveCount % 100 == 0) { // check the first time and every 100 thereafter
            QStorageInfo storage(cacheDirPath);
            storage.refresh();
            int mb = storage.bytesAvailable() / 1024 / 1024;
            // 1024px thumbs are around 1MB (PNG) so check for room for about 100 of them.
            // Add extra margin since the filesystem metadata may not be in sync
            cacheDisabled = mb < 1024;
            // qDebug() << "[CacheThumb] free space MB" << mb << dt;

            if (cacheDisabled) {
                qWarning() << "[CachedThumb] cache disabled, low free space on" << cacheDirPath;
                return;
            }
        }
        saveCount++;
    }

    QImage thumb = DkImage::createThumb(img, bin.size, ScaleConstraint::longest_side);
    if (thumb.isNull()) {
        qWarning() << "[CachedThumb] null image" << mFileInfo.fileName();
        return;
    }

    Q_ASSERT(thumb.width() == bin.size || thumb.height() == bin.size);

    thumb.setText(QStringLiteral("Software"), QStringLiteral("nomacs"));
    thumb.setText(QStringLiteral("Thumb::MTime"), QString::number(modTime.toSecsSinceEpoch()));
    thumb.setText(QStringLiteral("Thumb::Size"), QString::number(mFileInfo.size()));
    thumb.setText(QStringLiteral("Thumb::URI"), mUri);
    thumb.setText(QStringLiteral("Thumb::Image::Width"), QString::number(img.width()));
    thumb.setText(QStringLiteral("Thumb::Image::Height"), QString::number(img.height()));

    // TODO: most others also set this, can be used by file managers for grouping or badges
    // thumb.setText(QStringLiteral("Thumb::MimeType"), img.mimeType());

    // For interop we want sRGB thumbnails, createThumb() does the conversion
    Q_ASSERT(!thumb.colorSpace().isValid() || thumb.colorSpace() == QColorSpace{QColorSpace::SRgb});
    thumb.setColorSpace({}); // prevent libpng "warning: iCCP: known incorrect sRGB profile"

    const char *format = "png";
    int quality = 50;
    if (!isXdgCompliant() && !DkImage::alphaChannelUsed(thumb)) {
        // On Windows/macOS we don't need XDG compliance and can use JPEG thumbs
        // Linux users also have this option if they don't care about sharing thumbs
        // Use highest quality since these will be resampled
        format = "jpg";
        quality = 100;
    }

    // We have many threads writing; compress to buffer for less disk fragmentation/syscalls
    QByteArray data;
    QBuffer buffer(&data);
    if (thumb.save(&buffer, format, quality)) {
        // Write to a temporary and atomic swap to prevent thumb corruption (other processes or cleanup function)
        QSaveFile f(cacheFilePath);
        if (f.open(QFile::WriteOnly | QFile::Truncate)) {
            if (f.write(data) == data.length() && f.commit()) {
                // qInfo() << "[CachedThumb] SAVED" << mFileInfo.fileName() << img.size() << "to" << thumb.size() << dt;
            }
        }
        if (f.error() != QFile::NoError) {
            qWarning() << "[CacheThumb] write failed:" << f.error() << f.errorString() << cacheFilePath;
        }
    }
}

QString DkCachedThumb::cacheHome()
{
    QString path{};
#if defined(Q_OS_WIN)
    path = DkUtils::getTemporaryDirPath() + "/thumbnails";
#elif defined(Q_OS_MACOS)
    path = QDir::homePath() + "/Library/Caches/nomacs/thumbnails";
#else
    path = qEnvironmentVariable("XDG_CACHE_HOME");
    if (path.isEmpty()) {
        path = QDir::homePath() + "/.cache";
    }
    path += isXdgCompliant() ? "/thumbnails" : "/nomacs/thumbnails";
#endif
    return path;
}

bool DkCachedThumb::isLargeEnough(const QSize &sz) const
{
    switch (mConstraint) {
    case ScaleConstraint::longest_side:
        return qMax(sz.width(), sz.height()) >= mSize;
    case ScaleConstraint::shortest_side:
        return qMin(sz.width(), sz.height()) >= mSize;
    case ScaleConstraint::width:
        return sz.width() >= mSize;
    case ScaleConstraint::height:
        return sz.height() >= mSize;
    }
    return false;
}

QDateTime DkCachedThumb::lastModified() const
{
    // Try to use the zip member modtime - repacking the zip won't drop thumbnails.
    // If we don't have that, fallback to .zip modtime
    QDateTime lastModified = mFileInfo.lastModified();
    if (!lastModified.isValid() && mFileInfo.isFromZip()) {
        lastModified = QFileInfo(mFileInfo.dirPath()).lastModified();
    }
    return lastModified;
}
}
