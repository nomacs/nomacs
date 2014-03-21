/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef HENDPOINT_H
#define HENDPOINT_H

#include <HUpnpCore/HUpnp>

#include <QtNetwork/QHostAddress>

class QUrl;

namespace Herqq
{

namespace Upnp
{

/*!
 * Class that represents a network endpoint, which is a combination of
 * a host address and a port number.
 *
 * \headerfile hendpoint.h HEndpoint
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HEndpoint
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HEndpoint&, const HEndpoint&);

friend H_UPNP_CORE_EXPORT quint32 qHash(const HEndpoint&);

private:

    QHostAddress m_hostAddress;
    quint16      m_portNumber;

public:

    /*!
     * Creates a new instance with host address set to \c QHostAddress::Null
     * and port set to "0".
     *
     * \sa isNull()
     */
    HEndpoint();

    /*!
     * Creates a new instance with the specified host address and port set to zero.
     *
     * \param hostAddress specifies the host address.
     *
     * \sa isNull()
     */
    HEndpoint(const QHostAddress& hostAddress);

    /*!
     * Creates a new instance with the specified host address and port.
     *
     * \param hostAddress specifies the host address. If the host address
     * is null the port number is set to zero.
     * \param portNumber specifies the port number.
     *
     * \sa isNull()
     */
    HEndpoint(const QHostAddress& hostAddress, quint16 portNumber);

    /*!
     * Creates a new instance from the specified url.
     *
     * \param url specifies the url from which the endpoint and port information
     * is extracted (if present). If the URL does not contain a valid host information
     * the port number is set to zero.
     *
     * \sa isNull()
     */
    HEndpoint(const QUrl& url);

    /*!
     * Creates a new instance from the specified string. following format
     * "hostAddress:portNumber", where [:portNumber] is optional.
     *
     * \param arg specifies the string following format
     * "hostAddress:portNumber", where [:portNumber] is optional.
     * If the hostAddress is \c QHostAddress::Null the port number is set to
     * zero.
     *
     * \sa isNull()
     */
    HEndpoint(const QString& arg);

    /*!
     * \brief Destroys the instance.
     */
    ~HEndpoint();

    /*!
     * \brief Indicates whether or not the end point is properly defined.
     *
     * \return \e true in case the end point is not defined.
     */
    inline bool isNull() const { return m_hostAddress.isNull(); }

    /*!
     * \brief Returns the host address of the endpoint.
     *
     * \return The host address of the endpoint.
     */
    inline QHostAddress hostAddress() const { return m_hostAddress; }

    /*!
     * \brief Returns the port number of the endpoint.
     *
     * \return The port number of the endpoint.
     */
    inline quint16 portNumber() const { return m_portNumber; }

    /*!
     * \brief Indicates whether or not the end point refers to a multicast address.
     *
     * \return \e true in case the end point refers to a multicast address.
     */
    bool isMulticast() const;

    /*!
     * \brief Returns a string representation of the endpoint.
     *
     * \return The address and port number together separated by a ":". E.g
     * \c "192.168.0.1:80". If the instance is null, i.e. isNull() returns true
     * then an empty string is returned.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HEndpoint
 */
H_UPNP_CORE_EXPORT bool operator==(const HEndpoint&, const HEndpoint&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HEndpoint
 */
inline bool operator!=(const HEndpoint& obj1, const HEndpoint& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \param key specifies the object from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \relates HEndpoint
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HEndpoint& key);

}
}

#endif // HENDPOINT_H
