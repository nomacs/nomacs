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

#ifdef Q_WS_X11
#include <sys/sysinfo.h>
#endif

int nmc::DkUtils::debugLevel = DK_MODULE;

namespace nmc {

// code based on: http://stackoverflow.com/questions/8565430/complete-these-3-methods-with-linux-and-mac-code-memory-info-platform-independe
double DkMemory::getTotalMemory() {

	double mem = -1;

#ifdef WIN32

	MEMORYSTATUSEX MemoryStatus;
	ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
	MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

	if (GlobalMemoryStatusEx(&MemoryStatus)) {
		mem = MemoryStatus.ullTotalPhys;
	}

#elif defined Q_WS_X11

	struct sysinfo info;

	if (!sysinfo(&info))
		mem = info.totalram;


#elif defined Q_WS_MAC
	// TODO: could somebody (with a mac please add the corresponding calls?
#endif

	// convert to MB
	if (mem > 0)
		mem /= (1024*1024);

	return mem;
}

double DkMemory::getFreeMemory() {

	double mem = -1;
	

#ifdef WIN32

	MEMORYSTATUSEX MemoryStatus;

	ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
	MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

	if (GlobalMemoryStatusEx(&MemoryStatus)) {
		mem = MemoryStatus.ullAvailPhys;
	}

#elif defined Q_WS_X11

	struct sysinfo info;
	
	if (!sysinfo(&info))
		mem = info.freeram;

#elif defined Q_WS_MAC

	// TODO: could somebody with a mac please add the corresponding calls?

#endif

	// convert to MB
	if (mem > 0)
		mem /= (1024*1024);

	return mem;
}

// DkUtils --------------------------------------------------------------------
#ifdef WIN32

bool DkUtils::wCompLogic(const std::wstring & lhs, const std::wstring & rhs) {
	return StrCmpLogicalW(lhs.c_str(),rhs.c_str()) < 0;
	//return true;
}

bool DkUtils::compLogicQString(const QString & lhs, const QString & rhs) {
#if QT_VERSION < 0x050000
	return wCompLogic(lhs.toStdWString(), rhs.toStdWString());
	//return true;
#else
	return wCompLogic((wchar_t*)lhs.utf16(), (wchar_t*)rhs.utf16());	// TODO: is this nice?
#endif
}

#else // !WIN32

/*
* Natural sort algorithm - closes #305 under Linux, this time hopefully for good. 
* Qt does not provide a natural sort algorithm in the API. This is an implementation from
* https://github.com/zinnschlag/openmw/blob/master/components/contentselector/model/naturalsort.cpp
* The code is by Zinnschlag, and is under GPL3 -> OK to insert here.
*/

/*!
* Natural number sort, skips spaces.
*
* Examples:
* 1, 2, 10, 55, 100
* 01.jpg, 2.jpg, 10.jpg
*
* Note on the algorithm:
* Only as many characters as necessary are looked at and at most they all
* are looked at once.
*
* Slower then QString::compare() (of course)
*/
static inline QChar getCharAt(const QString &s, int location)
{
    return (location < s.length()) ? s.at(location) : QChar();
}

int naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs)
{
    for (int l1 = 0, l2 = 0; l1 <= s1.count() && l2 <= s2.count(); ++l1, ++l2) {
        // skip spaces, tabs and 0's
        QChar c1 = getCharAt(s1, l1);
        while (c1.isSpace()) {
            c1 = getCharAt(s1, ++l1);
        }
        QChar c2 = getCharAt(s2, l2);
        while (c2.isSpace()) {
            c2 = getCharAt(s2, ++l2);
        }
        if (c1.isDigit() && c2.isDigit()) {
            while (c1.digitValue() == 0) {
                c1 = getCharAt(s1, ++l1);
            }
            while (c2.digitValue() == 0) {
                c2 = getCharAt(s2, ++l2);
            }

            int lookAheadLocation1 = l1;
            int lookAheadLocation2 = l2;
            int currentReturnValue = 0;

            // find the last digit, setting currentReturnValue as we go if it isn't equal
            for ( QChar lookAhead1 = c1, lookAhead2 = c2;
                (lookAheadLocation1 <= s1.length() && lookAheadLocation2 <= s2.length());
                lookAhead1 = getCharAt(s1, ++lookAheadLocation1),
                lookAhead2 = getCharAt(s2, ++lookAheadLocation2)
                ) {
                bool is1ADigit = !lookAhead1.isNull() && lookAhead1.isDigit();
                bool is2ADigit = !lookAhead2.isNull() && lookAhead2.isDigit();
                if (!is1ADigit && !is2ADigit)
                    break;
                if (!is1ADigit)
                    return -1;
                if (!is2ADigit)
                    return 1;
                if (currentReturnValue == 0) {
                    if (lookAhead1 < lookAhead2) {
                        currentReturnValue = -1;
                    } 
                    else if (lookAhead1 > lookAhead2) {
                        currentReturnValue = 1;
                    }
                }
            }
            if (currentReturnValue != 0)
                return currentReturnValue;
        }
        if (cs == Qt::CaseInsensitive) {
            if (!c1.isLower()) c1 = c1.toLower();
            if (!c2.isLower()) c2 = c2.toLower();
        }

        int r = QString::localeAwareCompare(c1, c2);
        if (r < 0)
            return -1;
        if (r > 0)
            return 1;
    }
    
    // The two strings are the same ("02" == "2" when ignoring leading zeros) so fall back to the normal sort
    return QString::compare(s1, s2, cs);
}


bool DkUtils::compLogicQString(const QString & lhs, const QString & rhs) {
	
	// check if the filenames are numbers only
	// using double here lets nomacs sort correctly for files such as "1.jpg", "1.5.jpg", "2.jpg", which is nice
	// also files named e.g. "1e-6.jpg", "2e-6.jpg", "1e-5.jpg" are sorted naturally. 
	// double is accurate to approximately 16 significant digits, where using long long would work to about 19, so no big drawback.
	bool lhs_isNum;
	bool rhs_isNum;
	double lhn = lhs.left(lhs.lastIndexOf(".")).toDouble(&lhs_isNum);
	double rhn = rhs.left(rhs.lastIndexOf(".")).toDouble(&rhs_isNum);
	
	// if both filenames convert to clean numbers, compare them
	if (lhs_isNum && rhs_isNum) {
		return lhn < rhn;
	}

	// if not, let clean numbers always be sorted before mixed numbers and letters
	// logic here is deliberately spelled out, the compiler will do the optimization
	if (lhs_isNum && !rhs_isNum) {
		return true;
	}
	if (!lhs_isNum && rhs_isNum) {
		return false;
	}

    // if not clean numbers, call natural compare function
	return (naturalCompare(lhs, rhs, Qt::CaseInsensitive) < 0);
}

#endif //!WIN32


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

bool DkUtils::compRandom(const QFileInfo& lhf, const QFileInfo& rhf) {

	return qrand() % 2;
}

void DkUtils::addLanguages(QComboBox* langCombo, QStringList& languages) {

	QDir qmDir = qApp->applicationDirPath();
	
	// find all translations
	QStringList translationDirs = DkSettings::getTranslationDirs();
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
		DkSettings::loadTranslation(fileNames[i], translator);

		//: this should be the name of the language in which nomacs is translated to
		QString language = translator.translate("nmc::DkGlobalSettingsWidget", "English");
		if (language.isEmpty())
			continue;

		langCombo->addItem(language);
		languages << locale;
	}
	
	langCombo->setCurrentIndex(languages.indexOf(DkSettings::global.language));
	if (langCombo->currentIndex() == -1) // set index to English if language has not been found
		langCombo->setCurrentIndex(0);

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

	QFuture<bool> future = QtConcurrent::run(&DkUtils::checkFile, file);

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


}
