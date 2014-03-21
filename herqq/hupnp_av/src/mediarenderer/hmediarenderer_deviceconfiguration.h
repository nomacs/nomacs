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

#ifndef HMEDIARENDERER_DEVICECONFIGURATION_H_
#define HMEDIARENDERER_DEVICECONFIGURATION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HRendererConnectionManager>

#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMediaRendererDeviceConfigurationPrivate;

/*!
 * \brief This class is used to pass Media Renderer configuration information to a
 * HAvDeviceModelCreator instance.
 *
 * \headerfile hmediarenderer_deviceconfiguration.h HMediaRendererDeviceConfiguration
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAvDeviceModelCreator
 */
class H_UPNP_AV_EXPORT HMediaRendererDeviceConfiguration :
    public HClonable
{
H_DISABLE_COPY(HMediaRendererDeviceConfiguration)

protected:

    HMediaRendererDeviceConfigurationPrivate* h_ptr;

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;
    // Documented in HClonable
    virtual HMediaRendererDeviceConfiguration* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     *
     * \sa isValid(), setRendererConnectionManager()
     */
    HMediaRendererDeviceConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMediaRendererDeviceConfiguration();

    // Documented in HClonable
    virtual HMediaRendererDeviceConfiguration* clone() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. the rendererConnectionManager()
     * is provided.
     */
    virtual bool isValid() const;

    /*!
     * \brief Returns the mandatory renderer connection manager the
     * Media Renderer implementation should use.
     *
     * \return The mandatory renderer connection manager the Media Renderer
     * implementation should use.
     *
     * \sa setRendererConnectionManager()
     */
    HRendererConnectionManager* rendererConnectionManager() const;

    /*!
     * \brief Indicates if the configuration object has the ownership of the renderer
     * connection manager.
     *
     * \return \e true if the configuration object has the ownership of the
     * renderer connection manager.
     *
     * \remarks This function returns \e false when no renderer connection
     * manager is set.
     *
     * \sa setRendererConnectionManager(), rendererConnectionManager(),
     * detachRendererConnectionManager()
     */
    bool hasOwnershipOfRendererConnectionManager() const;

    /*!
     * \brief Sets the mandatory renderer connection manager the
     * Media Renderer implementation should use.
     *
     * \param rcm specifies the renderer connection manager the
     * Media Renderer implementation should use.
     *
     * \param takeOwnership specifies whether the ownership of the renderer
     * connection manager is passed to the MediaRendererDevice instance.
     * If \e true, the MediaRendererDevice will delete the renderer connection
     * manager once it is done using it. Otherwise the caller
     * is responsible for ensuring that the renderer connection manager is not
     * deleted before the MediaRendererDevice.
     *
     * \sa rendererConnectionManager()
     */
    void setRendererConnectionManager(
        HRendererConnectionManager* rcm, bool takeOwnership);

    /*!
     * \brief Detaches the renderer connection manager associated with this
     * instance, if any.
     *
     * If the object has a renderer connection manager set and the object
     * has the ownership of it, this call decreases the reference count of the
     * associated renderer connection manager. In this case if the reference
     * count goes to zero the associated renderer connection manager is deleted.
     *
     * \sa setRendererConnectionManager()
     */
    void detachRendererConnectionManager();
};

}
}
}

#endif /* HMEDIARENDERER_DEVICECONFIGURATION_H_ */
