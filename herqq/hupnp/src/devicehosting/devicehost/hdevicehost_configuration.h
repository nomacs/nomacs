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

#ifndef HDEVICEHOST_CONFIGURATION_H_
#define HDEVICEHOST_CONFIGURATION_H_

#include <HUpnpCore/HClonable>
#include <HUpnpCore/HDeviceModelCreator>

class QString;
class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HDeviceConfigurationPrivate;

/*!
 * This is a class for specifying a configuration to an HServerDevice that is
 * to be created and hosted by an HDeviceHost.
 *
 * A valid device configuration contains at least a path to a
 * device description file. See setPathToDeviceDescription().
 *
 * The other options available in this class affect the runtime behavior of a
 * HDeviceHost in regard to the HServerDevice instance that is created based
 * on the pathToDeviceDescription().
 *
 * \headerfile hdevicehost_configuration.h HDeviceConfiguration
 *
 * \ingroup hupnp_devicehosting
 *
 * \sa HDeviceHostConfiguration, HDeviceHost, HDeviceHost::init(), HServerDevice
 */
class H_UPNP_CORE_EXPORT HDeviceConfiguration :
    public HClonable
{
H_DISABLE_COPY(HDeviceConfiguration)

protected:

    HDeviceConfigurationPrivate* h_ptr;

    //
    // Documented in HClonable
    virtual void doClone(HClonable* target) const;

    //
    // Documented in HClonable
    virtual HDeviceConfiguration* newInstance() const;

public:

    /*!
     * Default constructor.
     *
     * \brief Creates a new, empty instance.
     */
    HDeviceConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HDeviceConfiguration();

    //
    // Documented in HClonable
    virtual HDeviceConfiguration* clone() const;

    /*!
     * \brief Sets the path to the UPnP device description.
     *
     * \param pathToDeviceDescription specifies the path to the UPnP
     * device description.
     *
     * \remarks The provided path or the device description document is not
     * validated in anyway. The device description validation occurs during the
     * initialization of the HDeviceHost.
     */
    void setPathToDeviceDescription(const QString& pathToDeviceDescription);

    /*!
     * \brief Returns the path to the device description.
     *
     * \return The path to the device description.
     */
    QString pathToDeviceDescription() const;

    /*!
     * \brief Sets the maximum age of presence announcements and discovery responses
     * in seconds.
     *
     * \param maxAge specifies the maximum age of presence announcements
     * and discovery messages. If a value smaller than 5 is specified,
     * the max age is set to 5. If positive value larger than a day is specified,
     * the max age is set to a day (60*60*24). The default is 1800 seconds,
     * which equals to 30 minutes.
     *
     * \attention the UDA instructs this value to be at least 30 minutes.
     */
    void setCacheControlMaxAge(qint32 maxAge=1800);

    /*!
     * \brief Returns the maximum age of presence announcements and discovery
     * responses in seconds.
     *
     * If the cache control max age has not been explicitly set,
     * the return value is 1800.
     *
     * \return The maximum age of presence announcements and discovery
     * responses in seconds.
     */
    qint32 cacheControlMaxAge() const;

    /*!
     * \brief Indicates whether or not the object contains the necessary details
     * for hosting an HServerDevice class in a HDeviceHost.
     *
     * \retval true in case the object contains the necessary details
     * for hosting an HServerDevice class in a HDeviceHost.
     *
     * \retval false otherwise. In this case, the initialization of HDeviceHost
     * cannot succeed. Make sure you have set the pathToDeviceDescription().
     *
     * \sa pathToDeviceDescription()
     */
    bool isValid() const;
};

class HDeviceHostConfigurationPrivate;

/*!
 * \brief This class is used to specify one or more device configurations to an
 * HDeviceHost instance and to configure the functionality of the HDeviceHost
 * that affect every hosted HServerDevice.
 *
 * The initialization of an HDeviceHost requires a valid host configuration.
 * A valid \e host \e configuration contains at least one \e device
 * \e configuration and a <em>device model creator</em>, as otherwise the host
 * would have nothing to do and no means to create UPnP device and service objects.
 *
 * The initialization of an HDeviceHost follows roughly these steps:
 *
 * - Create an HDeviceHostConfiguration instance.
 * - Set the device model creator using setDeviceModelCreator().
 * - Create and setup one or more HDeviceConfiguration instances.
 * - Add the device configurations to the HDeviceHostConfiguration instance
 * using add().
 * - Modify the behavior of the HDeviceHost by setting other variables
 * of this class.
 * - Create an HDeviceHost and initialize it by passing the
 * HDeviceHostConfiguration to its HDeviceHost::init() method.
 *
 * Besides specifying the device configurations, you can configure an HDeviceHost
 * in following ways:
 * - Specify how many times each resource advertisement is sent with
 * setIndividualAdvertisementCount(). The default is 2.
 * - Specify the timeout for event subscriptions with
 * setSubscriptionExpirationTimeout(). The default is 0, which means that
 * an HDeviceHost respects the subscription timeouts requested by control points
 * as long as the requested values are less than a day.
 * - Specify the network addresses an HDeviceHost should use in its operations
 * with setNetworkAddressesToUse().
 * The default is the first found interface that is up. Non-loopback interfaces
 * have preference, but if none are found the loopback is used. However, in this
 * case UDP multicast is not available.
 *
 * \headerfile hdevicehost_configuration.h HDeviceHostConfiguration
 *
 * \ingroup hupnp_devicehosting
 *
 * \sa HDeviceConfiguration, HDeviceHost
 */
class H_UPNP_CORE_EXPORT HDeviceHostConfiguration :
    public HClonable
{
H_DISABLE_COPY(HDeviceHostConfiguration)

protected:

    HDeviceHostConfigurationPrivate* h_ptr;

    //
    // Documented in HClonable
    virtual void doClone(HClonable* target) const;

    //
    // Documented in HClonable
    virtual HDeviceHostConfiguration* newInstance() const;

public:

    /*!
     * Default constructor.
     *
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty(), isValid()
     */
    HDeviceHostConfiguration();

    /*!
     * \brief Creates a new instance.
     *
     * Creates an instance with a single device configuration. This is a convenience
     * method.
     *
     * \sa isEmpty(), isValid()
     */
    HDeviceHostConfiguration(const HDeviceConfiguration&);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HDeviceHostConfiguration();

    //
    // Documented in HClonable
    virtual HDeviceHostConfiguration* clone() const;

    /*!
     * Adds a device configuration.
     *
     * \param deviceConfiguration specifies the device configuration to be added.
     * The configuration is added only if it is valid,
     * see HDeviceConfiguration::isValid().
     *
     * \return \e true in case the configuration was added. Only valid
     * HDeviceConfiguration instances are added,
     * see HDeviceConfiguration::isValid().
     */
    bool add(const HDeviceConfiguration& deviceConfiguration);

    /*!
     * Removes device configurations.
     *
     * \remarks This method removes the device configurations, but it does not
     * reset other set attributes to their default values.
     */
    void clear();

    /*!
     * \brief Returns the currently stored device configurations.
     *
     * \return The currently stored device configurations. The returned list
     * contains pointers to const device configuration objects this instance
     * owns. The ownership of the objects is \b not transferred.
     */
    QList<const HDeviceConfiguration*> deviceConfigurations() const;

    /*!
     * \brief Indicates how many times the device host sends each individual
     * advertisement / announcement.
     *
     * The default value is 2.
     *
     * \return how many times the device host sends each individual
     * advertisement / announcement.
     *
     * \sa setIndividualAdvertisementCount()
     */
    qint32 individualAdvertisementCount() const;

    /*!
     * \brief Returns the network addresses a device host should use in its
     * operations.
     *
     * \return The network addresses a device host should use in its
     * operations.
     *
     * \sa setNetworkAddressesToUse()
     */
    QList<QHostAddress> networkAddressesToUse() const;

    /*!
     * \brief Returns the timeout the device host uses for subscriptions.
     *
     * The default value is zero, which means that the device host honors the
     * timeouts requested by control points up to a day. Larger values are
     * set to a day.
     *
     * \return The timeout in seconds the device host uses for subscriptions.
     *
     * \sa setSubscriptionExpirationTimeout()
     */
    qint32 subscriptionExpirationTimeout() const;

    /*!
     * \brief Returns the device model creator the HDeviceHost should use
     * to create HServerDevice instances.
     *
     * \return The device model creator the HDeviceHost should use
     * to create HServerDevice instances.
     *
     * \sa setDeviceModelCreator()
     */
    HDeviceModelCreator* deviceModelCreator() const;

    /*!
     * \brief Returns the device model info provider the HDeviceHost should use to
     * validate device model components.
     *
     * \return The device model info provider the HDeviceHost should use to
     * validate device model components.
     */
    HDeviceModelInfoProvider* deviceModelInfoProvider() const;

    /*!
     * \brief Sets the device model creator the HDeviceHost should use
     * to create HServerDevice instances.
     *
     * \param creator specifies the device model creator the HDeviceHost should use
     * to create HServerDevice instances.
     *
     * \sa deviceModelCreator()
     */
    void setDeviceModelCreator(const HDeviceModelCreator& creator);

    /*!
     * \brief Sets the device model info provider the HDeviceHost should use to
     * validate device model components.
     *
     * \param infoProvider specifies the device model info provider the
     * HDeviceHost should use to validate device model components.
     */
    void setDeviceModelInfoProvider(const HDeviceModelInfoProvider& infoProvider);

    /*!
     * \brief Specifies how many times the device host sends each individual
     * advertisement / announcement.
     *
     * By default, each advertisement is sent twice.
     *
     * \param count specifies how many times the device host sends each individual
     * advertisement / announcement. If the provided value is smaller than 1 the
     * advertisement count is set to 1.
     *
     * \sa individualAdvertisementCount()
     */
    void setIndividualAdvertisementCount(qint32 count);

    /*!
     * \brief Specifies the timeout the device host uses for subscriptions.
     *
     * The default value is zero, which means that the device host honors the
     * timeouts requested by control points.
     *
     * \param timeout specifies the desired timeout in seconds.
     * - If timeout is greater than
     * zero the device host will use the timeout as such for subscriptions.
     * - If timeout is zero the device host will honor the timeout requested
     * by control points.
     * - If timeout is negative the subscription timeout is set to a day.
     *
     * \note the maximum expiration timeout value is a day. Larger values are
     * set to a day. This applies to the timeout requests made by control points
     * as well.
     *
     * \sa subscriptionExpirationTimeout()
     */
    void setSubscriptionExpirationTimeout(qint32 timeout);

    /*!
     * Defines the network addresses the device host should use in its
     * operations.
     *
     * \param addresses specifies the network addresses the device host
     * should use in its operations.
     *
     * \sa networkAddressesToUse()
     */
    bool setNetworkAddressesToUse(const QList<QHostAddress>& addresses);

    /*!
     * \brief Indicates if the instance contains any device configurations.
     *
     * \return \e true in case the instance contains no device configurations.
     * In this case the object cannot be used to initialize an HDeviceHost.
     *
     * \sa isValid()
     */
    bool isEmpty() const;

    /*!
     * \brief Indicates if the object is valid, i.e it can be used to initialize
     * an HDeviceHost instance.
     *
     * \return \e true if the object is valid. A valid object is not empty and
     * its deviceModelCreator() is set.
     */
    bool isValid() const;
};

}
}

#endif /* HDEVICEHOST_CONFIGURATION_H_ */
