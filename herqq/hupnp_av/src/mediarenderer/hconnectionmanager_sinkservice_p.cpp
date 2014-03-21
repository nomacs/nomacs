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

#include "hconnectionmanager_sinkservice_p.h"
#include "hmediarenderer_device_p.h"

#include "../hav_global_p.h"
#include "../common/hprotocolinfo.h"

#include "../connectionmanager/hconnectioninfo.h"
#include "../connectionmanager/hprepareforconnection_result.h"
#include "../connectionmanager/hprotocolinforesult.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HConnectionManagerSinkService
 ******************************************************************************/
HConnectionManagerSinkService::HConnectionManagerSinkService() :
    HConnectionManagerService(), m_owner(0)
{
}

HConnectionManagerSinkService::~HConnectionManagerSinkService()
{
}

bool HConnectionManagerSinkService::finalizeInit(QString* errDescription)
{
    Q_UNUSED(errDescription)

    if (sinkProtocolInfo().isEmpty())
    {
        setSinkProtocolInfo(HProtocolInfo("http-get:*:*:*"));
    }

    HMediaRendererDevice* parentDev = qobject_cast<HMediaRendererDevice*>(parent());

    HRendererConnectionManager* manager =
        parentDev->configuration()->rendererConnectionManager();

    bool ok = connect(manager,
                      SIGNAL(connectionAdded(Herqq::Upnp::Av::HAbstractConnectionManagerService*, Herqq::Upnp::Av::HConnectionInfo)),
                      this,
                      SLOT(connectionAdded(Herqq::Upnp::Av::HAbstractConnectionManagerService*, Herqq::Upnp::Av::HConnectionInfo)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    return true;
}

void HConnectionManagerSinkService::connectionAdded(
    HAbstractConnectionManagerService* cmService,
    const Herqq::Upnp::Av::HConnectionInfo& connectionInfo)
{
    if (cmService == this)
    {
        addConnection(connectionInfo);
    }
}

bool HConnectionManagerSinkService::init(HMediaRendererDevice* owner)
{
   Q_ASSERT(owner);
   m_owner = owner;
   return true;
}

qint32 HConnectionManagerSinkService::prepareForConnection(
    const HProtocolInfo& remoteProtocolInfo,
    const HConnectionManagerId& peerConnectionManager,
    qint32 peerConnectionId,
    HConnectionManagerInfo::Direction direction,
    HPrepareForConnectionResult* result)
{
    Q_ASSERT(result);

    if (!actions().value("PrepareForConnection"))
    {
        return UpnpOptionalActionNotImplemented;
    }

    if (direction != HConnectionManagerInfo::DirectionInput)
    {
        return HConnectionManagerInfo::IncompatibleDirections;
    }

    if (remoteProtocolInfo.protocol().compare("http-get", Qt::CaseInsensitive) &&
        remoteProtocolInfo.protocol() != "*")
    {
        return HConnectionManagerInfo::IncompatibleProtocolInfo;
    }

    if (!isMimetypeValid(
        remoteProtocolInfo.contentFormat(), sinkProtocolInfo()))
    {
        return HConnectionManagerInfo::IncompatibleProtocolInfo;
    }

    qint32 connectionId = nextId();

    HConnectionInfo connectionInfo(
        connectionId,
        0,
        0,
        remoteProtocolInfo,
        peerConnectionManager,
        peerConnectionId,
        HConnectionManagerInfo::DirectionInput,
        HConnectionManagerInfo::StatusOk);

    qint32 errCode = m_owner->prepareForConnection(&connectionInfo);

    if (errCode == UpnpSuccess)
    {
        result->setAvTransportId(connectionInfo.avTransportId());
        result->setConnectionId(connectionInfo.connectionId());
        result->setRcsId(connectionInfo.rcsId());
    }

    return errCode;
}

qint32 HConnectionManagerSinkService::connectionComplete(qint32 connectionId)
{
    if (!actions().value("ConnectionComplete"))
    {
        return UpnpOptionalActionNotImplemented;
    }

    if (!connectionIds().contains(connectionId))
    {
        return HConnectionManagerInfo::InvalidConnectionReference;
    }

    m_owner->connectionComplete(connectionId);

    HRendererConnectionManager* manager =
        m_owner->configuration()->rendererConnectionManager();

    if (manager->removeConnection(this, connectionId))
    {
        return UpnpSuccess;
    }

    return UpnpInvalidArgs;
}

qint32 HConnectionManagerSinkService::getProtocolInfo(
    HProtocolInfoResult* retVal)
{
    if (retVal)
    {
        retVal->setSource(sourceProtocolInfo());
        retVal->setSink(sinkProtocolInfo());
        return UpnpSuccess;
    }

    return UpnpInvalidArgs;
}

qint32 HConnectionManagerSinkService::getCurrentConnectionIDs(
    QList<quint32>* retVal)
{
    if (retVal)
    {
        HRendererConnectionManager* manager =
            m_owner->configuration()->rendererConnectionManager();

        QList<HRendererConnection*> connections = manager->connections(this);

        retVal->clear();

        foreach(HRendererConnection* connection, connections)
        {
            retVal->append(connection->connectionInfo()->connectionId());
        }

        return UpnpSuccess;
    }

    return UpnpInvalidArgs;
}

qint32 HConnectionManagerSinkService::getCurrentConnectionInfo(
    qint32 connectionId, HConnectionInfo* retVal)
{
    if (retVal)
    {
        HRendererConnectionManager* manager =
            m_owner->configuration()->rendererConnectionManager();

        HRendererConnection* connection = manager->connection(this, connectionId);

        if (connection)
        {
            *retVal = *connection->connectionInfo();
            return UpnpSuccess;
        }
    }

    return UpnpInvalidArgs;
}

qint32 HConnectionManagerSinkService::nextId()
{
    return ++(lastConnectionId());
}

}
}
}
