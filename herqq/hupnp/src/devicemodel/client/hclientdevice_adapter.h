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

#ifndef HCLIENTDEVICE_ADAPTER_H_
#define HCLIENTDEVICE_ADAPTER_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

class HClientDeviceAdapterPrivate;

/*!
 * This is an abstract base class for classes that compose HClientDevice instances
 * and provide alternate interfaces for interacting with HClientDevices.
 *
 * \headerfile hclientdevice_adapter.h HClientDeviceAdapter
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HClientDeviceAdapter :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HClientDeviceAdapter)

protected:

    HClientDeviceAdapterPrivate* h_ptr;

    /*!
     * \brief Creates a new instance.
     *
     * \param deviceType specifies the UPnP device type this adapter can handle.
     * Note, every HClientDevice to be associated with this adapter has to be
     * of the specified device type.
     *
     * \param parent specifies the parent \c QObject.
     */
    HClientDeviceAdapter(const HResourceType& deviceType, QObject* parent = 0);

    //
    // \internal
    //
    HClientDeviceAdapter(
        HClientDeviceAdapterPrivate& dd, QObject* parent = 0);

    /*!
     * Prepares the instance to be associated with the specified HClientDevice.
     *
     * \param device specifies the device to be associated.
     *
     * \return \e true if the instance was successfully prepared to be used
     * with the specified HClientDevice. If \e false is returned the specified
     * HClientDevice was not associated with this instance and
     * isReady() will return \e false.
     */
    virtual bool prepareDevice(HClientDevice* device);

    /*!
     * \brief Specifies the device model info provider the adapter should use to validate
     * HClientDevice instances with which this instance is associated.
     *
     * \param infoProvider specifies the device model info provider to be used.
     */
    void setDeviceModelInfoProvider(const HDeviceModelInfoProvider&);

    /*!
     * \brief Returns the device model info provider the adapter uses to validate
     * HClientDevice instances with which this instance is associated.
     *
     * \return The device model info provider the adapter uses to validate
     * HClientDevice instances with which this instance is associated.
     */
    const HDeviceModelInfoProvider* deviceModelInfoProvider() const;

    /*!
     * Specifies a human readable error description.
     *
     * \param arg specifies a human readable error description.
     *
     * \sa lastErrorDescription()
     */
    void setLastErrorDescription(const QString& arg);

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HClientDeviceAdapter() = 0;

    /*!
     * \brief This enumeration specifies the different methods of validating
     * HClientDevice instances.
     */
    enum ValidationType
    {
        /*!
         * Only the device type should be checked to be correct.
         *
         * \sa deviceType()
         */
        MinimalValidation,

        /*!
         * The device type is checked and the device should be validated
         * using the deviceModelInfoProvider().
         */
        FullValidation
    };

    /*!
     * Associates the specified \c HClientDevice with this instance.
     *
     * \param device specifies the HClientDevice to be associated with this instance.
     *
     * \return \e true in case the \c device was successfully associated with
     * this instance.
     *
     * \sa device()
     */
    bool setDevice(HClientDevice* device, ValidationType vt = FullValidation);

    /*!
     * \brief Returns a human readable description of the last error occurred.
     *
     * \return a human readable description of the last error occurred.
     */
    QString lastErrorDescription() const;

    /*!
     * \brief Returns the \c HClientDevice associated with this instance.
     *
     * \return The \c HClientDevice associated with this instance, if any.
     * A null pointer is returned in case setDevice() has not been called
     * successfully.
     *
     * \sa setDevice()
     */
    HClientDevice* device() const;

    /*!
     * \brief Indicates if the object is ready to be used.
     *
     * \return \e true if the object is ready to be used.
     */
    bool isReady() const;

    /*!
     * \brief Returns the device type this adapter abstracts.
     *
     * \return The device type this adapter abstracts.
     */
    const HResourceType& deviceType() const;
};

}
}

#endif /* HCLIENTDEVICE_ADAPTER_H_ */
