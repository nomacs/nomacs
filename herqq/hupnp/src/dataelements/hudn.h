/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HUDN_H_
#define HUDN_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QUuid>
#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

/*!
 * \brief This is a class used to depict a <em>Unique Device Name</em> (UDN), which is a
 * unique device identifier that has to remain the same over time for a
 * specific device instance.
 *
 * A valid UDN follows the format \c "uuid:"+"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
 * where the five hex fields form up a valid UUID.
 *
 * \headerfile hudn.h HUdn
 *
 * \remarks This class is not thread-safe.
 *
 * \ingroup hupnp_common
 */
class H_UPNP_CORE_EXPORT HUdn
{
friend H_UPNP_CORE_EXPORT quint32 qHash(const HUdn&);
friend H_UPNP_CORE_EXPORT bool operator==(const HUdn&, const HUdn&);

private:

    QString m_value;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * Instance created by this constructor is not valid, i.e. isValid() will
     * return false.
     *
     * \sa isValid
     */
    HUdn();

    /*!
     * Constructs a new instance based on the provided value.
     *
     * \param value specifies the UUID of the UDN. If the provided UUID is invalid,
     * the created HUdn is invalid as well.
     *
     * \sa isValid
     */
    HUdn(const QUuid& value);

    /*!
     * Constructs a new instance based on the provided value.
     *
     * \param value specifies the string from which the object is constructed.
     * The argument has to contain a valid UUID and it can be prefixed with
     * "uuid:". The UUID part in turn must be formatted along the requirements of \c QUuid:
     * the string "must be formatted as five hex fields separated
     * by '-', e.g., "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
     * where 'x' is a hex digit. The curly braces shown here are optional,
     * but it is normal to include them. If the argument does not form a
     * proper UUID, the created UDN is invalid.
     *
     * \sa isValid
     */
    HUdn(const QString& value);

    /*!
     * \brief Destroys the instance.
     */
    ~HUdn();

    /*!
     * \brief Indicates if the UDN is defined or not.
     *
     * \param checkLevel specifies whether the check should be done strictly
     * according to the UDA specifications (1.0 & 1.1). That is, the UDN
     * has to contain a proper UUID. If \c checkLevel is \e false the UDN is
     * considered valid if it is not empty.
     *
     * \return true in case the UDN is valid considering the \c checkLevel argument.
     */
    inline bool isValid(HValidityCheckLevel checkLevel) const
    {
        return checkLevel == StrictChecks ? !value().isNull() : !m_value.isEmpty();
    }

    /*!
     * \brief Returns the UUID component of the UDN.
     *
     * \return The UUID component of the UDN.
     *
     * \remarks if the UDN is not strictly valid, i.e. isValid(true) returns
     * \e false, this method will return a null \c QUuid.
     */
    QUuid value() const;

    /*!
     * \brief Returns the complete UDN value.
     *
     * \returns the complete UDN value when the UDN is valid.
     * For instance, \c "uuid:5d794fc2-5c5e-4460-a023-f04a51363300" is a valid UDN.
     * Otherwise an empty string is returned.
     */
    QString toString() const;

    /*!
     * \brief Returns the UUID component of the UDN as string.
     *
     * \returns the UUID component of the UDN as string when the UDN is valid. For instance,
     * if the complete UDN is \c "uuid:5d794fc2-5c5e-4460-a023-f04a51363300", this method
     * will return \c "5d794fc2-5c5e-4460-a023-f04a51363300". Otherwise an
     * empty string is returned.
     */
    QString toSimpleUuid() const;

    /*!
     * Creates a new strictly valid UDN.
     *
     * \return a new strictly valid UDN.
     */
    static HUdn createUdn();
};

/*!
 * Compares the two objects for equality.
 *
 * \return true in case the object are logically equivalent.
 *
 * \relates HUdn
 */
H_UPNP_CORE_EXPORT bool operator==(const HUdn&, const HUdn&);

/*!
 * Compares the two objects for inequality.
 *
 * \return true in case the objects are not logically equivalent.
 *
 * \relates HUdn
 */
inline bool operator!=(const HUdn& obj1, const HUdn& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the UDN object.
 *
 * \param key specifies the \em UDN from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the UDN object.
 *
 * \relates HUdn
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HUdn& key);

}
}

Q_DECLARE_METATYPE(Herqq::Upnp::HUdn)

#endif /* HUDN_H_ */
