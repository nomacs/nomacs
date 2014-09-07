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

#include "hconnectionmanager_id.h"

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HConnectionManagerId
 ******************************************************************************/
HConnectionManagerId::HConnectionManagerId() :
    m_udn(), m_serviceId()
{
}

HConnectionManagerId::HConnectionManagerId(
    const QString& arg, HValidityCheckLevel checkLevel) :
        m_udn(), m_serviceId()
{
    qint32 sepIndex = arg.indexOf('/');
    if (sepIndex < 0)
    {
        return;
    }

    HUdn udn = HUdn(arg.left(sepIndex));
    HServiceId sid = HServiceId(arg.mid(sepIndex+1));
    if (udn.isValid(checkLevel) && sid.isValid(checkLevel))
    {
        m_udn = udn;
        m_serviceId = sid;
    }
}

HConnectionManagerId::HConnectionManagerId(
    const HUdn& udn, const HServiceId& serviceId, HValidityCheckLevel checkLevel) :
        m_udn(), m_serviceId()
{
    if (udn.isValid(checkLevel) && serviceId.isValid(checkLevel))
    {
        m_udn = udn;
        m_serviceId = serviceId;
    }
}

HConnectionManagerId::~HConnectionManagerId()
{
}

bool HConnectionManagerId::isValid(HValidityCheckLevel checkLevel) const
{
    return m_udn.isValid(checkLevel) && m_serviceId.isValid(checkLevel);
}

QString HConnectionManagerId::toString() const
{
    if (!isValid(LooseChecks))
    {
        return QString();
    }

    return QString("%1/%2").arg(m_udn.toString(), m_serviceId.toString());
}

bool operator==(const HConnectionManagerId& obj1, const HConnectionManagerId& obj2)
{
    return obj1.udn() == obj2.udn() &&
           obj1.serviceId() == obj2.serviceId();
}

}
}
}
