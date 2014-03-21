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

#include "habstractmediaserver_device.h"
#include "hmediaserver_info.h"

#include "../transport/habstract_avtransport_service.h"
#include "../connectionmanager/habstractconnectionmanager_service.h"
#include "../contentdirectory/habstractcontentdirectory_service.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractMediaServerDevice
 ******************************************************************************/
HAbstractMediaServerDevice::HAbstractMediaServerDevice()
{
}

HAbstractMediaServerDevice::~HAbstractMediaServerDevice()
{
}

HAbstractContentDirectoryService* HAbstractMediaServerDevice::contentDirectory() const
{
    return qobject_cast<HAbstractContentDirectoryService*>(
        serviceById(HMediaServerInfo::defaultContentDirectoryId()));
}

HAbstractConnectionManagerService* HAbstractMediaServerDevice::connectionManager() const
{
    return qobject_cast<HAbstractConnectionManagerService*>(
        serviceById(HMediaServerInfo::defaultConnectionManagerId()));
}

HAbstractTransportService* HAbstractMediaServerDevice::avTransport() const
{
    return qobject_cast<HAbstractTransportService*>(
        serviceById(HMediaServerInfo::defaultAvTransportId()));
}

}
}
}
