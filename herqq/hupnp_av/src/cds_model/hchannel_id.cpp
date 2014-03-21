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

#include "hchannel_id.h"

#include <QtCore/QMetaType>
#include <QtCore/QStringList>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HChannelId>("Herqq::Upnp::Av::HChannelId");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HChannelIdPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HChannelIdPrivate)

public:

    HChannelId::Type m_type;
    QString m_typeAsString;
    QString m_value;

    HChannelIdPrivate() :
        m_type(HChannelId::Undefined), m_typeAsString(), m_value()
    {
    }
};

QString HChannelId::toString(Type type)
{
    QString retVal;
    switch(type)
    {
    case SeriesId:
        retVal = "SI_SERIESID";
        break;
    case ProgramId:
        retVal = "SI_PROGRAMID";
        break;
    default:
        break;
    }
    return retVal;
}

HChannelId::HChannelId() :
    h_ptr(new HChannelIdPrivate())
{
}

HChannelId::HChannelId(const QString& arg, const QString& type) :
    h_ptr(new HChannelIdPrivate())
{
    if (type == "SI_SERIESID")
    {
        if (arg.split(",").size() == 4)
        {
            h_ptr->m_value = arg;
            h_ptr->m_type = SeriesId;
            h_ptr->m_typeAsString = type;
        }
    }
    else if (type == "SI_PROGRAMID")
    {
        if (arg.split(",").size() == 4)
        {
            h_ptr->m_value = arg;
            h_ptr->m_type = ProgramId;
            h_ptr->m_typeAsString = type;
        }
    }
    else if (!type.isEmpty())
    {
        if (arg.indexOf("_") > 1)
        {
            h_ptr->m_value = arg;
            h_ptr->m_type = VendorDefined;
            h_ptr->m_typeAsString = type;
        }
    }
}

HChannelId::HChannelId(const HChannelId& other) :
    h_ptr(other.h_ptr)
{
}

HChannelId& HChannelId::operator=(const HChannelId& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HChannelId::~HChannelId()
{
}

bool HChannelId::isValid() const
{
    return !h_ptr->m_value.isEmpty() && h_ptr->m_type != Undefined;
}

HChannelId::Type HChannelId::type() const
{
    return h_ptr->m_type;
}

QString HChannelId::typeAsString() const
{
    return h_ptr->m_typeAsString;
}

QString HChannelId::value() const
{
    return h_ptr->m_value;
}

bool operator==(const HChannelId& obj1, const HChannelId& obj2)
{
    return obj1.typeAsString() == obj2.typeAsString() &&
           obj1.value() == obj2.value();
}

}
}
}

