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

#include "hrendererconnection_manager.h"
#include "hrendererconnection_manager_p.h"

#include "hrendererconnection.h"

#include <HUpnpCore/HServerStateVariable>

#include "../connectionmanager/hconnectioninfo.h"
#include "../connectionmanager/habstractconnectionmanager_service.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRendererConnectionManagerPrivate
 ******************************************************************************/
HRendererConnectionManagerPrivate::HRendererConnectionManagerPrivate() :
    m_connections()
{
}

/*******************************************************************************
 * HRendererConnectionManager
 ******************************************************************************/
HRendererConnectionManager::HRendererConnectionManager(QObject* parent) :
    QObject(parent),
        h_ptr(new HRendererConnectionManagerPrivate())
{
}

HRendererConnectionManager::~HRendererConnectionManager()
{
    delete h_ptr;
}

void HRendererConnectionManager::destroyed_(QObject* obj)
{
    Connections::iterator it = h_ptr->m_connections.begin();
    for(; it != h_ptr->m_connections.end(); ++it)
    {
        if (it->second == obj)
        {
            qint32 cid = it->first.second;
            h_ptr->m_connections.erase(it);
            emit connectionRemoved(it->first.first, cid);
            break;
        }
    }
}

HRendererConnection* HRendererConnectionManager::createAndAdd(
    HAbstractConnectionManagerService* cmService,
    const HConnectionInfo& connectionInfo)
{
    if (connection(cmService, connectionInfo.connectionId()))
    {
        return 0;
    }

    HConnectionInfo* copy = new HConnectionInfo(connectionInfo);
    HRendererConnection* connection = doCreate(cmService, copy);
    if (!connection)
    {
        return 0;
    }
    else
    {
        connection->init(cmService, copy);
        if (!connection->parent())
        {
            connection->setParent(this);
        }
    }

    bool ok = connect(connection, SIGNAL(destroyed(QObject*)), this, SLOT(destroyed_(QObject*)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    connection->finalizeInit();

    h_ptr->m_connections.append(
        qMakePair(qMakePair(cmService, connectionInfo.connectionId()), connection));

    emit connectionAdded(cmService, connectionInfo);

    return connection;
}

bool HRendererConnectionManager::removeConnection(
    const HAbstractConnectionManagerService* cmService, qint32 cid)
{
    Connections::iterator it;
    for(it = h_ptr->m_connections.begin(); it != h_ptr->m_connections.end(); ++it)
    {
        Connection connection = *it;
        if (connection.first.first == cmService && connection.first.second == cid)
        {
            h_ptr->m_connections.erase(it);
            return true;
        }
    }

    return false;
}

HRendererConnection* HRendererConnectionManager::connection(
    HAbstractConnectionManagerService* cmService, qint32 cid) const
{
    foreach(Connection connection, h_ptr->m_connections)
    {
        if (connection.first.first == cmService && connection.first.second == cid)
        {
            return connection.second;
        }
    }
    return 0;
}

QList<HRendererConnection*> HRendererConnectionManager::connections(
    HAbstractConnectionManagerService* cmService) const
{
    QList<HRendererConnection*> retVal;

    foreach(Connection connection, h_ptr->m_connections)
    {
        if (connection.first.first == cmService)
        {
            retVal.append(connection.second);
        }
    }

    return retVal;
}

bool HRendererConnectionManager::connectionComplete(
    HAbstractConnectionManagerService* cmService, qint32 connectionId)
{
    Connections::iterator it = h_ptr->m_connections.begin();
    for(; it != h_ptr->m_connections.end(); ++it)
    {
        if (it->first.first == cmService && it->first.second == connectionId)
        {
            HRendererConnection* conn =  it->second;
            h_ptr->m_connections.erase(it);
            conn->dispose();
            emit connectionRemoved(it->first.first, connectionId);
            return true;
        }
    }
    return false;
}

}
}
}
