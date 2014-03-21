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

#ifndef HDEVICEMODEL_INFOPROVIDER_H_
#define HDEVICEMODEL_INFOPROVIDER_H_

#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

class HDeviceModelInfoProviderPrivate;

/*!
 * A protocol class for providing information that is used to validate
 * components of UPnP's device architecture and to setup components of
 * HUPnP's device model.
 *
 * The main purpose of this class is to define an interface that enables the
 * users to provide information that HUPnP can use for verification and
 * validation purposes. Although optional, this information can be especially
 * useful when provided to HDeviceHost via HDeviceHostConfiguration. This enables
 * HUPnP to use the information to verify that device and service descriptions
 * are setup according to the specified information.
 *
 * The benefit of this is that your custom device model components can rest
 * assured that all the required state variables, actions, services and
 * embedded devices are properly defined and initialized before the instantiation
 * of the HUPnP's device model (device tree) is published for control points to use.
 *
 * The benefits of this may be somewhat difficult to realize at first, since most
 * of the time it is you, the user, who provides the implementation and
 * the description documents. Apart from inadvertent mistakes, you usually
 * get those right. However, when someone else provides the implementation of
 * the HUPnP's device model or the description documents, mismatches can easily
 * occur and this is where the benefits of this additional information are truly
 * useful. Remember, in UPnP architecture the description documents are used to
 * marshal device model information from servers to clients. If the description
 * documents do not accurately reflect the server-side implementation, the
 * client-side may not be able to correctly invoke the server-side.
 *
 * \headerfile hdevicemodel_infoprovider.h HDeviceModelInfoProvider
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa hupnp_devicehosting, HDeviceHostConfiguration
 */
class H_UPNP_CORE_EXPORT HDeviceModelInfoProvider :
    public HClonable
{
H_DISABLE_COPY(HDeviceModelInfoProvider)

public:

    /*!
     * \brief Creates a new instance.
     */
    HDeviceModelInfoProvider();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HDeviceModelInfoProvider() = 0;

    /*!
     * \brief Returns information of the services the specified device type may contain.
     *
     * \param info specifies the device type.
     *
     * \return information of the services the specified device type may contain.
     */
    virtual HServicesSetupData servicesSetupData(const HDeviceInfo& info) const;

    /*!
     * \brief Returns information of the embedded devices the specified device type may contain.
     *
     * \param info specifies the device type.
     *
     * \return information of the embedded devices the specified device type may contain.
     */
    virtual HDevicesSetupData embedddedDevicesSetupData(
        const HDeviceInfo& info) const;

    /*!
     * \brief Returns information of the actions the specified service type may contain.
     *
     * \param serviceInfo specifies the service type.
     *
     * \param parentDeviceInfo specifies information about the parent UPnP device
     * that contains this service.
     *
     * \return information of the actions the specified service type may contain.
     */
    virtual HActionsSetupData actionsSetupData(
        const HServiceInfo& serviceInfo, const HDeviceInfo& parentDeviceInfo) const;

    /*!
     * \brief Returns information of the state variables the specified service type
     * may contain.
     *
     * \param serviceInfo specifies the service type.
     *
     * \param parentDeviceInfo specifies information about the parent UPnP device
     * that contains this service.
     *
     * \return information of the state variables the specified service type
     * may contain.
     */
    virtual HStateVariablesSetupData stateVariablesSetupData(
        const HServiceInfo& serviceInfo, const HDeviceInfo& parentDeviceInfo) const;

    //
    // Documented in HClonable
    virtual HDeviceModelInfoProvider* clone() const;
};

}
}

#endif /* HDEVICEMODEL_INFOPROVIDER_H_ */
