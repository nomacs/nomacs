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

#include "hcontentdirectory_service.h"
#include "hcontentdirectory_service_p.h"

#include "hsearchresult.h"
#include "htransferprogressinfo.h"

#include "../cds_model/hsortinfo.h"
#include "../cds_model/model_mgmt/hcdsproperty.h"
#include "../cds_model/model_mgmt/hcdsproperty_db.h"
#include "../cds_model/model_mgmt/hcds_dlite_serializer.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamWriter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HContentDirectoryServicePrivate
 ******************************************************************************/
HContentDirectoryServicePrivate::HContentDirectoryServicePrivate() :
    m_dataSource(0), m_lastEventSent(false), m_timer(), m_modificationEvents()
{
}

HContentDirectoryServicePrivate::~HContentDirectoryServicePrivate()
{
    qDeleteAll(m_modificationEvents);
}

namespace
{
class Sorter
{
private:

    QList<HSortInfo> m_sortInfoObjects;

public:

    Sorter(){}

    Sorter(const QList<HSortInfo>& infoObjects) :
        m_sortInfoObjects(infoObjects)
    {
    }

    bool operator()(HObject* obj1, HObject* obj2) const
    {
        Q_ASSERT(obj1);
        Q_ASSERT(obj2);

        for(qint32 i = 0; i < m_sortInfoObjects.size(); ++i)
        {
            qint32 rc = 0;
            HSortInfo si = m_sortInfoObjects[i];
            QString property = si.property();

            QVariant value1, value2;
            if (obj1->getCdsProperty(property, &value1))
            {
                if (obj2->getCdsProperty(property, &value2))
                {
                    HCdsProperty prop = HCdsPropertyDb::instance().property(property);
                    if (prop.isValid())
                    {
                        if (!prop.handler().comparer()(value1, value2, &rc))
                        {
                            continue;
                        }
                    }
                }
            }

            if (rc < 0)
            {
                return si.sortModifier().ascending();
            }
            else if (rc > 0)
            {
                return !si.sortModifier().ascending();
            }
        }

        return true;
    }
};
}

qint32 HContentDirectoryServicePrivate::sort(
    const QStringList& sortCriteria, QList<HObject*>& objects)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HContentDirectoryService);

    QStringList sortExtensions;
    q->getSortExtensionCapabilities(&sortExtensions);

    QStringList sortCapabilities;
    q->getSortCapabilities(&sortCapabilities);

    QList<HSortInfo> sortInfoObjects;
    for (qint32 i = 0; i < sortCriteria.size(); ++i)
    {
        QString tmp = sortCriteria[i].trimmed();

        if (tmp.isEmpty())
        {
            continue;
        }

        qint32 j = 0;
        QChar ch = 0;
        QString sortExtension;
        for(; j < tmp.size(); ++j)
        {
            ch = tmp[j];
            if (ch == '+' || ch == '-')
            {
                break;
            }
            sortExtension.append(ch);
        }

        if (ch != '+' && ch != '-')
        {
            return HContentDirectoryInfo::InvalidSortCriteria;
        }

        bool ascending = ch == '+';

        if (!sortExtension.isEmpty() && !sortExtension.contains(
            sortExtension, Qt::CaseInsensitive))
        {
            return HContentDirectoryInfo::InvalidSortCriteria;
        }

        QString sortProperty;
        for(++j; j < tmp.size(); ++j)
        {
            ch = tmp[j];
            sortProperty.append(ch);
        }

        if (!sortCapabilities.contains(sortProperty, Qt::CaseInsensitive) &&
            !sortCapabilities.contains("*"))
        {
            return HContentDirectoryInfo::InvalidSortCriteria;
        }

        HSortModifier modifier(
            QString("%1%2").arg(
                sortExtension, ascending ? QString("+") : QString("-")));

        HSortInfo so(sortProperty, modifier);
        sortInfoObjects.append(so);
    }

    qStableSort(objects.begin(), objects.end(), Sorter(sortInfoObjects));

    return 0;
}

qint32 HContentDirectoryServicePrivate::browseDirectChildren(
    const QString& containerId, const QSet<QString>& filter,
    const QStringList& sortCriteria, quint32 startingIndex,
    quint32 requestedCount, HSearchResult* result)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HContentDirectoryService);

    HContainer* container = m_dataSource->findContainer(containerId);
    if (!container)
    {
        HLOG_WARN(QString(
            "The specified object ID [%1] does not map to a container").arg(
                containerId));

        return HContentDirectoryInfo::InvalidObjectId;
    }

    HLOG_DBG(QString(
        "Browsing container [id: %1, startingIndex: %2, "
        "requestedCount: %3, filter: %4, sortCriteria: %5]").arg(
            containerId,
            QString::number(startingIndex),
            QString::number(requestedCount),
            QStringList(filter.toList()).join(","),
            sortCriteria.join(",")));

    QSet<QString> childIDs = container->childIds();
    quint32 childCount = static_cast<quint32>(childIDs.size());

    if (startingIndex > childCount)
    {
        return UpnpInvalidArgs;
    }

    HObjects objects = m_dataSource->findObjects(childIDs);
    Q_ASSERT(objects.size() == childIDs.size());

    if (!sortCriteria.isEmpty())
    {
        qint32 rc = sort(sortCriteria, objects);
        if (rc != 0)
        {
            return rc;
        }
    }

    quint32 numberReturned = requestedCount > 0 ?
        qMin(requestedCount, childCount - startingIndex) :
        childCount - startingIndex;

    objects = objects.mid(startingIndex, requestedCount ? requestedCount : -1);

    HCdsDidlLiteSerializer ser;
    QString dliteDoc = ser.serializeToXml(objects, filter);

    HSearchResult retVal(
        dliteDoc, numberReturned, childCount,
        q->stateVariables().value("A_ARG_TYPE_UpdateID")->value().toUInt());

    *result = retVal;

    return UpnpSuccess;
}

qint32 HContentDirectoryServicePrivate::browseMetadata(
    const QString& objectId, const QSet<QString>& filter, quint32 startingIndex,
    HSearchResult* result)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HContentDirectoryService);

    if (startingIndex)
    {
        HLOG_WARN(QString(
            "The starting index was specified as [%1], although it "
            "should be zero when browsing meta data").arg(
                QString::number(startingIndex)));

        return UpnpInvalidArgs;
    }

    HObject* object = m_dataSource->findObject(objectId);
    if (!object)
    {
        HLOG_WARN(QString(
            "No object was found with the specified object ID [%1]").arg(
                objectId));

        return HContentDirectoryInfo::InvalidObjectId;
    }

    HCdsDidlLiteSerializer serializer;
    QString dliteDoc = serializer.serializeToXml(
        *object, filter, HCdsDidlLiteSerializer::Document);

    HSearchResult retVal(
        dliteDoc, 1, 1,
        q->stateVariables().value("A_ARG_TYPE_UpdateID")->value().toUInt());

    *result = retVal;

    return UpnpSuccess;
}

void HContentDirectoryServicePrivate::enableChangeTracking()
{
    H_Q(HContentDirectoryService);

    bool ok = QObject::connect(
        m_dataSource, SIGNAL(objectModified(Herqq::Upnp::Av::HObject*, Herqq::Upnp::Av::HObjectEventInfo)),
        q, SLOT(objectModified(Herqq::Upnp::Av::HObject*, Herqq::Upnp::Av::HObjectEventInfo)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = QObject::connect(
        m_dataSource, SIGNAL(containerModified(Herqq::Upnp::Av::HContainer*, Herqq::Upnp::Av::HContainerEventInfo)),
        q, SLOT(containerModified(Herqq::Upnp::Av::HContainer*, Herqq::Upnp::Av::HContainerEventInfo)));
    Q_ASSERT(ok);

    ok = QObject::connect(
        m_dataSource, SIGNAL(independentObjectAdded(Herqq::Upnp::Av::HObject*)),
        q, SLOT(independentObjectAdded(Herqq::Upnp::Av::HObject*)));
    Q_ASSERT(ok);

    foreach(HObject* object, m_dataSource->objects())
    {
        object->setTrackChangesOption(true);
    }

    m_timer.start();
}

QString HContentDirectoryServicePrivate::generateLastChange()
{
    QString retVal;

    QXmlStreamWriter writer(&retVal);

    writer.setCodec("UTF-8");
    writer.writeStartDocument();
    writer.writeStartElement("StateEvent");
    writer.writeDefaultNamespace("urn:schemas-upnp-org:av:cds-event");
    writer.writeAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
    writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");

    writer.writeAttribute("xsi:schemaLocation",
        "urn:schemas-upnp-org:av:cds-event" \
        "http://www.upnp.org/schemas/av/cds-events.xsd");

    foreach(const HModificationEvent* event, m_modificationEvents)
    {
        if (event->type() == HModificationEvent::ContainerModification)
        {
            HContainerEventInfo cEvent = event->containerEvent();

            switch(cEvent.type())
            {
            case HContainerEventInfo::ChildAdded:
                if (event->source())
                {
                    HObject* newObj = m_dataSource->findObject(cEvent.childId());
                    if (newObj)
                    {
                        writer.writeStartElement("objAdd");
                        writer.writeAttribute("objParentID", event->source()->id());
                        writer.writeAttribute("objClass", newObj->clazz());
                    }
                }
                break;
            case HContainerEventInfo::ChildRemoved:
                writer.writeStartElement("objDel");
                break;
            case HContainerEventInfo::ChildModified:
                writer.writeStartElement("objMod");
                break;
            default:
                Q_ASSERT(false);
                break;
            }

            writer.writeAttribute("objID", cEvent.childId());
            writer.writeAttribute("updateID", QString::number(cEvent.updateId()));
            writer.writeAttribute("stUpdate", "0");
            writer.writeEndElement();
        }
        else
        {
            if (event->source())
            {
                HObjectEventInfo oEvent = event->objectEvent();
                writer.writeStartElement("objMod");
                writer.writeAttribute("objID", event->source()->id());
                writer.writeAttribute("updateID", QString::number(oEvent.updateId()));
                writer.writeAttribute("stUpdate", "0");
                writer.writeEndElement();
            }
        }
    }
    writer.writeEndElement();

    return retVal;
}

/*******************************************************************************
 * HContentDirectoryService
 ******************************************************************************/
HContentDirectoryService::HContentDirectoryService(
    HContentDirectoryServicePrivate& dd) :
        HAbstractContentDirectoryService(dd)
{
    H_D(HContentDirectoryService);
    h->m_timer.setInterval(200);
    bool ok = connect(&h->m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    Q_ASSERT(ok); Q_UNUSED(ok)
}

HContentDirectoryService::HContentDirectoryService(HAbstractCdsDataSource* dataSource) :
    HAbstractContentDirectoryService(
        *new HContentDirectoryServicePrivate())
{
    H_D(HContentDirectoryService);
    Q_ASSERT_X(dataSource, "", "Valid HCdsDataSource has to be provided");
    h->m_dataSource = dataSource;
    h->m_timer.setInterval(200);
    bool ok = connect(&h->m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HContentDirectoryService::timeout()
{
    H_D(HContentDirectoryService);
    if (!h->m_lastEventSent && h->m_modificationEvents.size())
    {
        QString lastChangeData = h->generateLastChange();
        bool ok = setValue("LastChange", lastChangeData);
        h->m_lastEventSent = true;
        Q_ASSERT(ok); Q_UNUSED(ok)
    }
}

void HContentDirectoryService::objectModified(
    HObject* source, const HObjectEventInfo& eventInfo)
{
    H_D(HContentDirectoryService);
    if (h->m_lastEventSent)
    {
        h->m_modificationEvents.clear();
        h->m_lastEventSent = false;
    }

    HObjectEventInfo einfo(eventInfo);

    quint32 sysUpdateId;
    qint32 retVal = getSystemUpdateId(&sysUpdateId);
    Q_ASSERT(retVal == UpnpSuccess); Q_UNUSED(retVal)

    einfo.setUpdateId(sysUpdateId);

    source->setObjectUpdateId(sysUpdateId);

    h->m_modificationEvents.append(new HModificationEvent(source, einfo));
}

void HContentDirectoryService::containerModified(
    HContainer* source, const HContainerEventInfo& eventInfo)
{
    H_D(HContentDirectoryService);

    if (eventInfo.type() == HContainerEventInfo::ChildAdded)
    {
        HItem* item = h->m_dataSource->findItem(eventInfo.childId());
        if (item)
        {
            if (stateVariables().contains("LastChange"))
            {
                item->setTrackChangesOption(true);
            }
        }
    }

    if (h->m_lastEventSent)
    {
        h->m_modificationEvents.clear();
        h->m_lastEventSent = false;
    }

    HContainerEventInfo einfo(eventInfo);

    quint32 sysUpdateId;
    qint32 retVal = getSystemUpdateId(&sysUpdateId);
    Q_ASSERT(retVal == UpnpSuccess); Q_UNUSED(retVal)

    einfo.setUpdateId(sysUpdateId);

    source->setContainerUpdateId(sysUpdateId);

    h->m_modificationEvents.append(new HModificationEvent(source, einfo));
}

void HContentDirectoryService::independentObjectAdded(HObject* source)
{
    Q_UNUSED(source)
    //H_D(HContentDirectoryService);
}

bool HContentDirectoryService::init()
{
    H_D(HContentDirectoryService);

    if (stateVariables().contains("LastChange"))
    {
        h->enableChangeTracking();
    }

    return true;
}

HContentDirectoryService::~HContentDirectoryService()
{
}

qint32 HContentDirectoryService::getSearchCapabilities(QStringList* oarg) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT_X(oarg, H_AT, "Out argument(s) cannot be null");

    oarg->clear();

//    *oarg = QString(
//        "@id,@parentID,upnp:class,upnp:objectUpdateID,"
//        "upnp:containerUpdateID,dc:title,dc:creator,dc:date,res@size").split(',');
// TODO once search is included

    return UpnpSuccess;
}

qint32 HContentDirectoryService::getSortCapabilities(QStringList* oarg) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT_X(oarg, H_AT, "Out argument(s) cannot be null");

    *oarg = QString("dc:title,dc:creator,dc:date,res@size,upnp:class,upnp:album,upnp:originalTrackNumber").split(',');
    return UpnpSuccess;
}

qint32 HContentDirectoryService::getSortExtensionCapabilities(
    QStringList* oarg) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT_X(oarg, H_AT, "Out argument(s) cannot be null");

    if (!actions().value("GetSortExtensionCapabilities"))
    {
        return UpnpOptionalActionNotImplemented;
    }

    *oarg = QString("+,-,TIME+,TIME-").split(',');
    return UpnpSuccess;
}

qint32 HContentDirectoryService::getFeatureList(QString* oarg) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT_X(oarg, H_AT, "Out argument(s) cannot be null");

    *oarg = QString();
    return UpnpSuccess;
}

qint32 HContentDirectoryService::getSystemUpdateId(quint32* oarg) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT_X(oarg, H_AT, "Out argument(s) cannot be null");

    *oarg = stateVariables().value("SystemUpdateID")->value().toUInt();
    return UpnpSuccess;
}

qint32 HContentDirectoryService::getServiceResetToken(QString* oarg) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT_X(oarg, H_AT, "Out argument(s) cannot be null");

    *oarg = stateVariables().value("ServiceResetToken")->value().toString();
    return UpnpSuccess;
}

qint32 HContentDirectoryService::browse(
    const QString& objectId, HContentDirectoryInfo::BrowseFlag browseFlag,
    const QSet<QString>& filter, quint32 startingIndex, quint32 requestedCount,
    const QStringList& sortCriteria, HSearchResult* result)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT_X(result, H_AT, "Out argument(s) cannot be null");

    H_D(HContentDirectoryService);

    if (!result)
    {
        Q_ASSERT(false);
        return UpnpInvalidArgs;
    }

    HLOG_INFO(QString("processing browse request to object id %1").arg(objectId));

    qint32 retVal = 0;
    switch(browseFlag)
    {
    case HContentDirectoryInfo::BrowseDirectChildren:

        retVal = h->browseDirectChildren(
            objectId, filter, sortCriteria, startingIndex, requestedCount,
            result);

        break;

    case HContentDirectoryInfo::BrowseMetadata:

        retVal = h->browseMetadata(objectId, filter, startingIndex, result);
        break;

    default:
        HLOG_WARN(QString("received invalid browse flag"));
        retVal = UpnpInvalidArgs;
        break;
    }

    if (retVal != UpnpSuccess)
    {
        return retVal;
    }

    HLOG_INFO(QString(
        "Browse handled successfully: returned: [%1] matching objects of [%2] "
        "possible totals.").arg(
            QString::number(result->numberReturned()),
            QString::number(result->totalMatches())));

    return retVal;
}

qint32 HContentDirectoryService::search(
    const QString& containerId, const QString& /*searchCriteria*/,
    const QSet<QString>& /*filter*/, quint32 /*startingIndex*/,
    quint32 /*requestedCount*/, const QStringList& /*sortCriteria*/,
    HSearchResult* result)
{
    H_D(HContentDirectoryService);
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!result)
    {
        Q_ASSERT(false);
        return UpnpInvalidArgs;
    }

    if (!actions().value("Search"))
    {
        return UpnpOptionalActionNotImplemented;
    }

    HLOG_INFO(QString("attempting to locate container with id %1").arg(
        containerId));

    HContainer* container = qobject_cast<HContainer*>(
        h->m_dataSource->findObject(containerId));

    if (!container)
    {
        return HContentDirectoryInfo::InvalidObjectId;
    }

    return UpnpSuccess;
}

}
}
}
