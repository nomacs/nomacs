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

#include "hclientstatevariable.h"
#include "hdefault_clientservice_p.h"
#include "hdefault_clientstatevariable_p.h"

#include "../hstatevariable_p.h"
#include "../hstatevariable_event.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientStateVariable
 *******************************************************************************/
HClientStateVariable::HClientStateVariable(
    const HStateVariableInfo& info, HClientService* parent) :
        QObject(reinterpret_cast<QObject*>(parent)),
            h_ptr(new HStateVariablePrivate())
{
    Q_ASSERT_X(parent, H_AT, "Parent service must be defined.");
    Q_ASSERT_X(info.isValid(), H_AT, "Info object must be valid.");

    h_ptr->m_info = info;
    setValue(info.defaultValue());
}

HClientStateVariable::~HClientStateVariable()
{
    delete h_ptr;
}

HClientService* HClientStateVariable::parentService() const
{
    return reinterpret_cast<HClientService*>(parent());
}

QVariant HClientStateVariable::value() const
{
    return h_ptr->m_value;
}

const HStateVariableInfo& HClientStateVariable::info() const
{
    return h_ptr->m_info;
}

bool HClientStateVariable::setValue(const QVariant& newValue)
{
    QVariant oldValue = h_ptr->m_value;

    QString err;
    if (!h_ptr->setValue(newValue, &err))
    {
        return false;
    }

    if (h_ptr->m_info.eventingType() != HStateVariableInfo::NoEvents)
    {
        HStateVariableEvent event(oldValue, newValue);
        emit valueChanged(this, event);
    }

    return true;
}

/*******************************************************************************
 * HDefaultClientStateVariable
 *******************************************************************************/
HDefaultClientStateVariable::HDefaultClientStateVariable(
    const HStateVariableInfo& info, HDefaultClientService* parent) :
        HClientStateVariable(info, parent)
{
}

}
}
