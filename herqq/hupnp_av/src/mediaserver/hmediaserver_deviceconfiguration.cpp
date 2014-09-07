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

#include "hmediaserver_deviceconfiguration.h"
#include "hmediaserver_deviceconfiguration_p.h"

#include "../contentdirectory/hcontentdirectory_serviceconfiguration.h"
#include "../connectionmanager/hconnectionmanager_serviceconfiguration.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMediaServerDeviceConfigurationPrivate
 ******************************************************************************/
HMediaServerDeviceConfigurationPrivate::HMediaServerDeviceConfigurationPrivate() :
    m_cdsConf(0), m_cmConf(new HConnectionManagerServiceConfiguration())
{
}

HMediaServerDeviceConfigurationPrivate::~HMediaServerDeviceConfigurationPrivate()
{
}

/*******************************************************************************
 * HMediaServerDeviceConfiguration
 ******************************************************************************/
HMediaServerDeviceConfiguration::HMediaServerDeviceConfiguration() :
    h_ptr(new HMediaServerDeviceConfigurationPrivate())
{
}

HMediaServerDeviceConfiguration::~HMediaServerDeviceConfiguration()
{
    delete h_ptr;
}

void HMediaServerDeviceConfiguration::doClone(HClonable* target) const
{
    HMediaServerDeviceConfiguration* conf =
        dynamic_cast<HMediaServerDeviceConfiguration*>(target);

    if (!target)
    {
        return;
    }

    if (contentDirectoryConfiguration())
    {
        conf->h_ptr->m_cdsConf.reset(contentDirectoryConfiguration()->clone());
    }

    if (connectionManagerConfiguration())
    {
        conf->h_ptr->m_cmConf.reset(connectionManagerConfiguration()->clone());
    }
}

HMediaServerDeviceConfiguration* HMediaServerDeviceConfiguration::newInstance() const
{
    return new HMediaServerDeviceConfiguration();
}

void HMediaServerDeviceConfiguration::setContentDirectoryConfiguration(
    const HContentDirectoryServiceConfiguration& arg)
{
    if (arg.isValid())
    {
        h_ptr->m_cdsConf.reset(arg.clone());
    }
}

const HContentDirectoryServiceConfiguration*
    HMediaServerDeviceConfiguration::contentDirectoryConfiguration() const
{
    return h_ptr->m_cdsConf.data();
}

void HMediaServerDeviceConfiguration::setConnectionManagerConfiguration(
    const HConnectionManagerServiceConfiguration& arg)
{
    if (arg.isValid())
    {
        h_ptr->m_cmConf.reset(arg.clone());
    }
}

const HConnectionManagerServiceConfiguration*
    HMediaServerDeviceConfiguration::connectionManagerConfiguration() const
{
    return h_ptr->m_cmConf.data();
}

bool HMediaServerDeviceConfiguration::isValid() const
{
    return contentDirectoryConfiguration() && connectionManagerConfiguration();
}

HMediaServerDeviceConfiguration* HMediaServerDeviceConfiguration::clone() const
{
    return static_cast<HMediaServerDeviceConfiguration*>(HClonable::clone());
}

}
}
}
