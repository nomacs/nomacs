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

#ifndef HAV_GLOBAL_H_
#define HAV_GLOBAL_H_

#include <HUpnpAv/public/hav_defs.h>
#include <HUpnpAv/public/hav_fwd.h>

#include <QtCore/QMetaType>
#include <QtCore/QXmlStreamReader>

class QString;

/*!
 * \file
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This enumeration defines the week days.
 *
 * \ingroup hupnp_av
 */
enum HDayOfWeek
{
    /*!
     * The week day is not defined.
     */
    Undefined_DayOfWeek = 0,

    /*!
     * Monday.
     */
    Monday,

    /*!
     * Tuesday.
     */
    Tuesday,

    /*!
     * Wednesday.
     */
    Wednesday,

    /*!
     * Thursday.
     */
    Thursday,

    /*!
     * Friday.
     */
    Friday,

    /*!
     * Saturday.
     */
    Saturday,

    /*!
     * Sunday.
     */
    Sunday
};

/*!
 * \brief This enumeration defines the formats for week days.
 *
 * \ingroup hupnp_av
 */
enum HDayOfWeekFormat
{
    /*!
     * Full name of the day is used.
     */
    Full,

    /*!
     * Three first letters of the name of the day is used.
     */
    ThreeLetter
};

/*!
 * \brief Converts the specified HDayOfWeek value to string.
 *
 * \param type specifies the HDayOfWeek value to be converted to string.
 *
 * \param format specifies how the format of the return value.
 *
 * \return a string representation of the specified HDayOfWeek value.
 */
H_UPNP_AV_EXPORT QString toString(HDayOfWeek type, HDayOfWeekFormat format=Full);

/*!
 * \brief Returns a HDayOfWeek value corresponding to the specified string, if any.
 *
 * \param arg specifies the HDayOfWeek as string.
 *
 * \return a HDayOfWeek value corresponding to the specified string, if any.
 */
H_UPNP_AV_EXPORT HDayOfWeek dayOfWeekFromString(const QString& arg);

/*!
 * \brief This enumeration type defines the episode type of a recording.
 */
enum HEpisodeType
{
    /*!
     * The episode type is not defined.
     */
    EpisodeTypeUndefined,

    /*!
     * All programs are recorded.
     */
    EpisodeTypeAll,

    /*!
     * Only programs that have an original air date
     * equal to the current date are recorded.
     */
    EpisodeTypeFirstRun,

    /*!
     * Only programs that have an original air date
     * earlier than the current date are recorded.
     */
    EpisodeTypeRepeat
};

/*!
 * Converts the specified HEpisodeType value to string.
 *
 * \param type specifies the HEpisodeType value to be converted to string.
 *
 * \return a string representation of the specified HEpisodeType value.
 */
H_UPNP_AV_EXPORT QString toString(HEpisodeType type);

/*!
 * \brief Returns a HEpisodeType value corresponding to the specified string, if any.
 *
 * \param arg specifies the HEpisodeType as string.
 *
 * \return a HEpisodeType value corresponding to the specified string, if any.
 */
H_UPNP_AV_EXPORT HEpisodeType episodeTypeFromString(const QString& arg);

/*!
 * \brief This enumeration defines the Daylight Saving values used and defined
 * in the ContentDirectory:3 specification.
 *
 * See ContentDirectory:3, Appendix B.2.1.21.1 for more information.
 */
enum HDaylightSaving
{
    /*!
     * The reference point for the associated local time
     * value depends on whether Daylight Saving Time
     * is in effect or not. During the time interval
     * starting one hour before the switch is made from
     * Daylight Saving Time back to Standard time and
     * ending one hour after that switching point
     * however, the reference point is ambiguous and is
     * device dependent.
     */
    Unknown_DaylightSaving,

    /*!
     * The reference point for the associated local time
     * value is Daylight Saving Time, even if the
     * indicated time falls outside the period of the year
     * when Daylight Saving Time is actually observed.
     */
    DaylightSaving,

    /*!
     * The reference point for the associated local time
     * value is Standard Time, even if the indicated time
     * falls outside the period of the year when Standard
     * Time is actually observed.
     */
    Standard_DaylightSaving
};

/*!
 * Converts the specified HDaylightSaving value to string.
 *
 * \param type specifies the HDaylightSaving value to be converted to string.
 *
 * \return a string representation of the specified HDaylightSaving value.
 */
H_UPNP_AV_EXPORT QString toString(HDaylightSaving type);

/*!
 * \brief Returns a HDaylightSaving value corresponding to the specified string, if any.
 *
 * \param arg specifies the HDaylightSaving as string.
 *
 * \return a HDaylightSaving value corresponding to the specified string, if any.
 */
H_UPNP_AV_EXPORT HDaylightSaving daylightSavingFromString(const QString& arg);

/*!
 * Parses the specified XML and returns an object representing its contents.
 *
 * \param arg specifies an XML document following
 * <a href="http://www.upnp.org/schemas/av/avs-v2.xsd">this schema</a>.
 *
 * \return an object representing the XML content.
 */
H_UPNP_AV_EXPORT HStateVariableCollection parseStateVariableCollection(const QString& arg);

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HDayOfWeek)
Q_DECLARE_METATYPE(Herqq::Upnp::Av::HEpisodeType)
Q_DECLARE_METATYPE(Herqq::Upnp::Av::HDaylightSaving)

#endif /* HAV_GLOBAL_H_ */
