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

#include "halbum.h"
#include "halbum_p.h"
#include "../model_mgmt/hcdsproperties.h"
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
 * HAlbumPrivate
 ******************************************************************************/
HAlbumPrivate::HAlbumPrivate(const QString& clazz, HObject::CdsType cdsType) :
    HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::dc_publisher));
    insert(inst.get(HCdsProperties::dc_contributor));
    insert(inst.get(HCdsProperties::dc_date));
    insert(inst.get(HCdsProperties::dc_relation));
    insert(inst.get(HCdsProperties::dc_rights));
}

/*******************************************************************************
 * HAlbum
 ******************************************************************************/
HAlbum::HAlbum(const QString& clazz, CdsType cdsType) :
    HContainer(*new HAlbumPrivate(clazz, cdsType))
{
}

HAlbum::HAlbum(HAlbumPrivate& dd) :
    HContainer(dd)
{
}

HAlbum::HAlbum(
    const QString& title, const QString& parentId,
    const QString& id) :
        HContainer(*new HAlbumPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HAlbum::~HAlbum()
{
}

HAlbum* HAlbum::newInstance() const
{
    return new HAlbum();
}

void HAlbum::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HAlbum::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HAlbum::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

void HAlbum::setPublishers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_publisher, arg);
}

void HAlbum::setContributors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_contributor, arg);
}

void HAlbum::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

void HAlbum::setRelations(const QList<QUrl>& arg)
{
    setCdsProperty(HCdsProperties::dc_relation, toList(arg));
}

void HAlbum::setRights(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_rights, arg);
}

HStorageMedium HAlbum::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QString HAlbum::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

QString HAlbum::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QStringList HAlbum::publishers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_publisher, &value);
    return value.toStringList();
}

QStringList HAlbum::contributors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_contributor, &value);
    return value.toStringList();
}

QDateTime HAlbum::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

QList<QUrl> HAlbum::relations() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_relation, &value);
    return toList<QUrl>(value.toList());
}

QStringList HAlbum::rights() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_rights, &value);
    return value.toStringList();
}

}
}
}
