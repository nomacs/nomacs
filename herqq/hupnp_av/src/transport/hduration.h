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

#ifndef HDURATION_H_
#define HDURATION_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HDurationPrivate;

/*!
 * \brief This is a class representing the duration of a media content.
 *
 * \headerfile htime.h HDuration
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HDuration
{
friend H_UPNP_AV_EXPORT bool operator==(const HDuration& obj1, const HDuration& obj2);
private:

    QSharedDataPointer<HDurationPrivate> h_ptr;

public:

    /*!
     * Creates a new object.
     *
     * Every time component is set to 0, toString() returns:
     * \c "00:00:00" and isZero() returns \e true.
     */
    HDuration();

    /*!
     * Creates a new object based on the specified string.
     *
     * \param arg specifies the contents of the object in a string.
     * The format is defined in AVTransport:2 and it is:
     * <c>H+:MM:SS[.F+]</c> or <c>H+:MM:SS[.F0/F1]</c>, where:
     *
     * \li \c H+ means one or more digits to indicate elapsed hours
     * \li \c MM means exactly 2 digits to indicate minutes (00 to 59)
     * \li \c SS means exactly 2 digits to indicate seconds (00 to 59)
     * \li <c>[.F+]</c> means OPTIONALLY a dot followed by one or more digits to
     * indicate fractions of seconds
     * \li <c>[.F0/F1]</c> means OPTIONALLY a dot followed by a fraction,
     * with F0 and F1 at least one digit long, and F0 < F1.
     */
    HDuration(const QString& arg);

    /*!
     * \brief Destroys the instance.
     */
    ~HDuration();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HDuration(const HDuration&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HDuration& operator=(const HDuration&);

    /*!
     * Creates a new instance from QTime object.
     */
    HDuration(const QTime& time);

    /*!
     * \brief Returns the hours.
     *
     * \return The hours component of the total duration.
     */
    qint32 hours() const;

    /*!
     * \brief Returns the minutes.
     *
     * \return The minutes component of the total duration.
     */
    qint32 minutes() const;

    /*!
     * \brief Returns the seconds.
     *
     * \return The seconds component of the total duration.
     */
    qint32 seconds() const;

    /*!
     * \brief Returns the fractions of a second.
     *
     * \return The fractions of a second of the total duration.
     */
    qreal fractionsOfSecond() const;

    /*!
     * \brief Indicates if the duration is positive.
     *
     * \return \e true if the duration is positive.
     */
    bool isPositive() const;

    /*!
     * \brief Indicates if the duration is zero.
     *
     * \return \e true if the duration is zero.
     */
    bool isZero() const;

    /*!
     * \brief Returns the object represented as a string.
     *
     * \return The objecet represented as a string. The format is defined in
     * AVTransport:2 and it is:
     * <c>H+:MM:SS[.F+]</c> or <c>H+:MM:SS[.F0/F1]</c>, where:
     *
     * \li \c H+ means one or more digits to indicate elapsed hours
     * \li \c MM means exactly 2 digits to indicate minutes (00 to 59)
     * \li \c SS means exactly 2 digits to indicate seconds (00 to 59)
     * \li <c>[.F+]</c> means OPTIONALLY a dot followed by one or more digits to
     * indicate fractions of seconds
     * \li <c>[.F0/F1]</c> means OPTIONALLY a dot followed by a fraction, with F0 and F1
     * at least one digit long, and F0 < F1.
     */
    QString toString() const;

    /*!
     * \brief Returns the object as a QTime object.
     */
    QTime toTime() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HDuration
 */
H_UPNP_AV_EXPORT bool operator==(const HDuration& obj1, const HDuration& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HDuration
 */
inline bool operator!=(const HDuration& obj1, const HDuration& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HDURATION_H_ */
