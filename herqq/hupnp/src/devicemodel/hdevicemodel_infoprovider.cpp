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

#include "hdevicemodel_infoprovider.h"

#include "hdevices_setupdata.h"
#include "hactions_setupdata.h"
#include "hservices_setupdata.h"
#include "hstatevariables_setupdata.h"

namespace Herqq
{

namespace Upnp
{

HDeviceModelInfoProvider::HDeviceModelInfoProvider()
{
}

HDeviceModelInfoProvider::~HDeviceModelInfoProvider()
{
}

HServicesSetupData HDeviceModelInfoProvider::servicesSetupData(
    const HDeviceInfo&) const
{
    return HServicesSetupData();
}

HDevicesSetupData HDeviceModelInfoProvider::embedddedDevicesSetupData(
    const HDeviceInfo&) const
{
    return HDevicesSetupData();
}

HActionsSetupData HDeviceModelInfoProvider::actionsSetupData(
    const HServiceInfo&, const HDeviceInfo&) const
{
    return HActionsSetupData();
}

HStateVariablesSetupData HDeviceModelInfoProvider::stateVariablesSetupData(
    const HServiceInfo&, const HDeviceInfo&) const
{
    return HStateVariablesSetupData();
}

HDeviceModelInfoProvider* HDeviceModelInfoProvider::clone() const
{
    return static_cast<HDeviceModelInfoProvider*>(HClonable::clone());
}

}
}
