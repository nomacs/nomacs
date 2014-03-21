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

#ifndef HCONNECTIONMANAGER_INFO_H_
#define HCONNECTIONMANAGER_INFO_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HConnectionManagerInfoPrivate;

/*!
 * \brief This class provides general information of a UPnP A/V ConnectionManager and
 * information related to the operations of ConnectionManager
 * implementations provided by HUPnPAv.
 *
 * \headerfile hconnectionmanager.h HConnectionManager
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is thread-safe.
 */
class H_UPNP_AV_EXPORT HConnectionManagerInfo
{
H_FORCE_SINGLETON(HConnectionManagerInfo)

public:

    /*!
     * \brief This enumeration is used to describe the \e direction of a connection.
     *
     * The direction of a connection defines whether the connection is setup
     * for sending or receiving data.
     *
     * \sa directionFromString(), directionToString()
     */
    enum Direction
    {
        /*!
         * This is used in error scenarios.
         */
        DirectionUndefined,

        /*!
         * The connection is used for receiving data.
         */
        DirectionInput,

        /*!
         * The connection is used for sending data.
         */
        DirectionOutput
    };

    /*!
     * \brief Returns a HConnectionManagerInfo::Direction value corresponding to the
     * specified string, if any.
     *
     * \param arg specifies the string.
     *
     * \return The corresponding HConnectionManagerInfo::Direction value.
     */
    static Direction directionFromString(const QString& arg);

    /*!
     * \brief Returns a string representation of the specified \a value.
     *
     * \param value specifies the HConnectionManagerInfo::Direction value.
     *
     * \return a string representation of the specified
     * HConnectionManagerInfo::Direction value.
     */
    static QString directionToString(Direction value);

    /*!
     * \brief This enumeration describes the status of a connection.
     *
     * \sa statusFromString(), statusToString()
     */
    enum ConnectionStatus
    {
        /*!
         * The status of the connection cannot be determined.
         */
        StatusUnknown,

        /*!
         * The connection appears to be functioning normally.
         */
        StatusOk,

        /*!
         * Connection is set up, but the last transfer operation failed due to
         * an attempt to transfer unsupported content type.
         */
        StatusContentFormatMismatch,

        /*!
         * Connection is set up, but it cannot function due to the lack of
         * bandwidth.
         */
        StatusInsufficientBandwidth,

        /*!
         * Connection is set up, but the channel used to transfer content cannot
         * guarantee a successful transfer.
         */
        StatusUnreliableChannel
    };

    /*!
     * \brief Returns a HConnectionManagerInfo::ConnectionStatus value corresponding
     * to the specified string, if any.
     *
     * \param arg specifies the string.
     *
     * \return The corresponding HConnectionManagerInfo::ConnectionStatus value.
     */
    static ConnectionStatus statusFromString(const QString& arg);

    /*!
     * \brief Returns a string representation of the specified \a value.
     *
     * \param value specifies the HConnectionManagerInfo::ConnectionStatus value.
     *
     * \return a string representation of the specified
     * HConnectionManagerInfo::ConnectionStatus value.
     */
    static QString statusToString(ConnectionStatus arg);

    /*!
     * \brief This enumeration describes the error codes that have been
     * defined by the UPnP forum to the ConnectionManager:2 service.
     *
     * \note the descriptions are taken from the ConnectionManager:2 specification.
     */
    enum ErrorCode
    {
        /*!
         * The connection cannot be established because the protocol info
         * argument is incompatible.
         */
        IncompatibleProtocolInfo = 701,

        /*!
         * The connection cannot be established because the directions of the
         * involved ConnectionManagers (source and sink) are incompatible.
         */
        IncompatibleDirections = 702,

        /*!
         * The connection cannot be established because there are
         * insufficient network resources, such as bandwidth, channels, etc.
         */
        InsufficientNetworkResources = 703,

        /*!
         * The connection cannot be established because of local restrictions
         * in the device.
         *
         * This might happen, for example, when physical
         * resources on the device are already in use by other connections.
         */
        LocalRestrictions = 704,

        /*!
         * The connection cannot be established because the client is not
         * permitted to access the specified ConnectionManager.
         */
        AccessDenied = 705,

        /*!
         * The connection reference argument does not refer to a valid
         * connection established by this service.
         */
        InvalidConnectionReference = 706,

        /*!
         * The connection cannot be established because the
         * ConnectionManagers are not part of the same physical network.
         */
        NotInNetwork = 707,

        /*!
         * The connection cannot be established because the specified
         * ConnectionManager has instantiated the maximum number of
         * simultaneous connections it has room for in its internal data
         * structures.
         *
         * Closing one connection will resolve the issue.
         */
        ConnectionTableOverflow = 708,

        /*!
         * The connection cannot be established because the device does not
         * have sufficient internal processing resources to handle the new
         * connection.
         *
         * Closing one or more connections on this device may
         * resolve the issue.
         */
        InternalProcessingResourcesExceeded = 709,

        /*!
         * The connection cannot be established because the device does not
         * have sufficient internal memory resources to handle the new
         * connection.
         *
         * Closing one or more connections on this device may
         * resolve the issue.
         */
        InternalMemoryResourcesExceeded = 710,

        /*!
         * The connection cannot be established because the device does not
         * have sufficient internal storage system capabilities to handle the
         * new connection.
         *
         * Closing one or more connections on this device
         * may resolve the issue.
         */
        InternalStorageSystemCapabilitiesExceeded = 711
    };

    /*!
     * \brief Returns the type of the latest ConnectionManager specification the
     * \ref hupnp_av implementations support.
     *
     * \return The type of the latest ConnectionManager specification the
     * \ref hupnp_av implementations support.
     */
    static const HResourceType& supportedServiceType();

    /*!
     * \brief Returns information about the actions specified up until the
     * ConnectionManager specification supported by \ref hupnp_av implementations.
     *
     * \sa supportedServiceType(), stateVariablesSetupData()
     */
    static HActionsSetupData actionsSetupData();

    /*!
     * \brief Returns information about the state variables specified up until the
     * ConnectionManager specification supported by \ref hupnp_av implementations.
     *
     * \sa supportedServiceType(), actionsSetupData()
     */
    static HStateVariablesSetupData stateVariablesSetupData();
};

}
}
}

#endif /* HCONNECTIONMANAGER_INFO_H_ */
