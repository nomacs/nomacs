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

#include "hhttp_messaginginfo_p.h"

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HMessagingInfo
 ******************************************************************************/
HMessagingInfo::HMessagingInfo(
    QPair<QTcpSocket*, bool> sock, qint32 receiveTimeoutForNoData) :
        m_sock(), m_keepAlive(false),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData),
        m_chunkedInfo(), m_msecsToWaitOnSend(-1)
{
    m_sock = qMakePair(QPointer<QTcpSocket>(sock.first), sock.second);
}

HMessagingInfo::HMessagingInfo(
    QTcpSocket& sock, qint32 receiveTimeoutForNoData) :
        m_sock(), m_keepAlive(false),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData),
        m_chunkedInfo(), m_msecsToWaitOnSend(-1)
{
    m_sock = qMakePair(QPointer<QTcpSocket>(&sock), false);
}

HMessagingInfo::HMessagingInfo(
    QPair<QTcpSocket*, bool> sock, bool keepAlive, qint32 receiveTimeoutForNoData) :
        m_sock(), m_keepAlive(keepAlive),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData),
        m_msecsToWaitOnSend(-1)
{
    m_sock = qMakePair(QPointer<QTcpSocket>(sock.first), sock.second);
}

HMessagingInfo::HMessagingInfo(
    QTcpSocket& sock, bool keepAlive, qint32 receiveTimeoutForNoData) :
        m_sock(), m_keepAlive(keepAlive),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData),
        m_msecsToWaitOnSend(-1)
{
    m_sock = qMakePair(QPointer<QTcpSocket>(&sock), false);
}

HMessagingInfo::~HMessagingInfo()
{
    if (m_sock.second)
    {
        Q_ASSERT(!m_sock.first.isNull());
        m_sock.first->deleteLater();
    }
}

void HMessagingInfo::setHostInfo(const QUrl& hostInfo)
{
    QString tmp(hostInfo.host());

    if (hostInfo.port(0) > 0)
    {
        tmp.append(':').append(QString::number(hostInfo.port()));
    }

    m_hostInfo = tmp;
}

QString HMessagingInfo::hostInfo() const
{
    if (m_hostInfo.isEmpty())
    {
        // fall back to the ip address if no host information was provided.
        return QString("%1:%2").arg(
            socket().peerName(), QString::number(socket().peerPort()));
    }

    return m_hostInfo;
}

QString HMessagingInfo::lastErrorDescription() const
{
    return m_lastErrorDescription.isEmpty() ? socket().errorString() :
           m_lastErrorDescription;
}

}
}
