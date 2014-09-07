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

#include "hstoragevolume.h"
#include "hstoragevolume_p.h"
#include "../../common/hstoragemedium.h"
#include "../model_mgmt/hcdsproperties.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HStorageVolumePrivate
 ******************************************************************************/
HStorageVolumePrivate::HStorageVolumePrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_storageTotal).name(), -1LL);
    insert(inst.get(HCdsProperties::upnp_storageUsed).name(), -1LL);
    insert(inst.get(HCdsProperties::upnp_storageFree).name(), -1LL);
    insert(inst.get(HCdsProperties::upnp_storageMedium));
}

/*******************************************************************************
 * HStorageVolume
 ******************************************************************************/
HStorageVolume::HStorageVolume(const QString& clazz, CdsType cdsType) :
    HContainer(*new HStorageVolumePrivate(clazz, cdsType))
{
}

HStorageVolume::HStorageVolume(HStorageVolumePrivate& dd) :
    HContainer(dd)
{
}

HStorageVolume::HStorageVolume(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HStorageVolumePrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HStorageVolume::~HStorageVolume()
{
}

qint64 HStorageVolume::storageTotal() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageTotal, &value);
    return value.toLongLong();
}

qint64 HStorageVolume::storageUsed() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageUsed, &value);
    return value.toLongLong();
}

qint64 HStorageVolume::storageFree() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageFree, &value);
    return value.toLongLong();
}

HStorageMedium HStorageVolume::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

void HStorageVolume::setStorageTotal(qint64 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageTotal, arg);
}

void HStorageVolume::setStorageUsed(qint64 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageUsed, arg);
}

void HStorageVolume::setStorageFree(qint64 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageFree, arg);
}

void HStorageVolume::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

HStorageVolume* HStorageVolume::newInstance() const
{
    return new HStorageVolume();
}

}
}
}
