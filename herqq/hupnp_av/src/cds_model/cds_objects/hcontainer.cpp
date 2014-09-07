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

#include "hcontainer.h"
#include "hcontainer_p.h"
#include "../hcdsclassinfo.h"
#include "../model_mgmt/hcdsproperties.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HContainerEventInfoPrivate
 ******************************************************************************/
class HContainerEventInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HContainerEventInfoPrivate)

public:

    HContainerEventInfo::EventType m_eventType;
    QString m_childId;
    quint32 m_updateId;

    HContainerEventInfoPrivate() :
        m_eventType(), m_childId(), m_updateId(0)
    {
    }
};

/*******************************************************************************
 * HContainerEventInfo
 ******************************************************************************/
HContainerEventInfo::HContainerEventInfo() :
    h_ptr(new HContainerEventInfoPrivate())
{
}

HContainerEventInfo::HContainerEventInfo(
    EventType type, const QString& childId, quint32 updateId) :
        h_ptr(new HContainerEventInfoPrivate())
{
    h_ptr->m_eventType = type;
    h_ptr->m_childId = childId.trimmed();
    h_ptr->m_updateId = updateId;
}

HContainerEventInfo::HContainerEventInfo(const HContainerEventInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HContainerEventInfo& HContainerEventInfo::operator=(const HContainerEventInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HContainerEventInfo::~HContainerEventInfo()
{
}

bool HContainerEventInfo::isValid() const
{
    return h_ptr->m_eventType != Undefined && !h_ptr->m_childId.isEmpty();
}

HContainerEventInfo::EventType HContainerEventInfo::type() const
{
    return h_ptr->m_eventType;
}

QString HContainerEventInfo::childId() const
{
    return h_ptr->m_childId;
}

quint32 HContainerEventInfo::updateId() const
{
    return h_ptr->m_updateId;
}

void HContainerEventInfo::setType(EventType type)
{
    h_ptr->m_eventType = type;
}

void HContainerEventInfo::setChildId(const QString& arg)
{
    h_ptr->m_childId = arg.trimmed();
}

void HContainerEventInfo::setUpdateId(quint32 arg)
{
    h_ptr->m_updateId = arg;
}

bool operator==(const HContainerEventInfo& obj1, const HContainerEventInfo& obj2)
{
    return obj1.childId() == obj2.childId() &&
           obj1.updateId() == obj2.updateId() &&
           obj1.type() == obj2.type();
}

/*******************************************************************************
 * HContainerPrivate
 ******************************************************************************/
HContainerPrivate::HContainerPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HObjectPrivate(clazz, cdsType), m_childIds()
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_containerUpdateID).name(), 0U);
    insert(inst.get(HCdsProperties::upnp_totalDeletedChildCount).name(), 0U);
    insert(inst.get(HCdsProperties::dlite_childCount).name(), 0U);
    insert(inst.get(HCdsProperties::upnp_createClass));
    insert(inst.get(HCdsProperties::upnp_searchClass));
    insert(inst.get(HCdsProperties::dlite_searchable).name(), false);
}

/*******************************************************************************
 * HContainer
 ******************************************************************************/
HContainer::HContainer(const QString& clazz, CdsType cdsType) :
    HObject(*new HContainerPrivate(clazz, cdsType))
{
}

HContainer::HContainer(HContainerPrivate& dd) :
    HObject(dd)
{
}

HContainer::HContainer(
    const QString& title, const QString& parentId, const QString& id) :
        HObject(*new HContainerPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HContainer::~HContainer()
{
}

void HContainer::setContainerUpdateId(quint32 arg)
{
    bool ok = setCdsProperty(HCdsProperties::upnp_containerUpdateID, arg);
    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HContainer::setTotalDeletedChildCount(quint32 arg)
{
    bool ok = setCdsProperty(HCdsProperties::upnp_totalDeletedChildCount, arg);
    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HContainer::setExpectedChildCount(quint32 arg)
{
    H_D(HContainer);
    qint32 size = h->m_childIds.size();
    if (size && static_cast<quint32>(size) != arg)
    {
        Q_ASSERT_X(false, "", "Expected child count should not be different "
                              "than the size of non-empty child set");
        return;
    }

    setCdsProperty(HCdsProperties::dlite_childCount, arg);
}

bool HContainer::validate() const
{
    return true;
}

HContainer* HContainer::newInstance() const
{
    return new HContainer();
}

void HContainer::doClone(HClonable* target) const
{
    const H_D(HContainer);
    HContainer* obj = dynamic_cast<HContainer*>(target);
    if (obj)
    {
        static_cast<HContainerPrivate*>(obj->h_ptr)->m_childIds = h->m_childIds;
        HObject::doClone(obj);
    }
}

bool HContainer::hasChildId(const QString& childId) const
{
    const H_D(HContainer);
    return h->m_childIds.contains(childId);
}

void HContainer::setChildIds(const QSet<QString>& childIds)
{
    H_D(HContainer);

    bool differentExpectedCount = childIds.size() != h->m_childIds.size();

    QSet<QString> copy(childIds);

    QSet<QString>::iterator it = h->m_childIds.begin();
    while(it != h->m_childIds.end())
    {
        QString id = *it;
        if (!copy.contains(id))
        {
            it = h->m_childIds.erase(it);
            emit containerModified(
                this, HContainerEventInfo(HContainerEventInfo::ChildRemoved, id));
        }
        else
        {
            copy.remove(id);
            ++it;
        }
    }

    foreach(const QString& id, copy)
    {
        h->m_childIds.insert(id);
        emit containerModified(
            this, HContainerEventInfo(HContainerEventInfo::ChildAdded, id));
    }

    if (differentExpectedCount)
    {
        setExpectedChildCount(childIds.size());
    }
}

void HContainer::addChildIds(const QSet<QString>& childIds)
{
    H_D(HContainer);

    bool modified = false;
    foreach(const QString& id, childIds)
    {
        if (!h->m_childIds.contains(id))
        {
            h->m_childIds.insert(id);

            emit containerModified(
                this, HContainerEventInfo(HContainerEventInfo::ChildAdded, id));

            modified = true;
        }
    }

    if (modified)
    {
        setExpectedChildCount(h->m_childIds.size());
    }
}

void HContainer::addChildId(const QString& childId)
{
    H_D(HContainer);

    if (!h->m_childIds.contains(childId))
    {
        h->m_childIds.insert(childId);

        emit containerModified(
            this, HContainerEventInfo(HContainerEventInfo::ChildAdded, childId));

        setExpectedChildCount(h->m_childIds.size());
    }
}

void HContainer::removeChildId(const QString& childId)
{
    H_D(HContainer);

    if (h->m_childIds.contains(childId))
    {
        h->m_childIds.remove(childId);

        emit containerModified(
            this, HContainerEventInfo(HContainerEventInfo::ChildRemoved, childId));

        setExpectedChildCount(h->m_childIds.size());
    }
}

void HContainer::removeChildIds(const QSet<QString>& childIDs)
{
    H_D(HContainer);

    bool modified = false;
    foreach(const QString& id, childIDs)
    {
        if (h->m_childIds.contains(id))
        {
            h->m_childIds.remove(id);

            emit containerModified(
                this, HContainerEventInfo(HContainerEventInfo::ChildRemoved, id));

            modified = true;
        }
    }

    if (modified)
    {
        setExpectedChildCount(h->m_childIds.size());
    }
}

/*
bool HContainer::doCompare(
    const HObject* other, const HSortInfo& info, qint32* retVal)
{
    Q_ASSERT(other);
    Q_ASSERT(retVal);

    H_D(HContainer);
    QReadLocker locker(&h->m_contentLock);

    if (!other->isContainer())
    {
        return false;
    }

    const HContainer* otherContainer = static_cast<const HContainer*>(other);

    QString property = info.property();
    if (property == "@childCount" || property == "container@childCount")
    {
        *retVal = otherContainer->expectedChildCount() - h->m_expectedChildCount;
    }
    else if (property == "upnp:totalDeletedChildCount")
    {
        *retVal = otherContainer->totalDeletedChildCount() -
                  h->m_totalDeletedChildCount;
    }
    else if (property == "upnp:containerUpdateID")
    {
        *retVal = otherContainer->containerUpdateId() - h->m_containerUpdateId;
    }
    else if (property == "@searchable" || property == "container@searchable")
    {
        *retVal = searchable() != otherContainer->searchable() ? -1 : 0;
    }
    else if (property == "@neverPlayable" || property == "container@neverPlayable")
    {
        *retVal = neverPlayable() != otherContainer->neverPlayable() ? -1 : 0;
    }
    else
    {
        return false;
    }

    return true;
}*/

QSet<QString> HContainer::childIds() const
{
    const H_D(HContainer);
    return h->m_childIds;
}

bool HContainer::searchable() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dlite_searchable, &value);
    return value.toBool();
}

void HContainer::setSearchable(bool arg)
{
    setCdsProperty(HCdsProperties::dlite_searchable, arg);
}

quint32 HContainer::containerUpdateId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_containerUpdateID, &value);
    return value.toUInt();
}

QList<HCdsClassInfo> HContainer::createClassInfos() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_createClass, &value);
    return toList<HCdsClassInfo>(value.toList());
}

quint32 HContainer::expectedChildCount() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dlite_childCount, &value);
    return value.toUInt();
}

QList<HCdsClassInfo> HContainer::searchClassInfos() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_searchClass, &value);
    return toList<HCdsClassInfo>(value.toList());
}

quint32 HContainer::totalDeletedChildCount() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_totalDeletedChildCount, &value);
    return value.toUInt();
}

void HContainer::setCreateClassInfos(const QList<HCdsClassInfo>& arg)
{
    setCdsProperty(HCdsProperties::upnp_createClass, toList(arg));
}

void HContainer::setSearchClassInfos(const QList<HCdsClassInfo>& arg)
{
    setCdsProperty(HCdsProperties::upnp_searchClass, toList(arg));
}

}
}
}
