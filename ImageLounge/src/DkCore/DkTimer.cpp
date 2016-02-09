/*******************************************************************************************************
DkTimer.cpp
Created on:	30.08.2015

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

#include "DkTimer.h"

#include "DkUtils.h"
#include "DkMath.h"
#include "DkTimer.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QString>
#include <qmath.h>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

DkTimer::DkTimer() {
	firstTick = clock();
	lastTick = firstTick;
}

QString DkTimer::getTotal() {
	lastTick = clock();
	double ct = (double) (lastTick-firstTick) / CLOCKS_PER_SEC;

	return stringifyTime(ct);
}

double DkTimer::getTotalTime() {

	lastTick = clock();
	return (double) (lastTick-firstTick) / CLOCKS_PER_SEC;
}

QString DkTimer::getIvl() {
	
	clock_t tmp = clock();
	double ct = (double) (tmp-lastTick) / CLOCKS_PER_SEC;
	lastTick = tmp;

	return stringifyTime(ct);
}

QString DkTimer::stringifyTime(double ct) {

	std::string msg = " ";

	if (ct < 1)
		msg += DkUtils::stringify(ct * 1000) + " ms";
	else if (ct < 60)
		msg += DkUtils::stringify(ct) + " sec";
	else if (ct < 3600) {
		double m = qFloor((float)(ct / 60.0));
		msg += DkUtils::stringify(m) + " min " + DkUtils::stringify(ct - m * 60, 0) + " sec";
	}
	else {
		double h = qFloor((float)(ct / 3600.0));
		msg += DkUtils::stringify(h) + " hours " + DkUtils::stringify(ct - h*3600.0f, 0) + " min";
	}

	return QString::fromStdString(msg);

}
void DkTimer::stop() {
	lastTick = clock();
}
void DkTimer::start() {
	firstTick = clock();
	lastTick = firstTick;
}
double DkTimer::getTime() {
	return (double) clock();
}
}