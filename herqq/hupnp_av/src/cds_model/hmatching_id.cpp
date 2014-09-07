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

#include "hmatching_id.h"

#include <QtCore/QStringList>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HMatchingId>("Herqq::Upnp::Av::HMatchingId");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMatchingIdPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HMatchingIdPrivate)

public:

    HMatchingId::Type m_type;
    QString m_typeAsString;
    QString m_value;

    HMatchingIdPrivate() :
        m_type(HMatchingId::Undefined), m_typeAsString(), m_value()
    {
    }
};

QString HMatchingId::toString(Type type)
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

HMatchingId::Type HMatchingId::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare("SI_SERIESID", Qt::CaseInsensitive) == 0)
    {
        retVal = SeriesId;
    }
    else if (type.compare("SI_PROGRAMID", Qt::CaseInsensitive) == 0)
    {
        retVal = ProgramId;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

HMatchingId::HMatchingId() :
    h_ptr(new HMatchingIdPrivate())
{
}

namespace
{
QStringList trim(const QStringList& arg)
{
    QStringList retVal;
    foreach(const QString& str, arg)
    {
        QString trimmed = str.trimmed();
        if (trimmed.isEmpty())
        {
            break;
        }
        retVal.append(trimmed);
    }
    return retVal;
}
}

HMatchingId::HMatchingId(const QString& arg, const QString& type) :
    h_ptr(new HMatchingIdPrivate())
{
    QStringList argTrimmed = trim(arg.split(","));
    QString typeTrimmed = type.trimmed();
    if (typeTrimmed == "SI_SERIESID")
    {
        if (argTrimmed.size() == 4)
        {
            h_ptr->m_value = argTrimmed.join(",");
            h_ptr->m_type = SeriesId;
            h_ptr->m_typeAsString = typeTrimmed;
        }
    }
    else if (typeTrimmed == "SI_PROGRAMID")
    {
        if (argTrimmed.size() == 4)
        {
            h_ptr->m_value = argTrimmed.join(",");
            h_ptr->m_type = ProgramId;
            h_ptr->m_typeAsString = typeTrimmed;
        }
    }
    else if (!typeTrimmed.isEmpty())
    {
        QString argTrimmed = arg.trimmed();
        if (typeTrimmed.indexOf("_") > 3 && !argTrimmed.isEmpty())
        {
            // Simple (and frail) check to ensure that the Type format is followed
            // (it is impossible for a proper ICANN name to be less than 5 characters).
            // Could (should?) check the ICANN name for thoroughness.
            h_ptr->m_value = argTrimmed;
            h_ptr->m_type = VendorDefined;
            h_ptr->m_typeAsString = typeTrimmed;
        }
    }
}

HMatchingId::HMatchingId(const HMatchingId& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HMatchingId& HMatchingId::operator=(const HMatchingId& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HMatchingId::~HMatchingId()
{
}

bool HMatchingId::isValid() const
{
    return !h_ptr->m_value.isEmpty() && !h_ptr->m_typeAsString.isEmpty();
}

HMatchingId::Type HMatchingId::type() const
{
    return h_ptr->m_type;
}

QString HMatchingId::typeAsString() const
{
    return h_ptr->m_typeAsString;
}

QString HMatchingId::value() const
{
    return h_ptr->m_value;
}

bool operator==(const HMatchingId& obj1, const HMatchingId& obj2)
{
    return obj1.typeAsString() == obj2.typeAsString() &&
           obj1.value() == obj2.value();
}

}
}
}

