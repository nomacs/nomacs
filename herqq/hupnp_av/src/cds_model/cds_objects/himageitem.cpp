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

#include "himageitem.h"
#include "himageitem_p.h"
#include "../../common/hrating.h"
#include "../../common/hstoragemedium.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QDateTime>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HImageItemPrivate
 ******************************************************************************/
HImageItemPrivate::HImageItemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::upnp_rating));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::dc_publisher));
    insert(inst.get(HCdsProperties::dc_date));
    insert(inst.get(HCdsProperties::dc_rights));
}

/*******************************************************************************
 * HImageItem
 ******************************************************************************/
HImageItem::HImageItem(const QString& clazz, HObject::CdsType cdsType) :
    HItem(*new HImageItemPrivate(clazz, cdsType))
{
}

HImageItem::HImageItem(HImageItemPrivate& dd) :
    HItem(dd)
{
}

HImageItem::HImageItem(
    const QString& title, const QString& parentId, const QString& id) :
        HItem(*new HImageItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HImageItem::~HImageItem()
{
}

HImageItem* HImageItem::newInstance() const
{
    return new HImageItem();
}

QString HImageItem::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

HStorageMedium HImageItem::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QList<HRating> HImageItem::ratings() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_rating, &value);
    return toList<HRating>(value.toList());
}

QString HImageItem::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QStringList HImageItem::publishers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_publisher, &value);
    return value.toStringList();
}

QDateTime HImageItem::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

QStringList HImageItem::rights() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_rights, &value);
    return value.toStringList();
}

void HImageItem::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, QVariant::fromValue(arg));
}

void HImageItem::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HImageItem::setRatings(const QList<HRating>& arg)
{
    setCdsProperty(HCdsProperties::upnp_rating, toList(arg));
}

void HImageItem::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, QVariant::fromValue(arg));
}

void HImageItem::setPublishers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_publisher, QVariant::fromValue(arg));
}

void HImageItem::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, QVariant::fromValue(arg));
}

void HImageItem::setRights(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_rights, QVariant::fromValue(arg));
}

}
}
}
