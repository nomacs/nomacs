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

#ifndef HTRANSPORTINFO_H_
#define HTRANSPORTINFO_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HTransportState>

#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with Transport Status values
 * defined in the AVTransport:2 specification.
 *
 * \headerfile htransportinfo.h HTransportStatus
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HTransportStatus
{

public:

    /*!
     * \brief This enumeration specifies the Transport Status values defined
     * in the AVTransport:2 specification.
     */
    enum Type
    {
        /*!
         * This value is used when the Transport Status is unknown.
         */
        Undefined,

        /*!
         * The status is OK. No error has occurred.
         */
        OK,

        /*!
         * The last transport operation failed.
         */
        ErrorOccurred,

        /*!
         * The value was not defined in the AVTransport:2 specification.
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
    HTransportStatus();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the record medium write status.
     *
     * \sa isValid()
     */
    HTransportStatus(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the transport status. If it isn't one of the types
     * defined by the Type and it isn't empty, the type() is set to
     * HTransportStatus::VendorDefined.
     *
     * \sa isValid()
     */
    HTransportStatus(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HTransportStatus::Undefined.
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
 * \relates HTransportStatus
 */
H_UPNP_AV_EXPORT bool operator==(const HTransportStatus& obj1, const HTransportStatus& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HTransportStatus
 */
inline bool operator!=(const HTransportStatus& obj1, const HTransportStatus& obj2)
{
    return !(obj1 == obj2);
}

class HTransportInfoPrivate;

/*!
 * \brief This is a class used to contain information about the transport state of a
 * virtual AVTransport instance.
 *
 * \headerfile htransportinfo.h HTransportInfo
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HTransportInfo
{

private:

    QSharedDataPointer<HTransportInfoPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HTransportInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param state specifies the transport state.
     *
     * \param status specifies the transport status.
     *
     * \param speed specifies a rational fraction indicating
     * the speed relative to normal speed.
     *
     * \sa isValid()
     */
    HTransportInfo(
        const HTransportState& state,
        const HTransportStatus& status,
        const QString& speed = QString());

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HTransportInfo(const HTransportInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HTransportInfo& operator=(const HTransportInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HTransportInfo();

    /*!
     * \brief Returns the transport state.
     *
     * \return The transport state.
     *
     * \sa setState()
     */
    const HTransportState& state() const;

    /*!
     * \brief Returns the transport status.
     *
     * \return The transport status.
     *
     * \sa setStatus()
     */
    const HTransportStatus& status() const;

    /*!
     * \brief Returns the transport play speed.
     *
     * \return a rational fraction indicating the speed relative to
     * normal speed.
     *
     * \sa setSpeed()
     */
    QString speed() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. at least the
     * state() and status() are valid.
     */
    bool isValid();

    /*!
     * \brief Sets the transport state.
     *
     * \param arg specifies the transport state.
     *
     * \sa state()
     */
    void setState(const HTransportState& arg);

    /*!
     * \brief Sets the transport status.
     *
     * \param arg specifies the transport status.
     *
     * \sa status()
     */
    void setStatus(const HTransportStatus& arg);

    /*!
     * \brief Sets the transport play speed.
     *
     * \param arg specifies a rational fraction indicating
     * the speed relative to normal speed.
     *
     * \sa speed()
     */
    void setSpeed(const QString& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HTransportInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HTransportInfo& obj1, const HTransportInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HTransportInfo
 */
inline bool operator!=(const HTransportInfo& obj1, const HTransportInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HTRANSPORTINFO_H_ */
