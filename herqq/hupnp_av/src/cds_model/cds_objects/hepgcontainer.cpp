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

#include "hepgcontainer.h"
#include "hepgcontainer_p.h"

#include "../hprice.h"
#include "../hchannel_id.h"
#include "../hmatching_id.h"
#include "../hdatetimerange.h"
#include "../hchannelgroupname.h"
#include "../model_mgmt/hcdsproperties.h"
#include "../../common/hradioband.h"

#include <QtCore/QUrl>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HEpgContainerPrivate
 ******************************************************************************/
HEpgContainerPrivate::HEpgContainerPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_channelGroupName));
    insert(inst.get(HCdsProperties::upnp_epgProviderName));
    insert(inst.get(HCdsProperties::upnp_serviceProvider));
    insert(inst.get(HCdsProperties::upnp_channelName));
    insert(inst.get(HCdsProperties::upnp_channelNr));
    insert(inst.get(HCdsProperties::upnp_episodeType));
    insert(inst.get(HCdsProperties::upnp_radioCallSign));
    insert(inst.get(HCdsProperties::upnp_radioStationID));
    insert(inst.get(HCdsProperties::upnp_radioBand));
    insert(inst.get(HCdsProperties::upnp_callSign));
    insert(inst.get(HCdsProperties::upnp_networkAffiliation));
    insert(inst.get(HCdsProperties::upnp_price));
    insert(inst.get(HCdsProperties::upnp_payPerView));
    insert(inst.get(HCdsProperties::upnp_epgProviderName));
    insert(inst.get(HCdsProperties::upnp_icon));
    insert(inst.get(HCdsProperties::upnp_region));
    insert(inst.get(HCdsProperties::dc_language));
    insert(inst.get(HCdsProperties::dc_relation));
    insert(inst.get(HCdsProperties::upnp_dateTimeRange));
}

/*******************************************************************************
 * HEpgContainer
 ******************************************************************************/
HEpgContainer::HEpgContainer(const QString& clazz, CdsType cdsType) :
    HContainer(*new HEpgContainerPrivate(clazz, cdsType))
{
}

HEpgContainer::HEpgContainer(HEpgContainerPrivate& dd) :
    HContainer(dd)
{
}

HEpgContainer::HEpgContainer(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HEpgContainerPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HEpgContainer::~HEpgContainer()
{
}

HChannelGroupName HEpgContainer::channelGroupName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelGroupName, &value);
    return value.value<HChannelGroupName>();
}

QString HEpgContainer::epgProviderName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_epgProviderName, &value);
    return value.toString();
}

QString HEpgContainer::serviceProvider() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_serviceProvider, &value);
    return value.toString();
}

QString HEpgContainer::channelName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelName, &value);
    return value.toString();
}

qint32 HEpgContainer::channelNr() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelNr, &value);
    return value.toInt();
}

HChannelId HEpgContainer::channelId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelID, &value);
    return value.value<HChannelId>();
}

QString HEpgContainer::radioCallSign() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioCallSign, &value);
    return value.toString();
}

QString HEpgContainer::radioStationId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioStationID, &value);
    return value.toString();
}

HRadioBand HEpgContainer::radioBand() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioBand, &value);
    return value.value<HRadioBand>();
}

QString HEpgContainer::callSign() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_callSign, &value);
    return value.toString();
}

QString HEpgContainer::networkAffiliation() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_networkAffiliation, &value);
    return value.toString();
}

HPrice HEpgContainer::price() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_price, &value);
    return value.value<HPrice>();
}

bool HEpgContainer::payPerView() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_payPerView, &value);
    return value.toBool();
}

QUrl HEpgContainer::icon() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_icon, &value);
    return value.toUrl();
}

QString HEpgContainer::region() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_region, &value);
    return value.toString();
}

QStringList HEpgContainer::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

QList<QUrl> HEpgContainer::relations() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_relation, &value);
    return toList<QUrl>(value.toList());
}

HDateTimeRange HEpgContainer::dateTimeRange() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_dateTimeRange, &value);
    return value.value<HDateTimeRange>();
}

void HEpgContainer::setChannelGroupName(const HChannelGroupName& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelGroupName, QVariant::fromValue(arg));
}

void HEpgContainer::setEpgProviderName(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_epgProviderName, arg);
}

void HEpgContainer::setServiceProvider(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_serviceProvider, arg);
}

void HEpgContainer::setChannelName(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelName, arg);
}

void HEpgContainer::setChannelNr(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_channelNr, arg);
}

void HEpgContainer::setChannelId(const HMatchingId& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelID, QVariant::fromValue(arg));
}

void HEpgContainer::setRadioCallSign(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioCallSign, arg);
}

void HEpgContainer::setRadioStationId(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioStationID, arg);
}

void HEpgContainer::setRadioBand(const HRadioBand& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioBand, QVariant::fromValue(arg));
}

void HEpgContainer::setCallSign(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_callSign, arg);
}

void HEpgContainer::setNetworkAffiliation(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_networkAffiliation, arg);
}

void HEpgContainer::setPrice(const HPrice& arg)
{
    setCdsProperty(HCdsProperties::upnp_price, QVariant::fromValue(arg));
}

void HEpgContainer::setPayPerView(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_payPerView, arg);
}

void HEpgContainer::setIcon(const QUrl& arg)
{
    setCdsProperty(HCdsProperties::upnp_icon, arg);
}

void HEpgContainer::setRegion(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_region, arg);
}

void HEpgContainer::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

void HEpgContainer::setRelations(const QList<QUrl>& arg)
{
    setCdsProperty(HCdsProperties::dc_relation, toList(arg));
}

void HEpgContainer::setDateTimeRange(const HDateTimeRange& arg)
{
    setCdsProperty(HCdsProperties::upnp_dateTimeRange, QVariant::fromValue(arg));
}

HEpgContainer* HEpgContainer::newInstance() const
{
    return new HEpgContainer();
}

}
}
}
