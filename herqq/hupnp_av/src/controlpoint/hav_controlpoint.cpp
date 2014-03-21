/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hav_controlpoint.h"
#include "hav_controlpoint_p.h"
#include "hav_controlpoint_configuration.h"

#include "../mediaserver/hmediaserver_adapter.h"
#include "../mediaserver/hmediaserver_info.h"

#include "../mediarenderer/hmediarenderer_adapter.h"
#include "../mediarenderer/hmediarenderer_info.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HDiscoveryType>
#include <HUpnpCore/HControlPointConfiguration>

#include <QtNetwork/QHostAddress>

/*!
 * \defgroup hupnp_av_cp Control Point
 * \ingroup hupnp_av
 *
 * \brief This page discusses the use of HUPnPAv's ControlPoint functionality.
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAvControlPointPrivate
 ******************************************************************************/
HAvControlPointPrivate::HAvControlPointPrivate(
    const HControlPointConfiguration& cpConf, HAvControlPoint* avCp) :
        HControlPoint(cpConf, avCp),
            m_configuration(0)
{
    bool ok = connect(
        this,
        SIGNAL(rootDeviceOnline(Herqq::Upnp::HClientDevice*)),
        this,
        SLOT(rootDeviceOnline_(Herqq::Upnp::HClientDevice*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        this,
        SIGNAL(rootDeviceOffline(Herqq::Upnp::HClientDevice*)),
        this,
        SLOT(rootDeviceOffline_(Herqq::Upnp::HClientDevice*)));

    Q_ASSERT(ok);
}

HAvControlPointPrivate::~HAvControlPointPrivate()
{
    delete m_configuration;
}

bool HAvControlPointPrivate::acceptResource(
    const HDiscoveryType& usn, const HEndpoint& /*source*/)
{
    return true;
}

void HAvControlPointPrivate::rootDeviceOnline_(HClientDevice* device)
{
    HLOG(H_AT, H_FUN);

    HAvControlPoint* p = static_cast<HAvControlPoint*>(parent());

    HMediaServerAdapter* msAdapter = searchAdapter(m_mediaServers, device);
    if (msAdapter)
    {
        msAdapter->setDevice(device);
        subscribeEvents(device, VisitThisRecursively);
        emit p->mediaServerOnline(msAdapter);
        return;
    }

    HMediaRendererAdapter* mrAdapter = searchAdapter(m_mediaRenderers, device);
    if (mrAdapter)
    {
        mrAdapter->setDevice(device);
        subscribeEvents(device, VisitThisRecursively);
        emit p->mediaRendererOnline(mrAdapter);
        return;
    }

    if (device->info().deviceType().compare(
            HMediaServerInfo::supportedDeviceType(), HResourceType::Inclusive))
    {
        if (m_configuration->interestingDeviceTypes() &
                HAvControlPointConfiguration::MediaServer)
        {
            HMediaServerAdapter* mediaServer = new HMediaServerAdapter(this);
            if (!mediaServer->setDevice(device))
            {
                HLOG_DBG(QString("Discovered MediaServer failed validation: %1").arg(
                    mediaServer->lastErrorDescription()));

                delete mediaServer;
            }
            else
            {
                subscribeEvents(device, VisitThisRecursively);
                m_mediaServers.append(mediaServer);
                emit p->mediaServerOnline(mediaServer);
            }
        }
    }
    else if (device->info().deviceType().compare(
            HMediaRendererInfo::supportedDeviceType(), HResourceType::Inclusive))
    {
        if (m_configuration->interestingDeviceTypes() &
                HAvControlPointConfiguration::MediaRenderer)
        {
            HMediaRendererAdapter* mediaRenderer = new HMediaRendererAdapter(this);
            if (!mediaRenderer->setDevice(device))
            {
                HLOG_DBG(QString("Discovered MediaRenderer failed validation: %1").arg(
                    mediaRenderer->lastErrorDescription()));

                delete mediaRenderer;
            }
            else
            {
                subscribeEvents(device, VisitThisRecursively);
                m_mediaRenderers.append(mediaRenderer);
                emit p->mediaRendererOnline(mediaRenderer);
            }
        }
    }

    foreach(HClientDevice* embDev, device->embeddedDevices())
    {
        rootDeviceOnline_(embDev);
    }
}

void HAvControlPointPrivate::rootDeviceOffline_(HClientDevice* device)
{
    HLOG(H_AT, H_FUN);

    HAvControlPoint* p = static_cast<HAvControlPoint*>(parent());

    foreach(HMediaServerAdapter* server, m_mediaServers)
    {
        if (server->device()->rootDevice() == device)
        {
            emit p->mediaServerOffline(server);
        }
    }

    foreach(HMediaRendererAdapter* renderer, m_mediaRenderers)
    {
        if (renderer->device()->rootDevice() == device)
        {
            emit p->mediaRendererOffline(renderer);
        }
    }

    foreach(HClientDevice* embDev, device->embeddedDevices())
    {
        rootDeviceOffline_(embDev);
    }
}

namespace
{
HControlPointConfiguration* convert(const HAvControlPointConfiguration& arg)
{
    HControlPointConfiguration* retVal = new HControlPointConfiguration();
    retVal->setAutoDiscovery(arg.autoDiscovery());
    retVal->setDesiredSubscriptionTimeout(arg.desiredSubscriptionTimeout());
    retVal->setNetworkAddressesToUse(arg.networkAddressesToUse());
    retVal->setSubscribeToEvents(arg.subscribeToEvents());
    return retVal;
}
}

/*******************************************************************************
 * HAvControlPoint
 ******************************************************************************/
HAvControlPoint::HAvControlPoint(HAvControlPointPrivate& dd, QObject* parent) :
    QObject(parent), h_ptr(&dd)
{
}

HAvControlPoint::HAvControlPoint(QObject* parent) :
    QObject(parent), h_ptr(0)
{
    HAvControlPointConfiguration* avconf = new HAvControlPointConfiguration();
    avconf->setAutoDiscovery(false);
    avconf->setSubscribeToEvents(false);
    QScopedPointer<HControlPointConfiguration> cpconf(convert(*avconf));
    h_ptr = new HAvControlPointPrivate(*cpconf, this);
    h_ptr->m_configuration = avconf;
}

HAvControlPoint::HAvControlPoint(
    const HAvControlPointConfiguration& configuration, QObject* parent) :
        QObject(parent), h_ptr(0)
{
    HAvControlPointConfiguration* avconf = configuration.clone();
    avconf->setAutoDiscovery(false);
    avconf->setSubscribeToEvents(false);
    QScopedPointer<HControlPointConfiguration> cpconf(convert(*avconf));
    h_ptr = new HAvControlPointPrivate(*cpconf, this);
    h_ptr->m_configuration = avconf;
}

HAvControlPoint::~HAvControlPoint()
{
    delete h_ptr;
}

const HAvControlPointConfiguration* HAvControlPoint::configuration() const
{
    return h_ptr->m_configuration;
}

void HAvControlPoint::setError(
    HAvControlPointError error, const QString& errorDescr)
{
    h_ptr->setError(
        static_cast<HControlPoint::ControlPointError>(error), errorDescr);
}

bool HAvControlPoint::init()
{
    bool b = h_ptr->init();
    if (!b)
    {
        return false;
    }

    return h_ptr->scan(HDiscoveryType::createDiscoveryTypeForAllResources());
}

HAvControlPoint::HAvControlPointError HAvControlPoint::error() const
{
    switch(h_ptr->error())
    {
    case HControlPoint::UndefinedError:
        return UndefinedError;
    case HControlPoint::NotInitializedError:
        return NotInitializedError;
    case HControlPoint::AlreadyInitializedError:
        return AlreadyInitializedError;
    case HControlPoint::CommunicationsError:
        return CommunicationsError;
    case HControlPoint::InvalidArgumentError:
        return InvalidArgumentError;
    };

    return UndefinedError;
}

QString HAvControlPoint::errorDescription() const
{
    return h_ptr->errorDescription();
}

bool HAvControlPoint::isStarted() const
{
    return h_ptr->isStarted();
}

const HMediaRendererAdapters& HAvControlPoint::mediaRenderers() const
{
    return h_ptr->m_mediaRenderers;
}

HMediaRendererAdapter* HAvControlPoint::mediaRenderer(const HUdn& udn)
{
    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return 0;
    }

    foreach(HMediaRendererAdapter* renderer, h_ptr->m_mediaRenderers)
    {
        if (renderer->device()->info().udn() == udn)
        {
            return renderer;
        }
    }

    return 0;
}

const HMediaServerAdapters& HAvControlPoint::mediaServers() const
{
    return h_ptr->m_mediaServers;
}

HMediaServerAdapter* HAvControlPoint::mediaServer(const HUdn& udn)
{
    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return 0;
    }

    foreach(HMediaServerAdapter* server, h_ptr->m_mediaServers)
    {
        if (server->device()->info().udn() == udn)
        {
            return server;
        }
    }

    return 0;
}

bool HAvControlPoint::removeMediaServer(HMediaServerAdapter* mediaServer)
{
    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }

    HMediaServerAdapters::iterator it = h_ptr->m_mediaServers.begin();
    for(; it != h_ptr->m_mediaServers.end(); ++it)
    {
        if ((*it) == mediaServer)
        {
            delete *it;
            h_ptr->m_mediaServers.erase(it);
            return true;
        }
    }

    setError(InvalidArgumentError, "The specified media server was not found");
    return false;
}

bool HAvControlPoint::removeMediaRenderer(HMediaRendererAdapter* mediaRenderer)
{
    if (!isStarted())
    {
        setError(NotInitializedError, "The control point is not initialized");
        return false;
    }

    HMediaRendererAdapters::iterator it = h_ptr->m_mediaRenderers.begin();
    for(; it != h_ptr->m_mediaRenderers.end(); ++it)
    {
        if ((*it) == mediaRenderer)
        {
            delete *it;
            h_ptr->m_mediaRenderers.erase(it);
            return true;
        }
    }

    setError(InvalidArgumentError, "The specified media renderer was not found");
    return false;
}

void HAvControlPoint::quit()
{
    h_ptr->quit();
    qDeleteAll(h_ptr->m_mediaServers); h_ptr->m_mediaServers.clear();
    qDeleteAll(h_ptr->m_mediaRenderers); h_ptr->m_mediaRenderers.clear();
}

}
}
}
