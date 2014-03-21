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

#include "hchannelgroupname.h"

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HChannelGroupName>(
        "Herqq::Upnp::Av::HChannelGroupName");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

namespace
{
inline bool verifyId(const QString& id)
{
    return id.indexOf("_") > 3;
    // You really cannot express a domain in less than four characters.
}
}

HChannelGroupName::HChannelGroupName() :
    m_name(), m_id()
{
}

HChannelGroupName::HChannelGroupName(const QString& name, const QString& id) :
    m_name(), m_id()
{
    QString idTrimmed = id.trimmed();
    QString nameTrimmed = name.trimmed();
    if (!nameTrimmed.isEmpty() && verifyId(idTrimmed))
    {
        m_name = nameTrimmed;
        m_id = idTrimmed;
    }
}

HChannelGroupName::~HChannelGroupName()
{
}

bool HChannelGroupName::isValid() const
{
    return !m_name.isEmpty();
}

bool operator==(const HChannelGroupName& obj1, const HChannelGroupName& obj2)
{
    return obj1.id() == obj2.id() &&
           obj1.name() == obj2.name();
}

}
}
}

