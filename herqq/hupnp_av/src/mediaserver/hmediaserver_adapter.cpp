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

#include "hmediaserver_adapter.h"
#include "hmediaserver_adapter_p.h"
#include "hmediaserver_info.h"

#include "../hav_devicemodel_infoprovider.h"

#include "../transport/havtransport_adapter.h"
#include "../contentdirectory/hcontentdirectory_adapter.h"
#include "../connectionmanager/hconnectionmanager_adapter.h"
//#include "../scheduledrecording/hscheduledrecording_adapter.h"

#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HClientDevice>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMediaServerAdapterPrivate
 ******************************************************************************/
HMediaServerAdapterPrivate::HMediaServerAdapterPrivate() :
    HClientDeviceAdapterPrivate(HMediaServerInfo::supportedDeviceType()),
        m_transport(0), m_contentDirectory(0), m_connectionManager(0)
        //m_scheduledRecording(0)
{
}

HMediaServerAdapterPrivate::~HMediaServerAdapterPrivate()
{
}

/*******************************************************************************
 * HMediaServerAdapter
 ******************************************************************************/
HMediaServerAdapter::HMediaServerAdapter(QObject* parent) :
    HClientDeviceAdapter(*new HMediaServerAdapterPrivate(), parent)
{
}

HMediaServerAdapter::~HMediaServerAdapter()
{
}

bool HMediaServerAdapter::prepareDevice(HClientDevice* device)
{
    H_D(HMediaServerAdapter);

    HClientService* avTransport =
        device->serviceById(HServiceId("urn:upnp-org:serviceId:AVTransport"));

    if (!avTransport)
    {
        HClientServices tsServices =
            device->servicesByType(
                HAvTransportInfo::supportedServiceType(),
                HResourceType::Inclusive);

        if (!tsServices.isEmpty())
        {
            avTransport = tsServices.first();
        }
    }

    HClientService* cds = device->serviceById(
        HServiceId("urn:upnp-org:serviceId:ContentDirectory"));

    if (!cds)
    {
        HClientServices cdsServices =
            device->servicesByType(
                HContentDirectoryInfo::supportedServiceType(),
                HResourceType::Inclusive);

        if (!cdsServices.isEmpty())
        {
            cds = cdsServices.first();
        }
        else
        {
            return false;
        }
    }

    HClientService* cm = device->serviceById(
        HServiceId("urn:upnp-org:serviceId:ConnectionManager"));

    if (!cm)
    {
        HClientServices cmServices =
            device->servicesByType(
                HConnectionManagerInfo::supportedServiceType(),
                HResourceType::Inclusive);

        if (!cmServices.isEmpty())
        {
            cm = cmServices.first();
        }
        else
        {
            return false;
        }
    }

    /*srv = device->serviceById(
        HServiceId("urn:upnp-org:serviceId:ScheduledRecording"));
    if (srv)
    {
        h->m_scheduledRecording = new HScheduledRecording();
        ok = h->m_scheduledRecording->setService(srv);
        if (!ok)
        {
            return false;
        }
    }*/

    QScopedPointer<HContentDirectoryAdapter> cdsAdapter(new HContentDirectoryAdapter());
    if (!cdsAdapter->setService(cds, HClientServiceAdapter::MinimalValidation))
    {
        return false;
    }

    QScopedPointer<HConnectionManagerAdapter> cmAdapter(new HConnectionManagerAdapter());
    cmAdapter.reset(new HConnectionManagerAdapter());
    if (!cmAdapter->setService(cm, HClientServiceAdapter::MinimalValidation))
    {
        return false;
    }

    h->m_contentDirectory.reset(cdsAdapter.take());
    h->m_connectionManager.reset(cmAdapter.take());
    h->m_transport = avTransport;

    return true;
}

HContentDirectoryAdapter* HMediaServerAdapter::contentDirectory() const
{
    const H_D(HMediaServerAdapter);
    return h->m_contentDirectory.data();
}

HConnectionManagerAdapter* HMediaServerAdapter::connectionManager() const
{
    const H_D(HMediaServerAdapter);
    return h->m_connectionManager.data();
}

HAvTransportAdapter* HMediaServerAdapter::transportService(quint32 instanceId) const
{
    const H_D(HMediaServerAdapter);
    QScopedPointer<HAvTransportAdapter> retVal(0);
    if (h->m_transport)
    {
        retVal.reset(new HAvTransportAdapter(instanceId));
        if (!retVal->setService(h->m_transport))
        {
            retVal.reset(0);
        }
    }
    return retVal.take();
}

//HScheduledRecordingAdapter* HMediaServerAdapter::scheduledRecordingService() const
//{
//    //const H_D(HMediaServerAdapter);
//    return 0;//return h->m_scheduledRecording.data();
//}

}
}
}
