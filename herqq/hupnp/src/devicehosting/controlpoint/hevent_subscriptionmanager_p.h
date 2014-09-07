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

#ifndef HEVENT_SUBSCRIPTIONMANAGER_P_H_
#define HEVENT_SUBSCRIPTIONMANAGER_P_H_

#include "hevent_subscription_p.h"
#include "../../general/hupnp_global.h"
#include "../../devicemodel/client/hclientdevice.h"

#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QUuid>
#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

class HControlPointPrivate;

//
//
//
class HEventSubscriptionManager :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HEventSubscriptionManager)

private:

    HControlPointPrivate* m_owner;

    QHash<QUuid, HEventSubscription*> m_subscribtionsByUuid;
    QHash<HUdn, QList<HEventSubscription*>* > m_subscriptionsByUdn;

private:

    HEventSubscription* createSubscription(HClientService*, qint32 timeout);
    QUrl getSuitableHttpServerRootUrl(const QList<QUrl>& deviceLocations);
    // attempts to figure out the most suitable HTTP server URL for one of the
    // device locations specified

public Q_SLOTS:

    void subscribed_slot(HEventSubscription*);
    void subscriptionFailed_slot(HEventSubscription*);
    void unsubscribed(HEventSubscription*);

Q_SIGNALS:

    void subscribed(Herqq::Upnp::HClientService*);
    void subscriptionFailed(Herqq::Upnp::HClientService*);
    void unsubscribed(Herqq::Upnp::HClientService*);

public:

    HEventSubscriptionManager(HControlPointPrivate*);
    virtual ~HEventSubscriptionManager();

    enum SubscriptionResult
    {
        Sub_Success = 0,
        Sub_AlreadySubscribed = 1,
        Sub_Failed_NotEvented = 2,
    };

    bool subscribe(HClientDevice*, DeviceVisitType, qint32 timeout);
    SubscriptionResult subscribe(HClientService*, qint32 timeout);

    HEventSubscription::SubscriptionStatus subscriptionStatus(
        const HClientService*) const;

    // the unsubscribe flag specifies whether to send unsubscription to the UPnP device
    // if not, the subscription is just reset to default state (in which it does nothing)
    bool cancel(HClientDevice*, DeviceVisitType, bool unsubscribe);
    bool cancel(HClientService*, bool unsubscribe);
    void cancelAll(qint32 msecsToWait);

    bool remove(HClientDevice*, bool recursive);
    bool remove(HClientService*);
    void removeAll();

    StatusCode onNotify(const QUuid& id, const HNotifyRequest& req);
};

}
}

#endif /* HEVENT_SUBSCRIPTIONMANAGER_P_H_ */
