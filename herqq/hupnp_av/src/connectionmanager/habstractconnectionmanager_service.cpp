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

#include "habstractconnectionmanager_service.h"
#include "habstractconnectionmanager_service_p.h"

#include "hconnectioninfo.h"
#include "hprotocolinforesult.h"
#include "hconnectionmanager_id.h"
#include "hconnectionmanager_info.h"
#include "hprepareforconnection_result.h"

#include "../hav_global_p.h"
#include "../common/hprotocolinfo.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HActionArguments>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractConnectionManagerServicePrivate
 ******************************************************************************/
HAbstractConnectionManagerServicePrivate::HAbstractConnectionManagerServicePrivate() :
    HServerServicePrivate()
{
}

HAbstractConnectionManagerServicePrivate::~HAbstractConnectionManagerServicePrivate()
{
}

qint32 HAbstractConnectionManagerServicePrivate::getProtocolInfo(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HProtocolInfoResult result;
    qint32 retVal = q->getProtocolInfo(&result);
    if (retVal == UpnpSuccess)
    {
        if (!result.source().isEmpty())
        {
            QString sourceProtocolInfos = strToCsvString(result.source());
            outArgs->setValue("Source", sourceProtocolInfos);
        }
        if (!result.sink().isEmpty())
        {
            QString sinkProtocolInfos = strToCsvString(result.sink());
            outArgs->setValue("Sink", sinkProtocolInfos);
        }
    }

    return retVal;
}

qint32 HAbstractConnectionManagerServicePrivate::prepareForConnection(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HPrepareForConnectionResult pfcResult;
    qint32 retVal = q->prepareForConnection(
        HProtocolInfo(inArgs.value("RemoteProtocolInfo").toString()),
        HConnectionManagerId(inArgs.value("PeerConnectionManager").toString()),
        inArgs.value("PeerConnectionID").toInt(),
        HConnectionManagerInfo::directionFromString(inArgs.value("Direction").toString()),
        &pfcResult);

    outArgs->setValue("ConnectionID", pfcResult.connectionId());
    outArgs->setValue("AVTransportID", pfcResult.avTransportId());
    outArgs->setValue("RcsID", pfcResult.rcsId());

    return retVal;
}

qint32 HAbstractConnectionManagerServicePrivate::connectionComplete(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractConnectionManagerService);

    return q->connectionComplete(inArgs.value("ConnectionID").toInt());
}

qint32 HAbstractConnectionManagerServicePrivate::getCurrentConnectionIDs(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QList<quint32> connectionIds;
    qint32 retVal = q->getCurrentConnectionIDs(&connectionIds);
    if (retVal == UpnpSuccess)
    {
        QString idsAsCsv = numToCsvString(connectionIds);
        outArgs->setValue("ConnectionIDs", idsAsCsv);
    }

    return retVal;
}

qint32 HAbstractConnectionManagerServicePrivate::getCurrentConnectionInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HConnectionInfo connectionInfo;
    qint32 retVal = q->getCurrentConnectionInfo(
        inArgs.value("ConnectionID").toInt(), &connectionInfo);

    if (retVal == UpnpSuccess && connectionInfo.isValid())
    {
        outArgs->setValue("RcsID", connectionInfo.rcsId());
        outArgs->setValue("AVTransportID", connectionInfo.avTransportId());
        outArgs->setValue("ProtocolInfo", connectionInfo.protocolInfo().toString());
        outArgs->setValue("PeerConnectionManager", connectionInfo.peerConnectionManager().toString());
        outArgs->setValue("PeerConnectionID", connectionInfo.peerConnectionId());
        outArgs->setValue("Direction", HConnectionManagerInfo::directionToString(connectionInfo.direction()));
        outArgs->setValue("Status", HConnectionManagerInfo::statusToString(connectionInfo.status()));
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractConnectionManagerService
 ******************************************************************************/
HAbstractConnectionManagerService::HAbstractConnectionManagerService(
    HAbstractConnectionManagerServicePrivate& dd) :
        HServerService(dd)
{
}

HAbstractConnectionManagerService::HAbstractConnectionManagerService() :
    HServerService(*new HAbstractConnectionManagerServicePrivate())
{
}

HAbstractConnectionManagerService::~HAbstractConnectionManagerService()
{
}

HServerService::HActionInvokes HAbstractConnectionManagerService::createActionInvokes()
{
    H_D(HAbstractConnectionManagerService);

    HActionInvokes retVal;

    retVal.insert("GetProtocolInfo",
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::getProtocolInfo));

    retVal.insert("PrepareForConnection",
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::prepareForConnection));

    retVal.insert("ConnectionComplete",
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::connectionComplete));

    retVal.insert("GetCurrentConnectionIDs",
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::getCurrentConnectionIDs));

    retVal.insert("GetCurrentConnectionInfo",
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::getCurrentConnectionInfo));

    return retVal;
}

qint32 HAbstractConnectionManagerService::prepareForConnection(
    const HProtocolInfo&, const HConnectionManagerId&, qint32,
    HConnectionManagerInfo::Direction, HPrepareForConnectionResult*)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractConnectionManagerService::connectionComplete(qint32)
{
    return UpnpOptionalActionNotImplemented;
}

}
}
}
