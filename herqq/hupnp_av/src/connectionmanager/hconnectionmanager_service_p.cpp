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

#include "hconnectionmanager_service_p.h"

#include "hprotocolinforesult.h"
#include "hconnectionmanager_id.h"
#include "hconnectionmanager_info.h"

#include "../hav_global_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HConnectionManagerService
 ******************************************************************************/
HConnectionManagerService::HConnectionManagerService() :
    HAbstractConnectionManagerService(),
        m_sinkProtocolInfo(),
        m_sourceProtocolInfo(),
        m_currentConnectionIDs(),
        m_lastConnectionId(-1)
{
}

HConnectionManagerService::~HConnectionManagerService()
{
}

void HConnectionManagerService::updateConnectionsList()
{
    QString newStateVarValue = numToCsvString(m_currentConnectionIDs.keys());
    HServerStateVariable* sv = stateVariables().value("CurrentConnectionIDs");
    if (sv->value() != newStateVarValue)
    {
        bool ok = sv->setValue(newStateVarValue);
        Q_ASSERT(ok); Q_UNUSED(ok)
    }
}

qint32 HConnectionManagerService::getProtocolInfo(HProtocolInfoResult* result)
{
    H_ASSERT(result);

    bool ok = result->setSource(m_sourceProtocolInfo);
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = result->setSink(m_sinkProtocolInfo);
    Q_ASSERT(ok);

    return UpnpSuccess;
}

qint32 HConnectionManagerService::getCurrentConnectionIDs(QList<quint32>* oarg)
{
    Q_ASSERT(oarg);
    *oarg = m_currentConnectionIDs.keys();
    return UpnpSuccess;
}

qint32 HConnectionManagerService::getCurrentConnectionInfo(
    qint32 connectionId, HConnectionInfo* oarg)
{
    Q_ASSERT(oarg);

    if (m_currentConnectionIDs.contains(connectionId))
    {
        *oarg = *m_currentConnectionIDs.value(connectionId);
        return UpnpSuccess;
    }

    return HConnectionManagerInfo::InvalidConnectionReference;
}

void HConnectionManagerService::removeConnection(qint32 connectionId)
{
    m_currentConnectionIDs.remove(connectionId);
    updateConnectionsList();
}

void HConnectionManagerService::addConnection(const HConnectionInfo& connection)
{
    Q_ASSERT(connection.isValid());
    if (!m_currentConnectionIDs.contains(connection.connectionId()))
    {
        m_currentConnectionIDs.insert(
            connection.connectionId(),
            QSharedPointer<HConnectionInfo>(new HConnectionInfo(connection)));
    }
    updateConnectionsList();
}

QSharedPointer<HConnectionInfo> HConnectionManagerService::createDefaultConnection(
    const HProtocolInfo& pinfo)
{
    Q_ASSERT(m_currentConnectionIDs.size() == 0);

    QSharedPointer<HConnectionInfo> connectionInfo =
        QSharedPointer<HConnectionInfo>(new HConnectionInfo(0, pinfo));

    connectionInfo->setDirection(HConnectionManagerInfo::DirectionOutput);
    connectionInfo->setStatus(HConnectionManagerInfo::StatusOk);

    m_currentConnectionIDs.insert(0, connectionInfo);
    return connectionInfo;
}

void HConnectionManagerService::setSourceProtocolInfo(const HProtocolInfo& arg)
{
    HProtocolInfos infos; infos.append(arg);
    setSourceProtocolInfo(infos);
}

void HConnectionManagerService::setSinkProtocolInfo(const HProtocolInfo& arg)
{
    HProtocolInfos infos; infos.append(arg);
    setSinkProtocolInfo(infos);
}

void HConnectionManagerService::setSourceProtocolInfo(const HProtocolInfos& arg)
{
    m_sourceProtocolInfo = arg;

    QString sourceProtocolInfos = strToCsvString(m_sourceProtocolInfo);
    HServerStateVariable* sv = stateVariables().value("SourceProtocolInfo");
    bool ok = sv->setValue(sourceProtocolInfos);
    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HConnectionManagerService::setSinkProtocolInfo(const HProtocolInfos& arg)
{
    m_sinkProtocolInfo = arg;

    QString sinkProtocolInfos = strToCsvString(m_sinkProtocolInfo);
    HServerStateVariable* sv = stateVariables().value("SinkProtocolInfo");
    bool ok = sv->setValue(sinkProtocolInfos);
    Q_ASSERT(ok); Q_UNUSED(ok)
}

bool HConnectionManagerService::isMimetypeValid(
    const QString& contentFormat, const HProtocolInfos& protocolInfos)
{
    foreach(const HProtocolInfo& pinfo, protocolInfos)
    {
        if (pinfo.contentFormat() == contentFormat ||
            pinfo.contentFormat() == "*")
        {
            return true;
        }
    }

    return false;
}

}
}
}
