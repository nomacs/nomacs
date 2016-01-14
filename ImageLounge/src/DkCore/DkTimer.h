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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QObject>
#pragma warning(pop)		// no warnings from includes - end

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

namespace nmc {

/**
 * A small class which measures the time.
 * This class is designed to measure the time of a method, especially
 * intervals and the total time can be measured.
 **/
class DllCoreExport DkTimer {

protected:
	clock_t firstTick;	/**< the first tick**/
	clock_t	lastTick;	/**< the last tick**/

public:

	/**
	 * Initializes the class and stops the clock.
	 **/
	DkTimer();

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
	QString getTotal();

	double getTotalTime();

	/**
	 * Returns a string with the time interval.
	 * The time interval since the last call of stop(), getIvl()
	 * or getTotal().
	 * @return the time in seconds or milliseconds.
	 **/
	QString getIvl();

	/**
	 * Converts time to QString.
	 * @param ct current time interval
	 * @return QString the time interval as string
	 **/ 
	QString stringifyTime(double ct);

	/**
	 * Stops the clock.
	 **/
	void stop();

	void start();

	/**
	 * Returns the current time.
	 * @return double current time in seconds.
	 **/ 
	double static getTime();
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
