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

#include "hplaylistitem.h"
#include "hplaylistitem_p.h"

#include "../hgenre.h"
#include "../hpersonwithrole.h"
#include "../../common/hstoragemedium.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPlaylistItemPrivate
 ******************************************************************************/
HPlaylistItemPrivate::HPlaylistItemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_artist));
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::dc_date));
    insert(inst.get(HCdsProperties::dc_language));
}

/*******************************************************************************
 * HPlaylistItem
 ******************************************************************************/
HPlaylistItem::HPlaylistItem(const QString& clazz, CdsType cdsType) :
    HItem(*new HPlaylistItemPrivate(clazz, cdsType))
{
}

HPlaylistItem::HPlaylistItem(HPlaylistItemPrivate& dd) :
    HItem(dd)
{
}

HPlaylistItem::HPlaylistItem(
    const QString& title, const QString& parentId, const QString& id) :
        HItem(*new HPlaylistItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HPlaylistItem::~HPlaylistItem()
{
}

HPlaylistItem* HPlaylistItem::newInstance() const
{
    return new HPlaylistItem();
}

void HPlaylistItem::setArtists(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_artist, toList(arg));
}

void HPlaylistItem::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HPlaylistItem::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HPlaylistItem::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HPlaylistItem::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

void HPlaylistItem::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

void HPlaylistItem::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

QList<HPersonWithRole> HPlaylistItem::artists() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_artist, &value);
    return toList<HPersonWithRole>(value.toList());
}

QList<HGenre> HPlaylistItem::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QString HPlaylistItem::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

HStorageMedium HPlaylistItem::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QString HPlaylistItem::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QDateTime HPlaylistItem::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

QStringList HPlaylistItem::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

}
}
}
