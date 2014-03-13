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

#include "hstoragesystem.h"
#include "hstoragesystem_p.h"
#include "../../common/hstoragemedium.h"
#include "../model_mgmt/hcdsproperties.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HStorageSystemPrivate
 ******************************************************************************/
HStorageSystemPrivate::HStorageSystemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_storageTotal).name(), -1LL);
    insert(inst.get(HCdsProperties::upnp_storageUsed).name(), -1LL);
    insert(inst.get(HCdsProperties::upnp_storageFree).name(), -1LL);
    insert(inst.get(HCdsProperties::upnp_storageMaxPartition).name(), -1LL);
    insert(inst.get(HCdsProperties::upnp_storageMedium));
}

/*******************************************************************************
 * HStorageSystem
 ******************************************************************************/
HStorageSystem::HStorageSystem(const QString& clazz, CdsType cdsType) :
    HContainer(*new HStorageSystemPrivate(clazz, cdsType))
{
}

HStorageSystem::HStorageSystem(HStorageSystemPrivate& dd) :
    HContainer(dd)
{
}

HStorageSystem::HStorageSystem(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HStorageSystemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HStorageSystem::~HStorageSystem()
{
}

qint64 HStorageSystem::storageTotal() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageTotal, &value);
    return value.toLongLong();
}

qint64 HStorageSystem::storageUsed() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageUsed, &value);
    return value.toLongLong();
}

qint64 HStorageSystem::storageFree() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageFree, &value);
    return value.toLongLong();
}

qint64 HStorageSystem::storageMaxPartition() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMaxPartition, &value);
    return value.toLongLong();
}

HStorageMedium HStorageSystem::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

void HStorageSystem::setStorageTotal(qint64 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageTotal, arg);
}

void HStorageSystem::setStorageUsed(qint64 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageUsed, arg);
}

void HStorageSystem::setStorageFree(qint64 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageFree, arg);
}

void HStorageSystem::setStorageMaxPartition(qint64 arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMaxPartition, arg);
}

void HStorageSystem::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

HStorageSystem* HStorageSystem::newInstance() const
{
    return new HStorageSystem();
}

}
}
}
