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

#include "hmediarenderer_info.h"
#include "../transport/havtransport_info.h"
#include "../renderingcontrol/hrenderingcontrol_info.h"
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
 * \defgroup hupnp_av_mediarenderer Media Renderer
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's Media Renderer
 * functionality.
 */

HMediaRendererInfo::HMediaRendererInfo()
{
}

HMediaRendererInfo::~HMediaRendererInfo()
{
}

const HResourceType& HMediaRendererInfo::supportedDeviceType()
{
    static HResourceType retVal("urn:schemas-upnp-org:device:MediaRenderer:2");
    return retVal;
}

HServicesSetupData HMediaRendererInfo::servicesSetupData()
{
    HServicesSetupData retVal;

    retVal.insert(
        HServiceSetup(
            defaultRenderingControlId(),
            HRenderingControlInfo::supportedServiceType()));

    retVal.insert(
        HServiceSetup(
            defaultConnectionManagerId(),
            HConnectionManagerInfo::supportedServiceType()));

    retVal.insert(
        HServiceSetup(
            defaultAvTransportId(),
            HAvTransportInfo::supportedServiceType()));

    return retVal;
}

const HServiceId& HMediaRendererInfo::defaultConnectionManagerId()
{
    static const HServiceId retVal("urn:upnp-org:serviceId:ConnectionManager");
    return retVal;
}

const HServiceId& HMediaRendererInfo::defaultRenderingControlId()
{
    static const HServiceId retVal("urn:upnp-org:serviceId:RenderingControl");
    return retVal;
}

const HServiceId& HMediaRendererInfo::defaultAvTransportId()
{
    static const HServiceId retVal("urn:upnp-org:serviceId:AVTransport");
    return retVal;
}

}
}
}
