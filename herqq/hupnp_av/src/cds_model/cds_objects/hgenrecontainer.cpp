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

#include "hgenrecontainer.h"
#include "hgenrecontainer_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../hgenre.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HGenreContainerPrivate
 ******************************************************************************/
HGenreContainerPrivate::HGenreContainerPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::dc_description));
}

/*******************************************************************************
 * HGenreContainer
 ******************************************************************************/
HGenreContainer::HGenreContainer(const QString& clazz, CdsType cdsType) :
    HContainer(*new HGenreContainerPrivate(clazz, cdsType))
{
}

HGenreContainer::HGenreContainer(HGenreContainerPrivate& dd) :
    HContainer(dd)
{
}

HGenreContainer::HGenreContainer(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HGenreContainerPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HGenreContainer::~HGenreContainer()
{
}

HGenreContainer* HGenreContainer::newInstance() const
{
    return new HGenreContainer();
}

void HGenreContainer::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HGenreContainer::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HGenreContainer::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

QList<HGenre> HGenreContainer::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QString HGenreContainer::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

QString HGenreContainer::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

}
}
}
