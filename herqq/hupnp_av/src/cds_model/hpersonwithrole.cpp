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

#include "hpersonwithrole.h"

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HPersonWithRole>(
        "Herqq::Upnp::Av::HPersonWithRole");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HPersonWithRole::HPersonWithRole() :
    m_name(), m_role()
{
}

HPersonWithRole::HPersonWithRole(const QString& name, const QString& role) :
    m_name(), m_role()
{
    QString nameTrimmed = name.trimmed();
    if (!nameTrimmed.isEmpty())
    {
        m_name = nameTrimmed;
        m_role = role.trimmed();
    }
}

HPersonWithRole::~HPersonWithRole()
{
}

bool HPersonWithRole::isValid() const
{
    return !m_name.isEmpty();
}

bool operator==(const HPersonWithRole& obj1, const HPersonWithRole& obj2)
{
    return obj1.name() == obj2.name() &&
           obj1.role() == obj2.role();
}

}
}
}

