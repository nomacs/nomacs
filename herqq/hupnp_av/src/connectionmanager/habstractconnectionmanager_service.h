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

#ifndef HABSTRACTCONNECTIONMANAGER_SERVICE_H_
#define HABSTRACTCONNECTIONMANAGER_SERVICE_H_

#include <HUpnpAv/HConnectionManagerInfo>
#include <HUpnpCore/HServerService>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAbstractConnectionManagerServicePrivate;

/*!
 * \brief This is an abstract base class for implementing the standardized UPnP service type
 * ConnectionManager:2.
 *
 * The main purpose of this class is to marshal UPnP action invocations into
 * the virtual methods introduced in this class. It is an ideal choice for a
 * base class when you wish to implement the ConnectionManager service in full.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-ConnectionManager-v2-Service.pdf>
 * UPnP ConnectionManager:2 specification</a>.
 *
 * \headerfile habstractconnectionmanager_service.h HAbstractConnectionManagerService
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAbstractConnectionManagerService :
    public HServerService
{
Q_OBJECT
H_DECLARE_PRIVATE(HAbstractConnectionManagerService)
H_DISABLE_COPY(HAbstractConnectionManagerService)

protected:

    /*!
     * \brief Creates a new instance.
     */
    HAbstractConnectionManagerService();

    //
    // \internal
    //
    HAbstractConnectionManagerService(HAbstractConnectionManagerServicePrivate& dd);

    // Documented in HServerService
    virtual HActionInvokes createActionInvokes();

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAbstractConnectionManagerService();

    /*!
     * \brief Returns the protocol related information this instance supports in its
     * current state.
     *
     * \param result contains the result of the operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     */
    virtual qint32 getProtocolInfo(HProtocolInfoResult* result) = 0;

    /*!
     * \brief Prepares the device for the purpose of sending or receiving data.
     *
     * \param remoteProtocolInfo specifies the protocol-related information
     * that \b must be used to transfer the content.
     *
     * \param peerConnectionManager identifies the ConnectionManager service on
     * the other side of the connection.
     *
     * \param peerConnectionId specifies the ID of the connection the peer
     * ConnectionManager has allocated for this connection. This can be set to
     * \c -1 if the ID is not known. This is the case if the peer does not
     * implement this action or prepareForConnection() has not been invoked on
     * that ConnectionManager yet.
     *
     * \param direction specifies the direction to which data will be sent
     * using the new connection.
     *
     * \param result specifies a pointer to HPrepareForConnectionResult that
     * will contain the result of the action.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HConnectionManagerInfo::ErrorCode
     *
     * \remarks This is an optional action, but if it is implemented, you
     * \b have \b to implement connectionComplete() too. This is mandated by
     * the ConnectionManager:2 specification.
     */
    virtual qint32 prepareForConnection(
        const HProtocolInfo& remoteProtocolInfo,
        const HConnectionManagerId& peerConnectionManager,
        qint32 peerConnectionId,
        HConnectionManagerInfo::Direction direction,
        HPrepareForConnectionResult* result);

    /*!
     * \brief Informs the device that a previously \e prepared connection is no longer
     * needed.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HConnectionManagerInfo::ErrorCode
     *
     * \remarks This is an optional action, but if it is implemented, you
     * \b have \b to implement prepareForConnection() too. This is mandated by
     * the ConnectionManager:2 specification.
     */
    virtual qint32 connectionComplete(qint32 connectionId);

    /*!
     * \brief Returns the IDs of currently active connections.
     *
     * \param outArg specifies a pointer to a list of connection IDs that represent
     * the currently active connections.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     */
    virtual qint32 getCurrentConnectionIDs(QList<quint32>* outArg) = 0;

    /*!
     * \brief Returns information of the specified connection.
     *
     * \param connectionId specifies the connection.
     *
     * \param outArg specifies a pointer to HConnectionInfo that will contain
     * information about the specified connection.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HConnectionManagerInfo::ErrorCode
     */
    virtual qint32 getCurrentConnectionInfo(
        qint32 connectionId, HConnectionInfo* outArg) = 0;
};

}
}
}

#endif /* HABSTRACTCONNECTIONMANAGER_SERVICE_H_ */
