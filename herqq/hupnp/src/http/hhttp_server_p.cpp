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

#include "hhttp_server_p.h"
#include "hhttp_utils_p.h"
#include "hhttp_header_p.h"
#include "hhttp_asynchandler_p.h"
#include "hhttp_messaginginfo_p.h"
#include "hhttp_messagecreator_p.h"

#include "../general/hlogger_p.h"
#include "../utils/hmisc_utils_p.h"

#include "../socket/hendpoint.h"
#include "../general/hupnp_global_p.h"
#include "../devicehosting/messages/hcontrol_messages_p.h"
#include "../devicehosting/messages/hevent_messages_p.h"

#include <QtCore/QUrl>
#include <QtCore/QTime>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtNetwork/QTcpSocket>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HHttpServer::Server
 ******************************************************************************/
HHttpServer::Server::Server(HHttpServer* owner) :
    QTcpServer(owner), m_owner(owner)
{
}

void HHttpServer::Server::incomingConnection(qint32 socketDescriptor)
{
    m_owner->processRequest(socketDescriptor);
}

/*******************************************************************************
 * HHttpServer
 ******************************************************************************/
HHttpServer::HHttpServer(const QByteArray& loggingIdentifier, QObject* parent) :
    QObject(parent),
        m_servers(),
        m_loggingIdentifier(loggingIdentifier),
        m_httpHandler(new HHttpAsyncHandler(m_loggingIdentifier, this)),
        m_chunkedInfo(),
        m_maxBytesToLoad(1024*1024*5) // TODO make this configurable
{
    bool ok = connect(
        m_httpHandler, SIGNAL(msgIoComplete(HHttpAsyncOperation*)),
        this, SLOT(msgIoComplete(HHttpAsyncOperation*)));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

HHttpServer::~HHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    close();
    qDeleteAll(m_servers);
}

void HHttpServer::processRequest(HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HMessagingInfo* mi = op->messagingInfo();

    const HHttpRequestHeader* hdr =
        static_cast<const HHttpRequestHeader*>(op->headerRead());

    if (!hdr->isValid())
    {
        m_httpHandler->send(
            op->takeMessagingInfo(),
            HHttpMessageCreator::createResponse(BadRequest, *mi));

        return;
    }

    QString host = hdr->value("HOST");
    if (host.isEmpty())
    {
        m_httpHandler->send(
            op->takeMessagingInfo(),
            HHttpMessageCreator::createResponse(BadRequest, *mi));

        return;
    }

    mi->setHostInfo(host);
    mi->setKeepAlive(HHttpUtils::keepAlive(*hdr));

    QString method = hdr->method();
    if (method.compare("GET", Qt::CaseInsensitive) == 0)
    {
        processGet(op->takeMessagingInfo(), *hdr);
    }
    else if (method.compare("HEAD"), Qt::CaseInsensitive)
    {
        processHead(op->takeMessagingInfo(), *hdr);
    }
    else if (method.compare("POST", Qt::CaseInsensitive) == 0)
    {
        processPost(op->takeMessagingInfo(), *hdr, op->dataRead());
    }
    else if (method.compare("NOTIFY", Qt::CaseInsensitive) == 0)
    {
        processNotifyMessage(op->takeMessagingInfo(), *hdr, op->dataRead());
    }
    else if (method.compare("SUBSCRIBE", Qt::CaseInsensitive) == 0)
    {
        processSubscription(op->takeMessagingInfo(), *hdr);
    }
    else if (method.compare("UNSUBSCRIBE", Qt::CaseInsensitive) == 0)
    {
        processUnsubscription(op->takeMessagingInfo(), *hdr);
    }
    else
    {
        m_httpHandler->send(
            op->takeMessagingInfo(),
            HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
    }
}

void HHttpServer::processResponse(HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (op->state() == HHttpAsyncOperation::Failed)
    {
        HLOG_DBG(QString("HTTP failure: [%1]").arg(
            op->messagingInfo()->lastErrorDescription()));
    }

    incomingResponse(op);
}

void HHttpServer::msgIoComplete(HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    op->deleteLater();

    HMessagingInfo* mi = op->messagingInfo();
    if (op->state() == HHttpAsyncOperation::Failed)
    {
        HLOG_DBG(QString("HTTP failure: [%1]").arg(mi->lastErrorDescription()));
        return;
    }

    switch(op->opType())
    {
    case HHttpAsyncOperation::SendOnly:
        if (sendComplete(op))
        {
            if (mi->keepAlive() && mi->socket().state() == QTcpSocket::ConnectedState)
            {
                if (!m_httpHandler->receive(op->takeMessagingInfo(), true))
                {
                    HLOG_WARN(QString(
                        "Failed to read data from: [%1]. Disconnecting.").arg(
                            peerAsStr(mi->socket())));
                }
            }
        }
        break;

    case HHttpAsyncOperation::ReceiveRequest:
        processRequest(op);
        break;

    case HHttpAsyncOperation::MsgIO:
    case HHttpAsyncOperation::ReceiveResponse:
        processResponse(op);
        break;

    default:
        Q_ASSERT(false);
    }
}

void HHttpServer::processRequest(qint32 socketDescriptor)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QTcpSocket* client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);

    QString peer = peerAsStr(*client);
    HLOG_DBG(QString("Incoming connection from [%1]").arg(peer));

    HMessagingInfo* mi = new HMessagingInfo(qMakePair(client, true));
    mi->setChunkedInfo(m_chunkedInfo);
    mi->setServerInfo(HSysInfo::instance().herqqProductTokens());
    if (!m_httpHandler->receive(mi, true))
    {
        HLOG_WARN(QString(
            "Failed to read data from: [%1]. Disconnecting.").arg(peer));
    }
}

void HHttpServer::processNotifyMessage(
    HMessagingInfo* mi, const HHttpRequestHeader& hdr, const QByteArray& body)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HNotifyRequest nreq;
    HNotifyRequest::RetVal retVal =
        static_cast<HNotifyRequest::RetVal>(
            HHttpMessageCreator::create(hdr, body, nreq));

    switch(retVal)
    {
    case HNotifyRequest::Success:
        break;

    case HNotifyRequest::PreConditionFailed:
        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(PreconditionFailed, *mi));
        return;

    case HNotifyRequest::InvalidContents:
    case HNotifyRequest::InvalidSequenceNr:
        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(BadRequest, *mi));
        return;

    default:
        retVal = HNotifyRequest::BadRequest;
        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(BadRequest, *mi));
        return;
    }

    HLOG_DBG("Dispatching event notification.");
    incomingNotifyMessage(mi, nreq);
}

void HHttpServer::processGet(
    HMessagingInfo* mi, const HHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_DBG("Dispatching unknown GET request.");
    incomingUnknownGetRequest(mi, requestHdr);
}

void HHttpServer::processHead(
    HMessagingInfo* mi, const HHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_DBG("Dispatching unknown HEAD request.");
    incomingUnknownHeadRequest(mi, requestHdr);
}

void HHttpServer::processPost(
    HMessagingInfo* mi, const HHttpRequestHeader& requestHdr,
    const QByteArray& body)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString soapAction  = requestHdr.value("SOAPACTION");
    if (soapAction.indexOf("#") <= 0)
    {
        HLOG_DBG("Dispatching unknown POST request.");
        incomingUnknownPostRequest(mi, requestHdr, body);
        return;
    }

    QString actionName = soapAction.mid(soapAction.indexOf("#"));
    if (actionName.isEmpty())
    {
        HLOG_DBG("Dispatching unknown POST request.");
        incomingUnknownPostRequest(mi, requestHdr, body);
        return;
    }

    QtSoapMessage soapMsg;
    if (!soapMsg.setContent(body))
    {
        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(BadRequest, *mi));
        return;
    }

    QString controlUrl = requestHdr.path().simplified();
    if (controlUrl.isEmpty())
    {
        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(BadRequest, *mi));
        return;
    }

    HInvokeActionRequest iareq(soapAction, soapMsg, controlUrl);
    HLOG_DBG("Dispatching control request.");
    incomingControlRequest(mi, iareq);
}

void HHttpServer::processSubscription(
    HMessagingInfo* mi, const HHttpRequestHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HSubscribeRequest sreq;
    HSubscribeRequest::RetVal retVal =
        static_cast<HSubscribeRequest::RetVal>(
            HHttpMessageCreator::create(hdr, sreq));

    switch(retVal)
    {
    case HSubscribeRequest::Success:
        break;

    case HSubscribeRequest::PreConditionFailed:
        mi->setKeepAlive(false);
        m_httpHandler->send(
            mi, HHttpMessageCreator::createResponse(PreconditionFailed, *mi));

        break;

    case HSubscribeRequest::IncompatibleHeaders:
        mi->setKeepAlive(false);
        m_httpHandler->send(mi,
            HHttpMessageCreator::createResponse(IncompatibleHeaderFields, *mi));
        return;

    case HSubscribeRequest::BadRequest:
    default:
        mi->setKeepAlive(false);
        m_httpHandler->send(
            mi, HHttpMessageCreator::createResponse(BadRequest, *mi));

        return;
    }

    HLOG_DBG("Dispatching subscription request.");
    incomingSubscriptionRequest(mi, sreq);
}

void HHttpServer::processUnsubscription(
    HMessagingInfo* mi, const HHttpRequestHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HUnsubscribeRequest usreq;
    HUnsubscribeRequest::RetVal retVal =
        static_cast<HUnsubscribeRequest::RetVal>(
            HHttpMessageCreator::create(hdr, usreq));

    switch(retVal)
    {
    case HUnsubscribeRequest::Success:
        break;

    case HUnsubscribeRequest::IncompatibleHeaders:
        mi->setKeepAlive(false);
        m_httpHandler->send(mi,
            HHttpMessageCreator::createResponse(IncompatibleHeaderFields, *mi));

        return;

    case HUnsubscribeRequest::PreConditionFailed:
        mi->setKeepAlive(false);
        m_httpHandler->send(mi,
            HHttpMessageCreator::createResponse(PreconditionFailed, *mi));

        return;

    default:
        mi->setKeepAlive(false);
        m_httpHandler->send(
            mi, HHttpMessageCreator::createResponse(BadRequest, *mi));

        return;
    }

    HLOG_DBG("Dispatching unsubscription request.");
    incomingUnsubscriptionRequest(mi, usreq);
}

bool HHttpServer::setupIface(const HEndpoint& ep)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHostAddress ha = ep.hostAddress();
    if (ha == QHostAddress::Null || ha == QHostAddress::Any ||
        ha == QHostAddress::Broadcast)
    {
        return false;
    }

    QScopedPointer<Server> server(new Server(this));
    bool b = server->listen(ha, ep.portNumber());
    if (b)
    {
        HLOG_INFO(QString("HTTP server bound to %1:%2").arg(
            server->serverAddress().toString(),
            QString::number(server->serverPort())));

        m_servers.append(server.take());
    }
    else
    {
        HLOG_INFO(QString("Failed to bind HTTP server to %1").arg(
            ep.hostAddress().toString()));
    }

    return b;
}

void HHttpServer::incomingSubscriptionRequest(
    HMessagingInfo* mi, const HSubscribeRequest&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingSubscriptionRequest] implementation, which does nothing.");
    mi->setKeepAlive(false);
    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
}

void HHttpServer::incomingUnsubscriptionRequest(
    HMessagingInfo* mi, const HUnsubscribeRequest&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingUnsubscriptionRequest] implementation, which does nothing.");
    mi->setKeepAlive(false);
    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
}

void HHttpServer::incomingControlRequest(
    HMessagingInfo* mi, const HInvokeActionRequest&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingControlRequest] implementation, which does nothing.");
    mi->setKeepAlive(false);
    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
}

void HHttpServer::incomingNotifyMessage(
    HMessagingInfo* mi, const HNotifyRequest&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingNotifyMessage] implementation, which does nothing.");
    mi->setKeepAlive(false);
    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
}

void HHttpServer::incomingUnknownHeadRequest(
    HMessagingInfo* mi, const HHttpRequestHeader&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingUnknownHeadRequest] implementation, which does nothing.");
    mi->setKeepAlive(false);
    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
}

void HHttpServer::incomingUnknownGetRequest(
    HMessagingInfo* mi, const HHttpRequestHeader&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingUnknownGetRequest] implementation, which does nothing.");
    mi->setKeepAlive(false);
    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
}

void HHttpServer::incomingUnknownPostRequest(
    HMessagingInfo* mi, const HHttpRequestHeader&, const QByteArray&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingUnknownGetRequest] implementation, which does nothing.");
    mi->setKeepAlive(false);
    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(MethotNotAllowed, *mi));
}

void HHttpServer::incomingResponse(
    HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_WARN("Calling default [incomingResponse] implementation, which does nothing.");
    op->messagingInfo()->setKeepAlive(false);
}

bool HHttpServer::sendComplete(HHttpAsyncOperation*)
{
    return true;
}

QList<QUrl> HHttpServer::rootUrls() const
{
    QList<QUrl> retVal;
    foreach(const Server* server, m_servers)
    {
        QUrl url(QString("http://%1:%2").arg(
            server->serverAddress().toString(),
            QString::number(server->serverPort())));

        retVal.append(url);
    }

    return retVal;
}

QUrl HHttpServer::rootUrl(const QHostAddress& ha) const
{
    foreach(const Server* server, m_servers)
    {
        if (ha == server->serverAddress())
        {
            QUrl url(QString("http://%1:%2").arg(
                server->serverAddress().toString(),
                QString::number(server->serverPort())));

            return url;
        }
    }

    return QUrl();
}

QList<HEndpoint> HHttpServer::endpoints() const
{
    QList<HEndpoint> retVal;
    foreach(const Server* server, m_servers)
    {
        retVal.append(HEndpoint(server->serverAddress(), server->serverPort()));
    }
    return retVal;
}

bool HHttpServer::init()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    if (isInitialized())
    {
        return false;
    }

    QHostAddress ha = findBindableHostAddress();
    return setupIface(HEndpoint(ha));
}

bool HHttpServer::init(const HEndpoint& ep)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    if (isInitialized())
    {
        return false;
    }

    return setupIface(ep);
}

bool HHttpServer::init(const QList<HEndpoint>& eps)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    if (isInitialized())
    {
        return false;
    }

    bool b = false;
    foreach(const HEndpoint& ep, eps)
    {
        b = setupIface(ep);
        if (!b)
        {
            qDeleteAll(m_servers);
            m_servers.clear();
            return false;
        }
    }

    return true;
}

bool HHttpServer::isInitialized() const
{
    return m_servers.size();
}

void HHttpServer::close()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
            "The HTTP Server has to be shutdown in the thread in which "
            "it is currently located.");

    foreach(Server* server, m_servers)
    {
        if (server->isListening())
        {
            server->close();
        }
    }
}

qint32 HHttpServer::maxBytesToLoad() const
{
    return m_maxBytesToLoad;
}

}
}
