/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hdevicehost_ssdp_handler_p.h"
#include "hserverdevicecontroller_p.h"

#include "../../general/hupnp_global_p.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"
#include "../../dataelements/hdiscoverytype.h"
#include "../../dataelements/hproduct_tokens.h"

#include "../../devicemodel/hdevicestatus.h"
#include "../../devicemodel/server/hserverdevice.h"
#include "../../devicemodel/server/hserverservice.h"

#include "../../general/hlogger_p.h"
#include "../../utils/hsysutils_p.h"

#include <QtCore/QUuid>
#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDelayedWriter
 ******************************************************************************/
HDelayedWriter::HDelayedWriter(
    HDeviceHostSsdpHandler& ssdp,
    const QList<HDiscoveryResponse>& responses,
    const HEndpoint& source,
    qint32 msecs) :
        QObject(&ssdp),
            m_ssdp(ssdp), m_responses(responses), m_source(source), m_msecs(msecs)
{
}

void HDelayedWriter::timerEvent(QTimerEvent*)
{
    HLOG2(H_AT, H_FUN, m_ssdp.loggingIdentifier());
    foreach(const HDiscoveryResponse& resp, m_responses)
    {
        qint32 count = m_ssdp.sendDiscoveryResponse(resp, m_source);
        if (count <= 0)
        {
            HLOG_WARN(QString("Failed to send discovery response [%1] to: [%2].").arg(
                resp.usn().toString(), m_source.toString()));
        }
    }

    emit sent();
}

void HDelayedWriter::run()
{
   startTimer(m_msecs);
}

/*******************************************************************************
 * HDeviceHostSsdpHandler
 ******************************************************************************/
HDeviceHostSsdpHandler::HDeviceHostSsdpHandler(
    const QByteArray& loggingIdentifier,
    HDeviceStorage<HServerDevice, HServerService, HServerDeviceController>& ds,
    QObject* parent) :
        HSsdp(loggingIdentifier, parent), m_deviceStorage(ds)
{
    Q_ASSERT(parent);
    setFilter(DiscoveryRequest);
}

HDeviceHostSsdpHandler::~HDeviceHostSsdpHandler()
{
}

bool HDeviceHostSsdpHandler::processSearchRequest_specificDevice(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HDiscoveryType st = req.searchTarget();
    QUuid uuid = st.udn().value();
    if (uuid.isNull())
    {
        HLOG_DBG(QString("Invalid device-UUID: [%1]").arg(st.udn().toString()));
        return false;
    }

    const HServerDevice* device =
        m_deviceStorage.searchDeviceByUdn(HUdn(uuid), AllDevices);

    if (!device)
    {
        HLOG_DBG(QString("No device with the specified UUID: [%1]").arg(
            uuid.toString()));

        return false;
    }

    QUrl location;
    if (!m_deviceStorage.searchValidLocation(device, source, &location))
    {
        HLOG_DBG(QString(
            "Found a device with uuid: [%1], but it is not "
            "available on the interface that has address: [%2]").arg(
                uuid.toString(), source.toString()));

        return false;
    }

    const HServerDeviceController* controller =
        m_deviceStorage.getController(device);

    Q_ASSERT(controller);

    responses->push_back(
        HDiscoveryResponse(
            controller->deviceTimeoutInSecs() * 2,
            QDateTime::currentDateTime(),
            location,
            HSysInfo::instance().herqqProductTokens(),
            st, // the searched usn
            device->deviceStatus().bootId(),
            device->deviceStatus().configId()
            ));

    return true;
}

bool HDeviceHostSsdpHandler::processSearchRequest_deviceType(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HDiscoveryType st = req.searchTarget();

    QList<HServerDevice*> foundDevices =
        m_deviceStorage.searchDevicesByDeviceType(
            st.resourceType(),
            HResourceType::EqualOrGreater,
            AllDevices);

    if (!foundDevices.size())
    {
        HLOG_DBG(QString("No devices match the specified type: [%1]").arg(
            st.resourceType().toString()));

        return false;
    }

    qint32 prevSize = responses->size();
    foreach(const HServerDevice* device, foundDevices)
    {
        QUrl location;
        if (!m_deviceStorage.searchValidLocation(device, source, &location))
        {
            HLOG_DBG(QString(
                "Found a matching device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.toString()));

            continue;
        }

        st.setUdn(device->info().udn());

        const HServerDeviceController* controller =
            m_deviceStorage.getController(device);

        Q_ASSERT(controller);

        responses->push_back(
            HDiscoveryResponse(
                controller->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location,
                HSysInfo::instance().herqqProductTokens(),
                st,
                device->deviceStatus().bootId(),
                device->deviceStatus().configId()
                ));
    }

    return responses->size() > prevSize;
}

bool HDeviceHostSsdpHandler::processSearchRequest_serviceType(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HDiscoveryType st = req.searchTarget();

    QList<HServerService*> foundServices =
        m_deviceStorage.searchServicesByServiceType(
            st.resourceType(),
            HResourceType::EqualOrGreater);

    if (!foundServices.size())
    {
       HLOG_DBG(QString(
           "No services match the specified type: [%1]").arg(
               st.resourceType().toString()));

       return false;
    }

    qint32 prevSize = responses->size();
    foreach(const HServerService* service, foundServices)
    {
        const HServerDevice* device = service->parentDevice();
        Q_ASSERT(device);

        QUrl location;
        if (!m_deviceStorage.searchValidLocation(device, source, &location))
        {
            HLOG_DBG(QString(
                "Found a matching device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.toString()));

            continue;
        }

        HDeviceInfo deviceInfo = device->info();

        const HServerDevice* dc =
            m_deviceStorage.searchDeviceByUdn(deviceInfo.udn(), AllDevices);

        Q_ASSERT(dc);

        st.setUdn(deviceInfo.udn());

        const HServerDeviceController* controller =
            m_deviceStorage.getController(device);

        Q_ASSERT(controller);

        responses->push_back(
            HDiscoveryResponse(
                controller->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location,
                HSysInfo::instance().herqqProductTokens(),
                st,
                dc->deviceStatus().bootId(),
                dc->deviceStatus().configId()
                ));
    }

    return responses->size() > prevSize;
}

void HDeviceHostSsdpHandler::processSearchRequest(
    const HServerDevice* device, const QUrl& location,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT(device);

    HDeviceInfo deviceInfo = device->info();
    HProductTokens pt = HSysInfo::instance().herqqProductTokens();
    HDiscoveryType usn(deviceInfo.udn());

    const HServerDeviceController* controller =
        m_deviceStorage.getController(device);

    Q_ASSERT(controller);

    const HDeviceStatus& deviceStatus = device->deviceStatus();

    // device UDN
    responses->push_back(
        HDiscoveryResponse(
            controller->deviceTimeoutInSecs() * 2,
            QDateTime::currentDateTime(), location, pt, usn,
            deviceStatus.bootId(),
            deviceStatus.configId()));

    usn.setResourceType(deviceInfo.deviceType());

    // device type
    responses->push_back(
        HDiscoveryResponse(
            controller->deviceTimeoutInSecs() * 2,
            QDateTime::currentDateTime(), location, pt, usn,
            deviceStatus.bootId(),
            deviceStatus.configId()));

    const HServerServices& services = device->services();
    foreach(const HServerService* service, services)
    {
        usn.setResourceType(service->info().serviceType());

        responses->push_back(
            HDiscoveryResponse(
                controller->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(), location, pt, usn,
                deviceStatus.bootId(),
                deviceStatus.configId()
                ));
    }

    const HServerDevices& devices = device->embeddedDevices();
    foreach(const HServerDevice* embeddedDevice, devices)
    {
        processSearchRequest(embeddedDevice, location, responses);
    }
}

bool HDeviceHostSsdpHandler::processSearchRequest_AllDevices(
    const HDiscoveryRequest& /*req*/, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT(responses);

    const HProductTokens& pt = HSysInfo::instance().herqqProductTokens();

    const HServerDevices& rootDevices = m_deviceStorage.rootDevices();

    qint32 prevSize = responses->size();
    foreach(const HServerDevice* rootDevice, rootDevices)
    {
        QUrl location;
        if (!m_deviceStorage.searchValidLocation(rootDevice, source, &location))
        {
            HLOG_DBG(QString(
                "Found a device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.toString()));

            continue;
        }

        HDiscoveryType usn(rootDevice->info().udn(), true);

        const HServerDeviceController* controller =
            m_deviceStorage.getController(rootDevice);

        Q_ASSERT(controller);

        responses->push_back(
            HDiscoveryResponse(
                controller->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location,
                pt,
                usn,
                rootDevice->deviceStatus().bootId(),
                rootDevice->deviceStatus().configId()
                ));

        processSearchRequest(rootDevice, location, responses);

        const HServerDevices& devices = rootDevice->embeddedDevices();
        foreach(const HServerDevice* embeddedDevice, devices)
        {
            if (!m_deviceStorage.searchValidLocation(
                    embeddedDevice, source, &location))
            {
                // highly uncommon, but possible; the root device is "active"
                // on the network interface to which the request came,
                // but at least one of its embedded devices is not.

                HLOG_DBG(QString(
                    "Skipping an embedded device that is not "
                    "available on the interface that has address: [%1]").arg(
                        source.toString()));

                continue;
            }

            processSearchRequest(embeddedDevice, location, responses);
        }
    }

    return responses->size() > prevSize;
}

bool HDeviceHostSsdpHandler::processSearchRequest_RootDevice(
    const HDiscoveryRequest& /*req*/, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT(responses);

    const HServerDevices& rootDevices = m_deviceStorage.rootDevices();

    qint32 prevSize = responses->size();
    foreach(const HServerDevice* rootDevice, rootDevices)
    {
        QUrl location;
        if (!m_deviceStorage.searchValidLocation(rootDevice, source, &location))
        {
            HLOG_DBG(QString(
                "Found a root device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.hostAddress().toString()));

            continue;
        }

        HDiscoveryType usn(rootDevice->info().udn(), true);

        const HServerDeviceController* controller =
            m_deviceStorage.getController(rootDevice);

        Q_ASSERT(controller);

        responses->push_back(
            HDiscoveryResponse(
                controller->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location,
                HSysInfo::instance().herqqProductTokens(),
                usn,
                rootDevice->deviceStatus().bootId(),
                rootDevice->deviceStatus().configId()));
    }

    return responses->size() > prevSize;
}

bool HDeviceHostSsdpHandler::incomingDiscoveryRequest(
    const HDiscoveryRequest& msg, const HEndpoint& source,
    DiscoveryRequestMethod requestType)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HLOG_DBG(QString("Received discovery request for [%1] from [%2]").arg(
        msg.searchTarget().toString(), source.toString()));

    bool ok = false;
    QList<HDiscoveryResponse> responses;
    switch (msg.searchTarget().type())
    {
        case HDiscoveryType::All:
            ok = processSearchRequest_AllDevices(msg, source, &responses);
            break;

        case HDiscoveryType::RootDevices:
            ok = processSearchRequest_RootDevice(msg, source, &responses);
            break;

        case HDiscoveryType::SpecificDevice:
            ok = processSearchRequest_specificDevice(msg, source, &responses);
            break;

        case HDiscoveryType::DeviceType:
            ok = processSearchRequest_deviceType(msg, source, &responses);
            break;

        case HDiscoveryType::ServiceType:
            ok = processSearchRequest_serviceType(msg, source, &responses);
            break;

        default:
            return true;
    }

    if (ok)
    {
        if (requestType == MulticastDiscovery)
        {
            HDelayedWriter* writer =
                new HDelayedWriter(
                    *this, responses, source, (qrand() % msg.mx()) * 1000);

            bool ok =
                connect(writer, SIGNAL(sent()), writer, SLOT(deleteLater()));

            Q_ASSERT(ok); Q_UNUSED(ok)

            writer->run();
        }
        else
        {
            foreach (const HDiscoveryResponse& resp, responses)
            {
                qint32 count = sendDiscoveryResponse(resp, source);
                Q_ASSERT(count >= 0); Q_UNUSED(count)
            }
        }
    }
    else
    {
        HLOG_DBG(QString(
            "No resources found for discovery request [%1] from [%2]").arg(
                msg.searchTarget().toString(), source.toString()));
    }

    return true;
}

}
}
