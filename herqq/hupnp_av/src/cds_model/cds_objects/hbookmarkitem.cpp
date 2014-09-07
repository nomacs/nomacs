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

#include "hbookmarkitem.h"
#include "hbookmarkitem_p.h"
#include "../hdeviceudn.h"
#include "../hstatevariablecollection.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HBookmarkItemPrivate
 ******************************************************************************/
HBookmarkItemPrivate::HBookmarkItemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_bookmarkedObjectID));
    insert(inst.get(HCdsProperties::dlite_neverPlayable));
    insert(inst.get(HCdsProperties::upnp_deviceUdn));
    insert(inst.get(HCdsProperties::dc_date));
    insert(inst.get(HCdsProperties::upnp_stateVariableCollection));
}

/*******************************************************************************
 * HBookmarkItem
 ******************************************************************************/
HBookmarkItem::HBookmarkItem(const QString& clazz, CdsType cdsType) :
    HItem(*new HBookmarkItemPrivate(clazz, cdsType))
{
}

HBookmarkItem::HBookmarkItem(HBookmarkItemPrivate& dd) :
    HItem(dd)
{
}

HBookmarkItem::HBookmarkItem(
    const QString& title, const QString& parentId, const QString& id) :
        HItem(*new HBookmarkItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HBookmarkItem::~HBookmarkItem()
{
}

HBookmarkItem* HBookmarkItem::newInstance() const
{
    return new HBookmarkItem();
}

void HBookmarkItem::setBookmarkedObjectId(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_bookmarkedObjectID, arg);
}

void HBookmarkItem::setNeverPlayable(bool arg)
{
    setCdsProperty(HCdsProperties::dlite_neverPlayable, arg);
}

void HBookmarkItem::setDeviceUdn(const HDeviceUdn& arg)
{
    setCdsProperty(
        HCdsProperties::upnp_deviceUdn, QVariant::fromValue(arg));
}

void HBookmarkItem::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

void HBookmarkItem::setStateVariableCollection(
    const HStateVariableCollection& arg)
{
    setCdsProperty(
        HCdsProperties::upnp_stateVariableCollection,
        QVariant::fromValue(arg));
}

QString HBookmarkItem::bookmarkedObjectId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_bookmarkedObjectID, &value);
    return value.toString();
}

bool HBookmarkItem::neverPlayable() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dlite_neverPlayable, &value);
    return value.toBool();
}

HDeviceUdn HBookmarkItem::deviceUdn() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_deviceUdn, &value);
    return value.value<HDeviceUdn>();
}

QDateTime HBookmarkItem::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

HStateVariableCollection HBookmarkItem::stateVariableCollection() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_stateVariableCollection, &value);
    return value.value<HStateVariableCollection>();
}

}
}
}
