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

#ifndef HSTORAGEMEDIUM_H_
#define HSTORAGEMEDIUM_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>
#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * This is a convenience class for working with storage mediums defined in the
 * AVTransport:2 specification.
 *
 * \headerfile hstoragemedium.h HStorageMedium
 *
 * \ingroup hupnp_av_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HStorageMedium
{

public:

    /*!
     * \brief This enumeration specifies the storage medium types defined in the
     * AVTransport:2 specification.
     */
    enum Type
    {
        /*!
         * Unknown medium.
         */
        Unknown,

        /*!
         * Digital Video Tape medium.
         */
        DigitalVideo,

        /*!
         * Mini Digital Video Tape medium.
         */
        MiniDigitalVideo,

        /*!
         * VHS Tape medium.
         */
        VHS,

        /*!
         * W-VHS Tape medium.
         */
        W_VHS,

        /*!
         * Super VHS Tape medium.
         */
        S_VHS,

        /*!
         * Digital VHS Tape medium.
         */
        D_VHS,

        /*!
         * Compact VHS medium.
         */
        VHSC,

        /*!
         * 8 mm Video Tape medium.
         */
        Video8,

        /*!
         * High resolution 8 mm Video Tape medium.
         */
        HI8,

        /*!
         * Compact Disc-Read Only Memory medium.
         */
        CD_ROM,

        /*!
         * Compact Disc-Digital Audio medium.
         */
        CD_DA,

        /*!
         * Compact Disc-Recordable medium.
         */
        CD_R,

        /*!
         * Compact Disc-Rewritable medium.
         */
        CD_RW,

        /*!
         * Video Compact Disc medium.
         */
        Video_CD,

        /*!
         * Super Audio Compact Disc medium.
         */
        SACD,

        /*!
         * Mini Disc Audio medium.
         */
        MiniDiscAudio,

        /*!
         * Mini Disc Picture medium.
         */
        MiniDiscPicture,

        /*!
         * DVD Read Only medium.
         */
        DVD_ROM,

        /*!
         * DVD Video medium.
         */
        DVD_Video,

        /*!
         * DVD Recordable medium.
         */
        DVD_PlusRecordable,

        /*!
         * DVD Recordable medium.
         */
        DVD_MinusRecordable,

        /*!
         * DVD Rewritable medium.
         */
        DVD_PlusRewritable,

        /*!
         * DVD Rewritable medium.
         */
        DVD_MinusRewritable,

        /*!
         * DVD RAM medium.
         */
        DVD_RAM,

        /*!
         * DVD Audio medium.
         */
        DVD_Audio,

        /*!
         * Digital Audio Tape medium.
         */
        DAT,

        /*!
         * Laser Disk medium.
         */
        LD,

        /*!
         * Hard Disk Drive medium.
         */
        HDD,

        /*!
         * Micro MV Tape medium.
         */
        MicroMV,

        /*!
         * Network Interface medium.
         */
        Network,

        /*!
         * No medium present.
         */
        None,

        /*!
         * Medium type discovery is not implemented.
         */
        NotImplemented,

        /*!
         * SD (Secure Digital) Memory Card medium.
         */
        SecureDigital,

        /*!
         * PC Card medium.
         */
        PC_Card,

        /*!
         * MultimediaCard medium.
         */
        MultimediaCard,

        /*!
         * Compact Flash medium
         */
        CompactFlash,

        /*!
         * Blu-ray Disc medium.
         */
        BluRay,

        /*!
         * Memory Stick medium.
         */
        MemoryStick,

        /*!
         * HD DVD medium.
         */
        HD_DVD,

        /*!
         * Vendor-defined medium.
         */
        VendorDefined
    };

    QString m_typeAsString;
    Type m_type;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HStorageMedium();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the storage medium type.
     *
     * \sa isValid()
     */
    HStorageMedium(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the storage medium. If it isn't one of the types
     * defined by Type and it isn't empty, the type() is set to
     * HStorageMedium::VendorDefined.
     *
     * \sa isValid()
     */
    HStorageMedium(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HStorageMedium::Unknown.
     */
    inline bool isValid() const { return m_type != Unknown; }

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
 * \relates HStorageMedium
 */
H_UPNP_AV_EXPORT bool operator==(const HStorageMedium& obj1, const HStorageMedium& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HStorageMedium
 */
inline bool operator!=(const HStorageMedium& obj1, const HStorageMedium& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \param key specifies the HStorageMedium object from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 */
H_UPNP_AV_EXPORT quint32 qHash(const HStorageMedium& key);

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HStorageMedium)

#endif /* HSTORAGEMEDIUM_H_ */
