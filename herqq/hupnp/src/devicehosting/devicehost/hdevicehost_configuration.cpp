/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hdevicehost_configuration.h"
#include "hdevicehost_configuration_p.h"

#include "../../devicemodel/hdevicemodel_infoprovider.h"
#include "../../devicemodel/server/hdevicemodelcreator.h"

#include "../../general/hupnp_global_p.h"
#include "../../utils/hmisc_utils_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceConfigurationPrivate
 ******************************************************************************/
HDeviceConfigurationPrivate::HDeviceConfigurationPrivate() :
    m_pathToDeviceDescriptor(), m_cacheControlMaxAgeInSecs(1800)
{
}

HDeviceConfigurationPrivate::~HDeviceConfigurationPrivate()
{
}

/*******************************************************************************
 * HDeviceConfiguration
 ******************************************************************************/
HDeviceConfiguration::HDeviceConfiguration() :
    h_ptr(new HDeviceConfigurationPrivate())
{
}

HDeviceConfiguration::~HDeviceConfiguration()
{
    delete h_ptr;
}

HDeviceConfiguration* HDeviceConfiguration::newInstance() const
{
    return new HDeviceConfiguration();
}

void HDeviceConfiguration::doClone(HClonable* target) const
{
    HDeviceConfiguration* conf = dynamic_cast<HDeviceConfiguration*>(target);

    if (!conf)
    {
        return;
    }

    conf->h_ptr->m_cacheControlMaxAgeInSecs = h_ptr->m_cacheControlMaxAgeInSecs;
    conf->h_ptr->m_pathToDeviceDescriptor = h_ptr->m_pathToDeviceDescriptor;
}

HDeviceConfiguration* HDeviceConfiguration::clone() const
{
    return static_cast<HDeviceConfiguration*>(HClonable::clone());
}

QString HDeviceConfiguration::pathToDeviceDescription() const
{
    return h_ptr->m_pathToDeviceDescriptor;
}

void HDeviceConfiguration::setPathToDeviceDescription(
    const QString& pathToDeviceDescriptor)
{
    h_ptr->m_pathToDeviceDescriptor = pathToDeviceDescriptor;
}

void HDeviceConfiguration::setCacheControlMaxAge(qint32 maxAgeInSecs)
{
    static const qint32 max = 60*60*24;

    if (maxAgeInSecs < 5)
    {
        maxAgeInSecs = 5;
    }
    else if (maxAgeInSecs > max)
    {
        maxAgeInSecs = max;
    }

    h_ptr->m_cacheControlMaxAgeInSecs = maxAgeInSecs;
}

qint32 HDeviceConfiguration::cacheControlMaxAge() const
{
    return h_ptr->m_cacheControlMaxAgeInSecs;
}

bool HDeviceConfiguration::isValid() const
{
    return !h_ptr->m_pathToDeviceDescriptor.isEmpty();
}

/*******************************************************************************
 * HDeviceHostConfigurationPrivate
 ******************************************************************************/
HDeviceHostConfigurationPrivate::HDeviceHostConfigurationPrivate() :
    m_collection(),
    m_individualAdvertisementCount(2),
    m_subscriptionExpirationTimeout(0),
    m_networkAddresses(),
    m_deviceCreator(0),
    m_infoProvider(0)
{
    QHostAddress ha = findBindableHostAddress();
    m_networkAddresses.append(ha);
}

/*******************************************************************************
 * HDeviceHostConfiguration
 ******************************************************************************/
HDeviceHostConfiguration::HDeviceHostConfiguration() :
    h_ptr(new HDeviceHostConfigurationPrivate())
{
}

HDeviceHostConfiguration::HDeviceHostConfiguration(
    const HDeviceConfiguration& arg) :
        h_ptr(new HDeviceHostConfigurationPrivate())
{
    add(arg);
}

HDeviceHostConfiguration::~HDeviceHostConfiguration()
{
    qDeleteAll(h_ptr->m_collection);
    delete h_ptr;
}

HDeviceHostConfiguration* HDeviceHostConfiguration::newInstance() const
{
    return new HDeviceHostConfiguration();
}

void HDeviceHostConfiguration::doClone(HClonable* target) const
{
    HDeviceHostConfiguration* conf =
        dynamic_cast<HDeviceHostConfiguration*>(target);

    if (!conf)
    {
        return;
    }

    conf->h_ptr->m_individualAdvertisementCount =
        h_ptr->m_individualAdvertisementCount;

    conf->h_ptr->m_networkAddresses = h_ptr->m_networkAddresses;

    conf->h_ptr->m_subscriptionExpirationTimeout =
        h_ptr->m_subscriptionExpirationTimeout;

    QList<const HDeviceConfiguration*> confCollection;
    foreach(const HDeviceConfiguration* conf, h_ptr->m_collection)
    {
        confCollection.append(conf->clone());
    }

    qDeleteAll(conf->h_ptr->m_collection);
    conf->h_ptr->m_collection = confCollection;

    conf->h_ptr->m_deviceCreator.reset(
        h_ptr->m_deviceCreator ? h_ptr->m_deviceCreator->clone() : 0);

    conf->h_ptr->m_infoProvider.reset(
        h_ptr->m_infoProvider ? h_ptr->m_infoProvider->clone() : 0);
}

HDeviceHostConfiguration* HDeviceHostConfiguration::clone() const
{
    return static_cast<HDeviceHostConfiguration*>(HClonable::clone());
}

bool HDeviceHostConfiguration::add(const HDeviceConfiguration& arg)
{
    if (arg.isValid())
    {
        h_ptr->m_collection.push_back(arg.clone());
        return true;
    }

    return false;
}

void HDeviceHostConfiguration::clear()
{
    qDeleteAll(h_ptr->m_collection);
    h_ptr->m_collection.clear();
}

QList<const HDeviceConfiguration*> HDeviceHostConfiguration::deviceConfigurations() const
{
    return h_ptr->m_collection;
}

qint32 HDeviceHostConfiguration::individualAdvertisementCount() const
{
    return h_ptr->m_individualAdvertisementCount;
}

QList<QHostAddress> HDeviceHostConfiguration::networkAddressesToUse() const
{
    return h_ptr->m_networkAddresses;
}

HDeviceModelCreator* HDeviceHostConfiguration::deviceModelCreator() const
{
    return h_ptr->m_deviceCreator.data();
}

HDeviceModelInfoProvider* HDeviceHostConfiguration::deviceModelInfoProvider() const
{
    return h_ptr->m_infoProvider.data();
}

void HDeviceHostConfiguration::setDeviceModelCreator(
    const HDeviceModelCreator& deviceCreator)
{
    h_ptr->m_deviceCreator.reset(deviceCreator.clone());
}

void HDeviceHostConfiguration::setDeviceModelInfoProvider(
    const HDeviceModelInfoProvider& infoProvider)
{
    h_ptr->m_infoProvider.reset(infoProvider.clone());
}

void HDeviceHostConfiguration::setIndividualAdvertisementCount(qint32 arg)
{
    if (arg < 1)
    {
        arg = 1;
    }

    h_ptr->m_individualAdvertisementCount = arg;
}

qint32 HDeviceHostConfiguration::subscriptionExpirationTimeout() const
{
    return h_ptr->m_subscriptionExpirationTimeout;
}

void HDeviceHostConfiguration::setSubscriptionExpirationTimeout(qint32 arg)
{
    static const qint32 max = 60*60*24;

    if (arg > max)
    {
        arg = max;
    }

    h_ptr->m_subscriptionExpirationTimeout = arg;
}

bool HDeviceHostConfiguration::setNetworkAddressesToUse(
    const QList<QHostAddress>& addresses)
{
    if (!HSysInfo::instance().areLocalAddresses(addresses))
    {
        return false;
    }

    h_ptr->m_networkAddresses = addresses;
    return true;
}

bool HDeviceHostConfiguration::isEmpty() const
{
    return h_ptr->m_collection.isEmpty();
}

bool HDeviceHostConfiguration::isValid() const
{
    return !isEmpty() && deviceModelCreator();
}

}
}
