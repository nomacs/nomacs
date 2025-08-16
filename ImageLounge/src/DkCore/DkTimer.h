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

#include <QElapsedTimer>
#include <QObject>

#include "nmc_config.h"

namespace nmc
{

/**
 * A small class which measures the time.
 * This class is designed to measure the time of a method, especially
 * intervals and the total time can be measured.
 **/
class DllCoreExport DkTimer : public QObject
{
    Q_OBJECT

public:
    /**
     * Initializes the class and stops the clock.
     **/
    DkTimer();
    ~DkTimer() override = default;

    friend DllCoreExport QDataStream &operator<<(QDataStream &s, const DkTimer &t);
    friend DllCoreExport QDebug operator<<(QDebug d, const DkTimer &timer);

    QString getTotal() const;
    virtual QDataStream &put(QDataStream &s) const;
    QString stringifyTime(int ct) const;
    int elapsed() const;
    void start();

protected:
    QElapsedTimer mTimer;
};

}
