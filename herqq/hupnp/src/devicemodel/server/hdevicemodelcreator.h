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

#ifndef HDEVICEMODEL_CREATOR_H_
#define HDEVICEMODEL_CREATOR_H_

#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

class HDeviceModelCreatorPrivate;

/*!
 * A protocol class for creating HServerDevice and HServerService instances.
 *
 * The primary purpose of this protocol class is to build instances of the HUPnP's
 * \ref hupnp_devicemodel at server-side. If you wish to host a device in an
 * HDeviceHost you have to derive from this class and override its
 * abstract methods.
 *
 * \headerfile hdevicemodelcreator.h HDeviceModelCreator
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa hupnp_devicehosting, HServerDevice, HServerService
 */
class H_UPNP_CORE_EXPORT HDeviceModelCreator :
    public HClonable
{
H_DISABLE_COPY(HDeviceModelCreator)

public:

    /*!
     * \brief Creates a new instance.
     */
    HDeviceModelCreator();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HDeviceModelCreator();

    /*!
     * Creates a device matching the provided device information.
     *
     * \param info specifies information of the device type the creator is asked
     * to create.
     *
     * \return a heap allocated device matching the provided device information
     * or \c null in case the creator does not recognize the specified device type.
     *
     * \remarks The ownership of the created device is transferred to the caller.
     */
    virtual HServerDevice* createDevice(const HDeviceInfo& info) const;

    /*!
     * Creates a service matching the provided service information.
     *
     * \param serviceInfo specifies information of the service type the
     * creator is asked to create.
     *
     * \param parentDeviceInfo specifies information about the parent UPnP device
     * that contains this service.
     *
     * \return a heap allocated service matching the provided service information
     * or \c null in case the creator does not recognize the specified service type.
     *
     * \remarks The ownership of the created service is transferred to the caller.
     */
    virtual HServerService* createService(
        const HServiceInfo& serviceInfo, const HDeviceInfo& parentDeviceInfo) const = 0;

    //
    // Documented in HClonable
    virtual HDeviceModelCreator* clone() const;
};

}
}

#endif /* HDEVICEMODEL_CREATOR_H_ */
