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

#ifndef HCONTROLPOINT_CONFIGURATION_H_
#define HCONTROLPOINT_CONFIGURATION_H_

#include <HUpnpCore/HClonable>

class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HControlPointConfigurationPrivate;

/*!
 * Class for specifying initialization information to HControlPoint instances.
 *
 * \brief This class is used to pass initialization information for HControlPoint
 * instances. The use of this is optional and an HControlPoint instance is perfectly
 * functional with the default configuration.
 *
 * However, you can configure an HControlPoint in following ways:
 * - Define whether an HControlPoint should subscribe to events when a
 * device has been discovered by using setSubscribeToEvents().
 * By default an HControlPoint instances subscribes to all events.
 * - Set the timeout request for event subscriptions with setDesiredSubscriptionTimeout().
 * The default is 30 minutes.
 * - Specify whether an HControlPoint should perform initial discovery upon
 * startup using setAutoDiscovery(). The default is yes.
 * - Specify the network addresses an HControlPoint should use in its operations
 * with setNetworkAddressesToUse().
 * The default is the first found interface that is up. Non-loopback interfaces
 * have preference, but if none are found the loopback is used. However, in this
 * case UDP multicast is not available.
 *
 * \headerfile hcontrolpoint_configuration.h HControlPointConfiguration
 *
 * \ingroup hupnp_devicehosting
 *
 * \sa HControlPoint
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HControlPointConfiguration :
    public HClonable
{
H_DISABLE_COPY(HControlPointConfiguration)
friend class HControlPoint;

protected:

    //
    // Documented in HClonable
    virtual void doClone(HClonable* target) const;

    //
    // Documented in HClonable
    virtual HControlPointConfiguration* newInstance() const;

protected:

    HControlPointConfigurationPrivate* h_ptr;
    HControlPointConfiguration(HControlPointConfigurationPrivate& dd);

public:

    /*!
     * \brief Creates a new instance.
     *
     * Creates a new instance with default values.
     */
    HControlPointConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HControlPointConfiguration();

    //
    // Documented in HClonable
    virtual HControlPointConfiguration* clone() const;

    /*!
     * \brief Indicates whether to automatically subscribe to all events on all services
     * of a device when a new device is added into the control of an HControlPoint.
     *
     * \return \e true in case the HControlPoint instance should subscribe to all
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
     * \brief Indicates whether the control point should perform discovery upon
     * initialization.
     *
     * \return \e true in case the the control point should perform discovery upon
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
     * Defines whether a control point should automatically subscribe to all
     * events on all services of a device when a new device is added
     * into the control of an HControlPoint.
     *
     * \param subscribeAutomatically when \e true an HControlPoint instance
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
     * \param arg when \e true an HControlPoint instance will perform discovery
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

}
}

#endif /* HCONTROLPOINT_CONFIGURATION_H_ */
