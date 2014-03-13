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

#ifndef HDEVICES_SETUPDATA_H_
#define HDEVICES_SETUPDATA_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HResourceType>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

class HDeviceSetupPrivate;

/*!
 * \brief This class is used to specify information that can be used to validate
 * a UPnP device.
 *
 * \headerfile hdevices_setupdata.h HDeviceSetup
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HDevicesSetupData, HClientDevice, HServerDevice
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HDeviceSetup
{

private:

    QSharedDataPointer<HDeviceSetupPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HDeviceSetup();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the device type.
     *
     * \param incReq specifies <em>inclusion requirement</em> of the device.
     *
     * \sa isValid()
     *
     * \remarks the version() is set to 1.
     */
    HDeviceSetup(
        const HResourceType& type,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the device type.
     *
     * \param version specifies the version of the UPnP device,
     * which first specified the embedded device.
     *
     * \param incReq specifies <em>inclusion requirement</em> of the device.
     *
     * \sa isValid()
     */
    HDeviceSetup(
        const HResourceType& type,
        int version,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * \brief Destroys the instance.
     */
    ~HDeviceSetup();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \a other.
     */
    HDeviceSetup& operator=(const HDeviceSetup&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \a other to this.
     */
    HDeviceSetup(const HDeviceSetup&);

    /*!
     * \brief Returns the device type.
     *
     * \return The device type.
     *
     * \sa setDeviceType()
     */
    const HResourceType& deviceType() const;

    /*!
     * \brief Returns the <em>inclusion requirement</em>.
     *
     * \return The <em>inclusion requirement</em>.
     *
     * \sa setInclusionRequirement()
     */
    HInclusionRequirement inclusionRequirement() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, that is, the device type,
     * version and inclusion requirement are properly defined.
     *
     * \sa version(), deviceType(), inclusionRequirement()
     */
    bool isValid() const;

    /*!
     * \brief Returns the version of the UPnP device, which first specified the
     * embedded device.
     *
     * \return The version of the UPnP device, which first specified the
     * embedded device.
     *
     * \sa setVersion()
     */
    int version() const;

    /*!
     * \brief Sets the the <em>inclusion requirement</em>.
     *
     * \param arg specifies the <em>inclusion requirement</em>.
     *
     * \sa inclusionRequirement()
     */
    void setInclusionRequirement(HInclusionRequirement arg);

    /*!
     * \brief Sets the device type.
     *
     * \param arg specifies the device type.
     *
     * \sa deviceType()
     */
    void setDeviceType(const HResourceType& arg);

    /*!
     * \brief Specifies the version of the UPnP device, which first specified the
     * embedded device.
     *
     * \param version specifies the version of the UPnP device,
     * which first specified the embedded device.
     *
     * \sa version()
     */
    void setVersion(int version);
};

/*!
 * \brief This class is used to specify information that can be used to validate
 * UPnP devices.
 *
 * \headerfile hdevices_setupdata.h HDevicesSetupData
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HDeviceSetup
 */
class H_UPNP_CORE_EXPORT HDevicesSetupData
{

private:

    QHash<HResourceType, HDeviceSetup> m_deviceSetupInfos;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HDevicesSetupData();

    /*!
     * \brief Destroys the instance.
     */
    ~HDevicesSetupData();

    /*!
     * \brief Indicates if the instance contains an item with the
     * specified device type.
     *
     * \param deviceType specifies the device type of the searched item.
     *
     * \return \e true when the instance contains an item with the specified
     * device type.
     *
     * \sa get()
     */
    bool contains(const HResourceType& deviceType) const;

    /*!
     * \brief Returns the device types of the contained items.
     *
     * \return The device types of the contained items.
     */
    QSet<HResourceType> deviceTypes() const;

    /*!
     * \brief Retrieves an item.
     *
     * \param type specifies the device type of the item.
     *
     * \return The item with the specified device type. The returned item is
     * invalid in case no item with the specified device type was found.
     *
     * \sa contains()
     */
    HDeviceSetup get(const HResourceType& type) const;

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true in case the instance has no items.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the number of contained items.
     *
     * \return The number of contained items.
     */
    int size() const;

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \return \e true in case the item was added. The \a newItem will not be added
     * if the instance already contains an item that has the
     * same HDeviceSetup::deviceType() as the \a newItem or the \a newItem is
     * invalid.
     */
    bool insert(const HDeviceSetup& newItem);

    /*!
     * Removes an existing item.
     *
     * \param type specifies the device type of the item to be removed.
     *
     * \return \e true in case the item was found and removed.
     */
    bool remove(const HResourceType& type);
};

}
}

#endif /* HDEVICES_SETUPDATA_H_ */
