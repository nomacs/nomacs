/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hserviceinfo.h"
#include "hserviceinfo_p.h"

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServiceInfoPrivate
 ******************************************************************************/
HServiceInfoPrivate::HServiceInfoPrivate() :
    m_serviceId   (),
    m_serviceType (),
    m_scpdUrl     (),
    m_controlUrl  (),
    m_eventSubUrl (),
    m_inclusionRequirement(InclusionRequirementUnknown)
{
}

HServiceInfoPrivate::~HServiceInfoPrivate()
{
}

/*******************************************************************************
 * HServiceInfo
 ******************************************************************************/
HServiceInfo::HServiceInfo() :
    h_ptr(new HServiceInfoPrivate())
{
}

HServiceInfo::HServiceInfo(
    const HServiceId& serviceId,
    const HResourceType& serviceType,
    const QUrl& controlUrl,
    const QUrl& eventSubUrl,
    const QUrl& scpdUrl,
    HInclusionRequirement inclusionRequirement,
    HValidityCheckLevel checkLevel,
    QString* err) :
        h_ptr(new HServiceInfoPrivate())
{
    QString errTmp;
    if (!serviceId.isValid(checkLevel))
    {
        errTmp = "Invalid service ID";
    }
    else if (!serviceType.isValid())
    {
        errTmp = "Invalid service type";
    }
    else if (controlUrl.isEmpty() || !controlUrl.isValid())
    {
        errTmp = "Invalid control URL";
    }
    else if (eventSubUrl.isEmpty() || !eventSubUrl.isValid())
    {
        errTmp = "Invalid event sub URL";
    }
    else if (scpdUrl.isEmpty() || !scpdUrl.isValid())
    {
        errTmp = "Invalid SCPD URL";
    }
    else
    {
        h_ptr->m_controlUrl = controlUrl;
        h_ptr->m_eventSubUrl = eventSubUrl;
        h_ptr->m_scpdUrl = scpdUrl;
        h_ptr->m_serviceId = serviceId;
        h_ptr->m_serviceType = serviceType;
        h_ptr->m_inclusionRequirement = inclusionRequirement;
    }

    if (err && !errTmp.isEmpty())
    {
        *err = errTmp;
    }
}

HServiceInfo::HServiceInfo(const HServiceInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HServiceInfo& HServiceInfo::operator=(const HServiceInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HServiceInfo::~HServiceInfo()
{
}

bool HServiceInfo::isValid(HValidityCheckLevel checkLevel) const
{
    return h_ptr->m_serviceId.isValid(checkLevel);
}

const HServiceId& HServiceInfo::serviceId() const
{
    return h_ptr->m_serviceId;
}

const HResourceType& HServiceInfo::serviceType() const
{
    return h_ptr->m_serviceType;
}

QUrl HServiceInfo::scpdUrl() const
{
    return h_ptr->m_scpdUrl;
}

QUrl HServiceInfo::controlUrl() const
{
    return h_ptr->m_controlUrl;
}

QUrl HServiceInfo::eventSubUrl() const
{
    return h_ptr->m_eventSubUrl;
}

HInclusionRequirement HServiceInfo::inclusionRequirement() const
{
    return h_ptr->m_inclusionRequirement;
}

bool operator==(const HServiceInfo& obj1, const HServiceInfo& obj2)
{
    return obj1.h_ptr->m_controlUrl == obj2.h_ptr->m_controlUrl &&
           obj1.h_ptr->m_eventSubUrl == obj2.h_ptr->m_eventSubUrl &&
           obj1.h_ptr->m_scpdUrl == obj2.h_ptr->m_scpdUrl &&
           obj1.h_ptr->m_serviceId == obj2.h_ptr->m_serviceId &&
           obj1.h_ptr->m_serviceType == obj2.h_ptr->m_serviceType;
}

}
}
