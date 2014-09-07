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

#include "hbookmarkfolder.h"
#include "hbookmarkfolder_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../hgenre.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HBookmarkFolderPrivate
 ******************************************************************************/
HBookmarkFolderPrivate::HBookmarkFolderPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::dc_description));
}

/*******************************************************************************
 * HBookmarkFolder
 ******************************************************************************/
HBookmarkFolder::HBookmarkFolder(const QString& clazz, CdsType cdsType) :
    HContainer(*new HBookmarkFolderPrivate(clazz, cdsType))
{
}

HBookmarkFolder::HBookmarkFolder(HBookmarkFolderPrivate& dd) :
    HContainer(dd)
{
}

HBookmarkFolder::HBookmarkFolder(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HBookmarkFolderPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HBookmarkFolder::~HBookmarkFolder()
{
}

HBookmarkFolder* HBookmarkFolder::newInstance() const
{
    return new HBookmarkFolder();
}

void HBookmarkFolder::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HBookmarkFolder::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HBookmarkFolder::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

QList<HGenre> HBookmarkFolder::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QString HBookmarkFolder::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

QString HBookmarkFolder::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

}
}
}
