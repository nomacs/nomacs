/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hmisc_utils_p.h"

#include <QHostAddress>
#include <QNetworkInterface>

namespace Herqq
{

unsigned long hash(const char *str, int n)
{
    int c;
    unsigned long hash_value = 5381;

    while (n--)
    {
        c = *str++;
        hash_value = ((hash_value << 5) + hash_value) + c; // hash_value*33 + c
    }

    return hash_value;
}

QHostAddress findBindableHostAddress()
{
    QHostAddress address = QHostAddress::LocalHost;
    foreach (const QNetworkInterface& iface, QNetworkInterface::allInterfaces())
    {
        if (iface.flags() & QNetworkInterface::IsUp &&
          !(iface.flags() & QNetworkInterface::IsLoopBack))
        {
            QList<QNetworkAddressEntry> entries = iface.addressEntries();
            foreach(const QNetworkAddressEntry& entry, entries)
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    address = entry.ip();
                    goto end;
                }
            }
        }
    }

end:
    return address;
}

bool toBool(const QString& arg, bool* ok)
{
    bool retVal = false, match = true;
    if (arg == "1") { retVal = true; }
    else if (arg == "0") { retVal = false; }
    else if (arg.compare("true", Qt::CaseInsensitive) == 0) { retVal = true; }
    else if (arg.compare("false", Qt::CaseInsensitive) == 0) { retVal =  false; }
    else if (arg.compare("yes", Qt::CaseInsensitive) == 0) { retVal = true; }
    else if (arg.compare("no", Qt::CaseInsensitive) == 0) { retVal = false; }
    else { match = false; }

    if (ok) { *ok = match; }
    return retVal;
}

}
