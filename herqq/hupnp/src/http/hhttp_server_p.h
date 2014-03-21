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

#ifndef HHTTP_SERVER_P_H_
#define HHTTP_SERVER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/private/hhttp_asynchandler_p.h>
#include <HUpnpCore/private/hhttp_messaginginfo_p.h>

#include <QtNetwork/QTcpServer>

class QUrl;
class QString;
class QTcpSocket;

namespace Herqq
{

namespace Upnp
{

class HNotifyRequest;
class HSubscribeRequest;
class HUnsubscribeRequest;
class HInvokeActionRequest;

//
// Private class for handling HTTP server duties needed in UPnP messaging
//
class H_UPNP_CORE_EXPORT HHttpServer :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HHttpServer)
friend class Server;

private:

    class Server :
        public QTcpServer
    {
    H_DISABLE_COPY(Server)

    private:
        HHttpServer* m_owner;

    protected:
        virtual void incomingConnection(qint32 socketDescriptor);

    public:
        Server(HHttpServer* owner);
    };

private Q_SLOTS:

    void msgIoComplete(HHttpAsyncOperation* op);

private:

    QList<Server*> m_servers;

protected:

    const QByteArray m_loggingIdentifier;
    HHttpAsyncHandler* m_httpHandler;
    HChunkedInfo m_chunkedInfo;
    qint32 m_maxBytesToLoad;

private:

    void processRequest(HHttpAsyncOperation*);
    void processResponse(HHttpAsyncOperation*);

    void processRequest(qint32 socketDescriptor);

    void processNotifyMessage(
        HMessagingInfo*, const HHttpRequestHeader&, const QByteArray& body);

    void processGet (
        HMessagingInfo*, const HHttpRequestHeader&);

    void processHead(
        HMessagingInfo*, const HHttpRequestHeader&);

    void processPost(
        HMessagingInfo*, const HHttpRequestHeader&, const QByteArray& body);

    void processSubscription(
        HMessagingInfo*, const HHttpRequestHeader&);

    void processUnsubscription(
        HMessagingInfo*, const HHttpRequestHeader&);

    bool setupIface(const HEndpoint&);

protected:

    virtual void incomingSubscriptionRequest(
        HMessagingInfo*, const HSubscribeRequest&);

    virtual void incomingUnsubscriptionRequest(
        HMessagingInfo*, const HUnsubscribeRequest&);

    virtual void incomingControlRequest(
        HMessagingInfo*, const HInvokeActionRequest&);

    virtual void incomingNotifyMessage(
        HMessagingInfo*, const HNotifyRequest&);

    virtual void incomingUnknownHeadRequest(
        HMessagingInfo*, const HHttpRequestHeader&);

    virtual void incomingUnknownGetRequest(
        HMessagingInfo*, const HHttpRequestHeader&);

    virtual void incomingUnknownPostRequest(
        HMessagingInfo*, const HHttpRequestHeader&, const QByteArray& body);

    virtual void incomingResponse(HHttpAsyncOperation*);

    virtual bool sendComplete(HHttpAsyncOperation*);

public:

    HHttpServer(const QByteArray& loggingIdentifier, QObject* parent = 0);
    virtual ~HHttpServer();

    QList<QUrl> rootUrls() const;
    QUrl rootUrl(const QHostAddress&) const;
    QList<HEndpoint> endpoints() const;
    inline qint32 endpointCount() const { return m_servers.size(); }

    bool init();
    bool init(const HEndpoint&);
    bool init(const QList<HEndpoint>&);
    bool isInitialized() const;
    void close();

    qint32 maxBytesToLoad() const;
};

}
}

#endif /* HHTTP_SERVER_P_H_ */
