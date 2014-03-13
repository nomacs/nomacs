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

#include "hplaylistcontainer.h"
#include "hplaylistcontainer_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../hgenre.h"
#include "../hpersonwithrole.h"
#include "../../common/hstoragemedium.h"

#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPlaylistContainerPrivate
 ******************************************************************************/
HPlaylistContainerPrivate::HPlaylistContainerPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_artist));
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::upnp_producer));
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::dc_contributor));
    insert(inst.get(HCdsProperties::dc_date));
    insert(inst.get(HCdsProperties::dc_language));
    insert(inst.get(HCdsProperties::dc_rights));
}

/*******************************************************************************
 * HPlaylistContainer
 ******************************************************************************/
HPlaylistContainer::HPlaylistContainer(const QString& clazz, CdsType cdsType) :
    HContainer(*new HPlaylistContainerPrivate(clazz, cdsType))
{
}

HPlaylistContainer::HPlaylistContainer(HPlaylistContainerPrivate& dd) :
    HContainer(dd)
{
}

HPlaylistContainer::HPlaylistContainer(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HPlaylistContainerPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HPlaylistContainer::~HPlaylistContainer()
{
}

HPlaylistContainer* HPlaylistContainer::newInstance() const
{
    return new HPlaylistContainer();
}

void HPlaylistContainer::setArtists(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_artist, toList(arg));
}

void HPlaylistContainer::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HPlaylistContainer::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HPlaylistContainer::setProducers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_producer, arg);
}

void HPlaylistContainer::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HPlaylistContainer::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

void HPlaylistContainer::setContributors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_contributor, arg);
}

void HPlaylistContainer::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

void HPlaylistContainer::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

void HPlaylistContainer::setRights(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_rights, arg);
}

QList<HPersonWithRole> HPlaylistContainer::artists() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_artist, &value);
    return toList<HPersonWithRole>(value.toList());
}

QList<HGenre> HPlaylistContainer::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QString HPlaylistContainer::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

QStringList HPlaylistContainer::producers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_producer, &value);
    return value.toStringList();
}

HStorageMedium HPlaylistContainer::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QString HPlaylistContainer::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QStringList HPlaylistContainer::contributors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_contributor, &value);
    return value.toStringList();
}

QDateTime HPlaylistContainer::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

QStringList HPlaylistContainer::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

QStringList HPlaylistContainer::rights() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_rights, &value);
    return value.toStringList();
}

}
}
}
