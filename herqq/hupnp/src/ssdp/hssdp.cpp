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

#include "hssdp.h"
#include "hssdp_p.h"
#include "hdiscovery_messages.h"
#include "hssdp_messagecreator_p.h"

#include "../dataelements/hdiscoverytype.h"
#include "../dataelements/hproduct_tokens.h"

#include "../socket/hendpoint.h"

#include "../general/hlogger_p.h"
#include "../utils/hmisc_utils_p.h"

#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtNetwork/QHostAddress>

/*!
 * \defgroup hupnp_ssdp Ssdp
 * \ingroup hupnp_core
 *
 * \brief This page provides information about the HUPnP's classes that provide the
 * SSDP functionality required for the discovery phase of the UPnP Device Architecture.
 *
 * According to the UPnP Device Architecture specification version 1.1,
 * <em>When a device is added to the network, the UPnP discovery protocol allows that
 * device to advertise its services to control points on the network. Similarly,
 * when a control point is added to the network, the UPnP discovery protocol allows
 * that control point to search for devices of interest on the network</em> (p. 19).
 *
 * The mentioned <em>discovery protocol</em> is SSDP and it is about exchanging
 * HTTP messages over UDP.
 *
 * \note As mentioned in the Herqq::Upnp::HSsdp, these classes implement the SSDP
 * as it is required by the UDA specification. The IETF SSDP draft is not
 * implemented in full.
 *
 * To send or receive SSDP messages, you need to use Herqq::Upnp::HSsdp
 * class. You can either derive from it or use it directly. In either case,
 * sending messages is straightforward:
 *
 * \code
 *
 * Herqq::Upnp::HSsdp ssdp;
 *
 * Herqq::Upnp::HResourceAvailable deviceAvailable(
 *       1800, // how long the advertisement is valid in seconds
 *       QUrl("127.0.0.1:1900/mydevice"), // where the device description can be downloaded
 *       Herqq::Upnp::HProductTokens("unix/5.1 UPnP/1.1 MyProduct/1.0"), // some information about the host and product
 *       Herqq::Upnp::HDiscoveryType("uuid:5d724fc2-5c5e-4760-a123-f04a9136b300::upnp:rootdevice")); // universally unique identifier
 *
 * ssdp.announcePresence(deviceAvailable);
 *
 * \endcode
 *
 * The above example sends a single <em>ssdp:alive</em> message indicating that a
 * UPnP root device is now available.
 *
 * \note All SSDP classes validate the provided information during
 * object construction. For instance, if the argument to the Herqq::Upnp::HDiscoveryType
 * is invalid, the constructed object will be invalid as well,
 * e.g Herqq::Upnp::HDiscoveryType::isValid() returns false. In such a case, the creation
 * of Herqq::Upnp::HResourceAvailable will fail and consequenlty, the \c %HSsdp will
 * not send anything.
 *
 * Receiving messages is almost as simple. You can use the class directly in which
 * case you have to connect to the exposed signals. For instance, to receive signals
 * when <em>resource available</em> messages are received, you should do:
 *
 * \code
 *
 * MyClass::MyClass(QObject* parent) :
 *     QObject(parent), m_ssdp(new Herqq::Upnp::HSsdp())
 * {
 *     connect(
 *         m_ssdp, SIGNAL(resourceAvailableReceived(Herqq::Upnp::HResourceAvailable)),
 *         this  , SLOT  (resourceAvailableReceived(Herqq::Upnp::HResourceAvailable)));
 * }
 *
 * MyClass::resourceAvailableReceived(const Herqq::Upnp::HResourceAvailable&)
 * {
 *     // do something
 * }
 *
 * \endcode
 *
 * Note the used signature with the SIGNAL and SLOT macros. You can also derive
 * from \c %HSsdp in which case you can override the various \c protected \c virtual
 * methods that will be called upon message reception.
 *
 * \attention
 * Usually you have no need to use the classes in this module directly. These classes
 * may be useful in case you are writing your own device host or control point.
 * Otherwise, the Herqq::Upnp::HControlPoint and Herqq::Upnp::HDeviceHost classes
 * may suit your needs better.
 */

namespace Herqq
{

namespace Upnp
{

namespace
{
inline QHostAddress multicastAddress()
{
    static QHostAddress retVal("239.255.255.250");
    return retVal;
}

inline qint16 multicastPort()
{
    static qint16 retVal = 1900;
    return retVal;
}

inline HEndpoint multicastEndpoint()
{
    static HEndpoint retVal = HEndpoint("239.255.255.250:1900");
    return retVal;
}
}

/*******************************************************************************
 * HSsdpPrivate
 ******************************************************************************/
HSsdpPrivate::HSsdpPrivate(
    HSsdp* qptr, const QByteArray& loggingIdentifier) :
        m_loggingIdentifier(loggingIdentifier),
        m_multicastSocket(0),
        m_unicastSocket  (0),
        q_ptr            (qptr),
        m_allowedMessages(HSsdp::All),
        m_lastError()
{
}

HSsdpPrivate::~HSsdpPrivate()
{
    clear();
}

bool HSsdpPrivate::parseCacheControl(const QString& str, qint32* retVal)
{
    bool ok = false;

    QString cacheControl = str.simplified();
    QStringList slist = cacheControl.split('=');

    if (slist.size() != 2 || slist[0].simplified() != "max-age")
    {
        m_lastError =
            QString("Invalid Cache-Control field value: %1").arg(str);
        return false;
    }

    qint32 maxAge = slist[1].simplified().toInt(&ok);
    if (!ok)
    {
        m_lastError =
            QString("Invalid Cache-Control field value: %1").arg(str);
        return false;
    }

    *retVal = maxAge;
    return true;
}

bool HSsdpPrivate::checkHost(const QString& host)
{
    QStringList slist = host.split(':');
    if (slist.size() < 1 || slist[0].simplified() != "239.255.255.250")
    {
        m_lastError =
            QString("HOST header field is invalid: %1").arg(host);
        return false;
    }

    return true;
}

bool HSsdpPrivate::parseDiscoveryResponse(
    const HHttpResponseHeader& hdr, HDiscoveryResponse* retVal)
{
    QString   cacheControl  = hdr.value("CACHE-CONTROL");
    QDateTime date          = QDateTime::fromString(hdr.value("DATE"));
    QUrl      location      = hdr.value("LOCATION");
    QString   server        = hdr.value("SERVER");
    //QString   st            = hdr.value("ST");
    QString   usn           = hdr.value("USN");
    QString   bootIdStr     = hdr.value("BOOTID.UPNP.ORG");
    QString   configIdStr   = hdr.value("CONFIGID.UPNP.ORG");
    QString   searchPortStr = hdr.value("SEARCHPORT.UPNP.ORG");

    if (!hdr.hasKey("EXT"))
    {
        m_lastError = QString("EXT field is missing:\n%1").arg(
            hdr.toString());

        return false;
    }
    else if (!hdr.value("EXT").isEmpty())
    {
        m_lastError =
            QString("EXT field is not empty, although it should be:\n%1").
                arg(hdr.toString());

        return false;
    }

    qint32 maxAge;
    if (!parseCacheControl(cacheControl, &maxAge))
    {
        return false;
    }

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    qint32 searchPort = searchPortStr.toInt(&ok);
    if (!ok)
    {
        searchPort = -1;
    }

    *retVal = HDiscoveryResponse(
        maxAge,
        date,
        location,
        HProductTokens(server),
        HDiscoveryType(usn, LooseChecks),
        bootId,
        hdr.hasKey("CONFIGID.UPNP.ORG") ? configId : 0,
        // ^^ configid is optional even in UDA v1.1 ==> cannot provide -1
        // unless the header field is specified and the value is invalid
        searchPort);

    return retVal->isValid(LooseChecks);
}

bool HSsdpPrivate::parseDiscoveryRequest(
    const HHttpRequestHeader& hdr, HDiscoveryRequest* retVal)
{
    QString host = hdr.value("HOST");
    QString man  = hdr.value("MAN").simplified();

    bool ok = false;
    qint32 mx = hdr.value("MX").toInt(&ok);

    if (!ok)
    {
        m_lastError = QString("MX is not specified.");
        return false;
    }

    QString st = hdr.value("ST");
    QString ua = hdr.value("USER-AGENT");

    checkHost(host);

    if (man.compare(QString("\"ssdp:discover\""), Qt::CaseInsensitive) != 0)
    {
        m_lastError =
            QString("MAN header field is invalid: [%1].").arg(man);

        return false;
    }

    *retVal = HDiscoveryRequest(
        mx, HDiscoveryType(st, LooseChecks), HProductTokens(ua));

    return retVal->isValid(LooseChecks);
}

bool HSsdpPrivate::parseDeviceAvailable(
    const HHttpRequestHeader& hdr, HResourceAvailable* retVal)
{
    QString host          = hdr.value("HOST");
    QString server        = hdr.value("SERVER");
    QString usn           = hdr.value("USN");
    //QString nt            = hdr.value("NT");
    QUrl    location      = hdr.value("LOCATION");
    QString cacheControl  = hdr.value("CACHE-CONTROL");
    QString bootIdStr     = hdr.value("BOOTID.UPNP.ORG");
    QString configIdStr   = hdr.value("CONFIGID.UPNP.ORG");
    QString searchPortStr = hdr.value("SEARCHPORT.UPNP.ORG");

    qint32 maxAge;
    if (!parseCacheControl(cacheControl, &maxAge))
    {
        return false;
    }

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    checkHost(host);

    qint32 searchPort = searchPortStr.toInt(&ok);
    if (!ok)
    {
        searchPort = -1;
    }

    *retVal = HResourceAvailable(
        maxAge,
        location,
        HProductTokens(server),
        HDiscoveryType(usn, LooseChecks),
        bootId,
        configId,
        searchPort);

    return retVal->isValid(LooseChecks);
}

bool HSsdpPrivate::parseDeviceUnavailable(
    const HHttpRequestHeader& hdr, HResourceUnavailable* retVal)
{
    QString host        = hdr.value("HOST");
    //QString nt          = hdr.value("NT");
    QString usn         = hdr.value("USN");
    QString bootIdStr   = hdr.value("BOOTID.UPNP.ORG");
    QString configIdStr = hdr.value("CONFIGID.UPNP.ORG");

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    checkHost(host);

    *retVal = HResourceUnavailable(
        HDiscoveryType(usn, LooseChecks), bootId, configId);

    return retVal->isValid(LooseChecks);
}

bool HSsdpPrivate::parseDeviceUpdate(
    const HHttpRequestHeader& hdr, HResourceUpdate* retVal)
{
    QString host          = hdr.value("HOST");
    QUrl    location      = hdr.value("LOCATION");
    //QString nt            = hdr.value("NT");
    QString usn           = hdr.value("USN");
    QString bootIdStr     = hdr.value("BOOTID.UPNP.ORG");
    QString configIdStr   = hdr.value("CONFIGID.UPNP.ORG");
    QString nextBootIdStr = hdr.value("NEXTBOOTID.UPNP.ORG");
    QString searchPortStr = hdr.value("SEARCHPORT.UPNP.ORG");

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    qint32 nextBootId = nextBootIdStr.toInt(&ok);
    if (!ok)
    {
        nextBootId = -1;
    }

    qint32 searchPort = searchPortStr.toInt(&ok);
    if (!ok)
    {
        searchPort = -1;
    }

    checkHost(host);

    *retVal = HResourceUpdate(
        location,
        HDiscoveryType(usn, LooseChecks),
        bootId,
        configId,
        nextBootId,
        searchPort);

    return retVal->isValid(LooseChecks);
}

bool HSsdpPrivate::send(const QByteArray& data, const HEndpoint& receiver)
{
    Q_ASSERT(isInitialized());

    quint16 port = receiver.portNumber();
    if (!port) { port = 1900; }

    qint64 retVal = m_unicastSocket->writeDatagram(
        data, receiver.hostAddress(), port);

    return retVal == data.size();
}

void HSsdpPrivate::processResponse(const QString& msg, const HEndpoint& source)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HHttpResponseHeader hdr(msg);
    if (!hdr.isValid())
    {
        HLOG_WARN("Ignoring a malformed HTTP response.");
        return;
    }

    if (m_allowedMessages & HSsdp::DiscoveryResponse)
    {
        HDiscoveryResponse rcvdMsg;
        if (!parseDiscoveryResponse(hdr, &rcvdMsg))
        {
            HLOG_WARN(QString("Ignoring invalid message from [%1]: %2").arg(
                source.toString(), msg));
        }
        else if (!q_ptr->incomingDiscoveryResponse(rcvdMsg, source))
        {
            emit q_ptr->discoveryResponseReceived(rcvdMsg, source);
        }
    }
}

void HSsdpPrivate::processNotify(const QString& msg, const HEndpoint& source)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HHttpRequestHeader hdr(msg);
    if (!hdr.isValid())
    {
        HLOG_WARN("Ignoring an invalid HTTP NOTIFY request.");
        return;
    }

    QString nts = hdr.value("NTS");
    if (nts.compare(QString("ssdp:alive"), Qt::CaseInsensitive) == 0)
    {
        if (m_allowedMessages & HSsdp::DeviceAvailable)
        {
            HResourceAvailable rcvdMsg;
            if (!parseDeviceAvailable(hdr, &rcvdMsg))
            {
                HLOG_WARN(QString(
                    "Ignoring an invalid ssdp:alive announcement:\n%1").arg(msg));
            }
            else if (!q_ptr->incomingDeviceAvailableAnnouncement(rcvdMsg, source))
            {
                emit q_ptr->resourceAvailableReceived(rcvdMsg, source);
            }
        }
    }
    else if (nts.compare(QString("ssdp:byebye"), Qt::CaseInsensitive) == 0)
    {
        if (m_allowedMessages & HSsdp::DeviceUnavailable)
        {
            HResourceUnavailable rcvdMsg;
            if (!parseDeviceUnavailable(hdr, &rcvdMsg))
            {
                HLOG_WARN(QString(
                    "Ignoring an invalid ssdp:byebye announcement:\n%1").arg(msg));
            }
            else if (!q_ptr->incomingDeviceUnavailableAnnouncement(rcvdMsg, source))
            {
                emit q_ptr->resourceUnavailableReceived(rcvdMsg, source);
            }
        }
    }
    else if (nts.compare(QString("ssdp:update"), Qt::CaseInsensitive) == 0)
    {
        if (m_allowedMessages & HSsdp::DeviceUpdate)
        {
            HResourceUpdate rcvdMsg;
            if (!parseDeviceUpdate(hdr, &rcvdMsg))
            {
                HLOG_WARN(QString(
                    "Ignoring invalid ssdp:update announcement:\n%1").arg(msg));
            }
            else if (!q_ptr->incomingDeviceUpdateAnnouncement(rcvdMsg, source))
            {
                emit q_ptr->deviceUpdateReceived(rcvdMsg, source);
            }
        }
    }
    else
    {
        HLOG_WARN(QString(
            "Ignoring an invalid SSDP presence announcement: [%1].").arg(nts));
    }
}

void HSsdpPrivate::processSearch(
    const QString& msg, const HEndpoint& source, const HEndpoint& destination)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HHttpRequestHeader hdr(msg);
    if (!hdr.isValid())
    {
        HLOG_WARN("Ignoring an invalid HTTP M-SEARCH request.");
        return;
    }

    if (m_allowedMessages & HSsdp::DiscoveryRequest)
    {
        HSsdp::DiscoveryRequestMethod type = destination.isMulticast() ?
            HSsdp::MulticastDiscovery : HSsdp::UnicastDiscovery;

        HDiscoveryRequest rcvdMsg;
        if (!parseDiscoveryRequest(hdr, &rcvdMsg))
        {
            HLOG_WARN(QString("Ignoring invalid message from [%1]: %2").arg(
                source.toString(), msg));
        }
        else if (!q_ptr->incomingDiscoveryRequest(rcvdMsg, source, type))
        {
            emit q_ptr->discoveryRequestReceived(rcvdMsg, source, type);
        }
    }
}

void HSsdpPrivate::clear()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    if (m_multicastSocket &&
        m_multicastSocket->state() == QUdpSocket::BoundState)
    {
        m_multicastSocket->leaveMulticastGroup(
            multicastAddress(), m_unicastSocket->localAddress());
    }
    delete m_unicastSocket; m_unicastSocket = 0;
    delete m_multicastSocket; m_multicastSocket = 0;
}

bool HSsdpPrivate::init(const QHostAddress& addressToBind)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(!isInitialized());

    m_multicastSocket = new HMulticastSocket(q_ptr);
    m_unicastSocket   = new QUdpSocket(q_ptr);

    bool ok = QObject::connect(
        m_multicastSocket, SIGNAL(readyRead()),
        q_ptr, SLOT(multicastMessageReceived()));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = QObject::connect(
        m_unicastSocket, SIGNAL(readyRead()),
        q_ptr, SLOT(unicastMessageReceived()));

    Q_ASSERT(ok);

    if (!m_multicastSocket->bind(1900))
    {
        HLOG_WARN("Failed to bind multicast socket for listening");
        return false;
    }

    if (!m_multicastSocket->joinMulticastGroup(
            multicastAddress(), addressToBind))
    {
        HLOG_WARN(QString("Could not join %1").arg(
            multicastAddress().toString()));

        //return false;
    }

    HLOG_DBG(QString(
        "Attempting to use address [%1] for SSDP communications").arg(
            addressToBind.toString()));

    // always attempt to bind to the 1900 first
    if (!m_unicastSocket->bind(addressToBind, 1900))
    {
        HLOG_DBG("Could not bind UDP unicast socket to port 1900");

        // the range is specified by the UDA 1.1 standard
        for(qint32 i = 49152; i < 65535; ++i)
        {
            if (m_unicastSocket->bind(addressToBind, i))
            {
                HLOG_DBG(QString("Unicast UDP socket bound to port [%1].").arg(
                    QString::number(i)));

                break;
            }
        }
    }
    else
    {
        HLOG_DBG("Unicast UDP socket bound to port 1900");
    }

    if (m_unicastSocket->state() != QUdpSocket::BoundState)
    {
        HLOG_WARN(QString(
            "Failed to bind UDP unicast socket on address.").arg(
                addressToBind.toString()));

        clear();
        return false;
    }

    return true;
}

void HSsdpPrivate::messageReceived(QUdpSocket* socket, const HEndpoint* dest)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHostAddress ha; quint16 port;

    QByteArray buf;
    buf.resize(socket->pendingDatagramSize() + 1);

    qint64 read = socket->readDatagram(buf.data(), buf.size(), &ha, &port);
    if (read < 0)
    {
        HLOG_WARN(QString("Read failed: %1").arg(socket->errorString()));
        Q_ASSERT(false);
        return;
    }

    QString msg(QString::fromUtf8(buf, read));
    HEndpoint source(ha, port);
    HEndpoint destination(
        dest ? *dest : HEndpoint(socket->localAddress(), socket->localPort()));

    if (msg.startsWith("NOTIFY * HTTP/1.1", Qt::CaseInsensitive))
    {
        // Possible presence announcement
        processNotify(msg, source);
    }
    else if (msg.startsWith("M-SEARCH * HTTP/1.1", Qt::CaseInsensitive))
    {
        // Possible discovery request.
        processSearch(msg, source, destination);
    }
    else
    {
        // Possible discovery response
        processResponse(msg, source);
    }
}

/*******************************************************************************
 * HSsdp
 ******************************************************************************/
HSsdp::HSsdp(QObject* parent) :
    QObject(parent),
        h_ptr(new HSsdpPrivate(this))
{
}

HSsdp::HSsdp(const QByteArray& loggingIdentifier, QObject* parent) :
    QObject(parent),
        h_ptr(new HSsdpPrivate(this, loggingIdentifier))
{
}

HSsdp::~HSsdp()
{
    delete h_ptr;
}

void HSsdp::unicastMessageReceived()
{
    h_ptr->messageReceived(h_ptr->m_unicastSocket);
}

void HSsdp::multicastMessageReceived()
{
    HEndpoint ep = multicastEndpoint();
    h_ptr->messageReceived(h_ptr->m_multicastSocket, &ep);
}

void HSsdp::setFilter(AllowedMessages allowedMessages)
{
    h_ptr->m_allowedMessages = allowedMessages;
}

HSsdp::AllowedMessages HSsdp::filter() const
{
    return h_ptr->m_allowedMessages;
}

bool HSsdp::init()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (isInitialized())
    {
        return false;
    }

    QHostAddress ha(findBindableHostAddress());
    return h_ptr->init(ha);
}

bool HSsdp::init(const QHostAddress& unicastAddress)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (isInitialized())
    {
        return false;
    }

    return h_ptr->init(unicastAddress);
}

bool HSsdp::isInitialized() const
{
    return h_ptr->isInitialized();
}

HEndpoint HSsdp::unicastEndpoint() const
{
    return HEndpoint(
        h_ptr->m_unicastSocket->localAddress(),
        h_ptr->m_unicastSocket->localPort());
}

bool HSsdp::incomingDiscoveryRequest(
    const HDiscoveryRequest&, const HEndpoint& /*source*/,
    DiscoveryRequestMethod /*requestType*/)
{
    return false;
}

bool HSsdp::incomingDiscoveryResponse(
    const HDiscoveryResponse&, const HEndpoint& /*source*/)
{
    return false;
}

bool HSsdp::incomingDeviceAvailableAnnouncement(
    const HResourceAvailable&, const HEndpoint& /*source*/)
{
    return false;
}

bool HSsdp::incomingDeviceUnavailableAnnouncement(
    const HResourceUnavailable&, const HEndpoint& /*source*/)
{
    return false;
}

bool HSsdp::incomingDeviceUpdateAnnouncement(
    const HResourceUpdate&, const HEndpoint& /*source*/)
{
    return false;
}

namespace
{
template<class Msg>
qint32 send(HSsdpPrivate* hptr, const Msg& msg, const HEndpoint& receiver,
            qint32 count)
{
    HLOG(H_AT, H_FUN);
    if (!msg.isValid(StrictChecks) || receiver.isNull() || count < 0 ||
        !hptr->isInitialized())
    {
        return -1;
    }

    qint32 sent = 0;
    for (qint32 i = 0; i < count; ++i)
    {
        QByteArray data = HSsdpMessageCreator::create(msg);
        Q_ASSERT(!data.isEmpty());
        if (hptr->send(data, receiver))
        {
            ++sent;
        }
        else
        {
            HLOG_DBG(hptr->m_unicastSocket->errorString());
        }
    }

    return sent;
}
}

qint32 HSsdp::announcePresence(const HResourceAvailable& msg, qint32 count)
{
    return send(h_ptr, msg, multicastEndpoint(), count);
}

qint32 HSsdp::announcePresence(const HResourceUnavailable& msg, qint32 count)
{
    return send(h_ptr, msg, multicastEndpoint(), count);
}

qint32 HSsdp::announceUpdate(const HResourceUpdate& msg, qint32 count)
{
    return send(h_ptr, msg, multicastEndpoint(), count);
}

qint32 HSsdp::sendDiscoveryRequest(const HDiscoveryRequest& msg, qint32 count)
{
    return send(h_ptr, msg, multicastEndpoint(), count);
}

qint32 HSsdp::sendDiscoveryRequest(
    const HDiscoveryRequest& msg, const HEndpoint& endpoint, qint32 count)
{
    return send(h_ptr, msg, endpoint, count);
}

qint32 HSsdp::sendDiscoveryResponse(
    const HDiscoveryResponse& msg, const HEndpoint& destination, qint32 count)
{
    return send(h_ptr, msg, destination, count);
}

}
}
