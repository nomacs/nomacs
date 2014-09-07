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

#include "hevent_subscriber_p.h"

#include "../../devicemodel/server/hserverservice.h"
#include "../../dataelements/hserviceid.h"
#include "../../dataelements/hserviceinfo.h"

#include "../../http/hhttp_messagecreator_p.h"

#include "../../general/hlogger_p.h"
#include "../../utils/hsysutils_p.h"

#include <QtNetwork/QTcpSocket>

namespace Herqq
{

namespace Upnp
{

bool HServiceEventSubscriber::send(HMessagingInfo* mi)
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

    if (mi->socket().state() != QTcpSocket::ConnectedState)
    {
        HLOG_WARN(QString(
            "Client [sid: [%1]] is not connected. Failed to notify.").arg(
                m_sid.toString()));

        delete mi;
        return false;
    }

    QByteArray message = m_messagesToSend.head();
    qint32 seq = m_seq++;

    HNotifyRequest req(m_location, m_sid, seq, message);

    QByteArray data = HHttpMessageCreator::create(req, mi);

    HLOG_DBG(QString(
        "Sending notification [seq: %1] to subscriber [%2] @ [%3]").arg(
            QString::number(seq), m_sid.toString(), m_location.toString()));

    HHttpAsyncOperation* oper = m_asyncHttp.msgIo(mi, data);
    if (!oper)
    {
        // notify failed
        //
        // according to UDA v1.1:
        // "the publisher SHOULD abandon sending this message to the
        // subscriber but MUST keep the subscription active and send future event
        // messages to the subscriber until the subscription expires or is canceled."

        HLOG_WARN(QString(
            "Could not send notify [seq: %1, sid: %2] to host @ [%3].").arg(
                QString::number(seq), m_sid.toString(),
                m_location.toString()));
    }

    return oper;
}

HServiceEventSubscriber::HServiceEventSubscriber(
    const QByteArray& loggingIdentifier, HServerService* service,
    const QUrl location, const HTimeout& timeout, QObject* parent) :
        QObject(parent),
            m_service(service),
            m_location(location),
            m_sid(QUuid::createUuid()),
            m_seq(0),
            m_timeout(timeout),
            m_timer(this),
            m_asyncHttp(loggingIdentifier, this),
            m_socket(new QTcpSocket(this)),
            m_messagesToSend(),
            m_expired(false),
            m_loggingIdentifier(loggingIdentifier)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(service);
    Q_ASSERT(location.isValid());

    bool ok = connect(
        &m_timer, SIGNAL(timeout()), this, SLOT(subscriptionTimeout()));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        m_socket.data(), SIGNAL(connected()), this, SLOT(send()));

    Q_ASSERT(ok);

    ok = connect(
        &m_asyncHttp, SIGNAL(msgIoComplete(HHttpAsyncOperation*)),
        this, SLOT(msgIoComplete(HHttpAsyncOperation*)));

    Q_ASSERT(ok);

    if (!timeout.isInfinite())
    {
        m_timer.start(timeout.value() * 1000);
    }
}

HServiceEventSubscriber::~HServiceEventSubscriber()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    HLOG_DBG(QString(
        "Subscription from [%1] with SID %2 cancelled").arg(
            m_location.toString(), m_sid.toString()));
}

bool HServiceEventSubscriber::connectToHost()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(QThread::currentThread() == m_socket->thread());
    Q_ASSERT(m_socket.data());

    QTcpSocket::SocketState state = m_socket->state();

    if (state == QTcpSocket::ConnectedState)
    {
        return true;
    }
    else if (state == QTcpSocket::ConnectingState ||
             state == QTcpSocket::HostLookupState)
    {
        return false;
    }

    m_socket->connectToHost(m_location.host(), m_location.port());

    return false;
}

void HServiceEventSubscriber::msgIoComplete(HHttpAsyncOperation* operation)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    operation->deleteLater();

    if (operation->state() == HHttpAsyncOperation::Failed)
    {
        HLOG_WARN(QString(
            "Notification [seq: %1, sid: %2] to host @ [%3] failed: %4.").arg(
                QString::number(m_seq-1),
                m_sid.toString(),
                m_location.toString(),
                operation->messagingInfo()->lastErrorDescription()));

        if (m_seq == 1)
        {
            m_seq--;
            send();
            return;
        }
    }
    else
    {
        HLOG_DBG(QString(
            "Notification [seq: %1] successfully sent to subscriber [%2] @ [%3]").arg(
                QString::number(m_seq-1), m_sid.toString(), m_location.toString()));
    }

    if (!m_messagesToSend.isEmpty())
    {
        m_messagesToSend.dequeue();
    }

    if (!m_messagesToSend.isEmpty())
    {
        send();
    }
}

void HServiceEventSubscriber::send()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_messagesToSend.isEmpty() || !connectToHost() || !m_socket->isValid())
    {
        return;
    }

    QByteArray message = m_messagesToSend.head();
    qint32 seq = m_seq++;

    HMessagingInfo* mi = new HMessagingInfo(*m_socket, false, 10000);
    // timeout specified by UDA v 1.1 is 30 seconds, but that seems absurd
    // in this context. however, if this causes problems change it back.

    HNotifyRequest req(m_location, m_sid, seq, message);

    QByteArray data = HHttpMessageCreator::create(req, mi);

    HLOG_DBG(QString(
        "Sending notification [seq: %1] to subscriber [%2] @ [%3]").arg(
            QString::number(seq), m_sid.toString(), m_location.toString()));

    HHttpAsyncOperation* oper = m_asyncHttp.msgIo(mi, data);
    if (!oper)
    {
        // notify failed
        //
        // according to UDA v1.1:
        // "the publisher SHOULD abandon sending this message to the
        // subscriber but MUST keep the subscription active and send future event
        // messages to the subscriber until the subscription expires or is canceled."

        HLOG_WARN(QString(
            "Could not send notify [seq: %1, sid: %2] to host @ [%3].").arg(
                QString::number(seq), m_sid.toString(),
                m_location.toString()));
    }
}

void HServiceEventSubscriber::subscriptionTimeout()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    m_expired = true;

    if (m_timer.isActive())
    {
        m_timer.stop();
    }

    HLOG_DBG(QString(
        "Subscription from [%1] with SID %2 expired").arg(
            m_location.toString(), m_sid.toString()));
}

bool HServiceEventSubscriber::isInterested(const HServerService* service) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    return !expired() && m_seq && m_service->isEvented() &&
            m_service->info().serviceId() == service->info().serviceId();
}

void HServiceEventSubscriber::renew(const HTimeout& newTimeout)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(thread() == QThread::currentThread());

    if (expired())
    {
        return;
    }

    m_timeout = newTimeout;

    if (!m_timeout.isInfinite())
    {
        m_timer.start(m_timeout.value() * 1000);
    }
}

void HServiceEventSubscriber::notify(const QByteArray& msgBody)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(QThread::currentThread() == thread());

    m_messagesToSend.enqueue(msgBody);
    if (m_messagesToSend.size() <= 1)
    {
        // if there's more messages to send the sending process is active and
        // this message is enqueued to be sent once it's turn comes
        send();
    }
}

bool HServiceEventSubscriber::initialNotify(
    const QByteArray& msg, HMessagingInfo* mi)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(!m_seq);

    m_messagesToSend.enqueue(msg);

    if (!mi) { send()  ; }
    else     { send(mi); }

    return true;
}

}
}
