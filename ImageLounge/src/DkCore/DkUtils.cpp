/*******************************************************************************************************
 DkUtils.cpp
 Created on:	09.03.2010

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

#include "DkUtils.h"
#include "DkMath.h"
#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkVersion.h"
#include "DkViewPort.h"

#if defined(Q_OS_LINUX) && !defined(Q_OS_OPENBSD)
#include <sys/sysinfo.h>
#endif

#ifndef WITH_OPENCV
#include <cassert>
#endif

#pragma warning(push, 0) // no warnings from includes - begin
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QFuture>
#include <QMainWindow>
#include <QMimeDatabase>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <QUrl>
#include <QtConcurrentRun>
#include <qmath.h>

#include <QSystemSemaphore>
#pragma warning(pop) // no warnings from includes - end

#include <exiv2/version.hpp>

#ifdef WITH_LIBRAW
#include <libraw/libraw.h>
#endif

#ifdef WITH_LIBTIFF
#ifdef Q_CC_MSVC
#include <tif_config.h>
#endif

//  here we clash (typedef redefinition with different types ('long' vs 'int64_t' (aka 'long long')))
//  so we simply define our own int64 before including tiffio
#define uint64 uint64_hack_
#define int64 int64_hack_

#include <tiffio.h>

#undef uint64
#undef int64
#endif // WITH_LIBTIFF

#if defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h> // needed since libraw 0.16
#endif

#ifdef Q_OS_WIN
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")
#endif

#if !defined(QT_NO_DEBUG_OUTPUT)

QDebug qDebugClean()
{
    return qDebug().noquote().nospace();
}

QDebug qInfoClean()
{
    return qInfo().noquote().nospace();
}
QDebug qWarningClean()
{
    return qWarning().noquote().nospace();
}
#endif

namespace nmc
{

// code based on: http://stackoverflow.com/questions/8565430/complete-these-3-methods-with-linux-and-mac-code-memory-info-platform-independe
double DkMemory::getTotalMemory()
{
    double mem = -1;

#ifdef Q_OS_WIN

    MEMORYSTATUSEX MemoryStatus;
    ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
    MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&MemoryStatus)) {
        mem = (double)MemoryStatus.ullTotalPhys;
    }

#elif defined Q_OS_LINUX and not defined(Q_OS_OPENBSD)

    struct sysinfo info;

    if (!sysinfo(&info))
        mem = info.totalram;

#elif defined Q_OS_MAC
    // TODO: could somebody (with a mac please add the corresponding calls?
#endif

    // convert to MB
    if (mem > 0)
        mem /= (1024 * 1024);

    return mem;
}

double DkMemory::getFreeMemory()
{
    double mem = -1;

#ifdef Q_OS_WIN

    MEMORYSTATUSEX MemoryStatus;

    ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
    MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&MemoryStatus)) {
        mem = (double)MemoryStatus.ullAvailPhys;
    }

#elif defined Q_OS_LINUX and not defined(Q_OS_OPENBSD)

    struct sysinfo info;

    if (!sysinfo(&info))
        mem = info.freeram;

#elif defined Q_OS_MAC

    // TODO: could somebody with a mac please add the corresponding calls?

#endif

    // convert to MB
    if (mem > 0)
        mem /= (1024 * 1024);

    return mem;
}

// DkUtils --------------------------------------------------------------------
#ifdef Q_OS_WIN

bool DkUtils::wCompLogic(const std::wstring &lhs, const std::wstring &rhs)
{
    return StrCmpLogicalW(lhs.c_str(), rhs.c_str()) < 0;
}

bool DkUtils::compLogicQString(const QString &lhs, const QString &rhs)
{
    return wCompLogic(qStringToStdWString(lhs), qStringToStdWString(rhs));
}

#else // !Q_OS_WIN

bool DkUtils::compLogicQString(const QString &lhs, const QString &rhs)
{
    return naturalCompare(lhs, rhs, Qt::CaseInsensitive);
}

#endif //! Q_OS_WIN

bool DkUtils::naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs)
{
    int sIdx = 0;

    // the first value is the most significant bit
    // so we try to find the first difference in the strings
    // this gives us an advantage:
    // img1 and img10 are sorted correctly since the string compare
    // does here what it should
    // img4 and img10 are also sorted correctly since 4 < 10
    // in addition we don't get into troubles with large numbers
    // as we skip identical values...
    for (; sIdx < s1.length() && sIdx < s2.length(); sIdx++)
        if (s1[sIdx] != s2[sIdx])
            break;

    // if both values start with a digit
    if (sIdx < s1.length() && sIdx < s2.length() && s1[sIdx].isDigit() && s2[sIdx].isDigit()) {
        QString prefix = "";

        // if the number has zeros we get into troubles:
        // 101 and 12 result in '01' and '2'
        // for double sort this means: 01 < 2 (though 101 > 12)
        // so we simply search the last non zero number that was equal and prepend that
        // if there is no such number (e.g. img001 vs img101) we are fine already
        // this fixes #469
        if (s1[sIdx] == '0' || s2[sIdx] == '0') {
            for (int idx = sIdx - 1; idx >= 0; idx--) {
                if (s1[idx] != '0' && s1[idx].isDigit()) { // find the last non-zero number (just check one string they are the same)
                    prefix = s1[idx];
                    break;
                } else if (s1[idx] != '0')
                    break;
            }
        }

        QString cs1 = prefix + getLongestNumber(s1, sIdx);
        QString cs2 = prefix + getLongestNumber(s2, sIdx);

        double n1 = cs1.toDouble();
        double n2 = cs2.toDouble();

        if (n1 != n2)
            return n1 < n2;
    }

    // we're good to go with a string compare here...
    return QString::compare(s1, s2, cs) < 0;
}

/// <summary>
/// Resolves symbolic links.
/// </summary>
/// <param name="filePath">The file path of the (potential) sym link.</param>
/// <returns>If the file is no link, its path is returned.</returns>
QString DkUtils::resolveSymLink(const QString &filePath)
{
    QString rFilePath = filePath;

    QFileInfo fInfo(filePath);

    if (fInfo.isSymLink()) {
        rFilePath = fInfo.symLinkTarget();
    }
    // check if files < 1 kb contain a link
    else if (fInfo.size() < 1000) {
        QFile file(filePath);

        // silently ignore not readable files here
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream txt(&file);

            while (!txt.atEnd()) {
                // is there an absolute path?
                QString cl = txt.readLine();

                if (cl.isEmpty())
                    continue;

                QFileInfo fi(cl);
                if (fi.exists() && fi.isFile() && DkUtils::hasValidSuffix(fi.fileName())) {
                    rFilePath = fi.absoluteFilePath();
                    break;
                }

                // is there a relative path?
                fi = QFileInfo(fInfo.absolutePath() + QDir::separator() + cl);
                if (fi.exists() && fi.isFile() && DkUtils::hasValidSuffix(fi.fileName())) {
                    rFilePath = fi.absoluteFilePath();
                    break;
                }
            }
        }

        file.close();
    }

    return rFilePath;
}

QString DkUtils::getLongestNumber(const QString &str, int startIdx)
{
    int idx;

    for (idx = startIdx; idx < str.length(); idx++) {
        if (!str[idx].isDigit())
            break;
    }

    return str.mid(startIdx, idx - startIdx);
}

bool DkUtils::compDateCreated(const DkFileInfo &lhf, const DkFileInfo &rhf)
{
    // avoid equality because we keep our directory position/index using the sorted position
    auto left = lhf.birthTime(), right = rhf.birthTime();
    if (left != right)
        return left < right;
    else
        return compFilename(lhf, rhf);
}

bool DkUtils::compDateModified(const DkFileInfo &lhf, const DkFileInfo &rhf)
{
    auto left = lhf.lastModified(), right = rhf.lastModified();
    if (left != right)
        return left < right;
    else
        return compFilename(lhf, rhf);
}

bool DkUtils::compFilename(const DkFileInfo &lhf, const DkFileInfo &rhf)
{
    if (lhf.isFromZip() && rhf.isFromZip())
        return compLogicQString(lhf.pathInZip(), rhf.pathInZip());
    else
        return compLogicQString(lhf.fileName(), rhf.fileName());
}

bool DkUtils::compFileSize(const DkFileInfo &lhf, const DkFileInfo &rhf)
{
    auto left = lhf.size(), right = rhf.size();
    if (left != right)
        return left < right;
    else
        return compFilename(lhf, rhf);
}

bool DkUtils::compRandom(const DkFileInfo &lhf, const DkFileInfo &rhf)
{
    return QCryptographicHash::hash(lhf.path().toUtf8() + QByteArray::number(DkSettingsManager::param().global().sortSeed), QCryptographicHash::Algorithm::Md5)
        > QCryptographicHash::hash(rhf.path().toUtf8() + QByteArray::number(DkSettingsManager::param().global().sortSeed), QCryptographicHash::Algorithm::Md5);
}

void DkUtils::addLanguages(QComboBox *langCombo, QStringList &languages)
{
    QDir qmDir = qApp->applicationDirPath();

    // find all translations
    QStringList translationDirs = DkSettingsManager::param().getTranslationDirs();
    QStringList fileNames;

    for (int idx = 0; idx < translationDirs.size(); idx++) {
        fileNames += QDir(translationDirs[idx]).entryList(QStringList("nomacs_*.qm"));
    }

    langCombo->addItem("English");
    languages << "en";

    for (int i = 0; i < fileNames.size(); ++i) {
        QString locale = fileNames[i];
        locale.remove(0, locale.indexOf('_') + 1);
        locale.chop(3);

        QTranslator translator;
        DkSettingsManager::param().loadTranslation(fileNames[i], translator);

        //: this should be the name of the language in which nomacs is translated to
        QString language = translator.translate("nmc::DkGlobalSettingsWidget", "English");
        if (language.isEmpty())
            continue;

        langCombo->addItem(language);
        languages << locale;
    }

    langCombo->setCurrentIndex(languages.indexOf(DkSettingsManager::param().global().language));
    if (langCombo->currentIndex() == -1) // set index to English if language has not been found
        langCombo->setCurrentIndex(0);
}

/// <summary>
/// Saves log messages to a temporary log file.
/// Log messages are saved to DkUtils::instance().app().logPath() if
/// DkUtils::instance().app().useLogFile ist true.
/// </summary>
/// <param name="type">The message type (QtDebugMsg are not written to the log).</param>
/// <param name="context">Additional information about a log message</param>
/// <param name="msg">The message.</param>
void qtMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!DkSettingsManager::param().app().useLogFile)
        return;

    DkUtils::logToFile(type, qFormatLogMessage(type, context, msg));
}

void DkUtils::logToFile(QtMsgType type, const QString &msg)
{
    static QString filePath;

    if (filePath.isEmpty())
        filePath = DkUtils::getLogFilePath();

    QFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        printf("cannot open %s for logging\n", filePath.toStdString().c_str());
    } else {
        QTextStream ts(&outFile);
        ts << msg << Qt::endl;
    }
}

void DkUtils::initializeDebug()
{
    if (DkSettingsManager::param().app().useLogFile)
        qInstallMessageHandler(qtMessageOutput);

    // format message output
    QString p =
        "%{if-info}[INFO] %{endif}%{if-warning}[WARNING] %{endif}%{if-critical}[CRITICAL] %{endif}"
        "%{if-fatal}[ERROR] %{endif}%{if-debug}[DEBUG] %{endif}%{message}";
    qSetMessagePattern(p);
}

QString DkUtils::getLogFilePath()
{
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss");

    static QFileInfo fileInfo(logPath, "nomacs-" + now + "-log.txt");

    return fileInfo.absoluteFilePath();
}

QString DkUtils::getAppDataPath()
{
    QString appPath;

    // this gives us a roaming profile on windows
    appPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (!QDir().mkpath(appPath))
        qWarning() << "I could not create" << appPath;

    return appPath;
}

QString DkUtils::getTranslationPath()
{
    QString trPath;

    if (DkSettingsManager::param().isPortable())
        trPath = QCoreApplication::applicationDirPath();
    else
        trPath = DkUtils::getAppDataPath();

    trPath += QDir::separator() + QString("translations");

    if (!QDir().mkpath(trPath))
        qWarning() << "I could not create" << trPath;

    return trPath;
}

QWidget *DkUtils::getMainWindow()
{
    QWidgetList widgets = QApplication::topLevelWidgets();

    QMainWindow *win = 0;

    for (int idx = 0; idx < widgets.size(); idx++) {
        if (widgets.at(idx)->inherits("QMainWindow")) {
            win = qobject_cast<QMainWindow *>(widgets.at(idx));
            break;
        }
    }

    return win;
}

QSize DkUtils::getInitialDialogSize()
{
    auto win = getMainWindow();

    if (!win)
        return QSize(1024, 768);

    double width = qMax(win->width() * 0.8, 600.0);
    double height = qMax(width * 9 / 16, 450.0);
    QSize s(qRound(width), qRound(height));

    return s;
}

QString DkUtils::getBuildInfo()
{
    QString info;

    // architecture
    QString arch = QSysInfo::buildCpuArchitecture();

    // version & build date
    info += "nomacs " + QApplication::applicationVersion() + ", " + arch + "\n";
    info += QString(nmc::revisionString) + "\n";

// omit from Linux for reproducable builds (see #139)
#ifdef Q_OS_WIN
    info += QString(__DATE__);
#endif

    if (DkSettingsManager::param().isPortable())
        info += " Portable"; // same line as __DATE__ (running out of room)

    info += "\n\n";

    QString memory = QString::number(int(DkMemory::getTotalMemory() / 1000)) + "GB";
    info += QSysInfo::prettyProductName() + ", " + memory + "\n";

    // library versions (dynamic version where possible)
    info += "Qt " + QString(qVersion()) + ", " + qApp->platformName() + ", scale=" + QString::number(qApp->devicePixelRatio(), 10, 2) + "\n";
    info += "Exiv2 " + QString(Exiv2::version()) + "\n";

#ifdef WITH_LIBRAW
#ifdef _MSC_VER
    // FIXME: library call segfaults in appveyor build
    info += "LibRAW " + QString(LIBRAW_VERSION_STR) + "\n";
#else
    info += "LibRAW " + QString(LibRaw::version()) + "\n";
#endif
#else
    info += "No RAW Support\n";
#endif

#ifdef WITH_OPENCV
    info += "OpenCV " + QString(cv::getVersionString().c_str()) + "\n";
#else
    info += "No CV Support\n";
#endif

#if WITH_LIBTIFF
    {
        // TIFFGetVersion has other stuff in it, but it doesn't fit here
        QString version = "error";
        QRegularExpression re("(\\d+\\.\\d+\\.\\d+)");
        QRegularExpressionMatch match = re.match(TIFFGetVersion());
        if (match.hasMatch())
            version = match.captured(0);
        info += "TIFF " + version + "\n";
    }
#else
    info += "No TIFF Pages\n";
#endif

#ifdef WITH_QUAZIP
    const char *quazip = "v1";
#ifdef QUAZIP_STATIC
    const char *linkage = "(s)";
#else
    const char *linkage = "";
#endif
    info += QString("QuaZip ") + quazip + linkage + "\n";
#else
    info += "No Zip Support\n";
#endif

    return info;
}

void DkUtils::mSleep(int ms)
{
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);
#endif
}

bool DkUtils::exists(const QFileInfo &file, int waitMs)
{
    QFuture<bool> future = QtConcurrent::run(
        // TODO: if we have a lot of mounted files (windows) in the history
        // we can potentially exhaust the pool & image loading has
        // to wait for these exists
        // I now only moved it to the thumbnail pool which does
        // not fix this issue at all (now thumbnail preview stalls)
        // we could
        // - create a dedicated pool for exists
        // - create a dedicated pool for image loading
        DkThumbsThreadPool::pool(), // hook it to the thumbs pool
        [file] {
            return DkUtils::checkFile(file);
        });

    for (int idx = 0; idx < waitMs; idx++) {
        if (future.isFinished())
            break;

        // qDebug() << "you are trying the new exists method... - you are modern!";

        mSleep(1);
    }

    // future.cancel();

    // assume file is not existing if it took longer than waitMs
    return (future.isFinished()) ? future.result() : false;
}

bool DkUtils::checkFile(const QFileInfo &file)
{
    return file.exists();
}

QFileInfo DkUtils::urlToLocalFile(const QUrl &url)
{
    QUrl lurl = QUrl::fromUserInput(url.toString());

    // try manual conversion first, this fixes the DSC#josef.jpg problems (url fragments)
    QString fString = lurl.toString();
    fString = fString.replace("file:///", "");

    QFileInfo file = QFileInfo(fString);
    if (!file.exists()) // try an alternative conversion
        file = QFileInfo(lurl.toLocalFile());

    return file;
}

QString DkUtils::fileNameFromUrl(const QUrl &url)
{
    QString name(url.toString());

    // get Tweety.svg from https://upload.wikimedia.org/wikipedia/en/0/02/Tweety.svg
    name = name.split("/").last();
    // get 100919-snoop-dogg-feature.jpg from
    // https://thenypost.files.wordpress.com/2019/10/100919-snoop-dogg-feature.jpg?quality=90&strip=all&w=618&h=410&crop=1
    name = name.split("?").first();

    return name;
}

QString DkUtils::nowString()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh.mm.ss");
}

/**
 * @brief isValidByContent identifies a file by file content.
 * @param file is an existing file.
 * @return true, if file exists and has content we support, false otherwise.
 */
bool DkUtils::isValidByContent(const QFileInfo &file)
{
    if (!file.exists())
        return false;
    if (!file.isFile())
        return false;

    QMimeDatabase mimeDb;
    QMimeType fileMimeType = mimeDb.mimeTypeForFile(file, QMimeDatabase::MatchContent);

    for (QString sfx : fileMimeType.suffixes()) {
        QString tryFilename = file.fileName() + QString(".") + sfx;

        if (DkUtils::hasValidSuffix(tryFilename)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief: return if a file is supported by nomacs or not.
 * @param fileInfo the file info of the file to be validated.
 * @return bool true if the file format is supported.
 **/
bool DkUtils::isValid(const QFileInfo &fileInfo)
{
    QFileInfo fInfo = fileInfo;
    QString fileName = fInfo.fileName();

    if (fInfo.isSymLink())
        fInfo = QFileInfo(fileInfo.symLinkTarget());

    if (!fInfo.exists()) {
        return false;
    } else if (hasValidSuffix(fInfo.fileName())) {
        return true;
    } else if (isValidByContent(fInfo)) {
        return true;
    }

    return false;
}

bool DkUtils::isSavable(const QString &fileName)
{
    QStringList cleanSaveFilters = suffixOnly(DkSettingsManager::param().app().saveFilters);

    for (const QString &cFilter : cleanSaveFilters) {
        QRegularExpression exp = QRegularExpression(QRegularExpression::wildcardToRegularExpression(cFilter), QRegularExpression::CaseInsensitiveOption);

        qDebug() << "checking extension: " << exp;

        if (exp.match(fileName).hasMatch())
            return true;
    }

    return false;
}

bool DkUtils::hasValidSuffix(const QString &fileName)
{
    const QString fileSuffix = fileName.mid(fileName.lastIndexOf('.')).toLower(); // .jpg
    if (fileSuffix.isEmpty())
        return false;

    const QStringList &filters = DkSettingsManager::param().app().fileFilters; // [*.jpg, ...]
    for (const QString &filter : filters)
        if (filter.endsWith(fileSuffix))
            return true;

    return false;
}

QStringList DkUtils::suffixOnly(const QStringList &fileFilters)
{
    // converts a user readable file filter (e.g. WebP (*.webp)) to a suffix only filter
    QStringList cleanedFilters;

    for (QString cFilter : fileFilters) {
        cFilter = cFilter.section(QRegularExpression("(\\(|\\))"), 1);
        cFilter = cFilter.replace(")", "");
        cleanedFilters += cFilter.split(" ");
    }

    return cleanedFilters;
}

QDateTime DkUtils::getConvertableDate(const QString &date)
{
    QDateTime dateCreated;
    QStringList dateSplit = date.split(QRegularExpression("[/: \t]"));

    if (date.count(":") != 4 /*|| date.count(QRegExp("\t")) != 1*/)
        return dateCreated;

    if (dateSplit.size() >= 3) {
        int y = dateSplit[0].toInt();
        int m = dateSplit[1].toInt();
        int d = dateSplit[2].toInt();

        if (y == 0 || m == 0 || d == 0)
            return dateCreated;

        QDate dateV = QDate(y, m, d);
        QTime time;

        if (dateSplit.size() >= 6)
            time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());

        dateCreated = QDateTime(dateV, time);
    }

    return dateCreated;
}

QDateTime DkUtils::convertDate(const QString &date, const QFileInfo &file)
{
    // convert date
    QDateTime dateCreated;
    QStringList dateSplit = date.split(QRegularExpression("[/: \t]"));

    if (dateSplit.size() >= 3) {
        QDate dateV = QDate(dateSplit[0].toInt(), dateSplit[1].toInt(), dateSplit[2].toInt());
        QTime time;

        if (dateSplit.size() >= 6)
            time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());

        dateCreated = QDateTime(dateV, time);
    } else if (file.exists())
        dateCreated = file.birthTime();

    return dateCreated;
}

QString DkUtils::convertDateString(const QString &date, const QFileInfo &file)
{
    // convert date
    QString dateConverted;
    QStringList dateSplit = date.split(QRegularExpression("[/: \t]"));

    if (dateSplit.size() >= 3) {
        QDate dateV = QDate(dateSplit[0].toInt(), dateSplit[1].toInt(), dateSplit[2].toInt());
        dateConverted = dateV.toString();

        if (dateSplit.size() >= 6) {
            QTime time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());
            dateConverted += " " + time.toString();
        }
    } else if (file.exists()) {
        QDateTime dateCreated = file.birthTime();
        dateConverted += dateCreated.toString();
    } else
        dateConverted = "unknown date";

    return dateConverted;
}

QString DkUtils::formatToString(int format)
{
    QString msg;

    switch ((QImage::Format)format) {
    case QImage::Format_Invalid:
    case QImage::NImageFormats:
        break;
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        msg = QObject::tr("Binary");
        break;
    case QImage::Format_Indexed8:
        msg = QObject::tr("Indexed 8-bit");
        break;
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888_Premultiplied:
    case QImage::Format_RGB30:
    case QImage::Format_RGB32:
        msg = QObject::tr("RGB 32-bit");
        break;
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGBA8888:
    case QImage::Format_A2RGB30_Premultiplied:
    case QImage::Format_ARGB32:
        msg = QObject::tr("ARGB 32-bit");
        break;
    case QImage::Format_RGB555:
    case QImage::Format_RGB444:
    case QImage::Format_RGB16:
        msg = QObject::tr("RGB 16-bit");
        break;
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
        msg = QObject::tr("ARGB 24-bit");
        break;
    case QImage::Format_RGB888:
    case QImage::Format_RGB666:
        msg = QObject::tr("RGB 24-bit");
        break;
    case QImage::Format_BGR888:
        msg = QObject::tr("BGR 24-bit");
        break;
    case QImage::Format_ARGB4444_Premultiplied:
        msg = QObject::tr("ARGB 16-bit");
        break;
    case QImage::Format_BGR30:
        msg = QObject::tr("BGR 32-bit");
        break;
    case QImage::Format_A2BGR30_Premultiplied:
        msg = QObject::tr("ABGR 32-bit");
        break;
    case QImage::Format_Grayscale8:
        msg = QObject::tr("Grayscale 8-bit");
        break;
    case QImage::Format_Grayscale16:
        msg = QObject::tr("Grayscale 16-bit");
        break;
    case QImage::Format_Alpha8:
        msg = QObject::tr("Alpha 8-bit");
        break;
    case QImage::Format_RGBX64: // all 64-bit int formats have 4 channels
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
        msg = QObject::tr("RGBA 64-bit");
        break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    case QImage::Format_RGBX16FPx4: // all formats have 4 channels
    case QImage::Format_RGBA16FPx4:
    case QImage::Format_RGBA16FPx4_Premultiplied:
        msg = QObject::tr("RGBA FP16");
        break;
    case QImage::Format_RGBX32FPx4: // all formats have 4 channels
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBA32FPx4_Premultiplied:
        msg = QObject::tr("RGBA FP32");
        break;
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    case QImage::Format_CMYK8888:
        msg = QObject::tr("CMYK 32-Bit");
        break;
#endif
    }

    return msg;
}

QString DkUtils::colorToCssHex(const QColor &color, bool alpha)
{
    // CSS Color Module 4 hex color
    // https://www.w3.org/TR/css-color-4/#hex-notation

    // NOTE: css hex colors are 8-bpc sRgb
    //       QColor::rgba() converts to ARGB-8888 for wider formats
    if (color.spec() != QColor::Rgb)
        return "";

    QString hex = color.name(); // also uses color.rgba()
    if (alpha)
        hex += QString::number(qAlpha(color.rgba()) | 0x100, 16).right(2);
    return hex.toUpper();
}

QString DkUtils::colorToString(const QColor &col)
{
    return "rgba(" + QString::number(col.red()) + "," + QString::number(col.green()) + "," + QString::number(col.blue()) + ","
        + QString::number((float)col.alpha() / 255.0f * 100.0f) + "%)";
}

QStringList DkUtils::filterStringList(const QString &query, const QStringList &list)
{
    // white space is the magic thingy
    QStringList queries = query.split(" ");
    QStringList resultList = list;

    for (int idx = 0; idx < queries.size(); idx++) {
        // Detect and correct special case where a space is leading or trailing the search term - this should be significant
        if (idx == 0 && queries.size() > 1 && queries[idx].size() == 0)
            queries[idx] = " " + queries[idx + 1];
        if (idx == queries.size() - 1 && queries.size() > 2 && queries[idx].size() == 0)
            queries[idx] = queries[idx - 1] + " ";
        // The queries will be repeated, but this is okay - it will just be matched both with and without the space.
        resultList = resultList.filter(queries[idx], Qt::CaseInsensitive);
        qDebug() << "query: " << queries[idx];
    }

    // if string match returns nothing -> try a regexp
    if (resultList.empty()) {
        QRegularExpression regExp(query);
        resultList = list.filter(regExp);

        if (resultList.empty()) {
            QString wildcardExp = QRegularExpression::wildcardToRegularExpression(query);
            QRegularExpression re(QRegularExpression::anchoredPattern(wildcardExp), QRegularExpression::CaseInsensitiveOption);
            resultList = list.filter(re);
        }
    }

    return resultList;
}

bool DkUtils::moveToTrash(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    QFile file(filePath);
    bool ok = false;

    // delete links first; exists() will fail on a broken symlink
    if (fileInfo.isSymLink()) {
        qInfo() << "move to trash: deleting symlink" << filePath;
        ok = file.remove();
    } else if (!fileInfo.exists()) {
        qWarning() << "move to trash: cannot delete a non-existing file: " << filePath;
        return false;
    } else {
        qInfo() << "move to trash: moving" << filePath;
        ok = file.moveToTrash();
    }

    if (!ok) {
        // clang-format off
        qWarning().nospace() << "move to trash: error:" << file.errorString()
                             << "\n\terror:" << file.error()
                             << "\n\tisFile:" << fileInfo.isFile()
                             << "\n\tfile permissions:" << file.permissions()
                             << "\n\tdir permissions:" << QFileInfo(fileInfo.absolutePath()).permissions()
                             << "\n\towner:" << fileInfo.owner()
                             << "\n\tgroup:" << fileInfo.group();
        // clang-format on
    }

    return ok;
}

QString DkUtils::readableByte(float bytes)
{
    if (bytes >= 1024 * 1024 * 1024) {
        return QString::number(bytes / (1024.0f * 1024.0f * 1024.0f), 'f', 2) + " GB";
    } else if (bytes >= 1024 * 1024) {
        return QString::number(bytes / (1024.0f * 1024.0f), 'f', 2) + " MB";
    } else if (bytes >= 1024) {
        return QString::number(bytes / 1024.0f, 'f', 2) + " KB";
    } else {
        return QString::number(bytes, 'f', 2) + " B";
    }
}

QString DkUtils::cleanFraction(const QString &frac)
{
    QStringList sList = frac.split('/');
    QString cleanFrac = frac;

    if (sList.size() == 2) {
        int nom = sList[0].toInt(); // nominator
        int denom = sList[1].toInt(); // denominator

        // if exposure time is less than a second -> compute the gcd for nice values (1/500 instead of 2/1000)
        if (nom != 0 && denom != 0) {
            int gcd = DkMath::gcd(denom, nom);
            cleanFrac = QString::number(nom / gcd);

            // do not show fractions like 9/1 -> it is more natural to write 9 in these cases
            if (denom / gcd != 1)
                cleanFrac += QString("/") + QString::number(denom / gcd);

            qDebug() << frac << " >> " << cleanFrac;
        }
    }

    return cleanFrac;
}

QString DkUtils::resolveFraction(const QString &frac)
{
    QString result = frac;
    QStringList sList = frac.split('/');

    if (sList.size() == 2) {
        bool nok = false;
        bool dok = false;
        int nom = sList[0].toInt(&nok);
        int denom = sList[1].toInt(&dok);

        if (nok && dok && denom)
            result = QString::number((double)nom / denom);
    }

    return result;
}

QList<QUrl> DkUtils::findUrlsInTextNewline(QString text)
{
    QList<QUrl> urls;
    QStringList lines = text.split(QRegularExpression("\n|\r\n|\r"));
    for (QString fp : lines) {
        // fixes urls for windows - yes that actually annoys me
        fp = fp.replace("\\", "/");

        QUrl url(fp);

        if (url.isValid()) {
            if (url.isRelative()) {
                url.setScheme("file");
            }

            urls.append(url);
        }
    }
    return urls;
}

// code from: http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
std::wstring DkUtils::qStringToStdWString(const QString &str)
{
#ifdef _MSC_VER
    return std::wstring((const wchar_t *)str.utf16());
#else
    return str.toStdWString();
#endif
}

// code from: http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
QString DkUtils::stdWStringToQString(const std::wstring &str)
{
#ifdef _MSC_VER
    return QString::fromUtf16((const char16_t *)str.c_str());
#else
    return QString::fromStdWString(str);
#endif
}

// DkConvertFileName --------------------------------------------------------------------
DkFileNameConverter::DkFileNameConverter(const QString &p)
    : mFrags{}
{
    const QStringView pattern(p);
    qsizetype start = 0;
    Token type = Token::Text;
    std::vector<uint> numbers{};
    FragType fragType = FragType::Text;

    for (qsizetype i = 0; i < pattern.size(); i++) {
        const QChar ch = pattern.at(i);

        if (ch != '<' && ch != '>' && ch != ':') {
            continue;
        }

        QStringView token = pattern.sliced(start, i - start);
        start = i + 1;

        if (ch == '<') {
            if (type != Token::Text) {
                qWarning() << "DkFileNameConverter parse pattern failed: " << pattern << ", expecting token text at index " << i;
                return;
            }
            if (token.size() != 0) {
                mFrags.push_back(Frag{FragType::Text, 0, 0, token.toString(), 0});
            }
            numbers.resize(0);
            type = Token::TagName;

        } else if (ch == '>' || ch == ':') {
            switch (type) {
            case Token::TagName:
                if (token.compare(QString("c")) == 0) {
                    fragType = FragType::FileName;
                } else if (token.compare(QString("d")) == 0) {
                    fragType = FragType::Index;
                } else if (token.compare(QString("old")) == 0) {
                    mFrags.push_back(Frag{FragType::Ext, 0, 0, "", 0});
                    fragType = FragType::Ext;
                } else {
                    qWarning() << "DkFileNameConverter parse pattern failed: " << pattern << ", invalid tag " << token;
                    return;
                }
                break;

            case Token::Number:
                numbers.push_back(token.toUInt());
                break;

            default:
                qWarning() << "DkFileNameConverter parse pattern failed: " << pattern << ", expecting token tag name or number at index " << i;
                return;
            }

            if (ch == '>') {
                switch (fragType) {
                case FragType::FileName:
                    mFrags.push_back(Frag{
                        FragType::FileName,
                        0,
                        0,
                        "",
                        numbers.size() > 0 ? numbers.at(0) : 0,
                    });
                    break;
                case FragType::Index:
                    mFrags.push_back(Frag{
                        FragType::Index,
                        numbers.size() > 0 ? numbers.at(0) + 1 : 0,
                        numbers.size() > 1 ? numbers.at(1) : 0, // Plus 1 because pad n + index 0 counts 1 char
                        "",
                        0,
                    });
                    break;
                default:
                    break;
                }

                type = Token::Text;
            } else {
                type = Token::Number;
            }
        }
    }

    if (pattern.size() > start) {
        if (type != Token::Text) {
            qWarning() << "DkFileNameConverter parse pattern failed: " << pattern << ", expecting token text at index " << pattern.size();
            return;
        }
        QStringView token = pattern.sliced(start, pattern.size() - start);
        mFrags.push_back(Frag{FragType::Text, 0, 0, token.toString(), 0});
    }
}

/**
 * Converts file names with a given pattern (used for e.g. batch rename)
 * The pattern is:
 * <d:3> is replaced with the cIdx value (:3 -> zero padding up to 3 digits)
 * <c:0> int (0 = no change, 1 = to lower, 2 = to upper)
 *
 * if it ends with .jpg we assume a fixed extension.
 * .<old> is replaced with the fileName extension.
 *
 * So a filename could look like this:
 * some-fixed-name-<c:1><d:3>.<old>
 * @return QString
 **/
QString DkFileNameConverter::convert(const QString &file, int index) const
{
    QStringList list{};

    for (const auto &frag : mFrags) {
        switch (frag.type) {
        case FragType::Text:
            list.append(frag.text);
            break;
        case FragType::FileName: {
            const qsizetype sep = file.lastIndexOf('.');

            const QString fileName = sep >= 0 ? file.first(sep) : file;
            if (frag.caseConv == 1) {
                list.append(fileName.toLower());
            } else if (frag.caseConv == 2) {
                list.append(fileName.toUpper());
            } else {
                list.append(fileName);
            }
            break;
        }
        case FragType::Ext:
            list.append(QFileInfo(file).suffix());
            break;
        case FragType::Index:
            list.append(QStringLiteral("%1").arg(index + frag.indexStart, frag.indexDigits, 10, QLatin1Char('0')));
            break;
        }
    }
    return list.join("");
}

// TreeItem --------------------------------------------------------------------
TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
    parentItem = parent;
    itemData = data;
}

TreeItem::~TreeItem()
{
    clear();
}

void TreeItem::clear()
{
    qDeleteAll(childItems);
    childItems.clear();
}

void TreeItem::remove(int rowIdx)
{
    if (rowIdx < childCount()) {
        delete childItems[rowIdx];
        childItems.remove(rowIdx);
    }
}

void TreeItem::appendChild(TreeItem *item)
{
    childItems.append(item);
}

bool TreeItem::contains(const QRegularExpression &regExp, int column, bool recursive) const
{
    bool found = false;

    if (column == -1) {
        for (int idx = 0; idx < columnCount(); idx++)
            if (contains(regExp, idx))
                return true;

        return false;
    }

    found = data(column).toString().contains(regExp);

    // if the parent contains the key, I am valid too
    TreeItem *p = parent();
    if (!found && p)
        found = p->contains(regExp, column, false); // the parent must not check its kid's again

    // if a child contains the key, I am valid too
    if (!found && recursive) {
        for (int idx = 0; idx < childCount(); idx++) {
            assert(child(idx));
            found = child(idx)->contains(regExp, column, recursive);

            if (found)
                break;
        }
    }

    return found;
}

TreeItem *TreeItem::child(int row) const
{
    if (row < 0 || row >= childItems.size())
        return 0;

    return childItems[row];
}

int TreeItem::childCount() const
{
    return childItems.size();
}

int TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem *>(this));

    return 0;
}

int TreeItem::columnCount() const
{
    int columns = itemData.size();

    for (int idx = 0; idx < childItems.size(); idx++)
        columns = qMax(columns, childItems[idx]->columnCount());

    return columns;
}

QVariant TreeItem::data(int column) const
{
    if (column >= itemData.size())
        return QVariant();

    return itemData.value(column);
}

void TreeItem::setData(const QVariant &value, int column)
{
    if (column < 0 || column >= itemData.size())
        return;

    // qDebug() << "replacing: " << itemData[0] << " with: " << value;
    itemData.replace(column, value);
}

TreeItem *TreeItem::find(const QVariant &value, int column)
{
    if (column < 0)
        return 0;

    if (column < itemData.size() && itemData[column] == value)
        return this;

    for (int idx = 0; idx < childItems.size(); idx++)
        if (TreeItem *child = childItems[idx]->find(value, column))
            return child;

    return 0;
}

QStringList TreeItem::parentList() const
{
    QStringList pl;
    parentList(pl);

    return pl;
}

void TreeItem::parentList(QStringList &parentKeys) const
{
    if (parent()) {
        parent()->parentList(parentKeys);
        parentKeys << parent()->data(0).toString();
    }
}

TreeItem *TreeItem::parent() const
{
    return parentItem;
}

void TreeItem::setParent(TreeItem *parent)
{
    parentItem = parent;
}

bool TabMiddleMouseCloser::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::MiddleButton) {
            auto tabbar = static_cast<QTabBar *>(obj);
            for (int i = 0; i < tabbar->count(); i++) {
                QRect tabrect = tabbar->tabRect(i);
                if (tabrect.contains(mouseEvent->pos()))
                    callback(i);
            }
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

// DkRunGuard --------------------------------------------------------------------
DkRunGuard::DkRunGuard()
    : mSharedMem(mSharedMemKey)
{
    QSystemSemaphore lock(mLockKey, 1);
    lock.acquire();

    {
        // this fixes unix issues if the first instance crashes
        // see here for details: https://habrahabr.ru/post/173281/
        QSharedMemory fix(mSharedMemKey);
        fix.attach();
    }

    lock.release();
}

DkRunGuard::~DkRunGuard()
{
    QSystemSemaphore lock(mLockKey, 1);
    lock.acquire();

    if (mSharedMem.isAttached())
        mSharedMem.detach();

    lock.release();
}

/// <summary>
/// Checks if this instance is the first running.
/// If it's the first instance, a shared memory block is created.
/// </summary>
/// <returns>true if this is the first instance</returns>
bool DkRunGuard::tryRunning()
{
    QSystemSemaphore lock(mLockKey, 1);
    lock.acquire();

    // check if we can attach to the shared memory
    // if not: we are the first
    bool attached = mSharedMem.attach();
    if (!attached)
        mSharedMem.create(sizeof(quint64));

    lock.release();

    return !attached;
}
}
