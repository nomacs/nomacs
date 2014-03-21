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

#ifndef HCONTROLPOINT_H_
#define HCONTROLPOINT_H_

#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HResourceType>

#include <QtCore/QObject>

class QNetworkReply;
class QAuthenticator;

namespace Herqq
{

namespace Upnp
{

class HControlPointPrivate;
class HControlPointConfiguration;

/*!
 * \brief This is a class for discovering and interacting with UPnP devices in the network.
 *
 * \headerfile hcontrolpoint.h HControlPoint
 *
 * \ingroup hupnp_devicehosting
 *
 * According to the UPnP Device Architecture specification, a control point is an
 * entity, which <em> "retrieves device and service descriptions, sends actions to
 * services, polls for service state variables, and receives events from services" </em>.
 * In other words, a UPnP control point discovers UPnP devices, queries their state,
 * listens their asynchronous events and invokes them to perform actions. A
 * control point is the \em client in the UPnP architecture, whereas a UPnP
 * device is the \em server.
 *
 * \c %HControlPoint does all of the above, but mostly hiding it from the user if
 * the user wishes so.
 * To discover UPnP devices all you need to do is create an instance of
 * \c %HControlPoint, initialize it by calling init() and check if devices are
 * already found by calling rootDevices(), devices() or device().
 * You can also listen for a number of events, such as:
 * - HControlPoint::rootDeviceOnline(), which is emitted when a UPnP
 * device has become available on the network.
 * - HControlPoint::rootDeviceOffline(), which is emitted when a UPnP device in
 * control of the control point has gone offline.
 * - HControlPoint::rootDeviceRemoved(), which is emitted when a control point has
 * removed and deleted an HClientDevice. Note, an HClientDevice is never deleted without
 * an explicit request from a user. See removeRootDevice() for further information.
 *
 * Consider an example:
 *
 * \code
 *
 * // myclass.h

 * #include <HUpnpCore/HControlPoint>
 *
 * class MyClass :
 *     public QObject
 * {
 * Q_OBJECT
 *
 * private:
 *
 *     Herqq::Upnp::HControlPoint* m_controlPoint;
 *
 * private slots:
 *
 *     void rootDeviceOnline(Herqq::Upnp::HClientDevice*);
 *     void rootDeviceOffline(Herqq::Upnp::HClientDevice*);
 *
 * public:
 *
 *     MyClass(QObject* parent = 0);
 * };
 *
 * // myclass.cpp
 *
 * #include "myclass.h"
 * #include <HUpnpCore/HClientDevice>
 *
 * MyClass::MyClass(QObject* parent) :
 *     QObject(parent), m_controlPoint(new Herqq::Upnp::HControlPoint(this))
 * {
 *     connect(
 *         m_controlPoint,
 *         SIGNAL(rootDeviceOnline(Herqq::Upnp::HClientDevice*)),
 *         this,
 *         SLOT(rootDeviceOnline(Herqq::Upnp::HClientDevice*)));
 *
 *     connect(
 *         m_controlPoint,
 *         SIGNAL(rootDeviceOffline(Herqq::Upnp::HClientDevice*)),
 *         this,
 *         SLOT(rootDeviceOffline(Herqq::Upnp::HClientDevice*)));
 *
 *     if (!m_controlPoint->init())
 *     {
 *         // the initialization failed, perhaps you should do something?
 *         // for starters, you can call error() to check the error type and
 *         // errorDescription() for a human-readable description of
 *         // the error that occurred.
 *         return;
 *     }
 *
 *     // the control point is running and any standard-compliant UPnP device
 *     // on the same network should now be discoverable.
 *
 *     // remember also that the current thread has to have an event loop
 * }
 *
 * void MyClass::rootDeviceOnline(Herqq::Upnp::HClientDevice* newDevice)
 * {
 *     // device discovered, should something be done with it? Perhaps we want
 *     // to learn something from it:
 *     Herqq::Upnp::HDeviceInfo info = newDevice->info();
 *     // do something with the info object
 * }
 *
 * void MyClass::rootDeviceOffline(Herqq::Upnp::HClientDevice* rootDevice)
 * {
 *     // device announced that it is going away and the control point sends
 *     // a notification of this. However, the device isn't removed from the
 *     // control point until explicitly requested:
 *
 *     m_controlPoint->removeRootDevice(rootDevice);
 * }
 *
 * \endcode
 *
 * Once you have obtained a pointer to a HClientDevice you can
 * enumerate its services, invoke its actions, listen for events of changed
 * state and so on. Basically, a root \c %HClientDevice object at the control
 * point side is an entry point to a very accurate object model depicting the
 * real root UPnP device that has been discovered. For more information about
 * the \c %HClientDevice and the object model, see the page detailing the
 * HUPnP \ref hupnp_devicemodel.
 *
 * You can call quit() to stop an initialized control point instance from listening
 * the network and to clear its state.
 *
 * \remarks
 * \li This class has thread affinity. You have to use it and destroy it in the
 * thread in which it is located.
 * \li You can use \c QObject::moveToThread() on the \c %HControlPoint, which causes
 * the control point and every object managed by it to be moved to the chosen thread.
 * However, you cannot move individual objects managed by \c %HControlPoint.
 * \li a control point never transfers the ownership of the HClientDevice objects it manages.
 * \li <b>%HControlPoint always destroys every %HClientDevice it manages when it is
 * being destroyed</b>.
 *
 * \warning See notes about object deletion in ~HControlPoint().
 *
 * \sa HClientDevice, HClientDevices, hupnp_devicemodel
 */
class H_UPNP_CORE_EXPORT HControlPoint :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HControlPoint)
H_DECLARE_PRIVATE(HControlPoint)

protected:

    /*!
     * \brief This enumeration specifies the actions to take when a device has been
     * discovered.
     *
     * \sa acceptRootDevice()
     */
    enum DeviceDiscoveryAction
    {
        /*!
         * Ignores the device.
         *
         * In case the discovered device is a new device
         * this value instructs the control point to ignore and delete it.
         *
         * In case the discovered device is already in the control of the control
         * point this value instructs the control point to ignore it. However,
         * the device is not removed from the control point. To do that you have
         * to call removeRootDevice().
         */
        IgnoreDevice,

        /*!
         * Adds a new device into the control point and retains an existing device
         * in the control point.
         *
         * In case the discovered device is a new device the new device is
         * added into the \c %HControlPoint. Otherwise the device is already in
         * the control point and nothing is done.
         *
         * The control point will not subscribe to events.
         */
        AddDevice,

        /*!
         * Adds the device into the control point and subscribes to evented services
         * according to the configuration of the control point.
         *
         * In case the discovered device is a new device the new device is
         * added into the \c %HControlPoint. Otherwise the device is already in
         * the control point and nothing is done in this regard.
         *
         * The control point determines whether to subscribe to events based on
         * its configuration. Default configuration instructs the control point
         * to subscribe to all events. The same is done if no configuration was
         * provided.
         */
        AddDevice_SubscribeEventsIfConfigured,

        /*!
         * Adds the device into the control point and subscribes to all
         * evented services.
         *
         * In case the discovered device is a new device the new device is
         * added into the \c %HControlPoint. Otherwise the device is already in
         * the control point and nothing is done in this regard.
         *
         * The control points subscribes to all evented services contained by the
         * device and its embedded devices.
         */
        AddDevice_SubscribeAllEvents
    };

public:

    /*!
     * \brief This enumeration specifies error types some of the methods of
     * \c %HControlPoint may return.
     */
    enum ControlPointError
    {
        /*!
         * General failure or no error.
         *
         * This error code is used to indicate that either:
         * - the exact cause for an error could not be determined or
         * - no error has occurred.
         */
        UndefinedError,

        /*!
         * Control point is not initialized.
         *
         * This error code is used to indicate that the control point has not been
         * initialized.
         */
        NotInitializedError,

        /*!
         * Control point is already initialized.
         *
         * This error code is used to indicate that the control point is already
         * initialized.
         */
        AlreadyInitializedError,

        /*!
         * Networking error.
         *
         * This error code is used to indicate that an error occurred in some
         * networking component, such as in HTTP server or in SSDP module.
         */
        CommunicationsError,

        /*!
         * Argument error.
         *
         * This error code is used to indicate that a member function
         * was called with an invalid argument and the call was aborted.
         */
        InvalidArgumentError
    };

    /*!
     * \brief This enumeration is used to describe the status of an event subscription.
     *
     * \sa subscriptionStatus()
     */
    enum SubscriptionStatus
    {
        /*!
         * No subscription exists for the specified service.
         *
         * This value is used when there is no subscription or subscription attempt
         * being made to a specified service.
         */
        Unsubscribed,

        /*!
         * A subscription attempt is in progress.
         *
         * This value is used when a subscription attempt to the events of the
         * specified service is in progress.
         *
         * \sa subscriptionSucceeded(), subscriptionFailed()
         */
        Subscribing,

        /*!
         * A subscription is active.
         *
         * This value is used when the control point has successfully
         * subscribed to the events of the specified service.
         */
        Subscribed
    };

    /*!
     *
     */
    /*enum ErrorType
    {
    };*/

private:

    /*!
     * Performs the initialization of a derived class.
     *
     * The \c %HControlPoint uses two-phase initialization in which the user
     * first constructs an instance and then calls init() in order to ready
     * the object for use. This method is called by the \c %HControlPoint
     * during its private initialization after all the private data structures
     * are constructed but before any network activity. At this point, no HTTP
     * or SSDP requests are served.
     *
     * You can override this method to perform any further initialization of a
     * derived class.
     *
     * \return \e true if and only if the initialization succeeded.
     * If \e false is returned the initialization of the control point is
     * aborted. In addition, it is advised that you call setError()
     * before returning.
     *
     * \remarks the default implementation does nothing.
     *
     * \sa init()
     */
    virtual bool doInit();

    /*!
     * Performs the de-initialization of a derived class.
     *
     * Since it is possible to shutdown a control point without actually destroying the
     * instance by calling quit(), derived classes have the possibility to
     * run their own shutdown procedure by overriding this method.
     * This method is called \b before the \c %HControlPoint cleans its
     * private data structures but after it has stopped listening requests
     * from the network.
     *
     * \remarks the default implementation does nothing.
     *
     * \sa quit()
     */
    virtual void doQuit();

    /*!
     * This method specifies the actions to take when a device has been
     * discovered.
     *
     * A discovered device may be a new device to the control point
     * or a device that is already in the control point. The latter scenario is
     * true when a device goes offline, is not removed from the
     * control point and later comes back online with the same UPnP configuration
     * (see UDA for more information about a UPnP device configuration).
     *
     * If you have derived a class from HControlPoint you have the option of
     * choosing whether a discovered device should be managed by the
     * control point, and whether the control point should subscribe to the events
     * published by the services of the device. To do this you have to
     * override this method.
     *
     * \note
     * - This method takes precedence over any configuration options provided
     * to the control point at the time of its construction
     * - This method is called when:
     *   - a new root HClientDevice has been built and
     *   - a previously known device comes back online with the same UPnP device
     *   configuration value it had before it went offline.
     *
     * \param device specifies the discovered device.
     *
     * \return value indicating what should be done with the specified device.
     *
     * By default every successfully built device will be added to the control
     * point and its events are subscribed according to the control point
     * configuration.
     *
     * \sa DeviceDiscoveryAction()
     */
    virtual DeviceDiscoveryAction acceptRootDevice(HClientDevice* device);

    /*!
     * This method is called whenever a new a device has been detected on
     * the network.
     *
     * Override this method in case you want to control which devices get built.
     *
     * Every UPnP \e resource belongs to a UPnP device tree, and every \e advertisement
     * and \e notification of a \e resource contains all the information needed
     * to build a full model of the device tree. An \e advertisement
     * is sent by a UPnP device to advertise itself, its embedded devices or
     * any of the services contained within the device tree. A \e notification
     * is the response from a UPnP device to a discovery sent by a control point.
     *
     * If an advertisement or a notification is received that identifies a resource
     * belonging to a device that is currently not in the control of this control point,
     * this method is called. If you accept the specified resource the
     * control point will retrieve all the information from the target UPnP device
     * to build a device model of the device tree the UPnP device represents.
     *
     * \note once you have accepted a resource from a particular UPnP device,
     * this method will not be called again for other resource advertisements or
     * notifications from the UPnP device. On the other hand, if you do not accept
     * a resource and the same UPnP device sends another
     * notification or advertisement, this method will be called again.
     *
     * \param usn contains information about the resource.
     * \param source specifies the source of the advertisement.
     *
     * \return \e true when the resource is accepted and a device model should
     * be built for the UPnP device that sent the notification / advertisement.
     * If the device is built successfully the acceptRootDevice() will be called.
     * By default every new device is accepted, built and added into an
     * \c %HControlPoint.
     */
    virtual bool acceptResource(
        const HDiscoveryType& usn, const HEndpoint& source);

protected:

    HControlPointPrivate* h_ptr;

    //
    // \internal
    //
    HControlPoint(
        HControlPointPrivate& dd,
        const HControlPointConfiguration* config = 0, QObject* parent = 0);

    /*!
     * \brief Returns the configuration used to initialize the control point.
     *
     * \return The configuration used to initialize the control point.
     *
     * \note If no configuration was provided at the time of object construction
     * the control point creates a default configuration and uses that.
     * This method \b always returns a pointer to a valid object, even if the
     * control point is not initialized.
     *
     * \remarks the returned object is not a copy and the ownership of the
     * object is \b not transferred. Do \b not delete the object.
     */
    const HControlPointConfiguration* configuration() const;

    /*!
     * \brief Sets the type and description of the last occurred error.
     *
     * \param error specifies the error type.
     * \param errorDescr specifies a human readable description of the error.
     *
     * \sa error(), errorDescription()
     */
    void setError(ControlPointError error, const QString& errorDescr);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject, if any.
     *
     * \sa HControlPointConfiguration
     *
     * \remarks the created control point creates and uses a default
     * configuration.
     */
    HControlPoint(QObject* parent = 0);

    /*!
     * \brief Creates a new instance.
     *
     * \param configuration specifies information that can be used to modify the
     * default behavior of the control point instance. If you want to use
     * the default configuration, you should use the default constructor.
     *
     * \param parent specifies the parent \c QObject, if any.
     *
     * \sa HControlPointConfiguration
     */
    HControlPoint(
        const HControlPointConfiguration& configuration, QObject* parent = 0);

    /*!
     * Destroys the control point and every hosted device.
     *
     * \warning When a control point is being destroyed the control point
     * destroys all of its child objects. You should discard any pointers retrieved
     * from the control point to prevent accidental dereference.
     *
     * \remarks An \c %HControlPoint instance has to be destroyed in the thread
     * in which it is located.
     */
    virtual ~HControlPoint();

    /*!
     * Initializes the control point.
     *
     * This has to be called for the control point to start
     * monitoring the network for UPnP devices. To stop an initialized control point
     * instance from listening network events you can call quit() or delete
     * the object.
     *
     * \note
     * By default an \c %HControlPoint instance performs a device
     * discovery upon initialization. However, you can override this
     * in the configuration.
     *
     * \return \e true if the initialization of the control point succeeded.
     * If \e false is returned you can call error() to get the type of the error,
     * and you can call errorDescription() to get a human-readable description of the error.
     *
     * \sa quit(), error(), errorDescription()
     */
    bool init();

    /*!
     * \brief Returns the type of the last error occurred.
     * \return The type of the last error occurred.
     */
    ControlPointError error() const;

    /*!
     * \brief Returns a human readable description of the last error occurred.
     * \return a human readable description of the last error occurred.
     */
    QString errorDescription() const;

    /*!
     * \brief Indicates whether or not the host is successfully started.
     *
     * \return true in case the host is successfully started.
     */
    bool isStarted() const;

    /*!
     * \brief Returns a device with the specified Unique Device Name that the
     * control point is currently managing.
     *
     * \param udn specifies the Unique Device Name of the desired device.
     * \param deviceType specifies whether the search should consider root
     * devices only. The default is to search root devices only.
     *
     * \return The device with the specified Unique Device Name, or a
     * null pointer in case no currently managed device has the
     * specified UDN.
     *
     * \warning the returned device will be deleted at the latest when the
     * control point is being destroyed. In addition, you can call
     * removeRootDevice() to remove and delete a \b root device. However, the call
     * will fail if you pass it an embedded device. Moreover, do not delete a
     * device object directly. The ownership of an HClientDevice is \b never
     * transferred.
     *
     * \remarks This method does not perform a network scan. The search is run
     * against the devices that are already in the control of the control point.
     * You can call scan() to perform an explicit network scan.
     */
    HClientDevice* device(
        const HUdn& udn,
        TargetDeviceType deviceType = RootDevices) const;

    /*!
     * \brief Returns a list of UPnP root devices the control point is currently managing.
     *
     * The returned list contains pointers to root HClientDevice objects that are currently
     * managed by this instance.
     *
     * \return a list of pointers to root HClientDevice objects the control point
     * is currently managing.
     *
     * \warning the returned devices will be deleted at the latest when the
     * control point is being destroyed. In addition, you can call
     * removeRootDevice() to remove and delete a root device. However, do not delete
     * the device objects directly. The ownership of an HClientDevice is \b never
     * transferred.
     *
     * \remarks This method does not perform a network scan.
     */
    HClientDevices rootDevices() const;

    /*!
     * \brief Returns a list of UPnP devices the control point is currently managing and
     * that match the provided search criteria.
     *
     * The returned list contains pointers to HClientDevice objects that are currently
     * managed by this instance. It is important to note that unlike the method
     * rootDevices() this method may return pointers to both root and
     * embedded devices.
     *
     * \param deviceType specifies the UPnP device type to be searched.
     * \param versionMatch specifies how the version information in argument
     * \c deviceType should be used. The default is <em>inclusive match</em>,
     * which essentially means that any device with a device type version that
     * is \b less than or \b equal to the version specified in argument
     * \c deviceType is successfully matched.
     * \param deviceTypes specifies whether the search should consider root
     * devices only. The default is to search root devices only.
     *
     * \return a list of pointers to HClientDevice objects the control point
     * is currently managing.
     *
     * \warning the returned devices will be deleted at the latest when the
     * control point is being destroyed. In addition, you can call
     * removeRootDevice() to remove and delete a \b root device. However, the call
     * will fail if you pass it an embedded device. Moreover, do not delete the
     * device objects directly. The ownership of an HClientDevice is \b never transferred.
     *
     * \remarks This method does not perform a network scan. The search is run
     * against the devices that are already in the control of the control point.
     * You can call scan() to perform an explicit network scan.
     */
    HClientDevices devices(
        const HResourceType& deviceType,
        HResourceType::VersionMatch versionMatch = HResourceType::Inclusive,
        TargetDeviceType deviceTypes = RootDevices);

    /*!
     * Removes the root device from the control point and deletes it.
     *
     * \param rootDevice specifies the root device to be removed. Nothing is done if
     * the device is not in the control of this control point or the device is
     * not a root device.
     *
     * \retval true in case the device was successfully removed and deleted.
     * \retval false in case:
     * - the specified argument was null,
     * - the specified device is not managed by this control point or
     * - the specified device is not a root device.
     *
     * \remarks
     * the specified object is deleted if and only if the method returns \e true.
     *
     * \sa error(), errorDescription()
     */
    bool removeRootDevice(HClientDevice* rootDevice);

    /*!
     * Subscribes to events of the specified services contained by the
     * specified device.
     *
     * You can use this method to subscribe to events of multiple evented services
     * contained by the specified device at once.
     *
     * \param device specifies the device that contains the services, which
     * events are subscribed.
     *
     * \param visitType specifies how the device tree is traversed. A subscription
     * is sent to every service of every visited device.
     *
     * \retval true in case the subscriptions were successfully
     * dispatched. Note, \b any subscription may still fail. You can connect to
     * subscriptionSucceeded() and subscriptionFailed() signals to be notified
     * upon subscription success and failure.
     *
     * \retval false in case the specified argument
     * is null or it does not belong to a device held by the control point.
     *
     * \remarks
     * - services which events are already subscribed to are skipped.
     * - the method returns immediately. Every successful subscription results
     * in subscriptionSucceeded() signal sent. Every failed subscription results in
     * subscriptionFailed() signal sent.
     * - every subscription is maintained until:
     *     - an error occurs
     *     - it is explicitly canceled
     *
     * Thus, <b>a subscription is automatically renewed before expiration until
     * an error occurs or it is canceled</b>.
     *
     * \sa error(), errorDescription()
     */
    bool subscribeEvents(
        HClientDevice* device, DeviceVisitType visitType);

    /*!
     * Subscribes to the events of the service.
     *
     * \param service specifies the service
     *
     * \retval true in case the subscription request was successfully
     * dispatched. Note, the subscription \b may still fail. You can connect to
     * subscriptionSucceeded() and subscriptionFailed() signals to be notified
     * upon subscription success and failure.
     *
     * \retval false in case the specified argument:
     * - is null,
     * - it does not belong to a device held by the control point,
     * - is not evented or
     * - there already exists a subscription for the specified service.
     *
     * \remarks
     * - the method returns immediately. A successful subscription results
     * in subscriptionSucceeded() signal sent. A failed subscription results in
     * subscriptionFailed() signal sent.
     * - a subscription is maintained by the control point until:
     *     - an error occurs
     *     - it is explicitly canceled
     *
     * Thus, <b>a subscription is automatically renewed before expiration until
     * an error occurs or it is canceled</b>.
     *
     * \sa error(), errorDescription()
     */
    bool subscribeEvents(HClientService* service);

    /*!
     * Checks if there exists a subscription to the events of the specified service.
     *
     * \param service specifies the service to be checked.
     *
     * \retval HControlPoint::Unsubscribed when the service is not
     * evented or there is no active susbcription or subscription attempt going
     * on to the specified service.
     *
     * \retval HControlPoint::Subscribing when the service is
     * evented and an subscription attempt is being made to the specified service.
     *
     * \retval HControlPoint::Subscribed when there exists an active
     * subscription to the specified service.
     */
    SubscriptionStatus subscriptionStatus(const HClientService* service) const;

    /*!
     * Cancels the event subscriptions of every service contained by the device.
     *
     * Any services which events are not subscribed are skipped.
     *
     * \param device specifies the device that contains the services, which
     * subscriptions are canceled.
     *
     * \param visitType specifies how the device tree is traversed.
     * A subscription cancellation request is sent to every service of
     * every visited device.
     *
     * \retval true in case the cancellation request of a subscription
     * was successfully dispatched. Note, this does not mean that the cancellation
     * was successfully received and handled by the UPnP device in question.
     * Upon success the state of the control point instance is modified
     * appropriately, but it is never guaranteed that the
     * target UPnP device receives or processes the cancellation request.
     *
     * \retval false in case
     * - the specified argument is null,
     * - the device is not managed by this control point or
     * - there were no active subscriptions matching the search criteria
     * to cancel.
     *
     * \remarks This method returns immediately.
     *
     * \sa error(), errorDescription()
     */
    bool cancelEvents(HClientDevice* device, DeviceVisitType visitType);

    /*!
     * Cancels the event subscription of the service.
     *
     * \param service specifies the service, which event subscription is
     * to be canceled.
     *
     * \retval true in case the cancellation request of a subscription
     * was successfully dispatched. Note, this does not mean that the cancellation
     * was successfully received and handled by the UPnP device in question.
     * Upon success the state of the control point instance is modified
     * appropriately, but it is never guaranteed that the
     * target UPnP device receives or processes the cancellation request.
     *
     * \retval false in case
     * - the specified argument is null,
     * - the service does not belong to a device held by the control point or
     * - there is no active subscription to the specified service.
     *
     * \remarks This method returns immediately.
     *
     * \sa error(), errorDescription()
     */
    bool cancelEvents(HClientService* service);

    /*!
     * Scans the network for resources of interest.
     *
     * Using the default configuration \c %HControlPoint automatically searches
     * and adds every device it finds. In that case the device list returned
     * by rootDevices() usually reflects the UPnP device status of the network.
     * However, there are situations where you may want to explicitly ask
     * the \c %HControlPoint to update its status and you can call this method
     * to do that.
     *
     * \param discoveryType specifies the type of the discovery to perform.
     * \param count specifies how many times the discovery should be performed.
     * The number translates directly to the number of SSDP messages send. The
     * default is 1.
     *
     * \remarks
     * \li This method returns immediately.
     * \li As a result of this call there may be any number of rootDeviceOnline()
     * signals emitted as a consequence of newly found devices.
     * \li The call will not affect the expiration of existing devices.
     * More specifically, any device that does not respond
     * to the scan will not be considered as expired and no rootDeviceOffline()
     * signals will be sent consequently.
     */
    bool scan(const HDiscoveryType& discoveryType, qint32 count = 1);

     /*!
     * Scans the network for resources of interest.
     *
     * Using the default configuration \c %HControlPoint automatically searches
     * and adds every device it finds. In that case the device list returned
     * by rootDevices() usually reflects the UPnP device status of the network.
     * However, there are situations where you may want to explicitly ask
     * the \c %HControlPoint to perform a discovery on a specific TCP/IP
     * endpoint. In other words, you can perform a unicast device discovery using
     * this method.
     *
     * \param discoveryType specifies the type of the discovery to perform.
     * \param destination specifies the location where the discovery is sent.
     * If the port of the specified endpoint is set to zero the message is sent
     * to the specified host address using the default port 1900.
     * \param count specifies how many times the discovery should be performed.
     * The number translates directly to the number of SSDP messages send. The
     * default is 1.
     *
     * \remarks
     * \li This method returns immediately.
     * \li As a result of this call there may be any number of rootDeviceOnline()
     * signals emitted as a consequence of newly found devices.
     * \li The call will not affect the expiration of existing devices.
     * More specifically, any device that does not respond
     * to the scan will not be considered as expired and no rootDeviceOffline()
     * signals will be sent consequently.
     */
    bool scan(
        const HDiscoveryType& discoveryType, const HEndpoint& destination,
        qint32 count = 1);

public Q_SLOTS:

    /*!
     * Shuts down the control point.
     *
     * The control point stops listening for network events,
     * deletes all the devices it is hosting and cancels all event subscriptions.
     * In essence, the control point purges it state. You can re-initialize the
     * control point by calling init() again.
     *
     * \attention Every pointer to object retrieved from this instance will be
     * deleted. Be sure not to use any such pointer after calling this method.
     *
     * \sa init()
     */
    void quit();

Q_SIGNALS:

    /*!
     * \brief This signal is emitted whenever a final server requests authentication
     * before it delivers the requested contents.
     *
     * \brief This signal is relayed from the underlying \c QNetworkAccessManager,
     * which is used by the \c %HControlPoint to run HTTP messaging.
     * As specified in the reference documentation of QNetworkAccessManager,
     * <em>the slot connected to this signal should fill the credentials
     * for the contents (which can be determined by inspecting the reply object)
     * in the authenticator object. \c QNetworkAccessManager will cache the
     * credentials internally and will send the same values if the server
     * requires authentication again, without emitting the authenticationRequired() signal.
     * If it rejects the credentials, this signal will be emitted again.</em>
     *
     * \param reply specifies the requested contents.
     *
     * \param authenticator specifies the authenticator object to which the
     * user should fill in the credentials.
     */
    void authenticationRequired(
        QNetworkReply* reply, QAuthenticator* authenticator);

    /*!
     * \brief This signal is emitted when the initial subscription to the specified
     * service succeeded.
     *
     * \param service specifies the target service of the event subscription.
     *
     * \sa subscriptionFailed()
     */
    void subscriptionSucceeded(Herqq::Upnp::HClientService* service);

    /*!
     * \brief This signal is emitted when an event subscription to the specified
     * service failed.
     *
     * \note this signal may be emitted in three different scenarios:
     * - the initial subscription failed
     * - an automatic subscription renewal failed
     * - a re-subscription failed
     * If you want to try to re-subscribe to the service you can
     * call subscribe() again.
     *
     * \param service specifies the service, which event subscription
     * failed.
     *
     * \sa subscriptionSucceeded()
     */
    void subscriptionFailed(Herqq::Upnp::HClientService* service);

    /*!
     * \brief This signal is emitted when an event subscription to the specified
     * service has been canceled.
     *
     * \param service specifies the service, which subscription was canceled.
     */
    void subscriptionCanceled(Herqq::Upnp::HClientService* service);

    /*!
     * \brief This signal is emitted when a device has been discovered.
     *
     * \param device is the discovered device.
     *
     * \remarks the discovered device may already be managed by this instance.
     * This is the case when a device goes offline and comes back online before
     * it is removed from the control point.
     *
     * \sa rootDeviceOffline(), removeRootDevice()
     */
    void rootDeviceOnline(Herqq::Upnp::HClientDevice* device);

    /*!
     * \brief This signal is sent when a root device has announced that it is going
     * offline or the expiration timeout associated with the device tree has elapsed.
     *
     * After a device has gone offline you may want to remove the device from the
     * control point using removeRootDevice(). Alternatively, if you do not remove the
     * device and the device comes back online later:
     *
     * \li a rootDeviceOnline() signal is emitted in case the device uses the
     * same configuration as it did before going offline or
     *
     * \li a rootDeviceInvalidated() signal is emitted in case the device uses
     * a different configuration as it did before going offline. If this is the case
     * you should remove the device as it no longer reflects the real device
     * accurately.
     *
     * \param device is the device that went offline and is not reachable
     * at the moment.
     *
     * \sa rootDeviceOnline(), rootDeviceInvalidated(), removeRootDevice()
     */
    void rootDeviceOffline(Herqq::Upnp::HClientDevice* device);

    /*!
     * \brief This signal is emitted when a previously discovered device has changed its
     * configuration and must be discarded.
     *
     * The UDA v1.1 specifies the configuration of a root device to consist of
     * the device description documents of all the devices in the device tree and
     * all the service description documents of the services in the device tree.
     * If the configuration changes the old device tree has to be discarded in
     * place of the new one.
     *
     * After this signal is emitted the specified HClientDevice object has become
     * invalid and should be discarded and removed immediately.
     * In addition, rootDeviceOnline() signal may be emitted shortly after this
     * signal, but only when the new configuration of the device is accepted
     * by this instance.
     *
     * \param device is the device that has been invalidated.
     */
    void rootDeviceInvalidated(Herqq::Upnp::HClientDevice* device);

    /*!
     * \brief This signal is emitted when a root device has been removed from the control
     * of this control point and deleted.
     *
     * \param deviceInfo specifies information about the device that was removed
     * and deleted.
     *
     * \sa rootDeviceOffline()
     */
    void rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo& deviceInfo);

    /*!
     * \brief This signal is emitted when a run-time error has occurred.
     *
     * \param err specifies the type of the error that occurred
     * \param errStr specifies a human-readable description of the error that
     * occurred.
     */
    //void error(ErrorType err, const QString& errStr);
};

}
}

#endif /* HCONTROLPOINT_H_ */
