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

#include "hclientservice.h"
#include "hclientservice_p.h"

#include "hclientaction.h"
#include "hdefault_clientdevice_p.h"
#include "hdefault_clientservice_p.h"
#include "hdefault_clientstatevariable_p.h"

#include "../../dataelements/hactioninfo.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientServicePrivate
 ******************************************************************************/
HClientServicePrivate::HClientServicePrivate() :
    m_stateVariablesConst()
{
}

HClientServicePrivate::~HClientServicePrivate()
{
}

bool HClientServicePrivate::addStateVariable(HDefaultClientStateVariable* sv)
{
    if (HServicePrivate<HClientService, HClientAction, HDefaultClientStateVariable>::addStateVariable(sv))
    {
        m_stateVariablesConst.insert(sv->info().name(), sv);
        return true;
    }

    return false;
}

HClientServicePrivate::ReturnValue HClientServicePrivate::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    ReturnValue rv =
        HServicePrivate<HClientService, HClientAction, HDefaultClientStateVariable>::updateVariables(variables);

    if (rv == Updated && sendEvent && m_evented)
    {
        emit q_ptr->stateChanged(q_ptr);
    }

    return rv;
}

/*******************************************************************************
 * HClientService
 ******************************************************************************/
HClientService::HClientService(
    const HServiceInfo& info, HClientDevice* parentDevice) :
        QObject(reinterpret_cast<QObject*>(parentDevice)),
            h_ptr(new HClientServicePrivate())
{
    Q_ASSERT_X(parentDevice, "", "Parent device must be defined!");

    h_ptr->m_serviceInfo = info;
    h_ptr->q_ptr = this;
}

HClientService::~HClientService()
{
    delete h_ptr;
}

HClientDevice* HClientService::parentDevice() const
{
    return reinterpret_cast<HClientDevice*>(parent());
}

const HServiceInfo& HClientService::info() const
{
    return h_ptr->m_serviceInfo;
}

QString HClientService::description() const
{
    return h_ptr->m_serviceDescription;
}

const HClientActions& HClientService::actions() const
{
    return h_ptr->m_actions;
}

const HClientStateVariables& HClientService::stateVariables() const
{
    return h_ptr->m_stateVariablesConst;
}

void HClientService::notifyListeners()
{
    if (h_ptr->m_evented)
    {
        emit stateChanged(this);
    }
}

bool HClientService::isEvented() const
{
    return h_ptr->m_evented;
}

QVariant HClientService::value(const QString& stateVarName, bool* ok) const
{
    return h_ptr->value(stateVarName, ok);
}

/*******************************************************************************
 * HDefaultClientService
 ******************************************************************************/
HDefaultClientService::HDefaultClientService(
    const HServiceInfo& info, HDefaultClientDevice* parentDevice) :
        HClientService(info, parentDevice)
{
}

void HDefaultClientService::addAction(HClientAction* action)
{
    Q_ASSERT(action);
    Q_ASSERT(!h_ptr->m_actions.contains(action->info().name()));
    h_ptr->m_actions.insert(action->info().name(), action);
}

void HDefaultClientService::addStateVariable(HDefaultClientStateVariable* sv)
{
    h_ptr->addStateVariable(sv);
}

void HDefaultClientService::setDescription(const QString& description)
{
    h_ptr->m_serviceDescription = description;
}

bool HDefaultClientService::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    return h_ptr->updateVariables(variables, sendEvent) != HClientServicePrivate::Failed;
}

HDefaultClientDevice* HDefaultClientService::parentDevice() const
{
    return static_cast<HDefaultClientDevice*>(HClientService::parentDevice());
}

}
}
