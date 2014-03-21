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

#include "habstract_cds_datasource.h"
#include "habstract_cds_datasource_p.h"
#include "hcds_datasource_configuration.h"
#include "../cds_objects/hitem.h"
#include "../cds_objects/hcontainer.h"

#include <QtCore/QSet>

/*!
 * \defgroup hupnp_av_cds_ds Data Sources
 * \ingroup hupnp_av_cds
 *
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractCdsDataSourcePrivate
 *******************************************************************************/
HAbstractCdsDataSourcePrivate::HAbstractCdsDataSourcePrivate() :
    m_configuration(0), m_objectsById(), m_objectIdsByParentId(),
    m_initialized(false), q_ptr(0)
{
}

HAbstractCdsDataSourcePrivate::HAbstractCdsDataSourcePrivate(
    const HCdsDataSourceConfiguration& conf) :
        m_configuration(conf.clone()), m_objectsById(),
        m_objectIdsByParentId(), m_initialized(false), q_ptr(0)
{
}

HAbstractCdsDataSourcePrivate::~HAbstractCdsDataSourcePrivate()
{
    qDeleteAll(m_objectsById);
    qDeleteAll(m_objectIdsByParentId);
}

void HAbstractCdsDataSourcePrivate::add(HObject* obj)
{
    bool ok = QObject::connect(
        obj,
        SIGNAL(objectModified(Herqq::Upnp::Av::HObject*, Herqq::Upnp::Av::HObjectEventInfo)),
        q_ptr,
        SLOT(objectModified_(Herqq::Upnp::Av::HObject*, Herqq::Upnp::Av::HObjectEventInfo)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    m_objectsById.insert(obj->id(), obj);

    if (obj->isContainer())
    {
        ok = QObject::connect(
            obj,
            SIGNAL(containerModified(Herqq::Upnp::Av::HContainer*, Herqq::Upnp::Av::HContainerEventInfo)),
            q_ptr,
            SLOT(containerModified_(Herqq::Upnp::Av::HContainer*, Herqq::Upnp::Av::HContainerEventInfo)));
        Q_ASSERT(ok);
    }
}

bool HAbstractCdsDataSourcePrivate::add(
    HObject* object, HAbstractCdsDataSource::AddFlag addFlag)
{
    H_Q(HAbstractCdsDataSource);

    bool retVal = false;

    QString id = object->id();
    QString pid = object->parentId();
    switch(addFlag)
    {
    case HAbstractCdsDataSource::AddNewOnly:
        if (!m_objectsById.contains(id))
        {
            add(object);
            retVal = true;
        }
        else
        {
            retVal = false;
        }
        break;

    case HAbstractCdsDataSource::AddAndOverwrite:
        if (m_objectsById.contains(id))
        {
            delete m_objectsById.value(id);
        }
        add(object);
        retVal = true;
        break;

    default:
        Q_ASSERT(false);
    }

    if (retVal && pid != "-1")
    {
        if (!m_objectsById.contains(pid))
        {
            // The parent object of "object" is not in control of this data source.
            // Store the ID of this object so that IF the parent object is added later on,
            // this object can be marked as a child of the parent (that is not yet here).
            QSet<QString>* pids = m_objectIdsByParentId.value(pid);
            if (!pids)
            {
                pids = new QSet<QString>();
                m_objectIdsByParentId.insert(pid, pids);
            }
            pids->insert(id);
            emit q->independentObjectAdded(object);
        }
        else
        {
            // Parent object is in this data source, update it.
            HContainer* container = static_cast<HContainer*>(m_objectsById.value(pid));
            if (!container->hasChildId(id))
            {
                container->addChildId(id);
            }
            else
            {
                HContainerEventInfo einfo(HContainerEventInfo::ChildAdded, id);
                emit q->containerModified(container, einfo);
            }
        }

        // Check if there are object IDs stored and marked as children of the object
        // that was just added.
        QSet<QString>* children = m_objectIdsByParentId.value(id);
        if (children)
        {
            // There are, update the object accordingly.
            Q_ASSERT(HObject::isContainer(object->type()));
            HContainer* container = static_cast<HContainer*>(object);
            container->addChildIds(*children);

            delete children;
            m_objectIdsByParentId.remove(id);
            // Remove the temporary child IDs, since the parent object is now
            // controlled by the data source and its child information is up to date.
        }
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractCdsDataSource
 *******************************************************************************/
HAbstractCdsDataSource::HAbstractCdsDataSource(QObject* parent) :
    QObject(parent),
        h_ptr(new HAbstractCdsDataSourcePrivate())
{
    h_ptr->m_configuration.reset(new HCdsDataSourceConfiguration());
    h_ptr->q_ptr = this;
}

HAbstractCdsDataSource::HAbstractCdsDataSource(
    const HCdsDataSourceConfiguration& conf, QObject* parent) :
        QObject(parent),
            h_ptr(new HAbstractCdsDataSourcePrivate(conf))
{
    h_ptr->q_ptr = this;
}

HAbstractCdsDataSource::HAbstractCdsDataSource(
    HAbstractCdsDataSourcePrivate& dd, QObject* parent) :
        QObject(parent),
            h_ptr(&dd)
{
    if (!h_ptr->m_configuration)
    {
        h_ptr->m_configuration.reset(new HCdsDataSourceConfiguration());
    }

    h_ptr->q_ptr = this;
}

HAbstractCdsDataSource::~HAbstractCdsDataSource()
{
    delete h_ptr;
}

void HAbstractCdsDataSource::objectModified_(
    HObject* source, const HObjectEventInfo& eventInfo)
{
    emit objectModified(source, eventInfo);

    HContainer* parent = findContainer(source->parentId());
    if (parent)
    {
        HContainerEventInfo info(HContainerEventInfo::ChildModified, source->id());
        emit containerModified(parent, info);
    }
}

void HAbstractCdsDataSource::containerModified_(
    HContainer* source, const HContainerEventInfo& eventInfo)
{
    emit containerModified(source, eventInfo);
}

bool HAbstractCdsDataSource::doInit()
{
    return true;
}

const HCdsDataSourceConfiguration* HAbstractCdsDataSource::configuration() const
{
    return h_ptr->m_configuration.data();
}

bool HAbstractCdsDataSource::init()
{
    if (isInitialized())
    {
        return false;
    }

    if (doInit())
    {
        h_ptr->m_initialized = true;
        return true;
    }

    return false;
}

bool HAbstractCdsDataSource::isInitialized() const
{
    return h_ptr->m_initialized;
}

bool HAbstractCdsDataSource::supportsLoading() const
{
    return false;
}

bool HAbstractCdsDataSource::isLoadable(const QString& /*objectId*/) const
{
    return false;
}

HObject* HAbstractCdsDataSource::findObject(const QString& id)
{
    return h_ptr->m_objectsById.value(id);
}

HObjects HAbstractCdsDataSource::findObjects(const QSet<QString>& ids)
{
    QList<HObject*> retVal;
    foreach(const QString& objectId, ids)
    {
        HObject* obj = h_ptr->m_objectsById.value(objectId);
        if (obj)
        {
            retVal.push_back(obj);
        }
    }
    return retVal;
}

HContainer* HAbstractCdsDataSource::findContainerWithTitle(const QString& title)
{
    HContainer* retVal = 0;
    foreach(HContainer* container , containers())
    {
        if (container->title() == title)
        {
            retVal = container;
            break;
        }
    }
    return retVal;
}

HObjects HAbstractCdsDataSource::objects() const
{
    return h_ptr->m_objectsById.values();
}

qint32 HAbstractCdsDataSource::count() const
{
    return h_ptr->m_objectsById.size();
}

HItem* HAbstractCdsDataSource::findItem(const QString& id)
{
    HObject* obj = h_ptr->m_objectsById.value(id);
    return obj && obj->isItem(obj->type()) ?
        static_cast<HItem*>(obj) : 0;
}

HItems HAbstractCdsDataSource::findItems(const QSet<QString>& ids)
{
    QList<HItem*> retVal;
    foreach(const QString& objectId, ids)
    {
        HObject* obj = h_ptr->m_objectsById.value(objectId);
        if (obj && obj->isItem(obj->type()))
        {
            retVal.append(static_cast<HItem*>(obj));
        }
    }
    return retVal;
}

HItems HAbstractCdsDataSource::items() const
{
    QList<HItem*> retVal;

    QHash<QString, HObject*>::iterator it = h_ptr->m_objectsById.begin();
    for (; it != h_ptr->m_objectsById.end(); ++it)
    {
        if (it.value()->isItem())
        {
            retVal.append(static_cast<HItem*>(it.value()));
        }
    }

    return retVal;
}

HContainer* HAbstractCdsDataSource::findContainer(const QString& id)
{
    HObject* obj = h_ptr->m_objectsById.value(id);
    return obj && obj->isContainer(obj->type()) ?
        static_cast<HContainer*>(obj) : 0;
}

HContainers HAbstractCdsDataSource::findContainers(const QSet<QString>& ids)
{
    QList<HContainer*> retVal;

    foreach(const QString& objectId, ids)
    {
        HObject* obj = h_ptr->m_objectsById.value(objectId);
        if (obj && obj->isContainer(obj->type()))
        {
            retVal.append(static_cast<HContainer*>(obj));
        }
    }

    return retVal;
}

HContainers HAbstractCdsDataSource::containers() const
{
    QList<HContainer*> retVal;

    QHash<QString, HObject*>::iterator it = h_ptr->m_objectsById.begin();
    for (; it != h_ptr->m_objectsById.end(); ++it)
    {
        if (it.value()->isContainer())
        {
            retVal.append(static_cast<HContainer*>(it.value()));
        }
    }

    return retVal;
}

QIODevice* HAbstractCdsDataSource::loadItemData(const QString& /*itemId*/)
{
    return 0;
}

HObjects HAbstractCdsDataSource::add(const HObjects& objects, AddFlag addFlag)
{
    HObjects notAdded;
    foreach(HObject* obj, objects)
    {
        if (!add(obj, addFlag))
        {
            notAdded.append(obj);
        }
    }
    return notAdded;
}

bool HAbstractCdsDataSource::add(HObject* object, AddFlag addFlag)
{
    return h_ptr->add(object, addFlag);
}

bool HAbstractCdsDataSource::remove(const QString& id)
{
    if (h_ptr->m_objectsById.contains(id))
    {
        delete h_ptr->m_objectsById.value(id);
        h_ptr->m_objectsById.remove(id);
        return true;
    }
    return false;
}

qint32 HAbstractCdsDataSource::remove(const HObjects& objects)
{
    qint32 removed = 0;
    foreach(HObject* obj, objects)
    {
        QString id = obj->id();
        if (h_ptr->m_objectsById.contains(id))
        {
            delete h_ptr->m_objectsById.value(id);
            h_ptr->m_objectsById.remove(id);
            ++removed;
        }
    }
    return removed;
}

qint32 HAbstractCdsDataSource::remove(const QSet<QString>& ids)
{
    qint32 removed = 0;
    foreach(const QString& id, ids)
    {
        if (h_ptr->m_objectsById.contains(id))
        {
            delete h_ptr->m_objectsById.value(id);
            h_ptr->m_objectsById.remove(id);
            ++removed;
        }
    }
    return removed;
}

void HAbstractCdsDataSource::clear()
{
    qDeleteAll(h_ptr->m_objectsById);
    h_ptr->m_objectsById.clear();
    qDeleteAll(h_ptr->m_objectIdsByParentId);
    h_ptr->m_objectIdsByParentId.clear();
}

}
}
}
