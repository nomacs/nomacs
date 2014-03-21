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

#ifndef HCLIENTDEVICE_H_
#define HCLIENTDEVICE_H_

#include <HUpnpCore/HResourceType>

#include <QtCore/QObject>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

class HClientDevicePrivate;

/*!
 * \brief This is a client-side class that represents a server-side UPnP device.
 *
 * \c %HClientDevice is a core component of the HUPnP's client-side \ref hupnp_devicemodel
 * and it models a UPnP device, both root and embedded. As detailed in the
 * UPnP Device Architecture specification, a UPnP device is essentially a
 * container for services and possibly for other (embedded) UPnP devices.
 *
 * <h2>Using the class</h2>
 *
 * The most common uses of \c %HClientDevice involve reading the various device
 * information elements originally set in the device description file and enumerating the
 * exposed services. By calling info() you get an HDeviceInfo object from
 * which you can read all the informational elements found in the device description.
 * Calling services() gives you a list of HClientService instances the device
 * exposes. Note that it is the services that contain the functionality
 * and runtime status of the device.
 *
 * Some devices also contain embedded devices, which you can get by calling
 * embeddedDevices().
 *
 * You can retrieve the device's description file by calling description() or
 * you can manually read it from any of the locations returned by locations(). If
 * the device is an embedded device, it always has a parent device, which you can
 * get by calling parentDevice().
 *
 * \headerfile hclientdevice.h HClientDevice
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HClientDevice :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HClientDevice)
H_DECLARE_PRIVATE(HClientDevice)

protected:

    HClientDevicePrivate* h_ptr;

    /*!
     * \brief Creates a new instance.
     *
     * \param info specifies information of the device. This is often read
     * from a device description document.
     *
     * \param parentDev specifies the parent device of this device, if any.
     */
    HClientDevice(const HDeviceInfo& info, HClientDevice* parentDev = 0);

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HClientDevice() = 0;

    /*!
     * \brief Returns the parent device of this device, if any.
     *
     * \return The parent device of this device, if any, or a null pointer
     * in case the device is a root device.
     *
     * \remarks The pointer is guaranteed to be valid throughout the lifetime
     * of this object.
     */
    HClientDevice* parentDevice() const;

    /*!
     * \brief Returns the root device of the device tree to which this device belongs.
     *
     * \return The root device of the device tree to which this device belongs.
     *
     * \remarks This device could be the root device of the device tree in question,
     * in which case a pointer to this instance is returned.
     */
    HClientDevice* rootDevice() const;

    /*!
     * \brief Returns the service that has the specified service ID.
     *
     * \param serviceId specifies the service to be returned.
     *
     * \return The service that has the specified service ID or a null pointer
     * in case there is no service with the specified ID.
     *
     * \remarks The pointer is guaranteed to be valid throughout the lifetime
     * of this object.
     */
    HClientService* serviceById(const HServiceId& serviceId) const;

    /*!
     * \brief Returns the services this device exports.
     *
     * \return The services this device exports. The collection is empty
     * if the device has no services.
     *
     * \remarks The pointers are guaranteed to be valid throughout the lifetime
     * of this object.
     */
    HClientServices services() const;

    /*!
     * \brief Returns the services of a specific UPnP service type.
     *
     * \param serviceType specifies the UPnP service type of interest.
     * Only services matching the type are returned.
     *
     * \param versionMatch specifies how the version information in argument
     * \c serviceType should be used. The default is <em>inclusive match</em>,
     * which essentially means that any service with a service type version that
     * is \b less than or \b equal to the version specified in argument
     * \c serviceType is successfully matched.
     *
     * \return The services of the specified type.
     *
     * \remarks the pointers are guaranteed to be valid throughout the lifetime
     * of this object.
     */
    HClientServices servicesByType(
        const HResourceType& serviceType,
        HResourceType::VersionMatch versionMatch = HResourceType::Inclusive) const;

    /*!
     * \brief Returns the embedded devices of this device.
     *
     * \return The embedded devices of this device. The collection is empty
     * if the device has no embedded devices.
     *
     * \remarks the pointers are guaranteed to be valid throughout the lifetime
     * of this object.
     */
    HClientDevices embeddedDevices() const;

    /*!
     * \brief Returns the embedded devices of a specific UPnP device type.
     *
     * \param deviceType specifies the UPnP device type of interest.
     * Only devices matching the type are returned.
     *
     * \param versionMatch specifies how the version information in argument
     * \a deviceType should be used. The default is <em>inclusive match</em>,
     * which essentially means that any device with a device type version that
     * is \b less than or \b equal to the version specified in argument
     * \a deviceType is successfully matched.
     *
     * \return The embedded devices of the specified type.
     *
     * \remarks the pointers are guaranteed to be valid throughout the lifetime
     * of this object.
     */
    HClientDevices embeddedDevicesByType(
        const HResourceType& deviceType,
        HResourceType::VersionMatch versionMatch = HResourceType::Inclusive) const;

    /*!
     * \brief Returns information about the device.
     *
     * \return information about the device. This is often read from the
     * device description.
     */
    const HDeviceInfo& info() const;

    /*!
     * \brief Returns the UPnP device description of this device.
     *
     * \return The UPnP device description that is associated to this device.
     *
     * \remarks an embedded device returns the same device description as
     * its root device.
     */
    QString description() const;

    /*!
     * \brief Returns a list of locations where the device is currently available.
     *
     * \param urlType specifies whether the returned
     * location URLs are absolute URLs for retrieving the device description.
     * By default absolute URLs are returned and from these URLs the device
     * description should be retrievable.
     *
     * \return a list of locations where the device is currently available.
     */
    QList<QUrl> locations(LocationUrlType urlType=AbsoluteUrl) const;
};

}
}

#endif /* HCLIENTDEVICE_H_ */
