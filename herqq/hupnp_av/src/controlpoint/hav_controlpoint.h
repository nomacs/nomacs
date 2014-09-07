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

#ifndef HAV_CONTROLPOINT_H_
#define HAV_CONTROLPOINT_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClientDevice>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAvControlPointPrivate;

/*!
 * \brief A class for discovering and interacting with UPnP Media Renderers and
 * Media Servers in the network.
 *
 * \headerfile hav_controlpoint.h HAvControlPoint
 *
 * \ingroup hupnp_av_cp
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HControlPoint
 */
class H_UPNP_AV_EXPORT HAvControlPoint :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HAvControlPoint)
H_DECLARE_PRIVATE(HAvControlPoint)

public:

    /*!
     * \brief This enumeration specifies error codes some of the methods of
     * \c %HAvControlPoint may return.
     */
    enum HAvControlPointError
    {
        /*!
         * General failure or no error.
         *
         * This error code is used to indicate that either:
         * - the exact cause for an error could not be determined or
         * - no error has occurred.
         */
        UndefinedError = 0,

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
         * networking component.
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

protected:

    HAvControlPointPrivate* h_ptr;

    //
    // \internal
    //
    HAvControlPoint(HAvControlPointPrivate& dd, QObject* parent = 0);

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
    const HAvControlPointConfiguration* configuration() const;

    /*!
     * \brief Sets the type and description of the last occurred error.
     *
     * \param error specifies the error type.
     * \param errorDescr specifies a human readable description of the error.
     *
     * \sa error(), errorDescription()
     */
    void setError(HAvControlPointError error, const QString& errorDescr);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject, if any.
     *
     * \remarks The created control point creates and uses the default
     * configuration.
     */
    HAvControlPoint(QObject* parent = 0);

    /*!
     * \brief Creates a new instance.
     *
     * \param configuration specifies information that can be used to modify the
     * default behavior of the control point instance. If you want to use
     * the default configuration, you should use the default constructor.
     *
     * \param parent specifies the parent \c QObject, if any.
     *
     * \sa HAvControlPointConfiguration
     */
    HAvControlPoint(
        const HAvControlPointConfiguration& configuration, QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAvControlPoint();

     /*!
     * Initializes the control point.
     *
     * This has to be called for the control point to start
     * monitoring the network for UPnP A/V devices. To stop an initialized
     * control point instance from listening network events you can call
     * quit() or delete the object.
     *
     * \note
     * By default an \c %HAvControlPoint instance performs a device
     * discovery upon initialization. However, you can override this
     * in the configuration.
     *
     * \return \e true if the initialization of the control point succeeded.
     * If \e false is returned you can call error() to get the type of the error,
     * and you can call errorDescription() to get a human-readable description
     * of the error.
     *
     * \sa isStarted(), quit(), error(), errorDescription()
     */
    bool init();

    /*!
     * \brief Returns the type of the last error occurred.
     *
     * \return The type of the last error occurred.
     *
     * \sa errorDescription()
     */
    HAvControlPointError error() const;

    /*!
     * \brief Returns a human readable description of the last error occurred.
     *
     * \return a human readable description of the last error occurred.
     *
     * \sa error()
     */
    QString errorDescription() const;

    /*!
     * \brief Indicates whether or not the control point is successfully started.
     *
     * \return \e true in case the control point is successfully started.
     *
     * \sa init()
     */
    bool isStarted() const;

    /*!
     * \brief Returns a list of UPnP Media Renderers the control point is currently
     * managing.
     *
     * \return a list of UPnP Media Renderers the control point is currently
     * managing.
     *
     * \warning
     * \li The returned objects will be deleted at the latest when the
     * control point is being destroyed.
     *
     * \li The ownership of an HMediaRendererAdapter is \b never transferred.
     * Do \b not delete these objects directly.
     *
     * \remarks
     * \li This method does not perform a network scan.
     *
     * \li You can call removeMediaRenderer() to remove and delete a media
     * renderer.
     *
     * \sa removeMediaRenderer(), mediaRenderer()
     */
    const HMediaRendererAdapters& mediaRenderers() const;

    /*!
     * \brief Returns a UPnP Media Renderer with the specified UDN.
     *
     * \param udn specifies the Unique Device Name of the Media Renderer.
     *
     * \return a UPnP Media Renderer with the specified UDN.
     *
     * \warning
     * \li The returned object will be deleted at the latest when the
     * control point is being destroyed.
     *
     * \li The ownership of an HMediaRendererAdapter is \b never transferred.
     * Do \b not delete the returned object directly.
     *
     * \remarks
     * \li This method does not perform a network scan.
     *
     * \li You can call removeMediaRenderer() to remove and delete a media renderer.
     *
     * \sa removeMediaRenderer(), mediaRenderers()
     */
    HMediaRendererAdapter* mediaRenderer(const HUdn& udn);

    /*!
     * \brief Returns a list of UPnP Media Servers the control point is currently
     * managing.
     *
     * \return a list of UPnP Media Servers the control point is currently
     * managing.
     *
     * \warning
     * \li The returned objects will be deleted at the latest when the
     * control point is being destroyed.
     *
     * \li The ownership of an HMediaServerAdapter is \b never transferred.
     * Do \b not delete these objects directly.
     *
     * \remarks
     * \li This method does not perform a network scan.
     *
     * \li You can call removeMediaServer() to remove and delete a media server.
     *
     * \sa removeMediaServer(), mediaServer()
     */
    const HMediaServerAdapters& mediaServers() const;

    /*!
     * \brief Returns a UPnP Media Server with the specified UDN.
     *
     * \param udn specifies the Unique Device Name of the Media Server.
     *
     * \return a UPnP Media Server with the specified UDN.
     *
     * \warning
     * \li The returned object will be deleted at the latest when the
     * control point is being destroyed.
     *
     * \li The ownership of an HMediaServerAdapter is \b never transferred.
     * Do \b not delete the returned object directly.
     *
     * \remarks
     * \li This method does not perform a network scan.
     *
     * \li You can call removeMediaServer() to remove and delete a media server.
     *
     * \sa removeMediaServer(), mediaServers()
     */
    HMediaServerAdapter* mediaServer(const HUdn& udn);

    /*!
     * Removes a Media Server from the control point and deletes it.
     *
     * \param mediaServer specifies the Media Server to be removed.
     * Nothing is done if the object is not in the control of this control point.
     *
     * \retval true in case the object was successfully removed and deleted.
     * \retval false in case:
     * - the specified argument was null or
     * - the specified object is not managed by this control point.
     *
     * \remarks
     * The specified object is deleted if and only if the method returns \e true.
     *
     * \sa error(), errorDescription(), removeMediaRenderer()
     */
    bool removeMediaServer(HMediaServerAdapter* mediaServer);

    /*!
     * Removes a Media Renderer from the control point and deletes it.
     *
     * \param mediaRenderer specifies the Media Renderer to be removed.
     * Nothing is done if the object is not in the control of this control point.
     *
     * \retval true in case the object was successfully removed and deleted.
     * \retval false in case:
     * - the specified argument was null or
     * - the specified object is not managed by this control point.
     *
     * \remarks
     * The specified object is deleted if and only if the method returns \e true.
     *
     * \sa error(), errorDescription(), removeMediaServer()
     */
    bool removeMediaRenderer(HMediaRendererAdapter* mediaRenderer);

public Q_SLOTS:

    /*!
     * Shuts down the control point.
     *
     * The control point stops listening for network events,
     * deletes all the objects it is hosting and cancels all event subscriptions.
     * In essence, the control point purges it state. You can re-initialize the
     * control point by calling init() again.
     *
     * \attention Every object managed by this instance will be deleted.
     * Be sure not to use these objects after calling this method.
     *
     * \sa init()
     */
    void quit();

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when a Media Renderer device has been
     * discovered.
     *
     * \param renderer is an adapter object for using the discovered Media
     * Renderer device.
     *
     * \remarks The discovered device may already be managed by this instance.
     * This is the case when a device goes offline and comes back online before
     * it is removed from the control point.
     *
     * \sa mediaRendererOffline(), removeMediaRenderer()
     */
    void mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter* renderer);

    /*!
     * \brief This signal is emitted when a Media Server device has been
     * discovered.
     *
     * \param server is an adapter object for using the discovered Media Server
     * device.
     *
     * \remarks The discovered device may already be managed by this instance.
     * This is the case when a device goes offline and comes back online before
     * it is removed from the control point.
     *
     * \sa mediaServerOffline(), removeMediaServer()
     */
    void mediaServerOnline(Herqq::Upnp::Av::HMediaServerAdapter* server);

    /*!
     * \brief This signal is sent when a Media Renderer device has announced
     * that it is going offline or the expiration timeout associated with the
     * device has elapsed.
     *
     * After a Media Renderer has gone offline you may want to remove the
     * device from the control point using removeMediaRenderer().
     * Alternatively, if you do not remove the device and the device comes back
     * online later:
     *
     * \li mediaRendererOnline() will be sent if the configuration of the
     * device hasn't changed, or
     *
     * \li a mediaRendererInvalidated() signal is emitted in case the device uses
     * a different configuration as it did before going offline. If this is the case
     * you should remove the device as it no longer reflects the real device
     * accurately.
     *
     * \param renderer refers to the Media Renderer device that went offline
     * and is not reachable at the moment.
     *
     * \sa mediaRendererOnline(), mediaServerOnline(),
     * mediaRendererInvalidated()
     */
    void mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter* renderer);

    /*!
     * \brief This signal is sent when a Media Server device has announced
     * that it is going offline or the expiration timeout associated with the
     * device has elapsed.
     *
     * After a Media Server has gone offline you may want to remove the
     * device from the control point using removeMediaServer().
     * Alternatively, if you do not remove the device and the device comes back
     * online later:
     *
     * \li mediaServerOnline(); will be sent if the configuration of the
     * device hasn't changed, or
     *
     * \li a mediaServerInvalidated() signal is emitted in case the device uses
     * a different configuration as it did before going offline. If this is the case
     * you should remove the device as it no longer reflects the real device
     * accurately.
     *
     * \param server refers to the Media Server device that went offline
     * and is not reachable at the moment.
     *
     * \sa mediaServerOnline(), mediaRendererOnline(), mediaServerInvalidated()
     */
    void mediaServerOffline(Herqq::Upnp::Av::HMediaServerAdapter* server);

    /*!
     * This signal is emitted when a previously discovered device has changed
     * its configuration and must be discarded.
     *
     * \param renderer is the device that has been invalidated.
     */
    void mediaRendererInvalidated(Herqq::Upnp::Av::HMediaRendererAdapter* renderer);

    /*!
     * This signal is emitted when a previously discovered device has changed
     * its configuration and must be discarded.
     *
     * \param server is the device that has been invalidated.
     */
    void mediaServerInvalidated(Herqq::Upnp::Av::HMediaServerAdapter* server);
};

}
}
}

#endif /* HAV_CONTROLPOINT_H_ */
