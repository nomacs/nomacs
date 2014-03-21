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

#ifndef HRESOURCE_TYPE_H_
#define HRESOURCE_TYPE_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QMetaType>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

class HResourceTypePrivate;

/*!
 * \brief This is a class used to depict a UPnP resource, which is either a UPnP device or a
 * UPnP service.
 *
 * Both UPnP device and service descriptions use the \em type concept to a give the
 * corresponding device or service context that can be used in identification.
 * In device descriptions the device type is specified following the format
 * \verbatim urn:schemas-upnp-org:device:deviceType:ver \endverbatim  or
 * \verbatim urn:domain-name:device:deviceType:ver \endverbatim in case of a
 * vendor defined type.
 *
 * In service descriptions the service type is specified as
 * \verbatim urn:schemas-upnp-org:service:serviceType:ver \endverbatim  or
 * \verbatim urn:domain-name:service:serviceType:ver \endverbatim in case of a
 * vendor defined type.
 *
 * For more information, see the <em>device type</em> and <em>service type</em>
 * definitions in UDA v1.1 at pages 44 and 46, respectively.
 *
 * \brief This class abstracts the above service and device type concepts to a \em resource
 * and helps in handling the various elements of a <em>resource type</em>.
 *
 * \headerfile hresourcetype.h HResourceType
 *
 * \remarks This class is not thread-safe.
 *
 * \ingroup hupnp_common
 */
class H_UPNP_CORE_EXPORT HResourceType
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HResourceType&, const HResourceType&);

public:

    /*!
     * \brief Specifies the type of the resource. See UPnP v1.1 Device Architecture
     * specification for more information.
     */
    enum Type
    {
        /*!
         * No resource defined. This is used when the object is constructed using
         * the default constructor.
         */
        Undefined = 0,

        /*!
         * The resource is urn:schemas-upnp-org:device:deviceType:ver.
         */
        StandardDeviceType,

        /*!
         * The resource is urn:schemas-upnp-org:service:serviceType:ver.
         */
        StandardServiceType,

        /*!
         * The resource is urn:domain-name:device:deviceType:ver.
         */
        VendorSpecifiedDeviceType,

        /*!
         * The resource is urn:domain-name:service:serviceType:ver.
         */
        VendorSpecifiedServiceType
    };

    /*!
     * \brief This enumeration specifies how the version part of a HResourceType
     * is matched against a target value.
     */
    enum VersionMatch
    {
        /*!
         * The version part of HResoureType objects is ignored.
         */
        Ignore,

        /*!
         * The version part of HResourceType object has to be identical
         * to the specified value.
         */
        Exact,

        /*!
         * The version part of HResourceType object has to be
         * less than or equal to the specified value.
         */
        Inclusive,

        /*!
         * The version part of HResourceType object has to be greater than or
         * equal to the specified value.
         */
        EqualOrGreater
    };

private:

    Type m_type;
    QStringList m_resourceElements;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * Instance created by this constructor is not valid, i.e. isValid() will return false.
     *
     * \sa isValid()
     */
    HResourceType();

    /*!
     * Constructs a new, empty instance from the specified parameter.
     *
     * \param resourceTypeAsStr specifies the resource type following the one of
     * the following formats:
     *
     * \li <c>urn:schemas-upnp-org:device:deviceType:ver</c> for standard
     * <em>device type</em>
     *
     * \li <c>urn:domain-name:device:deviceType:ver</c> for vendor defined
     * <em>device type</em>
     *
     * \li <c>urn:schemas-upnp-org:service:serviceType:ver</c> for standard
     * <em>service type</em>
     *
     * \li <c>urn:domain-name:service:serviceType:ver for</c> vendor defined
     * <em>service type</em>
     *
     * \sa isValid()
     */
    HResourceType(const QString& resourceTypeAsStr);

    /*!
     * \brief Destroys the instance.
     */
    ~HResourceType();

    /*!
     * \brief Returns the type of the resource.
     *
     * \return The type of the resource.
     */
    inline Type type() const { return m_type; }

    /*!
     * \brief Indicates if the object is valid.
     *
     * This method is provided for convenience. It simply checks if the
     * type() is HResourceType::Undefined.
     *
     * \returns \e true in case the object represents a valid resource type.
     */
    inline bool isValid() const { return m_type != Undefined; }

    /*!
     * \brief Indicates whether or not the resource type is a device type.
     *
     * This method is provided for convenience. It checks if the type
     * is either HResourceType::StandardDeviceType or
     * HResourceType::VendorSpecifiedDeviceType.
     *
     * \return \e true in case the resource type is a device type.
     */
    inline bool isDeviceType() const
    {
        return m_type == StandardDeviceType ||
               m_type == VendorSpecifiedDeviceType;
    }

    /*!
     * \brief Indicates whether or not the resource type is a service type.
     *
     * This method is provided for convenience. It checks if the type
     * is either HResourceType::StandardServiceType or
     * HResourceType::VendorSpecifiedServiceType.
     *
     * \return \e true in case the resource type is a service type.
     */
    inline bool isServiceType() const
    {
        return m_type == StandardServiceType ||
               m_type == VendorSpecifiedServiceType;
    }

    /*!
     * \brief Indicates whether or not the resource type is a standard type defined
     * by the UPnP forum.
     *
     * This method is provided for convenience. It checks if the type
     * is either HResourceType::StandardDeviceType or
     * HResourceType::StandardServiceType.
     *
     * \retval true in case the resource type is defined by the UPnP forum.
     * \retval false in case the resource type is vendor defined.
     */
    inline bool isStandardType() const
    {
        return m_type == StandardDeviceType ||
               m_type == StandardServiceType;
    }

    /*!
     * Enumeration that specifies the tokens or parts of a resource type.
     * For instance, if the resource type
     * is defined as <c>urn:schemas-upnp-org:device:deviceType:ver</c> then
     * the tokens are the parts separated by colons.
     */
    enum Token
    {
        /*!
         * This is a special value used to denote "no tokens".
         */
        None = 0x00,

        /*!
         * The "urn:" token.
         */
        UrnPrefix = 0x01,

        /*!
         * The domain token, e.g. "schemas-upnp-org".
         */
        Domain = 0x02,

        /*!
         * The type of the resource, e.g. "device" or "service".
         */
        Type = 0x04,

        /*!
         * The \e type \e suffix of the resource, e.g. "deviceType" or "serviceType".
         */
        TypeSuffix = 0x08,

        /*!
         * The version of the resource type. Most commonly this is an integer.
         */
        Version = 0x10,

        /*!
         * This is a special value used to denote "all tokens".
         */
        All = 0x1f
    };

    Q_DECLARE_FLAGS(Tokens, Token);

    /*!
     * \brief Returns the version of the resource type.
     *
     * \returns the version of the resource type if the object is valid.
     * In case the object is invalid -1 is returned.
     *
     * \sa isValid()
     */
    qint32 version() const;

    /*!
     * \brief Returns a string representation of the object.
     *
     * A resource type can be broken into 5 tokens, which are depicted in the
     * Token enum. This function is used to retrieve an arbitrary combination
     * of these tokens as a string. For instance, if you would like to retrieve
     * the resource type, resource type suffix and the version as a string
     * you would issue:
     *
     * \code
     * QString retVal = tokens(
     *     HResourceType::Type | HResourceType::TypeSuffix | HResourceType::Version);
     * \endcode
     *
     * \param tokens specifies what components of the objects are included
     * in the returned string. The default is to return everything.
     *
     * \return a string representation of the object as defined by the provided
     * tokens if the object is valid. Otherwise an empty string is returned.
     *
     * \remarks
     * By default the contents of the object are returned in full.
     */
    QString toString(Tokens tokens=All) const;

    /*!
     * Compares this object to the provided object according to the specified
     * HResourceType::VersionMatch argument.
     *
     * \param other specifies the other \c %HResourceType object.
     * \param versionMatch specifies how the version information in the objects
     * are compared against one another. The target of the comparison is always
     * \e this object. Therefore if the \c versionMatch is set to
     * HResourceType::Inclusive, the specified \e other object defines
     * the upper bound for the comparison.
     *
     * \return \e true in case the two objects are considered a match
     * taking into account the specified HResourceType::VersionMatch argument.
     */
    bool compare(const HResourceType& other, VersionMatch versionMatch) const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return true in case the object are logically equivalent.
 *
 * \relates HResourceType
 */
H_UPNP_CORE_EXPORT bool operator==(const HResourceType&, const HResourceType&);

/*!
 * Compares the two objects for inequality.
 *
 * \return true in case the objects are not logically equivalent.
 *
 * \relates HResourceType
 */
inline bool operator!=(const HResourceType& obj1, const HResourceType& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \param key specifies the <em>resource type</em> from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \relates HResourceType
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HResourceType& key);

Q_DECLARE_OPERATORS_FOR_FLAGS(HResourceType::Tokens)

}
}

Q_DECLARE_METATYPE(Herqq::Upnp::HResourceType)

#endif /* HRESOURCE_TYPE_H_ */
