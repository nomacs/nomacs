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

#ifndef HSERVICEID_H_
#define HSERVICEID_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QMetaType>

class QString;

namespace Herqq
{

namespace Upnp
{

class HServiceIdPrivate;

/*!
 * Class that represents the <em>service identifier</em> of a UPnP service.
 *
 * Service identifiers are found in UPnP device descriptions and they use
 * the following format within services defined by the UPnP Forum:
 * \verbatim urn:upnp-org:serviceId:serviceID \endverbatim
 *
 * In the above format only the tailing \e serviceID varies.
 * Every service identifier of a standard service type has to begin with
 * <c>urn:upnp-org:serviceId:</c>.
 *
 * With a vendor defined service the format for a service identifier is:
 * \verbatim urn:domain-name:serviceId:serviceID \endverbatim
 *
 * Note, according to the UDA specification <em>Period characters in the
 * Vendor Domain Name MUST be replaced with hyphens in accordance with RFC 2141</em>.
 *
 * In both formats, the last \e serviceID component is the
 * <em>service identifier suffix</em>.
 *
 * \note
 * For interoperability reasons the class does not enforce the prefix prior
 * to the actual \c serviceID to be exactly as defined in the UDA. However, you
 * can call isValid() to check if the instance contains strictly valid information.
 *
 * \headerfile hserviceid.h HServiceId
 *
 * \remarks This class is not thread-safe.
 *
 * \ingroup hupnp_common
 */
class H_UPNP_CORE_EXPORT HServiceId
{
friend H_UPNP_CORE_EXPORT bool operator==(const HServiceId&, const HServiceId&);
friend H_UPNP_CORE_EXPORT quint32 qHash(const HServiceId& key);

private:

    HServiceIdPrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * Instance created by this constructor is not valid, i.e. isValid()
     * will return false.
     *
     * \sa isValid
     */
    HServiceId();

    /*!
     * Constructs a new instance.
     *
     * \param serviceId specifies the contents of the object. If the provided
     * argument is invalid an empty instance is created. For an object
     * to be strictly valid the parameter has to follow either of the formats
     * exactly:
     *
     * \li <c>urn:upnp-org:serviceId:serviceID</c> for service identifiers
     * belonging to a standard <em>service type</em>.
     *
     * \li <c>urn:domain-name:serviceId:serviceID</c> for service identifiers
     * belonging to a vendor defined <em>service type</em>.
     *
     * The postfix serviceID is the <em>service identifier suffix</em>.
     *
     * \note a valid object will be constructed when the specified string contains
     * the following tokens separated by a colon:
     * "urn", "domain-name", "some string" and "some string".
     * Case sensitivity is forced only when checking for strict validity.
     *
     * \sa isValid()
     */
    HServiceId(const QString& serviceId);

    /*!
     * Creates a new instance based on the other instance provided.
     *
     * \param other specifies the other instance.
     */
    HServiceId(const HServiceId& other);

    /*!
     * Assigns the contents of the other instance to this.
     *
     * \param other specifies the other instance.
     *
     * \return a reference to this instance.
     */
    HServiceId& operator=(const HServiceId& other);

    /*!
     * \brief Destroys the instance.
     */
    ~HServiceId();

    /*!
     * \brief Indicates if the service identifier is properly defined.
     *
     * \param level specifies whether the contents of the object are checked
     * for strict validity. Only an object that is strictly valid contains information
     * as defined in the UDA.
     *
     * \return \e true in case the object is considered valid in terms
     * of the requested strictness.
     */
    bool isValid(HValidityCheckLevel level) const;

    /*!
     * \brief Indicates whether the service identifier belongs to a standard service
     * type defined by the UPnP forum or to a vendor defined service.
     *
     * \retval true in case the service identifier belongs to a standard service
     * type defined by the UPnP forum.
     * \retval false in case the service identifier belongs to a vendor
     * defined service type or the object is invalid.
     *
     * \sa isValid()
     */
    bool isStandardType() const;

    /*!
     * \brief Returns the URN of the service identifier.
     *
     * \param completeUrn specifies whether the prefix \c urn is returned
     * as well. If the argument is false, only the actual URN is returned. i.e
     * if the service identifier is defined as <c>urn:upnp-org:serviceId:MyServiceId</c>
     * only <c>upnp-org</c> is returned.
     *
     * \returns the URN of the service identifier if the object is valid.
     * If the object is not valid, an empty string is returned.
     *
     * \sa isValid()
     */
    QString urn(bool completeUrn = true) const;

    /*!
     * \brief Returns the service identifier suffix.
     *
     * \returns the service identifier suffix if the object is valid. For instance, if the
     * service identifier is defined as <c>urn:upnp-org:serviceId:MyServiceId</c>,
     * the suffix identifier and thus the value returned is \c "MyServiceId".
     * If the object is not valid, an empty string is returned.
     *
     * \sa isValid()
     */
    QString suffix() const;

    /*!
     * \brief Returns a string representation of the instance.
     *
     * \return a string representation of the instance. The returned string
     * follows the format defined by UDA if the object is valid. In case of a valid
     * object, the return value is the string that was used to construct the object.
     * If the object is invalid, the returned string is empty.
     *
     * \sa isValid()
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the provided objects are equal, false otherwise.
 *
 * \relates HServiceId
 */
H_UPNP_CORE_EXPORT bool operator==(const HServiceId&, const HServiceId&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the provided objects are not equal, false otherwise.
 *
 * \relates HServiceId
 */
inline bool operator!=(const HServiceId& obj1, const HServiceId& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \param key specifies the <em>service ID</em> from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \relates HServiceId
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HServiceId& key);

}
}

Q_DECLARE_METATYPE(Herqq::Upnp::HServiceId)

#endif /* HSERVICEID_H_ */
