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

#include "hdevices_setupdata.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceSetupPrivate
 ******************************************************************************/
class HDeviceSetupPrivate :
    public QSharedData
{
public:

    HResourceType m_deviceType;
    int m_version;
    HInclusionRequirement m_inclusionReq;

    HDeviceSetupPrivate() :
        m_deviceType(), m_version(0),
        m_inclusionReq(InclusionRequirementUnknown)
    {
    }

    ~HDeviceSetupPrivate()
    {
    }
};

/*******************************************************************************
 * HDeviceSetup
 ******************************************************************************/
HDeviceSetup::HDeviceSetup() :
    h_ptr(new HDeviceSetupPrivate())
{
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, HInclusionRequirement incReq) :
        h_ptr(new HDeviceSetupPrivate())
{
    h_ptr->m_deviceType = type;
    h_ptr->m_version = 1;
    h_ptr->m_inclusionReq = incReq;
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, int version, HInclusionRequirement incReq) :
        h_ptr(new HDeviceSetupPrivate())
{
    h_ptr->m_deviceType = type;
    h_ptr->m_version = version;
    h_ptr->m_inclusionReq = incReq;
}

HDeviceSetup::~HDeviceSetup()
{
}

HDeviceSetup& HDeviceSetup::operator=(const HDeviceSetup& other)
{
    Q_ASSERT(&other != this);

    h_ptr = other.h_ptr;

    return *this;
}

HDeviceSetup::HDeviceSetup(const HDeviceSetup& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

const HResourceType& HDeviceSetup::deviceType() const
{
    return h_ptr->m_deviceType;
}

HInclusionRequirement HDeviceSetup::inclusionRequirement() const
{
    return h_ptr->m_inclusionReq;
}

bool HDeviceSetup::isValid() const
{
    return h_ptr->m_deviceType.isValid() &&
           h_ptr->m_version > 0 &&
           h_ptr->m_inclusionReq != InclusionRequirementUnknown;
}

int HDeviceSetup::version() const
{
    return h_ptr->m_version;
}

void HDeviceSetup::setInclusionRequirement(HInclusionRequirement arg)
{
    h_ptr->m_inclusionReq = arg;
}

void HDeviceSetup::setDeviceType(const HResourceType& arg)
{
    h_ptr->m_deviceType = arg;
}

void HDeviceSetup::setVersion(int version)
{
    h_ptr->m_version = version;
}

/*******************************************************************************
 * HDevicesSetupData
 ******************************************************************************/
HDevicesSetupData::HDevicesSetupData() :
    m_deviceSetupInfos()
{
}

HDevicesSetupData::~HDevicesSetupData()
{
}

bool HDevicesSetupData::insert(const HDeviceSetup& setupInfo)
{
    if (!setupInfo.isValid())
    {
        return false;
    }

    HResourceType id = setupInfo.deviceType();
    if (m_deviceSetupInfos.contains(id))
    {
        return false;
    }

    m_deviceSetupInfos.insert(id, setupInfo);
    return true;
}

bool HDevicesSetupData::remove(const HResourceType& deviceType)
{
    if (m_deviceSetupInfos.contains(deviceType))
    {
        m_deviceSetupInfos.remove(deviceType);
        return true;
    }

    return false;
}

HDeviceSetup HDevicesSetupData::get(const HResourceType& deviceType) const
{
    return m_deviceSetupInfos.value(deviceType);
}

bool HDevicesSetupData::contains(const HResourceType& id) const
{
    return m_deviceSetupInfos.contains(id);
}

QSet<HResourceType> HDevicesSetupData::deviceTypes() const
{
    return m_deviceSetupInfos.keys().toSet();
}

int HDevicesSetupData::size() const
{
    return m_deviceSetupInfos.size();
}

bool HDevicesSetupData::isEmpty() const
{
    return m_deviceSetupInfos.isEmpty();
}

}
}
