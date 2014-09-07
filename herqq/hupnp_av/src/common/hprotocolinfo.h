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

#ifndef HPROTOCOLINFO_H_
#define HPROTOCOLINFO_H_

#include <HUpnpAv/HUpnpAv>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HProtocolInfoPrivate;

/*!
 * \brief This class represents the \e ProtocolInfo concept defined in the
 * UPnP A/V ConnectionManager Service specification.
 *
 * The aforementioned specification defines the \e ProtocolInfo concept as:
 * <em>information needed by a control point in order to determine
 * (a certain level of) compatibility between the streaming mechanisms of two
 * UPnP controlled devices. For example, it contains the transport protocols
 * supported by a device, for input or output, as well as other information
 * such as the content formats (encodings) that can be sent, or received,
 * via the transport protocols.
 * </em>
 *
 * For more information, see the section 2.5.2 of UPnP A/V ConnectionManager v2
 * specification.
 *
 * \headerfile hprotocolinfo.h HProtocolInfo
 *
 * \ingroup hupnp_av_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HProtocolInfo
{

private:

    QSharedDataPointer<HProtocolInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HProtocolInfo();

    /*!
     * Constructs a new instance.
     *
     * \param contents contains the elements of a \e ProtocolInfo formatted as
     * <protocol>:<network>:<contentFormat>:<additionalInfo>. Note, the
     * inequality signs are used to clearly separate the elements of a
     * ProtocolInfo. They cannot be used as part of the specified string. Further,
     * the object will be constructed empty, if the specified string does not
     * follow the specified format.
     *
     * \remarks Each of the elements can be a wild-card "*".
     *
     * \sa isValid()
     */
    HProtocolInfo(const QString& contents);

    /*!
     * Constructs a new instance.
     *
     * \param protocol specifies the transport protocol.
     *
     * \param network specifies the network element.
     *
     * \param contentFormat specifies the content formats that can be sent or
     * received via the transport protocol.
     *
     * \param additionalInfo specifies additional information that is needed
     * to set up the transfer stream. See the ConnectionManager specification
     * for more information.
     *
     * \remarks Each of the elements can be a wild-card "*" and none of the
     * elements can contain the character ":", as that is used as a separator.
     *
     * \sa isValid()()
     */
    HProtocolInfo(
        const QString& protocol, const QString& network,
        const QString& contentFormat, const QString& additionalInfo);

    /*!
     * \brief Destroys the instance.
     */
    ~HProtocolInfo();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HProtocolInfo(const HProtocolInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HProtocolInfo& operator=(const HProtocolInfo&);

    /*!
     * \brief Returns the transport protocol.
     *
     * \return The transport protocol.
     *
     * \sa setProtocol()
     */
    QString protocol() const;

    /*!
     * \brief Returns the network element.
     *
     * \return The network element.
     *
     * \sa setNetwork()
     */
    QString network() const;

    /*!
     * \brief Returns the content formats that can be sent or
     * received via the transport protocol.
     *
     * \return The content formats that can be sent or
     * received via the transport protocol.
     *
     * \sa setContentFormat()
     */
    QString contentFormat() const;

    /*!
     * \brief Returns the additional information element.
     *
     * \return The additional information element.
     *
     * \sa setAdditionalInfo()
     */
    QString additionalInfo() const;

    /*!
     * \brief Sets the transport protocol.
     *
     * \param arg specifies the transport protocol.
     *
     * \sa protocol()
     */
    void setProtocol(const QString& arg);

    /*!
     * \brief Sets the network element.
     *
     * \param arg specifies the network element.
     *
     * \sa network()
     */
    void setNetwork(const QString& arg);

    /*!
     * \brief Sets the content formats that can be sent or
     * received via the transport protocol.
     *
     * \param arg specifies the content formats that can be sent or
     * received via the transport protocol.
     *
     * \sa contentFormat()
     */
    void setContentFormat(const QString& arg);

    /*!
     * \brief Sets the additional information element.
     *
     * \param arg specifies the additional information.
     *
     * \sa additionalInfo()
     */
    void setAdditionalInfo(const QString& arg);

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true if the object is empty, i.e. no content is defined.
     *
     * \sa isValid()
     */
    bool isEmpty() const;

    /*!
     * Indicates if the object represents a valid \e ProtocolInformation entity.
     *
     * \return \e true if the object represents a valid \e ProtocolInformation entity.
     *
     * \sa isEmpty()
     */
    bool isValid() const;

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return A string representation of the object. The format
     * is defined in UPnP A/V ConnectionManager Service specification as:
     * <em><protocol>:<network>:<contentFormat>:<additionalInfo></em>. The returned
     * string is empty if the object is invalid.
     *
     * \sa isValid()
     */
    QString toString() const;

    /*!
     * Creates a new instance with every field set to the wildcard value.
     *
     * \return a new instance with every field set to the wildcard (*) value.
     */
    static HProtocolInfo createUsingWildcards();
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HProtocolInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HProtocolInfo&, const HProtocolInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HProtocolInfo
 */
inline bool operator!=(const HProtocolInfo& obj1, const HProtocolInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HPROTOCOLINFO_H_ */
