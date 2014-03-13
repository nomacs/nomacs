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

#include "htextitem.h"
#include "htextitem_p.h"

#include "../hpersonwithrole.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../../common/hrating.h"
#include "../../common/hstoragemedium.h"

#include <QtCore/QUrl>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HTextItemPrivate
 ******************************************************************************/
HTextItemPrivate::HTextItemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_author));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::upnp_rating));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::dc_publisher));
    insert(inst.get(HCdsProperties::dc_contributor));
    insert(inst.get(HCdsProperties::dc_date));
    insert(inst.get(HCdsProperties::dc_relation));
    insert(inst.get(HCdsProperties::dc_language));
    insert(inst.get(HCdsProperties::dc_rights));
}

/*******************************************************************************
 * HTextItem
 ******************************************************************************/
HTextItem::HTextItem(const QString& clazz, CdsType cdsType) :
    HItem(*new HTextItemPrivate(clazz, cdsType))
{
}

HTextItem::HTextItem(
    const QString& title, const QString& parentId, const QString& id) :
        HItem(*new HTextItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HTextItem::~HTextItem()
{
}

HTextItem* HTextItem::newInstance() const
{
    return new HTextItem();
}

QList<HPersonWithRole> HTextItem::authors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_author, &value);
    return toList<HPersonWithRole>(value.toList());
}

QString HTextItem::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

HStorageMedium HTextItem::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QList<HRating> HTextItem::ratings() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_rating, &value);
    return toList<HRating>(value.toList());
}

QString HTextItem::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QStringList HTextItem::publishers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_publisher, &value);
    return value.toStringList();
}

QStringList HTextItem::contributors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_contributor, &value);
    return value.toStringList();
}

QDateTime HTextItem::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

QList<QUrl> HTextItem::relations() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_relation, &value);
    return toList<QUrl>(value.toList());
}

QStringList HTextItem::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

QStringList HTextItem::rights() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_rights, &value);
    return value.toStringList();
}

void HTextItem::setAuthors(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_author, toList(arg));
}

void HTextItem::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HTextItem::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HTextItem::setRatings(const QList<HRating>& arg)
{
    setCdsProperty(HCdsProperties::upnp_rating, toList(arg));
}

void HTextItem::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

void HTextItem::setPublishers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_publisher, arg);
}

void HTextItem::setContributors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_contributor, arg);
}

void HTextItem::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

void HTextItem::setRelations(const QList<QUrl>& arg)
{
    setCdsProperty(HCdsProperties::dc_relation, toList(arg));
}

void HTextItem::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

void HTextItem::setRights(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_rights, arg);
}

}
}
}
