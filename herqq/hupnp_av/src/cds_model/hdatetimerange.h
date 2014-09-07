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

#ifndef HDATETIMERANGE_H_
#define HDATETIMERANGE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

class QDateTime;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HDateTimeRangePrivate;

/*!
 * \brief This class is used to specify a time range.
 *
 * \headerfile hdatetimerange.h HDateTimeRange
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HDateTimeRange
{
private:

    QSharedDataPointer<HDateTimeRangePrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HDateTimeRange();

    /*!
     * \brief Creates a new instance.
     *
     * \param value specifies the date time range. The expected format is:
     * <c>start datetime value '/' end datetime value</c>. The datetime values
     * are expected to follow ISO 8601.
     *
     * \param dlSaving specifies the daylight saving reference point.
     *
     * \sa isValid()
     */
    HDateTimeRange(const QString& value, HDaylightSaving dlSaving = Unknown_DaylightSaving);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HDateTimeRange(const HDateTimeRange&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HDateTimeRange& operator=(const HDateTimeRange&);

    /*!
     * \brief Destroys the instance.
     */
    ~HDateTimeRange();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true when the object is valid, i.e. the startTime() and endTime()
     * are defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the starting date time value.
     *
     * \return The starting date time value.
     *
     * \sa setStartTime()
     */
    QDateTime startTime() const;

    /*!
     * \brief Returns the ending date time value.
     *
     * \return The ending date time value.
     *
     * \sa setEndTime()
     */
    QDateTime endTime() const;

    /*!
     * \brief Indicates whether the range is expressed using either Daylight Saving
     * or Standard Time as a point of reference.
     *
     * \return a value indicating whether the range is expressed using either
     * Daylight Saving or Standard Time as a point of reference.
     *
     * \sa setDaylightSaving()
     */
    HDaylightSaving daylightSaving() const;

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    QString toString() const;

    /*!
     * \brief Specifies the starting date time value.
     *
     * \param value specifies the starting date time value.
     *
     * \sa startTime()
     */
    void setStartTime(const QDateTime& value);

    /*!
     * \brief Specifies the ending date time value.
     *
     * \param value specifies the ending date time value.
     *
     * \sa endTime()
     */
    void setEndTime(const QDateTime& value);

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
 * \relates HDateTimeRange
 */
H_UPNP_AV_EXPORT bool operator==(const HDateTimeRange& obj1, const HDateTimeRange& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HDateTimeRange
 */
inline bool operator!=(const HDateTimeRange& obj1, const HDateTimeRange& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HDateTimeRange)

#endif /* HDATETIMERANGE_H_ */
