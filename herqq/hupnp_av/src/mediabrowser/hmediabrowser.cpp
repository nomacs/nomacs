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

#include "hmediabrowser.h"
#include "hmediabrowser_p.h"
#include "hcds_lastchange_info.h"

#include "../cds_model/cds_objects/hitem.h"
#include "../cds_model/cds_objects/hcontainer.h"
#include "../cds_model/datasource/hcds_datasource.h"
#include "../cds_model/model_mgmt/hcds_dlite_serializer.h"

#include "../contentdirectory/hsearchresult.h"
#include "../contentdirectory/hcontentdirectory_adapter.h"
#include "../contentdirectory/hcontentdirectory_info.h"

#include <HUpnpCore/private/hlogger_p.h>
#include <HUpnpCore/private/hmisc_utils_p.h>

#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>

/*!
 * \defgroup hupnp_av_cds_browsing Browsing
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
 * HBrowseParamsPrivate
 ******************************************************************************/
class HBrowseParamsPrivate
{
public:

    QString m_objectId;
    HBrowseParams::BrowseType m_loadType;
    QSet<QString> m_filter;

    HBrowseParamsPrivate() :
        m_objectId(), m_loadType(HBrowseParams::SingleItem), m_filter()
    {
    }
};

/*******************************************************************************
 * HBrowseParams
 ******************************************************************************/
HBrowseParams::HBrowseParams() :
    h_ptr(new HBrowseParamsPrivate())
{
    h_ptr->m_loadType = SingleItem;
}

HBrowseParams::HBrowseParams(BrowseType loadType) :
    h_ptr(new HBrowseParamsPrivate())
{
    h_ptr->m_loadType = loadType;
}

HBrowseParams::HBrowseParams(const QString& objectId, BrowseType loadType) :
    h_ptr(new HBrowseParamsPrivate())
{
    h_ptr->m_objectId = objectId.trimmed();
    h_ptr->m_loadType = loadType;
}

HBrowseParams::~HBrowseParams()
{
    delete h_ptr;
}

HBrowseParams::HBrowseParams(const HBrowseParams& other) :
    h_ptr(new HBrowseParamsPrivate(*other.h_ptr))
{
    Q_ASSERT(this != &other);
}

HBrowseParams& HBrowseParams::operator=(const HBrowseParams& other)
{
    Q_ASSERT(this != &other);
    delete h_ptr;
    h_ptr = new HBrowseParamsPrivate(*other.h_ptr);
    return *this;
}

void HBrowseParams::setObjectId(const QString& id)
{
    h_ptr->m_objectId = id.trimmed();
}

void HBrowseParams::setBrowseType(BrowseType type)
{
    h_ptr->m_loadType = type;
}

void HBrowseParams::setFilter(const QSet<QString>& filter)
{
    h_ptr->m_filter = filter;
}

bool HBrowseParams::isValid() const
{
    return !h_ptr->m_objectId.isEmpty();
}

QString HBrowseParams::objectId() const
{
    return h_ptr->m_objectId;
}

HBrowseParams::BrowseType HBrowseParams::browseType() const
{
    return h_ptr->m_loadType;
}

QSet<QString> HBrowseParams::filter() const
{
    return h_ptr->m_filter;
}

/*******************************************************************************
 * HMediaBrowserPrivate
 ******************************************************************************/
HMediaBrowserPrivate::HMediaBrowserPrivate(
    HMediaBrowser* parent) :
        QObject(parent),
            m_contentDirectory(0),
            m_hasOwnershipOfCds(true),
            m_autoUpdateEnabled(false),
            m_dataSource(new HCdsDataSource()),
            m_currentUserOp(0),
            m_currentAutoOp(0),
            m_lastErrorCode(0),
            m_lastErrorDescription(),
            q_ptr(0)
{
    Q_ASSERT(parent);
}

HMediaBrowserPrivate::~HMediaBrowserPrivate()
{
    delete m_dataSource;
    if (m_hasOwnershipOfCds)
    {
        delete m_contentDirectory;
    }
    qDeleteAll(m_autoOpQueue);
}

void HMediaBrowserPrivate::checkNextAutoOp()
{
    if (!m_autoOpQueue.isEmpty())
    {
        m_currentAutoOp.reset(m_autoOpQueue.dequeue());
        browse(m_currentAutoOp.data());
    }
}

void HMediaBrowserPrivate::browseCompleted(
    HContentDirectoryAdapter*, const HClientAdapterOp<HSearchResult>& op)
{
    HBrowseOp* browseOp = 0;
    if (m_currentAutoOp && m_currentAutoOp->m_currentOp->id() == op.id())
    {
        browseOp = m_currentAutoOp.data();
    }
    else if (m_currentUserOp && m_currentUserOp->m_currentOp->id() != op.id())
    {
        return;
    }
    else if (op.returnValue() != UpnpSuccess)
    {
        browseOp = m_currentUserOp.data();
        browseFailed(browseOp, op.errorDescription(), op.returnValue());
        return;
    }
    else
    {
        browseOp = m_currentUserOp.data();
    }

    HSearchResult result = op.value();

    HObjects objects;
    HCdsDidlLiteSerializer serializer;
    if (!serializer.serializeFromXml(result.result(), &objects))
    {
        browseFailed(browseOp, serializer.lastErrorDescription());
        return;
    }
    else if (objects.size() > 0)
    {
        browseOp->m_loadedObjects.append(objects);
        m_dataSource->add(objects);

        QSet<QString> ids;
        foreach(HObject* object, objects)
        {
            ids.insert(object->id());
        }
        if (browseOp == m_currentUserOp.data())
        {
            emit owner()->objectsBrowsed(owner(), ids);
        }
    }

    HBrowseParams::BrowseType loadType = browseOp->m_loadParams.browseType();
    switch(loadType)
    {
    case HBrowseParams::SingleItem:
    case HBrowseParams::DirectChildren:
        browseComplete(browseOp);
        break;

    case HBrowseParams::ObjectAndDirectChildren:
        if (browseOp->m_indexUnderProcessing >= 0)
        {
            browseComplete(browseOp);
        }
        else
        {
            browseOp->m_indexUnderProcessing = 0;
            browseOp->m_loadParams.setObjectId(objects.at(0)->id());
            browse(browseOp);
        }
        break;

    case HBrowseParams::ObjectAndChildrenRecursively:
    {
        qint32 i = ++browseOp->m_indexUnderProcessing;
        for(; i < browseOp->m_loadedObjects.size(); ++i)
        {
            HObject* object = browseOp->m_loadedObjects.at(i);
            if (object->isContainer())
            {
                browseOp->m_loadParams.setObjectId(object->id());
                browseOp->m_indexUnderProcessing = i;
                browse(browseOp);
                return;
            }
        }

        if (i == browseOp->m_loadedObjects.size())
        {
            browseComplete(browseOp);
        }
    }
    }
}

void HMediaBrowserPrivate::lastChangeReceived(
    HContentDirectoryAdapter*, const QString& data)
{
    HLOG(H_AT, H_FUN);

    QXmlStreamReader reader(data.trimmed());
    //addNamespaces(reader);

    if (reader.readNextStartElement())
    {
        if (reader.name().compare("StateEvent", Qt::CaseInsensitive) != 0)
        {
            return;
        }
    }
    else
    {
        return;
    }

    HCdsLastChangeInfos infos;
    while(!reader.atEnd() && reader.readNextStartElement())
    {
        QStringRef name = reader.name();

        QXmlStreamAttributes attrs = reader.attributes();
        QString objId = attrs.value("objID").toString();
        QString updateId = attrs.value("updateID").toString();
        QString stUpdated = attrs.value("stUpdate").toString();

        HCdsLastChangeInfo info;
        if (name == "objAdd")
        {
            QString parentId = attrs.value("objParentID").toString();
            QString objClass = attrs.value("objClass").toString();

            info = HCdsLastChangeInfo(objId, HCdsLastChangeInfo::ObjectAdded);
            info.setParentId(parentId);
            info.setObjectClass(objClass);
        }
        else if (name == "objMod")
        {
            info = HCdsLastChangeInfo(objId, HCdsLastChangeInfo::ObjectModified);
        }
        else if (name == "objDel")
        {
            info = HCdsLastChangeInfo(objId, HCdsLastChangeInfo::ObjectDeleted);
        }
        else
        {
            HLOG_WARN(QString("Encountered unknown XML element: [%1]").arg(name.toString()));
        }

        if (info.isValid())
        {
            bool ok = false;
            info.setUpdateId(updateId.toUInt());
            bool stu = toBool(stUpdated, &ok);
            if (ok) { info.setStUpdate(stu); }
            infos.append(info);
        }
    }

    if (!infos.isEmpty())
    {
        emit q_ptr->receivedLastChange(infos);

        if (m_autoUpdateEnabled)
        {
            update(infos);
        }
    }
}

void HMediaBrowserPrivate::autoBrowse(const HBrowseParams& params)
{
    HBrowseOp* newOp = new HBrowseOp(params);
    if (m_currentAutoOp || m_currentUserOp)
    {
        // Don't reset currently running auto update before its complete, and
        // if user has started a browse operation wait until it's complete in
        // order to get the user request to complete as fast as possible.
        m_autoOpQueue.enqueue(newOp);
    }
    else
    {
        m_currentAutoOp.reset(newOp);
        browse(m_currentAutoOp.data());
    }
}

void HMediaBrowserPrivate::update(const HCdsLastChangeInfos& data)
{
    foreach(const HCdsLastChangeInfo& info, data)
    {
        // TODO this properly
        switch(info.eventType())
        {
        case HCdsLastChangeInfo::ObjectDeleted:
             break;
        case HCdsLastChangeInfo::ObjectAdded:
        case HCdsLastChangeInfo::ObjectModified:
             {
                 HBrowseParams params(info.objectId(), HBrowseParams::SingleItem);
                 autoBrowse(params);
             }
             break;
        default:
             break;
        }
    }
}

bool HMediaBrowserPrivate::browse(HBrowseOp* browseOp)
{
    if (!m_contentDirectory)
    {
        return false;
    }

    HClientAdapterOp<HSearchResult> op =
        m_contentDirectory->browse(
            browseOp->m_loadParams.objectId(),
            browseOp->m_indexUnderProcessing < 0 ?
                HContentDirectoryInfo::BrowseMetadata :
                HContentDirectoryInfo::BrowseDirectChildren,
            browseOp->m_loadParams.filter(),
            0,
            0,
            QStringList());

    browseOp->m_currentOp.reset(new HClientAdapterOp<HSearchResult>(op));

    return true;
}

void HMediaBrowserPrivate::reset()
{
    m_dataSource->clear();
    if (m_hasOwnershipOfCds)
    {
        delete m_contentDirectory;
    }
    else
    {
        bool ok = disconnect(m_contentDirectory, 0, this, 0);
        Q_ASSERT(ok); Q_UNUSED(ok)
    }
    m_currentUserOp.reset(0);
    m_currentAutoOp.reset(0);
    qDeleteAll(m_autoOpQueue);
}

void HMediaBrowserPrivate::browseComplete(HBrowseOp* op)
{
    if (op == m_currentUserOp.data())
    {
        m_currentUserOp.reset(0);
        emit owner()->browseComplete(owner());
    }
    else
    {
        m_currentAutoOp.reset(0);
        checkNextAutoOp();
    }
}

void HMediaBrowserPrivate::browseFailed(
    HBrowseOp* op, const QString& errorDescription, qint32 errorCode)
{
    if (op == m_currentUserOp.data())
    {
        m_lastErrorDescription = errorDescription;
        m_lastErrorCode = errorCode;

        m_currentUserOp.reset(0);
        emit owner()->browseFailed(owner());
    }
    else
    {
        m_currentAutoOp.reset(0);
        checkNextAutoOp();
    }
}

/*******************************************************************************
 * HMediaBrowser
 ******************************************************************************/
HMediaBrowser::HMediaBrowser(QObject* parent) :
    QObject(parent),
        h_ptr(new HMediaBrowserPrivate(this))
{
    bool ok = h_ptr->m_dataSource->init();
    Q_ASSERT(ok); Q_UNUSED(ok)
    h_ptr->q_ptr = this;
}

HMediaBrowser::~HMediaBrowser()
{
    delete h_ptr;
}

bool HMediaBrowser::reset(HClientService* cds)
{
    Q_ASSERT_X(cds, "", "Null HClientService");

    h_ptr->reset();

    h_ptr->m_contentDirectory = new HContentDirectoryAdapter();
    if (!h_ptr->m_contentDirectory->setService(cds))
    {
        delete h_ptr->m_contentDirectory;
        h_ptr->m_contentDirectory = 0;
        return false;
    }

    bool ok = connect(
        h_ptr->m_contentDirectory,
        SIGNAL(browseCompleted(Herqq::Upnp::Av::HContentDirectoryAdapter*,
                               Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HSearchResult>)),
        h_ptr,
        SLOT(browseCompleted(Herqq::Upnp::Av::HContentDirectoryAdapter*,
                             Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HSearchResult>)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        h_ptr->m_contentDirectory,
        SIGNAL(lastChangeReceived(Herqq::Upnp::Av::HContentDirectoryAdapter*, QString)),
        h_ptr,
        SLOT(lastChangeReceived(Herqq::Upnp::Av::HContentDirectoryAdapter*, QString)));
    Q_ASSERT(ok);

    return true;
}

bool HMediaBrowser::reset(HContentDirectoryAdapter* cds, bool takeOwnership)
{
    Q_ASSERT_X(cds, "", "Null HContentDirectoryAdapter");

    h_ptr->reset();
    h_ptr->m_contentDirectory = cds;
    h_ptr->m_hasOwnershipOfCds = takeOwnership;

    bool ok = connect(
        h_ptr->m_contentDirectory,
        SIGNAL(browseCompleted(Herqq::Upnp::Av::HContentDirectoryAdapter*,
                               Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HSearchResult>)),
        h_ptr,
        SLOT(browseCompleted(Herqq::Upnp::Av::HContentDirectoryAdapter*,
                             Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HSearchResult>)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        h_ptr->m_contentDirectory,
        SIGNAL(lastChangeReceived(Herqq::Upnp::Av::HContentDirectoryAdapter*, QString)),
        h_ptr,
        SLOT(lastChangeReceived(Herqq::Upnp::Av::HContentDirectoryAdapter*, QString)));
    Q_ASSERT(ok);

    return true;
}

bool HMediaBrowser::browse(const HBrowseParams& loadParams)
{
    if (!isReady())
    {
        h_ptr->m_lastErrorCode = UpnpUndefinedFailure;
        h_ptr->m_lastErrorDescription = "Media Browse is not ready";
        return false;
    }
    else if (!loadParams.isValid())
    {
        h_ptr->m_lastErrorCode = UpnpInvalidArgs;
        h_ptr->m_lastErrorDescription = "Invalid arguments";
        return false;
    }

    HBrowseParams params = loadParams;
    if (!params.filter().contains("res"))
    {
        params.setFilter(QSet<QString>(params.filter()) << "res");
    }
    h_ptr->m_currentUserOp.reset(new HBrowseOp(params));
    return h_ptr->browse(h_ptr->m_currentUserOp.data());
}

bool HMediaBrowser::browseAll()
{
    HBrowseParams loadParams("0", HBrowseParams::ObjectAndChildrenRecursively);
    return browse(loadParams);
}

bool HMediaBrowser::browseRoot()
{
    HBrowseParams loadParams("0", HBrowseParams::ObjectAndDirectChildren);
    return browse(loadParams);
}

HCdsDataSource* HMediaBrowser::dataSource() const
{
    return h_ptr->m_dataSource;
}

HContentDirectoryAdapter* HMediaBrowser::contentDirectory() const
{
    return h_ptr->m_contentDirectory;
}

bool HMediaBrowser::isReady() const
{
    return h_ptr->m_contentDirectory && !isActive();
}

bool HMediaBrowser::isActive() const
{
    return h_ptr->m_currentUserOp.data();
}

qint32 HMediaBrowser::lastErrorCode() const
{
    return h_ptr->m_lastErrorCode;
}

QString HMediaBrowser::lastErrorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

void HMediaBrowser::cancel()
{
    if (h_ptr->m_currentUserOp.data())
    {
        h_ptr->m_currentUserOp->m_currentOp->abort();
        h_ptr->m_currentUserOp.reset(0);
    }
}

bool HMediaBrowser::isAutoUpdateEnabled()
{
    return h_ptr->m_autoUpdateEnabled;
}

void HMediaBrowser::setAutoUpdate(bool enable)
{
    h_ptr->m_autoUpdateEnabled = enable;
}

}
}
}
