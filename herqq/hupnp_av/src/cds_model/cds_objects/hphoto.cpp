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

#include "hphoto.h"
#include "hphoto_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPhotoPrivate
 ******************************************************************************/
HPhotoPrivate::HPhotoPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HImageItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_album));
}

/*******************************************************************************
 * HPhoto
 ******************************************************************************/
HPhoto::HPhoto(const QString& clazz, CdsType cdsType) :
    HImageItem(*new HPhotoPrivate(clazz, cdsType))
{
}

HPhoto::HPhoto(
    const QString& title, const QString& parentId, const QString& id) :
        HImageItem(*new HPhotoPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HPhoto::~HPhoto()
{
}

HPhoto* HPhoto::newInstance() const
{
    return new HPhoto();
}

void HPhoto::setAlbums(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_album, arg);
}

QStringList HPhoto::albums() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_album, &value);
    return value.toStringList();
}

}
}
}
