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

#ifndef HMEDIASERVER_DEVICECONFIGURATION_H_
#define HMEDIASERVER_DEVICECONFIGURATION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMediaServerDeviceConfigurationPrivate;

/*!
 * \brief This class is used to pass Media Server configuration information to a
 * HAvDeviceModelCreator instance.
 *
 * \headerfile hmediaserver_deviceconfiguration.h HMediaServerDeviceConfiguration
 *
 * \ingroup hupnp_av_mediaserver
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAvDeviceModelCreator
 */
class H_UPNP_AV_EXPORT HMediaServerDeviceConfiguration :
    public HClonable
{
H_DISABLE_COPY(HMediaServerDeviceConfiguration)

protected:

    HMediaServerDeviceConfigurationPrivate* h_ptr;

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;
    // Documented in HClonable
    virtual HMediaServerDeviceConfiguration* newInstance() const;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HMediaServerDeviceConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMediaServerDeviceConfiguration();

    /*!
     * \brief Specifies the configuration information of a ContentDirectory implementation.
     *
     * \param arg specifies the configuration information of a ContentDirectory implementation.
     *
     * \sa contentDirectoryServiceConfiguration()
     */
    void setContentDirectoryConfiguration(
        const HContentDirectoryServiceConfiguration& arg);

    /*!
     * \brief Returns the configuration information of a ContentDirectory implementation.
     *
     * \return The configuration information of a ContentDirectory implementation.

     * \sa setContentDirectoryConfiguration()
     */
    const HContentDirectoryServiceConfiguration* contentDirectoryConfiguration() const;

    /*!
     * \brief Specifies the configuration information of a ConnectionManager implementation.
     *
     * \param arg specifies the configuration information of a ConnectionManager implementation.
     *
     * \sa connectionManagerConfiguration()
     */
    void setConnectionManagerConfiguration(
        const HConnectionManagerServiceConfiguration& arg);

    /*!
     * \brief Returns the configuration information of a ConnectionManager implementation.
     *
     * \return The configuration information of a ConnectionManager implementation.

     * \sa setConnectionManagerConfiguration()
     */
    const HConnectionManagerServiceConfiguration* connectionManagerConfiguration() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e.
     * the contentDirectoryServiceConfiguration() is appropriately defined.
     */
    virtual bool isValid() const;

    // Documented in HClonable
    virtual HMediaServerDeviceConfiguration* clone() const;
};

}
}
}

#endif /* HMEDIASERVER_DEVICECONFIGURATION_H_ */
