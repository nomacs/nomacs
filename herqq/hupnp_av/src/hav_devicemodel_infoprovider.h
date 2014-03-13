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

#ifndef HAV_DEVICEMODEL_INFOPROVIDER_H_
#define HAV_DEVICEMODEL_INFOPROVIDER_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HDeviceModelInfoProvider>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to provide information of the components that comprise
 * the UPnP A/V device architecture.
 *
 * \headerfile hav_devicemodel_infoprovider.h HAvDeviceModelInfoProvider
 *
 * \ingroup hupnp_av
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAvDeviceModelInfoProvider :
    public HDeviceModelInfoProvider
{
H_DISABLE_COPY(HAvDeviceModelInfoProvider)

protected:

    // Documented in HClonable
    virtual HAvDeviceModelInfoProvider* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     */
    HAvDeviceModelInfoProvider();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAvDeviceModelInfoProvider();

    // Documented in HDeviceModelInfoProvider
    virtual HServicesSetupData servicesSetupData(const HDeviceInfo&) const;

    // Documented in HDeviceModelInfoProvider
    virtual HActionsSetupData actionsSetupData(
        const HServiceInfo&, const HDeviceInfo&) const;

    // Documented in HDeviceModelInfoProvider
    virtual HStateVariablesSetupData stateVariablesSetupData(
        const HServiceInfo&, const HDeviceInfo&) const;

    // Documented in HClonable
    virtual HAvDeviceModelInfoProvider* clone() const;
};

}
}
}

#endif /* HAV_DEVICEMODEL_INFOPROVIDER_H_ */
