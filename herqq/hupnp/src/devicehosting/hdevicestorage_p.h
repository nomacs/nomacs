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

#ifndef HDEVICESTORAGE_H_
#define HDEVICESTORAGE_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../general/hupnp_global_p.h"
#include "../general/hlogger_p.h"

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HEndpoint>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HResourceType>

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

//
//
//
template<typename Controller, typename T>
class MatchFunctor
{
private:

    T m_t;

public:

    MatchFunctor(const T& t) : m_t(t){}

    inline bool operator()(Controller* obj) const
    {
        return m_t(obj);
    }
};

static bool compareUrls(const QUrl& u1, const QUrl& u2)
{
    QString u1Str = extractRequestPart(u1);
    QString u2Str = extractRequestPart(u2);

    if (u1Str.startsWith('/')) { u1Str.remove(0, 1); }
    if (u2Str.startsWith('/')) { u2Str.remove(0, 1); }

    return u1Str == u2Str;
}

//
//
//
template<typename Service>
class ScpdUrlTester
{
private:
    QUrl m_url;

public:

    ScpdUrlTester(const QUrl& url) : m_url(url){}
    inline bool operator()(Service* service) const
    {
        Q_ASSERT(service);
        return compareUrls(m_url, service->info().scpdUrl());
    }
};

//
//
//
template<typename Service>
class ControlUrlTester
{
private:
    QUrl m_url;

public:

    ControlUrlTester(const QUrl& url) : m_url(url){}
    inline bool operator()(Service* service) const
    {
        Q_ASSERT(service);
        //return service->controlUrl() == m_url;
        return compareUrls(m_url, service->info().controlUrl());
    }
};

//
//
//
template<typename Service>
class EventUrlTester
{
private:
    QUrl m_url;

public:

    EventUrlTester(const QUrl& url) : m_url(url){}
    inline bool operator()(Service* service) const
    {
        Q_ASSERT(service);
        //return service->eventSubUrl() == m_url;
        return compareUrls(m_url, service->info().eventSubUrl());
    }
};

//
//
//
template<typename Device>
class UdnTester
{
private:

    HUdn m_udn;

public:

    UdnTester(const HUdn& udn) : m_udn(udn){}
    inline bool operator()(Device* device) const
    {
        Q_ASSERT(device);
        return device->info().udn() == m_udn;
    }
};

//
//
//
template<typename Device>
class DeviceTypeTester
{
private:

    HResourceType m_resourceType;
    HResourceType::VersionMatch m_versionMatch;

public:

    DeviceTypeTester(
        const HResourceType& resType, HResourceType::VersionMatch vm) :
            m_resourceType(resType), m_versionMatch(vm)
    {
    }

    bool test(const HResourceType& resType) const
    {
        return resType.compare(m_resourceType, m_versionMatch);
    }

    bool operator()(Device* device) const
    {
        Q_ASSERT(device);
        return test(device->info().deviceType());
    }
};

//
//
//
template<typename Service>
class ServiceTypeTester
{
private:

    HResourceType m_resourceType;
    HResourceType::VersionMatch m_versionMatch;

public:

    ServiceTypeTester(
        const HResourceType& resType, HResourceType::VersionMatch vm) :
            m_resourceType(resType), m_versionMatch(vm)
    {
    }

    bool test(const HResourceType& resType) const
    {
        return resType.compare(m_resourceType, m_versionMatch);
    }

    bool operator()(Service* service) const
    {
        Q_ASSERT(service);
        return test(service->info().serviceType());
    }
};

template<typename Device, typename T>
void seekDevices(
    Device* device, const MatchFunctor<Device, T>& mf,
    QList<Device*>& foundDevices, TargetDeviceType dts)
{
    Q_ASSERT(device);

    if (dts == RootDevices && device->parentDevice())
    {
        return;
    }

    if (mf(device))
    {
        foundDevices.push_back(device);
    }

    QList<Device*> devices = device->embeddedDevices();
    foreach(Device* device, devices)
    {
        seekDevices(device, mf, foundDevices, dts);
    }
}

//
//
//
template<typename Device, typename T>
void seekDevices(
    const QList<Device*>& devices, const MatchFunctor<Device, T>& mf,
    QList<Device*>& foundDevices, TargetDeviceType dts)
{
    foreach(Device* device, devices)
    {
        seekDevices(device, mf, foundDevices, dts);
    }
}

//
//
//
template<typename Device, typename Service, typename T>
Service* seekService(
    const QList<Device*>& devices, const MatchFunctor<Service, T>& mf)
{
    foreach(Device* device, devices)
    {
        QList<Service*> services = device->services();
        foreach(Service* service, services)
        {
            if (mf(service))
            {
                return service;
            }
        }

        Service* service = seekService(device->embeddedDevices(), mf);
        if (service)
        {
            return service;
        }
    }

    return 0;
}

//
//
//
template<typename Device, typename Service, typename T>
void seekServices(
    const QList<Device*>& devices,
    const MatchFunctor<Service, T>& mf,
    QList<Service*>& foundServices,
    bool rootDevicesOnly)
{
    foreach(Device* device, devices)
    {
        if (rootDevicesOnly && device->parentDevice())
        {
            continue;
        }

        QList<Service*> services = device->services();
        foreach(Service* service, services)
        {
            if (mf(service))
            {
                foundServices.push_back(service);
            }
        }

        if (rootDevicesOnly)
        {
            continue;
        }

        seekServices(device->embeddedDevices(), mf, foundServices, rootDevicesOnly);
    }
}

//
//
//
template<typename Device, typename Service, typename Controller = int>
class HDeviceStorage
{
H_DISABLE_COPY(HDeviceStorage)

private:

    const QByteArray m_loggingIdentifier;

    QList<Device*> m_rootDevices;
    // the device trees stored by this instance

    QList<QPair<Device*, Controller*> > m_deviceControllers;

    QString m_lastError;

public: // instance methods

    HDeviceStorage(const QByteArray& lid) :
        m_loggingIdentifier(lid), m_rootDevices()
    {
    }

    ~HDeviceStorage()
    {
        clear();
    }

    inline QString lastError() const
    {
        return m_lastError;
    }

    void clear()
    {
        qDeleteAll(m_rootDevices);
        m_rootDevices.clear();
        for(int i = 0; i < m_deviceControllers.size(); ++i)
        {
            delete m_deviceControllers.at(i).second;
        }
        m_deviceControllers.clear();
    }

    Controller* getController(const Device* device) const
    {
        Device* rootDev = device->rootDevice();
        for(int i = 0; i < m_deviceControllers.size(); ++i)
        {
            if (m_deviceControllers.at(i).first == rootDev)
            {
                return m_deviceControllers.at(i).second;
            }
        }

        return 0;
    }

    Device* searchDeviceByUdn(const HUdn& udn, TargetDeviceType dts) const
    {
        QList<Device*> devices;

        seekDevices(
            m_rootDevices,
            MatchFunctor<Device, UdnTester<Device> >(udn),
            devices,
            dts);

        return devices.size() > 0 ? devices[0] : 0;
    }

    bool searchValidLocation(
        const Device* device, const HEndpoint& interface, QUrl* location)
    {
        Q_ASSERT(device);

        QList<QUrl> locations = device->locations();
        QList<QUrl>::const_iterator ci = locations.constBegin();
        for(; ci != locations.constEnd(); ++ci)
        {
            /*if (QHostAddress(ci->host()) == interface.hostAddress())
            {
                *location = *ci;
                return true;
            }*/
    // TODO
            if (interface.hostAddress().isInSubnet(
                QHostAddress::parseSubnet(ci->host().append("/24"))))
            {
                *location = *ci;
                return true;
            }
        }

        return false;
    }

    QList<Device*> searchDevicesByDeviceType(
        const HResourceType& deviceType, HResourceType::VersionMatch vm,
        TargetDeviceType dts) const
    {
        QList<Device*> retVal;

        seekDevices(
            m_rootDevices,
            MatchFunctor<Device, DeviceTypeTester<Device> >(
                DeviceTypeTester<Device>(deviceType, vm)),
            retVal,
            dts);

        return retVal;
    }

    QList<Service*> searchServicesByServiceType(
        const HResourceType& serviceType, HResourceType::VersionMatch vm) const
    {
        QList<Service*> retVal;

        seekServices(
            m_rootDevices,
            MatchFunctor<Service, ServiceTypeTester<Service> >(
                ServiceTypeTester<Service>(serviceType, vm)),
            retVal,
            false);

        return retVal;
    }

    bool checkDeviceTreeForUdnConflicts(Device* device)
    {
        if (searchDeviceByUdn(device->info().udn(), RootDevices))
        {
            m_lastError =
                QString("Cannot host multiple devices with the same UDN [%1]").arg(
                    device->info().udn().toSimpleUuid());

            return false;
        }

        QList<Device*> devices = device->embeddedDevices();
        foreach(Device* embeddeDevice, devices)
        {
            if (!checkDeviceTreeForUdnConflicts(embeddeDevice))
            {
                return false;
            }
        }

        return true;
    }

    bool addRootDevice(Device* root, Controller* controller = 0)
    {
        HLOG2(H_AT, H_FUN, m_loggingIdentifier);

        Q_ASSERT(root);
        Q_ASSERT(root);
        Q_ASSERT(!root->parentDevice());

        if (!checkDeviceTreeForUdnConflicts(root))
        {
            return false;
        }

        m_rootDevices.push_back(root);
        m_deviceControllers.append(qMakePair(root, controller));

        HLOG_DBG(QString("New root device [%1] added. Current device count is %2").arg(
            root->info().friendlyName(), QString::number(m_rootDevices.size())));

        return true;
    }

    bool removeRootDevice(Device* root)
    {
        HLOG2(H_AT, H_FUN, m_loggingIdentifier);

        Q_ASSERT(root);
        Q_ASSERT(!root->parentDevice());

        HDeviceInfo devInfo = root->info();

        bool ok = m_rootDevices.removeOne(root);
        if (!ok)
        {
            HLOG_WARN(QString("Device [%1] was not found.").arg(
                devInfo.friendlyName()));

            return false;
        }

        bool found = false;
        for(int i = 0; i < m_deviceControllers.size(); ++i)
        {
            if (m_deviceControllers.at(i).first == root)
            {
                delete m_deviceControllers.at(i).second;
                m_deviceControllers.removeAt(i);
                found = true;
                break;
            }
        }

        delete root;
        Q_ASSERT(found);

        HLOG_DBG(QString("Root device [%1] removed. Current device count is %2").arg(
            devInfo.friendlyName(), QString::number(m_rootDevices.size())));

        return true;
    }

    QUrl seekIcon(Device* device, const QString& iconUrl)
    {
        Q_ASSERT(device);

        QList<QUrl> icons = device->info().icons();
        for (qint32 i = 0; i < icons.size(); ++i)
        {
            if (compareUrls(icons[i], iconUrl))
            {
                return icons[i];
            }
        }

        QList<Device*> devices = device->embeddedDevices();
        foreach(Device* device, devices)
        {
            QUrl icon = seekIcon(device, iconUrl);
            if (!icon.isEmpty() && icon.isValid())
            {
                return icon;
            }
        }

        return QUrl();
    }

    Service* searchServiceByScpdUrl(Device* device, const QUrl& scpdUrl) const
    {
        QList<Device*> tmp; tmp.push_back(device);
        return seekService(
            tmp,
            MatchFunctor<Service, ScpdUrlTester<Service> >(scpdUrl));
    }

    Service* searchServiceByScpdUrl(const QUrl& scpdUrl) const
    {
        return seekService(
            m_rootDevices,
            MatchFunctor<Service, ScpdUrlTester<Service> >(scpdUrl));
    }

    Service* searchServiceByControlUrl(
        Device* device, const QUrl& controlUrl) const
    {
        QList<Device*> tmp; tmp.push_back(device);
        return seekService(
            tmp,
            MatchFunctor<Service, ControlUrlTester<Service> >(controlUrl));
    }

    Service* searchServiceByControlUrl(const QUrl& controlUrl) const
    {
        return seekService(
            m_rootDevices,
            MatchFunctor<Service, ControlUrlTester<Service> >(controlUrl));
    }

    Service* searchServiceByEventUrl(Device* device, const QUrl& eventUrl) const
    {
        QList<Device*> tmp; tmp.push_back(device);
        return seekService(
            tmp,
            MatchFunctor<Service, EventUrlTester<Service> >(eventUrl));
    }

    Service* searchServiceByEventUrl(const QUrl& eventUrl) const
    {
        return seekService(
            m_rootDevices,
            MatchFunctor<Service, EventUrlTester<Service> >(eventUrl));
    }

    template<typename T>
    QList<T*> rootDevices() const
    {
        QList<T*> retVal;
        foreach(Device* dev, m_rootDevices)
        {
            retVal.append(dev);
        }
        return retVal;
    }

    const QList<Device*>& rootDevices() const
    {
        return m_rootDevices;
    }

    QList<Controller*> controllers() const
    {
        QList<Controller*> retVal;
        for(int i = 0; i < m_deviceControllers.size(); ++i)
        {
            retVal.append(m_deviceControllers.at(i).second);
        }
        return retVal;
    }
};

}
}

#endif /* HDEVICESTORAGE_H_ */
