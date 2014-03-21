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

#ifndef HRADIOBAND_H_
#define HRADIOBAND_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>
#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with radio bands defined in the
 * ContentDirectory:3 specification.
 *
 * \headerfile hradioband.h HRadioBand
 *
 * \ingroup hupnp_av_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HRadioBand
{

public:

    /*!
     * \brief This enumeration specifies the radio band types defined in the
     * ContentDirectory:2 specification.
     */
    enum Type
    {
        /*!
         * The radio band type is not defined.
         */
        Undefined = 0,

        /*!
         * AM.
         */
        AM,

        /*!
         * FM
         */
        FM,

        /*!
         * Short wave.
         */
        ShortWave,

        /*!
         * Internet.
         */
        Internet,

        /*!
         *  Satellite.
         */
        Satellite,

        /*!
         * Vendor-defined.
         */
        VendorDefined
    };

    QString m_typeAsString;
    Type m_type;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HRadioBand();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the radio band type.
     *
     * \sa isValid()
     */
    HRadioBand(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the radio band. If it isn't one of the types
     * defined by Type and it isn't empty, the type() is set to
     * HRadioBand::VendorDefined.
     *
     * \sa isValid()
     */
    HRadioBand(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HRadioBand::Undefined.
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
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HRadioBand
 */
H_UPNP_AV_EXPORT bool operator==(const HRadioBand& obj1, const HRadioBand& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HRadioBand
 */
inline bool operator!=(const HRadioBand& obj1, const HRadioBand& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HRadioBand)

#endif /* HRADIOBAND_H_ */
