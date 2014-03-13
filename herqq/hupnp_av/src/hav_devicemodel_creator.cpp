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

#include "hav_devicemodel_creator.h"

#include "transport/havtransport_info.h"

#include "contentdirectory/hcontentdirectory_info.h"
#include "contentdirectory/hcontentdirectory_service.h"
#include "contentdirectory/hcontentdirectory_serviceconfiguration.h"

#include "connectionmanager/hconnectionmanager_info.h"
#include "connectionmanager/hconnectionmanager_serviceconfiguration.h"

#include "renderingcontrol/hrenderingcontrol_info.h"
#include "renderingcontrol/hrenderingcontrol_service_p.h"

#include "mediaserver/hmediaserver_info.h"
#include "mediaserver/hmediaserver_device_p.h"
#include "mediaserver/hmediaserver_deviceconfiguration.h"
#include "mediaserver/hconnectionmanager_sourceservice_p.h"

#include "mediarenderer/hmediarenderer_info.h"
#include "mediarenderer/hmediarenderer_device_p.h"
#include "mediarenderer/htransport_sinkservice_p.h"
#include "mediarenderer/hconnectionmanager_sinkservice_p.h"
#include "mediarenderer/hmediarenderer_deviceconfiguration.h"

#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HServiceInfo>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAvDeviceModelCreatorPrivate
 ******************************************************************************/
class HAvDeviceModelCreatorPrivate
{
H_DISABLE_COPY(HAvDeviceModelCreatorPrivate)

public:

    QScopedPointer<HMediaServerDeviceConfiguration> m_mediaServerConf;
    QScopedPointer<HMediaRendererDeviceConfiguration> m_mediaRendererConf;

    HAvDeviceModelCreatorPrivate() :
        m_mediaServerConf(0), m_mediaRendererConf(0)
    {
    }
};

/*******************************************************************************
 * HAvDeviceModelCreator
 ******************************************************************************/
HAvDeviceModelCreator::HAvDeviceModelCreator() :
    h_ptr(new HAvDeviceModelCreatorPrivate())
{
}

HAvDeviceModelCreator::~HAvDeviceModelCreator()
{
    delete h_ptr;
}

HAvDeviceModelCreator* HAvDeviceModelCreator::newInstance() const
{
    return new HAvDeviceModelCreator();
}

void HAvDeviceModelCreator::doClone(HClonable* target) const
{
    HAvDeviceModelCreator* instance =
        dynamic_cast<HAvDeviceModelCreator*>(target);

    if (!instance)
    {
        return;
    }

    if (h_ptr->m_mediaRendererConf)
    {
        instance->setMediaRendererConfiguration(*h_ptr->m_mediaRendererConf);
    }

    if (h_ptr->m_mediaServerConf)
    {
        instance->setMediaServerConfiguration(*h_ptr->m_mediaServerConf);
    }
}

const HMediaServerDeviceConfiguration*
    HAvDeviceModelCreator::mediaServerConfiguration() const
{
    return h_ptr->m_mediaServerConf.data();
}

const HMediaRendererDeviceConfiguration*
    HAvDeviceModelCreator::mediaRendererConfiguration() const
{
    return h_ptr->m_mediaRendererConf.data();
}

void HAvDeviceModelCreator::setMediaServerConfiguration(
    const HMediaServerDeviceConfiguration& conf)
{
    h_ptr->m_mediaServerConf.reset(conf.clone());
}

void HAvDeviceModelCreator::setMediaRendererConfiguration(
    const HMediaRendererDeviceConfiguration& conf)
{
    h_ptr->m_mediaRendererConf.reset(conf.clone());
}

HServerDevice* HAvDeviceModelCreator::createDevice(const HDeviceInfo& info) const
{
    if (info.deviceType().compare(
        HMediaServerInfo::supportedDeviceType(), HResourceType::Inclusive))
    {
        if (h_ptr->m_mediaServerConf && h_ptr->m_mediaServerConf->isValid())
        {
            return new HMediaServerDevice(*h_ptr->m_mediaServerConf);
        }
    }
    else if (info.deviceType().compare(
        HMediaRendererInfo::supportedDeviceType(), HResourceType::Inclusive))
    {
        if (h_ptr->m_mediaRendererConf && h_ptr->m_mediaRendererConf->isValid())
        {
            return new HMediaRendererDevice(*h_ptr->m_mediaRendererConf);
        }
    }

    return 0;
}

HServerService* HAvDeviceModelCreator::createService(
    const HServiceInfo& serviceInfo, const HDeviceInfo& deviceInfo) const
{
    if (serviceInfo.serviceType().compare(
        HContentDirectoryInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        if (h_ptr->m_mediaServerConf && h_ptr->m_mediaServerConf->isValid())
        {
            return new HContentDirectoryService(
                h_ptr->m_mediaServerConf->contentDirectoryConfiguration()->dataSource());
        }
    }
    else if (serviceInfo.serviceType().compare(
        HRenderingControlInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        return new HRenderingControlService();
    }
    else if (serviceInfo.serviceType().compare(
        HConnectionManagerInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        if (deviceInfo.deviceType().compare(
            HMediaServerInfo::supportedDeviceType(), HResourceType::Inclusive))
        {
            if (h_ptr->m_mediaServerConf && h_ptr->m_mediaServerConf->isValid())
            {
                return new HConnectionManagerSourceService(
                    h_ptr->m_mediaServerConf->contentDirectoryConfiguration()->dataSource());
            }
        }
        else if (deviceInfo.deviceType().compare(
            HMediaRendererInfo::supportedDeviceType(), HResourceType::Inclusive))
        {
            return new HConnectionManagerSinkService();
        }
    }
    else if (serviceInfo.serviceType().compare(
        HAvTransportInfo::supportedServiceType(), HResourceType::Inclusive))
    {
        if (deviceInfo.deviceType().compare(
            HMediaRendererInfo::supportedDeviceType(), HResourceType::Inclusive))
        {
            return new HTransportSinkService();
        }
    }

    return 0;
}

HAvDeviceModelCreator* HAvDeviceModelCreator::clone() const
{
    return static_cast<HAvDeviceModelCreator*>(HClonable::clone());
}

}
}
}
