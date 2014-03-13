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

#ifndef HSSDP_H_
#define HSSDP_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QObject>

class QUrl;
class QString;
class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HSsdpPrivate;

/*!
 * \brief This class is used for sending and receiving SSDP messages defined by the
 * UPnP Device Architecture specification.
 *
 * Simple Service Discovery Protocol (SSDP) is an expired IETF Internet draft
 * on which the UPnP discovery mechanism is built. This class implements only the
 * SSDP functionality mandated by the UPnP Device Architecture specification.
 * \brief This class does not implement the SSDP draft in full.
 *
 * To use this class, you only need to instantiate it and connect to the
 * exposed signals to receive events when SSDP messages are received. You can also
 * derive a sub-class and override the various virtual member functions to handle
 * the received messages.
 *
 * \headerfile hssdp.h HSsdp
 *
 * \ingroup hupnp_ssdp
 *
 * \remarks
 * \li this class requires an event loop for listening incoming messages
 * \li this class has thread-affinity, which mandates that the instances of this
 * class has to be used in the thread in which they are located at the time.
 */
class H_UPNP_CORE_EXPORT HSsdp :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HSsdp)
H_DECLARE_PRIVATE(HSsdp)

public:

    /*!
     * \brief This enumeration specifies the different discovery methods the 
     * HSsdp class can run.
     */
    enum DiscoveryRequestMethod
    {
        /*!
         * This is the default multicast discovery supported both UDA v1.0 and 
         * UDA v1.1.
         */
        MulticastDiscovery,

        /*!
         * The unicast discovery specified in UDA v1.1.
         */
        UnicastDiscovery
    };

private Q_SLOTS:

    void unicastMessageReceived();
    void multicastMessageReceived();

protected:

    HSsdpPrivate* h_ptr;
    HSsdp(const QByteArray& loggingIdentifier, QObject* parent = 0);

protected:

    /*!
     * This method is called immediately after receiving a discovery request.
     *
     * Override this method if you want to handle the message. You can also connect
     * to the discoveryRequestReceived() signal.
     *
     * \param msg specifies the incoming message.
     * \param source specifies the source TCP/IP endpoint that sent the
     * message.
     * \param requestType specifies the type of the incoming discovery request.
     *
     * \retval true in case the message was handled successfully and the
     * discoveryRequestReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * discoveryRequestReceived() signal should be sent.
     *
     * \sa discoveryRequestReceived()
     */
    virtual bool incomingDiscoveryRequest(
        const HDiscoveryRequest& msg, const HEndpoint& source,
        DiscoveryRequestMethod requestType);

    /*!
     * This method is called immediately after receiving a discovery response.
     * Override this method if you want to handle message. You can also connect
     * to the discoveryResponseReceived() signal.
     *
     * \param msg specifies the incoming message.
     * \param source specifies the source TCP/IP endpoint that sent the
     * message.
     *
     * \retval true in case the message was handled successfully and the
     * discoveryResponseReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * discoveryResponseReceived() signal should be sent.
     *
     * \sa discoveryResponseReceived()
     */
    virtual bool incomingDiscoveryResponse(
        const HDiscoveryResponse& msg, const HEndpoint& source);

    /*!
     * This method is called immediately after receiving a device available announcement.
     * Override this method if you want to handle message. You can also connect
     * to the discoveryRequestReceived() signal.
     *
     * \param msg specifies the incoming message.
     * \param source specifies the source TCP/IP endpoint that sent the
     * message.
     *
     * \retval true in case the message was handled successfully and the
     * resourceAvailableReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * resourceAvailableReceived() signal should be sent.
     *
     * \sa resourceAvailableReceived()
     */
    virtual bool incomingDeviceAvailableAnnouncement(
        const HResourceAvailable& msg, const HEndpoint& source);

    /*!
     * This method is called immediately after receiving a device unavailable announcement.
     * Override this method if you want to handle message. You can also connect
     * to the resourceUnavailableReceived() signal.
     *
     * \param msg specifies the incoming message.
     * \param source specifies the source TCP/IP endpoint that sent the
     * message.
     *
     * \retval true in case the message was handled successfully and the
     * resourceUnavailableReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * resourceUnavailableReceived() signal should be sent.
     *
     * \sa resourceUnavailableReceived()
     */
    virtual bool incomingDeviceUnavailableAnnouncement(
        const HResourceUnavailable& msg, const HEndpoint& source);

    /*!
     * This method is called immediately after receiving a device update announcement.
     * Override this method if you want to handle message. You can also connect
     * to the deviceUpdateRecieved() signal.
     *
     * \param msg specifies the incoming message.
     * \param source specifies the source TCP/IP endpoint that sent the
     * message.
     *
     * \retval true in case the message was handled successfully and the
     * deviceUpdateRecieved() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * deviceUpdateRecieved() signal should be sent.
     *
     * \sa deviceUpdateRecieved()
     */
    virtual bool incomingDeviceUpdateAnnouncement(
        const HResourceUpdate& msg, const HEndpoint& source);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject.
     */
    HSsdp(QObject* parent=0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HSsdp();

    /*!
     * This enum is used to define a "filter", which specifies which message
     * types are to be processed when encountered.
     *
     * \sa filter(), setFilter()
     */
    enum AllowedMessage
    {
        /*!
         * No messages are processed.
         */
        None = 0x00,

        /*!
         * Device available messages are processed.
         */
        DeviceAvailable = 0x01,

        /*!
         * Device update messages are processed.
         */
        DeviceUpdate = 0x02,

        /*!
         * Device unavailable messages are processed.
         */
        DeviceUnavailable = 0x04,

        /*!
         * Discovery request messages are processed.
         */
        DiscoveryRequest = 0x08,

        /*!
         * Discovery response messages are processed.
         */
        DiscoveryResponse = 0x10,

        /*!
         * Discovery response messages are processed.
         */
        All = 0x1f
    };

    Q_DECLARE_FLAGS(AllowedMessages, AllowedMessage);

    /*!
     * \brief Sets the filter of what message types are accepted for processing.
     *
     * The default is HSsdp::All.
     *
     * \param allowedMessages defines the message types the instance should
     * accept for further processing. Other message types will be silently ignored.
     *
     * \sa filter()
     */
    void setFilter(AllowedMessages allowedMessages);

    /*!
     * \brief Returns the message types that are currently accepted for processing.
     *
     * Default is HSsdp::All.
     *
     * \return The message types that are currently accepted for processing.
     *
     * \sa setFilter()
     */
    AllowedMessages filter() const;

    /*!
     * \brief Sets the instance to listen the network for SSDP messages and and attempts to
     * init the unicast socket of the instance to the address of the first
     * found network address that is up and that is not loopback. If no such
     * interface is found the loopback address is used.
     *
     * \retval true in case the instances was successfully bound to some address.
     * \retval false in case the instance could not be bound or the instance
     * was already bound.
     *
     * \remarks \c %HSsdp has to be bound to receive messages of any type.
     */
    bool init();

    /*!
     * \brief Sets the instance to listen the network for SSDP messages and attempts to
     * init a unicast socket of the instance to the specified address.
     *
     * \param unicastAddress specifies the address that should be used for
     * unicast messaging.
     *
     * \retval true in case the instance was successfully bound to the
     * specified address.
     *
     * \retval false in case the instance could not be bound or the instance
     * was already bound to the specified address.
     *
     * \remarks \c %HSsdp has to be bound to receive messages of any type.
     */
    bool init(const QHostAddress& unicastAddress);

    /*!
     * \brief Indicates if the instance is bound to listen for messages using one
     * or more network interfaces.
     *
     * \return \e true in case the instance is bound to listen for messages
     * using one or more network interfaces.
     */
    bool isInitialized() const;

    /*!
     * \brief Returns the UDP endpoint that is used for unicast communication.
     *
     * \return The UDP endpoint that is used for unicast communication.
     */
    HEndpoint unicastEndpoint() const;

    /*!
     * Sends the specified device availability announcement.
     *
     * \param msg specifies the announcement to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \return The number of messages sent, 0 in case no messages was sent or
     * -1 in case the provided message is not valid.
     */
    qint32 announcePresence(const HResourceAvailable& msg, qint32 count = 1);

    /*!
     * Sends the specified device availability announcement.
     *
     * \param msg specifies the announcement to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \return The number of messages sent, 0 in case no messages was sent or
     * -1 in case the provided message is not valid.
     */
    qint32 announcePresence(const HResourceUnavailable& msg, qint32 count = 1);

    /*!
     * Sends the specified device update announcement.
     *
     * \param msg specifies the message to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \return The number of messages sent, 0 in case no messages was sent or
     * -1 in case the provided message is not valid.
     */
    qint32 announceUpdate(const HResourceUpdate& msg, qint32 count = 1);

    /*!
     * Sends the specified discovery request.
     *
     * Sends the specified discovery request to a multicast address
     * 239.255.255.250.
     *
     * \param msg specifies the announcement to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \return The number of messages sent, 0 in case no messages was sent or
     * -1 in case the provided message is not valid.
     */
    qint32 sendDiscoveryRequest(const HDiscoveryRequest& msg, qint32 count = 1);

     /*!
     * Sends the specified discovery request.
     *
     * Sends the specified discovery request to a specified address. The
     * address can be an unicast address or a multicast address.
     *
     * \param msg specifies the announcement to send.
     * \param destination specifies the target UDP endpoint of the message.
     * If the port of the specified endpoint is set to zero the message is sent
     * to the specified host address using the default port 1900.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \return The number of messages sent, 0 in case no messages was sent or
     * -1 in case the provided message or the destination is not valid.
     */
    qint32 sendDiscoveryRequest(
        const HDiscoveryRequest& msg, const HEndpoint& destination,
        qint32 count = 1);

    /*!
     * Sends the specified discovery response.
     *
     * \param msg specifies the announcement to send.
     *
     * \param destination specifies the target of the response.
     * If the port of the specified endpoint is set to zero the message is sent
     * to the specified host address using the default port 1900.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \return The number of messages sent, 0 in case no messages was sent or
     * -1 in case the provided message is not valid.
     */
    qint32 sendDiscoveryResponse(
        const HDiscoveryResponse& msg, const HEndpoint& destination,
        qint32 count = 1);

////
////////////////////////////////////////////////////////////////////////////////
Q_SIGNALS:

    /*!
     * \brief This signal is emitted when a <em>discovery request</em> is received.
     *
     * \param msg specifies the received <em>discovery request</em> message.
     * \param source specifies the location where the message came.
     * \param requestType specifies the type of the incoming discovery request.
     */
    void discoveryRequestReceived(
        const Herqq::Upnp::HDiscoveryRequest& msg,
        const Herqq::Upnp::HEndpoint& source,
        Herqq::Upnp::HSsdp::DiscoveryRequestMethod requestType);

    /*!
     * \brief This signal is emitted when a <em>discovery response</em> is received.
     *
     * \param msg specifies the received <em>discovery response</em> message.
     * \param source specifies the location where the message came.
     */
    void discoveryResponseReceived(
        const Herqq::Upnp::HDiscoveryResponse& msg,
        const Herqq::Upnp::HEndpoint& source);

    /*!
     * \brief This signal is emitted when a <em>device announcement</em> is received.
     *
     * \param msg specifies the <em>device announcement</em> message.
     * \param source specifies the location where the message came.
     */
    void resourceAvailableReceived(
        const Herqq::Upnp::HResourceAvailable& msg,
        const Herqq::Upnp::HEndpoint& source);

    /*!
     * \brief This signal is emitted when a <em>device update</em> is received.
     *
     * \param msg specifies the <em>device update</em> message.
     * \param source specifies the location where the message came.
     */
    void deviceUpdateReceived(
        const Herqq::Upnp::HResourceUpdate& msg,
        const Herqq::Upnp::HEndpoint& source);

    /*!
     * \brief This signal is emitted when a <em>device announcement</em> is received.
     *
     * \param msg specifies the <em>device announcement</em> message.
     * \param source specifies the location where the message came.
     */
    void resourceUnavailableReceived(
        const Herqq::Upnp::HResourceUnavailable& msg,
        const Herqq::Upnp::HEndpoint& source);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HSsdp::AllowedMessages)

}
}

#endif /* HSSDP_H_ */
