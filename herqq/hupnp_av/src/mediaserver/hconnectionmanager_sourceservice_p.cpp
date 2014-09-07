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

#include "hconnectionmanager_sourceservice_p.h"

#include "../connectionmanager/hconnectioninfo.h"
#include "../connectionmanager/hconnectionmanager_service_p.h"

#include "../hav_global_p.h"
#include "../common/hresource.h"
#include "../common/hprotocolinfo.h"
#include "../cds_model/cds_objects/hitem.h"
#include "../cds_model/cds_objects/hcontainer.h"
#include "../cds_model/datasource/hcds_datasource.h"

#include <HUpnpCore/private/hlogger_p.h>
#include <HUpnpCore/private/hhttp_server_p.h>
#include <HUpnpCore/private/hhttp_header_p.h>
#include <HUpnpCore/private/hhttp_messagecreator_p.h>

#include <QtNetwork/QTcpSocket>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HHttpStreamer::HHttpStreamer(
    HMessagingInfo* mi, const QByteArray& header, QIODevice* data, QObject* parent) :
        QObject(parent),
            m_bufSize(1024*64), m_buf(new char[m_bufSize]), m_dataToSend(data),
            m_mi(mi), m_header(header), m_read(0), m_written(0)
{
    bool ok = connect(
        &m_mi->socket(), SIGNAL(bytesWritten(qint64)),
        this, SLOT(bytesWritten(qint64)));
    Q_ASSERT(ok); Q_UNUSED(ok)
}

HHttpStreamer::~HHttpStreamer()
{
    delete m_mi;
    delete m_dataToSend;
    delete m_buf;
}

void HHttpStreamer::bytesWritten(qint64 written)
{
    HLOG(H_AT, H_FUN);

    if (m_dataToSend->atEnd())
    {
        deleteLater();
        return;
    }
    else if (m_dataToSend->pos() > 0)
    {
        // We've past the HTTP header.
        m_written += written;
    }

    if (m_written >= m_read)
    {
        m_read = m_dataToSend->read(m_buf, m_bufSize);
        if (m_read <= 0)
        {
            HLOG_WARN(QString("Failed to read data from the data source: [%1]").arg(
                m_dataToSend->errorString()));

            deleteLater();
            return;
        }

        m_written = 0;
    }

    written = m_mi->socket().write(m_buf + m_written, m_read - m_written);
    if (written == -1)
    {
        HLOG_WARN(QString("Failed to send data: %1").arg(m_mi->socket().errorString()));
        deleteLater();
    }
}

void HHttpStreamer::send()
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(m_dataToSend->pos() == 0);

    qint64 wrote = m_mi->socket().write(m_header);
    if (wrote < m_header.size())
    {
        HLOG_WARN(QString(
            "Failed to send HTTP header to the destination: [%1]. "
            "Aborting data transfer.").arg(m_mi->socket().errorString()));

        deleteLater();
    }
}

/*******************************************************************************
 * HConnectionManagerHttpServer
 ******************************************************************************/
HConnectionManagerHttpServer::HConnectionManagerHttpServer(
    const QByteArray& loggingId, HConnectionManagerSourceService* owner) :
        HHttpServer(loggingId, owner), m_owner(owner)
{
    Q_ASSERT(owner);
}

HConnectionManagerHttpServer::~HConnectionManagerHttpServer()
{
}

void HConnectionManagerHttpServer::incomingUnknownGetRequest(
    HMessagingInfo* mi, const HHttpRequestHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QScopedPointer<QIODevice> dev(
        m_owner->m_dataSource->loadItemData(hdr.path().remove('/')));

    if (dev)
    {
        if (dev->isSequential())
        {
            // TODO send in chunks
            Q_ASSERT_X(false, "", "Currently sequential data sources are not supported");
        }
        else if (dev->size() < maxBytesToLoad())
        {
            QByteArray data = dev->readAll();
            mi->setKeepAlive(true);
            m_httpHandler->send(
                mi,
                HHttpMessageCreator::createResponse(
                    Ok, *mi, data, ContentType_Undefined)); // TODO content type
        }
        else
        {
            HHttpStreamer* streamer =
                new HHttpStreamer(
                    mi,
                    HHttpMessageCreator::createHeaderData(
                        Ok, *mi, dev->size(), ContentType_Undefined), // TODO content type
                    dev.take(),
                    this);

            streamer->send();
        }
    }
    else
    {
        mi->setKeepAlive(true);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(BadRequest, *mi));
    }
}

/*******************************************************************************
 * HConnectionManagerSourceService
 ******************************************************************************/
HConnectionManagerSourceService::HConnectionManagerSourceService(
    HAbstractCdsDataSource* dataSource) :
        HConnectionManagerService(),
            m_dataSource(0),
            m_httpServer(new HConnectionManagerHttpServer(
                h_ptr->m_loggingIdentifier, this))
{
    Q_ASSERT_X(dataSource, "", "Valid HCdsDataSource has to be provided");
    m_dataSource = dataSource;
}

HConnectionManagerSourceService::~HConnectionManagerSourceService()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    delete m_httpServer;
}

bool HConnectionManagerSourceService::finalizeInit(QString* /*errDescription*/)
{
    if (sourceProtocolInfo().isEmpty())
    {
        setSourceProtocolInfo(HProtocolInfo("http-get:*:*:*"));
    }

    if (connectionIds().isEmpty())
    {
        createDefaultConnection(sourceProtocolInfo().at(0));
    }

    bool ok = connect(
        m_dataSource, SIGNAL(objectModified(Herqq::Upnp::Av::HObject*, Herqq::Upnp::Av::HObjectEventInfo)),
        this, SLOT(objectModified(Herqq::Upnp::Av::HObject*, Herqq::Upnp::Av::HObjectEventInfo)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        m_dataSource, SIGNAL(containerModified(Herqq::Upnp::Av::HContainer*, Herqq::Upnp::Av::HContainerEventInfo)),
        this, SLOT(containerModified(Herqq::Upnp::Av::HContainer*, Herqq::Upnp::Av::HContainerEventInfo)));
    Q_ASSERT(ok);

    return true;
}

void HConnectionManagerSourceService::objectModified(
    HObject* source, const HObjectEventInfo& eventInfo)
{
    Q_UNUSED(source)
    Q_UNUSED(eventInfo)
}

void HConnectionManagerSourceService::containerModified(
    HContainer* source, const HContainerEventInfo& eventInfo)
{
    Q_UNUSED(source)
    if (eventInfo.type() == HContainerEventInfo::ChildAdded)
    {
        HItem* item = m_dataSource->findItem(eventInfo.childId());
        if (item)
        {
            addLocation(item);
        }
    }
}

void HConnectionManagerSourceService::addLocation(HItem* item)
{
    QList<QUrl> rootUrls = m_httpServer->rootUrls();
    Q_ASSERT(!rootUrls.isEmpty());

    HResources resources = item->resources();

    if (resources.size())
    {
        bool added = false;
        int urlsIndex = 0;
        for(int i = 0; i < resources.size(); ++i)
        {
            if (resources[i].location().isEmpty())
            {
                QString rootUrl =
                    rootUrls[urlsIndex++ % rootUrls.size()].
                        toString().append('/').append(item->id());

                resources[i].setLocation(rootUrl);

                HProtocolInfo pi = resources[i].protocolInfo();
                pi.setProtocol("http-get");

                resources[i].setProtocolInfo(pi);

                added = true;
            }
        }
        if (!added)
        {
            HProtocolInfo pi = resources[0].protocolInfo();
            pi.setProtocol("http-get");

            HResource res(
                rootUrls[0].toString().append('/').append(item->id()), pi);

            resources.append(res);
        }
    }
    else
    {
        foreach(const QUrl& rootUrl, rootUrls)
        {
            QUrl location(rootUrl.toString().append('/').append(item->id()));
            HResource resource(location, sourceProtocolInfo()[0]);
            resources.append(resource);
        }
    }

    item->setResources(resources);
}

bool HConnectionManagerSourceService::init()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!m_httpServer->init())
    {
        HLOG_WARN("Failed to initialize HTTP server");
        return false;
    }
    else if (m_httpServer->rootUrls().isEmpty())
    {
        return false;
    }

    HItems items = m_dataSource->items();
    foreach(HItem* item, items)
    {
        addLocation(item);
    }

    return true;
}

bool HConnectionManagerSourceService::isInitialized() const
{
    return m_httpServer->isInitialized();
}

}
}
}
