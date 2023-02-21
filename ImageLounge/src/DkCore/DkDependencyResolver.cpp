/*******************************************************************************************************
 DkDependencyResolver.cpp
 Created on:	13.07.2016

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

#include "DkDependencyResolver.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <limits>
#include <QByteArrayMatcher>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

DkDllDependency::DkDllDependency(const QString &filePath)
{
    mFilePath = filePath;
}

bool DkDllDependency::findDependencies()
{
    if (mFilePath.isEmpty()) {
        qWarning() << "cannot find dependenies - dll path is empty...";
        return false;
    }

    QFile dllFile(mFilePath);

    if (!dllFile.open(QIODevice::ReadOnly)) {
        qWarning() << "cannot open" << mFilePath << "for read...";
        return false;
    }

    mDependencies.clear();

    QByteArray ba(dllFile.readAll());
    dllFile.close();

    QString myName = QFileInfo(mFilePath).fileName();
    QVector<int> dllLocations = markerLocations(ba, marker());

    for (int l : dllLocations) {
        QString n = resolveName(ba, l);

        if (!n.isEmpty() && n != myName) {
            mDependencies << n;
        } else if (n.isEmpty())
            qWarning() << "I could not resolve the name at location" << l;
    }

    return true;
}

QStringList DkDllDependency::filteredDependencies() const
{
    QStringList fd;
    QRegExp re(filter());

    for (const QString &n : mDependencies) {
        if (re.exactMatch(n)) {
            fd << n;
        }
    }

    return fd;
}

QStringList DkDllDependency::dependencies() const
{
    return mDependencies;
}

QString DkDllDependency::filePath() const
{
    return mFilePath;
}

QString DkDllDependency::filter()
{
    static QString filter = "(opencv.*|Read.*|libDk.*)";

    return filter;
}

QByteArray DkDllDependency::marker()
{
    static QByteArray m(".dll");
    return m;
}

QVector<int> DkDllDependency::markerLocations(const QByteArray &ba, const QByteArray &marker) const
{
    QByteArrayMatcher matcher(marker);
    QVector<int> locations;

    for (int offset = 0; offset < ba.size();) {
        int idx = matcher.indexIn(ba, offset);

        if (idx == -1)
            break;

        offset = idx + 1;
        locations << idx;
    }

    return locations;
}

QString DkDllDependency::resolveName(const QByteArray &ba, int location) const
{
    // dll names are terminated with NULL on both sides - so find their index...
    int start = -1;
    for (int idx = location; idx > 0; idx--) {
        if (isStopCharacter(ba[idx])) {
            start = idx;
            break;
        }
    }

    if (start == -1)
        return QString();

    int end = -1;
    for (int idx = location; idx < ba.size(); idx++) {
        if (isStopCharacter(ba[idx])) {
            end = idx;
            break;
        }
    }

    QString name(ba.mid(start + 1, end - start));

    return name;
}

bool DkDllDependency::isStopCharacter(const char &val) const
{
    return val == '\0' || val == '\u0001' || val == '\u0003' || val == '\u0006'; // NULL || SOH || ETX || ACK
}

}
