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

#ifndef HRECORDQUALITYMODE_H_
#define HRECORDQUALITYMODE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with Record Quality Modes defined in the
 * AVTransport:2 specification.
 *
 * \headerfile hrecordqualitymode.h HRecordQualityMode
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HRecordQualityMode
{
public:

    /*!
     * \brief This enumeration specifies the Record Quality Modes defined in the
     * AVTransport:2 specification.
     */
    enum Type
    {
        /*!
         * The record quality mode is not defined.
         */
        Undefined = 0,

        /*!
         * 0:EP.
         */
        Ep,

        /*!
         * 1:LP.
         */
        Lp,

        /*!
         * 2:SP.
         */
        Sp,

        /*!
         * 0:BASIC.
         */
        Basic,

        /*!
         * 1:MEDIUM.
         */
        Medium,

        /*!
         * 2:HIGH.
         */
        High,

        /*!
         * The service implementation does not support recording.
         */
        NotImplemented,

        /*!
         * Vendor-defined.
         */
        VendorDefined
    };

    Type m_mode;
    QString m_modeAsString;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HRecordQualityMode();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the record quality mode.
     *
     * \sa isValid()
     */
    HRecordQualityMode(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the record quality mode. If the specified string does not
     * correspond to any Type value and the string is not empty, the type() is set to
     * HRecordQualityMode::VendorDefined.
     *
     * \sa isValid()
     */
    HRecordQualityMode(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HRecordQualityMode::Undefined.
     */
    inline bool isValid() const { return m_mode != Undefined; }

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    inline QString toString() const { return m_modeAsString; }

    /*!
     * \brief Returns the Type value.
     *
     * \return The Type value.
     */
    inline Type type() const { return m_mode; }

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
 * \relates HRecordQualityMode
 */
H_UPNP_AV_EXPORT bool operator==(const HRecordQualityMode& obj1, const HRecordQualityMode& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HRecordQualityMode
 */
inline bool operator!=(const HRecordQualityMode& obj1, const HRecordQualityMode& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \param key specifies the HRecordQualityMode object from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 */
H_UPNP_AV_EXPORT quint32 qHash(const HRecordQualityMode& key);

}
}
}

#endif /* HRECORDQUALITYMODE_H_ */
