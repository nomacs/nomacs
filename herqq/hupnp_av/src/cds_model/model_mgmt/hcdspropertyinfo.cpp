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

#include "hcdspropertyinfo.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsPropertyInfoPrivate :
    public QSharedData
{
public:

    QString m_name;
    QVariant m_defaultValue;
    HCdsPropertyInfo::PropertyFlags m_propertyFlags;
    HCdsProperties::Property m_nameType;

    HCdsPropertyInfoPrivate() :
        m_name(), m_defaultValue(), m_propertyFlags(0), m_nameType()
    {
    }

    HCdsPropertyInfoPrivate(
        const QString& name, const QVariant& defaultValue,
        HCdsPropertyInfo::PropertyFlags pfs) :
            m_name(name.trimmed()), m_defaultValue(defaultValue),
            m_propertyFlags(pfs & ~1), m_nameType(HCdsProperties::undefined)
    {
    }
};

const HCdsPropertyInfo HCdsPropertyInfo::s_empty;

HCdsPropertyInfo* HCdsPropertyInfo::create(
    const QString& name, HCdsProperties::Property prop,
    const QVariant& defaultValue, PropertyFlags pfs)
{
    QString trimmed = name.trimmed();
    Q_ASSERT(!trimmed.isEmpty());
    Q_ASSERT(prop != HCdsProperties::undefined);

    HCdsPropertyInfo* retVal = new HCdsPropertyInfo(trimmed, defaultValue);
    retVal->h_ptr->m_propertyFlags = pfs;
    retVal->h_ptr->m_nameType = prop;
    return retVal;
}

HCdsPropertyInfo::HCdsPropertyInfo() :
    h_ptr(new HCdsPropertyInfoPrivate())
{
}

HCdsPropertyInfo::HCdsPropertyInfo(
    const QString& name, const QVariant& defaultValue, PropertyFlags pfs) :
        h_ptr(new HCdsPropertyInfoPrivate(name, defaultValue, pfs))
{
}

HCdsPropertyInfo::HCdsPropertyInfo(const HCdsPropertyInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HCdsPropertyInfo& HCdsPropertyInfo::operator=(const HCdsPropertyInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HCdsPropertyInfo::~HCdsPropertyInfo()
{
}

const QString& HCdsPropertyInfo::name() const
{
    return h_ptr->m_name;
}

HCdsProperties::Property HCdsPropertyInfo::type() const
{
    return h_ptr->m_nameType;
}

const QVariant& HCdsPropertyInfo::defaultValue() const
{
    return h_ptr->m_defaultValue;
}

HCdsPropertyInfo::PropertyFlags HCdsPropertyInfo::propertyFlags() const
{
    return h_ptr->m_propertyFlags;
}

bool HCdsPropertyInfo::isValid() const
{
    return !name().isEmpty();
}

bool operator==(const HCdsPropertyInfo& obj1, const HCdsPropertyInfo& obj2)
{
    return obj1.name() == obj2.name() &&
           obj1.propertyFlags() == obj2.propertyFlags() &&
           obj1.defaultValue() == obj2.defaultValue();
}

}
}
}
