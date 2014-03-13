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

#include "hendpoint.h"
#include "../utils/hmisc_utils_p.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HEndpoint
 ******************************************************************************/
HEndpoint::HEndpoint(const QHostAddress& hostAddress, quint16 portNumber) :
    m_hostAddress(hostAddress),
    m_portNumber(hostAddress == QHostAddress::Null ? (quint16)0 : portNumber)
{
}

HEndpoint::HEndpoint(const QHostAddress& hostAddress) :
    m_hostAddress(hostAddress), m_portNumber(0)
{
}

HEndpoint::HEndpoint() :
    m_hostAddress(QHostAddress::Null), m_portNumber(0)
{
}

HEndpoint::HEndpoint(const QUrl& url) :
    m_hostAddress(QHostAddress(url.host())),
    m_portNumber(m_hostAddress == QHostAddress::Null ? (quint16)0 : url.port())
{
}

HEndpoint::HEndpoint(const QString& arg) :
    m_hostAddress(), m_portNumber(0)
{
    qint32 delim = arg.indexOf(':');
    if (delim < 0)
    {
        m_hostAddress = arg;
    }
    else
    {
        m_hostAddress = arg.left(delim);
        if (m_hostAddress == QHostAddress::Null)
        {
            m_portNumber = 0;
        }
        else
        {
            m_portNumber = arg.mid(delim+1).toUShort();
        }
    }
}

HEndpoint::~HEndpoint()
{
}

bool HEndpoint::isMulticast() const
{
    quint32 ipaddr = m_hostAddress.toIPv4Address();
    return ((ipaddr & 0xe0000000) == 0xe0000000) ||
           ((ipaddr & 0xe8000000) == 0xe8000000) ||
           ((ipaddr & 0xef000000) == 0xef000000);
}

QString HEndpoint::toString() const
{
    return isNull() ? QString() :
        m_hostAddress.toString().append(":").append(QString::number(m_portNumber));
}

bool operator==(const HEndpoint& ep1, const HEndpoint& ep2)
{
    return ep1.m_hostAddress == ep2.m_hostAddress &&
           ep1.m_portNumber  == ep2.m_portNumber;
}

quint32 qHash(const HEndpoint& key)
{
    quint32 tmp = key.m_hostAddress.toIPv4Address() ^ key.portNumber();
    return hash(reinterpret_cast<char*>(&tmp), sizeof(tmp));
}

}
}
