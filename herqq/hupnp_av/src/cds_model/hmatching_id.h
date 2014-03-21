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

#ifndef HMATCHINGID_H_
#define HMATCHINGID_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMatchingIdPrivate;

/*!
 * \brief This class is used to represent a unique ID of a program or series.
 *
 * For more information, see ScheduledRecording:2, Appendix B.5.2.
 *
 * \headerfile hmatching_id.h HMatchingId
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMatchingId
{
private:

    QSharedDataPointer<HMatchingIdPrivate> h_ptr;

public:

    /*!
     * \brief This enumeration specifies the MatchingID values defined
     * in the ScheduledRecording:2 specification.
     */
    enum Type
    {
        /*!
         * The value is not defined.
         */
        Undefined,

        /*!
         * The ID value identifies a series and it follows the format defined
         * for series identification.
         */
        SeriesId,

        /*!
         * The ID value identifies a program and it follows the format defined
         * for program identification.
         */
        ProgramId,

        /*!
         * The ID is defined by a vendor.
         */
        VendorDefined
    };

    /*!
     * \brief Returns a string representation of the HMatchingId::Type value specified.
     *
     * \return a string representation of the HMatchingId::Type value specified.
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
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HMatchingId();

    /*!
     * \brief Creates a new instance.
     *
     * \param value specifies the value of the matchingID property.
     *
     * \param type specifies the type of the matchingID property. Note, if the
     * type is \e VendorDefined the type should be formatted as
     * <c><ICANN NAME>_<Identifier>. For example, <c>mydomain.org_myIdentifier</c>
     *
     * \sa HMatchingId::Type, toString(Type)
     */
    HMatchingId(const QString& value, const QString& type);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HMatchingId(const HMatchingId&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HMatchingId& operator=(const HMatchingId&);

    /*!
     * \brief Destroys the instance.
     */
    ~HMatchingId();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. both the value() and type()
     * are appropriately defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the type value.
     *
     * \return The type value.
     */
    Type type() const;

    /*!
     * \brief Returns a string representation of the type value.
     *
     * \return a string representation of the type value.
     */
    QString typeAsString() const;

    /*!
     * \brief Returns the value of the matchingID property.
     *
     * \return The value of the matchingID property.
     */
    QString value() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HMatchingId
 */
H_UPNP_AV_EXPORT bool operator==(const HMatchingId& obj1, const HMatchingId& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HMatchingId
 */
inline bool operator!=(const HMatchingId& obj1, const HMatchingId& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HMatchingId)

#endif /* HMATCHINGID_H_ */
