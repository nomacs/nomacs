/*******************************************************************************************************
 DkTimer.h
 Created on:	09.02.2010
 
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
#include <time.h>
#include "DkMath.h"
#include "DkUtils.h"

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

namespace nmc {

/**
 * A small class which measures the time.
 * This class is designed to measure the time of a method, especially
 * intervals and the total time can be measured.
 **/
class DllExport DkTimer {

protected:
	clock_t firstTick;	/**< the first tick**/
	clock_t	lastTick;	/**< the last tick**/

public:

	/**
	 * Initializes the class and stops the clock.
	 **/
	DkTimer() {
		firstTick = clock();
		lastTick = firstTick;
	};

	/**
	 * Default destructor.
	 **/
	~DkTimer() {};

	/**
	 * Returns a string with the total time interval.
	 * The time interval is measured from the time,
	 * the object was initialized.
	 * @return the time in seconds or milliseconds.
	 **/
	QString getTotal() {
		lastTick = clock();
		double ct = (double) (lastTick-firstTick) / CLOCKS_PER_SEC;

		return stringifyTime(ct);
	};

	double getTotalTime() {

		lastTick = clock();
		return (double) (lastTick-firstTick) / CLOCKS_PER_SEC;
	}

	/**
	 * Returns a string with the time interval.
	 * The time interval since the last call of stop(), getIvl()
	 * or getTotal().
	 * @return the time in seconds or milliseconds.
	 **/
	QString getIvl() {
		clock_t tmp = clock();
		double ct = (double) (tmp-lastTick) / CLOCKS_PER_SEC;
		lastTick = tmp;

		return stringifyTime(ct);
	};



	/**
	 * Converts time to QString.
	 * @param ct current time interval
	 * @return QString the time interval as string
	 **/ 
	QString stringifyTime(double ct) {

		std::string msg = " ";

		if (ct < 1)
			msg += DkUtils::stringify(ct*1000) + " ms";
		else if (ct < 60)
			msg += DkUtils::stringify(ct) + " sec";
		else if (ct < 3600) {
			double m = cvFloor((float)(ct/60.0));
			msg += DkUtils::stringify(m) + " min " + DkUtils::stringify(ct-m*60, 0) + " sec";
		}
		else {
			double h = cvFloor((float)(ct/3600.0));
			msg += DkUtils::stringify(h) + " hours " + DkUtils::stringify(ct-h*3600.0f, 0) + " min";
		}

		return QString::fromStdString(msg);

	};

	/**
	 * Stops the clock.
	 **/
	void stop() {
		lastTick = clock();
	};

	void start() {
		firstTick = clock();
		lastTick = firstTick;
	};

	/**
	 * Returns the current time.
	 * @return double current time in seconds.
	 **/ 
	double static getTime() {
		return (double) clock();
	};
};


/**
 * A small class which measures the time.
 * This class is designed to measure the time of a method, especially
 * intervals and the total time can be measured.
 **/
class DkIvlTimer : public DkTimer {

private:
	clock_t timeIvl;

public:

	/**
	 * Initializes the class and stops the clock.
	 **/
	DkIvlTimer() : DkTimer() {
		timeIvl = 0;
	};

	/**
	 * Default destructor.
	 **/
	~DkIvlTimer() {};

	/**
	 * Divides the time interval by the specified value.
	 * @param val the number of calls
	 **/ 
	void operator/= (const int &val) {

		timeIvl /= (clock_t)val;
	};


	/**
	 * Returns a string with the time interval.
	 * The time interval of all start() stop() calls.
	 * @return the time in seconds or milliseconds.
	 **/
	QString getIvl() {
		
		double ct = (double) (timeIvl) / CLOCKS_PER_SEC;
		
		// return the interval in ms or sec depending on the interval's length
		return stringifyTime(ct);
	};

	/**
	 * Starts the clock.
	 **/ 
	void start() {
		lastTick = clock();
	};

	/**
	 * Stops the clock.
	 **/
	void stop() {
		clock_t cTime = clock();
		timeIvl += cTime-lastTick;
		lastTick = cTime;
	};
};
};
