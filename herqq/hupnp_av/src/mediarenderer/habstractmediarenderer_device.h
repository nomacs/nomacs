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

#ifndef HABSTRACTMEDIARENDERER_DEVICE_H_
#define HABSTRACTMEDIARENDERER_DEVICE_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HServerDevice>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class specifies an interface for accessing a standardized
 * UPnP device type MediaRenderer:2 implementation.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-MediaRenderer-v2-Device.pdf>
 * UPnP MediaRenderer:2 specification</a>.
 *
 * \headerfile hmediarenderer_device.h HAbstractMediaRendererDevice
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HDeviceHost, HServerDevice
 */
class H_UPNP_AV_EXPORT HAbstractMediaRendererDevice :
    public HServerDevice
{
Q_OBJECT
H_DISABLE_COPY(HAbstractMediaRendererDevice)

protected:

    /*!
     * \brief Creates a new instance.
     */
    HAbstractMediaRendererDevice();

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAbstractMediaRendererDevice() = 0;

    /*!
     * \brief Returns a pointer to the mandatory RenderingControl service implementation.
     *
     * \return a pointer to the mandatory RenderingControl service implementation.
     *
     * \remarks the ownership of the object is not transferred; do \b not delete
     * the return value.
     *
     * \sa connectionManager(), avTransport()
     */
    virtual HAbstractRenderingControlService* renderingControl() const;

    /*!
     * \brief Returns a pointer to the mandatory ConnectionManager service implementation.
     *
     * \return a pointer to the mandatory ConnectionManager service implementation.
     *
     * \remarks the ownership of the object is not transferred; do \b not delete
     * the return value.
     *
     * \sa renderingControl(), avTransport()
     */
    virtual HAbstractConnectionManagerService* connectionManager() const;

    /*!
     * \brief Returns a pointer to the mandatory AVTransport service implementation.
     *
     * \return a pointer to the mandatory AVTransport service implementation.
     *
     * \remarks the ownership of the object is not transferred; do \b not delete
     * the return value.
     *
     * \sa connectionManager(), renderingControl()
     */
    virtual HAbstractTransportService* avTransport() const;
};

}
}
}

#endif /* HABSTRACTMEDIARENDERER_DEVICE_H_ */
