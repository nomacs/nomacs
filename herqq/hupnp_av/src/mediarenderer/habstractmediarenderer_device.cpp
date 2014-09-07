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

#include "habstractmediarenderer_device.h"
#include "hmediarenderer_info.h"

#include "../transport/habstract_avtransport_service.h"
#include "../renderingcontrol/habstractrenderingcontrol_service.h"
#include "../connectionmanager/habstractconnectionmanager_service.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractMediaRendererDevice
 ******************************************************************************/
HAbstractMediaRendererDevice::HAbstractMediaRendererDevice()
{
}

HAbstractMediaRendererDevice::~HAbstractMediaRendererDevice()
{
}

HAbstractRenderingControlService* HAbstractMediaRendererDevice::renderingControl() const
{
    return qobject_cast<HAbstractRenderingControlService*>(
        serviceById(HMediaRendererInfo::defaultRenderingControlId()));
}

HAbstractConnectionManagerService* HAbstractMediaRendererDevice::connectionManager() const
{
    return qobject_cast<HAbstractConnectionManagerService*>(
        serviceById(HMediaRendererInfo::defaultConnectionManagerId()));
}

HAbstractTransportService* HAbstractMediaRendererDevice::avTransport() const
{
    return qobject_cast<HAbstractTransportService*>(
        serviceById(HMediaRendererInfo::defaultAvTransportId()));
}

}
}
}
