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

#include "htransportinfo.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HTransportStatus
 ******************************************************************************/
HTransportStatus::HTransportStatus() :
    m_type(Undefined), m_typeAsString()
{
}

HTransportStatus::HTransportStatus(Type type) :
    m_type(type), m_typeAsString(toString(type))
{
}

HTransportStatus::HTransportStatus(const QString& arg) :
    m_type(), m_typeAsString()
{
    QString trimmed = arg.trimmed();
    m_type = fromString(trimmed);
    m_typeAsString = trimmed;
}

QString HTransportStatus::toString(HTransportStatus::Type type)
{
    QString retVal;
    switch(type)
    {
    case HTransportStatus::OK:
        retVal = "OK";
        break;
    case HTransportStatus::ErrorOccurred:
        retVal = "ERROR_OCCURRED";
        break;
    default:
        break;
    }
    return retVal;
}

HTransportStatus::Type HTransportStatus::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare("OK", Qt::CaseInsensitive) == 0)
    {
        retVal = OK;
    }
    else if (type.compare("ERROR_OCCURRED", Qt::CaseInsensitive) == 0)
    {
        retVal = ErrorOccurred;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HTransportStatus& obj1, const HTransportStatus& obj2)
{
    return obj1.toString() == obj2.toString();
}

/*******************************************************************************
 * HTransportInfoPrivate
 ******************************************************************************/
class HTransportInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HTransportInfoPrivate)

public:

    HTransportState m_transportState;
    HTransportStatus m_transportStatus;
    QString m_transportPlaySpeed;

    HTransportInfoPrivate() :
        m_transportState(), m_transportStatus(), m_transportPlaySpeed()
    {
    }
};

/*******************************************************************************
 * HTransportInfo
 ******************************************************************************/
HTransportInfo::HTransportInfo() :
    h_ptr(new HTransportInfoPrivate())
{
}

HTransportInfo::HTransportInfo(
    const HTransportState& state, const HTransportStatus& status,
    const QString& speed) :
        h_ptr(new HTransportInfoPrivate())
{
    h_ptr->m_transportState = state;
    h_ptr->m_transportStatus = status;
    h_ptr->m_transportPlaySpeed = speed;
}

HTransportInfo::HTransportInfo(const HTransportInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HTransportInfo& HTransportInfo::operator =(const HTransportInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HTransportInfo::~HTransportInfo()
{
}

const HTransportState& HTransportInfo::state() const
{
    return h_ptr->m_transportState;
}

const HTransportStatus& HTransportInfo::status() const
{
    return h_ptr->m_transportStatus;
}

QString HTransportInfo::speed() const
{
    return h_ptr->m_transportPlaySpeed;
}

bool HTransportInfo::isValid()
{
    return h_ptr->m_transportState.isValid() && h_ptr->m_transportStatus.isValid();
}

void HTransportInfo::setState(const HTransportState& arg)
{
    h_ptr->m_transportState = arg;
}

void HTransportInfo::setStatus(const HTransportStatus& arg)
{
    h_ptr->m_transportStatus = arg;
}

void HTransportInfo::setSpeed(const QString& arg)
{
    h_ptr->m_transportPlaySpeed = arg;
}

bool operator==(const HTransportInfo& obj1, const HTransportInfo& obj2)
{
    return obj1.speed() == obj2.speed() &&
           obj1.state() == obj2.state() &&
           obj1.status() == obj2.status();
}

}
}
}
