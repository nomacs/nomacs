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

#ifndef HEVENT_SUBSCRIPTION_H_
#define HEVENT_SUBSCRIPTION_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../messages/hsid_p.h"
#include "../messages/hevent_messages_p.h"
#include "../../http/hhttp_asynchandler_p.h"

#include "../../general/hupnp_defs.h"
#include "../../general/hupnp_fwd.h"

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QByteArray>

#include <QtNetwork/QTcpSocket>

namespace Herqq
{

namespace Upnp
{

class HHttpAsyncOperation;

//
// This class represents and maintains a subscription to a service instantiated on the
// device host (server) side.
//
class HEventSubscription :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HEventSubscription)

private:

    const QByteArray m_loggingIdentifier;

    const QUuid m_randomIdentifier;
    // identifies the service subscription. used in the callback url

    QList<QUrl> m_deviceLocations;
    // the URLs of the device where the desired service is located

    qint32 m_nextLocationToTry;
    // index of the device location URL that has been tried / used previously
    // the URL identified by this index will be used until communication to the
    // URL fails. At that time the index is incremented if there are more
    // device locations to try.

    QUrl m_eventUrl;
    // the URL that is currently used in HTTP messaging

    qint32 m_connectErrorCount;

    HSid m_sid;
    // the unique identifier of the subscription created by the upnp device

    qint32 m_seq;
    // the sequence number which is incremented upon each notify

    HTimeout m_desiredTimeout;
    // the desired timeout for the subscription

    HTimeout m_timeout;
    // the actual timeout of the subscription. this is received from the device
    // upon successful subscription. if no error occurs, the subscription will
    // be renewed before the specified timeout elapses.

    QTimer m_subscriptionTimer;
    // used to signal the time when the subscription should be renewed

    QTimer m_announcementTimer;
    bool m_announcementTimedOut;

    HClientService* m_service;
    // the target service of the subscription

    QUrl m_serverRootUrl;
    // the URL of the server that relays the notifications to this instance.
    // this is used in subscription requests to tell the upnp device where the
    // notifications are to be sent

    HHttpAsyncHandler m_http;
    // the class used to perform HTTP messaging

    enum OperationType
    {
        Op_None = 0,
        Op_Subscribe,
        Op_Renew,
        Op_Unsubscribe
    };

    QTcpSocket m_socket;
    // socket for the messaging

    OperationType m_currentOpType;
    OperationType m_nextOpType;

    bool m_subscribed;

    QList<HNotifyRequest> m_queuedNotifications;

private Q_SLOTS:

    void subscriptionTimeout();
    void announcementTimeout();

    void connected();
    void msgIoComplete(HHttpAsyncOperation*);

    void error(QAbstractSocket::SocketError);

private:

    bool connectToDevice(qint32 msecsToWait=0);
    void subscribe_done(HHttpAsyncOperation*);
    void renewSubscription_done(HHttpAsyncOperation*);
    void unsubscribe_done(HHttpAsyncOperation*);

    void runNextOp();
    void resubscribe();
    void renewSubscription();
    StatusCode processNotify(const HNotifyRequest&);

Q_SIGNALS:

    void subscribed(HEventSubscription*);
    void subscriptionFailed(HEventSubscription*);
    void unsubscribed(HEventSubscription*);

public:

    enum SubscriptionStatus
    {
        Status_Unsubscribed = 0,
        Status_Subscribing,
        Status_Subscribed
    };

    HEventSubscription(
        const QByteArray& loggingIdentifier,
        HClientService* service,
        const QUrl& serverRootUrl,
        const HTimeout& desiredTimeout,
        QObject* parent = 0);

    virtual ~HEventSubscription();

    inline QUuid id() const { return m_randomIdentifier ; }
    inline HClientService* service() const { return m_service; }

    void subscribe();
    void unsubscribe(qint32 msecsToWait=0);
    void resetSubscription();
    StatusCode onNotify(const HNotifyRequest&);

    SubscriptionStatus subscriptionStatus() const;
};


}
}

#endif /* HEVENT_SUBSCRIPTION_H_ */
