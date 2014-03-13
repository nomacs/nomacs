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

#ifndef HDEVICE_P_H_
#define HDEVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HDeviceStatus>
#include <HUpnpCore/HResourceType>

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

namespace Herqq
{

namespace Upnp
{

//
//
//
template<typename Device, typename Service>
class HDevicePrivate
{
H_DISABLE_COPY(HDevicePrivate)

public: // attributes

    QScopedPointer<HDeviceInfo> m_deviceInfo;
    // The static device information read from a device description.

    QList<Device*> m_embeddedDevices;
    // The embedded devices this instance contains.

    QList<Service*> m_services;
    // The services this instance contains.

    Device* m_parentDevice;
    // ^^ this is not the "QObject" parent, but rather the parent in the
    // device tree.

    Device* q_ptr;
    // The "parent" QObject

    QList<QUrl> m_locations;
    // The URLs at which this device is available

    QString m_deviceDescription;
    // The full device description.
    // CONSIDER: would it be better to load this into memory only when needed?

    QScopedPointer<HDeviceStatus> m_deviceStatus;

public: // methods

    HDevicePrivate() :
        m_deviceInfo(0), m_embeddedDevices(), m_services(), m_parentDevice(0),
        q_ptr(0), m_locations(), m_deviceDescription(), m_deviceStatus(0)
    {
    }

    virtual ~HDevicePrivate(){}

    inline bool isValid() const { return m_deviceInfo.data(); }

    Device* rootDevice() const
    {
        Device* root = q_ptr;
        while(root->parentDevice())
        {
            root = root->parentDevice();
        }

        return root;
    }

    Service* serviceById(const HServiceId& serviceId) const
    {
        foreach(Service* sc, m_services)
        {
            if (sc->info().serviceId() == serviceId)
            {
                return sc;
            }
        }

        return 0;
    }

    QList<Service*> servicesByType(
        const HResourceType& type, HResourceType::VersionMatch versionMatch) const
    {
        if (!type.isValid())
        {
            return QList<Service*>();
        }

        QList<Service*> retVal;
        foreach(Service* sc, m_services)
        {
            if (sc->info().serviceType().compare(type, versionMatch))
            {
                retVal.push_back(sc);
            }
        }

        return retVal;
    }

    QList<Device*> embeddedDevicesByType(
        const HResourceType& type, HResourceType::VersionMatch versionMatch) const
    {
        if (!type.isValid())
        {
            return QList<Device*>();
        }

        QList<Device*> retVal;
        foreach(Device* dev, m_embeddedDevices)
        {
            if (dev->info().deviceType().compare(type, versionMatch))
            {
                retVal.push_back(dev);
            }
        }

        return retVal;
    }
};

}
}

#endif /* HDEVICE_P_H_ */
