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

#include "hevent_notifier_p.h"
#include "hevent_subscriber_p.h"
#include "hdevicehost_configuration.h"

#include "../messages/hevent_messages_p.h"

#include "../../devicemodel/server/hserverdevice.h"
#include "../../devicemodel/server/hserverservice.h"
#include "../../devicemodel/server/hserverstatevariable.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"
#include "../../dataelements/hstatevariableinfo.h"

#include "../../http/hhttp_messaginginfo_p.h"

#include "../../general/hlogger_p.h"

#include <QtXml/QDomDocument>
#include <QtNetwork/QTcpSocket>

namespace Herqq
{

namespace Upnp
{

namespace
{
void getCurrentValues(QByteArray& msgBody, const HServerService* service)
{
    HLOG(H_AT, H_FUN);

    QDomDocument dd;

    QDomProcessingInstruction proc =
        dd.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");

    dd.appendChild(proc);

    QDomElement propertySetElem =
        dd.createElementNS("urn:schemas-upnp-org:event-1-0", "e:propertyset");

    dd.appendChild(propertySetElem);

    HServerStateVariables stateVars = service->stateVariables();
    QHash<QString, HServerStateVariable*>::const_iterator ci = stateVars.constBegin();
    for(; ci != stateVars.constEnd(); ++ci)
    {
        HServerStateVariable* stateVar = ci.value();
        Q_ASSERT(stateVar);

        const HStateVariableInfo& info = stateVar->info();
        if (info.eventingType() == HStateVariableInfo::NoEvents)
        {
            continue;
        }

        QDomElement propertyElem =
            dd.createElementNS("urn:schemas-upnp-org:event-1-0", "e:property");

        QDomElement variableElem = dd.createElement(info.name());
        variableElem.appendChild(dd.createTextNode(stateVar->value().toString()));

        propertyElem.appendChild(variableElem);
        propertySetElem.appendChild(propertyElem);
    }

    msgBody = dd.toByteArray();
}
}

/*******************************************************************************
 * HEventNotifier
 ******************************************************************************/
HEventNotifier::HEventNotifier(
    const QByteArray& loggingIdentifier,
    HDeviceHostConfiguration& configuration,
    QObject* parent) :
        QObject(parent),
            m_loggingIdentifier(loggingIdentifier),
            m_subscribers(),
            m_configuration(configuration)
{
}

HEventNotifier::~HEventNotifier()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    qDeleteAll(m_subscribers);
}

HTimeout HEventNotifier::getSubscriptionTimeout(const HSubscribeRequest& sreq)
{
    const static qint32 max = 60*60*24;

    qint32 configuredTimeout = m_configuration.subscriptionExpirationTimeout();
    if (configuredTimeout > 0)
    {
        return HTimeout(configuredTimeout);
    }
    else if (configuredTimeout == 0)
    {
        HTimeout requested = sreq.timeout();
        if (requested.isInfinite() || requested.value() > max)
        {
            return HTimeout(max);
        }

        return requested;
    }

    return HTimeout(max);
}

namespace
{
bool isSameService(HServerService* srv1, HServerService* srv2)
{
    return srv1->parentDevice()->info().udn() ==
           srv2->parentDevice()->info().udn() &&
           srv1->info().scpdUrl() == srv2->info().scpdUrl();
}
}

HServiceEventSubscriber* HEventNotifier::remoteClient(const HSid& sid) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HServiceEventSubscriber*>::const_iterator it =
        m_subscribers.constBegin();

    for(; it != m_subscribers.end(); ++it)
    {
        if ((*it)->sid() == sid)
        {
            return *it;
        }
    }

    return 0;
}

StatusCode HEventNotifier::addSubscriber(
    HServerService* service, const HSubscribeRequest& sreq, HSid* sid)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(sid);

    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.
    // This is enforced at the HServerService class, which should not send any
    // events unless one or more of its state variables are evented.

    for(qint32 i = 0; i < m_subscribers.size(); ++i)
    {
        HServiceEventSubscriber* rc = m_subscribers[i];

        if (isSameService(rc->service(), service) &&
            sreq.callbacks().contains(rc->location()))
        {
            HLOG_WARN(QString(
                "subscriber [%1] to the specified service URL [%2] already "
                "exists").arg(
                    rc->location().toString(),
                    service->info().scpdUrl().toString()));

            return PreconditionFailed;
        }
    }

    HLOG_INFO(QString("adding subscriber from [%1]").arg(
        sreq.callbacks().at(0).toString()));

    HTimeout timeout;
    if (service->isEvented())
    {
        timeout = getSubscriptionTimeout(sreq);
    }
    else
    {
        HLOG_WARN(QString(
            "Received subscription request to a service [%1] that has no evented state variables. "
            "No events will be sent to this subscriber.").arg(
                service->info().serviceType().toString()));
        timeout = HTimeout(60*60*24);
    }

    HServiceEventSubscriber* rc =
        new HServiceEventSubscriber(
            m_loggingIdentifier,
            service,
            sreq.callbacks().at(0),
            timeout,
            this);

    m_subscribers.push_back(rc);

    *sid = rc->sid();

    return Ok;
}

bool HEventNotifier::removeSubscriber(const HUnsubscribeRequest& req)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    bool found = false;

    QList<HServiceEventSubscriber*>::iterator it = m_subscribers.begin();
    for(; it != m_subscribers.end(); )
    {
        if ((*it)->sid() == req.sid())
        {
            HLOG_INFO(QString("removing subscriber [SID [%1]] from [%2]").arg(
                req.sid().toString(), (*it)->location().toString()));

            delete *it;
            it = m_subscribers.erase(it);

            found = true;
        }
        else if ((*it)->expired())
        {
            HLOG_INFO(QString(
                "removing an expired subscription [SID [%1]] from [%2]").arg(
                    (*it)->sid().toString(), (*it)->location().toString()));

            delete *it;
            it = m_subscribers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (!found)
    {
        HLOG_WARN(QString("Could not cancel subscription. Invalid SID [%1]").arg(
            req.sid().toString()));
    }

    return found;
}

StatusCode HEventNotifier::renewSubscription(
    const HSubscribeRequest& req, HSid* sid)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(sid);

    QList<HServiceEventSubscriber*>::iterator it = m_subscribers.begin();
    for(; it != m_subscribers.end();)
    {
        HServiceEventSubscriber* sub = (*it);
        if (sub->sid() == req.sid())
        {
            HLOG_INFO(QString("renewing subscription from [%1]").arg(
                (*it)->location().toString()));

            sub->renew(getSubscriptionTimeout(req));
            *sid = sub->sid();
            return Ok;
        }
        else if (sub->expired())
        {
            HLOG_INFO(QString("removing subscriber [SID [%1]] from [%2]").arg(
                sub->sid().toString(), sub->location().toString()));

            delete *it;
            it = m_subscribers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    HLOG_WARN(QString("Cannot renew subscription. Invalid SID: [%1]").arg(
        req.sid().toString()));

    return PreconditionFailed;
}

void HEventNotifier::stateChanged(const HServerService* source)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(source->isEvented());

    QByteArray msgBody;
    getCurrentValues(msgBody, source);

    QList<HServiceEventSubscriber*>::iterator it = m_subscribers.begin();
    for(; it != m_subscribers.end(); )
    {
        HServiceEventSubscriber* sub = (*it);
        if (sub->isInterested(source))
        {
            sub->notify(msgBody);
            ++it;
        }
        else if ((*it)->expired())
        {
            HLOG_INFO(QString("removing subscriber [SID [%1]] from [%2]").arg(
                sub->sid().toString(), sub->location().toString()));

            delete *it;
            it = m_subscribers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // TODO add multicast event support
}

void HEventNotifier::initialNotify(
    HServiceEventSubscriber* rc, HMessagingInfo* mi)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray msgBody;
    getCurrentValues(msgBody, rc->service());

    if (mi->keepAlive() && mi->socket().state() == QTcpSocket::ConnectedState)
    {
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!slight deviation from the UDA v1.1 specification!!
        //
        // the timeout for acknowledging a initial notify request using the
        // same connection is set to two seconds, instead of the 30 as specified
        // in the standard. This is for two reasons:
        // 1) there exists UPnP software that do not implement and respect
        // HTTP keep-alive properly.
        // 2) initial notify using HTTP keep-alive is very fast (unless something
        // is wrong) and even a second should be more than enough.

        // with the above in mind, if the subscriber seems to use HTTP keep-alive,
        // the initial notify is sent using the connection in which the
        // subscription came. However, if that fails, the initial notify is
        // re-send using a new connection.

        mi->setReceiveTimeoutForNoData(2000);

        if (!rc->initialNotify(msgBody, mi))
        {
            HLOG_WARN_NONSTD(QString(
                "Initial notify to SID [%1] failed. The device does not seem to " \
                "respect HTTP keep-alive. Re-sending the initial notify using a new connection.").arg(
                    rc->sid().toString()));
        }

        return;
    }

    // before sending the initial event message (specified in UDA),
    // the UDA mandates that FIN has been sent to the subscriber unless
    // the connection is to be kept alive.
    if (mi->socket().state() == QTcpSocket::ConnectedState)
    {
        mi->socket().disconnectFromHost();
        if (mi->socket().state() != QAbstractSocket::UnconnectedState)
        {
            mi->socket().waitForDisconnected(50);
        }
    }

    delete mi;
    rc->initialNotify(msgBody);
}

}
}
