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

#ifndef HAV_CONTROLPOINT_CONFIGURATION_H_
#define HAV_CONTROLPOINT_CONFIGURATION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClonable>

class QHostAddress;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAvControlPointConfigurationPrivate;

/*!
 * Class for specifying initialization information to HAvControlPoint instances.
 *
 * \brief This class is used to pass initialization information for HAvControlPoint
 * instances. The use of this is optional and an HAvControlPoint instance is
 * perfectly functional with the default configuration.
 *
 * However, you can configure an HAvControlPoint in following ways:
 * - Define what UPnP A/V device types the control point should look for.
 * - Define whether an HAvControlPoint should subscribe to events when a
 * device has been discovered by using setSubscribeToEvents().
 * By default an HAvControlPoint instances subscribes to all events.
 * - Set the timeout request for event subscriptions with setDesiredSubscriptionTimeout().
 * The default is 30 minutes.
 * - Specify whether an HAvControlPoint should perform initial discovery upon
 * startup using setAutoDiscovery(). The default is yes.
 * - Specify the network addresses an HAvControlPoint should use in its operations
 * with setNetworkAddressesToUse().
 * The default is the first found interface that is up. Non-loopback interfaces
 * have preference, but if none are found the loopback is used. However, in this
 * case UDP multicast is not available.
 *
 * \headerfile hav_controlpoint_configuration.h HAvControlPointConfiguration
 *
 * \ingroup hupnp_av_cp
 *
 * \sa HAvControlPoint
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAvControlPointConfiguration :
    public HClonable
{
H_DISABLE_COPY(HAvControlPointConfiguration)
H_DECLARE_PRIVATE(HAvControlPointConfiguration)

protected:

    // Documented in HClonable
    virtual void doClone(HClonable*) const;

    // Documented in HClonable
    virtual HAvControlPointConfiguration* newInstance() const;

    HAvControlPointConfigurationPrivate* h_ptr;

public:

    /*!
     * \brief This enumeration is used to specify the device types the HAvControlPoint
     * should search and use.
     */
    enum InterestingDeviceType
    {
        /*!
         * No device types should be accepted.
         */
        None = 0,

        /*!
         * Media Renderers should be accepted.
         */
        MediaRenderer = 0x01,

        /*!
         * Media Servers should be accepted.
         */
        MediaServer = 0x02,

        /*!
         * All device types should be accepted.
         *
         * \remarks This is the default value.
         */
        All = 0x1f
    };

    Q_DECLARE_FLAGS(InterestingDeviceTypes, InterestingDeviceType);

    /*!
     * \brief Creates a new instance.
     *
     * Creates a new instance with default values.
     */
    HAvControlPointConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAvControlPointConfiguration();

    /*!
     * \brief Returns a deep copy of the instance.
     *
     * \return a deep copy of the instance.
     *
     * \remarks
     * \li The ownership of the returned object is transferred to the caller.
     */
    virtual HAvControlPointConfiguration* clone() const;

    /*!
     * \brief Returns the device types the HAvControlPoint should search, accept and
     * use.
     *
     * \return The device types the HAvControlPoint should search, accept and
     * use.
     *
     * \sa setInterestingDeviceTypes()
     */
    InterestingDeviceTypes interestingDeviceTypes() const;

    /*!
     * \brief Indicates whether the control point should run discovery upon
     * initialization.
     *
     * \return \e true in case the the control point should run discovery upon
     * initialization. This is the default value.
     *
     * \remarks if the discovery is not performed the control point will be
     * unaware of UPnP devices that are already active in the network until they
     * re-advertise themselves.
     *
     * \sa setAutoDiscovery()
     */
    bool autoDiscovery() const;

    /*!
     * \brief Returns the network addresses a control point should use in its
     * operations.
     *
     * \return The network addresses a control point should use in its
     * operations.
     *
     * \sa setNetworkAddressesToUse()
     */
    QList<QHostAddress> networkAddressesToUse() const;

    /*!
     * \brief Indicates whether to automatically subscribe to all events on all services
     * of a device when a new device is added into the control of an HAvControlPoint.
     *
     * \return \e true in case the HAvControlPoint instance should subscribe to all
     * events of all services of a newly added device.
     *
     * \sa setSubscribeToEvents()
     */
    bool subscribeToEvents() const;

    /*!
     * \brief Returns the subscription timeout a control point requests when it subscribes
     * to an evented service.
     *
     * The default value is 30 minutes.
     *
     * \return The subscription timeout in seconds a control point requests
     * when it subscribes to an evented service.
     *
     * \sa setDesiredSubscriptionTimeout()
     */
    qint32 desiredSubscriptionTimeout() const;

    /*!
     * \brief Sets the device types the HAvControlPoint should search, accept and
     * use.
     *
     * \param types specifies the device types the HAvControlPoint should
     * search, accept and use.
     *
     * \sa interestingDeviceTypes()
     */
     void setInterestingDeviceTypes(InterestingDeviceTypes types) const;

    /*!
     * Defines whether a control point should automatically subscribe to all
     * events on all services of a device when a new device is added
     * into the control of an HAvControlPoint.
     *
     * \param subscribeAutomatically when \e true an HAvControlPoint instance
     * should by default subscribe to all events of all services of a newly added
     * device.
     *
     * \sa subscribeToEvents()
     */
    void setSubscribeToEvents(bool subscribeAutomatically);

    /*!
     * \brief Sets the subscription timeout a control point requests when it subscribes
     * to an evented service.
     *
     * Values less than or equal to zero are rejected and instead the default value
     * is used. The default value is 30 minutes.
     *
     * \param timeout specifies the requested timeout in seconds.
     *
     * \sa desiredSubscriptionTimeout()
     */
    void setDesiredSubscriptionTimeout(qint32 timeout);

    /*!
     * Defines whether the control point should perform discovery upon
     * initialization.
     *
     * \param arg when \e true an HAvControlPoint instance will perform discovery
     * when it is initialized. This is the default.
     *
     * \remarks if the discovery is not performed the control point will be
     * unaware of UPnP devices that are already active in the network until they
     * re-advertise themselves.
     *
     * \sa autoDiscovery()
     */
    void setAutoDiscovery(bool arg);

    /*!
     * Defines the network addresses the control point should use in its
     * operations.
     *
     * \param addresses specifies the network addresses the control point
     * should use in its operations.
     *
     * \return \e true in case the provided addresses are valid and can be
     * used.
     *
     * \sa networkAddressesToUse()
     */
    bool setNetworkAddressesToUse(const QList<QHostAddress>& addresses);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HAvControlPointConfiguration::InterestingDeviceTypes)

}
}
}

#endif /* HAV_CONTROLPOINT_CONFIGURATION_H_ */
