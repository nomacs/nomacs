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

#include "hhttp_utils_p.h"

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtNetwork/QTcpSocket>

namespace Herqq
{

namespace Upnp
{

QString HHttpUtils::callbackAsStr(const QList<QUrl>& callbacks)
{
    QString retVal;

    foreach(const QUrl& cb, callbacks)
    {
        retVal.append(QString("<%1>").arg(cb.toString()));
    }

    return retVal;
}

bool HHttpUtils::readLines(
    QTcpSocket& socket, QByteArray& target, qint32 lineCount)
{
    char readChar = 0;
    qint32 linesRead = 0;

    while(linesRead < lineCount && socket.getChar(&readChar))
    {
        target.push_back(readChar);

        if (readChar != '\r')
        {
            if (linesRead > 0) { linesRead = 0; }
            continue;
        }

        if (socket.getChar(&readChar))
        {
            target.push_back(readChar);

            if (readChar == '\n')
            {
                ++linesRead;
            }
            else if (linesRead > 0)
            {
                linesRead = 0;
            }
        }
    }

    return linesRead >= lineCount;
}

}
}
