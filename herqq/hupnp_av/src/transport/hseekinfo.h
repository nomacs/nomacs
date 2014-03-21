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

#ifndef HSEEKINFO_H_
#define HSEEKINFO_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with Seek Modes defined in the
 * AVTransport:2 specification.
 *
 * \headerfile hseekinfo.h HSeekMode
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HSeekMode
{

public:

    /*!
     * \brief This enumeration specifies the Seek Modes defined in the
     * AVTransport:2 specification.
     *
     * \remarks Many of the descriptions here are more or less direct quotes
     * from the AV Transport:2 specification, section 2.4.13.
     */
    enum Type
    {
        /*!
         * The seek mode is undefined.
         */
        Unknown = -1,

        /*!
         * Track number.
         *
         * Seeks to the beginning of a particular track number.
         *
         * For track-unaware media, special track number "0" is used to indicate
         * the end of the media and thus it is equivalent to the common
         * \e FastReverse VCR functionality. Similarly, track number "1" is
         * equivalent to the common \e FastForward functionality.
         */
        TrackNr = 0,

        /*!
         * Absolute time.
         *
         * The range is from "00:00:00" to the duration of the current media.
         */
        AbsTime,

        /*!
         * Relative time.
         *
         * Seeks within the current track on the current media. The range is
         * from "00:00:00" to the duration of the current track.
         */
        RelTime,

        /*!
         * Absolute count.
         *
         * The range is from 0 to 2147483646, where 0 corresponds to the beginning
         * of the media.
         */
        AbsCount,

        /*!
         * Relative count.
         *
         * Seeks within the current track on the current media. The range is
         * from 0 to the vendor-defined value that corresponds to the end
         * of the current track.
         */
        RelCount,

        /*!
         * Channel frequency.
         *
         * Seeks to the specified channel frequency.
         */
        ChannelFreq,

        /*!
         * Seeks to the pre-defined index at the current media, measured
         * from the beginning of the media.
         *
         * This value applies only track-unaware media and it assumes that the
         * media contains a set of subsequent "marks" that indicate some
         * relevant position on the media.
         *
         * For more information, see the AV Transport:2 specification, section
         * 2.4.13.
         */
        TapeIndex,

        /*!
         * Seeks to the pre-defined index at the current media, measured
         * from the current position on the media.
         *
         * This value applies only track-unaware media and it assumes that the
         * media contains a set of subsequent "marks" that indicate some
         * relevant position on the media.
         *
         * For more information, see the AV Transport:2 specification, section
         * 2.4.13.
         */
        RelTapeIndex,

        /*!
         * Seeks to the desired position, in terms of frames, measured
         * from the beginning of the media. The range of the target value
         * is from "0" to the total duration of the current media, expressed
         * in frames. The value has to be positive.
         */
        Frame,

        /*!
         * For track-aware media, seeks to the desired position in the current
         * track, in terms of frames, measured from the
         * beginning of the current track. The range for the target value
         * is from "0" to the duration of the current track,
         * measured in number of frames. For track-aware media, the target value
         * has to be positive.
         *
         * For track-unaware media (e.g. a single tape), the target value
         * contains the desired position, in terms of frames, measured from a
         * zero reference point on the media. The range for the Target
         * argument is from the beginning of the media, measured from the zero
         * reference point to the end of the media, also measured from the
         * zero reference point. For track-unaware media, the target value
         * can be negative. Indeed, when the zero reference point
         * does not coincide with the beginning of the media, all positions
         * before the zero reference point are expressed as negative
         * values.
         */
        RelFrame,

        /*!
         * Vendor-defined.
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
    HSeekMode();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the seek mode.
     *
     * \sa isValid()
     */
    HSeekMode(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the record quality mode. If the specified string does not
     * correspond to any Type value and the string is not empty, the type() is set to
     * HSeekMode::VendorDefined.
     *
     * \sa isValid()
     */
    HSeekMode(const QString&);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HSeekMode::Undefined.
     */
    inline bool isValid() const { return m_type != Unknown; }

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    QString toString() const;

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
 * \relates HSeekMode
 */
H_UPNP_AV_EXPORT bool operator==(const HSeekMode& obj1, const HSeekMode& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HSeekMode
 */
inline bool operator!=(const HSeekMode& obj1, const HSeekMode& obj2)
{
    return !(obj1 == obj2);
}

class HSeekInfoPrivate;

/*!
 * \brief This class is used to contain information that is required to run a
 * AVTransport seek() operation.
 *
 * \headerfile hseekinfo.h HSeekInfo
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractTransportService::seek(), HAvTransportAdapter::seek()
 */
class H_UPNP_AV_EXPORT HSeekInfo
{

private:

    QSharedDataPointer<HSeekInfoPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HSeekInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param unit specifies how the \a target value should be interpreted.
     *
     * \param target specifies the value that indicates the target position.
     *
     * \sa isValid()
     */
    HSeekInfo(const HSeekMode& unit, const QString& target);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HSeekInfo(const HSeekInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     *
     * \return a reference to this.
     */
    HSeekInfo& operator=(const HSeekInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HSeekInfo();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. unit() is
     * not valid.
     */
    bool isValid() const;

    /*!
     * \brief Returns the seek mode, which indicates how the target() value should be
     * interpreted.
     *
     * \return The seek mode, which indicates how the target() value should be
     * interpreted.
     */
    HSeekMode unit() const;

    /*!
     * \brief Returns the value that indicates the target position.
     *
     * \return The value that indicates the target position.
     */
    QString target() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HSeekInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HSeekInfo& obj1, const HSeekInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HSeekInfo
 */
inline bool operator!=(const HSeekInfo& obj1, const HSeekInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HSEEKINFO_H_ */
