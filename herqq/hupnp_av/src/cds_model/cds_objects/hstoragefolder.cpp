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

#include "hstoragefolder.h"
#include "hstoragefolder_p.h"
#include "../model_mgmt/hcdsproperties.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HStorageFolderPrivate
 ******************************************************************************/
HStorageFolderPrivate::HStorageFolderPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_storageUsed).name(), -1LL);
}

/*******************************************************************************
 * HStorageFolder
 ******************************************************************************/
HStorageFolder::HStorageFolder(const QString& clazz, CdsType cdsType) :
    HContainer(*new HStorageFolderPrivate(clazz, cdsType))
{
}

HStorageFolder::HStorageFolder(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HStorageFolderPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HStorageFolder::~HStorageFolder()
{
}

HStorageFolder* HStorageFolder::newInstance() const
{
    return new HStorageFolder();
}

bool HStorageFolder::validate() const
{
    return isCdsPropertySet(HCdsProperties::upnp_storageUsed);
}

qint64 HStorageFolder::storageUsed() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageUsed, &value);
    return value.toLongLong();
}

void HStorageFolder::setStorageUsed(quint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageUsed, arg);
}

}
}
}
