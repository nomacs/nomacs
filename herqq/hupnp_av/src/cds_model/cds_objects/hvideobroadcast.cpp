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

#include "hvideobroadcast.h"
#include "hvideobroadcast_p.h"

#include "../hprice.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HVideoBroadcastPrivate
 ******************************************************************************/
HVideoBroadcastPrivate::HVideoBroadcastPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HVideoItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_icon));
    insert(inst.get(HCdsProperties::upnp_region));
    insert(inst.get(HCdsProperties::upnp_channelNr));
    insert(inst.get(HCdsProperties::upnp_signalStrength));
    insert(inst.get(HCdsProperties::upnp_signalLocked));
    insert(inst.get(HCdsProperties::upnp_tuned));
    insert(inst.get(HCdsProperties::upnp_recordable));
    insert(inst.get(HCdsProperties::upnp_callSign));
    insert(inst.get(HCdsProperties::upnp_price));
    insert(inst.get(HCdsProperties::upnp_payPerView));
}

/*******************************************************************************
 * HVideoBroadcast
 ******************************************************************************/
HVideoBroadcast::HVideoBroadcast(const QString& clazz, CdsType cdsType) :
    HVideoItem(*new HVideoBroadcastPrivate(clazz, cdsType))
{
}

HVideoBroadcast::HVideoBroadcast(HVideoBroadcastPrivate& dd) :
    HVideoItem(dd)
{
}

HVideoBroadcast::HVideoBroadcast(
    const QString& title, const QString& parentId, const QString& id) :
        HVideoItem(*new HVideoBroadcastPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HVideoBroadcast::~HVideoBroadcast()
{
}

HVideoBroadcast* HVideoBroadcast::newInstance() const
{
    return new HVideoBroadcast();
}

void HVideoBroadcast::setIcon(const QUrl& arg)
{
    setCdsProperty(HCdsProperties::upnp_icon, arg);
}

void HVideoBroadcast::setRegion(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_region, arg);
}

void HVideoBroadcast::setChannelNr(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_channelNr, arg);
}

void HVideoBroadcast::setSignalStrength(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_signalStrength, arg);
}

void HVideoBroadcast::setSignalLocked(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_signalLocked, arg);
}

void HVideoBroadcast::setTuned(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_tuned, arg);
}

void HVideoBroadcast::setRecordable(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_recordable, arg);
}

void HVideoBroadcast::setCallSign(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_callSign, arg);
}

void HVideoBroadcast::setPrice(const HPrice& arg)
{
    setCdsProperty(
        HCdsProperties::upnp_price, QVariant::fromValue(arg));
}

void HVideoBroadcast::setPayPerView(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_payPerView, arg);
}

QUrl HVideoBroadcast::icon() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_icon, &value);
    return value.toUrl();
}

QString HVideoBroadcast::region() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_region, &value);
    return value.toString();
}

qint32 HVideoBroadcast::channelNr() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelNr, &value);
    return value.toInt();
}

qint32 HVideoBroadcast::signalStrength() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_signalStrength, &value);
    return value.toInt();
}

bool HVideoBroadcast::signalLocked() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_signalLocked, &value);
    return value.toBool();
}

bool HVideoBroadcast::tuned() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_tuned, &value);
    return value.toBool();
}

bool HVideoBroadcast::recordable() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_recordable, &value);
    return value.toBool();
}

QString HVideoBroadcast::callSign() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_callSign, &value);
    return value.toString();
}

HPrice HVideoBroadcast::price() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_price, &value);
    return value.value<HPrice>();
}

bool HVideoBroadcast::payPerView() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_payPerView, &value);
    return value.toBool();
}

}
}
}
