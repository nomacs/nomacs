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

#include "hcds_lastchange_info.h"

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HCdsLastChangeInfoPrivate
 ******************************************************************************/
class HCdsLastChangeInfoPrivate
{
public:

    QString m_objectId, m_parentId, m_upnpClass;
    HCdsLastChangeInfo::EventType m_eventType;
    quint32 m_updateId;
    bool m_stUpdate;

    HCdsLastChangeInfoPrivate() :
        m_objectId(), m_parentId(), m_upnpClass(),
        m_eventType(HCdsLastChangeInfo::Undefined),
        m_updateId(0), m_stUpdate(false)
    {
    }
};

/*******************************************************************************
 * HCdsLastChangeInfo
 ******************************************************************************/
HCdsLastChangeInfo::HCdsLastChangeInfo() :
    h_ptr(new HCdsLastChangeInfoPrivate())
{
}

HCdsLastChangeInfo::HCdsLastChangeInfo(const QString& objectId, EventType eventType) :
    h_ptr(new HCdsLastChangeInfoPrivate())
{
    h_ptr->m_objectId = objectId.trimmed();
    h_ptr->m_eventType = eventType;
}

HCdsLastChangeInfo::~HCdsLastChangeInfo()
{
    delete h_ptr;
}

HCdsLastChangeInfo::HCdsLastChangeInfo(const HCdsLastChangeInfo& other) :
    h_ptr(new HCdsLastChangeInfoPrivate(*other.h_ptr))
{
    Q_ASSERT(this != &other);
}

HCdsLastChangeInfo& HCdsLastChangeInfo::operator=(const HCdsLastChangeInfo& other)
{
    Q_ASSERT(this != &other);
    delete h_ptr;
    h_ptr = new HCdsLastChangeInfoPrivate(*other.h_ptr);
    return *this;
}

void HCdsLastChangeInfo::setObjectId(const QString& id)
{
    h_ptr->m_objectId = id.trimmed();
}

void HCdsLastChangeInfo::setEventType(EventType type)
{
    h_ptr->m_eventType = type;
}

void HCdsLastChangeInfo::setParentId(const QString& id)
{
    h_ptr->m_parentId = id.trimmed();
}

void HCdsLastChangeInfo::setObjectClass(const QString& objClass)
{
    h_ptr->m_upnpClass = objClass.trimmed();
}

void HCdsLastChangeInfo::setUpdateId(quint32 arg)
{
    h_ptr->m_updateId = arg;
}

void HCdsLastChangeInfo::setStUpdate(bool set)
{
    h_ptr->m_stUpdate = set;
}

bool HCdsLastChangeInfo::isValid() const
{
    bool b = true;
    if (eventType() == ObjectAdded)
    {
        b = !parentId().isEmpty() && !objectClass().isEmpty();
    }
    return b && !h_ptr->m_objectId.isEmpty() && h_ptr->m_eventType != Undefined;
}

QString HCdsLastChangeInfo::objectId() const
{
    return h_ptr->m_objectId;
}

HCdsLastChangeInfo::EventType HCdsLastChangeInfo::eventType() const
{
    return h_ptr->m_eventType;
}

QString HCdsLastChangeInfo::parentId() const
{
    return h_ptr->m_parentId;
}

QString HCdsLastChangeInfo::objectClass() const
{
    return h_ptr->m_upnpClass;
}

quint32 HCdsLastChangeInfo::updateId() const
{
    return h_ptr->m_updateId;
}

bool HCdsLastChangeInfo::stUpdate() const
{
    return h_ptr->m_stUpdate;
}

bool operator==(const HCdsLastChangeInfo& obj1, const HCdsLastChangeInfo& obj2)
{
    return obj1.objectId() == obj2.objectId() &&
           obj1.eventType() == obj2.eventType() &&
           obj1.parentId() == obj2.parentId() &&
           obj1.objectClass() == obj2.objectClass() &&
           obj1.updateId() == obj2.updateId() &&
           obj1.stUpdate() == obj2.stUpdate();
}

}
}
}
