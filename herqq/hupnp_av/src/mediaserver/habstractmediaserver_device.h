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

#ifndef HABSTRACT_MEDIASERVERDEVICE_H_
#define HABSTRACT_MEDIASERVERDEVICE_H_

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
 * UPnP device type MediaServer:3 implementation.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-MediaServer-v3-Device.pdf>
 * UPnP MediaServer:3 specification</a>.
 *
 * \headerfile hmediaserver_device.h HMediaServerDevice
 *
 * \ingroup hupnp_av_mediaserver
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HDeviceHost, HServerDevice
 */
class H_UPNP_AV_EXPORT HAbstractMediaServerDevice :
    public HServerDevice
{
Q_OBJECT
H_DISABLE_COPY(HAbstractMediaServerDevice)

protected:

    /*!
     * \brief Creates a new instance.
     */
    HAbstractMediaServerDevice();

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAbstractMediaServerDevice() = 0;

    /*!
     * \brief Returns a pointer to the mandatory ContentDirectory service implementation.
     *
     * \return a pointer to the mandatory ContentDirectory service implementation.
     *
     * \remarks The ownership of the object is not transferred; do \b not delete
     * the return value.
     */
    virtual HAbstractContentDirectoryService* contentDirectory() const;

    /*!
     * \brief Returns a pointer to the mandatory ConnectionManager service implementation.
     *
     * \return a pointer to the mandatory ConnectionManager service implementation.
     *
     * \remarks The ownership of the object is not transferred; do \b not delete
     * the return value.
     */
    virtual HAbstractConnectionManagerService* connectionManager() const;

    /*!
     * \brief Returns a pointer to the optional AVTransport service implementation,
     * if such exists.
     *
     * \return a pointer to the optional AVTransport service implementation,
     * if such exists.
     *
     * \remarks The ownership of the object is not transferred; do \b not delete
     * the return value.
     */
    virtual HAbstractTransportService* avTransport() const;

    /*!
     *
     */
    //virtual HAbstractScheduledRecordingService* scheduledRecording() const;
};

}
}
}

#endif /* HABSTRACT_MEDIASERVERDEVICE_H_ */
