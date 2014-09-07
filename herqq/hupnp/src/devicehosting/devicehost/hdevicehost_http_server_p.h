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

#ifndef HDEVICEHOST_HTTP_SERVER_H_
#define HDEVICEHOST_HTTP_SERVER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hevent_notifier_p.h"
#include "hserverdevicecontroller_p.h"

#include "../hdevicestorage_p.h"
#include "../messages/hevent_messages_p.h"

#include "../../http/hhttp_server_p.h"

#include <QtCore/QPointer>

namespace Herqq
{

namespace Upnp
{

//
//
//
class HOpInfo
{
public:

    HServerService* m_service;
    HSubscribeRequest m_req;
    HServiceEventSubscriber* m_subscriber;

    HOpInfo() :
        m_service(0), m_req(), m_subscriber(0)
    {
    }

    HOpInfo(HServerService* service, const HSubscribeRequest& sreq,
        HServiceEventSubscriber* subscriber) :
            m_service(service), m_req(sreq), m_subscriber(subscriber)
    {
    }

    inline bool isValid() const { return m_service; }
};

//
// Internal class that provides minimal HTTP server functionality for the needs of
// Device Host
//
class HDeviceHostHttpServer :
    public HHttpServer
{
Q_OBJECT
H_DISABLE_COPY(HDeviceHostHttpServer)

private:

    HDeviceStorage<HServerDevice, HServerService, HServerDeviceController>& m_deviceStorage;
    HEventNotifier& m_eventNotifier;
    QString m_ddPostFix;

    QList<QPair<QPointer<HHttpAsyncOperation>, HOpInfo> > m_ops;

protected:

    virtual void incomingSubscriptionRequest(
        HMessagingInfo*, const HSubscribeRequest&);

    virtual void incomingUnsubscriptionRequest(
        HMessagingInfo*, const HUnsubscribeRequest&);

    virtual void incomingControlRequest(
        HMessagingInfo*, const HInvokeActionRequest&);

    virtual void incomingUnknownGetRequest(
        HMessagingInfo*, const HHttpRequestHeader&);

    virtual bool sendComplete(HHttpAsyncOperation*);

public:

    HDeviceHostHttpServer(
        const QByteArray& loggingId, const QString& ddPostFix,
        HDeviceStorage<HServerDevice, HServerService, HServerDeviceController>&, HEventNotifier&,
        QObject* parent = 0);

    virtual ~HDeviceHostHttpServer();
};

}
}

#endif /* HDEVICEHOST_HTTP_SERVER_H_ */
