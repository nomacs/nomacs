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

#ifndef HSSDP_P_H_
#define HSSDP_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hssdp.h"
#include "hdiscovery_messages.h"

#include "../socket/hendpoint.h"
#include "../general/hupnp_defs.h"
#include "../http/hhttp_header_p.h"
#include "../socket/hmulticast_socket.h"

#include <QtCore/QByteArray>

class QUrl;
class QString;
class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HSsdp;

//
// Implementation details of HSsdp
//
class HSsdpPrivate
{
H_DISABLE_COPY(HSsdpPrivate)

private:

    bool parseCacheControl(const QString&, qint32*);
    bool checkHost(const QString& host);

    bool parseDiscoveryResponse(const HHttpResponseHeader&, HDiscoveryResponse*);
    bool parseDiscoveryRequest (const HHttpRequestHeader&, HDiscoveryRequest*);
    bool parseDeviceAvailable  (const HHttpRequestHeader&, HResourceAvailable*);
    bool parseDeviceUnavailable(const HHttpRequestHeader&, HResourceUnavailable*);
    bool parseDeviceUpdate     (const HHttpRequestHeader&, HResourceUpdate*);

    void clear();

public: // attributes

    QByteArray m_loggingIdentifier;

    HMulticastSocket* m_multicastSocket;
    // for listening multicast messages

    QUdpSocket* m_unicastSocket;
    // for sending datagrams and listening messages directed to this instance

    HSsdp* q_ptr;

    HSsdp::AllowedMessages m_allowedMessages;

    QString m_lastError;

public: // methods

    HSsdpPrivate(
        HSsdp* qptr, const QByteArray& loggingIdentifier = QByteArray());

    ~HSsdpPrivate();

    bool init(const QHostAddress& addressToBind);

    inline bool isInitialized() const
    {
        return m_unicastSocket && m_multicastSocket;
    }

    void processNotify(const QString& msg, const HEndpoint& source);
    void processSearch(const QString& msg, const HEndpoint& source,
                       const HEndpoint& destination);

    void processResponse(const QString& msg, const HEndpoint& source);

    bool send(const QByteArray& data, const HEndpoint& receiver);

    void messageReceived(QUdpSocket*, const HEndpoint* = 0);
};

}
}

#endif /* HSSDP_P_H_ */
