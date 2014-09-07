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

#ifndef HPRESENCE_ANNOUNCER_H_
#define HPRESENCE_ANNOUNCER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hserverdevicecontroller_p.h"

#include "../../general/hupnp_global_p.h"
#include "../../devicemodel/hdevicestatus.h"
#include "../../devicemodel/server/hserverdevice.h"
#include "../../devicemodel/server/hserverservice.h"

#include "../../ssdp/hssdp.h"
#include "../../ssdp/hdiscovery_messages.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"
#include "../../dataelements/hdiscoverytype.h"
#include "../../dataelements/hproduct_tokens.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

//
//
//
class Announcement
{

protected:

    HServerDevice* m_device;
    HDiscoveryType m_usn;
    QUrl m_location;
    int m_deviceTimeoutInSecs;

public:

    Announcement()
    {
    }

    Announcement(
        HServerDevice* device, const HDiscoveryType& usn,
        const QUrl& location, int deviceTimeoutInSecs) :
            m_device(device), m_usn(usn), m_location(location),
            m_deviceTimeoutInSecs(deviceTimeoutInSecs)
    {
        Q_ASSERT(m_device);
        Q_ASSERT(m_usn.type() != HDiscoveryType::Undefined);
        Q_ASSERT(m_location.isValid() && !m_location.isEmpty());
    }

    virtual ~Announcement()
    {
    }
};

//
//
//
class ResourceAvailableAnnouncement :
    private Announcement
{

public:

    ResourceAvailableAnnouncement()
    {
    }

    ResourceAvailableAnnouncement(
        HServerDevice* device, const HDiscoveryType& usn,
        const QUrl& location, int deviceTimeoutInSecs) :
            Announcement(device, usn, location, deviceTimeoutInSecs)
    {
    }

    HResourceAvailable operator()() const
    {
        const HProductTokens& pt = HSysInfo::instance().herqqProductTokens();

        return HResourceAvailable(
            m_deviceTimeoutInSecs * 2,
            m_location,
            pt,
            m_usn,
            m_device->deviceStatus().bootId(),
            m_device->deviceStatus().configId()
            );
    }
};

//
//
//
class ResourceUnavailableAnnouncement :
    private Announcement
{
public:

    ResourceUnavailableAnnouncement()
    {
    }

    ResourceUnavailableAnnouncement(
        HServerDevice* device, const HDiscoveryType& usn,
        const QUrl& location, int deviceTimeoutInSecs) :
            Announcement(device, usn, location, deviceTimeoutInSecs)
    {
    }

    HResourceUnavailable operator()() const
    {
        return HResourceUnavailable(
            m_usn,
            m_device->deviceStatus().bootId(),
            m_device->deviceStatus().configId()
            );
    }
};

class HDeviceHostSsdpHandler;

//
// Class that sends the SSDP announcements.
//
class PresenceAnnouncer
{
private:

    QList<HDeviceHostSsdpHandler*> m_ssdps;
    quint32 m_advertisementCount;

private:

    template<typename AnnouncementType>
    void createAnnouncementMessagesForEmbeddedDevice(
        HServerDevice* device, int deviceTimeoutInSecs,
        QList<AnnouncementType>* announcements)
    {
        QList<QUrl> locations = device->locations();
        foreach(const QUrl& location, locations)
        {
            HDeviceInfo deviceInfo = device->info();

            HUdn udn = deviceInfo.udn();
            HDiscoveryType usn(udn);

            // device UDN advertisement
            announcements->push_back(
                AnnouncementType(device, usn, location, deviceTimeoutInSecs));

            // device type advertisement
            usn.setResourceType(deviceInfo.deviceType());
            announcements->push_back(
                AnnouncementType(device, usn, location, deviceTimeoutInSecs));

            // service advertisements
            const HServerServices& services = device->services();
            foreach(HServerService* service, services)
            {
                usn.setResourceType(service->info().serviceType());
                announcements->push_back(
                    AnnouncementType(device, usn, location, deviceTimeoutInSecs));
            }
        }

        const HServerDevices& devices = device->embeddedDevices();
        foreach(HServerDevice* embeddedDevice, devices)
        {
            createAnnouncementMessagesForEmbeddedDevice(
                embeddedDevice, deviceTimeoutInSecs, announcements);
        }
    }

public:

    PresenceAnnouncer(
        const QList<HDeviceHostSsdpHandler*>& ssdps, quint32 advertisementCount) :
            m_ssdps(ssdps), m_advertisementCount(advertisementCount)
    {
        Q_ASSERT(m_advertisementCount > 0);
    }

    ~PresenceAnnouncer()
    {
    }

    template<typename AnnouncementType>
    void announce(const HServerDeviceController* rootDevice)
    {
        Q_ASSERT(rootDevice);

        QList<AnnouncementType> announcements;

        createAnnouncementMessagesForRootDevice(
            rootDevice->m_device, rootDevice->deviceTimeoutInSecs(),
            &announcements);

        sendAnnouncements(announcements);
    }

    template<typename AnnouncementType>
    void announce(const QList<HServerDeviceController*>& rootDevices)
    {
        QList<AnnouncementType> announcements;

        foreach(HServerDeviceController* rootDevice, rootDevices)
        {
            createAnnouncementMessagesForRootDevice(
                rootDevice->m_device, rootDevice->deviceTimeoutInSecs(),
                &announcements);
        }

        sendAnnouncements(announcements);
    }

    template<typename AnnouncementType>
    void createAnnouncementMessagesForRootDevice(
        HServerDevice* rootDevice, int deviceTimeoutInSecs,
        QList<AnnouncementType>* announcements)
    {
        QList<QUrl> locations = rootDevice->locations();
        foreach(const QUrl& location, locations)
        {
            HUdn udn(rootDevice->info().udn());
            HDiscoveryType usn(udn, true);

            announcements->push_back(
                AnnouncementType(rootDevice, usn, location, deviceTimeoutInSecs));
        }

        // generic device advertisement (same for both root and embedded devices)
        createAnnouncementMessagesForEmbeddedDevice(
            rootDevice, deviceTimeoutInSecs, announcements);
    }

    template<typename AnnouncementType>
    void sendAnnouncements(const QList<AnnouncementType>& announcements)
    {
        for (quint32 i = 0; i < m_advertisementCount; ++i)
        {
            foreach(HSsdp* ssdp, m_ssdps)
            {
                foreach(const AnnouncementType& at, announcements)
                {
                    ssdp->announcePresence(at());
                }
            }
        }
    }
};

}
}

#endif /* HPRESENCE_ANNOUNCER_H_ */
