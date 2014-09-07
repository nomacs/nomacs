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

#include "hstatevariable_event.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HStateVariableEventPrivate
 *******************************************************************************/
class HStateVariableEventPrivate :
    public QSharedData
{
public:

    QVariant m_previousValue;
    QVariant m_newValue;

public:

    HStateVariableEventPrivate ();
    ~HStateVariableEventPrivate();
};

HStateVariableEventPrivate::HStateVariableEventPrivate() :
    m_previousValue(), m_newValue()
{
}

HStateVariableEventPrivate::~HStateVariableEventPrivate()
{
}

/*******************************************************************************
 * HStateVariableEvent
 *******************************************************************************/
HStateVariableEvent::HStateVariableEvent() :
    h_ptr(new HStateVariableEventPrivate())
{
}

HStateVariableEvent::HStateVariableEvent(
    const QVariant& previousValue, const QVariant& newValue) :
        h_ptr(new HStateVariableEventPrivate())
{
    h_ptr->m_previousValue = previousValue;
    h_ptr->m_newValue      = newValue;
}

HStateVariableEvent::HStateVariableEvent(const HStateVariableEvent& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HStateVariableEvent::~HStateVariableEvent()
{
}

HStateVariableEvent& HStateVariableEvent::operator=(
    const HStateVariableEvent& other)
{
    Q_ASSERT(&other != this);

    h_ptr = other.h_ptr;

    return *this;
}

bool HStateVariableEvent::isEmpty() const
{
    return h_ptr->m_previousValue.isNull() &&
           h_ptr->m_newValue.isNull();
}

QVariant HStateVariableEvent::previousValue() const
{
    return h_ptr->m_previousValue;
}

QVariant HStateVariableEvent::newValue() const
{
    return h_ptr->m_newValue;
}

}
}
