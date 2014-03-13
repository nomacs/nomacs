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

#include "haudiobroadcast.h"
#include "haudiobroadcast_p.h"
#include "../model_mgmt/hcdsproperties.h"
#include "../../common/hradioband.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAudioBroadcastPrivate
 ******************************************************************************/
HAudioBroadcastPrivate::HAudioBroadcastPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HAudioItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_region));
    insert(inst.get(HCdsProperties::upnp_radioCallSign));
    insert(inst.get(HCdsProperties::upnp_radioStationID));
    insert(inst.get(HCdsProperties::upnp_radioBand));
    insert(inst.get(HCdsProperties::upnp_channelNr));
    insert(inst.get(HCdsProperties::upnp_signalStrength));
    insert(inst.get(HCdsProperties::upnp_signalLocked).name(), false);
    insert(inst.get(HCdsProperties::upnp_tuned).name(), false);
    insert(inst.get(HCdsProperties::upnp_recordable).name(), false);
}

/*******************************************************************************
 * HAudioBroadcast
 ******************************************************************************/
HAudioBroadcast::HAudioBroadcast(const QString& clazz, CdsType cdsType) :
    HAudioItem(*new HAudioBroadcastPrivate(clazz, cdsType))
{
}

HAudioBroadcast::HAudioBroadcast(HAudioBroadcastPrivate& dd) :
    HAudioItem(dd)
{
}

HAudioBroadcast::HAudioBroadcast(
    const QString& title, const QString& parentId, const QString& id) :
        HAudioItem(*new HAudioBroadcastPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HAudioBroadcast::~HAudioBroadcast()
{
}

HAudioBroadcast* HAudioBroadcast::newInstance() const
{
    return new HAudioBroadcast();
}

void HAudioBroadcast::setRegion(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_region, arg);
}

void HAudioBroadcast::setRadioCallSign(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioCallSign, arg);
}

void HAudioBroadcast::setRadioStationId(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioStationID, arg);
}

void HAudioBroadcast::setRadioBand(const HRadioBand& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioBand, QVariant::fromValue(arg));
}

void HAudioBroadcast::setChannelNr(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_channelNr, arg);
}

void HAudioBroadcast::setSignalStrength(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_signalStrength, arg);
}

void HAudioBroadcast::setSignalLocked(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_signalLocked, arg);
}

void HAudioBroadcast::setTuned(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_tuned, arg);
}

void HAudioBroadcast::setRecordable(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_recordable, arg);
}

QString HAudioBroadcast::region() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_region, &value);
    return value.toString();
}

QString HAudioBroadcast::radioCallSign() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioCallSign, &value);
    return value.toString();
}

QString HAudioBroadcast::radioStationId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioStationID, &value);
    return value.toString();
}

HRadioBand HAudioBroadcast::radioBand() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioBand, &value);
    return value.value<HRadioBand>();
}

qint32 HAudioBroadcast::channelNr() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelNr, &value);
    return value.toInt();
}

qint32 HAudioBroadcast::signalStrength() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_signalStrength, &value);
    return value.toInt();
}

bool HAudioBroadcast::signalLocked() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_signalLocked, &value);
    return value.toBool();
}

bool HAudioBroadcast::tuned() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_tuned, &value);
    return value.toBool();
}

bool HAudioBroadcast::recordable() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_recordable, &value);
    return value.toBool();
}

}
}
}
