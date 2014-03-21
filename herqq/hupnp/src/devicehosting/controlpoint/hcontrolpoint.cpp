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

#include "hcontrolpoint.h"
#include "hcontrolpoint_p.h"
#include "hevent_subscription_p.h"
#include "hclientmodel_creator_p.h"
#include "hcontrolpoint_configuration.h"
#include "hcontrolpoint_configuration_p.h"
#include "hcontrolpoint_dataretriever_p.h"

#include "../../general/hupnp_global_p.h"
#include "../../general/hupnp_datatypes_p.h"

#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hdiscoverytype.h"
#include "../../dataelements/hproduct_tokens.h"

#include "../../devicemodel/client/hdefault_clientdevice_p.h"
#include "../../devicemodel/client/hdefault_clientservice_p.h"

#include "../../http/hhttp_messagecreator_p.h"

#include "../../general/hlogger_p.h"
#include "../../utils/hsysutils_p.h"

#include <QtCore/QUrl>
#include <QtCore/QString>

#include <QtCore/QMetaType>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::HUdn>("Herqq::Upnp::HUdn");
    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * ControlPointHttpServer
 ******************************************************************************/
ControlPointHttpServer::ControlPointHttpServer(HControlPointPrivate* owner) :
    HHttpServer(owner->m_loggingIdentifier, owner),
        m_owner(owner)
{
    Q_ASSERT(m_owner);
}

ControlPointHttpServer::~ControlPointHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    close();
}

void ControlPointHttpServer::incomingNotifyMessage(
    HMessagingInfo* mi, const HNotifyRequest& req)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HLOG_DBG(QString(
        "Incoming event notify from [%1]").arg(peerAsStr(mi->socket())));

    QString serviceCallbackId = req.callback().path().remove('/');
    StatusCode statusCode =
        m_owner->m_eventSubscriber->onNotify(serviceCallbackId, req);

    if (statusCode != Ok)
    {
        mi->setKeepAlive(false);
    }

    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(statusCode, *mi));
}

/*******************************************************************************
 * HControlPointSsdpHandler
 ******************************************************************************/
HControlPointSsdpHandler::HControlPointSsdpHandler(
    HControlPointPrivate* owner) :
        HSsdp(owner->m_loggingIdentifier, owner), m_owner(owner)
{
    setFilter(DiscoveryResponse | DeviceUnavailable | DeviceAvailable);
}

HControlPointSsdpHandler::~HControlPointSsdpHandler()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
}

bool HControlPointSsdpHandler::incomingDiscoveryResponse(
    const HDiscoveryResponse& msg, const HEndpoint& source)
{
    return m_owner->processDeviceDiscovery(msg, source, this);
}

bool HControlPointSsdpHandler::incomingDeviceAvailableAnnouncement(
    const HResourceAvailable& msg, const HEndpoint& source)
{
    return m_owner->processDeviceDiscovery(msg, source, this);
}

bool HControlPointSsdpHandler::incomingDeviceUnavailableAnnouncement(
    const HResourceUnavailable& msg, const HEndpoint& source)
{
    return m_owner->processDeviceOffline(msg, source, this);
}

/*******************************************************************************
 * HControlPointPrivate
 ******************************************************************************/
HControlPointPrivate::HControlPointPrivate() :
    QObject(),
        m_deviceBuildTasks(),
        m_loggingIdentifier(
            QString("__CONTROL POINT %1__: ").arg(
                QUuid::createUuid().toString()).toLocal8Bit()),
        m_configuration(),
        m_ssdps(),
        m_server(0),
        m_eventSubscriber(0),
        m_lastError(HControlPoint::UndefinedError),
        q_ptr(0),
        m_nam(new QNetworkAccessManager(this)),
        m_state(HControlPointPrivate::Uninitialized),
        m_threadPool(new HThreadPool(this)),
        m_deviceStorage(m_loggingIdentifier)
{
}

HControlPointPrivate::~HControlPointPrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

HDefaultClientDevice* HControlPointPrivate::buildDevice(
    const QUrl& deviceLocation, qint32 maxAgeInSecs, QString* err)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HDataRetriever dataRetriever(m_loggingIdentifier);

    QString deviceDescr;
    if (!dataRetriever.retrieveDeviceDescription(deviceLocation, &deviceDescr))
    {
        *err = dataRetriever.lastError();
        return 0;
    }

    QList<QUrl> deviceLocations;
    deviceLocations.push_back(deviceLocation);

    HClientModelCreationArgs creatorParams(m_nam);
    creatorParams.m_deviceDescription = deviceDescr;
    creatorParams.m_deviceLocations = deviceLocations;

    creatorParams.m_serviceDescriptionFetcher =
        ServiceDescriptionFetcher(
            &dataRetriever, &HDataRetriever::retrieveServiceDescription);

    creatorParams.m_deviceTimeoutInSecs = maxAgeInSecs;

    creatorParams.m_iconFetcher =
        IconFetcher(&dataRetriever, &HDataRetriever::retrieveIcon);

    creatorParams.m_loggingIdentifier = m_loggingIdentifier;

    HClientModelCreator creator(creatorParams);
    HDefaultClientDevice* device = creator.createRootDevice();
    if (!device && err)
    {
        *err = creator.lastErrorDescription();
    }

    return device;
}

bool HControlPointPrivate::addRootDevice(HDefaultClientDevice* newRootDevice)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(thread() == QThread::currentThread());

    HDefaultClientDevice* existingDevice =
        static_cast<HDefaultClientDevice*>(
            m_deviceStorage.searchDeviceByUdn(
                newRootDevice->info().udn(), AllDevices));

    if (existingDevice)
    {
        // it seems that the device we've built has already been added
        // (it is possible, although unlikely, we begin multiple device build
        // processes of the same device tree)
        // in this case we only make sure that the device's location list is
        // updated if necessary

        existingDevice = static_cast<HDefaultClientDevice*>(existingDevice->rootDevice());
        existingDevice->addLocations(newRootDevice->locations());
        return false;
    }

    if (q_ptr->acceptRootDevice(newRootDevice) == HControlPoint::IgnoreDevice)
    {
        HLOG_DBG(QString("Device [%1] rejected").arg(
            newRootDevice->info().udn().toString()));
        return false;
    }

    newRootDevice->setParent(this);
    newRootDevice->startStatusNotifier(HDefaultClientDevice::All);

    bool ok = connect(
        newRootDevice, SIGNAL(statusTimeout(HDefaultClientDevice*)),
        this, SLOT(deviceExpired(HDefaultClientDevice*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    if (!m_deviceStorage.addRootDevice(newRootDevice))
    {
        HLOG_WARN(QString(
            "Failed to add root device [UDN: %1]: %2").arg(
                newRootDevice->info().udn().toSimpleUuid(),
                m_deviceStorage.lastError()));

        return false;
    }

    emit q_ptr->rootDeviceOnline(newRootDevice);
    return true;
}

void HControlPointPrivate::deviceExpired(HDefaultClientDevice* source)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    // according to the UDA v1.1 a "device tree" (root, embedded and services)
    // are "timed out" only when every advertisement has timed out.

    source = static_cast<HDefaultClientDevice*>(source->rootDevice());

    if (source->isTimedout(HDefaultClientDevice::All))
    {
        source->deviceStatus()->setOnline(false);
        m_eventSubscriber->cancel(
            source, VisitThisRecursively, false);

        emit q_ptr->rootDeviceOffline(source);
    }
}

void HControlPointPrivate::unsubscribed(HClientService* service)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(service);
    emit q_ptr->subscriptionCanceled(service);
}

bool HControlPointPrivate::processDeviceOffline(
    const HResourceUnavailable& msg, const HEndpoint& /*source*/,
    HControlPointSsdpHandler* /*origin*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    HDefaultClientDevice* device =
        static_cast<HDefaultClientDevice*>(
            m_deviceStorage.searchDeviceByUdn(msg.usn().udn(), AllDevices));

    if (!device)
    {
        // the device is not known by us.
        // note that even service announcements contain the "UDN", which identifies
        // the device that contains them.
        return true;
    }

    if (device->deviceStatus()->online())
    {

        HLOG_INFO(QString("Resource [%1] is unavailable.").arg(
            msg.usn().resourceType().toString()));

        // according to the UDA v1.1 specification, if a bye bye message of any kind
        // is received, the control point can assume that nothing in that
        // device tree is available anymore

        HDefaultClientDevice* root =
            static_cast<HDefaultClientDevice*>(device->rootDevice());

        Q_ASSERT(root);

        root->deviceStatus()->setOnline(false);

        m_eventSubscriber->remove(root, true);

        root->clearLocations();
        root->stopStatusNotifier(HDefaultClientDevice::All);

        emit q_ptr->rootDeviceOffline(root);
    }

    return true;
}

template<typename Msg>
bool HControlPointPrivate::processDeviceDiscovery(
    const Msg& msg, const HEndpoint& source, HControlPointSsdpHandler*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    const HUdn& resourceUdn = msg.usn().udn();

    HDefaultClientDevice* device =
        static_cast<HDefaultClientDevice*>(
            m_deviceStorage.searchDeviceByUdn(msg.usn().udn(), AllDevices));

    if (device)
    {
        // according to the UDA v1.1 spec, if a control point receives an
        // alive announcement of any type for a device tree, the control point
        // can assume that all devices and services are available.
        // ==> reset timeouts for entire device tree and all services.

        device = static_cast<HDefaultClientDevice*>(device->rootDevice());
        device->startStatusNotifier(HDefaultClientDevice::All);

        // it cannot be that only some embedded device is available at certain
        // interface, since the device description is always fetched from the
        // the location that the root device specifies ==> the entire device
        // tree has to be available at that location.
        if (device->addLocation(msg.location()))
        {
            HLOG_DBG(QString("Existing device [%1] now available at [%2]").arg(
                resourceUdn.toString(), msg.location().toString()));
        }

        if (!device->deviceStatus()->online())
        {
            device->deviceStatus()->setOnline(true);
            emit q_ptr->rootDeviceOnline(device);
            processDeviceOnline(device, false);
        }

        return true;
    }

    // it does not matter if the device is an embedded device, since the
    // location of the device always points to the root device's description
    // and the internal device model is built of that. Hence, any advertisement
    // will do to build the entire model correctly.

    DeviceBuildTask* dbp = m_deviceBuildTasks.get(msg);
    if (dbp)
    {
        if (!dbp->m_locations.contains(msg.location()))
        {
            dbp->m_locations.push_back(msg.location());
        }

        return true;
    }

    if (!q_ptr->acceptResource(msg.usn(), source))
    {
        HLOG_DBG(QString("Resource advertisement [%1] rejected").arg(
            msg.usn().toString()));

        return true;
    }

    DeviceBuildTask* newBuildTask = new DeviceBuildTask(this, msg);

    newBuildTask->setAutoDelete(false);

    m_deviceBuildTasks.add(newBuildTask);

    bool ok = connect(
        newBuildTask, SIGNAL(done(Herqq::Upnp::HUdn)),
        this, SLOT(deviceModelBuildDone(Herqq::Upnp::HUdn)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    HLOG_INFO(QString(
        "New resource [%1] is available @ [%2]. "
        "Attempting to build the device model.").arg(
            msg.usn().toString(), msg.location().toString()));

    m_threadPool->start(newBuildTask);

    return true;
}

void HControlPointPrivate::processDeviceOnline(
    HDefaultClientDevice* device, bool newDevice)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HControlPoint::DeviceDiscoveryAction actionToTake =
        q_ptr->acceptRootDevice(device);

    bool subscribe = false;
    switch(actionToTake)
    {
    case HControlPoint::IgnoreDevice:

        HLOG_DBG(QString("Discarding device with UDN %1").arg(
            device->info().udn().toString()));

        if (newDevice)
        {
            delete device; device = 0;
        }
        break;

    case HControlPoint::AddDevice:
        break;

    case HControlPoint::AddDevice_SubscribeEventsIfConfigured:
        subscribe = m_configuration->subscribeToEvents();
        break;

    case HControlPoint::AddDevice_SubscribeAllEvents:
        subscribe = true;
        break;

    default:
        Q_ASSERT(false);
        break;
    };

    if (device)
    {
        if (newDevice)
        {
            if (!addRootDevice(device))
            {
                delete device;
                return;
            }
        }
        if (subscribe)
        {
            m_eventSubscriber->subscribe(
                device, VisitThisRecursively,
                m_configuration->desiredSubscriptionTimeout());
        }
    }
}

void HControlPointPrivate::deviceModelBuildDone(const Herqq::Upnp::HUdn& udn)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    DeviceBuildTask* build = m_deviceBuildTasks.get(udn);
    Q_ASSERT(build);

    if (m_state == Initialized)
    {
        // The check is done because it is possible that a user has called
        // HControlPoint::quit() before this event is delivered.
        if (build->completionValue() == 0)
        {
            HLOG_INFO(QString("Device model for [%1] built successfully.").arg(
                udn.toString()));

            HDefaultClientDevice* device = build->createdDevice();
            Q_ASSERT(device);

            for (qint32 i = 0; i < build->m_locations.size(); ++i)
            {
                device->addLocation(build->m_locations[i]);
            }

            processDeviceOnline(device, true);
        }
        else
        {
            HLOG_WARN(QString("Device model for [%1] could not be built: %2.").arg(
                udn.toString(), build->errorString()));
        }
    }

    m_deviceBuildTasks.remove(udn);
}

/*******************************************************************************
 * HControlPoint
 ******************************************************************************/
HControlPoint::HControlPoint(QObject* parent) :
    QObject(parent), h_ptr(new HControlPointPrivate())
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_configuration.reset(new HControlPointConfiguration());

    h_ptr->setParent(this);
    h_ptr->q_ptr = this;

    bool ok = connect(
        h_ptr->m_nam,
        SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
        this,
        SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

HControlPoint::HControlPoint(
    const HControlPointConfiguration& configuration, QObject* parent) :
        QObject(parent), h_ptr(new HControlPointPrivate())
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_configuration.reset(configuration.clone());

    h_ptr->setParent(this);
    h_ptr->q_ptr = this;
}

HControlPoint::HControlPoint(
    HControlPointPrivate& dd, const HControlPointConfiguration* configuration,
    QObject* parent) :
        QObject(parent), h_ptr(&dd)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_configuration.reset(configuration ?
        configuration->clone() : new HControlPointConfiguration());

    h_ptr->setParent(this);
    h_ptr->q_ptr = this;
}

HControlPoint::~HControlPoint()
{
    quit();
    delete h_ptr;
}

bool HControlPoint::doInit()
{
    // the default implementation does nothing.
    return true;
}

void HControlPoint::doQuit()
{
    // the default implementation does nothing.
}

HControlPoint::DeviceDiscoveryAction HControlPoint::acceptRootDevice(
    HClientDevice* /*device*/)
{
    return AddDevice_SubscribeEventsIfConfigured;
}

bool HControlPoint::acceptResource(
    const HDiscoveryType& /*usn*/, const HEndpoint& /*source*/)
{
    return true;
}

const HControlPointConfiguration* HControlPoint::configuration() const
{
    return h_ptr->m_configuration.data();
}

void HControlPoint::setError(ControlPointError error, const QString& errorStr)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_lastError = error;
    h_ptr->m_lastErrorDescription = errorStr;
}

bool HControlPoint::init()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be initialized in the thread in which it is "
        "currently located.");

    if (h_ptr->m_state == HControlPointPrivate::Initialized)
    {
        setError(
            AlreadyInitializedError,
            "The control point is already initialized");

        return false;
    }

    Q_ASSERT(h_ptr->m_state == HControlPointPrivate::Uninitialized);

    bool ok = true;
    const QList<QHostAddress> addrs =
        h_ptr->m_configuration->networkAddressesToUse();

    h_ptr->m_state = HControlPointPrivate::Initializing;

    HLOG_INFO("ControlPoint initializing.");

    h_ptr->m_eventSubscriber = new HEventSubscriptionManager(h_ptr);

    ok = connect(
        h_ptr->m_eventSubscriber,
        SIGNAL(subscribed(Herqq::Upnp::HClientService*)),
        this,
        SIGNAL(subscriptionSucceeded(Herqq::Upnp::HClientService*)));

    ok = connect(
        h_ptr->m_eventSubscriber,
        SIGNAL(subscriptionFailed(Herqq::Upnp::HClientService*)),
        this,
        SIGNAL(subscriptionFailed(Herqq::Upnp::HClientService*)));

    Q_ASSERT(ok);

    ok = connect(
        h_ptr->m_eventSubscriber,
        SIGNAL(unsubscribed(Herqq::Upnp::HClientService*)),
        h_ptr,
        SLOT(unsubscribed(Herqq::Upnp::HClientService*)));

    Q_ASSERT(ok);

    h_ptr->m_server = new ControlPointHttpServer(h_ptr);

    if (!doInit())
    {
        // it is assumed that the derived class filled the error and
        // error description
        ok = false;
        goto end;
    }

    if (!h_ptr->m_server->init(convertHostAddressesToEndpoints(addrs)))
    {
        setError(CommunicationsError, "Failed to start HTTP server");
        ok = false;
        goto end;
    }

    foreach(const QHostAddress& ha, addrs)
    {
        quint32 netwAddr;
        ok = HSysInfo::instance().localNetwork(ha, &netwAddr);
        Q_ASSERT(ok);

        HControlPointSsdpHandler* ssdp = new HControlPointSsdpHandler(h_ptr);
        if (!ssdp->init(ha))
        {
            delete ssdp;
            setError(CommunicationsError, "Failed to start SSDP");
            ok = false;
            goto end;
        }
        h_ptr->m_ssdps.append(qMakePair(netwAddr, ssdp));
    }

    if (h_ptr->m_configuration->autoDiscovery())
    {
        HLOG_DBG("Searching for UPnP devices");

        for(qint32 i = 0; i < h_ptr->m_ssdps.size(); ++i)
        {
            QString ep =
                h_ptr->m_ssdps[i].second->unicastEndpoint().toString();

            HLOG_DBG(QString(
                "Sending discovery request using endpoint [%1]").arg(ep));

            qint32 messagesSent =
                h_ptr->m_ssdps[i].second->sendDiscoveryRequest(
                    HDiscoveryRequest(
                        1,
                        HDiscoveryType::createDiscoveryTypeForRootDevices(),
                        HSysInfo::instance().herqqProductTokens()));

            if (!messagesSent)
            {
                HLOG_WARN(QString(
                    "Failed to send discovery request using endpoint "
                    "[%1]").arg(ep));
            }
        }
    }
    else
    {
        HLOG_DBG("Omitting initial device discovery as configured");
    }

    h_ptr->m_state = HControlPointPrivate::Initialized;

end:

    if (!ok)
    {
        h_ptr->m_state = HControlPointPrivate::Exiting;
        quit();

        HLOG_INFO("ControlPoint initialization failed.");
        return false;
    }

    setError(UndefinedError, "");
    HLOG_INFO("ControlPoint initialized.");
    return true;
}

HControlPoint::ControlPointError HControlPoint::error() const
{
    return h_ptr->m_lastError;
}

QString HControlPoint::errorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

void HControlPoint::quit()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be shutdown in the thread in which it is "
        "currently located.");

    if (!isStarted())
    {
        return;
    }

    HLOG_INFO("Shutting down.");

    h_ptr->m_state = HControlPointPrivate::Exiting;

    h_ptr->m_eventSubscriber->cancelAll(100);
    h_ptr->m_eventSubscriber->removeAll();

    h_ptr->m_server->close();

    h_ptr->m_threadPool->shutdown();

    doQuit();

    delete h_ptr->m_server; h_ptr->m_server = 0;
    for(qint32 i = 0; i < h_ptr->m_ssdps.size(); ++i)
    {
        delete h_ptr->m_ssdps[i].second; h_ptr->m_ssdps[i].second = 0;
    }
    h_ptr->m_ssdps.clear();

    h_ptr->m_deviceStorage.clear();

    delete h_ptr->m_eventSubscriber; h_ptr->m_eventSubscriber = 0;

    h_ptr->m_state = HControlPointPrivate::Uninitialized;
    HLOG_INFO("Shut down.");
}

bool HControlPoint::isStarted() const
{
    return h_ptr->m_state == HControlPointPrivate::Initialized;
}

HClientDevices HControlPoint::rootDevices() const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The control point is not started");
        return HClientDevices();
    }

    return h_ptr->m_deviceStorage.rootDevices();
}

HClientDevices HControlPoint::devices(
    const HResourceType& deviceType, HResourceType::VersionMatch vm,
    TargetDeviceType dts)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The control point is not started");
        return HClientDevices();
    }

    return h_ptr->m_deviceStorage.searchDevicesByDeviceType(deviceType, vm, dts);
}

HClientDevice* HControlPoint::device(
    const HUdn& udn, TargetDeviceType dts) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The control point is not started");
        return 0;
    }

    return h_ptr->m_deviceStorage.searchDeviceByUdn(udn, dts);
}

bool HControlPoint::subscribeEvents(
    HClientDevice* device, DeviceVisitType visitType)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }
    else if (!device)
    {
        setError(InvalidArgumentError, "Null pointer error");
        return false;
    }
    else if (!h_ptr->m_deviceStorage.searchDeviceByUdn(
        device->info().udn(), AllDevices))
    {
        setError(InvalidArgumentError,
            "The specified device was not found in this control point");

        return false;
    }

    bool ok =
        h_ptr->m_eventSubscriber->subscribe(
            device, visitType,
            h_ptr->m_configuration->desiredSubscriptionTimeout());

    if (!ok)
    {
        setError(
            InvalidArgumentError,
            "Could not subscribe to any of the services contained by the device; "
               "The device may not have services or none of them are evented, or "
               "there is active subscription to every one of them already");

        return false;
    }

    return true;
}

bool HControlPoint::subscribeEvents(HClientService* service)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }
    else if (!service)
    {
        setError(InvalidArgumentError, "Null pointer error");
        return false;
    }
    else if (!h_ptr->m_deviceStorage.searchDeviceByUdn(
                service->parentDevice()->info().udn(), AllDevices))
    {
        setError(InvalidArgumentError,
            "The specified service was not found in this control point");

        return false;
    }

    HEventSubscriptionManager::SubscriptionResult res =
        h_ptr->m_eventSubscriber->subscribe(
            static_cast<HDefaultClientService*>(service),
            h_ptr->m_configuration->desiredSubscriptionTimeout());

    switch(res)
    {
    case HEventSubscriptionManager::Sub_Success:
        return true;

    case HEventSubscriptionManager::Sub_AlreadySubscribed:
        setError(
            InvalidArgumentError,
            "Already subscribed to the specified service");

        break;

    case HEventSubscriptionManager::Sub_Failed_NotEvented:
        setError(
            InvalidArgumentError,
            "The specified service is not evented");

        break;

    default:
        Q_ASSERT(false);
    }

    return false;
}

HControlPoint::SubscriptionStatus HControlPoint::subscriptionStatus(
    const HClientService* service) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    return static_cast<HControlPoint::SubscriptionStatus>(
        h_ptr->m_eventSubscriber->subscriptionStatus(service));
}

bool HControlPoint::cancelEvents(
    HClientDevice* device, DeviceVisitType visitType)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }
    else if (!device)
    {
        setError(InvalidArgumentError, "Null pointer error");
        return false;
    }
    else if (!h_ptr->m_deviceStorage.searchDeviceByUdn(
                device->info().udn(), AllDevices))
    {
        setError(
            InvalidArgumentError,
            "The specified device was not found in this control point");

        return false;
    }

    if (h_ptr->m_eventSubscriber->cancel(device, visitType, true))
    {
        return true;
    }

    setError(
        InvalidArgumentError,
        "No active subscriptions to any of the services contained by the device");

    return false;
}

bool HControlPoint::cancelEvents(HClientService* service)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }
    else if (!service)
    {
        setError(InvalidArgumentError, "Null pointer error");
        return false;
    }
    else if (!h_ptr->m_deviceStorage.searchDeviceByUdn(
                service->parentDevice()->info().udn(), AllDevices))
    {
        setError(
            InvalidArgumentError,
            "The specified service was not found in this control point");

        return false;
    }

    if (h_ptr->m_eventSubscriber->cancel(service, true))
    {
        return true;
    }

    setError(
        InvalidArgumentError,
        "No active subscription to the specified service");

    return false;
}

bool HControlPoint::removeRootDevice(HClientDevice* rootDevice)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }
    else if (!rootDevice)
    {
        setError(InvalidArgumentError, "Null pointer error");
        return false;
    }
    else if (rootDevice->parentDevice())
    {
        setError(InvalidArgumentError, "Cannot remove embedded devices");
        return false;
    }

    Q_ASSERT(thread() == QThread::currentThread());

    h_ptr->m_eventSubscriber->remove(rootDevice, true);
    // TODO should send unsubscription to the UPnP device?

    HDeviceInfo info(rootDevice->info());
    if (h_ptr->m_deviceStorage.removeRootDevice(rootDevice))
    {
        emit rootDeviceRemoved(info);
        return true;
    }

    setError(
        InvalidArgumentError,
        "The device was not found in this control point");

    return false;
}

bool HControlPoint::scan(const HDiscoveryType& discoveryType, qint32 count)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }
    else if (discoveryType.type() == HDiscoveryType::Undefined)
    {
        setError(InvalidArgumentError, "Discovery type was undefined");
        return false;
    }
    else if (count <= 0)
    {
        setError(
            InvalidArgumentError,
            "The number of messages has to be greater than zero");

        return false;
    }

    for(qint32 i = 0; i < h_ptr->m_ssdps.size(); ++i)
    {
        QPair<quint32, HControlPointSsdpHandler*> ssdp = h_ptr->m_ssdps[i];

        HDiscoveryRequest req(
            1, discoveryType, HSysInfo::instance().herqqProductTokens());

        qint32 messagesSent = ssdp.second->sendDiscoveryRequest(req, count);
        if (messagesSent != count)
        {
            return false;
        }
    }

    return true;
}

bool HControlPoint::scan(
    const HDiscoveryType& discoveryType, const HEndpoint& destination,
    qint32 count)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }
    else if (discoveryType.type() == HDiscoveryType::Undefined)
    {
        setError(InvalidArgumentError, "Discovery type was undefined");
        return false;
    }
    else if (count <= 0)
    {
        setError(
            InvalidArgumentError,
            "The number of messages has to be greater than zero");

        return false;
    }

    for(qint32 i = 0; i < h_ptr->m_ssdps.size(); ++i)
    {
        QPair<quint32, HControlPointSsdpHandler*> ssdp = h_ptr->m_ssdps[i];

        HDiscoveryRequest req(
            1, discoveryType, HSysInfo::instance().herqqProductTokens());

        quint32 netAddr;
        bool ok = HSysInfo::instance().localNetwork(
            destination.hostAddress(), &netAddr);

        if (ok && netAddr == ssdp.first)
        {
            qint32 messagesSent = ssdp.second->sendDiscoveryRequest(
                req, destination, count);

            return messagesSent == count;
        }
    }

    return false;
}

}
}
