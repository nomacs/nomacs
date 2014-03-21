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

#ifndef HMEDIARENDERER_ADAPTER_H_
#define HMEDIARENDERER_ADAPTER_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClientDeviceAdapter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMediaRendererAdapterPrivate;

/*!
 * This is a convenience class for using a UPnP Media Renderer device.
 *
 * \brief This class provides a simple API for using a server-side MediaRenderer
 * from the client-side. The class can be instantiated with any HClientDevice that
 * provides the mandatory functionality of a MediaRenderer.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-MediaRenderer-v2-Device.pdf>
 * UPnP MediaRenderer:2 specification</a>.
 *
 * \headerfile hmediarenderer.h HMediaRendererAdapter
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMediaRendererAdapter :
    public HClientDeviceAdapter
{
Q_OBJECT
H_DISABLE_COPY(HMediaRendererAdapter)
H_DECLARE_PRIVATE(HMediaRendererAdapter)

private Q_SLOTS:

    void getProtocolInfoCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter*,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HProtocolInfoResult>&);

    void prepareForConnectionCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter*,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HPrepareForConnectionResult>&);

    void connectionCompleteCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter*,
        const Herqq::Upnp::HClientAdapterOpNull&);

    void getCurrentConnectionIDsCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter*,
        const Herqq::Upnp::HClientAdapterOp<QList<quint32> >&);

    void getCurrentConnectionInfoCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter*,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HConnectionInfo>&);

protected:

    virtual bool prepareDevice(HClientDevice* device);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject.
     *
     * \sa HClientDeviceAdapter::isReady()
     */
    HMediaRendererAdapter(QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMediaRendererAdapter();

    /*!
     * Indicates if the Media Renderer device advertises a capability for
     * preparing a new connection.
     *
     * \return \e true if the Media Renderer device should be able to prepare
     * a new connection.
     */
    bool canPrepareNewConnection() const;

    /*!
     * \brief Returns a pointer to a class enabling the access to the Media Renderer's
     * ConnectionManager.
     *
     * \return a pointer to a class enabling the access to the Media Renderer's
     * ConnectionManager.
     *
     * \remarks This is a mandatory service and as such a properly initialized
     * HMediaRendererAdapter will always return a valid pointer.
     *
     * \sa HClientDeviceAdapter::isReady()
     */
    HConnectionManagerAdapter* connectionManager() const;

    /*!
     * Attempts to prepare a new connection with the Media Renderer.
     *
     * \param info specifies protocol-related information of how the intended
     * use of the connection.
     *
     * \return \e true in case request was successfully dispatched to the
     * Media Renderer. If the device is able to prepare a new connection with
     * the provided parameters the connectionReady() signal will be emitted.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa connectionReady()
     */
    HClientAdapterOp<qint32> prepareNewConnection(const HProtocolInfo& info);

    /*!
     * Attempts to prepare a new connection with the Media Renderer.
     *
     * \param info specifies protocol-related information of how the intended
     * use of the connection.
     *
     * \param peerCm specifies information of the peer Connection Manager.
     *
     * \return \e true in case request was successfully dispatched to the
     * Media Renderer. If the device is able to prepare a new connection with
     * the provided parameters the connectionReady() signal will be emitted.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa connectionReady()
     */
    HClientAdapterOp<qint32> prepareNewConnection(
        const HProtocolInfo& info, const HConnectionManagerId& peerCm);

    /*!
     * Attempts to retrieve the currently active connections from the
     * Media Renderer.
     *
     * \return \e true in case the request was successfully dispatched to the
     * Media Renderer. The connectionReady() signal will be emitted for each
     * active connection received from the device.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa connectionReady(), getConnection(), prepareNewConnection()
     */
    HClientAdapterOp<qint32> getCurrentConnections();

    /*!
     * Attempts to retrieve the currently active connection with the specified
     * ID from the Media Renderer.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \return \e true in case the request was successfully dispatched to the
     * Media Renderer. The connectionReady() signal will be emitted for each
     * active connection received from the device.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa connectionReady(), getCurrentConnections(), prepareNewConnection()
     */
    HClientAdapterOp<qint32> getConnection(qint32 connectionId);

    /*!
     * \brief Indicates if the instance has cached an HConnection with the specified
     * ID.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \return \e true if the instance has cached an HConnection with the specified
     * ID.
     *
     * \sa takeConnection(), takeConnections(), connection()
     */
    bool hasConnection(qint32 connectionId) const;

    /*!
     * \brief Returns a cached HConnection with the specified ID, if any.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \return a cached HConnection with the specified ID, if any.
     *
     * \remarks The ownership of the returned object is not transferred.
     *
     * \sa hasConnection(), takeConnection(), takeConnections(), connections()
     */
    HConnection* connection(qint32 connectionId) const;

    /*!
     * \brief Returns the HConnection instances that are currently cached by this instance.
     *
     * \return The HConnection instances that are currently cached by this instance.
     *
     * \sa hasConnection(), takeConnections(), connection(), takeConnection()
     */
    HConnections connections() const;

    /*!
     * Takes the ownership of the specified connection.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \return a pointer to the specified HConnection, or null if the specified
     * \a connectionId does not correspond to any cached HConnection.
     *
     * \sa hasConnection(), takeConnections(), connection(), connections()
     */
    HConnection* takeConnection(qint32 connectionId);

    /*!
     * Takes the ownership of every HConnection instance cached by this
     * instance.
     *
     * \return every HConnection instance cached by this instance. The
     * ownership of the connections is transferred to the caller.
     *
     * \sa hasConnection(), takeConnection(), connection(), connections()
     */
    HConnections takeConnections();

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when a previously started connection related
     * asynchronous operation failed.
     *
     * \param source specifies the source of the signal.
     *
     * \param op specifies the operation that failed.
     *
     * \sa connectionReady()
     */
    void error(
        Herqq::Upnp::Av::HMediaRendererAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<qint32>& op);

    /*!
     * \brief This signal is emitted when an HConnection object is ready to be retrieved.
     *
     * \param source specifies the source of the signal.
     *
     * \param connectionId specifies the ID of the connection that is ready.
     *
     * \sa takeConnection(), connection(), error()
     */
    void connectionReady(
        Herqq::Upnp::Av::HMediaRendererAdapter* source, qint32 connectionId);
};

}
}
}

#endif /* HMEDIARENDERER_ADAPTER_H_ */
