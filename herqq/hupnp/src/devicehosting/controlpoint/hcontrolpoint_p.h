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

#ifndef HCONTROL_POINT_P_H_
#define HCONTROL_POINT_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hcontrolpoint.h"
#include "hdevicebuild_p.h"
#include "hevent_subscriptionmanager_p.h"

#include "../hdevicestorage_p.h"

#include "../../devicemodel/client/hclientdevice.h"
#include "../../devicemodel/client/hclientservice.h"

#include "../../ssdp/hssdp.h"
#include "../../ssdp/hssdp_p.h"
#include "../../http/hhttp_server_p.h"
#include "../../ssdp/hdiscovery_messages.h"

#include "../../utils/hthreadpool_p.h"

#include <QtCore/QUuid>
#include <QtCore/QScopedPointer>
#include <QtNetwork/QNetworkAccessManager>

namespace Herqq
{

namespace Upnp
{

class HControlPointPrivate;

//
// The HTTP server the control point uses to receive event notifications.
//
class ControlPointHttpServer :
    public HHttpServer
{
Q_OBJECT
H_DISABLE_COPY(ControlPointHttpServer)

private:

    HControlPointPrivate* m_owner;

protected:

    virtual void incomingNotifyMessage(HMessagingInfo*, const HNotifyRequest&);

public:

    explicit ControlPointHttpServer(HControlPointPrivate*);
    virtual ~ControlPointHttpServer();
};

//
//
//
class HControlPointSsdpHandler :
    public HSsdp
{
H_DISABLE_COPY(HControlPointSsdpHandler)

private:

    HControlPointPrivate* m_owner;

protected:

    virtual bool incomingDiscoveryResponse(
        const HDiscoveryResponse&, const HEndpoint& source);

    virtual bool incomingDeviceAvailableAnnouncement(
        const HResourceAvailable&, const HEndpoint& source);

    virtual bool incomingDeviceUnavailableAnnouncement(
        const HResourceUnavailable&, const HEndpoint& source);

public:

    HControlPointSsdpHandler(HControlPointPrivate*);
    virtual ~HControlPointSsdpHandler();
};

//
// Implementation details of HControlPoint
//
class H_UPNP_CORE_EXPORT HControlPointPrivate :
    public QObject
{
Q_OBJECT
H_DECLARE_PUBLIC(HControlPoint)
H_DISABLE_COPY(HControlPointPrivate)
friend class DeviceBuildTask;
friend class HControlPointSsdpHandler;

private:

    DeviceBuildTasks m_deviceBuildTasks;
    // this is accessed only from the thread in which all the HUpnp objects live.

private Q_SLOTS:

    void deviceModelBuildDone(const Herqq::Upnp::HUdn&);

private:

    bool addRootDevice(HDefaultClientDevice*);
    void subscribeToEvents(HDefaultClientDevice*);

    void processDeviceOnline(HDefaultClientDevice*, bool newDevice);

    bool processDeviceOffline(
        const HResourceUnavailable&, const HEndpoint& source,
        HControlPointSsdpHandler* origin);

    template<class Msg>
    bool processDeviceDiscovery(
        const Msg&, const HEndpoint& source, HControlPointSsdpHandler* origin);

    template<class Msg>
    bool shouldFetch(const Msg&);

private Q_SLOTS:

    void deviceExpired(HDefaultClientDevice* source);
    void unsubscribed(Herqq::Upnp::HClientService*);

public:

    const QByteArray m_loggingIdentifier;
    // the prefix shown before the actual log output

    QScopedPointer<HControlPointConfiguration> m_configuration;
    QList<QPair<quint32, HControlPointSsdpHandler*> > m_ssdps;
    // the int is a ipv4 network address

    ControlPointHttpServer* m_server;
    HEventSubscriptionManager* m_eventSubscriber;

    HControlPoint::ControlPointError m_lastError;

    QString m_lastErrorDescription;
    // description of the error that occurred last

    HControlPoint* q_ptr;

    QNetworkAccessManager* m_nam;

    enum InitState
    {
        Exiting = -1,
        Uninitialized = 0,
        Initializing = 1,
        Initialized = 2
    };

    volatile InitState m_state;

    HThreadPool* m_threadPool;

    HDeviceStorage<HClientDevice, HClientService> m_deviceStorage;

    HControlPointPrivate();
    virtual ~HControlPointPrivate();

    HDefaultClientDevice* buildDevice(
        const QUrl& deviceLocation, qint32 maxAge, QString* err);
};

}
}

#endif /* HCONTROL_POINT_P_H_ */
