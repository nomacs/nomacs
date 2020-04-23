#include "DkUtils.h"
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
#include "DkSettings.h"
#include "DkNoMacs.h"
#include "DkViewPort.h"

#if defined(Q_OS_LINUX) && !defined(Q_OS_OPENBSD)
#include <sys/sysinfo.h>
#endif

#ifndef WITH_OPENCV
#include <cassert>
#endif

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QString>
#include <QFileInfo>
#include <QDate>
#include <QRegExp>
#include <QStringList>
#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QFuture>
#include <QtConcurrentRun>
#include <QDir>
#include <QComboBox>
#include <QCoreApplication>
#include <QTranslator>
#include <QUrl>
#include <QStandardPaths>
#include <QApplication>
#include <QMainWindow>
#include <QMouseEvent>
#include <qmath.h>
#include <QMimeDatabase>

#include <QSystemSemaphore>
#pragma warning(pop)		// no warnings from includes - end

#if defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h>	// needed since libraw 0.16
#endif

#ifdef Q_OS_WIN
#include "shlwapi.h"
#pragma comment (lib, "shlwapi.lib")
#endif

#if QT_VERSION >= 0x050500 && !defined(QT_NO_DEBUG_OUTPUT)

QDebug qDebugClean() {
	 return qDebug().noquote().nospace();
}

QDebug qInfoClean() {
	return qInfo().noquote().nospace();
}
QDebug qWarningClean() {
	return qWarning().noquote().nospace();
}
#endif

namespace nmc {


// code based on: http://stackoverflow.com/questions/8565430/complete-these-3-methods-with-linux-and-mac-code-memory-info-platform-independe
double DkMemory::getTotalMemory() {

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
		mem /= (1024*1024);

	return mem;
}

double DkMemory::getFreeMemory() {

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
		mem /= (1024*1024);

	return mem;
}

// DkUtils --------------------------------------------------------------------
#ifdef Q_OS_WIN

bool DkUtils::wCompLogic(const std::wstring & lhs, const std::wstring & rhs) {
	return StrCmpLogicalW(lhs.c_str(),rhs.c_str()) < 0;
}

bool DkUtils::compLogicQString(const QString & lhs, const QString & rhs) {
#if QT_VERSION < 0x050000
	return wCompLogic(lhs.toStdWString(), rhs.toStdWString());
#else
	return wCompLogic(qStringToStdWString(lhs), qStringToStdWString(rhs));
#endif
}

#else // !Q_OS_WIN

bool DkUtils::compLogicQString(const QString & lhs, const QString & rhs) {

	return naturalCompare(lhs, rhs, Qt::CaseInsensitive);
}

#endif //!Q_OS_WIN

bool DkUtils::naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs) {

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

			for (int idx = sIdx-1; idx >= 0; idx--) {

				if (s1[idx] != '0' && s1[idx].isDigit()) {	// find the last non-zero number (just check one string they are the same)
					prefix = s1[idx];
					break;
				}
				else if (s1[idx] != '0')
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
QString DkUtils::resolveSymLink(const QString & filePath) {
	
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

QString DkUtils::getLongestNumber(const QString& str, int startIdx) {

	int idx;

	for (idx = startIdx; idx < str.length(); idx++) {

		if (!str[idx].isDigit())
			break;
	}

	return str.mid(startIdx, idx-startIdx);
}

bool DkUtils::compDateCreated(const QFileInfo& lhf, const QFileInfo& rhf) {

	return lhf.created() < rhf.created();
}

bool DkUtils::compDateCreatedInv(const QFileInfo& lhf, const QFileInfo& rhf) {

	return !compDateCreated(lhf, rhf);
}

bool DkUtils::compDateModified(const QFileInfo& lhf, const QFileInfo& rhf) {

	return lhf.lastModified() < rhf.lastModified();
}

bool DkUtils::compDateModifiedInv(const QFileInfo& lhf, const QFileInfo& rhf) {

	return !compDateModified(lhf, rhf);
}

bool DkUtils::compFilename(const QFileInfo& lhf, const QFileInfo& rhf) {

	return compLogicQString(lhf.fileName(), rhf.fileName());
}

bool DkUtils::compFilenameInv(const QFileInfo& lhf, const QFileInfo& rhf) {

	return !compFilename(lhf, rhf);
}

bool DkUtils::compRandom(const QFileInfo&, const QFileInfo&) {

	return qrand() % 2 != 0;
}

void DkUtils::addLanguages(QComboBox* langCombo, QStringList& languages) {

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

void DkUtils::registerFileVersion() {

#ifdef Q_OS_WIN
	// this function is based on code from:
	// http://stackoverflow.com/questions/316626/how-do-i-read-from-a-version-resource-in-visual-c

	QString version(NOMACS_VERSION);	// default version (we do not know the build)

	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = {0};
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
		qWarning() << "Sorry, I can't read the module fileInfo name";
		return;
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0) {
		qWarning() << "The version info size is zero\n";
		return;
	}
	std::vector<BYTE> bytes(dwSize);

	if (bytes.empty()) {
		qWarning() << "The version info is empty\n";
		return;
	}

	// load the version info
	if (!bytes.empty() && !GetFileVersionInfo(szFilename, NULL, dwSize, &bytes[0])) {
		qWarning() << "Sorry, I can't read the version info\n";
		return;
	}

	// get the name and version strings
	UINT                uiVerLen = 0;
	VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure

	if (!bytes.empty() && !VerQueryValue(&bytes[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen)) {
		qWarning() << "Sorry, I can't get the version values...\n";
		return;
	}

	// pFixedInfo contains a lot more information...
	version = QString::number(HIWORD(pFixedInfo->dwFileVersionMS)) + "."
		+ QString::number(LOWORD(pFixedInfo->dwFileVersionMS)) + "."
		+ QString::number(HIWORD(pFixedInfo->dwFileVersionLS)) + "."
		+ QString::number(LOWORD(pFixedInfo->dwFileVersionLS));

#else
	QString version(NOMACS_VERSION);	// default version (we do not know the build)
#endif
	QApplication::setApplicationVersion(version);
}

/// <summary>
/// Saves log messages to a temporary log file.
/// Log messages are saved to DkUtils::instance().app().logPath() if
/// DkUtils::instance().app().useLogFile ist true.
/// </summary>
/// <param name="type">The message type (QtDebugMsg are not written to the log).</param>
/// <param name=""></param>
/// <param name="msg">The message.</param>
void qtMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg) {

	if (!DkSettingsManager::param().app().useLogFile)
		return;

	DkUtils::logToFile(type, msg);
}

void DkUtils::logToFile(QtMsgType type, const QString &msg) {

#if QT_VERSION >= 0x050500

	static QString filePath;

	if (filePath.isEmpty())
		filePath = DkUtils::getLogFilePath();

	QString txt;

	switch (type) {
	case QtDebugMsg:
		return;	// ignore debug messages
		break;
	case QtInfoMsg:
		txt = msg;
		break;
	case QtWarningMsg:
		txt = "[Warning] " + msg;
		break;
	case QtCriticalMsg:
		txt = "[Critical] " + msg;
		break;
	case QtFatalMsg:
		txt = "[FATAL] " + msg;
		break;
	default:
		//txt = "unknown message type: " + QString::number(type) + msg;
		return;
	}

	QFile outFile(filePath);
	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append))
		printf("cannot open %s for logging\n", filePath.toStdString().c_str());

	QTextStream ts(&outFile);
	ts << txt << endl;
#endif
}

void DkUtils::initializeDebug() {

	if (DkSettingsManager::param().app().useLogFile)
		qInstallMessageHandler(qtMessageOutput);

	// format console
	QString p = "%{if-info}[INFO] %{endif}%{if-warning}[WARNING] %{endif}%{if-critical}[CRITICAL] %{endif}%{if-fatal}[ERROR] %{endif}%{message}";
	qSetMessagePattern(p);
}

QString DkUtils::getLogFilePath() {

	QString logPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss");

	static QFileInfo fileInfo(logPath, "nomacs-" + now + "-log.txt");

	return fileInfo.absoluteFilePath();
}

QString DkUtils::getAppDataPath() {

	QString appPath;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
	// this gives us a roaming profile on windows
	appPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif QT_VERSION >= 0x050000
	appPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

	// make our own folder - AppDataLocation already does this...
	appPath += QDir::separator() + QCoreApplication::organizationName();
#else
	appPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	// make our own folder
	appPath += QDir::separator() + QCoreApplication::organizationName();
#endif

	
	if (!QDir().mkpath(appPath))
		qWarning() << "I could not create" << appPath;

	return appPath;
}

QString DkUtils::getTranslationPath() {

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

QWidget * DkUtils::getMainWindow() {

	QWidgetList widgets = QApplication::topLevelWidgets();

	QMainWindow* win = 0;

	for (int idx = 0; idx < widgets.size(); idx++) {

		if (widgets.at(idx)->inherits("QMainWindow")) {
			win = qobject_cast<QMainWindow*>(widgets.at(idx));
			break;
		}
	}

	return win;
}

QSize DkUtils::getInitialDialogSize() {

	auto win = getMainWindow();

	if (!win)
		return QSize(1024, 768);

	double width = qMax(win->width() * 0.8, 600.0);
	double height = qMax(width * 9 / 16, 450.0);
	QSize s(qRound(width), qRound(height));

	return s;
}

void DkUtils::mSleep(int ms) {

#ifdef Q_OS_WIN
	Sleep(uint(ms));
#else
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
	nanosleep(&ts, NULL);
#endif

}


bool DkUtils::exists(const QFileInfo& file, int waitMs) {

	QFuture<bool> future = QtConcurrent::run(
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
		// TODO: if we have a lot of mounted files (windows) in the history
        // we can potentially exhaust the pool & image loading has
        // to wait for these exists
        // I now only moved it to the thumbnail pool which does
        // not fix this issue at all (now thumbnail preview stalls)
        // we could
        // - create a dedicated pool for exists
        // - create a dedicated pool for image loading
        DkThumbsThreadPool::pool(),		// hook it to the thumbs pool
#endif
		&DkUtils::checkFile, file);

	for (int idx = 0; idx < waitMs; idx++) {
		if (future.isFinished())
			break;

		//qDebug() << "you are trying the new exists method... - you are modern!";

		mSleep(1);
	}

	//future.cancel();

	// assume file is not existing if it took longer than waitMs
	return (future.isFinished()) ? future : false;
}

bool DkUtils::checkFile(const QFileInfo& file) {

	return file.exists();
}

QFileInfo DkUtils::urlToLocalFile(const QUrl& url) {

	QUrl lurl = QUrl::fromUserInput(url.toString());

	// try manual conversion first, this fixes the DSC#josef.jpg problems (url fragments)
	QString fString = lurl.toString();
	fString = fString.replace("file:///", "");

	QFileInfo file = QFileInfo(fString);
	if (!file.exists())	// try an alternative conversion
		file = QFileInfo(lurl.toLocalFile());

	return file;
}

/**
 * @brief isValidByContent identifies a file by file content.
 * @param file is an existing file.
 * @return true, if file exists and has content we support, false otherwise.
 */
bool isValidByContent(const QFileInfo& file) {

    if(!file.exists()) return false;
    if(!file.isFile()) return false;

    QMimeDatabase mimeDb;
    QMimeType fileMimeType = mimeDb.mimeTypeForFile(file, QMimeDatabase::MatchContent);

    for(QString sfx: fileMimeType.suffixes()) {
        QString tryFilename = file.fileName() + QString(".") + sfx;

        if(DkUtils::hasValidSuffix(tryFilename)) {
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
bool DkUtils::isValid(const QFileInfo& fileInfo) {

	QFileInfo fInfo = fileInfo;
	QString fileName = fInfo.fileName();

	if (fInfo.isSymLink())
		fInfo = fileInfo.symLinkTarget();

    if (!fInfo.exists()) {
		return false;
    } 
	else if (hasValidSuffix(fInfo.fileName()))  {
		return true;
    } 
	else if (isValidByContent(fInfo))  {
		return true;
    }

    return false;
}

bool DkUtils::isSavable(const QString & fileName) {
	
	QStringList cleanSaveFilters = suffixOnly(DkSettingsManager::param().app().saveFilters);

	for (const QString& cFilter : cleanSaveFilters) {

		QRegExp exp = QRegExp(cFilter, Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);

		qDebug() << "checking extension: " << exp;

		if (exp.exactMatch(fileName))
			return true;
	}

	return false;
}

bool DkUtils::hasValidSuffix(const QString& fileName) {

	for (int idx = 0; idx < DkSettingsManager::param().app().fileFilters.size(); idx++) {

		QRegExp exp = QRegExp(DkSettingsManager::param().app().fileFilters.at(idx), Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);
		if (exp.exactMatch(fileName))
			return true;
	}

	return false;
}

QStringList DkUtils::suffixOnly(const QStringList & fileFilters) {

	// converts a user readable file filter (e.g. WebP (*.webp)) to a suffix only filter
	QStringList cleanedFilters;

	for (QString cFilter : fileFilters) {
		
		cFilter = cFilter.section(QRegExp("(\\(|\\))"), 1);
		cFilter = cFilter.replace(")", "");
		cleanedFilters += cFilter.split(" ");
	}

	return cleanedFilters;
}

QDateTime DkUtils::getConvertableDate(const QString& date) {

	QDateTime dateCreated;
	QStringList dateSplit = date.split(QRegExp("[/: \t]"));

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

QDateTime DkUtils::convertDate(const QString& date, const QFileInfo& file) {

	// convert date
	QDateTime dateCreated;
	QStringList dateSplit = date.split(QRegExp("[/: \t]"));

	if (dateSplit.size() >= 3) {

		QDate dateV = QDate(dateSplit[0].toInt(), dateSplit[1].toInt(), dateSplit[2].toInt());
		QTime time;

		if (dateSplit.size() >= 6)
			time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());

		dateCreated = QDateTime(dateV, time);
	}
	else if (file.exists())
		dateCreated = file.created();

	return dateCreated;
}

QString DkUtils::convertDateString(const QString& date, const QFileInfo& file) {

	// convert date
	QString dateConverted;
	QStringList dateSplit = date.split(QRegExp("[/: \t]"));

	if (dateSplit.size() >= 3) {

		QDate dateV = QDate(dateSplit[0].toInt(), dateSplit[1].toInt(), dateSplit[2].toInt());
		dateConverted = dateV.toString(Qt::SystemLocaleShortDate);

		if (dateSplit.size() >= 6) {
			QTime time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());
			dateConverted += " " + time.toString(Qt::SystemLocaleShortDate);
		}
	}
	else if (file.exists()) {
		QDateTime dateCreated = file.created();
		dateConverted += dateCreated.toString(Qt::SystemLocaleShortDate);
	}
	else
		dateConverted = "unknown date";

	return dateConverted;
}

QString DkUtils::formatToString(int format) {

	QString msg;

	switch (format) {
	case QImage::Format_Mono:
	case QImage::Format_MonoLSB:					msg = QObject::tr("Binary"); break;
	case QImage::Format_Indexed8:					msg = QObject::tr("Indexed 8-bit"); break;
	case QImage::Format_RGBX8888:
	case QImage::Format_RGBA8888_Premultiplied:
#if QT_VERSION >= 0x050500
	case QImage::Format_RGB30:
#endif
	case QImage::Format_RGB32:						msg = QObject::tr("RGB 32-bit"); break;
	case QImage::Format_ARGB32_Premultiplied:
	case QImage::Format_RGBA8888:
#if QT_VERSION >= 0x050500
	case QImage::Format_A2RGB30_Premultiplied:
#endif
	case QImage::Format_ARGB32:						msg = QObject::tr("ARGB 32-bit"); break;
	case QImage::Format_RGB555:
	case QImage::Format_RGB444:
	case QImage::Format_RGB16:						msg = QObject::tr("RGB 16-bit"); break;
	case QImage::Format_ARGB6666_Premultiplied:
	case QImage::Format_ARGB8555_Premultiplied:
	case QImage::Format_ARGB8565_Premultiplied:		msg = QObject::tr("ARGB 24-bit"); break;
	case QImage::Format_RGB888:
	case QImage::Format_RGB666:						msg = QObject::tr("RGB 24-bit"); break;
	case QImage::Format_ARGB4444_Premultiplied:		msg = QObject::tr("ARGB 16-bit"); break;

#if QT_VERSION >= 0x050500
	case QImage::Format_BGR30:						msg = QObject::tr("BGR 32-bit"); break;
	case QImage::Format_A2BGR30_Premultiplied:		msg = QObject::tr("ABGR 32-bit"); break;
	case QImage::Format_Grayscale8:					msg = QObject::tr("Grayscale 8-bit"); break;
	case QImage::Format_Alpha8:						msg = QObject::tr("Alpha 8-bit"); break;
#endif
	}

	return msg;
}

QString DkUtils::colorToString(const QColor& col) {

	return "rgba(" + QString::number(col.red()) + "," + QString::number(col.green()) + "," + QString::number(col.blue()) + "," + QString::number((float)col.alpha()/255.0f*100.0f) + "%)";
}

QStringList DkUtils::filterStringList(const QString& query, const QStringList& list) {

	// white space is the magic thingy
	QStringList queries = query.split(" ");
	QStringList resultList = list;

	for (int idx = 0; idx < queries.size(); idx++) {
		// Detect and correct special case where a space is leading or trailing the search term - this should be significant
		if (idx == 0 && queries.size() > 1 && queries[idx].size() == 0) queries[idx] = " " + queries[idx + 1];
		if (idx == queries.size() - 1 && queries.size() > 2 && queries[idx].size() == 0) queries[idx] = queries[idx - 1] + " ";
		// The queries will be repeated, but this is okay - it will just be matched both with and without the space. 
		resultList = resultList.filter(queries[idx], Qt::CaseInsensitive);
		qDebug() << "query: " << queries[idx];
	}

	// if string match returns nothing -> try a regexp
	if (resultList.empty()) {
		QRegExp regExp(query);
		resultList = list.filter(regExp);

		if (resultList.empty()) {
			regExp.setPatternSyntax(QRegExp::Wildcard);
			resultList = list.filter(regExp);
		}
	}

	return resultList;
}

bool DkUtils::moveToTrash(const QString& filePath) {

	QFileInfo fileInfo(filePath);

	// delete links
	if (fileInfo.isSymLink()) {
		QFile fh(filePath);
		return fh.remove();
	}

	if (!fileInfo.exists()) {
		qDebug() << "Sorry, I cannot delete a non-existing file: " << filePath;
		return false;
	}

// code is based on:http://stackoverflow.com/questions/17964439/move-files-to-trash-recycle-bin-in-qt
#ifdef Q_OS_WIN

	std::wstring winPath = (fileInfo.isSymLink()) ? qStringToStdWString(fileInfo.symLinkTarget()) : qStringToStdWString(filePath);
	winPath.append(1, L'\0');	// path string must be double nul-terminated

	SHFILEOPSTRUCTW shfos = {};
	shfos.hwnd   = nullptr;		// handle to window that will own generated windows, if applicable
	shfos.wFunc  = FO_DELETE;
	shfos.pFrom  = winPath.c_str();
	shfos.pTo    = nullptr;		// not used for deletion operations
	shfos.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT; // use the recycle bin

	const int retVal = SHFileOperationW(&shfos);

	return retVal == 0;		// true if no error code

#elif defined(Q_OS_LINUX)

	QString trashFilePath = QDir::homePath() + "/.local/share/Trash/files/";    // trash file path contain delete files

	QDir file;
	return file.rename(filePath, trashFilePath + fileInfo.fileName());  // rename(file old path, file trash path)

#else
	QFile fileHandle(filePath);
	return fileHandle.remove();
#endif

	return false;	// should never be hit
}

QString DkUtils::readableByte(float bytes) {

	if (bytes >= 1024*1024*1024) {
		return QString::number(bytes/(1024.0f*1024.0f*1024.0f), 'f', 2) + " GB";
	}
	else if (bytes >= 1024*1024) {
		return QString::number(bytes/(1024.0f*1024.0f), 'f', 2) + " MB";
	}
	else if (bytes >= 1024) {
		return QString::number(bytes/1024.0f, 'f', 2) + " KB";
	}
	else {
		return QString::number(bytes, 'f', 2) + " B";
	}

}

QString DkUtils::cleanFraction(const QString& frac) {

	QStringList sList = frac.split('/');
	QString cleanFrac = frac;

	if (sList.size() == 2) {
		int nom = sList[0].toInt();		// nominator
		int denom = sList[1].toInt();	// denominator

		// if exposure time is less than a second -> compute the gcd for nice values (1/500 instead of 2/1000)
		if (nom != 0 && denom != 0) {
			int gcd = DkMath::gcd(denom, nom);
			cleanFrac = QString::number(nom/gcd);

			// do not show fractions like 9/1 -> it is more natural to write 9 in these cases
			if (denom/gcd != 1)
				 cleanFrac += QString("/") + QString::number(denom/gcd);

			qDebug() << frac << " >> " << cleanFrac;
		}
	}

	return cleanFrac;
}

QString DkUtils::resolveFraction(const QString& frac) {

	QString result = frac;
	QStringList sList = frac.split('/');

	if (sList.size() == 2) {

		bool nok = false;
		bool dok = false;
		int nom = sList[0].toInt(&nok);
		int denom = sList[1].toInt(&dok);

		if (nok && dok && denom)
			result = QString::number((double)nom/denom);
	}

	return result;
}

QList<QUrl> DkUtils::findUrlsInTextNewline(QString text){
    QList<QUrl> urls;
    QStringList lines = text.split(QRegExp("\n|\r\n|\r"));
    for (QString fp : lines) {
        
		// fixes urls for windows - yes that actually annoys me
		fp = fp.replace("\\", "/");
		
		QUrl url(fp);

		if (url.isValid()) {
            
			if (url.isRelative()){
                url.setScheme("file");
            }

            urls.append(url);
        }
    }
    return urls;
}


// code from: http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
std::wstring DkUtils::qStringToStdWString(const QString &str) {
#ifdef _MSC_VER
	return std::wstring((const wchar_t *)str.utf16());
#else
	return str.toStdWString();
#endif
}

// code from: http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
QString DkUtils::stdWStringToQString(const std::wstring &str) {
#ifdef _MSC_VER
	return QString::fromUtf16((const ushort *)str.c_str());
#else
	return QString::fromStdWString(str);
#endif
}

// DkConvertFileName --------------------------------------------------------------------
DkFileNameConverter::DkFileNameConverter(const QString& fileName, const QString& pattern, int cIdx) {

	this->mFileName = fileName;
	this->mPattern = pattern;
	this->mCIdx = cIdx;
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
QString DkFileNameConverter::getConvertedFileName() {

	QString newFileName = mPattern;
	QRegExp rx("<.*>");
	rx.setMinimal(true);

	while (rx.indexIn(newFileName) != -1) {
		QString tag = rx.cap();
		QString res = "";

		if (tag.contains("<c:"))
			res = resolveFilename(tag);
		else if (tag.contains("<d:"))
			res = resolveIdx(tag);
		else if (tag.contains("<old>"))
			res = resolveExt(tag);

		// replace() replaces all matches - so if two tags are the very same, we save a little computing
		newFileName = newFileName.replace(tag, res);

	}

	return newFileName;
}

QString DkFileNameConverter::resolveFilename(const QString& tag) const {

	QString result = mFileName;

	// remove extension (Qt's QFileInfo.baseName() does a bad job if you have filenames with dots)
	result = result.replace("." + QFileInfo(mFileName).suffix(), "");

	int attr = getIntAttribute(tag);

	if (attr == 1)
		result = result.toLower();
	else if (attr == 2)
		result = result.toUpper();

	return result;
}

QString DkFileNameConverter::resolveIdx(const QString& tag) const {

	QString result = "";

	// append zeros
	int numZeros = getIntAttribute(tag);
	int startIdx = getIntAttribute(tag, 2);
	int fIdx = startIdx+mCIdx;

	if (numZeros > 0) {

		// if fIdx <= 0, log10 must not be evaluated
		int cNumZeros = fIdx > 0 ? numZeros - qFloor(std::log10(fIdx)) : numZeros;

		// zero padding
		for (int idx = 0; idx < cNumZeros; idx++) {
			result += "0";
		}
	}

	result += QString::number(fIdx);

	return result;
}

QString DkFileNameConverter::resolveExt(const QString&) const {

	QString result = QFileInfo(mFileName).suffix();

	return result;
}

int DkFileNameConverter::getIntAttribute(const QString& tag, int idx) const {

	int attr = 0;

	QStringList num = tag.split(":");

	if (num.length() > idx) {
		QString attrStr = num.at(idx);
		attrStr.replace(">", "");
		attr = attrStr.toInt();

		// no negative idx
		if (attr < 0)
			return 0;
	}

	return attr;
}

// TreeItem --------------------------------------------------------------------
TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent) {
	parentItem = parent;
	itemData = data;
}

TreeItem::~TreeItem() {
	clear();
}

void TreeItem::clear() {
	qDeleteAll(childItems);
	childItems.clear();
}

void TreeItem::remove(int rowIdx) {
	
	if (rowIdx < childCount()) {
		delete childItems[rowIdx];
		childItems.remove(rowIdx);
	}
}

void TreeItem::appendChild(TreeItem *item) {
	childItems.append(item);
}

bool TreeItem::contains(const QRegExp& regExp, int column, bool recursive) const {

	bool found = false;

	if (column == -1) {

		for (int idx = 0; idx < columnCount(); idx++)
			if (contains(regExp, idx))
				return true;

		return false;
	}

	found = data(column).toString().contains(regExp);

	// if the parent contains the key, I am valid too
	TreeItem* p = parent();
	if (!found && p)
		found = p->contains(regExp, column, false);	// the parent must not check its kid's again

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

TreeItem* TreeItem::child(int row) const {

	if (row < 0 || row >= childItems.size())
		return 0;

	return childItems[row];
}

int TreeItem::childCount() const {
	return childItems.size();
}

int TreeItem::row() const {

	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

int TreeItem::columnCount() const {

	int columns = itemData.size();

	for (int idx = 0; idx < childItems.size(); idx++)
		columns = qMax(columns, childItems[idx]->columnCount());

	return columns;
}

QVariant TreeItem::data(int column) const {
	
	if (column >= itemData.size())
		return QVariant();
	
	return itemData.value(column);
}

void TreeItem::setData(const QVariant& value, int column) {

	if (column < 0 || column >= itemData.size())
		return;

	//qDebug() << "replacing: " << itemData[0] << " with: " << value;
	itemData.replace(column, value);
}

TreeItem* TreeItem::find(const QVariant& value, int column) {

	if (column < 0)
		return 0;

	if (column < itemData.size() && itemData[column] == value)
		return this;

	for (int idx = 0; idx < childItems.size(); idx++)
		if (TreeItem* child = childItems[idx]->find(value, column))
			return child;

	return 0;
}

QStringList TreeItem::parentList() const {

	QStringList pl;
	parentList(pl);

	return pl;
}

void TreeItem::parentList(QStringList& parentKeys) const {

	if (parent()) {
		parent()->parentList(parentKeys);
		parentKeys << parent()->data(0).toString();
	}
}

TreeItem* TreeItem::parent() const {
	return parentItem;
}

void TreeItem::setParent(TreeItem* parent) {
	parentItem = parent;
}

bool TabMiddleMouseCloser::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if (mouseEvent->button() == Qt::MidButton) {
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
DkRunGuard::DkRunGuard() : mSharedMem(mSharedMemKey) {

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

DkRunGuard::~DkRunGuard() {

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
bool DkRunGuard::tryRunning() {

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
