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

#ifndef HEVENT_SUBSCRIBER_P_H_
#define HEVENT_SUBSCRIBER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../messages/hevent_messages_p.h"
#include "../../http/hhttp_asynchandler_p.h"

#include <QtCore/QQueue>
#include <QtCore/QTimer>
#include <QtCore/QObject>

class QByteArray;
class QTcpSocket;

namespace Herqq
{

namespace Upnp
{

class HMessagingInfo;

//
// Internal class used to maintain information about a single event subscriber.
//
class HServiceEventSubscriber :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HServiceEventSubscriber)

private: // attributes

    HServerService* m_service;
    QUrl m_location;
    HSid m_sid;
    quint32 m_seq;
    HTimeout m_timeout;
    QTimer m_timer;
    HHttpAsyncHandler m_asyncHttp;

    QScopedPointer<QTcpSocket> m_socket;
    QQueue<QByteArray> m_messagesToSend;

    bool m_expired;

    const QByteArray m_loggingIdentifier;

    bool connectToHost();

private Q_SLOTS:

    void send();
    void msgIoComplete(HHttpAsyncOperation*);
    void subscriptionTimeout();

private:

    bool send(HMessagingInfo* mi);

public:

    HServiceEventSubscriber(
        const QByteArray& loggingIdentifier,
        HServerService* service, const QUrl location, const HTimeout& timeout,
        QObject* parent = 0);

    virtual ~HServiceEventSubscriber();

    void notify(const QByteArray& msgBody);
    bool initialNotify(const QByteArray& msgBody, HMessagingInfo* = 0);

    bool isInterested(const HServerService* service) const;

    inline QUrl      location() const { return m_location; }
    inline HSid      sid     () const { return m_sid;      }
    inline quint32   seq     () const { return m_seq;      }
    inline HTimeout  timeout () const { return m_timeout;  }
    inline HServerService* service () const { return m_service;  }
    inline bool      expired () const { return m_expired;  }

    void renew(const HTimeout&);
};

}
}

#endif /* HEVENT_SUBSCRIBER_P_H_ */
