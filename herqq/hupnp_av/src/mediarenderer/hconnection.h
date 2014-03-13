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

#ifndef HCONNECTION_H
#define HCONNECTION_H

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HConnectionPrivate;

/*!
 * \brief This class represents a \e connection to a MediaRenderer device.
 *
 * \headerfile hconnection.h HConnection
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HConnection :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HConnection)

private Q_SLOTS:

    void currentConnectionIdsChanged(
        Herqq::Upnp::Av::HConnectionManagerAdapter*,
        const QList<quint32>& currentIds);

    void lastChangeReceived(
        Herqq::Upnp::Av::HRenderingControlAdapter*, const QString&);

    void lastChangeReceived(
        Herqq::Upnp::Av::HAvTransportAdapter*, const QString&);

protected:

    HConnectionPrivate* h_ptr;

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param info specifies information about the connection.
     *
     * \param cm specifies the ConnectionManager to which this connection instance
     * is logically connected.
     *
     * \param avt specifies the virtual AVTransport instance associated with
     * this connection. Note, the instance takes the ownership of the provided
     * object.
     *
     * \param rcs specifies the virtual RenderingControl instance associated with
     * this connection. Note, the instance takes the ownership of the provided
     * object.
     *
     * \param parent specifies the \c QObject parent of this instance.
     */
    HConnection(
        const HConnectionInfo& info,
        HConnectionManagerAdapter* cm,
        HAvTransportAdapter* avt,
        HRenderingControlAdapter* rcs,
        QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HConnection();

    /*!
     * \brief Returns information about the connection.
     *
     * \return information about the connection.
     */
    const HConnectionInfo& info() const;

    /*!
     * \brief Returns the virtual AVTransport instance associated with
     * this connection.
     *
     * \return The virtual AVTransport instance associated with
     * this connection.
     *
     * \remarks The ownership of the object is not transferred; do \b not delete
     * the return value.
     */
    HAvTransportAdapter* transport() const;

    /*!
     * \brief Returns the virtual RenderingControl instance associated with
     * this connection.
     *
     * \return The virtual RenderingControl instance associated with
     * this connection.
     *
     * \remarks The ownership of the object is not transferred; do \b not delete
     * the return value.
     */
    HRenderingControlAdapter* renderingControl() const;

    /*!
     * Indicates if the instance should attempt to close the underlying connection
     * identified by the info().connectionId() when this instance is being shut down.
     *
     * \return \e true if the instance should attempt to close the underlying connection
     * identified by the info().connectionId() when this instance is being shut down.
     *
     * \sa setAutoCloseConnection()
     */
    bool autoCloseConnection() const;

    /*!
     * Indicates if the connection appears to be valid and usable.
     *
     * \return \e true if the connection appears to be valid and usable.
     *
     * \sa invalidated()
     */
    bool isValid() const;

    /*!
     * Specifies whether the instance should attempt to close the underlying connection
     * identified by the info().connectionId() when this instance is being shut down.
     *
     * \param enable specifies whether the instance should attempt to close the
     * underlying connection identified by the info().connectionId() when this
     * instance is being shut down.
     *
     * \sa autoCloseConnection()
     */
    void setAutoCloseConnection(bool enable);

Q_SIGNALS:

    /*!
     * This signal is emitted when the connection is rendered invalid at the
     * server-side.
     *
     * \param source specifies the HConnection that sent the event.
     *
     * \sa isValid()
     */
    void invalidated(Herqq::Upnp::Av::HConnection* source);

    /*!
     * This signal is emitted when the state of the virtual AVTransport instance
     * associated with this connection has changed at the server-side.
     *
     * \param source specifies the HConnection that sent the event.
     *
     * \param info specifies information about the changed state.
     *
     * \sa isValid()
     */
    void avTransportStateChanged(
        Herqq::Upnp::Av::HConnection* source,
        const Herqq::Upnp::Av::HAvtLastChangeInfos& info);

    /*!
     * This signal is emitted when the state of the virtual RenderingControl instance
     * associated with this connection has changed at the server-side.
     *
     * \param source specifies the HConnection that sent the event.
     *
     * \param info specifies information about the changed state.
     *
     * \sa isValid()
     */
    void renderingControlStateChanged(
        Herqq::Upnp::Av::HConnection* source,
        const Herqq::Upnp::Av::HRcsLastChangeInfos& info);
};

}
}
}

#endif /* HCONNECTION_H */
