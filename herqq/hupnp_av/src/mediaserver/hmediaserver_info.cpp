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

#include "hmediaserver_info.h"

#include "../transport/havtransport_info.h"
#include "../contentdirectory/hcontentdirectory_info.h"
#include "../connectionmanager/hconnectionmanager_info.h"

#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HServicesSetupData>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av_mediaserver Media Server
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's MediaServer
 * functionality.
 */

/*******************************************************************************
 * HMediaServerInfo
 ******************************************************************************/
HMediaServerInfo::HMediaServerInfo()
{
}

HMediaServerInfo::~HMediaServerInfo()
{
}

const HResourceType& HMediaServerInfo::supportedDeviceType()
{
    static HResourceType retVal("urn:schemas-upnp-org:device:MediaServer:3");
    return retVal;
}

HServicesSetupData HMediaServerInfo::servicesSetupData()
{
    HServicesSetupData retVal;

    retVal.insert(
        HServiceSetup(
            defaultContentDirectoryId(),
            HContentDirectoryInfo::supportedServiceType()));

    retVal.insert(
        HServiceSetup(
            defaultConnectionManagerId(),
            HConnectionManagerInfo::supportedServiceType()));

    retVal.insert(
        HServiceSetup(
            defaultAvTransportId(),
            HAvTransportInfo::supportedServiceType(),
            InclusionOptional));

//    retVal.insert(
//        HServiceSetup(
//            HServiceId("urn:upnp-org:serviceId:ScheduledRecording"),
//            HResourceType("urn:schemas-upnp-org:service:ScheduledRecording:2"),
//            InclusionOptional));

    return retVal;
}

const HServiceId& HMediaServerInfo::defaultContentDirectoryId()
{
    static const HServiceId retVal("urn:upnp-org:serviceId:ContentDirectory");
    return retVal;
}

const HServiceId& HMediaServerInfo::defaultConnectionManagerId()
{
    static const HServiceId retVal("urn:upnp-org:serviceId:ConnectionManager");
    return retVal;
}

const HServiceId& HMediaServerInfo::defaultAvTransportId()
{
    static const HServiceId retVal("urn:upnp-org:serviceId:AVTransport");
    return retVal;
}

}
}
}
