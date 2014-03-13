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

#include "hrcs_lastchange_info.h"
#include "../renderingcontrol/hchannel.h"

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRcsLastChangeInfoPrivate
 ******************************************************************************/
class HRcsLastChangeInfoPrivate
{
public:

    QString m_name;
    QVariant m_value;
    HChannel m_channel;

    HRcsLastChangeInfoPrivate() :
        m_name(), m_value(), m_channel()
    {
    }
};

/*******************************************************************************
 * HRcsLastChangeInfo
 ******************************************************************************/
HRcsLastChangeInfo::HRcsLastChangeInfo() :
    h_ptr(new HRcsLastChangeInfoPrivate())
{
}

HRcsLastChangeInfo::HRcsLastChangeInfo(
    const QString& propertyName, const QVariant& value) :
        h_ptr(new HRcsLastChangeInfoPrivate())
{
    h_ptr->m_name = propertyName.trimmed();
    h_ptr->m_value = value;
}

HRcsLastChangeInfo::~HRcsLastChangeInfo()
{
    delete h_ptr;
}

HRcsLastChangeInfo::HRcsLastChangeInfo(const HRcsLastChangeInfo& other) :
    h_ptr(new HRcsLastChangeInfoPrivate(*other.h_ptr))
{
    Q_ASSERT(this != &other);
}

HRcsLastChangeInfo& HRcsLastChangeInfo::operator=(const HRcsLastChangeInfo& other)
{
    Q_ASSERT(this != &other);
    delete h_ptr;
    h_ptr = new HRcsLastChangeInfoPrivate(*other.h_ptr);
    return *this;
}

void HRcsLastChangeInfo::setPropertyName(const QString& name)
{
    h_ptr->m_name = name.trimmed();
}

void HRcsLastChangeInfo::setValue(const QVariant& value)
{
    h_ptr->m_value = value;
}

void HRcsLastChangeInfo::setChannel(const HChannel& channel)
{
    h_ptr->m_channel = channel;
}

bool HRcsLastChangeInfo::isValid() const
{
    return !propertyName().isEmpty();
}

QString HRcsLastChangeInfo::propertyName() const
{
    return h_ptr->m_name;
}

QVariant HRcsLastChangeInfo::value() const
{
    return h_ptr->m_value;
}

HChannel HRcsLastChangeInfo::channel() const
{
    return h_ptr->m_channel;
}

bool operator==(const HRcsLastChangeInfo& obj1, const HRcsLastChangeInfo& obj2)
{
    return obj1.propertyName() == obj2.propertyName() &&
           obj1.value() == obj2.value() &&
           obj1.channel() == obj2.channel();
}

}
}
}
