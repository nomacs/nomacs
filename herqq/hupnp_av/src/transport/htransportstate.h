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

#ifndef HTRANSPORTSTATE_H_
#define HTRANSPORTSTATE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with Transport State values
 * defined in the AVTransport:2 specification.
 *
 * \headerfile htransportstate.h HTransportState
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HTransportState
{

public:

    /*!
     * \brief This enumeration specifies the Transport State values defined
     * in the AVTransport:2 specification.
     */
    enum Type
    {
        /*!
         * Transport state is not defined.
         *
         * This value is used in error situations.
         */
        Undefined,

        /*!
         * There is no media configured.
         */
        NoMediaPresent,

        /*!
         * The current operation on the current media is stopped.
         *
         * Note, this is different from PausedPlayback and PausedRecording,
         * in which both the transport is already prepared for playback or
         * recording and because of that the device may respond faster.
         */
        Stopped,

        /*!
         * The current media is playing.
         */
        Playing,

        /*!
         * The state of the device is being changed or a seek operation
         * is in progress.
         */
        Transitioning,

        /*!
         * Playback has been paused.
         */
        PausedPlayback,

        /*!
         * Recording has been paused.
         */
        PausedRecording,

        /*!
         * The device is recording on the current media.
         */
        Recording,

        /*!
         * Vendor-defined.
         *
         * \remarks
         * Control points that do not know the vendor-defined state should
         * not try to change the state of the device until it is again in a
         * state they know.
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
    HTransportState();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the transport state.
     *
     * \sa isValid()
     */
    HTransportState(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the transport state. If the specified string does not
     * correspond to any Type value and the string is not empty, the type() is set to
     * HTransportState::VendorDefined.
     *
     * \sa isValid()
     */
    HTransportState(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HTransportState::Undefined.
     */
    inline bool isValid() const { return m_type != Undefined; }

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
 * \relates HTransportState
 */
H_UPNP_AV_EXPORT bool operator==(const HTransportState& obj1, const HTransportState& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HTransportState
 */
inline bool operator!=(const HTransportState& obj1, const HTransportState& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HTRANSPORTSTATE_H_ */
