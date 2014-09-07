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

#include "hav_controlpoint_configuration.h"
#include "hav_controlpoint_configuration_p.h"

#include <QtNetwork/QHostAddress>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAvControlPointConfigurationPrivate
 ******************************************************************************/
HAvControlPointConfigurationPrivate::HAvControlPointConfigurationPrivate() :
    m_controlpointConfiguration(new HControlPointConfiguration()),
    m_interestingDeviceTypes(HAvControlPointConfiguration::All)
{
}

HAvControlPointConfigurationPrivate::~HAvControlPointConfigurationPrivate()
{
}

/*******************************************************************************
 * HAvControlPointConfiguration
 ******************************************************************************/
HAvControlPointConfiguration::HAvControlPointConfiguration() :
    h_ptr(new HAvControlPointConfigurationPrivate())
{
}

HAvControlPointConfiguration::~HAvControlPointConfiguration()
{
    delete h_ptr;
}

void HAvControlPointConfiguration::doClone(HClonable* target) const
{
    HAvControlPointConfiguration* conf =
        dynamic_cast<HAvControlPointConfiguration*>(target);

    if (!conf)
    {
        return;
    }

    conf->h_ptr->m_controlpointConfiguration.reset(
       h_ptr->m_controlpointConfiguration ? h_ptr->m_controlpointConfiguration->clone() : 0);
}

HAvControlPointConfiguration* HAvControlPointConfiguration::newInstance() const
{
    return new HAvControlPointConfiguration();
}

HAvControlPointConfiguration* HAvControlPointConfiguration::clone() const
{
    return static_cast<HAvControlPointConfiguration*>(HClonable::clone());
}

HAvControlPointConfiguration::InterestingDeviceTypes
    HAvControlPointConfiguration::interestingDeviceTypes() const
{
    return h_ptr->m_interestingDeviceTypes;
}

void HAvControlPointConfiguration::setInterestingDeviceTypes(
    InterestingDeviceTypes arg) const
{
    h_ptr->m_interestingDeviceTypes = arg;
}

bool HAvControlPointConfiguration::autoDiscovery() const
{
    return h_ptr->m_controlpointConfiguration->autoDiscovery();
}

QList<QHostAddress> HAvControlPointConfiguration::networkAddressesToUse() const
{
    return h_ptr->m_controlpointConfiguration->networkAddressesToUse();
}

bool HAvControlPointConfiguration::subscribeToEvents() const
{
    return h_ptr->m_controlpointConfiguration->subscribeToEvents();
}

qint32 HAvControlPointConfiguration::desiredSubscriptionTimeout() const
{
    return h_ptr->m_controlpointConfiguration->desiredSubscriptionTimeout();
}

void HAvControlPointConfiguration::setSubscribeToEvents(
    bool subscribeAutomatically)
{
    return h_ptr->m_controlpointConfiguration->setSubscribeToEvents(subscribeAutomatically);
}

void HAvControlPointConfiguration::setDesiredSubscriptionTimeout(qint32 timeout)
{
    return h_ptr->m_controlpointConfiguration->setDesiredSubscriptionTimeout(timeout);
}

void HAvControlPointConfiguration::setAutoDiscovery(bool arg)
{
    return h_ptr->m_controlpointConfiguration->setAutoDiscovery(arg);
}

bool HAvControlPointConfiguration::setNetworkAddressesToUse(
    const QList<QHostAddress>& addresses)
{
    return h_ptr->m_controlpointConfiguration->setNetworkAddressesToUse(addresses);
}

}
}
}
