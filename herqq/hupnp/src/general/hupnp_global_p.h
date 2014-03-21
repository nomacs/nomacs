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

#ifndef HUPNP_GLOBAL_P_H_
#define HUPNP_GLOBAL_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hupnp_fwd.h"
#include "hupnp_defs.h"

#include <QtCore/QUrl>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

class QDomElement;

namespace Herqq
{

namespace Upnp
{

//
//
//
class HSysInfo
{
H_DISABLE_COPY(HSysInfo)

private:

    HSysInfo();

    static QScopedPointer<HSysInfo> s_instance;
    static QMutex s_initMutex;

    QScopedPointer<HProductTokens> m_productTokens;

    void createProductTokens();
    static QList<QPair<quint32, quint32> > createLocalNetworks();

public:

    ~HSysInfo();

    static HSysInfo& instance();

    inline const HProductTokens& herqqProductTokens()
    {
        return *m_productTokens;
    }

    bool localNetwork(const QHostAddress&, quint32*) const;

    bool isLocalAddress(const QHostAddress&) const;
    bool areLocalAddresses(const QList<QHostAddress>&) const;
};

//
//
//
HEndpoints convertHostAddressesToEndpoints(const QList<QHostAddress>&);

//
//
//
QString readElementValue(
    const QString elementTagToSearch,
    const QDomElement& parentElement,
    bool* wasDefined = 0);

//
//
//
QString toString(const QDomElement&);

//
//
//
bool verifyName(const QString& name, QString* err=0);

//
// Returns the provided URLs as a string following format "#N URL\n",
// where N = 0..., and URL is the N'th URL in the list.
//
QString urlsAsStr(const QList<QUrl>&);

//
//
//
inline QString peerAsStr(const QTcpSocket& sock)
{
    return QString("%1:%2").arg(
        sock.peerAddress().toString(), QString::number(sock.peerPort()));
}

//
//
//
QString extractBaseUrl(const QString& url);

//
//
//
inline QUrl extractBaseUrl(const QUrl& url)
{
    QString urlAsStr = url.toString();
    return extractBaseUrl(urlAsStr);
}

//
//
//
inline QString extractHostPart(const QUrl& arg)
{
    return arg.toString(
        QUrl::RemovePassword | QUrl::RemoveUserInfo |
        QUrl::StripTrailingSlash | QUrl::RemovePath | QUrl::RemoveQuery |
        QUrl::RemoveFragment);
}

//
// Returns the part + query (== request in entirety) sections of the url
//
inline QString extractRequestPart(const QUrl& arg)
{
    return arg.toString(
        QUrl::RemoveAuthority | QUrl::RemovePassword | QUrl::RemoveUserInfo |
        QUrl::RemoveScheme | QUrl::RemovePort | QUrl::RemoveFragment);
}

//
//
//
QUrl resolveUri(const QUrl& baseUrl, const QUrl& relativeUrl);

//
//
//
QUrl appendUrls(const QUrl& baseUrl, const QUrl& relativeUrl);

}
}

#endif /* HUPNP_GLOBAL_P_H_ */
