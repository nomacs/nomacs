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

#ifndef HAV_DEVICEMODEL_CREATOR_H_
#define HAV_DEVICEMODEL_CREATOR_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HDeviceModelCreator>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAvDeviceModelCreatorPrivate;

/*!
 * \brief This class is used to create instances of HUPnP A/V's device model classes.
 *
 * \headerfile hav_devicemodel_creator.h HAvDeviceModelCreator
 *
 * \ingroup hupnp_av
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAvDeviceModelCreator :
    public HDeviceModelCreator
{
H_DISABLE_COPY(HAvDeviceModelCreator)

protected:

    HAvDeviceModelCreatorPrivate* h_ptr;

protected:

    // Documented in HClonable
    virtual HAvDeviceModelCreator* newInstance() const;

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;

public:

    /*!
     * \brief Creates a new instance.
     */
    HAvDeviceModelCreator();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAvDeviceModelCreator();

    /*!
     * \brief Returns the configuration that is used to initialize MediaServer
     * device instances.
     *
     * \return The configuration that is used to initialize MediaServer device
     * instances.
     *
     * \sa setMediaServerConfiguration()
     */
    const HMediaServerDeviceConfiguration* mediaServerConfiguration() const;

    /*!
     * \brief Returns the configuration that is used to initialize MediaRenderer
     * device instances.
     *
     * \return The configuration that is used to initialize MediaRenderer
     * device instances.
     *
     * \sa setMediaRendererConfiguration()
     */
    const HMediaRendererDeviceConfiguration* mediaRendererConfiguration() const;

    /*!
     * \brief Sets the configuration that is used to initialize MediaServer
     * device instances.
     *
     * \param conf specifies the configuration that is used to initialize
     * MediaServer device instances.
     *
     * \sa mediaServerConfiguration()
     */
    void setMediaServerConfiguration(const HMediaServerDeviceConfiguration& conf);

    /*!
     * \brief Sets the configuration that is used to initialize MediaRenderer
     * device instances.
     *
     * \param conf specifies the configuration that is used to initialize
     * MediaRenderer device instances.
     *
     * \s mediaRendererConfiguration()
     */
    void setMediaRendererConfiguration(const HMediaRendererDeviceConfiguration& conf);

    // Documented in HDeviceModelCreator
    virtual HServerDevice* createDevice(const HDeviceInfo& info) const;

    // Documented in HDeviceModelCreator
    virtual HServerService* createService(
        const HServiceInfo&, const HDeviceInfo&) const;

    // Documented in HClonable
    virtual HAvDeviceModelCreator* clone() const;
};

}
}
}

#endif /* HAV_DEVICEMODEL_CREATOR_H_ */
