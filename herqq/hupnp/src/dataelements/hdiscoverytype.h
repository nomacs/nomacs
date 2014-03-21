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

#ifndef HDISCOVERYTYPE_H_
#define HDISCOVERYTYPE_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QSharedDataPointer>

class QString;

namespace Herqq
{

namespace Upnp
{

class HDiscoveryTypePrivate;

/*!
 * \brief This is a class that depicts the different <em>discovery types</em> used in
 * UPnP networking.
 *
 * The <em>UPnP discovery</em> is based on SSDP messages that provide information
 * about UPnP devices and services the UPnP devices provide. When UPnP devices
 * use SSDP messages they \e advertise themselves and the embedded devices and
 * services they contain. When UPnP control points use SSDP messages they
 * \e search UPnP devices or services matching some criteria. In HUPnP these
 * different search and and advertisement types are called \e discovery \e types
 * represented by instances of this class.
 *
 * For instance, if a UPnP device advertises itself to the network the
 * \e discovery \e type is HDiscoveryType::SpecificDevice, since the
 * Unique Device Name that identifies the device is known. On the other hand,
 * a control point may be interested in searching all UPnP root devices on a
 * network in which case the \e discovery \e type would be HDiscoveryType::RootDevices.
 * Then again, if a control point is interested in searching some specific
 * UPnP root device it can issue a search with the discovery type set to
 * HDiscoveryType::SpecificRootDevice and the UDN set to the desired value.
 *
 * The above example implies that a \e discovery \e type may identify a UPnP device.
 * If a discovery type identifies a UPnP device you can call udn() to retrieve the
 * Unique Device Name. Similarly, you can call setUdn() to specify the desired UDN.
 *
 * A discovery type may also have a \e resource \e type associated with it.
 * A \e resource \e type specifies the exact type of a device or a service
 * and whether it is standard or vendor defined. If the resource type is specified
 * you can call resourceType() to retrieve it. Similarly, you can call
 * setResourceType() to specify it.
 *
 * \headerfile hdiscoverytype.h HDiscoveryType
 *
 * \remarks This class is not thread-safe.
 *
 * \ingroup hupnp_common
 *
 * \sa Type, type()
 */
class H_UPNP_CORE_EXPORT HDiscoveryType
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HDiscoveryType&, const HDiscoveryType&);

private:

    QSharedDataPointer<HDiscoveryTypePrivate> h_ptr;

public:

    /*!
     * \brief Specifies the discovery type. See UPnP v1.1 Device Architecture
     * specification chapter 1 for more information.
     */
    enum Type
    {
        /*!
         * The discovery type is unspecified.
         *
         * This value is used when the discovery type is invalid.
         */
        Undefined = 0,

        /*!
         * The discovery type is <c>"ssdp:all"</c>.
         *
         * This is used when the discovery type specifies all UPnP devices and services.
         * This value is used by control points to perform a device discovery for all
         * UPnP devices and services.
         */
        All,

        /*!
         * The discovery type is <c>"upnp:rootdevice"</c>.
         *
         * This is used when the discovery type specifies all UPnP root devices.
         * This value is used by control points to perform a device discovery for
         * all UPnP root devices.
         */
        RootDevices,

        /*!
         * The discovery type is <c>"uuid:device-UUID::upnp:rootdevice"</c>.
         *
         * This is used when the discovery type specifies a particular
         * UPnP root device.
         *
         * This value is used by control points and UPnP devices to search or
         * advertise a specific UPnP root device.
         */
        SpecificRootDevice,

        /*!
         * The discovery type is <c>"uuid:device-UUID"</c>.
         *
         * This is used when the discovery type specifies a particular UPnP device,
         * which may be a root or an embedded device.
         *
         * This value is used by control points and UPnP devices to search or
         * advertise a specific UPnP device.
         */
        SpecificDevice,

        /*!
         * The discovery type is <c>"urn:schemas-upnp-org:device:deviceType:ver"</c> or
         * <c>"urn:domain-name:device:deviceType:ver"</c>.
         *
         * This is used when the discovery type specifies devices of certain type.
         *
         * This value is used by control points to perform discovery for
         * specific type of UPnP devices.
         */
        DeviceType,

        /*!
         * The discovery type is
         * <c>"uuid-device-UUID::urn:schemas-upnp-org:device:deviceType:ver"</c>
         * or <c>"uuid-device-UUID::urn:domain-name:device:deviceType:ver"</c>.
         *
         * This is used when the discovery type specifies a particular device
         * that is of certain type.
         *
         * This value is used by control points and UPnP devices to search or
         * advertise a specific UPnP device that is of specific type.
         */
        SpecificDeviceWithType,

        /*!
         * The discovery type is <c>"urn:schemas-upnp-org:service:serviceType:ver"</c> or
         * <c>"urn:domain-name:service:serviceType:ver"</c>.
         *
         * This is used when the discovery type specifies services of certain type.
         *
         * This value is used by control points to perform discovery for
         * specific type of UPnP services.
         */
        ServiceType,

        /*!
         * The discovery type is
         * <c>"uuid-device-UUID::urn:schemas-upnp-org:service:serviceType:ver"</c>
         * or <c>"uuid-device-UUID::urn:domain-name:service:serviceType:ver"</c>.
         *
         * This is used when the discovery type specifies particular service
         * that is of certain type.
         *
         * This value is used by control points and UPnP devices to search or
         * advertise a specific type of UPnP service within a specific UPnP device.
         */
        SpecificServiceWithType
    };

    /*!
     * Creates a new empty instance.
     *
     * The type is set to HDiscoveryType::Undefined.
     *
     * \sa type()
     */
    HDiscoveryType();

    /*!
     * \brief Creates a new instance.
     *
     * A discovery type created using a valid UDN identifies a UPnP
     * device as the resource. The resource type is not specified.
     *
     * \param udn specifies the contents of the object. In case the provided
     * argument is valid the type() of the created object is
     * HDiscoveryType::SpecificDevice. Otherwise the type is set to
     * HDiscoveryType::Undefined.
     *
     * \param isRootDevice indicates whether the specified UDN belongs to a
     * root device. If the value is \e true the type() is set to
     * HDiscoveryType::SpecificRootDevice. If the value is \e false the
     * type is set to HDiscoveryType::SpecificDevice.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified arguments. This parameter is optional.
     *
     * \sa type()
     */
    explicit HDiscoveryType(
        const HUdn& udn, bool isRootDevice=false,
        HValidityCheckLevel checkLevel = StrictChecks);

    /*!
     * \brief Creates a new instance.
     *
     * A discovery type created using a valid resource type specifies the device
     * or service type. No UDN is provided, which means the discovery type does not
     * specify a UPnP device.
     *
     * \param resourceType specifies the contents of the object. In case
     * the provided argument is valid the type() of the created object is either
     * - HDiscoveryType::DeviceType or
     * - HDiscoveryType::ServiceType depending of the argument. In case the
     * provided argument is invalid the type is set to HDiscoveryType::Undefined.
     *
     * \sa type(), setUdn()
     */
    explicit HDiscoveryType(const HResourceType& resourceType);

    /*!
     * \brief Creates a new instance.
     *
     * A discovery type created with a valid UDN and a valid resource type
     * identifies a specific UPnP device or UPnP service that is of certain type.
     * Both of the provided arguments have to be valid in order to create a valid type.
     * Otherwise the type() is set to HDiscoveryType::Undefined.
     *
     * \param udn specifies the Unique Device Name.
     *
     * \param resourceType specifies the resource type.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified arguments. This parameter is optional.
     *
     * \sa type(), udn(), resourceType()
     */
    HDiscoveryType(
        const HUdn& udn, const HResourceType& resourceType,
        HValidityCheckLevel checkLevel = StrictChecks);

    /*!
     * \brief Creates a new instance.
     *
     * \param resource specifies the contents of the object. In case the
     * the provided argument cannot be parsed to a valid resource identifier the
     * type() is set to HDiscoveryType::Undefined.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified arguments. This parameter is optional.
     *
     * Valid string formats are:
     *
     * - <c>ssdp:all</c> to specify "any UPnP device and service",
     * - <c>[uuid:device-UUID]</c> to spesify a specific UPnP device,
     * - <c>[uuid:device-UUID::]upnp:rootdevice</c> to specify a specific UPnP root device,
     * - <c>[uuid:device-UUID::]urn:schemas-upnp-org:device:deviceType:ver</c>
     * to specify a specific UPnP device, which type is standard defined.
     * - <c>[uuid:device-UUID::]urn:domain-name:device:deviceType:ver</c>
     * to specify a specific UPnP device that, which type is vendor defined.
     * - <c>[uuid:device-UUID::]urn:schemas-upnp-org:service:serviceType:ver</c>
     * to specify a specific UPnP service, which type is standard defined.
     * - <c>[uuid:device-UUID::]urn:domain-name:service:serviceType:ver</c>
     * to specify a specific UPnP device, which type is vendor defined.
     *
     * The content inside square brackets (uuid:device-UUID) is optional and does
     * not have to be provided.
     */
    explicit HDiscoveryType(
        const QString& resource, HValidityCheckLevel checkLevel = StrictChecks);

    /*!
     * \brief Destroys the instance.
     */
    ~HDiscoveryType();

    /*!
     * Copies the contents of the other object to this.
     */
    HDiscoveryType(const HDiscoveryType&);

    /*!
     * Assigns the contents of the other object to this.
     *
     * \return a reference to this object.
     */
    HDiscoveryType& operator=(const HDiscoveryType&);

    /*!
     * \brief Returns the type of the object.
     *
     * \return The type of the object. If the resource is not specified the type
     * returned is HDiscoveryType::Undefined.
     */
    Type type() const;

    /*!
     * \brief Returns the Unique Device Name of the resource.
     *
     * \return The Unique Device Name of the resource if it is set. Note,
     * the UDN is never set when the type() is either
     * HDiscoveryType::AllDevices or HDiscoveryType::Undefined.
     *
     * \sa setUdn()
     */
    const HUdn& udn() const;

    /*!
     * \brief Sets the UDN of the object.
     *
     * \note Changing the UDN may change the resourceType() and the type().
     * For instance, if the object did not have UDN set before,
     * changing the UDN will change the type() of the object.
     *
     * \param udn specifies the new UDN. The specified UDN has to be valid
     * in terms of the level of validity checks being run. Otherwise the UDN
     * will not be set.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified UDN. This parameter is optional, but by default
     * the UDN is verified strictly.
     *
     * \sa udn()
     */
    void setUdn(const HUdn& udn, HValidityCheckLevel checkLevel = StrictChecks);

    /*!
     * \brief Returns the resource type associated with this identifier, if any.
     *
     * \return The resource type associated with this identifier if it is set.
     * Note, the returned object is valid only when the type() is either
     * standard or vendor specified device or service type.
     *
     * \sa setResourceType()
     */
    const HResourceType& resourceType() const;

    /*!
     * \brief Sets the resource type of the object.
     *
     * \param resourceType specifies the new resource type.
     *
     * \sa resourceType()
     */
    void setResourceType(const HResourceType& resourceType);

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object or an empty string,
     * if the object does not specify a valid resource.
     *
     * \sa Type
     */
    QString toString() const;

    /*!
     * Creates an object, which type is set to HDiscoveryType::RootDevices.
     *
     * \return an object, which type is set to HDiscoveryType::RootDevices.
     *
     * \remarks This is only a helper method. A logically equivalent object
     * can be constructed with the string <c>"upnp:rootdevice"</c>.
     */
    static HDiscoveryType createDiscoveryTypeForRootDevices();

    /*!
     * Creates an object, which type is set to HDiscoveryType::All.
     *
     * \return an object, which type is set to HDiscoveryType::All.
     *
     * \remarks This is only a helper method. A logically equivalent object
     * can be constructed with the string <c>"ssdp:all"</c>.
     */
    static HDiscoveryType createDiscoveryTypeForAllResources();
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HDiscoveryType
 */
H_UPNP_CORE_EXPORT bool operator==(const HDiscoveryType&, const HDiscoveryType&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HDiscoveryType
 */
inline bool operator!=(const HDiscoveryType& obj1, const HDiscoveryType& obj2)
{
    return !(obj1 == obj2);
}

}
}

#endif /* HDISCOVERYTYPE_H_ */
