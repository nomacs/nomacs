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

#ifndef HTTP_UTILS_H_
#define HTTP_UTILS_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HUpnp>

#include <QtCore/QString>

class QUrl;

template<typename T>
class QList;

class QTcpSocket;
class QByteArray;

namespace Herqq
{

namespace Upnp
{

//
//
//
class HHttpUtils
{
H_DISABLE_COPY(HHttpUtils)
HHttpUtils();

public:

    template<typename Hdr>
    static bool keepAlive(const Hdr& hdr)
    {
        QString connection = hdr.value("CONNECTION");
        if (hdr.minorVersion() == 1)
        {
            return connection.compare("close", Qt::CaseInsensitive) != 0;
        }

        return connection.compare("Keep-Alive", Qt::CaseInsensitive) == 0;
    }

    // returns the URLs as a string inside brackets. This is the format used in
    // UPnP eventing when subscribing to events.
    static QString callbackAsStr(const QList<QUrl>& callbacks);

    // the date format used in UPnP
    inline static QString rfc1123DateFormat()
    {
        QString retVal = "ddd, dd MMM yyyy HH:mm:ss";
        return retVal;
    }

    //
    // reads byte by byte to the target bytearray until \r\n\r\n is found,
    // in which case true is returned
    static bool readLines(QTcpSocket&, QByteArray& target, qint32 lineCount = 2);
};

}
}

#endif /* HTTP_UTILS_H_ */
