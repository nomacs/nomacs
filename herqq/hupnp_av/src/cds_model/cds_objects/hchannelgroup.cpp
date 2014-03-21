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

#include "hchannelgroup.h"
#include "hchannelgroup_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../hchannelgroupname.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HChannelGroupPrivate
 ******************************************************************************/
HChannelGroupPrivate::HChannelGroupPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_channelGroupName));
    insert(inst.get(HCdsProperties::upnp_epgProviderName));
    insert(inst.get(HCdsProperties::upnp_serviceProvider));
    insert(inst.get(HCdsProperties::upnp_icon));
    insert(inst.get(HCdsProperties::upnp_region));
}

/*******************************************************************************
 * HChannelGroup
 ******************************************************************************/
HChannelGroup::HChannelGroup(const QString& clazz, CdsType cdsType) :
    HContainer(*new HChannelGroupPrivate(clazz, cdsType))
{
}

HChannelGroup::HChannelGroup(HChannelGroupPrivate& dd) :
    HContainer(dd)
{
}

HChannelGroup::HChannelGroup(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HChannelGroupPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HChannelGroup::~HChannelGroup()
{
}

HChannelGroup* HChannelGroup::newInstance() const
{
    return new HChannelGroup();
}

HChannelGroupName HChannelGroup::channelGroupName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelGroupName, &value);
    return value.value<HChannelGroupName>();
}

QString HChannelGroup::epgProviderName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_epgProviderName, &value);
    return value.toString();
}

QString HChannelGroup::serviceProvider() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_serviceProvider, &value);
    return value.toString();
}

QUrl HChannelGroup::icon() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_icon, &value);
    return value.toUrl();
}

QString HChannelGroup::region() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_region, &value);
    return value.toString();
}

void HChannelGroup::setChannelGroupName(const HChannelGroupName& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelGroupName, QVariant::fromValue(arg));
}

void HChannelGroup::setEpgProviderName(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_epgProviderName, arg);
}

void HChannelGroup::setServiceProvider(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_serviceProvider, arg);
}

void HChannelGroup::setIcon(const QUrl& arg)
{
    setCdsProperty(HCdsProperties::upnp_icon, arg);
}

void HChannelGroup::setRegion(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_region, arg);
}

}
}
}
