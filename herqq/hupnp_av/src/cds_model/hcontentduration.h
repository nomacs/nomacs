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

#ifndef HCONTENTDURATION_H_
#define HCONTENTDURATION_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

class QTime;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HContentDurationPrivate;

/*!
 * \brief This class is used to represent duration of media content as specified
 * in the ContentDirectory:3 specification.
 *
 * \headerfile hcontentduration.h HContentDuration
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HContentDuration
{
private:

    QSharedDataPointer<HContentDurationPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     */
    HContentDuration();

    /*!
     * \brief Creates a new instance.
     *
     * \param arg specifies the duration. The expected format is:
     * <c>'P' [n 'D'] time</c>, where n is an integer, P and D are constant
     * characters and time is a ISO 8601 compatible value for time.
     * The number of days component is optional.
     *
     * \sa isValid()
     */
    HContentDuration(const QString& arg);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HContentDuration(const HContentDuration&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HContentDuration& operator=(const HContentDuration&);

    /*!
     * \brief Destroys the instance.
     */
    ~HContentDuration();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e if the object is valid.
     */
    bool isValid() const;

    /*!
     * \brief Returns the number of days of the duration.
     *
     * \return The number of days of the duration.
     */
    qint32 days() const;

    /*!
     * \brief Returns the time component of the duration.
     *
     * \return The time component of the duration.
     */
    QTime time() const;

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    QString toString() const;

    /*!
     * \brief Specifies the number of days of the duration.
     *
     * \param arg the number of days of the duration.
     */
    void setDays(qint32 arg);

    /*!
     * \brief Specifies the time component of the duration.
     *
     * \param arg specifies the time component of the duration.
     */
    void setTime(const QTime& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HContentDuration
 */
H_UPNP_AV_EXPORT bool operator==(const HContentDuration& obj1, const HContentDuration& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HSoHContentDurationrtInfo
 */
inline bool operator!=(const HContentDuration& obj1, const HContentDuration& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HContentDuration)

#endif /* HCONTENTDURATION_H_ */
