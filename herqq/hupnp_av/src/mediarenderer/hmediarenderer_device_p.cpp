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

#include "hmediarenderer_device_p.h"
#include "hrendererconnection_manager.h"

#include "../renderingcontrol/hchannel.h"
#include "../connectionmanager/hconnectionmanager_id.h"

#include <HUpnpCore/HStateVariablesSetupData>

#include <QtCore/QEvent>
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QXmlStreamWriter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMediaRendererDevice
 ******************************************************************************/
HMediaRendererDevice::HMediaRendererDevice(
    const HMediaRendererDeviceConfiguration& conf) :
        m_configuration(conf.clone()), m_timer(this), m_avtInstanceEvents(),
        m_rcsInstanceEvents()
{
    m_timer.setInterval(200);
    bool ok = connect(
        m_configuration->rendererConnectionManager(),
        SIGNAL(connectionRemoved(Herqq::Upnp::Av::HAbstractConnectionManagerService*,qint32)),
        this,
        SLOT(rendererConnectionRemoved(Herqq::Upnp::Av::HAbstractConnectionManagerService*,qint32)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    Q_ASSERT(ok); Q_UNUSED(ok)
}

HMediaRendererDevice::~HMediaRendererDevice()
{
    delete m_configuration;
    qDeleteAll(m_avtInstanceEvents);
    qDeleteAll(m_rcsInstanceEvents);
}

namespace
{
void clear(QList<HInstanceEvents*>* events)
{
    foreach(HInstanceEvents* ie, *events)
    {
        ie->m_changedProperties.clear();
    }
}

bool generateLastChange(
    const QList<HInstanceEvents*>& events, bool rcs, QString* xml)
{
    QXmlStreamWriter writer(xml);

    writer.setCodec("UTF-8");
    writer.writeStartDocument();

    writer.writeStartElement("Event");

    if (rcs)
    {
        writer.writeDefaultNamespace("urn:schemas-upnp-org:metadata-1-0/RCS/");
        writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        writer.writeAttribute("xsi:schemaLocation",
            "urn:schemas-upnp-org:metadata-1-0/RCS/ " \
            "http://www.upnp.org/schemas/av/rcs-event-v1.xsd");
    }
    else
    {
        writer.writeDefaultNamespace("urn:schemas-upnp-org:metadata-1-0/AVT/");
        writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        writer.writeAttribute("xsi:schemaLocation",
            "urn:schemas-upnp-org:metadata-1-0/AVT/ " \
            "http://www.upnp.org/schemas/av/avt-event-v2.xsd");
    }

    int count = 0;
    foreach(const HInstanceEvents* event, events)
    {
        if (event->m_changedProperties.size() > 0)
        {
            writer.writeStartElement("InstanceID");
            writer.writeAttribute("val", QString::number(event->m_instanceId));

            QHash<QString, QPair<QString, QString> >::const_iterator ci =
                event->m_changedProperties.constBegin();

            while(ci != event->m_changedProperties.constEnd())
            {
                writer.writeStartElement(ci.key());
                writer.writeAttribute("val", ci.value().first);
                if (!ci.value().second.isEmpty())
                {
                    writer.writeAttribute("channel", ci.value().second);
                }
                writer.writeEndElement();
                ++ci;
            }

            writer.writeEndElement();
            ++count;
        }
    }
    writer.writeEndElement();

    return count > 0;
}
}

void HMediaRendererDevice::timeout()
{
    m_timer.stop();

    QString lastChangeData;
    if (!m_avtInstanceEvents.isEmpty())
    {
        if (generateLastChange(m_avtInstanceEvents, false, &lastChangeData))
        {
            bool ok = avTransport()->setValue("LastChange", lastChangeData);
            Q_ASSERT(ok); Q_UNUSED(ok)
            clear(&m_avtInstanceEvents);
        }
    }
    if (!m_rcsInstanceEvents.isEmpty())
    {
        if (generateLastChange(m_rcsInstanceEvents, true, &lastChangeData))
        {
            bool ok = renderingControl()->setValue("LastChange", lastChangeData);
            Q_ASSERT(ok); Q_UNUSED(ok)
            clear(&m_rcsInstanceEvents);
        }
    }
    m_timer.start();
}

namespace
{
HInstanceEvents* getInstanceEvents(QList<HInstanceEvents*>* events, qint32 id)
{
    HInstanceEvents* retVal = 0;
    foreach(HInstanceEvents* event, *events)
    {
        if (event->m_instanceId == id)
        {
            retVal = event;
            break;
        }
    }
    return retVal;
}
}

void HMediaRendererDevice::propertyChanged(
    HRendererConnectionInfo* source, const HRendererConnectionEventInfo& eventInfo)
{
    HConnectionInfo info;
    qint32 retVal = connectionManager()->getCurrentConnectionInfo(
        source->connection()->connectionInfo()->connectionId(), &info);

    Q_ASSERT(retVal == UpnpSuccess); Q_UNUSED(retVal)

    HInstanceEvents* events = 0;
    if (HAvTransportInfo::stateVariablesSetupData().contains(eventInfo.propertyName()))
    {
        events = getInstanceEvents(&m_avtInstanceEvents, info.avTransportId());
        if (!events)
        {
            events = new HInstanceEvents(info.avTransportId());
            m_avtInstanceEvents.append(events);
        }
    }
    else
    {
        events = getInstanceEvents(&m_rcsInstanceEvents, info.rcsId());
        if (!events)
        {
            events = new HInstanceEvents(info.rcsId());
            m_rcsInstanceEvents.append(events);
        }
    }
    events->m_changedProperties.insert(
        eventInfo.propertyName(),
        qMakePair(eventInfo.newValue(), eventInfo.channel().toString()));
}

void HMediaRendererDevice::rendererConnectionRemoved(
    HAbstractConnectionManagerService*, qint32 cid)
{
    connectionManager()->removeConnection(cid);
}

bool HMediaRendererDevice::event(QEvent* e)
{
    if (e->type() == QEvent::ThreadChange)
    {
        Q_ASSERT_X(m_configuration->hasOwnershipOfRendererConnectionManager(),
                   "",
                   "Cannot move this instance to different thread, since this instance does "
                   "not have the ownership of the configured Renderer Connection Manager.");
        return true;
    }
    return false;
}

bool HMediaRendererDevice::finalizeInit(QString* errDescription)
{
    HRendererConnectionManager* rcm = m_configuration->rendererConnectionManager();
    Q_ASSERT_X(rcm->thread() == thread(), "",
               "Renderer Connection Manager has to live in the same thread with the Media Renderer");
    Q_UNUSED(rcm)

    HRendererConnectionManager* rcManager = m_configuration->rendererConnectionManager();
    if (!rcManager)
    {
        if (errDescription)
        {
            *errDescription = "Renderer Connection Manager was not set";
        }
        return false;
    }

    HConnectionManagerSinkService* cm =
        qobject_cast<HConnectionManagerSinkService*>(connectionManager());

    if (!cm || !cm->init(this))
    {
        if (errDescription)
        {
            *errDescription = "Failed to initialize ConnectionManager";
        }
        return false;
    }

    HTransportSinkService* tss =
        qobject_cast<HTransportSinkService*>(avTransport());

    if (!tss || !tss->init(this))
    {
        if (errDescription)
        {
            *errDescription = "Failed to initialize AV Transport";
        }
        return false;
    }

    HRenderingControlService* rcs =
        qobject_cast<HRenderingControlService*>(renderingControl());

    if (!rcs || !rcs->init(this))
    {
        if (errDescription)
        {
            *errDescription = "Failed to initialize RenderingControl";
        }
        return false;
    }

    if (m_configuration->hasOwnershipOfRendererConnectionManager())
    {
        m_configuration->rendererConnectionManager()->setParent(this);
    }

    if (!connectionManager()->actions().value("PrepareForConnection"))
    {
        // The ConnectionManager does not implement the action "PrepareForConnection",
        // which means that the ConnectionManager has to have a single connection with
        // ID zero. This connection handles whatever the Renderer is supposed to handle,
        // and this is why we have to create the default RendererConnection here:

        HProtocolInfo pinfo("http-get:*:*:*");

        HConnectionInfo connectionInfo(0, pinfo);
        connectionInfo.setAvTransportId(0);
        connectionInfo.setRcsId(0);

        HRendererConnection* mediaConnection =
            createRendererConnection(connectionInfo);

        if (!mediaConnection)
        {
            if (errDescription)
            {
                *errDescription = "Could not create the default media renderer connection";
            }
            return false;
        }
    }

    m_timer.start();

    return true;
}

qint32 HMediaRendererDevice::prepareForConnection(HConnectionInfo* connectionInfo)
{
    Q_ASSERT(connectionInfo);

    connectionInfo->setAvTransportId(avTransport()->nextId());
    connectionInfo->setRcsId(renderingControl()->nextId());

    HRendererConnection* rendererConnection =
        createRendererConnection(*connectionInfo);

    if (!rendererConnection)
    {
        return HConnectionManagerInfo::LocalRestrictions;
    }

    return UpnpSuccess;
}

qint32 HMediaRendererDevice::connectionComplete(qint32 connectionId)
{
    return m_configuration->rendererConnectionManager()->connectionComplete(
            connectionManager(), connectionId) ?
                (qint32) UpnpSuccess :
                (qint32) HConnectionManagerInfo::InvalidConnectionReference;
}

HRendererConnection* HMediaRendererDevice::createRendererConnection(
    const HConnectionInfo& connectionInfo)
{
    Q_ASSERT(connectionInfo.isValid());

    HRendererConnection* connection =
        m_configuration->rendererConnectionManager()->createAndAdd(
            connectionManager(), connectionInfo);

    if (connection)
    {
        bool ok = connect(
            connection->rendererConnectionInfo(),
            SIGNAL(propertyChanged(Herqq::Upnp::Av::HRendererConnectionInfo*, Herqq::Upnp::Av::HRendererConnectionEventInfo)),
            this,
            SLOT(propertyChanged(Herqq::Upnp::Av::HRendererConnectionInfo*, Herqq::Upnp::Av::HRendererConnectionEventInfo)));
        Q_ASSERT(ok); Q_UNUSED(ok)
    }

    return connection;
}

HRendererConnection* HMediaRendererDevice::findConnectionByAvTransportId(qint32 id) const
{
    QList<quint32> connectionIds;
    qint32 rc = connectionManager()->getCurrentConnectionIDs(&connectionIds);
    Q_ASSERT(rc == UpnpSuccess); Q_UNUSED(rc)

    foreach(quint32 cid, connectionIds)
    {
        HConnectionInfo info;
        rc = connectionManager()->getCurrentConnectionInfo((qint32)cid, &info);
        Q_ASSERT(rc == UpnpSuccess);

        if (info.avTransportId() == id)
        {
            HRendererConnection* connection =
                m_configuration->rendererConnectionManager()->connection(
                    connectionManager(), cid);

            return connection;
        }
    }

    return 0;
}

HRendererConnection* HMediaRendererDevice::findConnectionByRcsId(qint32 id) const
{
    QList<quint32> connectionIds;
    qint32 rc = connectionManager()->getCurrentConnectionIDs(&connectionIds);
    Q_ASSERT(rc == UpnpSuccess); Q_UNUSED(rc)

    foreach(quint32 cid, connectionIds)
    {
        HConnectionInfo info;
        rc = connectionManager()->getCurrentConnectionInfo((qint32)cid, &info);
        Q_ASSERT(rc == UpnpSuccess);

        if (info.rcsId() == id)
        {
            HRendererConnection* connection =
                m_configuration->rendererConnectionManager()->connection(
                    connectionManager(), cid);
            Q_ASSERT(connection);

            return connection;
        }
    }

    return 0;
}

HMediaRendererDeviceConfiguration* HMediaRendererDevice::configuration() const
{
    return m_configuration;
}

HRenderingControlService* HMediaRendererDevice::renderingControl() const
{
    if (!h_ptr->isValid())
    {
        return 0;
    }

    return static_cast<HRenderingControlService*>(
        HAbstractMediaRendererDevice::renderingControl());
}

HConnectionManagerSinkService* HMediaRendererDevice::connectionManager() const
{
    if (!h_ptr->isValid())
    {
        return 0;
    }

    return static_cast<HConnectionManagerSinkService*>(
        HAbstractMediaRendererDevice::connectionManager());
}

HTransportSinkService* HMediaRendererDevice::avTransport() const
{
    if (!h_ptr->isValid())
    {
        return 0;
    }

    return static_cast<HTransportSinkService*>(
        HAbstractMediaRendererDevice::avTransport());
}

}
}
}
