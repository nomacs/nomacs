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

#include "hevent_subscriptionmanager_p.h"
#include "hcontrolpoint_configuration.h"
#include "hcontrolpoint_p.h"

#include "../../general/hupnp_global_p.h"

#include "../../devicemodel/client/hclientdevice.h"
#include "../../devicemodel/client/hdefault_clientservice_p.h"

#include "../../dataelements/hserviceid.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"

#include "../../general/hlogger_p.h"

namespace Herqq
{

namespace Upnp
{

HEventSubscriptionManager::HEventSubscriptionManager(HControlPointPrivate* owner) :
    QObject(owner),
        m_owner(owner), m_subscribtionsByUuid(), m_subscriptionsByUdn()
{
    Q_ASSERT(m_owner);
}

HEventSubscriptionManager::~HEventSubscriptionManager()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    removeAll();
}

void HEventSubscriptionManager::subscribed_slot(HEventSubscription* sub)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(sub);

    emit subscribed(sub->service());
}

void HEventSubscriptionManager::subscriptionFailed_slot(HEventSubscription* sub)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(sub);

    HClientService* service = sub->service();
    sub->resetSubscription();
    emit subscriptionFailed(service);
}

void HEventSubscriptionManager::unsubscribed(HEventSubscription* sub)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(sub);

    emit unsubscribed(sub->service());
}

HEventSubscription* HEventSubscriptionManager::createSubscription(
    HClientService* service, qint32 timeout)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    Q_ASSERT(service);
    Q_ASSERT(thread() == QThread::currentThread());

    QUrl httpSrvRootUrl = getSuitableHttpServerRootUrl(
        service->parentDevice()->locations());

    Q_ASSERT(!httpSrvRootUrl.isEmpty());

    HEventSubscription* subscription =
        new HEventSubscription(
            m_owner->m_loggingIdentifier,
            service,
            httpSrvRootUrl,
            HTimeout(timeout),
            this);

    bool ok = connect(
        subscription,
        SIGNAL(subscribed(HEventSubscription*)),
        this,
        SLOT(subscribed_slot(HEventSubscription*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        subscription,
        SIGNAL(subscriptionFailed(HEventSubscription*)),
        this,
        SLOT(subscriptionFailed_slot(HEventSubscription*)));

    Q_ASSERT(ok);

    ok = connect(
        subscription, SIGNAL(unsubscribed(HEventSubscription*)),
        this, SLOT(unsubscribed(HEventSubscription*)));

    Q_ASSERT(ok);

    return subscription;
}

QUrl HEventSubscriptionManager::getSuitableHttpServerRootUrl(
    const QList<QUrl>& deviceLocations)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    if (m_owner->m_server->endpointCount() == 1)
    {
        return m_owner->m_server->rootUrls().at(0);
    }

    foreach(const QUrl& deviceLocation, deviceLocations)
    {
        quint32 localNetw;
        bool b = HSysInfo::instance().localNetwork(
            HEndpoint(deviceLocation).hostAddress(), &localNetw);
        if (b)
        {
            QUrl rootUrl = m_owner->m_server->rootUrl(QHostAddress(localNetw));
            if (rootUrl.isValid() && !rootUrl.isEmpty())
            {
                return rootUrl;
            }
        }
    }

    return m_owner->m_server->rootUrls().at(0);
}

bool HEventSubscriptionManager::subscribe(
    HClientDevice* device, DeviceVisitType visitType,
    qint32 timeout)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(device);

    bool ok = false;
    HClientServices services(device->services());
    for(qint32 i = 0; i < services.size(); ++i)
    {
        HClientService* service = services.at(i);
        if (service->isEvented())
        {
            if (subscribe(service, timeout) == Sub_Success)
            {
                ok = true;
            }
        }
    }

    if (visitType == VisitThisAndDirectChildren ||
        visitType == VisitThisRecursively)
    {
        HClientDevices devices(device->embeddedDevices());
        for(qint32 i = 0; i < devices.size(); ++i)
        {
            DeviceVisitType visitTypeForChildren =
                visitType == VisitThisRecursively ?
                    VisitThisRecursively : VisitThisOnly;

            if (subscribe(devices.at(i), visitTypeForChildren, timeout) && !ok)
            {
                ok = true;
            }
        }
    }

    return ok;
}

HEventSubscriptionManager::SubscriptionResult
    HEventSubscriptionManager::subscribe(HClientService* service, qint32 timeout)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);

    if (!service->isEvented())
    {
        HLOG_WARN(QString(
            "Cannot subscribe to a service [%1] that is not evented").arg(
                service->info().serviceId().toString()));

        return Sub_Failed_NotEvented;
    }

    HUdn deviceUdn = service->parentDevice()->info().udn();

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(deviceUdn);

    if (!subs)
    {
        subs = new QList<HEventSubscription*>();
        goto end;
    }
    else
    {
        QList<HEventSubscription*>::iterator it = subs->begin();
        for(; it != subs->end(); ++it)
        {
            HEventSubscription* sub = (*it);
            if (sub->service() == service)
            {
                if (sub->subscriptionStatus() == HEventSubscription::Status_Subscribed)
                {
                    HLOG_WARN(QString("Subscription to service [%1] exists").arg(
                        service->info().serviceId().toString()));
                    return Sub_AlreadySubscribed;
                }
                else
                {
                    sub->subscribe();
                    return Sub_Success;
                }
            }
        }
    }

end:

    HEventSubscription* sub = createSubscription(service, timeout);
    m_subscribtionsByUuid.insert(sub->id(), sub);
    m_subscriptionsByUdn.insert(deviceUdn, subs);
    subs->append(sub);

    sub->subscribe();

    return Sub_Success;
}

HEventSubscription::SubscriptionStatus
    HEventSubscriptionManager::subscriptionStatus(
        const HClientService* service) const
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);

    HUdn udn = service->parentDevice()->info().udn();

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return HEventSubscription::Status_Unsubscribed;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);

        if (sub->service() == service)
        {
            return sub->subscriptionStatus();
        }
    }

    return HEventSubscription::Status_Unsubscribed;
}

bool HEventSubscriptionManager::cancel(
    HClientDevice* device, DeviceVisitType visitType,
    bool unsubscribe)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(device);
    Q_ASSERT(thread() == QThread::currentThread());

    HUdn udn = device->info().udn();

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        if (unsubscribe)
        {
            (*it)->unsubscribe();
        }
        else
        {
            (*it)->resetSubscription();
        }
    }

    if (visitType == VisitThisAndDirectChildren ||
        visitType == VisitThisRecursively)
    {
        HClientDevices devices(device->embeddedDevices());
        for(qint32 i = 0; i < devices.size(); ++i)
        {
            DeviceVisitType visitTypeForChildren =
                visitType == VisitThisRecursively ?
                    VisitThisRecursively : VisitThisOnly;

            cancel(devices.at(i), visitTypeForChildren, unsubscribe);
        }
    }

    return true;
}

bool HEventSubscriptionManager::remove(HClientDevice* device, bool recursive)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(device);
    Q_ASSERT(thread() == QThread::currentThread());

    HUdn udn = device->info().udn();

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);
        m_subscribtionsByUuid.remove(sub->id());
        delete sub;
    }

    m_subscriptionsByUdn.remove(udn);
    delete subs;

    if (recursive)
    {
        HClientDevices devices(device->embeddedDevices());
        for(qint32 i = 0; i < devices.size(); ++i)
        {
            remove(devices.at(i), recursive);
        }
    }

    return true;
}

bool HEventSubscriptionManager::cancel(HClientService* service, bool unsubscribe)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);
    Q_ASSERT(thread() == QThread::currentThread());

    HClientDevice* parentDevice = service->parentDevice();

    HUdn udn = parentDevice->info().udn();

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);

        if (sub->service() != service)
        {
            continue;
        }

        if (unsubscribe)
        {
            (*it)->unsubscribe();
        }
        else
        {
            (*it)->resetSubscription();
        }

        return true;
    }

    return false;
}

bool HEventSubscriptionManager::remove(HClientService* service)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);
    Q_ASSERT(thread() == QThread::currentThread());

    HClientDevice* parentDevice = service->parentDevice();

    HUdn udn = parentDevice->info().udn();

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);

        if (sub->service() != service)
        {
            continue;
        }

        subs->erase(it);
        if (subs->isEmpty())
        {
            delete subs;
            m_subscriptionsByUdn.remove(udn);
        }

        m_subscribtionsByUuid.remove(sub->id());
        delete sub;

        return true;
    }

    return false;
}

void HEventSubscriptionManager::cancelAll(qint32 msecsToWait)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    QHash<QUuid, HEventSubscription*>::iterator it =
        m_subscribtionsByUuid.begin();

    for(; it != m_subscribtionsByUuid.end(); ++it)
    {
        (*it)->unsubscribe(msecsToWait);
    }
}

void HEventSubscriptionManager::removeAll()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    qDeleteAll(m_subscribtionsByUuid);
    m_subscribtionsByUuid.clear();

    qDeleteAll(m_subscriptionsByUdn);
    m_subscriptionsByUdn.clear();
}

StatusCode HEventSubscriptionManager::onNotify(
    const QUuid& id, const HNotifyRequest& req)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    HEventSubscription* sub = m_subscribtionsByUuid.value(id);
    if (!sub)
    {
        HLOG_WARN(QString(
            "Ignoring notification [seq: %1] due to invalid callback ID [%2]: "
            "no such subscription found.").arg(
                QString::number(req.seq()), id.toString()));

        return BadRequest;
    }

    return sub->onNotify(req);
}

}
}
