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

#ifndef HCONNECTIONINFO_H_
#define HCONNECTIONINFO_H_

#include <HUpnpAv/HConnectionManagerInfo>

#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HConnectionInfoPrivate;

/*!
 * \brief This class represents information of a connection managed by a UPnP A/V
 * ConnectionManager.
 *
 * \headerfile hconnection_info.h HConnectionInfo
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HConnectionInfo
{
private:

    QSharedDataPointer<HConnectionInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HConnectionInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \param protocolInfo specifies the protocol-related information, which
     * describes the capabilities of the connection.
     *
     * \sa isValid()
     */
    HConnectionInfo(qint32 connectionId, const HProtocolInfo& protocolInfo);

    /*!
     * \brief Creates a new instance.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \param avTransportId specifies the ID of the virtual AVTransport instance
     * associated with this connection.
     *
     * \param rcsId specifies the ID of the virtual RenderingControl instance
     * associated with this connection.
     *
     * \param protocolInfo specifies the protocol-related information, which
     * describes the capabilities of the connection.
     *
     * \param peerConnectionManager identifies the peer ConnectionManager.
     *
     * \param peerConnectionId specifies the ID of the connection at the
     * peer ConnectionManager side.
     *
     * \param direction specifies the direction of the data transfers the specified
     * connection will be used for from the point of view of the connection.
     *
     * \param status specifies the status of the connection.
     *
     * \sa isValid()
     */
    HConnectionInfo(
        qint32 connectionId,
        qint32 avTransportId,
        qint32 rcsId,
        const HProtocolInfo& protocolInfo,
        const HConnectionManagerId& peerConnectionManager,
        qint32 peerConnectionId,
        HConnectionManagerInfo::Direction direction,
        HConnectionManagerInfo::ConnectionStatus status);

    /*!
     * \brief Destroys the instance.
     */
    ~HConnectionInfo();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HConnectionInfo(const HConnectionInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HConnectionInfo& operator=(const HConnectionInfo&);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. connectionId() is >= 0 and
     * protocolInfo() is valid.
     */
    bool isValid() const;

    /*!
     * \brief Returns the ID of the connection issued by the ConnectionManager.
     *
     * \return The ID of the connection issued by the ConnectionManager. This ID
     * can be used to identify a connection \b only within the
     * ConnectionManager that issued it.
     */
    qint32 connectionId() const;

    /*!
     * \brief Returns the ID of the virtual RenderingControl instance
     * associated with this connection.
     *
     * \return The ID of the virtual RenderingControl instance
     * associated with this connection.
     *
     * \sa setRcsId()
     */
    qint32 rcsId() const;

    /*!
     * \brief Returns the ID of the virtual AVTransport instance
     * associated with this connection.
     *
     * \return The ID of the virtual AVTransport instance
     * associated with this connection.
     *
     * \sa setAvTransportId()
     */
    qint32 avTransportId() const;

    /*!
     * \brief Returns the protocol-related information, which
     * describes the capabilities of the connection.
     *
     * \return The protocol-related information, which
     * describes the capabilities of the connection.
     *
     * \sa setProtocolInfo()
     */
    HProtocolInfo protocolInfo() const;

    /*!
     * \brief Returns information identifying the peer ConnectionManager.
     *
     * \return information identifying the peer ConnectionManager.
     *
     * \sa setPeerConnectionManager()
     */
    HConnectionManagerId peerConnectionManager() const;

    /*!
     * \brief Returns the ID of the connection at the peer ConnectionManager side.
     *
     * \return The ID of the connection at the peer ConnectionManager side.
     *
     * \sa setPeerConnectionId()
     */
    qint32 peerConnectionId() const;

    /*!
     * \brief Returns the direction of the data transfers the specified
     * connection will be used for from the point of view of the connection.
     *
     * \return The direction of the data transfers the specified
     * connection will be used for from the point of view of the connection.
     *
     * \sa setDirection()
     */
    HConnectionManagerInfo::Direction direction() const;

    /*!
     * \brief Returns the status of the connection.
     *
     * \return The status of the connection.
     *
     * \sa setStatus()
     */
    HConnectionManagerInfo::ConnectionStatus status() const;

    /*!
     * \brief Specifies the the ID of the virtual RenderingControl instance
     * associated with this connection.
     *
     * \param arg specifies the ID of the virtual RenderingControl instance
     * associated with this connection.
     *
     * \sa rcsId()
     */
    void setRcsId(qint32 arg);

    /*!
     * \brief Specifies the ID of the virtual AVTransport instance
     * associated with this connection.
     *
     * \param arg specifies the ID of the virtual AVTransport instance
     * associated with this connection.
     *
     * \sa avTransportId()
     */
    void setAvTransportId(qint32 arg);

    /*!
     * \brief Specifies information identifying the peer ConnectionManager.
     *
     * \param arg specifies information identifying the peer ConnectionManager.
     *
     * \sa peerConnectionManager()
     */
    void setPeerConnectionManager(const HConnectionManagerId& arg);

    /*!
     * \brief Specifies the ID of the connection at the peer ConnectionManager side.
     *
     * \param arg specifies the ID of the connection at the peer
     * ConnectionManager side.
     *
     * \sa peerConnectionId()
     */
    void setPeerConnectionId(qint32 arg);

    /*!
     * \brief Specifies the direction of the data transfers the specified
     * connection will be used for from the point of view of the connection.
     *
     * \param arg specifies the direction of the data transfers the specified
     * connection will be used for from the point of view of the connection.
     *
     * \sa direction()
     */
    void setDirection(HConnectionManagerInfo::Direction arg);

    /*!
     * \brief Specifies the status of the connection.
     *
     * \param arg specifies the status of the connection.
     *
     * \sa status()
     */
    void setStatus(HConnectionManagerInfo::ConnectionStatus arg);

    /*!
     * \brief Specifies the protocol-related information, which
     * describes the capabilities of the connection.
     *
     * \param protocolInfo specifies the protocol-related information, which
     * describes the capabilities of the connection.
     *
     * \sa protocolInfo()
     */
    void setProtocolInfo(const HProtocolInfo& protocolInfo);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HConnectionInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HConnectionInfo&, const HConnectionInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HConnectionInfo
 */
inline bool operator!=(const HConnectionInfo& obj1, const HConnectionInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HCONNECTIONINFO_H_ */
