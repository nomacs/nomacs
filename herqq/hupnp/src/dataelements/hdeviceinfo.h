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

#ifndef HDEVICEINFO_H_
#define HDEVICEINFO_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QPair>
#include <QtCore/QSharedDataPointer>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

class HDeviceInfoPrivate;

/*!
 * \brief This class is used to contain information of a UPnP device
 * found in a UPnP device description document.
 *
 * A device description specifies a UPnP device. A device description specifies
 * the services of a device, the embedded devices of a device and other information,
 * such as the manufacturer, model name, serial number and the Unique Device Name
 * that uniquely identifies a device. Instances of this class contain the
 * previously mentioned "other" information found in device description documents.
 *
 * \headerfile hdeviceinfo.h HDeviceInfo
 *
 * \ingroup hupnp_common
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HServiceInfo, HActionInfo and HStateVariableInfo.
 */
class H_UPNP_CORE_EXPORT HDeviceInfo
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HDeviceInfo& obj1, const HDeviceInfo& obj2);

private:

    QSharedDataPointer<HDeviceInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HDeviceInfo();

    /*!
     * Constructs a new instance from the specified parameters that the UDA
     * specification mandates for a UPnP device.
     *
     * The parameters the constructor expects are arguments defined in the
     * device description file and they are all mandatory for a valid UPnP device.
     *
     * \param deviceType specifies the device type.
     *
     * \param friendlyName specifies a short description for the end-user. This
     * cannot be empty and should be less than 64 characters.
     *
     * \param manufacturer specifies the name of the manufacturer. This cannot
     * be empty and should be less than 64 characters.
     *
     * \param modelName specifies the model name. This cannot be empty and
     * should be less than 32 characters.
     *
     * \param udn specifies the unique device name. This is a universally
     * unique identifier for the device, regardless if the device is root
     * or embedded. The specified UDN has to be valid.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified arguments. This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that contains
     * an error description in case the construction failed.
     * This parameter is optional.
     *
     * \remarks in case any of the provided arguments does not meet the
     * specified requirements, the created object is \e invalid.
     *
     * \sa isValid()
     */
    HDeviceInfo(
        const HResourceType& deviceType,
        const QString& friendlyName,
        const QString& manufacturer,
        const QString& modelName,
        const HUdn&    udn,
        HValidityCheckLevel checkLevel = StrictChecks,
        QString* err = 0);

    /*!
     * Constructs a new instance from the specified parameters. These are
     * all the arguments found in the device description file.
     *
     * \param deviceType specifies the device type.
     *
     * \param friendlyName specifies a short description for the end-user. This
     * cannot be empty and should be less than 64 characters.
     *
     * \param manufacturer specifies the name of the manufacturer. This cannot
     * be empty and should be less than 64 characters.
     *
     * \param manufacturerUrl specifies the web site for the manufacturer.
     *
     * \param modelDescription specifies the long description for the end user.
     * This can be empty and should be less than 128 characters.
     *
     * \param modelName specifies the model name. This cannot be empty and
     * should be less than 32 characters.
     *
     * \param modelNumber specifies the model number of the device. There is
     * no format specified. This should be less than 32 characters.
     *
     * \param modelUrl specifies the web site for the device model.
     *
     * \param serialNumber specifies the serial number of the device. No
     * format specified. This should be less than 64 characters.
     *
     * \param udn specifies the unique device name. This is a universally
     * unique identifier for the device, regardless if the device is root
     * or embedded. The specified UDN has to be valid.
     *
     * \param upc specifies the Universal Product Code, which is 12-digit,
     * all-numeric code that identifies the consumer package.
     * Managed by the Uniform Code Council.
     *
     * \param icons specifies the icons of the device, if any.
     *
     * \param presentationUrl specifies the URL for HTML-based user interface
     * for controlling and/or viewing device status.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified arguments. This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that contains
     * an error description in case the construction failed. This is optional.
     *
     * \remarks in case any of the provided arguments does not meet the
     * specified requirements, the created object is \e invalid.
     *
     * \sa isValid()
     */
    HDeviceInfo(
        const HResourceType& deviceType,
        const QString& friendlyName,
        const QString& manufacturer,
        const QUrl&    manufacturerUrl,
        const QString& modelDescription,
        const QString& modelName,
        const QString& modelNumber,
        const QUrl&    modelUrl,
        const QString& serialNumber,
        const HUdn&    udn,
        const QString& upc,
        const QList<QUrl>& icons,
        const QUrl&    presentationUrl,
        HValidityCheckLevel checkLevel = StrictChecks,
        QString* err = 0);

    /*!
     * \brief Destroys the instance.
     */
    ~HDeviceInfo();

    /*!
     * Copies the contents of the other to this.
     *
     * \param other specifies the object to be copied.
     */
    HDeviceInfo(const HDeviceInfo& other);

    /*!
     * Assigns the contents of the other to this.
     *
     * \param other specifies the object to be copied.
     */
    HDeviceInfo& operator=(const HDeviceInfo& other);

    /*!
     * \brief Indicates if the object is valid.
     *
     * A valid object contains the mandatory data of a device description.
     *
     * \param level specifies the level of strictness used in validating
     * the object. This parameter is optional and the default level is strict.
     *
     * \return \e true in case the object is valid.
     */
    bool isValid(HValidityCheckLevel level) const;

    /*!
     * \brief Sets the URL for the web site of the manufacturer.
     *
     * \param arg specifies the URL for the web site of the manufacturer.
     *
     * \sa manufacturerUrl()
     */
    void setManufacturerUrl(const QUrl& arg);

    /*!
     * \brief Sets the model description.
     *
     * A model description is used to display a long description for end user.
     * Should be < 128 characters.
     *
     * \param arg specifies the model description.
     *
     * \sa modelDescription()
     */
    void setModelDescription(const QString& arg);

    /*!
     * \brief Sets the model number.
     *
     * There is no format specified for the model number,
     * other than it should be < 32 characters.
     *
     * \param arg specifies the model number.
     *
     * \sa modelNumber()
     */
    void setModelNumber(const QString& arg);

    /*!
     * \brief Sets the URL for the web site of the model.
     *
     * \param arg specifies the model URL.
     *
     * \sa modelUrl()
     */
    void setModelUrl(const QUrl& arg);

    /*!
     * \brief Sets the serial number of the device.
     *
     * There is no format specified for the serial number,
     * other than it should be < 64 characters.
     *
     * \param arg specifies the serial number.
     *
     * \sa serialNumber()
     */
    void setSerialNumber(const QString& arg);

    /*!
     * \brief Sets the Universal Product Code.
     *
     * UPC is a 12-digit, all-numeric code that
     * identifies the consumer package. Managed by the Uniform Code Council.
     *
     * \param arg specifies the UPC.
     *
     * \sa upc()
     */
    void setUpc(const QString& arg);

    /*!
     * \brief Sets the icons of the device.
     *
     * \param arg specifies the icons of the device.
     *
     * \sa icons()
     */
    void setIcons(const QList<QUrl>& arg);

    /*!
     * \brief Sets the presentation URL.
     *
     * Presentation URL specifies the URL for HTML-based user interface
     * for controlling and/or viewing device status.
     *
     * \param arg specifies the presentation URL.
     *
     * \sa presentationUrl()
     */
    void setPresentationUrl(const QUrl& arg);

    /*!
     * \brief Returns the type of the device found in the device description file.
     *
     * \return The type of the device found in the device description file.
     */
    const HResourceType& deviceType() const;

    /*!
     * \brief Returns short description for end user.
     *
     * \return short description for end user.
     */
    QString friendlyName() const;

    /*!
     * \brief Returns manufacturer's name.
     *
     * \return manufacturer's name.
     */
    QString manufacturer() const;

    /*!
     * \brief Returns the manufacturer's web site.
     *
     * \return The manufacturer's web site.
     *
     * \sa setManufacturerUrl()
     */
    QUrl manufacturerUrl() const;

    /*!
     * \brief Returns long description for end user.
     *
     * \return long description for end user.
     *
     * \sa setModelDescription()
     */
    QString modelDescription() const;

    /*!
     * \brief Returns the model name.
     *
     * \return The model name.
     */
    QString modelName() const;

    /*!
     * \brief Returns the model number.
     *
     * \return The model number.
     *
     * \sa setModelNumber()
     */
    QString modelNumber() const;

    /*!
     * \brief Returns the web site for the device model.
     *
     * \return The web site for the device model.
     *
     * \sa setModelUrl()
     */
    QUrl modelUrl() const;

    /*!
     * \brief Returns the serial number.
     *
     * \return The serial number.
     *
     * \sa setSerialNumber()
     */
    QString serialNumber() const;

    /*!
     * \brief Returns the Unique Device Name.
     *
     * \return Universally-unique identifier for the device, whether root or embedded.
     *
     * \remarks the UDN is same over time for a specific device instance.
     */
    const HUdn& udn() const;

    /*!
     * \brief Returns the Universal Product Code.
     *
     * \return The Universal Product Code.
     *
     * \sa setUpc()
     */
    QString upc() const;

    /*!
     * \brief Returns the icons of the device, if any.
     *
     * \return The icons of the device.
     *
     * \sa setIcons()
     */
    QList<QUrl> icons() const;

    /*!
     * \brief Returns the location of the device's presentation page.
     *
     * \return The location of the device's presentation page.
     *
     * \sa setPresentationUrl()
     */
    QUrl presentationUrl() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HDeviceInfo
 */
H_UPNP_CORE_EXPORT bool operator==(const HDeviceInfo&, const HDeviceInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HDeviceInfo
 */
inline bool operator!=(const HDeviceInfo& obj1, const HDeviceInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}

#endif /* HDEVICEINFO_H_ */
