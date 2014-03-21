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

#include "hservices_setupdata.h"
#include "../dataelements/hserviceid.h"
#include "../dataelements/hresourcetype.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServiceSetupPrivate
 ******************************************************************************/
class HServiceSetupPrivate :
    public QSharedData
{

public:

    HServiceId m_serviceId;
    HResourceType m_serviceType;
    int m_version;
    HInclusionRequirement m_inclusionReq;

    HServiceSetupPrivate() :
        m_serviceId(), m_serviceType(), m_version(0),
        m_inclusionReq(InclusionRequirementUnknown)
    {
    }

    ~HServiceSetupPrivate()
    {
    }
};

/*******************************************************************************
 * HServiceSetup
 ******************************************************************************/
HServiceSetup::HServiceSetup() :
    h_ptr(new HServiceSetupPrivate())
{
}

HServiceSetup::HServiceSetup(
    const HServiceId& id, const HResourceType& serviceType,
    HInclusionRequirement ireq) :
        h_ptr(new HServiceSetupPrivate())
{
    h_ptr->m_serviceId = id;
    h_ptr->m_serviceType = serviceType;
    h_ptr->m_version = 1;
    h_ptr->m_inclusionReq = ireq;
}

HServiceSetup::HServiceSetup(
    const HServiceId& id, const HResourceType& serviceType, int version,
    HInclusionRequirement ireq) :
        h_ptr(new HServiceSetupPrivate())
{
    h_ptr->m_serviceId = id;
    h_ptr->m_serviceType = serviceType;
    h_ptr->m_version = version;
    h_ptr->m_inclusionReq = ireq;
}

HServiceSetup::~HServiceSetup()
{
}

HServiceSetup& HServiceSetup::operator=(const HServiceSetup& other)
{
    Q_ASSERT(&other != this);

    h_ptr = other.h_ptr;

    return *this;
}

HServiceSetup::HServiceSetup(const HServiceSetup& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

bool HServiceSetup::isValid(HValidityCheckLevel checkLevel) const
{
    return h_ptr->m_serviceId.isValid(checkLevel) &&
           h_ptr->m_serviceType.isValid() &&
           h_ptr->m_version > 0 &&
           h_ptr->m_inclusionReq != InclusionRequirementUnknown;
}

HInclusionRequirement HServiceSetup::inclusionRequirement() const
{
    return h_ptr->m_inclusionReq;
}

const HServiceId& HServiceSetup::serviceId() const
{
    return h_ptr->m_serviceId;
}

const HResourceType& HServiceSetup::serviceType() const
{
    return h_ptr->m_serviceType;
}

int HServiceSetup::version() const
{
    return h_ptr->m_version;
}

void HServiceSetup::setInclusionRequirement(HInclusionRequirement arg)
{
    h_ptr->m_inclusionReq = arg;
}

void HServiceSetup::setServiceId(const HServiceId& arg)
{
    h_ptr->m_serviceId = arg;
}

void HServiceSetup::setServiceType(const HResourceType& arg)
{
    h_ptr->m_serviceType = arg;
}

void HServiceSetup::setVersion(int version)
{
    h_ptr->m_version = version;
}

bool operator==(const HServiceSetup& obj1, const HServiceSetup& obj2)
{
    return obj1.inclusionRequirement() == obj2.inclusionRequirement() &&
           obj1.serviceId() == obj2.serviceId() &&
           obj1.serviceType() == obj2.serviceType() &&
           obj1.version() == obj2.version();
}

/*******************************************************************************
 * HServicesSetupData
 ******************************************************************************/
HServicesSetupData::HServicesSetupData() :
    m_serviceSetupInfos()
{
}

HServicesSetupData::~HServicesSetupData()
{
}

bool HServicesSetupData::insert(const HServiceSetup& setupInfo, bool overWrite)
{
    if (!setupInfo.isValid(StrictChecks))
    {
        return false;
    }

    const HServiceId& id = setupInfo.serviceId();
    if (!overWrite && m_serviceSetupInfos.contains(id))
    {
        return false;
    }

    m_serviceSetupInfos.insert(id, setupInfo);
    return true;
}

bool HServicesSetupData::remove(const HServiceId& serviceId)
{
    if (m_serviceSetupInfos.contains(serviceId))
    {
        m_serviceSetupInfos.remove(serviceId);
        return true;
    }

    return false;
}

HServiceSetup HServicesSetupData::get(const HServiceId& serviceId) const
{
    return m_serviceSetupInfos.value(serviceId);
}

bool HServicesSetupData::contains(const HServiceId& id) const
{
    return m_serviceSetupInfos.contains(id);
}

QSet<HServiceId> HServicesSetupData::serviceIds() const
{
    return m_serviceSetupInfos.keys().toSet();
}

int HServicesSetupData::size() const
{
    return m_serviceSetupInfos.size();
}

bool HServicesSetupData::isEmpty() const
{
    return m_serviceSetupInfos.isEmpty();
}

bool operator==(const HServicesSetupData& obj1, const HServicesSetupData& obj2)
{
    return obj1.m_serviceSetupInfos == obj2.m_serviceSetupInfos;
}

}
}
