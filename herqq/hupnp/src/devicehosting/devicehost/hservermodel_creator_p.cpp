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

#include "hservermodel_creator_p.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hserviceid.h"
#include "../../dataelements/hdeviceinfo.h"

#include "../../general/hupnp_global_p.h"
#include "../../general/hupnp_datatypes_p.h"

#include "../../devicemodel/hdevicemodel_infoprovider.h"

#include "../../devicemodel/server/hserverservice.h"
#include "../../devicemodel/server/hserverservice_p.h"
#include "../../devicemodel/server/hserverdevice_p.h"
#include "../../devicemodel/server/hdevicemodelcreator.h"
#include "../../devicemodel/server/hdefault_serverdevice_p.h"
#include "../../devicemodel/server/hdefault_serveraction_p.h"
#include "../../devicemodel/server/hdefault_serverstatevariable_p.h"

#include "../../devicemodel/hactionarguments.h"
#include "../../devicemodel/hactions_setupdata.h"
#include "../../devicemodel/hdevices_setupdata.h"
#include "../../devicemodel/hservices_setupdata.h"
#include "../../devicemodel/hstatevariables_setupdata.h"

#include "../../general/hlogger_p.h"

#include <QtXml/QDomElement>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServerModelCreationArgs
 ******************************************************************************/
HServerModelCreationArgs::HServerModelCreationArgs(
    HDeviceModelCreator* creator) :
        m_deviceModelCreator(creator), m_infoProvider(0), m_ddPostFix()
{
}

HServerModelCreationArgs::~HServerModelCreationArgs()
{
}

HServerModelCreationArgs::HServerModelCreationArgs(
    const HServerModelCreationArgs& other) :
        HModelCreationArgs(other),
            m_deviceModelCreator(other.m_deviceModelCreator),
            m_infoProvider(other.m_infoProvider),
            m_ddPostFix(other.m_ddPostFix)
{
}

HServerModelCreationArgs& HServerModelCreationArgs::operator=(
    const HServerModelCreationArgs& other)
{
    Q_ASSERT(this != &other);

    HModelCreationArgs::operator=(other);

    m_deviceModelCreator = other.m_deviceModelCreator;
    m_infoProvider = other.m_infoProvider;
    m_ddPostFix = other.m_ddPostFix;

    return *this;
}

/*******************************************************************************
 * HServerModelCreator
 ******************************************************************************/
HServerModelCreator::HServerModelCreator(
    const HServerModelCreationArgs& creationParameters) :
        m_creationParameters(new HServerModelCreationArgs(creationParameters)),
        m_docParser(creationParameters.m_loggingIdentifier, StrictChecks),
        m_lastErrorDescription()
{
    Q_ASSERT(creationParameters.m_serviceDescriptionFetcher);
    Q_ASSERT(creationParameters.m_deviceLocations.size() > 0);
    Q_ASSERT(creationParameters.m_iconFetcher);
    Q_ASSERT(!creationParameters.m_loggingIdentifier.isEmpty());
    Q_ASSERT(!creationParameters.deviceDescriptionPostfix().isEmpty());
}

HStateVariablesSetupData HServerModelCreator::getStateVariablesSetupData(
    HServerService* service)
{
    if (m_creationParameters->infoProvider())
    {
        return m_creationParameters->infoProvider()->stateVariablesSetupData(
            service->info(), service->parentDevice()->info());
    }
    return HStateVariablesSetupData();
}

HActionsSetupData HServerModelCreator::getActionsSetupData(
    HServerService* service)
{
    if (m_creationParameters->infoProvider())
    {
        return m_creationParameters->infoProvider()->actionsSetupData(
            service->info(), service->parentDevice()->info());
    }
    return HActionsSetupData();
}

HServicesSetupData HServerModelCreator::getServicesSetupData(
    HServerDevice* device)
{
    if (m_creationParameters->infoProvider())
    {
        return m_creationParameters->infoProvider()->servicesSetupData(
            device->info());
    }
    return HServicesSetupData();
}

HDevicesSetupData HServerModelCreator::getDevicesSetupData(
    HServerDevice* device)
{
    if (m_creationParameters->infoProvider())
    {
        return m_creationParameters->infoProvider()->embedddedDevicesSetupData(
            device->info());
    }
    return HDevicesSetupData();
}

bool HServerModelCreator::parseStateVariables(
    HServerService* service, QDomElement stateVariableElement)
{
    HStateVariablesSetupData stateVariablesSetup =
        getStateVariablesSetupData(service);

    while(!stateVariableElement.isNull())
    {
        HStateVariableInfo svInfo;
        if (!m_docParser.parseStateVariable(stateVariableElement, &svInfo))
        {
            m_lastError = convert(m_docParser.lastError());
            m_lastErrorDescription = m_docParser.lastErrorDescription();
            return false;
        }

        QString name = svInfo.name();
        HStateVariableInfo setupData = stateVariablesSetup.get(name);
        if (!setupData.isValid() &&
            stateVariablesSetup.defaultInclusionPolicy() ==
            HStateVariablesSetupData::Deny)
        {
            m_lastError = InvalidServiceDescription;
            m_lastErrorDescription = QString(
                "Implementation of service [%1] does not support state variable"
                "[%2]").arg(service->info().serviceId().toString(), name);

            return false;
        }

        HDeviceValidator validator;
        if (!validator.validate(stateVariablesSetup.get(name), svInfo))
        {
            m_lastError = InvalidServiceDescription;
            m_lastErrorDescription =
                QString("Service [%1] validation error: %2").arg(
                    service->info().serviceId().toString(),
                    validator.lastErrorDescription());

            return false;
        }

        HDefaultServerStateVariable* sv =
            new HDefaultServerStateVariable(svInfo, service);

        service->h_ptr->addStateVariable(sv);

        bool ok = QObject::connect(
            sv,
            SIGNAL(valueChanged(
                Herqq::Upnp::HServerStateVariable*,
                const Herqq::Upnp::HStateVariableEvent&)),
            service,
            SLOT(notifyListeners()));

        Q_ASSERT(ok); Q_UNUSED(ok)

        stateVariableElement =
            stateVariableElement.nextSiblingElement("stateVariable");

        stateVariablesSetup.remove(name);
    }

    if (!stateVariablesSetup.isEmpty())
    {
        foreach(const QString& name, stateVariablesSetup.names())
        {
            HStateVariableInfo svSetup = stateVariablesSetup.get(name);
            if (svSetup.inclusionRequirement() == InclusionMandatory &&
                svSetup.version() <= service->info().serviceType().version())
            {
                m_lastError = InvalidServiceDescription;
                m_lastErrorDescription = QString(
                    "Service description is missing a mandatory state variable "
                    "[%1]").arg(name);

                return false;
            }
        }
    }

    return true;
}

bool HServerModelCreator::parseActions(
    HServerService* service, QDomElement actionElement,
    const HStateVariableInfos& svInfos)
{
    HActionsSetupData actionsSetupData = getActionsSetupData(service);

    QHash<QString, HActionInvoke> actionInvokes = service->createActionInvokes();

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

        QString name = actionInfo.name();

        HActionInvoke actionInvoke = actionInvokes.value(name);
        if (!actionInvoke)
        {
            m_lastError = UnimplementedAction;
            m_lastErrorDescription = QString(
                "Service [%1]: action [%2] lacks an implementation").arg(
                    service->info().serviceId().toString(), name);

            return false;
        }

        QScopedPointer<HDefaultServerAction> action(
            new HDefaultServerAction(actionInfo, actionInvoke, service));

        HDeviceValidator validator;
        if (!validator.validate(actionsSetupData.get(name), actionInfo))
        {
            m_lastError = InvalidServiceDescription;
            m_lastErrorDescription =
                QString("Service [%1] validation error: %2").arg(
                    service->info().serviceId().toString(),
                    validator.lastErrorDescription());

            return false;
        }

        service->h_ptr->m_actions.insert(name, action.take());

        actionElement = actionElement.nextSiblingElement("action");

        actionsSetupData.remove(name);
    }

    if (!actionsSetupData.isEmpty())
    {
        foreach(const QString& name, actionsSetupData.names())
        {
            HActionSetup setupInfo = actionsSetupData.get(name);
            if (setupInfo.inclusionRequirement() == InclusionMandatory &&
                setupInfo.version() <= service->info().serviceType().version())
            {
                m_lastError = InvalidServiceDescription;
                m_lastErrorDescription = QString(
                    "Service description for [%1] is missing a mandatory action "
                    "[%2]").arg(service->info().serviceId().toString(), name);

                return false;
            }
        }
    }

    return true;
}

bool HServerModelCreator::parseServiceDescription(HServerService* service)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);
    Q_ASSERT(service);

    QDomDocument doc;
    QDomElement firstSv, firstAction;
    if (!m_docParser.parseServiceDescription(
        service->h_ptr->m_serviceDescription, &doc, &firstSv, &firstAction))
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
    HServerStateVariables svs = service->stateVariables();
    foreach(const QString& key, svs.keys())
    {
        svInfos.insert(key, svs.value(key)->info());
    }

    return parseActions(service, firstAction, svInfos);
}

bool HServerModelCreator::parseServiceList(
    const QDomElement& serviceListElement, HServerDevice* device,
    QList<HServerService*>* retVal)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    Q_ASSERT(device);
    Q_ASSERT(!serviceListElement.isNull());

    QDomElement serviceElement =
        serviceListElement.firstChildElement("service");

    HServicesSetupData setupData = getServicesSetupData(device);

    while(!serviceElement.isNull())
    {
        HServiceInfo info;
        if (!m_docParser.parseServiceInfo(serviceElement, &info))
        {
            m_lastError = convert(m_docParser.lastError());
            m_lastErrorDescription = m_docParser.lastErrorDescription();
            return false;
        }

        QScopedPointer<HServerService> service(
            m_creationParameters->creator()->createService(info, device->info()));

        if (!service)
        {
            m_lastError = UndefinedTypeError;
            m_lastErrorDescription =
                QString("No object for service type [%1] was created").arg(
                    info.serviceType().toString());

            return 0;
        }

        if (!service->init(info, device))
        {
            m_lastError = InitializationError;
            m_lastErrorDescription =
                QString("Failed to initialize service [%1]").arg(
                    info.serviceId().toString());

            return false;
        }

        if (!m_creationParameters->m_serviceDescriptionFetcher(
                extractBaseUrl(m_creationParameters->m_deviceLocations[0]),
                info.scpdUrl(), &service->h_ptr->m_serviceDescription))
        {
            m_lastError = FailedToGetDataError;
            m_lastErrorDescription = QString(
                "Could not retrieve service description from [%1]").arg(
                    info.scpdUrl().toString());

            return false;
        }

        if (!parseServiceDescription(service.data()))
        {
            return false;
        }

        QString errDescr;
        bool ok = service->finalizeInit(&errDescr);
        if (!ok)
        {
            m_lastError = InitializationError;
            m_lastErrorDescription = errDescr;
            return false;
        }

        retVal->push_back(service.take());

        serviceElement = serviceElement.nextSiblingElement("service");

        setupData.remove(info.serviceId());
    }

    if (!setupData.isEmpty())
    {
        foreach(const HServiceId& id, setupData.serviceIds())
        {
            HServiceSetup setupInfo = setupData.get(id);
            if (setupInfo.inclusionRequirement() == InclusionMandatory &&
                setupInfo.version() <= device->info().deviceType().version())
            {
                m_lastError = InvalidServiceDescription;
                m_lastErrorDescription = QString(
                    "Device description for [%1] is missing a mandatory service "
                    "[%2]").arg(device->info().deviceType().toString(),
                                id.toString());

                return false;
            }
        }
    }

    return true;
}

namespace
{
QList<QUrl> generateLocations(
    const HUdn& udn, const QList<QUrl>& locations, const QString& ddPostFix)
{
    QList<QUrl> retVal;
    foreach(const QUrl& location, locations)
    {
        QString locStr = location.toString();
        if (!locStr.endsWith('/'))
        {
            locStr.append(QString("/%1/%2").arg(udn.toSimpleUuid(), ddPostFix));
        }

        retVal.append(locStr);
    }

    return retVal;
}
}

HServerDevice* HServerModelCreator::parseDevice(
    const QDomElement& deviceElement, HServerDevice* parentDevice)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    HDeviceInfo deviceInfo;
    if (!m_docParser.parseDeviceInfo(deviceElement, &deviceInfo))
    {
        m_lastError = convert(m_docParser.lastError());
        m_lastErrorDescription = m_docParser.lastErrorDescription();
        return 0;
    }

    QScopedPointer<HServerDevice> device(
        m_creationParameters->creator()->createDevice(deviceInfo));

    if (!device.data())
    {
        HLOG_DBG(QString("Creating default device for [%1]").arg(
            deviceInfo.deviceType().toString()));

        device.reset(new HDefaultServerDevice());
    }

    if (!device->init(deviceInfo, parentDevice))
    {
        m_lastError = InitializationError;
        m_lastErrorDescription =
            QString("Failed to initialize device [%1]").arg(
                deviceInfo.udn().toString());

        return false;
    }

    device->h_ptr->m_deviceDescription =
        m_creationParameters->m_deviceDescription;

    QDomElement serviceListElement =
        deviceElement.firstChildElement("serviceList");

    if (!serviceListElement.isNull())
    {
        HServerServices services;
        if (!parseServiceList(serviceListElement, device.data(), &services))
        {
            qDeleteAll(services);
            return 0;
        }

        device->h_ptr->m_services = services;
    }

    HDevicesSetupData setupData = getDevicesSetupData(device.data());

    QDomElement deviceListElement = deviceElement.firstChildElement("deviceList");
    if (!deviceListElement.isNull())
    {
        QList<HServerDevice*> embeddedDevices;

        QDomElement embeddedDeviceElement =
            deviceListElement.firstChildElement("device");

        while(!embeddedDeviceElement.isNull())
        {
            HServerDevice* embeddedDevice =
                parseDevice(embeddedDeviceElement, device.data());

            if (!embeddedDevice)
            {
                qDeleteAll(embeddedDevices);
                return 0;
            }

            embeddedDevices.push_back(embeddedDevice);

            embeddedDeviceElement =
                embeddedDeviceElement.nextSiblingElement("device");
        }

        device->h_ptr->m_embeddedDevices = embeddedDevices;

        setupData.remove(deviceInfo.deviceType());
    }

    if (!setupData.isEmpty())
    {
        foreach(const HResourceType& dt, setupData.deviceTypes())
        {
            HDeviceSetup setupInfo = setupData.get(dt);
            if (setupInfo.inclusionRequirement() == InclusionMandatory &&
                setupInfo.version() <= device->info().deviceType().version())
            {
                m_lastError = InvalidServiceDescription;
                m_lastErrorDescription = QString(
                    "Device description for [%1] is missing a mandatory embedded device "
                    "[%2]").arg(device->info().deviceType().toString(),
                                dt.toString());

                return false;
            }
        }
    }

    if (!device->finalizeInit(&m_lastErrorDescription))
    {
        m_lastError = InitializationError;
        return 0;
    }

    return device.take();
}

HServerDevice* HServerModelCreator::createRootDevice()
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

    QScopedPointer<HServerDevice> createdDevice(parseDevice(rootElement, 0));
    if (!createdDevice)
    {
        return 0;
    }

    createdDevice->h_ptr->m_deviceStatus.reset(new HDeviceStatus());
    createdDevice->h_ptr->m_deviceStatus->setConfigId(
        m_docParser.readConfigId(rootElement));

    createdDevice->h_ptr->m_locations =
        generateLocations(
            createdDevice->info().udn(),
            m_creationParameters->m_deviceLocations,
            m_creationParameters->deviceDescriptionPostfix());

    HDeviceValidator validator;
    if (!validator.validateRootDevice<HServerDevice, HServerService>(
            createdDevice.data()))
    {
        m_lastError = convert(validator.lastError());
        m_lastErrorDescription = validator.lastErrorDescription();
        return 0;
    }

    return createdDevice.take();
}

}
}
