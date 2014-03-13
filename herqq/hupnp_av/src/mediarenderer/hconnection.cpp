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

#include "hconnection.h"
#include "hrcs_lastchange_info.h"
#include "havt_lastchange_info.h"
#include "../connectionmanager/hconnectioninfo.h"
#include "../connectionmanager/hconnectionmanager_adapter.h"

#include "../transport/havtransport_adapter.h"

#include "../renderingcontrol/hchannel.h"
#include "../renderingcontrol/hrenderingcontrol_adapter.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <QtCore/QVariant>
#include <QtCore/QXmlStreamReader>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HConnectionPrivate
 ******************************************************************************/
class H_UPNP_AV_EXPORT HConnectionPrivate
{
H_DISABLE_COPY(HConnectionPrivate)

public:

    HConnectionInfo m_connectionInfo;
    HConnectionManagerAdapter* m_cm;
    HAvTransportAdapter* m_transport;
    HRenderingControlAdapter* m_renderingControl;
    bool m_autoClose;
    bool m_valid;

    HConnectionPrivate(
        const HConnectionInfo& info,
        HConnectionManagerAdapter* cm,
        HAvTransportAdapter* avt,
        HRenderingControlAdapter* rcs) :
            m_connectionInfo(info),
            m_cm(cm),
            m_transport(avt),
            m_renderingControl(rcs),
            m_autoClose(false),
            m_valid(true)
    {
    }

    ~HConnectionPrivate()
    {
        delete m_transport;
        delete m_renderingControl;
    }
};

/*******************************************************************************
 * HConnection
 ******************************************************************************/
HConnection::HConnection(
    const HConnectionInfo& connectionInfo,
    HConnectionManagerAdapter* cm,
    HAvTransportAdapter* avt,
    HRenderingControlAdapter* rcs,
    QObject* parent) :
        QObject(parent),
            h_ptr(new HConnectionPrivate(connectionInfo, cm, avt, rcs))
{
    bool ok = connect(
        cm,
        SIGNAL(currentConnectionIdsChanged(Herqq::Upnp::Av::HConnectionManagerAdapter*, QList<quint32>)),
        this,
        SLOT(currentConnectionIdsChanged(Herqq::Upnp::Av::HConnectionManagerAdapter*, QList<quint32>)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        rcs,
        SIGNAL(lastChangeReceived(Herqq::Upnp::Av::HRenderingControlAdapter*, QString)),
        this,
        SLOT(lastChangeReceived(Herqq::Upnp::Av::HRenderingControlAdapter*, QString)));
    Q_ASSERT(ok);

    ok = connect(
        avt,
        SIGNAL(lastChangeReceived(Herqq::Upnp::Av::HAvTransportAdapter*, QString)),
        this,
        SLOT(lastChangeReceived(Herqq::Upnp::Av::HAvTransportAdapter*, QString)));
    Q_ASSERT(ok);
}

HConnection::~HConnection()
{
    if (autoCloseConnection())
    {
        h_ptr->m_cm->connectionComplete(info().connectionId());
    }
    delete h_ptr;
}

void HConnection::currentConnectionIdsChanged(
    HConnectionManagerAdapter*, const QList<quint32>& currentIds)
{
    if (!currentIds.contains(info().connectionId()))
    {
        h_ptr->m_valid = false;
        emit invalidated(this);
    }
}

void HConnection::lastChangeReceived(
    HRenderingControlAdapter*, const QString& data)
{
    HLOG(H_AT, H_FUN);

    QXmlStreamReader reader(data.trimmed());
    //addNamespaces(reader);

    if (reader.readNextStartElement())
    {
        if (reader.name().compare("Event", Qt::CaseInsensitive) != 0)
        {
            return;
        }
    }
    else
    {
        return;
    }

    HRcsLastChangeInfos infos;
    while(!reader.atEnd() && reader.readNextStartElement())
    {
        QStringRef name = reader.name();

        if (name.compare("InstanceID", Qt::CaseInsensitive))
        {
            continue;
        }

        if (reader.attributes().value("val").toString().toInt() !=
            h_ptr->m_connectionInfo.connectionId())
        {
            continue;
        }

        while(!reader.atEnd() && reader.readNextStartElement())
        {
            name = reader.name();

            QXmlStreamAttributes attrs = reader.attributes();
            QString value = attrs.value("val").toString();
            QString channel = attrs.value("channel").toString();

            HRcsLastChangeInfo info(name.toString(), value);
            if (info.isValid())
            {
                if (!channel.isEmpty())
                {
                    info.setChannel(channel);
                }
                infos.append(info);
            }
        }
    }

    if (!infos.isEmpty())
    {
        emit renderingControlStateChanged(this, infos);
    }
}

void HConnection::lastChangeReceived(HAvTransportAdapter*, const QString& data)
{
    HLOG(H_AT, H_FUN);

    QXmlStreamReader reader(data.trimmed());
    //addNamespaces(reader);

    if (reader.readNextStartElement())
    {
        if (reader.name().compare("Event", Qt::CaseInsensitive) != 0)
        {
            return;
        }
    }
    else
    {
        return;
    }

    HAvtLastChangeInfos infos;
    while(!reader.atEnd() && reader.readNextStartElement())
    {
        QStringRef name = reader.name();

        if (name.compare("InstanceID", Qt::CaseInsensitive))
        {
            continue;
        }

        if (reader.attributes().value("val").toString().toInt() !=
            h_ptr->m_connectionInfo.connectionId())
        {
            continue;
        }

        while(!reader.atEnd() && reader.readNextStartElement())
        {
            name = reader.name();

            QXmlStreamAttributes attrs = reader.attributes();
            QString value = attrs.value("val").toString();

            HAvtLastChangeInfo info(name.toString(), value);
            if (info.isValid())
            {
                infos.append(info);
            }
        }
    }

    if (!infos.isEmpty())
    {
        emit avTransportStateChanged(this, infos);
    }
}

const HConnectionInfo& HConnection::info() const
{
    return h_ptr->m_connectionInfo;
}

HAvTransportAdapter* HConnection::transport() const
{
    return h_ptr->m_transport;
}

HRenderingControlAdapter* HConnection::renderingControl() const
{
    return h_ptr->m_renderingControl;
}

bool HConnection::autoCloseConnection() const
{
    return h_ptr->m_autoClose;
}

bool HConnection::isValid() const
{
    return h_ptr->m_valid;
}

void HConnection::setAutoCloseConnection(bool enable)
{
    h_ptr->m_autoClose = enable;
}

}
}
}
