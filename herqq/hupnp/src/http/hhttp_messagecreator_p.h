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

#ifndef HHTTP_MESSAGECREATOR_H_
#define HHTTP_MESSAGECREATOR_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hhttp_p.h"

#include <HUpnpCore/HUpnp>

#include <QtCore/QString>

class QByteArray;

namespace Herqq
{

namespace Upnp
{

class HNotifyRequest;
class HSubscribeRequest;
class HUnsubscribeRequest;
class HSubscribeResponse;

class HHttpHeader;
class HMessagingInfo;
class HHttpRequestHeader;
class HHttpResponseHeader;

//
//
//
class H_UPNP_CORE_EXPORT HHttpMessageCreator
{
H_FORCE_SINGLETON(HHttpMessageCreator)

private:

    static QByteArray setupData(
        const HMessagingInfo&, qint32 statusCode,
        const QString& reasonPhrase, const QString& body,
        ContentType);

    static QByteArray setupData(
        HHttpHeader& reqHdr, qint64 bodySizeBytesInBytes, const HMessagingInfo& mi,
        ContentType);

public:

    static QByteArray setupData(HHttpHeader& hdr, const HMessagingInfo&);

    static QByteArray setupData(
        HHttpHeader& hdr, const QByteArray& body, const HMessagingInfo&,
        ContentType);

    static QByteArray createResponse(
        StatusCode sc, const HMessagingInfo& mi);

    static QByteArray createHeaderData(
        StatusCode, const HMessagingInfo&, qint64 bodySizeInBytes, ContentType);

    static QByteArray createResponse(
        StatusCode, const HMessagingInfo&, const QByteArray& body,
        ContentType);

    static QByteArray createResponse(
        const HMessagingInfo&, qint32 actionErrCode, const QString& msg=QString());

    static QByteArray create(const HNotifyRequest&     , HMessagingInfo*);
    static QByteArray create(const HSubscribeRequest&  , const HMessagingInfo&);
    static QByteArray create(const HUnsubscribeRequest&, HMessagingInfo*);
    static QByteArray create(const HSubscribeResponse& , const HMessagingInfo&);

    static int create(
        const HHttpRequestHeader& reqHdr, const QByteArray& body,
        HNotifyRequest& req);

    static int create(
        const HHttpRequestHeader& reqHdr, HSubscribeRequest& req);

    static int create(
        const HHttpRequestHeader& reqHdr, HUnsubscribeRequest& req);

    static bool create(
        const HHttpResponseHeader& respHdr, HSubscribeResponse& resp);
};

}
}

#endif /* HHTTP_MESSAGECREATOR_H_ */
