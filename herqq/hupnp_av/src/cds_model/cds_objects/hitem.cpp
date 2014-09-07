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

#include "hitem.h"
#include "hitem_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../../common/hresource.h"
#include "../../common/hprotocolinfo.h"

#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HItemPrivate
 ******************************************************************************/
HItemPrivate::HItemPrivate(const QString& clazz, HObject::CdsType cdsType) :
    HObjectPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_bookmarkID));
    insert(inst.get(HCdsProperties::dlite_refId));
}

/*******************************************************************************
 * HItem
 ******************************************************************************/
HItem::HItem(const QString& clazz, CdsType cdsType) :
    HObject(*new HItemPrivate(clazz, cdsType))
{
}

HItem::HItem(HItemPrivate& dd) :
    HObject(dd)
{
}

HItem::HItem(
    const QString& title, const QString& parentId, const QString& id) :
        HObject(*new HItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HItem::~HItem()
{
}

HItem* HItem::newInstance() const
{
    return new HItem();
}

QStringList HItem::bookmarkIds() const
{
    QVariant variant;
    bool b = getCdsProperty(HCdsProperties::upnp_bookmarkID, &variant);
    Q_ASSERT(b); Q_UNUSED(b)
    return variant.toStringList();
}

QString HItem::refId() const
{
    QVariant variant;
    bool b = getCdsProperty(HCdsProperties::dlite_refId, &variant);
    Q_ASSERT(b); Q_UNUSED(b)
    return variant.toString();
}

bool HItem::isRef() const
{
    return !refId().isEmpty();
}

bool HItem::hasContentFormat() const
{
    foreach(const HResource& resource, resources())
    {
        QString contentFormat = resource.protocolInfo().contentFormat();
        if (!contentFormat.isEmpty() && contentFormat != "*")
        {
            return true;
        }
    }
    return false;
}

void HItem::setBookmarkIds(const QStringList& bookmarkId)
{
    bool b = setCdsProperty(HCdsProperties::upnp_bookmarkID, bookmarkId);
    Q_ASSERT(b); Q_UNUSED(b)
}

void HItem::setRefId(const QString& referenceId)
{
    bool b = setCdsProperty(HCdsProperties::dlite_refId, referenceId);
    Q_ASSERT(b); Q_UNUSED(b)
}

void HItem::setContentFormat(const QString& mimeType)
{
    HResources newRes, oldRes = resources();
    if (!oldRes.isEmpty())
    {
        foreach(HResource resource, oldRes)
        {
            HProtocolInfo pinfo = resource.protocolInfo();
            pinfo.setContentFormat(mimeType);
            resource.setProtocolInfo(pinfo);

            newRes.append(resource);
        }
    }
    else
    {
        HProtocolInfo pinfo = HProtocolInfo::createUsingWildcards();
        pinfo.setContentFormat(mimeType);
        newRes.append(HResource(pinfo));
    }

    setResources(newRes);
}

}
}
}
