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

#include "haudiobook.h"
#include "haudiobook_p.h"
#include "../model_mgmt/hcdsproperties.h"
#include "../../common/hstoragemedium.h"

#include <QtCore/QDateTime>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAudioBookPrivate
 ******************************************************************************/
HAudioBookPrivate::HAudioBookPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HAudioItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::upnp_producer));
    insert(inst.get(HCdsProperties::dc_contributor));
    insert(inst.get(HCdsProperties::dc_date));
}

/*******************************************************************************
 * HAudioBook
 ******************************************************************************/
HAudioBook::HAudioBook(const QString& clazz, CdsType cdsType) :
    HAudioItem(*new HAudioBookPrivate(clazz, cdsType))
{
}

HAudioBook::HAudioBook(HAudioBookPrivate& dd) :
    HAudioItem(dd)
{
}

HAudioBook::HAudioBook(
    const QString& title, const QString& parentId,
    const QString& id) :
        HAudioItem(*new HAudioBookPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HAudioBook::~HAudioBook()
{
}

HAudioBook* HAudioBook::newInstance() const
{
    return new HAudioBook();
}

void HAudioBook::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HAudioBook::setProducers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_producer, arg);
}

void HAudioBook::setContributors(const QStringList &arg)
{
    setCdsProperty(HCdsProperties::dc_contributor, arg);
}

void HAudioBook::setDate(const QDateTime &arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

HStorageMedium HAudioBook::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QStringList HAudioBook::producers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_producer, &value);
    return value.toStringList();
}

QStringList HAudioBook::contributors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_contributor, &value);
    return value.toStringList();
}

QDateTime HAudioBook::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

}
}
}
