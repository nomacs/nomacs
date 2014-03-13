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

#ifndef HCHANNELID_H_
#define HCHANNELID_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HChannelIdPrivate;

/*!
 * \brief This class is used to represent a unique ID of a program or series.
 *
 * For more information, see ScheduledRecording:2, Appendix B.5.2.
 *
 * \headerfile hmatching_id.h HChannelId
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HChannelId
{
private:

    QSharedDataPointer<HChannelIdPrivate> h_ptr;

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
     * \brief Returns a string representation of the HChannelId::Type value specified.
     *
     * \return a string representation of the HChannelId::Type value specified.
     */
    static QString toString(Type type);

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HChannelId();

    /*!
     * \brief Creates a new instance.
     *
     * \param value specifies the value of the matchingID property.
     *
     * \param type specifies the type of the matchingID property.
     *
     * \sa HChannelId::Type, toString(Type)
     */
    HChannelId(const QString& value, const QString& type);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HChannelId(const HChannelId&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HChannelId& operator=(const HChannelId&);

    /*!
     * \brief Destroys the instance.
     */
    ~HChannelId();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid.
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
 * \relates HChannelId
 */
H_UPNP_AV_EXPORT bool operator==(const HChannelId& obj1, const HChannelId& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HChannelId
 */
inline bool operator!=(const HChannelId& obj1, const HChannelId& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HChannelId)

#endif /* HCHANNELID_H_ */
