/*******************************************************************************************************
 DkUtils.h
 Created on:	05.02.2010

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

#pragma once

#include <functional>
#include <math.h>

#pragma warning(push, 0) // no warnings from includes - begin
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVector>

#include <QSharedMemory>
#pragma warning(pop) // no warnings from includes - end

#pragma warning(disable : 4251) // dll interface missing
#pragma warning(disable : 4714) // Qt's force inline

#include <assert.h> // convenience for minimal builds

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable : 4127) // no 'conditional expression is constant' if qDebug() messages are removed
#endif

#ifndef Q_OS_WIN
#include <time.h>
#endif

#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#pragma warning(disable : 4996)
#endif

#include "opencv2/core/core.hpp"
#else

// #define int64 long long;
#include <sstream>
#define CV_PI 3.141592653589793238462643383279
#endif

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

#if !defined(QT_NO_DEBUG_OUTPUT)
DllCoreExport QDebug qDebugClean();
DllCoreExport QDebug qInfoClean();
DllCoreExport QDebug qWarningClean();
#else
#define qDebugClean() qDebug()
#define qInfoClean() qDebug()
#define qWarningClean() qDebug()
#endif

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// fixes Qt's damn no latin1 on tr() policy
#define dk_degree_str QChar(0x00B0)

// Qt defines
class QComboBox;
class QColor;
class QUrl;

namespace nmc
{

// nomacs defines
class TreeItem;

/**
 * This class contains general functions which are useful.
 **/
class DllCoreExport DkUtils
{
private:
public:
#ifdef Q_OS_WIN

    /**
     * Logical string compare function.
     * This function is used to sort:
     * a1.png
     * a2.png
     * a10.png
     * instead of:
     * a1.png
     * a10.png
     * a2.png
     * @param lhs left string
     * @param rhs right string
     * @return bool true if left string < right string
     **/
    static bool wCompLogic(const std::wstring &lhs, const std::wstring &rhs);
#endif

    static bool compLogicQString(const QString &lhs, const QString &rhs);

    static bool compFilename(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compFilenameInv(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compFileSize(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compFileSizeInv(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compDateCreated(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compDateCreatedInv(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compDateModified(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compDateModifiedInv(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool compRandom(const QFileInfo &lhf, const QFileInfo &rhf);

    static bool naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    static QString resolveSymLink(const QString &filePath);

    static QString getLongestNumber(const QString &str, int startIdx = 0);

    static void addLanguages(QComboBox *langCombo, QStringList &languages);

    static void initializeDebug();

    static void logToFile(QtMsgType type, const QString &msg);

    static QString getLogFilePath();

    static QString getAppDataPath();

    static QString getTranslationPath();

    static QWidget *getMainWindow();

    static QSize getInitialDialogSize();

    /**
     * Sleeps n ms.
     * This function is based on the QTest::qSleep(int ms)
     * @param ms time to sleep
     **/
    static void mSleep(int ms);

    /**
     * Fast file exists method.
     * This function seems to be a bit unnecessary, however
     * at least windows has long (> 10 sec) timeouts if a
     * network drive is disconnected and you want to find
     * a file on that network. This function calls the normal
     * file.exists() but returns false if a timeout > waitMs
     * is reached.
     * @param file the file to check
     * @param waitMs time in milli seconds to wait for file.exists()
     * @return bool true if the file exists
     **/
    static bool exists(const QFileInfo &file, int waitMs = 10);
    static bool checkFile(const QFileInfo &file);
    static QFileInfo urlToLocalFile(const QUrl &url);
    static QString fileNameFromUrl(const QUrl &url);
    static QString nowString();
    static QString colorToString(const QColor &col);
    static QString readableByte(float bytes);
    static QStringList filterStringList(const QString &query, const QStringList &list);
    static bool moveToTrash(const QString &filePath);
    static QList<QUrl> findUrlsInTextNewline(QString text);

#ifdef WITH_OPENCV
    /**
     * Prints a matrix to the standard output.
     * This is especially useful for copy and pasting e.g.
     * histograms to matlab and visualizing them there.
     * @param src an image CV_32FC1.
     * @param varName the variable name for Matlab.
     **/
    static void printMat(const cv::Mat src, const char *varName)
    {
        if (src.depth() != CV_32FC1) {
            // qDebug() << "I could not visualize the mat: " << QString::fromAscii(varName);
            return;
        }

        printf("%s = %s", varName, printMat(src).c_str());
    }

    /**
     * Prints a matrix to the standard output.
     * This is especially useful for copy and pasting e.g.
     * histograms to matlab and visualizing them there.
     * @param src an image CV_32FC1.
     * @param varName the variable name for Matlab.
     **/
    static std::string printMat(const cv::Mat src)
    {
        if (src.depth() != CV_32FC1) {
            // qDebug() << "I could not visualize the mat: " << QString::fromStdString(DkUtils::getMatInfo(src));
            return "";
        }

        std::string msg = " ["; // matlab...

        int cnt = 0;

        for (int rIdx = 0; rIdx < src.rows; rIdx++) {
            const float *srcPtr = src.ptr<float>(rIdx);

            for (int cIdx = 0; cIdx < src.cols; cIdx++, cnt++) {
                msg += DkUtils::stringify(srcPtr[cIdx], 3);

                msg += (cIdx < src.cols - 1) ? " " : "; "; // next row matlab?

                if (cnt % 7 == 0)
                    msg += "...\n";
            }
        }
        msg += "];\n";

        return msg;
    }

    /**
     * Prints the cv::Mat's attributes to the standard output.
     * The cv::Mat's attributes are: size, depth, number of channels and
     * dynamic range.
     * @param img an image (if it has more than one channel, the dynamic range
     * is not displayed)
     * @param varname the name of the matrix
     **/
    static void getMatInfo(cv::Mat img, std::string varname)
    {
        printf("%s: %s\n", varname.c_str(), getMatInfo(img).c_str());
    }

    /**
     * Converts the cv::Mat's attributes to a string.
     * The cv::Mat's attributes are: size, depth, number of channels and
     * dynamic range.
     * @param img an image (if it has more than one channel, the dynamic range
     * is not converted).
     * @return a string with the cv::Mat's attributes.
     **/
    static std::string getMatInfo(cv::Mat img)
    {
        std::string info = "\n\nimage info:\n";

        if (img.empty()) {
            info += "   <empty image>\n";
            return info;
        }

        info += "   " + DkUtils::stringify(img.rows) + " x " + DkUtils::stringify(img.cols) + " (rows x cols)\n";
        info += "   channels: " + DkUtils::stringify(img.channels()) + "\n";

        int depth = img.depth();
        info += "   depth: ";
        switch (depth) {
        case CV_8U:
            info += "CV_8U";
            break;
        case CV_32F:
            info += "CV_32F";
            break;
        case CV_16S:
            info += "CV_16S";
            break;
        case CV_16U:
            info += "CV_16U";
            break;
        case CV_32S:
            info += "CV_32S";
            break;
        case CV_64F:
            info += "CV_64F";
            break;
        default:
            info += "unknown";
            break;
        }

        if (img.channels() == 1) {
            info += "\n   dynamic range: ";

            double min, max;
            minMaxLoc(img, &min, &max);
            info += "[" + DkUtils::stringify(min) + " " + DkUtils::stringify(max) + "]\n";
        } else if (img.channels() > 1) {
            info += "\n   dynamic range: ";

            double min, max;
            minMaxLoc(img, &min, &max);
            info += "[" + DkUtils::stringify(min) + " " + DkUtils::stringify(max) + "]\n";
        } else
            info += "\n";

        return info;
    }
#endif

    /**
     * Appends an attribute name to the filename given.
     * generates: image0001.tif -> img0001_mask.tif
     * @param fName the filename with extension.
     * @param ext the new file extension if it is "" the old extension is used.
     * @param attribute the attribute which extends the filename.
     * @return the generated filename.
     **/
    static std::string createFileName(std::string fName, std::string attribute, std::string ext = "")
    {
        if (ext == "")
            ext = fName.substr(fName.length() - 4, fName.length()); // use the old extension

        // generate: img0001.tif -> img0001_mask.tif
        return fName.substr(0, fName.length() - 4) + attribute + ext;
    }

    static std::string removeExtension(std::string fName)
    {
        return fName.substr(0, fName.find_last_of("."));
    }

    static std::string getFileNameFromPath(std::string fName)
    {
        return fName.substr(fName.find_last_of("/") + 1); // TODO: Schiach!!
    }

    /**
     * Converts a number to a string.
     * @throws an exception if number is not a number.
     * @param number any number.
     * @return a string representing the number.
     **/
    template<typename numFmt>
    static std::string stringify(numFmt number)
    {
        std::stringstream stream;
        if (!(stream << number)) {
            std::string msg = "Sorry, I could not cast it to a string";
            // throw DkCastException(msg, __LINE__, __FILE__);
            printf("%s", msg.c_str()); // TODO: we need a solution for DkSnippet here...
        }

        return stream.str();
    }

    /**
     * Converts a number to a string.
     * @throws an exception if number is not a number.
     * @param number any number.
     * @param n the number of decimal places.
     * @return a string representing the number.
     **/
    template<typename numFmt>
    static std::string stringify(numFmt number, double n)
    {
        int rounded = qRound(number * pow(10, n));

        return stringify(rounded / pow(10, n));
    }

    static bool isValid(const QFileInfo &fileInfo);
    static bool isSavable(const QString &fileName);
    static bool hasValidSuffix(const QString &fileName);
    static QStringList suffixOnly(const QStringList &fileFilters);
    static QDateTime getConvertableDate(const QString &date);
    static QDateTime convertDate(const QString &date, const QFileInfo &file = QFileInfo());
    static QString convertDateString(const QString &date, const QFileInfo &file = QFileInfo());
    static QString formatToString(int format);
    static QString cleanFraction(const QString &frac);
    static QString resolveFraction(const QString &frac);
    static std::wstring qStringToStdWString(const QString &str);
    static QString stdWStringToQString(const std::wstring &str);

    static std::string stringTrim(const std::string str)
    {
        std::string strT = str;

        if (strT.length() <= 1)
            return strT; // .empty() may result in errors

        // remove whitespace
        size_t b = strT.find_first_not_of(" ");
        size_t e = strT.find_last_not_of(" ");
        strT = strT.substr(b, e + 1);

        if (strT.length() <= 1)
            return strT; // nothing to trim left

        // remove tabs
        b = strT.find_first_not_of("\t");
        e = strT.find_last_not_of("\t");
        strT = strT.substr(b, e + 1);

        return strT;
    };

    static std::string stringRemove(const std::string str, const std::string repStr)
    {
        std::string strR = str;

        if (strR.length() <= 1)
            return strR;

        size_t pos = 0;

        while ((pos = strR.find_first_of(repStr)) < strR.npos) {
            strR.erase(pos, repStr.length());
        }

        return strR;
    };
};

class DllCoreExport DkMemory
{
public:
    static double getTotalMemory();
    static double getFreeMemory();
};

class DllCoreExport DkFileNameConverter
{
public:
    DkFileNameConverter(const QString &fileName, const QString &pattern, int cIdx);

    QString getConvertedFileName();

protected:
    QString resolveFilename(const QString &tag) const;
    QString resolveIdx(const QString &tag) const;
    QString resolveExt(const QString &tag) const;
    int getIntAttribute(const QString &tag, int idx = 1) const;

    QString mFileName;
    QString mPattern;
    int mCIdx;
};

// from: http://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
class DllCoreExport DkRunGuard
{
public:
    DkRunGuard();
    ~DkRunGuard();

    bool tryRunning();

private:
    QString mSharedMemKey = "nomacs | run guard shared memory";
    QString mLockKey = "nomacs | run guard semaphore";

    QSharedMemory mSharedMem; /* gcc cannot deal with this: = mSharedMemKey;*/

    Q_DISABLE_COPY(DkRunGuard)
};

// from: http://qt-project.org/doc/qt-4.8/itemviews-simpletreemodel.html
class DllCoreExport TreeItem
{
public:
    TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    bool contains(const QRegularExpression &regExp, int column = 0, bool recursive = true) const;

    TreeItem *child(int row) const;
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    void setData(const QVariant &value, int column);
    int row() const;
    TreeItem *parent() const;
    TreeItem *find(const QVariant &value, int column);

    QStringList parentList() const;
    void setParent(TreeItem *parent);
    void clear();
    void remove(int rowIdx);

private:
    QVector<TreeItem *> childItems;
    QVector<QVariant> itemData;
    TreeItem *parentItem = 0;

    void parentList(QStringList &parentKeys) const;
};

class DllCoreExport TabMiddleMouseCloser : public QObject
{
    Q_OBJECT

public:
    TabMiddleMouseCloser(std::function<void(int)> callback)
        : callback(callback){};

protected:
    std::function<void(int)> callback;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

}
