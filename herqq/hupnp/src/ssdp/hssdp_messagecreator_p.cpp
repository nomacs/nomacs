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

#include "hssdp_messagecreator_p.h"
#include "hdiscovery_messages.h"

#include "../dataelements/hresourcetype.h"
#include "../dataelements/hdiscoverytype.h"
#include "../dataelements/hproduct_tokens.h"

#include "../socket/hendpoint.h"
#include "../general/hlogger_p.h"

#include <QtCore/QUrl>
#include <QtCore/QTextStream>

namespace Herqq
{

namespace Upnp
{

namespace
{
HEndpoint multicastEndpoint()
{
    static HEndpoint retVal(QHostAddress("239.255.255.250"), 1900);
    return retVal;
}

QString getTarget(const HDiscoveryType& ri)
{
    switch(ri.type())
    {
        case HDiscoveryType::Undefined:
            return "";
        case HDiscoveryType::All:
        case HDiscoveryType::RootDevices:
        case HDiscoveryType::SpecificDevice:
            return ri.toString();

        case HDiscoveryType::SpecificRootDevice:
            return "upnp:rootdevice";

        default:
            return ri.resourceType().toString();
    }

    return QString();
}
}

HSsdpMessageCreator::HSsdpMessageCreator()
{
}

HSsdpMessageCreator::~HSsdpMessageCreator()
{
}

QByteArray HSsdpMessageCreator::create(const HResourceUpdate& msg)
{
    if (!msg.isValid(StrictChecks))
    {
        return QByteArray();
    }

    QString retVal;
    QTextStream ts(&retVal);

    ts << "NOTIFY * HTTP/1.1\r\n"
       << "HOST: "     << multicastEndpoint().toString()  << "\r\n"
       << "LOCATION: " << msg.location().toString()           << "\r\n"
       << "NT: "       << getTarget(msg.usn()) << "\r\n"
       << "NTS: "      << "ssdp:update\r\n"
       << "USN: "      << msg.usn().toString() << "\r\n";

    if (msg.bootId() >= 0)
    {
        ts << "BOOTID.UPNP.ORG: "       << msg.bootId()     << "\r\n"
           << "CONFIGID.UPNP.ORG: "     << msg.configId()   << "\r\n"
           << "NEXTBOOTID.UPNP.ORG: "   << msg.nextBootId() << "\r\n";

        if (msg.searchPort() >= 0)
        {
            ts << "SEARCHPORT.UPNP.ORG: " << msg.searchPort() << "\r\n";
        }
    }

    ts << "\r\n";

    return retVal.toUtf8();
}

QByteArray HSsdpMessageCreator::create(const HDiscoveryRequest& msg)
{
    if (!msg.isValid(StrictChecks))
    {
        return QByteArray();
    }

    QString retVal;
    QTextStream out(&retVal);

    out << "M-SEARCH * HTTP/1.1\r\n"
        << "HOST: "       << multicastEndpoint().toString() << "\r\n"
        << "MAN: "        << "\"ssdp:discover\"\r\n"
        << "MX: "         << msg.mx()                  << "\r\n"
        << "ST: "         << getTarget(msg.searchTarget()) << "\r\n"
        << "USER-AGENT: " << msg.userAgent().toString()<< "\r\n\r\n";

    return retVal.toUtf8();
}

QByteArray HSsdpMessageCreator::create(const HDiscoveryResponse& msg)
{
    if (!msg.isValid(StrictChecks))
    {
        return QByteArray();
    }

    QString retVal;
    QTextStream out(&retVal);

    out << "HTTP/1.1 200 OK\r\n"
        << "CACHE-CONTROL: max-age=" << msg.cacheControlMaxAge() << "\r\n"
        << "EXT:\r\n"
        << "LOCATION: "              << msg.location().toString() << "\r\n"
        << "SERVER: "                << msg.serverTokens().toString() << "\r\n"
        << "ST: "                    << getTarget(msg.usn()) << "\r\n"
        << "USN: "                   << msg.usn().toString() << "\r\n";

    if (msg.bootId() >= 0)
    {
        out << "BOOTID.UPNP.ORG: "   << msg.bootId()   << "\r\n"
            << "CONFIGID.UPNP.ORG: " << msg.configId() <<"\r\n";

        if (msg.searchPort() >= 0)
        {
            out << "SEARCHPORT.UPNP.ORG: " << msg.searchPort() << "\r\n";
        }
    }

    out << "\r\n";

    return retVal.toUtf8();
}

QByteArray HSsdpMessageCreator::create(const HResourceAvailable& msg)
{
    if (!msg.isValid(StrictChecks))
    {
        return QByteArray();
    }

    QString retVal;
    QTextStream ts(&retVal);

    ts << "NOTIFY * HTTP/1.1\r\n"
       << "HOST: "                  << multicastEndpoint().toString() << "\r\n"
       << "CACHE-CONTROL: max-age=" << msg.cacheControlMaxAge() << "\r\n"
       << "LOCATION: "              << msg.location().toString() << "\r\n"
       << "NT: "                    << getTarget(msg.usn()) << "\r\n"
       << "NTS: "                   << "ssdp:alive\r\n"
       << "SERVER: "                << msg.serverTokens().toString() << "\r\n"
       << "USN: "                   << msg.usn().toString() << "\r\n";

    if (msg.serverTokens().upnpToken().minorVersion() > 0)
    {
        ts << "BOOTID.UPNP.ORG: "   << msg.bootId()   << "\r\n"
           << "CONFIGID.UPNP.ORG: " << msg.configId() << "\r\n";

        if (msg.searchPort() >= 0)
        {
            ts << "SEARCHPORT.UPNP.ORG: " << msg.searchPort() << "\r\n";
        }
    }

    ts << "\r\n";

    return retVal.toUtf8();
}

QByteArray HSsdpMessageCreator::create(const HResourceUnavailable& msg)
{
    if (!msg.isValid(StrictChecks))
    {
        return QByteArray();
    }

    QString retVal;
    QTextStream ts(&retVal);

    ts << "NOTIFY * HTTP/1.1\r\n"
       << "HOST: " << multicastEndpoint().toString()<< "\r\n"
       << "NT: "   << getTarget(msg.usn()) << "\r\n"
       << "NTS: "  << "ssdp:byebye\r\n"
       << "USN: "  << msg.usn().toString() << "\r\n";

    if (msg.bootId() >= 0)
    {
        ts << "BOOTID.UPNP.ORG: "   << msg.bootId  () << "\r\n"
           << "CONFIGID.UPNP.ORG: " << msg.configId() << "\r\n";
    }

    ts << "\r\n";

    return retVal.toUtf8();
}

}
}
