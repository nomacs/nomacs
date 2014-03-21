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

#ifndef HCHANNEL_H_
#define HCHANNEL_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

template<typename T>
class QSet;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with channel types defined in the
 * RenderingControl:2 specification.
 *
 * \headerfile hchannel.h HChannel
 *
 * \ingroup hupnp_av_rcs
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HChannel
{
public:

    /*!
     * \brief This enumeration specifies the Channel types defined in the
     * RenderingControl:2 specification.
     */
    enum Type
    {
        /*!
         * The channel is not defined.
         */
        Undefined = 0,

        /*!
         * Master.
         */
        Master,

        /*!
         * Left front.
         */
        LeftFront,

        /*!
         * Right front.
         */
        RightFront,

        /*!
         * Center front.
         */
        CenterFront,

        /*!
         * Low frequency enhancement.
         */
        LFE,

        /*!
         * Left surround.
         */
        LeftSurround,

        /*!
         * Right surround.
         */
        RightSurround,

        /*!
         * Left of center.
         */
        LeftOfCenter,

        /*!
         * Right of center.
         */
        RightOfCenter,

        /*!
         * Rear surround.
         */
        Surround,

        /*!
         * Side left.
         */
        SideLeft,

        /*!
         * Side right.
         */
        SideRight,

        /*!
         * Top, overhead.
         */
        Top,

        /*!
         * Bottom
         */
        Bottom,

        /*!
         * Vendor defined.
         */
        VendorDefined
    };

    Type m_type;
    QString m_typeAsString;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HChannel();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the channel type.
     *
     * \sa isValid()
     */
    HChannel(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the channel type. If the specified string does not
     * correspond to any Type value and the string is not empty, the type() is set to
     * HChannel::VendorDefined.
     *
     * \sa isValid()
     */
    HChannel(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HChannel::Undefined.
     */
    inline bool isValid() const { return m_type != Undefined; }

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    inline QString toString() const { return m_typeAsString; }

    /*!
     * \brief Returns the type value.
     *
     * \return The type value.
     */
    inline Type type() const { return m_type; }

    /*!
     * Converts the specified Type value to string.
     *
     * \param type specifies the Type value to be converted to string.
     *
     * \return a string representation of the specified Type value.
     */
    static QString toString(Type type);

    /*!
     * \brief Returns a Type value corresponding to the specified string, if any.
     *
     * \param type specifies the Type as string.
     *
     * \return a Type value corresponding to the specified string, if any.
     */
    static Type fromString(const QString& type);

    /*!
     * Returns a set containing every standard audio channel identifier.
     *
     * \return a set containing every standard audio channel identifier.
     */
    static QSet<HChannel> allChannels();
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HChannel
 */
H_UPNP_AV_EXPORT bool operator==(const HChannel& obj1, const HChannel& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HChannel
 */
inline bool operator!=(const HChannel& obj1, const HChannel& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \param key specifies the HChannel object from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 */
H_UPNP_AV_EXPORT quint32 qHash(const HChannel& key);

}
}
}

#endif /* HCHANNEL_H_ */
