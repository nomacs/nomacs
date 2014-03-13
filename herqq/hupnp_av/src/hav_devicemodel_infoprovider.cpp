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

#include "hav_devicemodel_infoprovider.h"

#include "transport/havtransport_info.h"
#include "mediaserver/hmediaserver_info.h"
#include "mediarenderer/hmediarenderer_info.h"
#include "renderingcontrol/hrenderingcontrol_info.h"
#include "contentdirectory/hcontentdirectory_info.h"
#include "connectionmanager/hconnectionmanager_info.h"

#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HServiceInfo>

#include <HUpnpCore/HDevicesSetupData>
#include <HUpnpCore/HActionsSetupData>
#include <HUpnpCore/HServicesSetupData>
#include <HUpnpCore/HStateVariablesSetupData>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAvDeviceModelInfoProvider
 ******************************************************************************/
HAvDeviceModelInfoProvider::HAvDeviceModelInfoProvider()
{
}

HAvDeviceModelInfoProvider::~HAvDeviceModelInfoProvider()
{
}

HAvDeviceModelInfoProvider* HAvDeviceModelInfoProvider::newInstance() const
{
    return new HAvDeviceModelInfoProvider();
}

HServicesSetupData HAvDeviceModelInfoProvider::servicesSetupData(
    const HDeviceInfo& info) const
{
    if (info.deviceType().compare(
        HMediaRendererInfo::supportedDeviceType(), HResourceType::Inclusive))
    {
        return HMediaRendererInfo::servicesSetupData();
    }
    else if (info.deviceType().compare(
        HMediaServerInfo::supportedDeviceType(), HResourceType::Inclusive))
    {
        return HMediaServerInfo::servicesSetupData();
    }
    return HServicesSetupData();
}

HActionsSetupData HAvDeviceModelInfoProvider::actionsSetupData(
    const HServiceInfo& serviceInfo, const HDeviceInfo&) const
{
    if (serviceInfo.serviceType().compare(
        HContentDirectoryInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HContentDirectoryInfo::actionsSetupData();
    }
    else if (serviceInfo.serviceType().compare(
        HRenderingControlInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HRenderingControlInfo::actionsSetupData();
    }
    else if (serviceInfo.serviceType().compare(
        HConnectionManagerInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HConnectionManagerInfo::actionsSetupData();
    }
    else if (serviceInfo.serviceType().compare(
        HAvTransportInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HAvTransportInfo::actionsSetupData();
    }

    return HActionsSetupData();
}

HStateVariablesSetupData HAvDeviceModelInfoProvider::stateVariablesSetupData(
    const HServiceInfo& serviceInfo, const HDeviceInfo&) const
{
    if (serviceInfo.serviceType().compare(
        HContentDirectoryInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HContentDirectoryInfo::stateVariablesSetupData();
    }
    else if (serviceInfo.serviceType().compare(
        HRenderingControlInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HRenderingControlInfo::stateVariablesSetupData();
    }
    else if (serviceInfo.serviceType().compare(
        HConnectionManagerInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HConnectionManagerInfo::stateVariablesSetupData();
    }
    else if (serviceInfo.serviceType().compare(
        HAvTransportInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return HAvTransportInfo::stateVariablesSetupData();
    }

    return HStateVariablesSetupData();
}

HAvDeviceModelInfoProvider* HAvDeviceModelInfoProvider::clone() const
{
    return static_cast<HAvDeviceModelInfoProvider*>(HClonable::clone());
}

}
}
}
