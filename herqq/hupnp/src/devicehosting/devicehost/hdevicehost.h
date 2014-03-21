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

#ifndef HDEVICEHOST_H_
#define HDEVICEHOST_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

class HDeviceHostPrivate;

/*!
 * \brief This is a class for creating and hosting \c %HServerDevice instances on the network.
 *
 * \headerfile hdevicehost.h HDeviceHost
 *
 * \ingroup hupnp_devicehosting
 *
 * As the name implies, this is the class in the HUPnP library
 * used to expose UPnP devices to UPnP control points.
 * The class \e hosts instances of HServerDevice, which means that the class takes
 * care of all of the UPnP mechanics detaching the HServerDevice from it. This
 * separation leaves the HServerDevice to model the UPnP device structure and to
 * focus on the functionality of the specific device type. This is what the
 * HUPnP \ref hupnp_devicemodel is all about.
 *
 * Hosting a device is simple, assuming you have the necessary device and service
 * descriptions ready and the HUPnP device and service classes implemented.
 * Basically, you only need to:
 *
 * \li instantiate an HDeviceConfiguration for each UPnP device type to be hosted and
 * pass them to the \c %HDeviceHost inside a HDeviceHostConfiguration instance
 * \li instantiate and initialize an \c %HDeviceHost
 * \li make sure a Qt event loop is present in the thread in which the
 * \c %HDeviceHost is run.
 *
 * As an example, consider the following:
 *
 * \code
 *
 * // myclass.h
 *
 * #include "my_hdevice.h" // your code containing the type MyHDevice
 *
 * #include <HUpnpCore/HDeviceHost>
 * #include <HUpnpCore/HDeviceModelCreator>
 *
 * #include <QtCore/QObject>
 *
 * class MyClass :
 *     public QObject
 * {
 * Q_OBJECT
 *
 * private:
 *     Herqq::Upnp::HDeviceHost* m_deviceHost;
 *
 * public:
 *     MyClass(QObject* parent = 0);
 * };
 *
 * class MyCreator : public Herqq::Upnp::HDeviceModelCreator
 * {
 *
 * private:
 *
 *   // overridden from HDeviceModelCreator
 *   virtual MyCreator* newInstance() const;
 *
 * public:
 *
 *   // overridden from HDeviceModelCreator
 *   virtual MyHServerDevice* createDevice(
 *       const Herqq::Upnp::HDeviceInfo& info) const;
 *
 *   // overridden from HDeviceModelCreator
 *   virtual MyHServerService* createService(
 *       const Herqq::Upnp::HServiceInfo& serviceInfo,
 *       const Herqq::Upnp::HDeviceInfo& parentDeviceInfo) const;
 * };
 *
 * // myclass.cpp
 *
 * MyCreator* MyCreator::newInstance() const
 * {
 *     return new MyCreator();
 * }
 *
 * MyHServerDevice* MyCreator::createDevice(const Herqq::Upnp::HDeviceInfo& info) const
 * {
 *     if (info.deviceType().toString() == "urn:herqq-org:device:MyDevice:1")
 *     {
 *         return new MyHServerDevice();
 *     }
 *
 *     return 0;
 * }
 *
 * MyHServerService* MyCreator::createService(
 *     const Herqq::Upnp::HServiceInfo& serviceInfo,
 *     const Herqq::Upnp::HDeviceInfo& parentDeviceInfo) const
 * {
 *     if (serviceInfo.serviceType().toString() == "urn:herqq-org:service:MyService:1")
 *     {
 *         return new HMyServerService();
 *     }
 *
 *     // Note, parentDeviceInfo is not needed in this case, but there are
 *     // scenarios when it is mandatory to know information of the parent
 *     // device to create the correct HServerService type.
 *
 *     return 0;
 * }
 *
 * MyClass::MyClass(QObject* parent) :
 *     QObject(parent),
 *         m_deviceHost(new Herqq::Upnp::HDeviceHost(this))
 * {
 *     Herqq::Upnp::HDeviceHostConfiguration hostConf;
 *     hostConf.setDeviceModelCreator(MyCreator());
 *
 *     Herqq::Upnp::HDeviceConfiguration deviceConf;
 *     deviceConf.setPathToDeviceDescription("my_hdevice_devicedescription.xml");
 *
 *     hostConf.add(deviceConf);
 *
 *     if (!m_deviceHost->init(hostConf))
 *     {
 *         // The initialization failed, perhaps you should do something?
 *         // for starters, you can call error() to check the error type and
 *         // errorDescription() for a human-readable description of
 *         // the error that occurred.
 *         return;
 *     }
 *
 *     // The host is running and your device should now be accessible to
 *     // UPnP Control points until the host is destroyed (assuming the current
 *     // thread has an event loop).
 * }
 *
 * \endcode
 *
 * There are a few noteworthy issues in the example above.
 *
 * -# The device host will fail to initialize if your HDeviceHostConfiguration
 * instance is invalid; for instance, the \e device \e model \e creator is not
 * specified or any of the paths to your UPnP device or service description
 * documents are invalid. The point is, <b>you should always check the return
 * value</b>.
 * -# Your HServerDevice is accessible only as long as your \c %HDeviceHost
 * is alive. When the device host is destroyed every UPnP device it hosted
 * are destroyed as well.
 * -# \c %HDeviceHost requires an event loop to function.
 * -# The example above uses an \c %HDeviceHost instance to host a single UPnP root
 * device, but the same host could be used to host multiple UPnP root devices.
 * Certainly you can create multiple \c %HDeviceHost instances that each host a single
 * root HServerDevice within a thread, even sharing an event loop.
 * However, using a single \c %HDeviceHost for multiple root HServerDevice instances
 * reduces resource usage in various ways and makes all the configured UPnP
 * root devices accessible to you from the same \c %HDeviceHost instance.
 *
 * \remarks
 *
 * \li \c %HDeviceHost has thread affinity, which mandates
 * that the \c %HDeviceHost and any object managed by it must be destroyed in the
 * thread in which the \c %HDeviceHost at the time lives.
 * You can use <c>QObject::moveToThread()</c> on the \c %HDeviceHost, which causes
 * the device host and every object managed by it to be moved to the chosen thread.
 * However, you cannot move individual objects managed by \c %HDeviceHost.
 *
 * \li \c %HDeviceHost is the owner of the instances of
 * \c %HServerDevice it manages. It manages the memory of every object it has created.
 * In other words, a device host \b never transfers the ownership of the
 * HServerDevice objects it manages; <b>%HDeviceHost always destroys every
 * %HServerDevice it manages when it is being destroyed</b>.
 *
 * \sa hupnp_devicehosting, HServerDevice, HDeviceHostConfiguration,
 * HDeviceConfiguration
 */
class H_UPNP_CORE_EXPORT HDeviceHost :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HDeviceHost)
H_DECLARE_PRIVATE(HDeviceHost)
friend class HDeviceHostRuntimeStatus;

public:

    /*!
     * \brief Specifies return values that some of the methods of the class may return.
     */
    enum DeviceHostError
    {
        /*!
         * Return value signifying general failure.
         *
         * This return code is used when
         * an operation could not be successfully completed, but the exact
         * cause for the error could not be determined.
         */
        UndefinedError = -1,

        /*!
         * No error has occurred.
         */
        NoError = 0,

        /*!
         * Return value signifying that the device host is already successfully
         * initialized.
         */
        AlreadyInitializedError = 1,

        /*!
         * Return value signifying that the provided host configuration was incorrect.
         */
        InvalidConfigurationError = 2,

        /*!
         * Return value signifying that a provided device description document
         * was invalid.
         */
        InvalidDeviceDescriptionError = 3,

        /*!
         * Return value signifying that a provided service description document
         * was invalid.
         */
        InvalidServiceDescriptionError = 4,

        /*!
         * Return value used to indicate one or more more problems in communications
         * layer.
         *
         * For instance, perhaps the HTTP server or could the SSDP listener
         * could not be initialized.
         */
        CommunicationsError = 5,

        /*!
         * Return value used to indicate that the device host instance is not
         * property initiated.
         */
        NotStarted = 6,

        /*!
         * Return value used to indicate that operation failed due to a resource
         * conflict.
         *
         * For instance, trying to add more than one device with a same UDN
         * will fail because of this.
         */
        ResourceConflict = 7
    };

private:

    /*!
     * Performs the initialization of a derived class.
     *
     * The \c %HDeviceHost uses two-phase initialization in which the user
     * first constructs an instance and then calls init() in order to ready
     * the object for use. This method is called by the \c %HDeviceHost
     * during its private initialization after all the private data structures
     * are constructed, but before any network operations are performed. At this
     * point no HTTP or SSDP requests are served.
     *
     * You can override this method to perform any further initialization of a derived
     * class.
     *
     * \return \e true if and only if the initialization succeeded.
     * If \e false is returned the initialization of the device host is
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
     * Since it is possible to shutdown a device host without actually destroying the
     * instance by calling quit(), derived classes have the possibility to
     * run their own shutdown procedure by overriding this method.
     * This method is called \b before the \c %HDeviceHost cleans its
     * private data structures but after it has stopped listening requests
     * from the network.
     *
     * \remarks the default implementation does nothing.
     *
     * \sa quit()
     */
    virtual void doQuit();

    /*!
     * Checks if a (re-)subscription should be accepted.
     *
     * Derived classes can opt to override this method to decide what
     * event subscriptions are accepted and what are not.
     *
     * \param targetService specifies the target of the subscription.
     *
     * \param source specifies the location where the subscription came.
     *
     * \param isNew indicates the type of the subscription. The value is
     * \e true in case the subscription is new and \e false in case the
     * subscription is a renewal to an existing subscription.
     *
     * \return \e true in case the subscription should be accepted.
     *
     * \remarks by default all subscriptions are accepted.
     */
    virtual bool acceptSubscription(
        HServerService* targetService, const HEndpoint& source, bool isNew);

protected:

    HDeviceHostPrivate* h_ptr;

    /*!
     * \brief Returns the configuration used to initialize the device host.
     *
     * \return The configuration used to initialize the device host or null
     * in case the device host is not initialized.
     *
     * \remarks the returned object is not a copy and the ownership of the
     * object is not transferred. Do \b not delete it.
     */
    const HDeviceHostConfiguration* configuration() const;

    /*!
     * \brief Returns the object that details information of the status of a
     * device host.
     *
     * \return The object that details information of the status of a
     * device host.
     *
     * \remarks
     * \li A device host creates a single HDeviceHostRuntimeStatus object
     * during its construction and deletes it when the device host is being
     * deleted.
     * \li The returned object is always owned by the device host.
     */
    const HDeviceHostRuntimeStatus* runtimeStatus() const;

    /*!
     * \brief Sets the type and description of the last error occurred.
     *
     * \param error specifies the error type.
     * \param errorDescr specifies a human readable description of the error.
     *
     * \sa error(), errorDescription()
     */
    void setError(DeviceHostError error, const QString& errorDescr);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject.
     */
    explicit HDeviceHost(QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     *
     * Destroys the device host and every hosted device.
     */
    virtual ~HDeviceHost();

    /*!
     * \brief Returns a root device with the specified Unique Device Name.
     *
     * \param udn specifies the Unique Device Name of the desired root device.
     * \param target specifies the type of devices that are included in the
     * search.
     *
     * \return The root device with the specified Unique Device Name, or a
     * null pointer in case no currently managed root device has the
     * specified UDN.
     *
     * \warning the returned device will be deleted when the
     * device host is being destroyed. However, do \b not delete
     * the device object directly. The ownership of an HServerDevice is \b
     * never transferred.
     */
    HServerDevice* device(
        const HUdn& udn,
        TargetDeviceType target = RootDevices) const;

    /*!
     * \brief Returns a list of UPnP root devices the host is currently managing.
     *
     * The returned list contains pointers to root HServerDevice objects
     * that are currently hosted by this instance.
     *
     * \return a list of pointers to root HServerDevice objects that are
     * currently managed by the device host.
     *
     * \warning the returned HServerDevice instances will be deleted when the
     * device host is being destroyed. However, do \b not delete
     * the device objects directly. The ownership of an HServerDevice is \b never
     * transferred.
     */
    HServerDevices rootDevices() const;

    /*!
     * Initializes the device host and the devices it is supposed to host.
     *
     * \param configuration specifies the configuration for the instance. The
     * object has to contain at least one device configuration.
     *
     * \return \e true if the initialization of the device host succeeded.
     * If \e false is returned you can call error() to get the type of the error,
     * and you can call errorDescription() to get a human-readable description
     * of the error.
     *
     * \sa quit()
     */
    bool init(const HDeviceHostConfiguration& configuration);

    /*!
     * \brief Returns the type of the last error occurred.
     *
     * \return The type of the last error occurred.
     */
    DeviceHostError error() const;

    /*!
     * \brief Returns a human readable description of the last error occurred.
     *
     * \return a human readable description of the last error occurred.
     */
    QString errorDescription() const;

    /*!
     * \brief Indicates whether or not the host is successfully started.
     *
     * \return \e true in case the host is successfully started.
     */
    bool isStarted() const;

    /*!
     * Adds a new root device configuration to the device host.
     *
     * If the provided configuration is valid, the device host creates a new
     * device model instance, announces the new resource(s) to the network and adds
     * the device model available into the control of this instance.
     *
     * \param configuration specifies the new root device to add.
     *
     * \return \e true if a device model corresponding to the specified
     * configuration was created and added to the device host. If the method
     * returns \e false, you can call error() and errorDescription() to get
     * more information of the error that occurred.
     *
     * \remarks The specified device configuration has to be compatible with
     * the specified HDeviceHostConfiguration specified in init().
     *
     * \sa error(), errorDescription()
     */
    bool add(const HDeviceConfiguration& configuration);

public Q_SLOTS:

    /*!
     * Quits the device host and destroys the UPnP devices it is hosting. Note that
     * this is automatically called when the device host is destroyed.
     *
     * \attention Every pointer to object retrieved from this instance will be
     * deleted. Be sure not to use any such pointer after calling this method.
     *
     * \sa init()
     */
    void quit();
};

class HDeviceHostRuntimeStatusPrivate;

/*!
 * This is a class for detailing information of the runtime status of an
 * HDeviceHost instance.
 *
 * \headerfile hdevicehost.h HDeviceHostRuntimeStatus
 *
 * \ingroup hupnp_devicehosting
 *
 * \sa HDeviceHost
 */
class H_UPNP_CORE_EXPORT HDeviceHostRuntimeStatus
{
H_DISABLE_COPY(HDeviceHostRuntimeStatus)
friend class HDeviceHost;

protected:

    HDeviceHostRuntimeStatusPrivate* h_ptr;

    //
    // \internal
    //
    HDeviceHostRuntimeStatus(HDeviceHostRuntimeStatusPrivate& dd);

    /*!
     * \brief Creates an instance.
     *
     * Creates an instance.
     */
    HDeviceHostRuntimeStatus();

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HDeviceHostRuntimeStatus();

    /*!
     * \brief Returns the IP endpoints that the device host uses for SSDP communications.
     *
     * \return The IP endpoints that the device host uses for SSDP communications.
     */
    QList<HEndpoint> ssdpEndpoints() const;

    /*!
     * \brief Returns the IP endpoints that the device host uses for HTTP communications.
     *
     * \return The IP endpoints that the device host uses for HTTP communications.
     */
    QList<HEndpoint> httpEndpoints() const;
};

}
}

#endif /* HDEVICEHOST_H_ */
