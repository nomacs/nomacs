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

#include "hdeviceudn.h"

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HResourceType>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HDeviceUdn>(
        "Herqq::Upnp::Av::HDeviceUdn");

    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HDeviceUdnPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HDeviceUdnPrivate)

public:
    HUdn m_deviceUdn;
    HResourceType m_serviceType;
    HServiceId m_serviceId;

    HDeviceUdnPrivate() : m_deviceUdn(), m_serviceType(), m_serviceId()
    {
    }
};


HDeviceUdn::HDeviceUdn() :
    h_ptr(new HDeviceUdnPrivate())
{
}

HDeviceUdn::HDeviceUdn(
    const HUdn& udn, const HResourceType& srvType, const HServiceId& srvId,
    HValidityCheckLevel checkLevel) :
        h_ptr(new HDeviceUdnPrivate())
{
    if (udn.isValid(checkLevel) && srvType.isValid() &&
        srvId.isValid(checkLevel))
    {
        h_ptr->m_deviceUdn = udn;
        h_ptr->m_serviceType = srvType;
        h_ptr->m_serviceId = srvId;
    }
}

HDeviceUdn::HDeviceUdn(const HDeviceUdn& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HDeviceUdn& HDeviceUdn::operator=(const HDeviceUdn& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HDeviceUdn::~HDeviceUdn()
{
}

bool HDeviceUdn::isValid(HValidityCheckLevel checkLevel) const
{
    return h_ptr->m_deviceUdn.isValid(checkLevel);
}

const HUdn& HDeviceUdn::udn() const
{
    return h_ptr->m_deviceUdn;
}

const HResourceType& HDeviceUdn::serviceType() const
{
    return h_ptr->m_serviceType;
}

const HServiceId& HDeviceUdn::serviceId() const
{
    return h_ptr->m_serviceId;
}

bool operator==(const HDeviceUdn& obj1, const HDeviceUdn& obj2)
{
    return obj1.serviceId() == obj2.serviceId() &&
           obj1.serviceType() == obj2.serviceType() &&
           obj1.udn() == obj2.udn();
}

}
}
}

