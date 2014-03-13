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

#ifndef HTRANSPORTACTION_H_
#define HTRANSPORTACTION_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

template<typename T>
class QSet;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with Transport Actions
 * defined in the AVTransport:2 specification.
 *
 * \headerfile htransportaction.h HTransportAction
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HTransportAction
{
public:

    /*!
     * \brief This enumeration specifies the Transport Action types defined in the
     * AVTransport:2 specification.
     */
    enum Type
    {
        /*!
         * This value is used in situations when a proper Transport Action value
         * is not defined.
         */
        Undefined,

        /*!
         * Play.
         */
        Play,

        /*!
         * Stop.
         */
        Stop,

        /*!
         * Pause.
         */
        Pause,

        /*!
         * Seek.
         */
        Seek,

        /*!
         * Next.
         */
        Next,

        /*!
         * Previous.
         */
        Previous,

        /*!
         * Record.
         */
        Record,

        /*!
         * The value was not defined in the AVTransport:2 specification.
         */
        VendorDefined
    };

private:

    Type m_type;
    QString m_typeAsString;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HTransportAction();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the Transport Action type.
     *
     * \sa isValid()
     */
    HTransportAction(Type type);

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the Transport Action type as string.
     * If the specified string does not correspond to any Type value and the
     * string is not empty, the type() is set to HTransportAction::VendorDefined.
     *
     * \sa isValid()
     */
    HTransportAction(const QString& type);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true when the object is valid, i.e. the type() is defined.
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

    /*!
     * Returns a set containing every standard Transport Action.
     *
     * \return a set containing every standard Transport Action.
     */
    static QSet<HTransportAction> allActions();
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HTransportAction
 */
H_UPNP_AV_EXPORT bool operator==(const HTransportAction&, const HTransportAction&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HTransportAction
 */
inline bool operator!=(const HTransportAction& obj1, const HTransportAction& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \param key specifies the HTransportAction object from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 */
H_UPNP_AV_EXPORT quint32 qHash(const HTransportAction& key);

}
}
}

#endif /* HTRANSPORTACTION_H_ */
