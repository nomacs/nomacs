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

#ifndef HPLAYMODE_H_
#define HPLAYMODE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with Play Modes defined in the
 * AVTransport:2 specification.
 *
 * \headerfile hplaymode.h HPlayMode
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HPlayMode
{
public:

    /*!
     * \brief This enumeration specifies the Play Mode values defined in the
     * AVTransport:2 specification.
     *
     * \sa toString(), fromString()
     */
    enum Type
    {
        /*!
         * The play mode is not known.
         */
        Undefined,

        /*!
         * Play all the tracks on the media in the order they are on the media.
         */
        Normal,

        /*!
         * Shuffle all the tracks on the media and then play all the tracks
         * in that order.
         */
        Shuffle,

        /*!
         * Play the current track on the media repeatedly and indefinitely.
         */
        RepeatOne,

        /*!
         * Play all tracks on the media repeatedly and indefinitely.
         */
        RepeatAll,

        /*!
         * Continuously choose and play a random track on the media.
         */
        Random,

        /*!
         * Play a single track and stop.
         */
        Direct_1,

        /*!
         * Play a short sample of each track on the media.
         */
        Intro,

        /*!
         * Vendor defined.
         */
        VendorDefined
    };

    Type m_type;
    QString m_typeAsString;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HPlayMode();

    /*!
     * \brief Creates a new instance.
     *
     * \param mode specifies the Play Mode value.
     *
     * \sa isValid()
     */
    HPlayMode(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param type specifies the play mode type. If it isn't one of the types
     * defined by Type and it isn't empty, the type() is set to
     * HPlayMode::VendorDefined.
     *
     * \sa isValid()
     */
    HPlayMode(const QString& type);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HPlayMode::Undefined.
     */
    inline bool isValid() const { return m_type != Undefined; }

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    inline QString toString() const { return m_typeAsString; }

    /*!
     * \brief Returns the Type value.
     *
     * \return The Type value.
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
 * \relates HPlayMode
 */
H_UPNP_AV_EXPORT bool operator==(const HPlayMode& obj1, const HPlayMode& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HPlayMode
 */
inline bool operator!=(const HPlayMode& obj1, const HPlayMode& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HPLAYMODE_H_ */
