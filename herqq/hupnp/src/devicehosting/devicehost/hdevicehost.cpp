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

#include "hdevicehost.h"
#include "hdevicehost_p.h"
#include "hevent_notifier_p.h"
#include "hpresence_announcer_p.h"
#include "hdevicehost_configuration.h"
#include "hserverdevicecontroller_p.h"
#include "hdevicehost_http_server_p.h"
#include "hdevicehost_ssdp_handler_p.h"
#include "hdevicehost_runtimestatus_p.h"
#include "hdevicehost_dataretriever_p.h"

#include "hservermodel_creator_p.h"

#include "../../general/hlogger_p.h"
#include "../../utils/hsysutils_p.h"

#include <ctime>

#include <QtCore/QTimer>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServerDeviceController
 ******************************************************************************/
HServerDeviceController::HServerDeviceController(
    HServerDevice* device, qint32 deviceTimeoutInSecs, QObject* parent) :
        QObject(parent),
            m_statusNotifier(new QTimer(this)),
            m_deviceStatus(new HDeviceStatus()),
            m_device(device)
{
    Q_ASSERT(m_device);
    //m_device->setParent(this);

    m_statusNotifier->setInterval(deviceTimeoutInSecs * 1000);
    bool ok = connect(
        m_statusNotifier.data(), SIGNAL(timeout()), this, SLOT(timeout_()));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

HServerDeviceController::~HServerDeviceController()
{
}

qint32 HServerDeviceController::deviceTimeoutInSecs() const
{
    return m_statusNotifier->interval() / 1000;
}

void HServerDeviceController::timeout_()
{
    HLOG(H_AT, H_FUN);

    m_timedout = true;
    stopStatusNotifier();

    emit statusTimeout(this);
}

void HServerDeviceController::startStatusNotifier()
{
    HLOG(H_AT, H_FUN);
    m_statusNotifier->start();
    m_timedout = false;
}

void HServerDeviceController::stopStatusNotifier()
{
    HLOG(H_AT, H_FUN);
    m_statusNotifier->stop();
}

bool HServerDeviceController::isTimedout() const
{
    return m_timedout;
}

/*******************************************************************************
 * HDeviceHostPrivate
 ******************************************************************************/
HDeviceHostPrivate::HDeviceHostPrivate() :
    QObject(),
        m_loggingIdentifier(
            QString("__DEVICE HOST %1__: ").arg(
                QUuid::createUuid().toString()).toLocal8Bit()),
        m_config           (),
        m_ssdps            (),
        m_httpServer       (0),
        m_eventNotifier    (0),
        m_presenceAnnouncer(0),
        m_runtimeStatus    (0),
        q_ptr(0),
        m_lastError(HDeviceHost::UndefinedError),
        m_initialized(false),
        m_deviceStorage(m_loggingIdentifier),
        m_nam(0)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    qsrand(time(0));
}

HDeviceHostPrivate::~HDeviceHostPrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

void HDeviceHostPrivate::announcementTimedout(
    HServerDeviceController* controller)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<ResourceAvailableAnnouncement> announcements;
    m_presenceAnnouncer->createAnnouncementMessagesForRootDevice(
        controller->m_device, controller->deviceTimeoutInSecs(), &announcements);

    m_presenceAnnouncer->sendAnnouncements(announcements);

    controller->startStatusNotifier();
}

bool HDeviceHostPrivate::createRootDevice(const HDeviceConfiguration* deviceconfig)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString baseDir = extractBaseUrl(deviceconfig->pathToDeviceDescription());

    DeviceHostDataRetriever dataRetriever(m_loggingIdentifier, baseDir);

    QString deviceDescr;
    if (!dataRetriever.retrieveDeviceDescription(
        deviceconfig->pathToDeviceDescription(), &deviceDescr))
    {
        m_lastError = HDeviceHost::InvalidConfigurationError;
        m_lastErrorDescription = dataRetriever.lastError();
        return false;
    }

    HServerModelCreationArgs creatorParams(m_config->deviceModelCreator());
    creatorParams.m_deviceDescription = deviceDescr;
    creatorParams.m_deviceLocations = m_httpServer->rootUrls();
    creatorParams.setDeviceDescriptionPostfix(deviceDescriptionPostFix());
    creatorParams.setInfoProvider(m_config->deviceModelInfoProvider());

    creatorParams.m_serviceDescriptionFetcher = ServiceDescriptionFetcher(
        &dataRetriever, &DeviceHostDataRetriever::retrieveServiceDescription);

    creatorParams.m_deviceTimeoutInSecs = deviceconfig->cacheControlMaxAge() / 2;
    // this timeout value instructs the device host to re-announce the
    // device presence well before the advertised cache-control value
    // expires.

    creatorParams.m_iconFetcher =
        IconFetcher(&dataRetriever, &DeviceHostDataRetriever::retrieveIcon);

    creatorParams.m_loggingIdentifier = m_loggingIdentifier;

    HServerModelCreator creator(creatorParams);
    QScopedPointer<HServerDevice> rootDevice(creator.createRootDevice());

    if (!rootDevice)
    {
        m_lastErrorDescription = creator.lastErrorDescription();

        switch (creator.lastError())
        {
            case HServerModelCreator::UndefinedTypeError:
            case HServerModelCreator::InvalidDeviceDescription:
                m_lastError = HDeviceHost::InvalidDeviceDescriptionError;
                break;
            case HServerModelCreator::UnimplementedAction:
            case HServerModelCreator::InvalidServiceDescription:
                m_lastError = HDeviceHost::InvalidServiceDescriptionError;
                break;
            default:
                m_lastError = HDeviceHost::UndefinedError;
                break;
        }

        return false;
    }

    Q_ASSERT(rootDevice);

    HServerDeviceController* controller =
        new HServerDeviceController(
            rootDevice.data(), creatorParams.m_deviceTimeoutInSecs, this);

    if (!m_deviceStorage.addRootDevice(rootDevice.data(), controller))
    {
        delete controller;
        m_lastError = HDeviceHost::ResourceConflict;
        m_lastErrorDescription = m_deviceStorage.lastError();
        return false;
    }

    rootDevice->setParent(this);
    connectSelfToServiceSignals(rootDevice.take());

    return true;
}

bool HDeviceHostPrivate::createRootDevices()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<const HDeviceConfiguration*> diParams =
        m_config->deviceConfigurations();

    foreach(const HDeviceConfiguration* deviceconfig, diParams)
    {
        if (!createRootDevice(deviceconfig))
        {
            return false;
        }
    }

    return true;
}

void HDeviceHostPrivate::connectSelfToServiceSignals(HServerDevice* device)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HServerServices services(device->services());
    for(qint32 i = 0; i < services.size(); ++i)
    {
        HServerService* service = services.at(i);
        bool ok = connect(
            service,
            SIGNAL(stateChanged(const Herqq::Upnp::HServerService*)),
            m_eventNotifier.data(),
            SLOT(stateChanged(const Herqq::Upnp::HServerService*)));

        Q_ASSERT(ok); Q_UNUSED(ok)
    }

    HServerDevices devices(device->embeddedDevices());
    for(qint32 i = 0; i < devices.size(); ++i)
    {
        connectSelfToServiceSignals(devices.at(i));
    }
}

void HDeviceHostPrivate::startNotifiers(HServerDeviceController* controller)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(controller);

    bool ok = connect(
        controller, SIGNAL(statusTimeout(HServerDeviceController*)),
        this, SLOT(announcementTimedout(HServerDeviceController*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    controller->startStatusNotifier();
}

void HDeviceHostPrivate::startNotifiers()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HServerDeviceController*> controllers = m_deviceStorage.controllers();
    foreach(HServerDeviceController* controller, controllers)
    {
        startNotifiers(controller);
    }
}

void HDeviceHostPrivate::stopNotifiers()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HServerDeviceController*> controllers = m_deviceStorage.controllers();
    foreach(HServerDeviceController* controller, controllers)
    {
        controller->stopStatusNotifier();
    }
}

/*******************************************************************************
 * HDeviceHost
 *******************************************************************************/
HDeviceHost::HDeviceHost(QObject* parent) :
    QObject(parent), h_ptr(new HDeviceHostPrivate())
{
    h_ptr->setParent(this);
    h_ptr->q_ptr = this;

    h_ptr->m_runtimeStatus.reset(new HDeviceHostRuntimeStatus());
    h_ptr->m_runtimeStatus->h_ptr->m_deviceHost = this;
}

HDeviceHost::~HDeviceHost()
{
    quit();
    delete h_ptr;
}

bool HDeviceHost::doInit()
{
    // default implementation does nothing
    return true;
}

void HDeviceHost::doQuit()
{
    // default implementation does nothing
}

bool HDeviceHost::acceptSubscription(
    HServerService* /*targetService*/, const HEndpoint& /*source*/, bool /*renewal*/)
{
    return true;
}

const HDeviceHostConfiguration* HDeviceHost::configuration() const
{
    return h_ptr->m_config.data();
}

const HDeviceHostRuntimeStatus* HDeviceHost::runtimeStatus() const
{
    return h_ptr->m_runtimeStatus.data();
}

void HDeviceHost::setError(DeviceHostError error, const QString& errorStr)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_lastError = error;
    h_ptr->m_lastErrorDescription = errorStr;
}

bool HDeviceHost::init(const HDeviceHostConfiguration& config)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be initialized in the thread in which "
        "it is currently located.");

    if (h_ptr->m_initialized)
    {
        setError(AlreadyInitializedError, "The device host is already initialized");
        return false;
    }

    if (!config.isValid())
    {
        setError(InvalidConfigurationError, "The provided configuration is not valid");
        return false;
    }

    bool ok = false;
    HLOG_INFO("DeviceHost Initializing.");

    h_ptr->m_config.reset(config.clone());

    h_ptr->m_eventNotifier.reset(
        new HEventNotifier(
            h_ptr->m_loggingIdentifier,
            *h_ptr->m_config,
            this));

    h_ptr->m_httpServer.reset(
        new HDeviceHostHttpServer(
            h_ptr->m_loggingIdentifier,
            HDeviceHostPrivate::deviceDescriptionPostFix(),
            h_ptr->m_deviceStorage,
            *h_ptr->m_eventNotifier, this));

    QList<QHostAddress> addrs = config.networkAddressesToUse();
    if (!h_ptr->m_httpServer->init(convertHostAddressesToEndpoints(addrs)))
    {
        setError(CommunicationsError, "Failed to initialize HTTP server");
        goto err;
    }
    else
    {
         if (!h_ptr->createRootDevices())
         {
             goto err;
         }

        foreach(const QHostAddress& ha, addrs)
        {
            HDeviceHostSsdpHandler* ssdp =
                new HDeviceHostSsdpHandler(
                    h_ptr->m_loggingIdentifier, h_ptr->m_deviceStorage, this);

            h_ptr->m_ssdps.append(ssdp);

            if (!ssdp->init(ha))
            {
                setError(CommunicationsError, "Failed to initialize SSDP");
                goto err;
            }
        }

        h_ptr->m_presenceAnnouncer.reset(
            new PresenceAnnouncer(
                h_ptr->m_ssdps,
                h_ptr->m_config->individualAdvertisementCount()));

        // allow the derived classes to perform their initialization routines
        // before the hosted devices are announced to the network and timers
        // are started. In addition, at this time no HTTP or SSDP requests
        // are served.

        ok = doInit();
        // continue only if the derived class succeeded in initializing itself
    }

    if (ok)
    {
        h_ptr->m_presenceAnnouncer->announce<ResourceAvailableAnnouncement>(
            h_ptr->m_deviceStorage.controllers());

        h_ptr->startNotifiers();

        h_ptr->m_initialized = true;

        HLOG_INFO("DeviceHost initialized.");
        return true;
    }

err:
    quit();
    HLOG_WARN("DeviceHost initialization failed");
    return false;
}

HDeviceHost::DeviceHostError HDeviceHost::error() const
{
    return h_ptr->m_lastError;
}

QString HDeviceHost::errorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

void HDeviceHost::quit()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be shutdown in the thread in which it is "
        "currently located.");

    if (!h_ptr->m_initialized)
    {
        return;
    }

    HLOG_INFO("Shutting down.");

    h_ptr->stopNotifiers();

    h_ptr->m_presenceAnnouncer->announce<ResourceUnavailableAnnouncement>(
        h_ptr->m_deviceStorage.controllers());

    h_ptr->m_httpServer->close();

    h_ptr->m_initialized = false;

    doQuit();

    h_ptr->m_presenceAnnouncer.reset(0);

    qDeleteAll(h_ptr->m_ssdps);
    h_ptr->m_ssdps.clear();

    h_ptr->m_httpServer.reset(0);
    h_ptr->m_eventNotifier.reset(0);
    h_ptr->m_config.reset(0);

    h_ptr->m_deviceStorage.clear();

    HLOG_INFO("Shut down.");
}

bool HDeviceHost::isStarted() const
{
    return h_ptr->m_initialized;
}

bool HDeviceHost::add(const HDeviceConfiguration& configuration)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        setError(NotStarted, "The device host is not started");
        return false;
    }
    else if (!configuration.isValid())
    {
        setError(InvalidConfigurationError, "The provided configuration is not valid");
        return false;
    }

    bool b = h_ptr->createRootDevice(&configuration);
    if (b)
    {
        HServerDeviceController* newController =
            h_ptr->m_deviceStorage.controllers().last();

        h_ptr->m_config->add(configuration);
        h_ptr->m_presenceAnnouncer->announce<ResourceAvailableAnnouncement>(newController);
        h_ptr->startNotifiers(newController);
    }
    return b;
}

HServerDevices HDeviceHost::rootDevices() const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The device host is not started");
        return HServerDevices();
    }

    return h_ptr->m_deviceStorage.rootDevices<HServerDevice>();
}

HServerDevice* HDeviceHost::device(const HUdn& udn, TargetDeviceType dts) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The device host is not started");
        return 0;
    }

    return h_ptr->m_deviceStorage.searchDeviceByUdn(udn, dts);
}

/*******************************************************************************
 * HDeviceHostRuntimeStatusPrivate
 ******************************************************************************/
HDeviceHostRuntimeStatusPrivate::HDeviceHostRuntimeStatusPrivate() :
    m_deviceHost(0)
{
}

/*******************************************************************************
 * HDeviceHostRuntimeStatus
 ******************************************************************************/
HDeviceHostRuntimeStatus::HDeviceHostRuntimeStatus() :
    h_ptr(new HDeviceHostRuntimeStatusPrivate())
{
}

HDeviceHostRuntimeStatus::~HDeviceHostRuntimeStatus()
{
    delete h_ptr;
}

QList<HEndpoint> HDeviceHostRuntimeStatus::ssdpEndpoints() const
{
    Q_ASSERT(h_ptr->m_deviceHost);

    QList<HEndpoint> retVal;
    foreach(HDeviceHostSsdpHandler* ssdp, h_ptr->m_deviceHost->h_ptr->m_ssdps)
    {
        retVal.append(ssdp->unicastEndpoint());
    }
    return retVal;
}

QList<HEndpoint> HDeviceHostRuntimeStatus::httpEndpoints() const
{
    Q_ASSERT(h_ptr->m_deviceHost);
    return h_ptr->m_deviceHost->h_ptr->m_httpServer->endpoints();
}

}
}
