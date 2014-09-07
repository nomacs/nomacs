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

#include "hdevicemodel_validator.h"
#include "hdevicemodel_validator_p.h"
#include "hdevicemodel_infoprovider.h"

#include "client/hclientaction.h"
#include "client/hclientdevice.h"
#include "client/hclientservice.h"
#include "client/hclientstatevariable.h"

#include "server/hserveraction.h"
#include "server/hserverdevice.h"
#include "server/hserverservice.h"
#include "server/hserverstatevariable.h"

#include "../dataelements/hactioninfo.h"
#include "../dataelements/hdeviceinfo.h"
#include "../dataelements/hserviceinfo.h"
#include "../dataelements/hstatevariableinfo.h"

#include "hactionarguments.h"
#include "hactions_setupdata.h"
#include "hdevices_setupdata.h"
#include "hservices_setupdata.h"
#include "hstatevariables_setupdata.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceModelValidatorPrivate
 ******************************************************************************/
HDeviceModelValidatorPrivate::HDeviceModelValidatorPrivate() :
    m_lastErrorDescription(), m_infoProvider(0)
{
}

template<typename Device, typename Service, typename Action, typename StateVariable>
HDeviceModelValidator::ValidationResult
    HDeviceModelValidatorPrivate::validateDevice(Device* device)
{
    const HDeviceInfo& deviceInfo = device->info();

    qint32 deviceVersion = deviceInfo.deviceType().version();

    HServicesSetupData servicesSetupData =
        m_infoProvider->servicesSetupData(deviceInfo);

    foreach(const HServiceId& srvId, servicesSetupData.serviceIds())
    {
        HServiceSetup serviceSetup = servicesSetupData.get(srvId);

        if (deviceVersion >= serviceSetup.version())
        {
            Service* service = device->serviceById(srvId);
            if (!service)
            {
                if (serviceSetup.inclusionRequirement() == InclusionMandatory)
                {
                    m_lastErrorDescription =
                        QString("Mandatory service [%1] is missing").arg(
                            srvId.toString());

                    return HDeviceModelValidator::MandatoryServiceMissing;
                }
            }
            else
            {
                HDeviceModelValidator::ValidationResult res =
                    validateService<Service, Action, StateVariable>(service);

                if (res != HDeviceModelValidator::ValidationSucceeded)
                {
                    return res;
                }
            }
        }
    }

    HDevicesSetupData embDevSetupData =
        m_infoProvider->embedddedDevicesSetupData(deviceInfo);

    foreach(const HResourceType& devType, embDevSetupData.deviceTypes())
    {
        HDeviceSetup deviceSetup = embDevSetupData.get(devType);

        if (deviceVersion >= deviceSetup.version())
        {
            QList<Device*> devices = device->embeddedDevicesByType(devType);
            if (devices.isEmpty() &&
                deviceSetup.inclusionRequirement() == InclusionMandatory)
            {
                m_lastErrorDescription =
                    QString("Mandatory device [%1] is missing").arg(
                        devType.toString());

                return HDeviceModelValidator::MandatoryDeviceMissing;
            }

            foreach(Device* device, devices)
            {
                HDeviceModelValidator::ValidationResult res =
                    validateDevice<Device, Service, Action, StateVariable>(device);

                if (res != HDeviceModelValidator::ValidationSucceeded)
                {
                    return res;
                }
            }
        }
    }

    return HDeviceModelValidator::ValidationSucceeded;
}

template<typename Service, typename Action, typename StateVariable>
HDeviceModelValidator::ValidationResult
    HDeviceModelValidatorPrivate::validateService(Service* service)
{
    const HServiceInfo& serviceInfo = service->info();
    const HDeviceInfo& deviceInfo = service->parentDevice()->info();

    qint32 serviceVersion = serviceInfo.serviceType().version();

    HActionsSetupData actionSetupData =
        m_infoProvider->actionsSetupData(serviceInfo, deviceInfo);

    // All the action setup elements will be checked. If there exists an action,
    // but the info provider does not return an action setup information of it,
    // the action is considered valid.

    foreach(const QString& actionName, actionSetupData.names())
    {
        HActionSetup actionSetup = actionSetupData.get(actionName);

        if (serviceVersion >= actionSetup.version())
        {
            // Validate only if the version of the service is equal or greater
            // to the version defined in the setup data. If the service version
            // is smaller then the action was not defined in that version at all,
            // but that is not an error. This only means that when interacting with
            // this service version, this action should never be called.
            //
            // However, this version check is just an extra sanity check, since a
            // properly functioning "info provider" should check the version of
            // the provided service info and return information considering that.

            const Action* action = service->actions().value(actionName);

            if (!action)
            {
                if (actionSetup.inclusionRequirement() == InclusionMandatory)
                {
                    m_lastErrorDescription =
                        QString("Mandatory action [%1] is missing").arg(
                            actionName);

                    return HDeviceModelValidator::MandatoryActionMissing;
                }
            }
            else
            {
                HDeviceModelValidator::ValidationResult res =
                    validateAction(action, actionSetup);

                if (res != HDeviceModelValidator::ValidationSucceeded)
                {
                    m_lastErrorDescription =
                        QString("Action [%1] failed validation: %2").arg(
                            actionName, m_lastErrorDescription);

                    return res;
                }
            }
        }
    }

    HStateVariablesSetupData svData =
        m_infoProvider->stateVariablesSetupData(serviceInfo, deviceInfo);

    foreach(const QString& svName, svData.names())
    {
        HStateVariableInfo svInfo = svData.get(svName);

        if (serviceVersion >= svInfo.version())
        {
            const StateVariable* stateVar =
                service->stateVariables().value(svName);

            if (!stateVar)
            {
                if (svInfo.inclusionRequirement() == InclusionMandatory)
                {
                    m_lastErrorDescription =
                        QString("Mandatory state variable [%1] is missing").arg(
                            svName);

                    return HDeviceModelValidator::MandatoryStateVariableMissing;
                }
            }
            else
            {
                HDeviceModelValidator::ValidationResult res =
                    validateStateVariable(stateVar, svInfo);

                if (res != HDeviceModelValidator::ValidationSucceeded)
                {
                    m_lastErrorDescription =
                        QString("State Variable [%1] failed validation: %2").arg(
                            svName, m_lastErrorDescription);

                    return res;
                }
            }
        }
    }

    return HDeviceModelValidator::ValidationSucceeded;
}

template<typename Action>
HDeviceModelValidator::ValidationResult
    HDeviceModelValidatorPrivate::validateAction(
        Action* action, const HActionSetup& actionSetup)
{
    HActionArguments inArgs = action->info().inputArguments();
    HActionArguments outArgs = action->info().outputArguments();

    HActionArguments inArgsSetup = actionSetup.inputArguments();
    HActionArguments outArgsSetup = actionSetup.outputArguments();

    foreach(const QString& argName, inArgsSetup.names())
    {
        HActionArgument argSetup = inArgsSetup.get(argName);
        HActionArgument arg = inArgs.get(argName);

        if (!arg.isValid())
        {
            m_lastErrorDescription =
                QString("Mandatory action argument [%1] is missing").arg(
                    argName);

            return HDeviceModelValidator::MandatoryActionArgumentMissing;
        }
        else if (arg.dataType() != argSetup.dataType())
        {
            m_lastErrorDescription =
                QString("Invalid data type specified in action argument [%1]").arg(
                    argName);

            return HDeviceModelValidator::InvalidArgumentDefinition;
        }
    }

    foreach(const QString& argName, outArgsSetup.names())
    {
        HActionArgument argSetup = outArgsSetup.get(argName);
        HActionArgument arg = outArgs.get(argName);

        if (!arg.isValid())
        {
            m_lastErrorDescription =
                QString("Mandatory action argument [%1] is missing").arg(
                    argName);

            return HDeviceModelValidator::MandatoryActionArgumentMissing;
        }
        else if (arg.dataType() != argSetup.dataType())
        {
            m_lastErrorDescription =
                QString("Invalid data type specified in action argument [%1]").arg(
                    argName);

            return HDeviceModelValidator::InvalidArgumentDefinition;
        }
    }

    return HDeviceModelValidator::ValidationSucceeded;
}

template<typename Action>
HDeviceModelValidator::ValidationResult
    HDeviceModelValidatorPrivate::validateAction(Action* action)
{
    Q_ASSERT_X(action, "", "Null pointer provided");

    const HServiceInfo& serviceInfo = action->parentService()->info();
    const HDeviceInfo& deviceInfo   =
        action->parentService()->parentDevice()->info();

    HActionsSetupData actionSetupData =
        m_infoProvider->actionsSetupData(serviceInfo, deviceInfo);

    HActionSetup actionSetup = actionSetupData.get(action->info().name());

    return actionSetup.isValid() ?
        validateAction(action, actionSetup) :
        HDeviceModelValidator::ValidationSucceeded;
}

template<typename StateVariable>
HDeviceModelValidator::ValidationResult
    HDeviceModelValidatorPrivate::validateStateVariable(
        StateVariable* stateVar, const HStateVariableInfo& svInfo)
{
    if (stateVar->info().dataType() != svInfo.dataType())
    {
        m_lastErrorDescription =
            QString("Invalid data type specified. Was [%1], expected [%2].").arg(
                HUpnpDataTypes::toString(stateVar->info().dataType()),
                HUpnpDataTypes::toString(svInfo.dataType()));

        return HDeviceModelValidator::InvalidStateVariableDefinition;
    }

    return HDeviceModelValidator::ValidationSucceeded;
}

template<typename StateVariable>
HDeviceModelValidator::ValidationResult
    HDeviceModelValidatorPrivate::validateStateVariable(
        StateVariable* stateVar)
{
    Q_ASSERT_X(stateVar, "", "Null pointer provided");

    const HServiceInfo& serviceInfo = stateVar->parentService()->info();
    const HDeviceInfo& deviceInfo   =
        stateVar->parentService()->parentDevice()->info();

    HStateVariablesSetupData svSetupData =
        m_infoProvider->stateVariablesSetupData(serviceInfo, deviceInfo);

    HStateVariableInfo svInfo = svSetupData.get(stateVar->info().name());

    return svInfo.isValid() ?
        validateStateVariable(stateVar, svInfo) :
        HDeviceModelValidator::ValidationSucceeded;
}

/*******************************************************************************
 * HDeviceModelValidator
 ******************************************************************************/
HDeviceModelValidator::HDeviceModelValidator() :
    h_ptr(new HDeviceModelValidatorPrivate())
{
}

HDeviceModelValidator::~HDeviceModelValidator()
{
    delete h_ptr;
}

void HDeviceModelValidator::setInfoProvider(
    const HDeviceModelInfoProvider& provider)
{
    h_ptr->m_infoProvider.reset(provider.clone());
}

const HDeviceModelInfoProvider* HDeviceModelValidator::infoProvider() const
{
    return h_ptr->m_infoProvider.data();
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateDevice(HClientDevice* device)
{
    if (!infoProvider())
    {
        return InfoProviderMissing;
    }

    return h_ptr->validateDevice<
        HClientDevice, HClientService, HClientAction, HClientStateVariable>(device);
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateDevice(HServerDevice* device)
{
    if (!infoProvider())
    {
        return InfoProviderMissing;
    }

    return h_ptr->validateDevice<
        HServerDevice, HServerService, HServerAction, HServerStateVariable>(device);
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateService(HClientService* service)
{
    if (!infoProvider())
    {
        return InfoProviderMissing;
    }

    return h_ptr->validateService<HClientService, HClientAction, HClientStateVariable>(service);
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateService(HServerService* service)
{
    if (!infoProvider())
    {
        return InfoProviderMissing;
    }

    return h_ptr->validateService<HServerService, HServerAction, HServerStateVariable>(service);
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateAction(HClientAction* action)
{
    if (!infoProvider())
    {
        return HDeviceModelValidator::InfoProviderMissing;
    }

    return h_ptr->validateAction(action);
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateAction(HServerAction* action)
{
    if (!infoProvider())
    {
        return HDeviceModelValidator::InfoProviderMissing;
    }

    return h_ptr->validateAction(action);
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateStateVariable(HClientStateVariable* stateVar)
{
    if (!infoProvider())
    {
        return HDeviceModelValidator::InfoProviderMissing;
    }

    return h_ptr->validateStateVariable(stateVar);
}

HDeviceModelValidator::ValidationResult
    HDeviceModelValidator::validateStateVariable(HServerStateVariable* stateVar)
{
    if (!infoProvider())
    {
        return HDeviceModelValidator::InfoProviderMissing;
    }

    return h_ptr->validateStateVariable(stateVar);
}

QString HDeviceModelValidator::lastErrorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

}
}
