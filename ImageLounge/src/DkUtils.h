/*******************************************************************************************************
 DkUtils.h
 Created on:	05.02.2010
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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

#include <QString>
#include <QFileInfo>
#include <QDate>
#include <QRegExp>
#include <QStringList>

#include <cmath>
#include <sstream>
#include <stdarg.h>

// for svm params
#include <iostream>
#include <fstream>

#include "DkError.h"

#ifdef WIN32
	#include <wtypes.h>

#endif

#ifdef WITH_OPENCV
#include <opencv/cv.h>
using namespace cv;
#else

//#define int64 long long;
#define CV_PI 3.141592653589793238462643383279

#endif

namespace nmc {

enum morphTypes {DK_ERODE=0, DK_DILATE};
enum DebugLevel {DK_NONE=0,DK_WARNING, DK_MODULE, DK_DEBUG_A, DK_DEBUG_B, DK_DEBUG_C, DK_DEBUG_ALL};
enum SpeedLebel {DK_NO_SPEED_UP=0, DK_SPEED_UP, DK_APPROXIMATE};

/**
 * This class contains general functions which are useful.
 **/
class DkUtils {

private:
	static int debugLevel;

public:

#ifdef WITH_OPENCV
	/**
	 * Prints a matrix to the standard output.
	 * This is especially useful for copy and pasting e.g.
	 * histograms to matlab and visualizing them there.
	 * @param src an image CV_32FC1.
	 * @param varName the variable name for Matlab.
	 **/
	static void printMat(const Mat src, const char* varName) {

		if (src.depth() != CV_32FC1) {
			//qDebug() << "I could not visualize the mat: " << QString::fromAscii(varName);
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
	static std::string printMat(const Mat src) {

		if (src.depth() != CV_32FC1) {

			//qDebug() << "I could not visualize the mat: " << QString::fromStdString(DkUtils::getMatInfo(src));
			return "";
		}

		std::string msg = " [";	// matlab...

		int cnt = 0;
		
		for (int rIdx = 0; rIdx < src.rows; rIdx++) {
			
			const float* srcPtr = src.ptr<float>(rIdx);
			
			for (int cIdx = 0; cIdx < src.cols; cIdx++, cnt++) {
							

				msg += DkUtils::stringify(srcPtr[cIdx], 3);

				msg += (cIdx < src.cols-1) ? " " : "; "; // next row matlab?
				
				if (cnt % 7 == 0)
					msg += "...\n";
			}

		}
		msg += "];\n";

		return msg;
	}

	/**
	 * Prints the Mat's attributes to the standard output.
	 * The Mat's attributes are: size, depth, number of channels and
	 * dynamic range.
	 * @param img an image (if it has more than one channel, the dynamic range
	 * is not displayed)
	 * @param varname the name of the matrix
	 **/
	static void getMatInfo(Mat img, std::string varname) {

		printf("%s: %s\n", varname.c_str(), getMatInfo(img).c_str());
	}

	/**
	 * Converts the Mat's attributes to a string.
	 * The Mat's attributes are: size, depth, number of channels and
	 * dynamic range.
	 * @param img an image (if it has more than one channel, the dynamic range
	 * is not converted).
	 * @return a string with the Mat's attributes.
	 **/
	static std::string getMatInfo(Mat img) {

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
		}
		else
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
	static std::string createFileName(std::string fName, std::string attribute, std::string ext = "") {

		if (ext == "") ext = fName.substr(fName.length()-4, fName.length()); // use the old extension

		// generate: img0001.tif -> img0001_mask.tif
		return fName.substr(0, fName.length()-4) + attribute + ext;
	}

	static std::string removeExtension(std::string fName) {

		return fName.substr(0, fName.find_last_of("."));
	}

	static std::string getFileNameFromPath(std::string fName) {
		return fName.substr(fName.find_last_of("/")+1); //TODO: Schiach!!
	}

	/**
	 * Converts a number to a string.
	 * @throws an exception if number is not a number.
	 * @param number any number.
	 * @return a string representing the number.
	 **/
	template <typename numFmt>
	static std::string stringify(numFmt number) {

		std::stringstream stream;
		if (! (stream << number)) {
			std::string msg = "Sorry, I could not cast it to a string";
			//throw DkCastException(msg, __LINE__, __FILE__);
			printf("%s", msg.c_str());	// TODO: we need a solution for DkSnippet here...
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
	template <typename numFmt>
	static std::string stringify(numFmt number, double n) {

		int rounded = cvRound(number * std::pow(10,n));

		return stringify(rounded/std::pow(10,n));
	};

	static QString convertDate(const QString& date, const QFileInfo& file = QFileInfo()) {
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

#ifdef WIN32
	static LPCWSTR stringToWchar(std::string str) {
		wchar_t *wChar = new wchar_t[(int)str.length()+1];
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wChar, str.length()+1, str.c_str(), _TRUNCATE);
		//mbstowcs(wChar, str.c_str(), str.length()+1);

		return (LPCWSTR)wChar;
	};
#endif

#ifdef linux
	static const char* stringToWchar(std::string str) {
		return str.c_str();
	};
#endif

	static std::string stringTrim(const std::string str) {


		std::string strT = str;

		if (strT.length() <= 1) return strT;	// .empty() may result in errors

		// remove whitespace
		size_t b = strT.find_first_not_of(" ");
		size_t e = strT.find_last_not_of(" ");
		strT = strT.substr(b, e+1);

		if (strT.length() <= 1) return strT;	// nothing to trim left

		// remove tabs
		b = strT.find_first_not_of("\t");
		e = strT.find_last_not_of("\t");
		strT = strT.substr(b, e+1);

		return strT;
	};

	static std::string stringRemove(const std::string str, const std::string repStr) {

		std::string strR = str;

		if (strR.length() <= 1) return strR;

		size_t pos = 0;

		while ((pos = strR.find_first_of(repStr)) < strR.npos) {
			
			strR.erase(pos, repStr.length());
		}

		return strR;
	};

	/**
	 * Sets the actual debug level.
	 * @param l the debug level of the application.
	 **/
	static void setDebug(int l) {
		debugLevel = l;
	};

	/**
	 * Returns the current debug level.
	 * @return the debug level of the application.
	 **/
	static int getDebug() {
		return debugLevel;
	};




#ifdef DK_DEBUG
	
	/**
	 * Prints a debug message according the message level and the current debug level defined in DkUtils.
	 * The debug command prints only a message if DK_DEBUG is defined.
	 * Debug levels are: DK_NONE=0,DK_WARNING, DK_MODULE, DK_DEBUG_A, DK_DEBUG_B, DK_DEBUG_C, DK_DEBUG_ALL.
	 * @param level the debug level of the message.
	 * @param fmt the format string of the message.
	 **/
	static void printDebug(int level,const char *fmt,...) {
		va_list ap;


		if (debugLevel >= DK_WARNING && level == DK_WARNING)
			printf("WARNING: ");
		else if (debugLevel >= DK_MODULE && level == DK_MODULE)
			printf(">> ");

		va_start(ap,fmt);
		if ((fmt) && (level <= debugLevel)) {
			vprintf(fmt, ap);
			fflush(stdout);
		}
		va_end(ap);
	}
#else
	/**
	 * If DK_DEBUG is undefined do nothing.
	 **/
	inline static void printDebug(...) {};
#endif

};
};