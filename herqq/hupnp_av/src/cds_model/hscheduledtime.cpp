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

#include "hscheduledtime.h"

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HScheduledTime>(
        "Herqq::Upnp::Av::HScheduledTime");
    return true;
}

static bool regMetaT = registerMetaTypes();

#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HScheduledTimePrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HScheduledTimePrivate)

public:

    QDateTime m_value;
    HScheduledTime::Type m_type;
    HDaylightSaving m_dlSaving;

    HScheduledTimePrivate() :
        m_value(), m_type(HScheduledTime::Undefined),
        m_dlSaving(Unknown_DaylightSaving)
    {
    }
};

HScheduledTime::HScheduledTime() :
    h_ptr(new HScheduledTimePrivate())
{
}

HScheduledTime::HScheduledTime(const QDateTime& value, Type type) :
    h_ptr(new HScheduledTimePrivate())
{
    setValue(value);
    setType(type);
}

HScheduledTime::HScheduledTime(const HScheduledTime& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HScheduledTime& HScheduledTime::operator=(const HScheduledTime& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HScheduledTime::~HScheduledTime()
{
}

QString HScheduledTime::toString(Type type)
{
    QString retVal;
    switch(type)
    {
    case ScheduledProgram:
        retVal = "SCHEDULED_PROGRAM";
        break;
    case OnDemand:
        retVal = "ON_DEMAND";
        break;
    default:
        break;
    }
    return retVal;
}

HScheduledTime::Type HScheduledTime::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare("SCHEDULED_PROGRAM", Qt::CaseInsensitive) == 0)
    {
        retVal = ScheduledProgram;
    }
    else if (type.compare("ON_DEMAND", Qt::CaseInsensitive) == 0)
    {
        retVal = OnDemand;
    }
    return retVal;
}

bool HScheduledTime::isValid() const
{
    return h_ptr->m_type != Undefined && h_ptr->m_value.isValid();
}

bool operator==(const HScheduledTime& obj1, const HScheduledTime& obj2)
{
    return obj1.daylightSaving() == obj2.daylightSaving() &&
           obj1.type() == obj2.type() &&
           obj1.value() == obj2.value();
}

QDateTime HScheduledTime::value() const
{
    return h_ptr->m_value;
}

HScheduledTime::Type HScheduledTime::type() const
{
    return h_ptr->m_type;
}

HDaylightSaving HScheduledTime::daylightSaving() const
{
    return h_ptr->m_dlSaving;
}

void HScheduledTime::setValue(const QDateTime& arg)
{
    h_ptr->m_value = arg;
}

void HScheduledTime::setType(Type arg)
{
    h_ptr->m_type = arg;
}

void HScheduledTime::setDaylightSaving(HDaylightSaving arg)
{
    h_ptr->m_dlSaving = arg;
}

}
}
}
