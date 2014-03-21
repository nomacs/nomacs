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

#ifndef HEVENT_NOTIFIER_P_H_
#define HEVENT_NOTIFIER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../http/hhttp_p.h"
#include "../../general/hupnp_fwd.h"
#include "../../general/hupnp_defs.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

class HSid;
class HTimeout;
class HMessagingInfo;
class HSubscribeRequest;
class HUnsubscribeRequest;
class HServiceEventSubscriber;

//
// Internal class used to notify event subscribers of events.
//
class HEventNotifier :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HEventNotifier)

private: // attributes

    const QByteArray m_loggingIdentifier;
    // prefix for logging

    QList<HServiceEventSubscriber*> m_subscribers;

    HDeviceHostConfiguration& m_configuration;

private: // methods

    HTimeout getSubscriptionTimeout(const HSubscribeRequest&);

private Q_SLOTS:

    void stateChanged(const Herqq::Upnp::HServerService* source);

public:

    HEventNotifier(
        const QByteArray& loggingIdentifier,
        HDeviceHostConfiguration&,
        QObject* parent);

    virtual ~HEventNotifier();

    StatusCode addSubscriber(HServerService*, const HSubscribeRequest&, HSid*);

    bool removeSubscriber(const HUnsubscribeRequest&);
    StatusCode renewSubscription(const HSubscribeRequest&, HSid*);
    HServiceEventSubscriber* remoteClient(const HSid&) const;

    void initialNotify(HServiceEventSubscriber*, HMessagingInfo*);
};

}
}

#endif /* HEVENT_NOTIFIER_P_H_ */
