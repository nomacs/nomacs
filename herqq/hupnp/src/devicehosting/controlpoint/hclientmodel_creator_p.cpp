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

#include "hclientmodel_creator_p.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hserviceid.h"
#include "../../dataelements/hdeviceinfo.h"

#include "../../general/hupnp_global_p.h"
#include "../../general/hupnp_datatypes_p.h"

#include "../../devicemodel/client/hclientservice_p.h"
#include "../../devicemodel/client/hclientdevice_p.h"
#include "../../devicemodel/client/hdefault_clientaction_p.h"
#include "../../devicemodel/client/hdefault_clientdevice_p.h"
#include "../../devicemodel/client/hdefault_clientservice_p.h"
#include "../../devicemodel/client/hdefault_clientstatevariable_p.h"

#include "../../devicemodel/hactionarguments.h"

#include "../../general/hlogger_p.h"

#include <QtXml/QDomElement>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientModelCreationArgs
 ******************************************************************************/
HClientModelCreationArgs::HClientModelCreationArgs(QNetworkAccessManager* nam) :
    m_nam(nam)
{
}

HClientModelCreationArgs::~HClientModelCreationArgs()
{
}

HClientModelCreationArgs::HClientModelCreationArgs(
    const HClientModelCreationArgs& other) :
        HModelCreationArgs(other),
            m_nam(other.m_nam)
{
}

HClientModelCreationArgs& HClientModelCreationArgs::operator=(
    const HClientModelCreationArgs& other)
{
    Q_ASSERT(this != &other);
    HModelCreationArgs::operator=(other);
    m_nam = other.m_nam;
    return *this;
}

/*******************************************************************************
 * HClientModelCreator
 ******************************************************************************/
HClientModelCreator::HClientModelCreator(
    const HClientModelCreationArgs& creationParameters) :
        m_creationParameters(new HClientModelCreationArgs(creationParameters)),
        m_docParser(creationParameters.m_loggingIdentifier, LooseChecks)
{
    Q_ASSERT(creationParameters.m_serviceDescriptionFetcher);
    Q_ASSERT(creationParameters.m_deviceLocations.size() > 0);
    Q_ASSERT(creationParameters.m_iconFetcher);
    Q_ASSERT(!creationParameters.m_loggingIdentifier.isEmpty());
}

bool HClientModelCreator::parseStateVariables(
    HDefaultClientService* service, QDomElement stateVariableElement)
{
    while(!stateVariableElement.isNull())
    {
        HStateVariableInfo svInfo;
        if (!m_docParser.parseStateVariable(stateVariableElement, &svInfo))
        {
            m_lastError = convert(m_docParser.lastError());
            m_lastErrorDescription = m_docParser.lastErrorDescription();
            return false;
        }

        HDefaultClientStateVariable* sv =
            new HDefaultClientStateVariable(svInfo, service);

        service->addStateVariable(sv);

        bool ok = QObject::connect(
            sv,
            SIGNAL(valueChanged(
                const Herqq::Upnp::HClientStateVariable*,
                const Herqq::Upnp::HStateVariableEvent&)),
            service,
            SLOT(notifyListeners()));

        Q_ASSERT(ok); Q_UNUSED(ok)

        Q_ASSERT(ok); Q_UNUSED(ok)

        stateVariableElement =
            stateVariableElement.nextSiblingElement("stateVariable");
    }

    return true;
}

bool HClientModelCreator::parseActions(
    HDefaultClientService* service, QDomElement actionElement,
    const HStateVariableInfos& svInfos)
{
    while(!actionElement.isNull())
    {
        HActionInfo actionInfo;
        if (!m_docParser.parseActionInfo(
                actionElement, svInfos, &actionInfo))
        {
            m_lastError = convert(m_docParser.lastError());
            m_lastErrorDescription = m_docParser.lastErrorDescription();
            return false;
        }

        HDefaultClientAction* action =
            new HDefaultClientAction(
                actionInfo,
                service,
                *m_creationParameters->m_nam);

        QString name = action->info().name();

        service->addAction(action);

        actionElement = actionElement.nextSiblingElement("action");
    }

    return true;
}

bool HClientModelCreator::parseServiceDescription(HDefaultClientService* service)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);
    Q_ASSERT(service);

    QDomDocument doc;
    QDomElement firstSv, firstAction;
    if (!m_docParser.parseServiceDescription(
        service->description(), &doc, &firstSv, &firstAction))
    {
        m_lastError = convert(m_docParser.lastError());
        m_lastErrorDescription = m_docParser.lastErrorDescription();
        return false;
    }

    if (!parseStateVariables(service, firstSv))
    {
        return false;
    }

    HStateVariableInfos svInfos;
    HClientStateVariables svs = service->stateVariables();
    foreach(const QString& key, svs.keys())
    {
        svInfos.insert(key, svs.value(key)->info());
    }

    return parseActions(service, firstAction, svInfos);
}

bool HClientModelCreator::parseServiceList(
    const QDomElement& serviceListElement, HDefaultClientDevice* device,
    QList<HDefaultClientService*>* retVal)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    Q_ASSERT(device);
    Q_ASSERT(!serviceListElement.isNull());

    QDomElement serviceElement = serviceListElement.firstChildElement("service");
    while(!serviceElement.isNull())
    {
        HServiceInfo info;
        if (!m_docParser.parseServiceInfo(serviceElement, &info))
        {
            m_lastError = convert(m_docParser.lastError());
            m_lastErrorDescription = m_docParser.lastErrorDescription();
            return false;
        }

        QScopedPointer<HDefaultClientService> service(
            new HDefaultClientService(info, device));

        QString description;
        if (!m_creationParameters->m_serviceDescriptionFetcher(
                extractBaseUrl(m_creationParameters->m_deviceLocations[0]),
                info.scpdUrl(), &description))
        {
            m_lastError = FailedToGetDataError;
            m_lastErrorDescription = QString(
                "Could not retrieve service description from [%1]").arg(
                    info.scpdUrl().toString());

            return false;
        }

        service->setDescription(description);
        if (!parseServiceDescription(service.data()))
        {
            return false;
        }

        retVal->push_back(service.take());

        serviceElement = serviceElement.nextSiblingElement("service");
    }

    return true;
}

HDefaultClientDevice* HClientModelCreator::parseDevice(
    const QDomElement& deviceElement, HDefaultClientDevice* parentDevice)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    HDeviceInfo deviceInfo;
    if (!m_docParser.parseDeviceInfo(deviceElement, &deviceInfo))
    {
        m_lastError = convert(m_docParser.lastError());
        m_lastErrorDescription = m_docParser.lastErrorDescription();
        return 0;
    }

    QScopedPointer<HDefaultClientDevice> device(
        new HDefaultClientDevice(
            m_creationParameters->m_deviceDescription,
            m_creationParameters->m_deviceLocations,
            deviceInfo,
            m_creationParameters->m_deviceTimeoutInSecs,
            parentDevice));

    QDomElement serviceListElement =
        deviceElement.firstChildElement("serviceList");

    if (!serviceListElement.isNull())
    {
        QList<HDefaultClientService*> services;
        if (!parseServiceList(serviceListElement, device.data(), &services))
        {
            return 0;
        }
        device->setServices(services);
    }

    QDomElement deviceListElement = deviceElement.firstChildElement("deviceList");
    if (!deviceListElement.isNull())
    {
        QList<HDefaultClientDevice*> embeddedDevices;

        QDomElement embeddedDeviceElement =
            deviceListElement.firstChildElement("device");

        while(!embeddedDeviceElement.isNull())
        {
            HDefaultClientDevice* embeddedDevice =
                parseDevice(embeddedDeviceElement, device.data());

            if (!embeddedDevice)
            {
                return 0;
            }

            embeddedDevice->setParent(device.data());

            embeddedDevices.push_back(embeddedDevice);

            embeddedDeviceElement =
                embeddedDeviceElement.nextSiblingElement("device");
        }

        device->setEmbeddedDevices(embeddedDevices);
    }

    return device.take();
}

HDefaultClientDevice* HClientModelCreator::createRootDevice()
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    QDomDocument doc;
    QDomElement rootElement;
    if (!m_docParser.parseRoot(
            m_creationParameters->m_deviceDescription, &doc, &rootElement))
    {
        m_lastError = convert(m_docParser.lastError());
        m_lastErrorDescription = m_docParser.lastErrorDescription();
        return 0;
    }

    QScopedPointer<HDefaultClientDevice> createdDevice(
        parseDevice(rootElement, 0));

    if (!createdDevice)
    {
        return 0;
    }

    createdDevice->setConfigId(m_docParser.readConfigId(rootElement));

    HDeviceValidator validator;
    if (!validator.validateRootDevice<HClientDevice, HClientService>(createdDevice.data()))
    {
        m_lastError = convert(validator.lastError());
        m_lastErrorDescription = validator.lastErrorDescription();
        return 0;
    }

    return createdDevice.take();
}

}
}
