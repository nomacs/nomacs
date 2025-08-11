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

#include <QDebug>
#include <QString>

namespace nmc
{

// DkTimer --------------------------------------------------------------------
/**
 * Initializes the class and stops the clock.
 **/
DkTimer::DkTimer()
{
    mTimer.start();
}

QDataStream &operator<<(QDataStream &s, const DkTimer &t)
{
    // this makes the operator<< virtual (stroustrup)
    return t.put(s);
}

QDebug operator<<(QDebug d, const DkTimer &timer)
{
    d << qPrintable(timer.stringifyTime(timer.elapsed()));
    return d;
}

/**
 * Returns a string with the total time interval.
 * The time interval is measured from the time,
 * the object was initialized.
 * @return the time in seconds or milliseconds.
 **/
QString DkTimer::getTotal() const
{
    return qPrintable(stringifyTime(mTimer.elapsed()));
}

QDataStream &DkTimer::put(QDataStream &s) const
{
    s << stringifyTime(mTimer.elapsed());

    return s;
}

/**
 * Converts time to QString.
 * @param ct current time interval
 * @return QString the time interval as string
 **/
QString DkTimer::stringifyTime(int ct) const
{
    if (ct < 2000)
        return QString::number(ct) + " ms";

    int v = qRound(ct / 1000.0);
    int sec = v % 60;
    v = qRound(v / 60.0);
    int min = v % 60;
    v = qRound(v / 60.0);
    int h = v % 24;
    v = qRound(v / 24.0);
    int d = v;

    QString ds = QString::number(d);
    QString hs = QString::number(h);
    QString mins = QString::number(min);
    QString secs = QString::number(sec);

    if (ct < 60000)
        return secs + " sec";

    if (min < 10)
        mins = "0" + mins;
    if (sec < 10)
        secs = "0" + secs;
    if (h < 10)
        hs = "0" + hs;

    if (ct < 3600000)
        return mins + ":" + secs;
    if (d == 0)
        return hs + ":" + mins + ":" + secs;

    return ds + "days" + hs + ":" + mins + ":" + secs;
}

void DkTimer::start()
{
    mTimer.restart();
}

int DkTimer::elapsed() const
{
    return mTimer.elapsed();
}
}
