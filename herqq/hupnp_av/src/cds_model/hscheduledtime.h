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

#ifndef HSCHEDULEDTIME_H_
#define HSCHEDULEDTIME_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HScheduledTimePrivate;

/*!
 * \brief This class is used to represent scheduledStartTime and scheduledEndTime
 * properties defined and used in the ContentDirectory:3 specification.
 *
 * See ContentDirectory:3, Appendix B.11.3 and B.11.4 for more information.
 *
 * \headerfile hscheduledtime.h HScheduledTime
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HScheduledTime
{
private:

    QSharedDataPointer<HScheduledTimePrivate> h_ptr;

public:

    /*!
     * \brief This enumeration defines the usage types of a \e scheduled \e time
     * element.
     */
    enum Type
    {
        /*!
         * The usage type is not defined.
         */
        Undefined,

        /*!
         * The time value defines the start or end time of a scheduled event.
         */
        ScheduledProgram,

        /*!
         * The time value defines the start of end time window within which
         * on-demand content is available.
         */
        OnDemand
    };

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
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HScheduledTime();

    /*!
     * \brief Creates a new instance.
     *
     * \param value specifies the date time value.
     *
     * \param type specifies the usage type.
     *
     * \sa isValid()
     */
    HScheduledTime(const QDateTime& value, Type type = ScheduledProgram);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HScheduledTime(const HScheduledTime&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HScheduledTime& operator=(const HScheduledTime&);

    /*!
     * \brief Destroys the instance.
     */
    ~HScheduledTime();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. type() and value() are
     * appropriately defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the date time value.
     *
     * \return The date time value.
     *
     * \sa setValue()
     */
    QDateTime value() const;

    /*!
     * \brief Returns the usage type.
     *
     * \return The usage type.
     *
     * \sa setType()
     */
    Type type() const;

    /*!
     * \brief Indicates whether the value() is expressed using either Daylight Saving
     * or Standard Time as a point of reference.
     *
     * \return a value indicating whether the value() is expressed using either
     * Daylight Saving or Standard Time as a point of reference.
     */
    HDaylightSaving daylightSaving() const;

    /*!
     * \brief Specifies the date time value.
     *
     * \param arg specifies the date time value.
     *
     * \sa value()
     */
    void setValue(const QDateTime& arg);

    /*!
     * \brief Specifies the usage type.
     *
     * \param arg specifies the usage type.
     *
     * \sa type()
     */
    void setType(Type arg);

    /*!
     * \brief Specifies whether the value() is expressed using either Daylight Saving
     * or Standard Time as a point of reference.
     *
     * \param arg specifies whether the value() is expressed using either Daylight Saving
     * or Standard Time as a point of reference.
     *
     * \sa daylightSaving()
     */
    void setDaylightSaving(HDaylightSaving arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HScheduledTime
 */
H_UPNP_AV_EXPORT bool operator==(const HScheduledTime& obj1, const HScheduledTime& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HScheduledTime
 */
inline bool operator!=(const HScheduledTime& obj1, const HScheduledTime& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HScheduledTime)

#endif /* HSCHEDULEDTIME_H_ */
