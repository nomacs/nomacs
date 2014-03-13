/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hconnectioninfo.h"
#include "hconnectionmanager_id.h"
#include "hconnectionmanager_info.h"

#include "../common/hprotocolinfo.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HConnectionInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HConnectionInfoPrivate)

public:

    qint32 m_connectionId;
    qint32 m_avTransportId;
    qint32 m_rcsId;
    HProtocolInfo m_protocolInfo;
    HConnectionManagerId m_peerConnectionManager;
    qint32  m_peerConnectionId;
    HConnectionManagerInfo::Direction m_direction;
    HConnectionManagerInfo::ConnectionStatus m_status;

    HConnectionInfoPrivate();
    HConnectionInfoPrivate(
        qint32 connectionId,
        qint32 avTransportId,
        qint32 rcsId,
        const HProtocolInfo& protocolInfo,
        const HConnectionManagerId& peerConnectionManager,
        qint32 peerConnectionId,
        HConnectionManagerInfo::Direction direction,
        HConnectionManagerInfo::ConnectionStatus status) :
            m_connectionId(connectionId < 0 ? -1 : connectionId),
            m_avTransportId(avTransportId),
            m_rcsId(rcsId),
            m_protocolInfo(protocolInfo),
            m_peerConnectionManager(peerConnectionManager),
            m_peerConnectionId(peerConnectionId),
            m_direction(direction),
            m_status(status)
    {
    }
};

HConnectionInfoPrivate::HConnectionInfoPrivate() :
    m_connectionId(-1),
    m_avTransportId(-1),
    m_rcsId(-1),
    m_protocolInfo(),
    m_peerConnectionManager(),
    m_peerConnectionId(-1),
    m_direction(HConnectionManagerInfo::DirectionUndefined),
    m_status(HConnectionManagerInfo::StatusUnknown)
{
}

/*******************************************************************************
 * HConnectionInfo
 ******************************************************************************/
HConnectionInfo::HConnectionInfo() :
    h_ptr(new HConnectionInfoPrivate())
{
}

HConnectionInfo::HConnectionInfo(
    qint32 connectionId, const HProtocolInfo& pinfo) :
        h_ptr(new HConnectionInfoPrivate())
{
    h_ptr->m_connectionId = connectionId < 0 ? -1 : connectionId;
    h_ptr->m_protocolInfo = pinfo;
}

HConnectionInfo::HConnectionInfo(
    qint32 connectionId,
    qint32 avTransportId,
    qint32 rcsId,
    const HProtocolInfo& protocolInfo,
    const HConnectionManagerId& peerConnectionManager,
    qint32 peerConnectionId,
    HConnectionManagerInfo::Direction direction,
    HConnectionManagerInfo::ConnectionStatus status) :
        h_ptr(new HConnectionInfoPrivate(
            connectionId, avTransportId, rcsId, protocolInfo, peerConnectionManager,
            peerConnectionId, direction, status))
{
}

HConnectionInfo::~HConnectionInfo()
{
}

HConnectionInfo::HConnectionInfo(const HConnectionInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HConnectionInfo& HConnectionInfo::operator=(const HConnectionInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

bool HConnectionInfo::isValid() const
{
    return h_ptr->m_connectionId >= 0 && h_ptr->m_protocolInfo.isValid();
}

qint32 HConnectionInfo::connectionId() const
{
    return h_ptr->m_connectionId;
}

qint32 HConnectionInfo::rcsId() const
{
    return h_ptr->m_rcsId;
}

qint32 HConnectionInfo::avTransportId() const
{
    return h_ptr->m_avTransportId;
}

HProtocolInfo HConnectionInfo::protocolInfo() const
{
    return h_ptr->m_protocolInfo;
}

HConnectionManagerId HConnectionInfo::peerConnectionManager() const
{
    return h_ptr->m_peerConnectionManager;
}

qint32 HConnectionInfo::peerConnectionId() const
{
    return h_ptr->m_peerConnectionId;
}

HConnectionManagerInfo::Direction HConnectionInfo::direction() const
{
    return h_ptr->m_direction;
}

HConnectionManagerInfo::ConnectionStatus HConnectionInfo::status() const
{
    return h_ptr->m_status;
}

void HConnectionInfo::setRcsId(qint32 arg)
{
    h_ptr->m_rcsId = arg;
}

void HConnectionInfo::setAvTransportId(qint32 arg)
{
    h_ptr->m_avTransportId = arg;
}

void HConnectionInfo::setPeerConnectionManager(const HConnectionManagerId& arg)
{
    h_ptr->m_peerConnectionManager = arg;
}

void HConnectionInfo::setPeerConnectionId(qint32 arg)
{
    h_ptr->m_peerConnectionId = arg;
}

void HConnectionInfo::setDirection(HConnectionManagerInfo::Direction arg)
{
    h_ptr->m_direction = arg;
}

void HConnectionInfo::setStatus(HConnectionManagerInfo::ConnectionStatus arg)
{
    h_ptr->m_status = arg;
}

void HConnectionInfo::setProtocolInfo(const HProtocolInfo& protocolInfo)
{
    h_ptr->m_protocolInfo = protocolInfo;
}

bool operator==(const HConnectionInfo& obj1, const HConnectionInfo& obj2)
{
    return obj1.avTransportId() == obj2.avTransportId() &&
           obj1.connectionId() == obj2.connectionId() &&
           obj1.direction() == obj2.direction() &&
           obj1.peerConnectionId() == obj2.peerConnectionId() &&
           obj1.peerConnectionManager() == obj2.peerConnectionManager() &&
           obj1.protocolInfo() == obj2.protocolInfo() &&
           obj1.rcsId() == obj2.rcsId() &&
           obj1.status() == obj2.status();
}

}
}
}
