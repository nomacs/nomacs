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

#ifndef HMEDIASERVER_ADAPTER_H_
#define HMEDIASERVER_ADAPTER_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClientDeviceAdapter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMediaServerAdapterPrivate;

/*!
 * This is a convenience class for using a UPnP Media Server device.
 *
 * \brief This class provides a simple API for using a server-side MediaServer
 * from the client-side. The class can be instantiated with any HClientDevice that
 * provides the mandatory functionality of a MediaServer.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-MediaServer-v3-Device.pdf>
 * UPnP MediaServer:3 specification</a>.
 *
 * \headerfile hmediaserver.h HMediaServerAdapter
 *
 * \ingroup hupnp_av_mediaserver
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMediaServerAdapter :
    public HClientDeviceAdapter
{
Q_OBJECT
H_DISABLE_COPY(HMediaServerAdapter)
H_DECLARE_PRIVATE(HMediaServerAdapter)

protected:

    // Documented in HClientDeviceAdapter
    virtual bool prepareDevice(HClientDevice*);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the \c QObject parent.
     *
     * \sa HClientDeviceAdapter::isReady()
     */
    HMediaServerAdapter(QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMediaServerAdapter();

    /*!
     * \brief Returns a pointer to a class enabling the access to the Media Server's
     * ContentDirectory.
     *
     * \return a pointer to a class enabling the access to the Media Server's
     * ContentDirectory.
     *
     * \remarks This is a mandatory service and as such a properly initialized
     * HMediaServerAdapter will always return a valid pointer.
     *
     * \sa HClientDeviceAdapter::isReady()
     */
    HContentDirectoryAdapter* contentDirectory() const;

    /*!
     * \brief Returns a pointer to a class enabling the access to the Media Server's
     * ConnectionManager.
     *
     * \return a pointer to a class enabling the access to the Media Server's
     * ConnectionManager.
     *
     * \remarks This is a mandatory service and as such a properly initialized
     * HMediaServerAdapter will always return a valid pointer.
     *
     * \sa HClientDeviceAdapter::isReady()
     */
    HConnectionManagerAdapter* connectionManager() const;

    /*!
     * \brief Returns a pointer to a class enabling the access to the Media Server's
     * AVTransport.
     *
     * \return a pointer to a class enabling the access to the Media Server's
     * AVTransport.
     *
     * \remarks This is an optional service and as such null pointer will be
     * returned in case the HClientDevice used to initialize the object did not
     * implement the service.
     *
     * \sa HClientDeviceAdapter::isReady()
     */
    HAvTransportAdapter* transportService(quint32 instanceId) const;

     /*!
     * \brief Returns a pointer to a class enabling the access to the Media Server's
     * ScheduledRecording.
     *
     * \return a pointer to a class enabling the access to the Media Server's
     * ScheduledRecording.
     *
     * \remarks This is an optional service and as such null pointer will be
     * returned in case the HClientDevice used to initialize the object did not
     * implement the service.
     */
    //HScheduledRecordingAdapter* scheduledRecordingService() const;
};

}
}
}

#endif /* HMEDIASERVER_ADAPTER_H_ */
