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

#include "hclientservice_adapter.h"
#include "hclientservice_adapter_p.h"
#include "hclientservice.h"
#include "hclientdevice.h"

#include "../hactions_setupdata.h"
#include "../hdevicemodel_validator.h"
#include "../hdevicemodel_infoprovider.h"

#include "../../dataelements/hserviceinfo.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientServiceAdapterPrivate
 ******************************************************************************/
HClientServiceAdapterPrivate::HClientServiceAdapterPrivate(const HResourceType& rt) :
    HAbstractClientAdapterPrivate(rt),
        m_service(0), q_ptr(0), m_actionsSetupData()
{
}

HClientServiceAdapterPrivate::~HClientServiceAdapterPrivate()
{
}

HClientAction* HClientServiceAdapterPrivate::getAction(
    const QString& arg, qint32* rc) const
{
    if (!m_service)
    {
        if (rc) { *rc = UpnpUndefinedFailure; }
        return 0;
    }

    if (!m_actionsSetupData.isNull() && m_actionsSetupData->contains(arg))
    {
        HActionSetup actionSetup = m_actionsSetupData->get(arg);
        if (actionSetup.version() > m_service->info().serviceType().version())
        {
            // The requested action was defined at a later service version than
            // what the adapter is currently configured to use. Because of this,
            // the action should not be invoked.
            if (rc) { *rc = UpnpInvalidAction; }
            return 0;
        }
    }

    HClientAction* action = m_service->actions().value(arg);
    if (action)
    {
        *rc = UpnpSuccess;
        return action;
    }
    else if (!rc)
    {
        return 0;
    }

    if (m_infoProvider.data())
    {
        HActionsSetupData sdata = m_infoProvider->actionsSetupData(
            m_service->info(), m_service->parentDevice()->info());

        if (sdata.contains(arg))
        {
            HActionSetup actionSetup = sdata.get(arg);
            *rc = actionSetup.inclusionRequirement() == InclusionOptional ?
                UpnpOptionalActionNotImplemented : UpnpInvalidAction;
        }
        else
        {
            *rc = UpnpInvalidAction;
        }
    }
    else
    {
        *rc = UpnpInvalidAction;
    }

    return 0;
}

bool HClientServiceAdapterPrivate::validate(HClientService* service)
{
    HDeviceModelValidator validator;
    validator.setInfoProvider(*m_infoProvider.data());
    bool b = validator.validateService(service) == HDeviceModelValidator::ValidationSucceeded;
    if (!b)
    {
        m_lastErrorDescription = validator.lastErrorDescription();
    }
    return b;
}

/*******************************************************************************
 * HClientServiceAdapter
 ******************************************************************************/
HClientServiceAdapter::HClientServiceAdapter(
    const HResourceType& serviceType, QObject* parent) :
        QObject(parent),
            h_ptr(new HClientServiceAdapterPrivate(serviceType))
{
    h_ptr->q_ptr = this;
    Q_ASSERT(serviceType.isValid());
}

HClientServiceAdapter::HClientServiceAdapter(
    HClientServiceAdapterPrivate& dd, QObject* parent) :
        QObject(parent),
            h_ptr(&dd)
{
    h_ptr->q_ptr = this;
}

HClientServiceAdapter::~HClientServiceAdapter()
{
    delete h_ptr;
}

bool HClientServiceAdapter::prepareService(HClientService*)
{
    return true;
}

void HClientServiceAdapter::setDeviceModelInfoProvider(
    const HDeviceModelInfoProvider& infoProvider)
{
    h_ptr->m_infoProvider.reset(infoProvider.clone());
}

const HDeviceModelInfoProvider* HClientServiceAdapter::deviceModelInfoProvider() const
{
    return h_ptr->m_infoProvider.data();
}

HClientAction* HClientServiceAdapter::getAction(
    const QString& name, qint32* rc) const
{
    return h_ptr->getAction(name, rc);
}

void HClientServiceAdapter::setLastErrorDescription(const QString& errDescr)
{
    h_ptr->m_lastErrorDescription = errDescr;
}

bool HClientServiceAdapter::setService(
    HClientService* service, ValidationType vt)
{
    HResourceType st = service->info().serviceType();

    if (!service || !st.compare(serviceType(), HResourceType::Inclusive))
    {
        setLastErrorDescription(QString("Unsupported service type: [%1]").arg(st.toString()));
        return false;
    }

    if (vt == FullValidation)
    {
        if (deviceModelInfoProvider())
        {
            if (!h_ptr->validate(service))
            {
                return false;
            }
        }
    }

    if (!prepareService(service))
    {
        return false;
    }

    h_ptr->m_service = service;

    if (deviceModelInfoProvider())
    {
        HActionsSetupData actionsSetupData =
            deviceModelInfoProvider()->actionsSetupData(
                service->info(), service->parentDevice()->info());

        if (!actionsSetupData.isEmpty())
        {
            h_ptr->m_actionsSetupData.reset(
                new HActionsSetupData(actionsSetupData));
        }
    }

    return true;
}

QString HClientServiceAdapter::lastErrorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

HClientService* HClientServiceAdapter::service() const
{
    return h_ptr->m_service;
}

bool HClientServiceAdapter::implementsAction(const QString& name) const
{
    return h_ptr->m_service && h_ptr->m_service->actions().contains(name);
}

bool HClientServiceAdapter::implementsStateVariable(const QString& name) const
{
    return h_ptr->m_service && h_ptr->m_service->stateVariables().contains(name);
}

bool HClientServiceAdapter::isReady() const
{
    return h_ptr->m_service;
}

const HResourceType& HClientServiceAdapter::serviceType() const
{
    return h_ptr->m_resourceType;
}

}
}
