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

#ifndef HRECORDMEDIUMWRITESTATUS_H_
#define HRECORDMEDIUMWRITESTATUS_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with Record Medium Write Status values
 * defined in the AVTransport:2 specification.
 *
 * \headerfile hrecordmediumwritestatus.h HRecordMediumWriteStatus
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HRecordMediumWriteStatus
{

public:

    /*!
     * \brief This enumeration specifies the Record Medium Write Status values defined
     * in the AVTransport:2 specification.
     */
    enum Type
    {
        /*!
         * No media is loaded.
         */
        Unknown = 0,

        /*!
         * The media is writable.
         */
        Writable,

        /*!
         * Recording isn't supported on the current media.
         */
        Protected,

        /*!
         * Read-only media.
         */
        NotWritable,

        /*!
         * Implementation does not support recording.
         */
        NotImplemented,

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
    HRecordMediumWriteStatus();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the record medium write status.
     *
     * \sa isValid()
     */
    HRecordMediumWriteStatus(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the record medium write status. If the specified string does not
     * correspond to any Type value and the string is not empty, the type() is set to
     * HRecordMediumWriteStatus::VendorDefined.
     *
     * \sa isValid()
     */
    HRecordMediumWriteStatus(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HRecordMediumWriteStatus::Unknown.
     */
    inline bool isValid() const { return m_type != Unknown; }

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
 * \relates HRecordMediumWriteStatus
 */
H_UPNP_AV_EXPORT bool operator==(const HRecordMediumWriteStatus& obj1, const HRecordMediumWriteStatus& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HRecordMediumWriteStatus
 */
inline bool operator!=(const HRecordMediumWriteStatus& obj1, const HRecordMediumWriteStatus& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HRECORDMEDIUMWRITESTATUS_H_ */
