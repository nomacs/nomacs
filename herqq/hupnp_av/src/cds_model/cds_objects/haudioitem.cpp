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

#include "haudioitem.h"
#include "haudioitem_p.h"

#include "../hgenre.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAudioItemPrivate
 ******************************************************************************/
HAudioItemPrivate::HAudioItemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::dc_publisher));
    insert(inst.get(HCdsProperties::dc_language));
    insert(inst.get(HCdsProperties::dc_relation));
    insert(inst.get(HCdsProperties::dc_rights));
}

/*******************************************************************************
 * HAudioItem
 ******************************************************************************/
HAudioItem::HAudioItem(const QString& clazz, CdsType cdsType) :
    HItem(*new HAudioItemPrivate(clazz, cdsType))
{
}

HAudioItem::HAudioItem(HAudioItemPrivate& dd) :
    HItem(dd)
{
}

HAudioItem::HAudioItem(
    const QString& title, const QString& parentId,
    const QString& id) :
        HItem(*new HAudioItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HAudioItem::~HAudioItem()
{
}

HAudioItem* HAudioItem::newInstance() const
{
    return new HAudioItem();
}

QList<HGenre> HAudioItem::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QString HAudioItem::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QString HAudioItem::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

QStringList HAudioItem::publishers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_publisher, &value);
    return value.toStringList();
}

QStringList HAudioItem::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

QList<QUrl> HAudioItem::relations() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_relation, &value);
    return toList<QUrl>(value.toList());
}

QStringList HAudioItem::rights() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_rights, &value);
    return value.toStringList();
}

void HAudioItem::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HAudioItem::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

void HAudioItem::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HAudioItem::setPublishers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_publisher, arg);
}

void HAudioItem::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

void HAudioItem::setRelations(const QList<QUrl>& arg)
{
    setCdsProperty(HCdsProperties::dc_relation, toList(arg));
}

void HAudioItem::setRights(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_rights, arg);
}

}
}
}
