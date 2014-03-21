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

#include "hmediarenderer_adapter.h"
#include "hmediarenderer_adapter_p.h"
#include "hmediarenderer_info.h"

#include "hconnection.h"
#include "../hav_devicemodel_infoprovider.h"

#include "../transport/havtransport_adapter.h"
#include "../connectionmanager/hconnectioninfo.h"
#include "../renderingcontrol/hrenderingcontrol_info.h"
#include "../renderingcontrol/hrenderingcontrol_adapter.h"
#include "../connectionmanager/hconnectionmanager_adapter.h"
#include "../connectionmanager/hprepareforconnection_result.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HClientDevice>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPrepareForConnectionOp
 *******************************************************************************/
HPrepareForConnectionOp::HPrepareForConnectionOp(
    const HPrepareForConnectionOp& other) :
        HClientAdapterOp<qint32>(other),
            m_protocolInfo(other.m_protocolInfo), m_peerCm(other.m_peerCm)
{
}

HPrepareForConnectionOp::HPrepareForConnectionOp(
    const HProtocolInfo& protocolInfo, const HConnectionManagerId& peerCm) :
        m_protocolInfo(protocolInfo), m_peerCm(peerCm)
{
}

/*******************************************************************************
 * HMediaRendererAdapterPrivate
 *******************************************************************************/
HMediaRendererAdapterPrivate::HMediaRendererAdapterPrivate() :
    HClientDeviceAdapterPrivate(HMediaRendererInfo::supportedDeviceType()),
        m_cm(0), m_transportService(0), m_renderingControlService(0),
        m_cachedConnections()
{
}

HMediaRendererAdapterPrivate::~HMediaRendererAdapterPrivate()
{
    qDeleteAll(m_cachedConnections);
}

void HMediaRendererAdapterPrivate::cacheConnection(HConnection* connection)
{
    qint32 conId = connection->info().connectionId();
    if (m_cachedConnections.contains(conId))
    {
        delete m_cachedConnections.value(conId);
    }
    m_cachedConnections.insert(conId, connection);
}

void HMediaRendererAdapterPrivate::connectSignals()
{
    H_Q(HMediaRendererAdapter);

    bool ok = QObject::connect(
        m_cm.data(),
        SIGNAL(getProtocolInfoCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HProtocolInfoResult>)),
        q,
        SLOT(getProtocolInfoCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HProtocolInfoResult>)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = QObject::connect(
        m_cm.data(),
        SIGNAL(prepareForConnectionCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HPrepareForConnectionResult>)),
        q,
        SLOT(prepareForConnectionCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HPrepareForConnectionResult>)));
    Q_ASSERT(ok);

    ok = QObject::connect(
        m_cm.data(),
        SIGNAL(connectionCompleteCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOpNull)),
        q,
        SLOT(connectionCompleteCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOpNull)));
    Q_ASSERT(ok);

    ok = QObject::connect(
        m_cm.data(),
        SIGNAL(getCurrentConnectionIDsCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<QList<quint32> >)),
        q,
        SLOT(getCurrentConnectionIDsCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<QList<quint32> >)));
    Q_ASSERT(ok);

    ok = QObject::connect(
        m_cm.data(),
        SIGNAL(getCurrentConnectionInfoCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HConnectionInfo>)),
        q,
        SLOT(getCurrentConnectionInfoCompleted(
            Herqq::Upnp::Av::HConnectionManagerAdapter*,
            Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HConnectionInfo>)));
    Q_ASSERT(ok);
}

/*******************************************************************************
 * HMediaRendererAdapter
 *******************************************************************************/
HMediaRendererAdapter::HMediaRendererAdapter(QObject* parent) :
    HClientDeviceAdapter(*new HMediaRendererAdapterPrivate(), parent)
{
}

HMediaRendererAdapter::~HMediaRendererAdapter()
{
}

bool HMediaRendererAdapter::canPrepareNewConnection() const
{
    return connectionManager()->implementsAction("PrepareForConnection");
}

void HMediaRendererAdapter::getProtocolInfoCompleted(
    HConnectionManagerAdapter*, const HClientAdapterOp<HProtocolInfoResult>&)
{
    //H_D(HMediaRendererAdapter);
}

void HMediaRendererAdapter::prepareForConnectionCompleted(
    HConnectionManagerAdapter*,
    const HClientAdapterOp<HPrepareForConnectionResult>& op)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HMediaRendererAdapter);

    HPrepareForConnectionOp pOp = h_ptr->takeOp<HPrepareForConnectionOp>(op);

    if (pOp.isNull())
    {
        return;
    }
    else if (pOp.returnValue() != UpnpSuccess)
    {
        emit error(this, pOp);
        return;
    }

    HPrepareForConnectionResult result = op.value();

    HAvTransportAdapter* transport = 0;
    if (result.avTransportId() >= 0)
    {
        transport = new HAvTransportAdapter(
            static_cast<quint32>(result.avTransportId()));

        bool ok = transport->setService(h->m_transportService, HClientServiceAdapter::MinimalValidation);
        Q_ASSERT_X(ok, "", transport->lastErrorDescription().toLocal8Bit()); Q_UNUSED(ok)
    }

    HRenderingControlAdapter* rc = 0;
    if (result.rcsId() >= 0)
    {
        rc = new HRenderingControlAdapter(static_cast<quint32>(result.rcsId()));
        bool ok = rc->setService(h->m_renderingControlService, HClientServiceAdapter::MinimalValidation);
        Q_ASSERT(ok); Q_UNUSED(ok)
    }

    HConnectionInfo info(
        result.connectionId(),
        result.avTransportId(),
        result.rcsId(),
        pOp.protocolInfo(),
        pOp.peerConnectionManager(),
        -1,
        HConnectionManagerInfo::DirectionInput,
        HConnectionManagerInfo::StatusOk);

    pOp.setValue(info.connectionId());

    HConnection* connection = new HConnection(info, connectionManager(), transport, rc);
    h->cacheConnection(connection);
    emit connectionReady(this, info.connectionId());
}

void HMediaRendererAdapter::connectionCompleteCompleted(
    HConnectionManagerAdapter*, const HClientAdapterOpNull&)
{
    //H_D(HMediaRendererAdapter);
}

void HMediaRendererAdapter::getCurrentConnectionIDsCompleted(
    HConnectionManagerAdapter*,
    const HClientAdapterOp<QList<quint32> >& curConIDsOp)
{
    H_D(HMediaRendererAdapter);
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HClientAdapterOp<qint32> pOp =
        h->takeOp<HClientAdapterOp<qint32> >(curConIDsOp);

    if (pOp.isNull())
    {
        return;
    }

    foreach(quint32 conId, curConIDsOp.value())
    {
        HClientAdapterOp<HConnectionInfo> curConInfoOp =
            h->m_cm->getCurrentConnectionInfo(conId);

        if (curConInfoOp.isNull())
        {
            HLOG_WARN("Failed to dispatch GetCurrentConnectionInfo() request");
            continue;
        }

        h->addOp(pOp, curConInfoOp);
    }
}

void HMediaRendererAdapter::getCurrentConnectionInfoCompleted(
    HConnectionManagerAdapter*, const HClientAdapterOp<HConnectionInfo>& op)
{
    H_D(HMediaRendererAdapter);

    HClientAdapterOp<qint32> pOp = h->takeOp<HClientAdapterOp<qint32> >(op);
    if (pOp.isNull())
    {
        return;
    }

    HConnectionInfo result = op.value();
    if (!result.isValid())
    {
        return;
    }

    HAvTransportAdapter* transport = new HAvTransportAdapter(result.avTransportId());
    bool ok = transport->setService(h->m_transportService);
    Q_ASSERT_X(ok, "", transport->lastErrorDescription().toLocal8Bit());

    HRenderingControlAdapter* rc = new HRenderingControlAdapter(result.rcsId());
    ok = rc->setService(h->m_renderingControlService);
    Q_ASSERT_X(ok, "", rc->lastErrorDescription().toLocal8Bit());

    pOp.setValue(result.connectionId());

    h->cacheConnection(new HConnection(result, connectionManager(), transport, rc));

    emit connectionReady(this, result.connectionId());
}

bool HMediaRendererAdapter::prepareDevice(HClientDevice* device)
{
    H_D(HMediaRendererAdapter);
    HClientService* cmService = device->serviceById(
        HServiceId("urn:upnp-org:serviceId:ConnectionManager"));

    if (!cmService)
    {
        HClientServices cmServices =
            device->servicesByType(
                HConnectionManagerInfo::supportedServiceType(),
                HResourceType::Inclusive);

        if (cmServices.isEmpty())
        {
            setLastErrorDescription("Mandatory service [ConnectionManager] is missing");
            return false;
        }
        else
        {
            cmService = cmServices.first();
        }
    }

    HClientService* tsService = device->serviceById(
        HServiceId("urn:upnp-org:serviceId:AVTransport"));

    if (!tsService)
    {
        HClientServices tsServices =
            device->servicesByType(
                HAvTransportInfo::supportedServiceType(),
                HResourceType::Inclusive);

        if (tsServices.isEmpty())
        {
            setLastErrorDescription("Mandatory service [AVTransport] is missing");
            return false;
        }
        else
        {
            tsService = tsServices.first();
        }
    }

    HClientService* rcService = device->serviceById(
        HServiceId("urn:upnp-org:serviceId:RenderingControl"));

    if (!rcService)
    {
        HClientServices rcServices =
            device->servicesByType(
                HRenderingControlInfo::supportedServiceType(),
                HResourceType::Inclusive);

        if (rcServices.isEmpty())
        {
            setLastErrorDescription("Mandatory service [RenderingControl] is missing");
            return false;
        }
        else
        {
            rcService = rcServices.first();
        }
    }

    h->m_cm.reset(new HConnectionManagerAdapter());
    if (!h->m_cm->setService(
        cmService, HClientServiceAdapter::MinimalValidation))
    {
        h->m_cm.reset(0);
        return false;
    }

    h->m_transportService = tsService;
    h->m_renderingControlService = rcService;

    h->connectSignals();

    return true;
}

HConnectionManagerAdapter* HMediaRendererAdapter::connectionManager() const
{
    const H_D(HMediaRendererAdapter);
    return h->m_cm.data();
}

HClientAdapterOp<qint32> HMediaRendererAdapter::prepareNewConnection(
    const HProtocolInfo& protocolInfo)
{
    return prepareNewConnection(protocolInfo, HConnectionManagerId());
}

HClientAdapterOp<qint32> HMediaRendererAdapter::prepareNewConnection(
    const HProtocolInfo& protocolInfo, const HConnectionManagerId& peerCm)
{
    if (!h_ptr->m_device)
    {
        return HClientAdapterOp<qint32>::createInvalid(UpnpUndefinedFailure, "");
    }

    HConnectionManagerAdapter* conMgr = connectionManager();
    if (!conMgr->implementsAction("PrepareForConnection"))
    {
        return HClientAdapterOp<qint32>::createInvalid(UpnpOptionalActionNotImplemented, "");
    }

    HClientAdapterOp<HPrepareForConnectionResult> op =
        conMgr->prepareForConnection(
            protocolInfo, peerCm, -1, HConnectionManagerInfo::DirectionInput);

    if (op.isNull())
    {
        return HClientAdapterOp<qint32>::createInvalid(op.returnValue(), "");
    }

    HPrepareForConnectionOp retVal(protocolInfo, peerCm);
    h_ptr->addOp(retVal, op);
    return retVal;
}

HClientAdapterOp<qint32> HMediaRendererAdapter::getCurrentConnections()
{
    if (!h_ptr->m_device)
    {
        return HClientAdapterOp<qint32>::createInvalid(UpnpUndefinedFailure, "");
    }

    HConnectionManagerAdapter* conMgr = connectionManager();
    HClientAdapterOp<QList<quint32> > op = conMgr->getCurrentConnectionIDs();

    if (op.isNull())
    {
        return HClientAdapterOp<qint32>::createInvalid(op.returnValue(), "");
    }

    HClientAdapterOp<qint32> retVal;
    h_ptr->addOp(retVal, op);
    return retVal;
}

HClientAdapterOp<qint32> HMediaRendererAdapter::getConnection(qint32 connectionId)
{
    if (!h_ptr->m_device)
    {
        return HClientAdapterOp<qint32>::createInvalid(UpnpUndefinedFailure, "");
    }

    HConnectionManagerAdapter* conMgr = connectionManager();

    HClientAdapterOp<HConnectionInfo> op =
        conMgr->getCurrentConnectionInfo(connectionId);

    if (op.isNull())
    {
        return HClientAdapterOp<qint32>::createInvalid(op.returnValue(), "");
    }

    HClientAdapterOp<qint32> retVal;
    h_ptr->addOp(retVal, op);
    return retVal;
}

bool HMediaRendererAdapter::hasConnection(qint32 connectionId) const
{
    const H_D(HMediaRendererAdapter);
    return h_ptr->m_device && h->m_cachedConnections.contains(connectionId);
}

HConnection* HMediaRendererAdapter::connection(qint32 connectionId) const
{
    const H_D(HMediaRendererAdapter);
    return h_ptr->m_device ? h->m_cachedConnections.value(connectionId) : 0;
}

HConnections HMediaRendererAdapter::connections() const
{
    const H_D(HMediaRendererAdapter);

    HConnections retVal;

    if (h_ptr->m_device)
    {
        retVal.append(h->m_cachedConnections.values());
    }

    return retVal;
}

HConnection* HMediaRendererAdapter::takeConnection(qint32 id)
{
    H_D(HMediaRendererAdapter);

    HConnection* retVal = 0;

    if (h_ptr->m_device)
    {
        retVal = h->m_cachedConnections.value(id);
        if (retVal)
        {
            h->m_cachedConnections.remove(id);
        }
    }

    return retVal;
}

HConnections HMediaRendererAdapter::takeConnections()
{
    H_D(HMediaRendererAdapter);

    HConnections retVal;

    if (h_ptr->m_device)
    {
        retVal.append(h->m_cachedConnections.values());
        h->m_cachedConnections.clear();
    }

    return retVal;
}

}
}
}
