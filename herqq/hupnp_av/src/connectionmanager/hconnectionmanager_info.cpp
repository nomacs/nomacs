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

#include "hconnectionmanager_info.h"

#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HActionsSetupData>
#include <HUpnpCore/HStateVariablesSetupData>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av_cm Connection Manager
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's ConnectionManager
 * functionality.
 */

/*******************************************************************************
 * HConnectionManagerInfo
 ******************************************************************************/
HConnectionManagerInfo::HConnectionManagerInfo()
{
}

HConnectionManagerInfo::~HConnectionManagerInfo()
{
}

HConnectionManagerInfo::Direction
    HConnectionManagerInfo::directionFromString(const QString& arg)
{
    Direction retVal = DirectionUndefined;
    if (arg.compare("Input", Qt::CaseInsensitive) == 0)
    {
        retVal = DirectionInput;
    }
    else if (arg.compare("Output", Qt::CaseInsensitive) == 0)
    {
        retVal = DirectionOutput;
    }
    return retVal;
}

QString HConnectionManagerInfo::directionToString(Direction arg)
{
    QString retVal;
    switch(arg)
    {
    case DirectionUndefined:
        break;
    case DirectionInput:
        retVal = "Input";
        break;
    case DirectionOutput:
        retVal = "Output";
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return retVal;
}

HConnectionManagerInfo::ConnectionStatus
    HConnectionManagerInfo::statusFromString(const QString& arg)
{
    ConnectionStatus retVal = StatusUnknown;
    if (arg.compare("Ok", Qt::CaseInsensitive) == 0)
    {
        retVal = StatusOk;
    }
    else if (arg.compare("Unknown", Qt::CaseInsensitive) == 0)
    {
        retVal = StatusUnknown;
    }
    else if (arg.compare("ContentFormatMismatch", Qt::CaseInsensitive) == 0)
    {
        retVal = StatusContentFormatMismatch;
    }
    else if (arg.compare("InsufficientBandwidth", Qt::CaseInsensitive) == 0)
    {
        retVal = StatusInsufficientBandwidth;
    }
    else if (arg.compare("UnreliableChannel", Qt::CaseInsensitive) == 0)
    {
        retVal = StatusUnreliableChannel;
    }
    return retVal;
}

QString HConnectionManagerInfo::statusToString(ConnectionStatus arg)
{
    QString retVal;
    switch(arg)
    {
    case StatusUnknown:
        retVal = "Unknown";
        break;
    case StatusOk:
        retVal = "OK";
        break;
    case StatusContentFormatMismatch:
        retVal = "ContentFormatMismatch";
        break;
    case StatusInsufficientBandwidth:
        retVal = "InsufficientBandwidth";
        break;
    case StatusUnreliableChannel:
        retVal = "UnreliableChannel";
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return retVal;
}

const HResourceType& HConnectionManagerInfo::supportedServiceType()
{
    static const HResourceType retVal("urn:schemas-upnp-org:service:ConnectionManager:2");
    return retVal;
}

HActionsSetupData HConnectionManagerInfo::actionsSetupData()
{
    HActionsSetupData retVal;

    retVal.insert(HActionSetup("GetProtocolInfo"));
    retVal.insert(HActionSetup("PrepareForConnection", InclusionOptional));
    retVal.insert(HActionSetup("ConnectionComplete", InclusionOptional));

    retVal.insert(HActionSetup("GetCurrentConnectionIDs"));
    retVal.insert(HActionSetup("GetCurrentConnectionInfo"));

    return retVal;
}

HStateVariablesSetupData HConnectionManagerInfo::stateVariablesSetupData()
{
    HStateVariablesSetupData retVal;

    retVal.insert(HStateVariableInfo("SourceProtocolInfo", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("SinkProtocolInfo", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("CurrentConnectionIDs", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_ConnectionStatus", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_ConnectionManager",HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_ProtocolInfo", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_ConnectionID", HUpnpDataTypes::i4));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_AVTransportID", HUpnpDataTypes::i4));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_RcsID", HUpnpDataTypes::i4));

    return retVal;
}

}
}
}
