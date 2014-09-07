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

#include "havt_lastchange_info.h"

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAvtLastChangeInfoPrivate
 ******************************************************************************/
class HAvtLastChangeInfoPrivate
{
public:

    QString m_name;
    QVariant m_value;

    HAvtLastChangeInfoPrivate() :
        m_name(), m_value()
    {
    }
};

/*******************************************************************************
 * HAvtLastChangeInfo
 ******************************************************************************/
HAvtLastChangeInfo::HAvtLastChangeInfo() :
    h_ptr(new HAvtLastChangeInfoPrivate())
{
}

HAvtLastChangeInfo::HAvtLastChangeInfo(
    const QString& propertyName, const QVariant& value) :
        h_ptr(new HAvtLastChangeInfoPrivate())
{
    h_ptr->m_name = propertyName.trimmed();
    h_ptr->m_value = value;
}

HAvtLastChangeInfo::~HAvtLastChangeInfo()
{
    delete h_ptr;
}

HAvtLastChangeInfo::HAvtLastChangeInfo(const HAvtLastChangeInfo& other) :
    h_ptr(new HAvtLastChangeInfoPrivate(*other.h_ptr))
{
    Q_ASSERT(this != &other);
}

HAvtLastChangeInfo& HAvtLastChangeInfo::operator=(const HAvtLastChangeInfo& other)
{
    Q_ASSERT(this != &other);
    delete h_ptr;
    h_ptr = new HAvtLastChangeInfoPrivate(*other.h_ptr);
    return *this;
}

void HAvtLastChangeInfo::setPropertyName(const QString& name)
{
    h_ptr->m_name = name.trimmed();
}

void HAvtLastChangeInfo::setValue(const QVariant& value)
{
    h_ptr->m_value = value;
}

bool HAvtLastChangeInfo::isValid() const
{
    return !propertyName().isEmpty();
}

QString HAvtLastChangeInfo::propertyName() const
{
    return h_ptr->m_name;
}

QVariant HAvtLastChangeInfo::value() const
{
    return h_ptr->m_value;
}

bool operator==(const HAvtLastChangeInfo& obj1, const HAvtLastChangeInfo& obj2)
{
    return obj1.propertyName() == obj2.propertyName() &&
           obj1.value() == obj2.value();
}

}
}
}
