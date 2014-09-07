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

#include "hdevicehost_http_server_p.h"
#include "hevent_subscriber_p.h"

#include "../messages/hcontrol_messages_p.h"

#include "../../http/hhttp_messagecreator_p.h"

#include "../../general/hupnp_global_p.h"
#include "../../general/hupnp_datatypes_p.h"

#include "../../devicemodel/hactionarguments.h"
#include "../../devicemodel/server/hserveraction.h"
#include "../../devicemodel/server/hserverdevice.h"
#include "../../devicemodel/server/hserverservice.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hactioninfo.h"
#include "../../dataelements/hserviceinfo.h"

#include "../../general/hlogger_p.h"

#include <QtCore/QUrl>
#include <QtCore/QPair>

namespace Herqq
{

namespace Upnp
{

namespace
{
QUuid extractUdn(const QUrl& arg)
{
    QString path = extractRequestPart(arg);

    QUuid udn(path.section('/', 1, 1));
    if (udn.isNull())
    {
        return QUuid();
    }

    return udn;
}

inline QString extractRequestExludingUdn(const QUrl& arg)
{
    QString pathToSearch = extractRequestPart(arg).section(
        '/', 2, -1, QString::SectionIncludeLeadingSep);

    return pathToSearch;
}
}

/*******************************************************************************
 * HDeviceHostHttpServer
 ******************************************************************************/
HDeviceHostHttpServer::HDeviceHostHttpServer(
    const QByteArray& loggingId, const QString& ddPostFix,
    HDeviceStorage<HServerDevice, HServerService, HServerDeviceController>& ds,
    HEventNotifier& en,
    QObject* parent) :
        HHttpServer(loggingId, parent),
            m_deviceStorage(ds), m_eventNotifier(en), m_ddPostFix(ddPostFix),
            m_ops()
{
}

HDeviceHostHttpServer::~HDeviceHostHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<QPair<QPointer<HHttpAsyncOperation>, HOpInfo> >::iterator it = m_ops.begin();
    for(; it != m_ops.end(); ++it)
    {
        if (it->first)
        {
            it->first->deleteLater();
        }
    }
}

void HDeviceHostHttpServer::incomingSubscriptionRequest(
    HMessagingInfo* mi, const HSubscribeRequest& sreq)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG("Subscription received.");

    QUuid udn = extractUdn(sreq.eventUrl());

    HServerDevice* device =
        !udn.isNull() ? m_deviceStorage.searchDeviceByUdn(HUdn(udn), AllDevices) : 0;

    HServerService* service = 0;

    if (!device)
    {
        // the request did not have the UDN prefix, which means that either
        // 1) the request was for a EventUrl that was defined as an absolute URL
        //    in the device description or
        // 2) the request is invalid

        service = m_deviceStorage.searchServiceByEventUrl(sreq.eventUrl());
        if (!service)
        {
            HLOG_WARN(QString(
                "Ignoring invalid event subscription to: [%1].").arg(
                    sreq.eventUrl().toString()));

            mi->setKeepAlive(false);
            m_httpHandler->send(
                mi, HHttpMessageCreator::createResponse(BadRequest, *mi));

            return;
        }
    }
    else if (!service)
    {
        service = m_deviceStorage.searchServiceByEventUrl(
            device, extractRequestExludingUdn(sreq.eventUrl()));
    }

    if (!service)
    {
        HLOG_WARN(QString("Subscription defined as [%1] is invalid.").arg(
            sreq.eventUrl().path()));

        mi->setKeepAlive(false);
        m_httpHandler->send(
            mi, HHttpMessageCreator::createResponse(BadRequest, *mi));

        return;
    }

    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.

    HSid sid;
    StatusCode sc;
    if (sreq.isRenewal())
    {
        sc = m_eventNotifier.renewSubscription(sreq, &sid);
    }
    else
    {
        sc = m_eventNotifier.addSubscriber(service, sreq, &sid);
    }

    if (sc != Ok)
    {
        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(sc, *mi));
        return;
    }

    HServiceEventSubscriber* subscriber = m_eventNotifier.remoteClient(sid);

    HSubscribeResponse response(
        subscriber->sid(),
        HSysInfo::instance().herqqProductTokens(),
        subscriber->timeout());

    HHttpAsyncOperation* op =
        m_httpHandler->send(mi, HHttpMessageCreator::create(response, *mi));

    if (op)
    {
        HOpInfo opInfo(service, sreq, subscriber);
        m_ops.append(qMakePair(QPointer<HHttpAsyncOperation>(op), opInfo));
    }
}

void HDeviceHostHttpServer::incomingUnsubscriptionRequest(
    HMessagingInfo* mi, const HUnsubscribeRequest& usreq)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_DBG("Unsubscription received.");

    bool ok = m_eventNotifier.removeSubscriber(usreq);

    mi->setKeepAlive(false);
    m_httpHandler->send(
        mi, HHttpMessageCreator::createResponse(ok ? Ok : PreconditionFailed, *mi));
}

void HDeviceHostHttpServer::incomingControlRequest(
    HMessagingInfo* mi, const HInvokeActionRequest& invokeActionRequest)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QString("Control message to [%1] received.").arg(
        invokeActionRequest.soapAction()));

    QUuid udn = extractUdn(invokeActionRequest.serviceUrl());

    HServerDevice* device =
        !udn.isNull() ? m_deviceStorage.searchDeviceByUdn(HUdn(udn), AllDevices) : 0;

    HServerService* service = 0;

    if (!device)
    {
        // the request did not have the UDN prefix, which means that either
        // 1) the request was for a ControlURL that was defined as an absolute URL
        //    in the device description or
        // 2) the request is invalid

        service = m_deviceStorage.searchServiceByControlUrl(
            invokeActionRequest.serviceUrl());

        if (!service)
        {
            HLOG_WARN(QString(
                "Ignoring invalid action invocation to: [%1].").arg(
                    invokeActionRequest.serviceUrl().toString()));

            mi->setKeepAlive(false);

            m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
                BadRequest, *mi));

            return;
        }
    }
    else if (!service)
    {
        service = m_deviceStorage.searchServiceByControlUrl(
            device, extractRequestExludingUdn(invokeActionRequest.serviceUrl()));
    }

    if (!service)
    {
        HLOG_WARN(QString("Ignoring invalid action invocation to: [%1].").arg(
            invokeActionRequest.serviceUrl().toString()));

        mi->setKeepAlive(false);

        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
            BadRequest, *mi));

        return;
    }

    const QtSoapMessage* soapMsg = invokeActionRequest.soapMsg();
    const QtSoapType& method = soapMsg->method();
    if (!method.isValid())
    {
        HLOG_WARN("Invalid control method.");

        mi->setKeepAlive(false);

        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
            BadRequest, *mi));

        return;
    }

    HServerAction* action = service->actions().value(method.name().name());

    if (!action)
    {
        HLOG_WARN(QString("The service has no action named [%1].").arg(
            method.name().name()));

        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
            *mi, UpnpInvalidArgs, soapMsg->toXmlString()));

        return;
    }

    HActionArguments iargs = action->info().inputArguments();
    HActionArguments::iterator it = iargs.begin();
    for(; it != iargs.end(); ++it)
    {
        HActionArgument iarg = *it;

        const QtSoapType& arg = method[iarg.name()];
        if (!arg.isValid())
        {
            mi->setKeepAlive(false);
            m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
                *mi, UpnpInvalidArgs, soapMsg->toXmlString()));

            return;
        }

        if (!iarg.setValue(
                HUpnpDataTypes::convertToRightVariantType(
                    arg.value().toString(), iarg.dataType())))
        {
            mi->setKeepAlive(false);
            m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
                *mi, UpnpInvalidArgs, soapMsg->toXmlString()));

            return;
        }
    }

    HActionArguments outArgs = action->info().outputArguments();
    qint32 retVal = action->invoke(iargs, &outArgs);
    if (retVal != UpnpSuccess)
    {
        mi->setKeepAlive(false);
        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
            *mi, retVal, soapMsg->toXmlString()));

        return;
    }

    QtSoapNamespaces::instance().registerNamespace(
        "u", service->info().serviceType().toString());

    QtSoapMessage soapResponse;
    soapResponse.setMethod(QtSoapQName(
        QString("%1%2").arg(action->info().name(), "Response"),
        service->info().serviceType().toString()));

    foreach(const HActionArgument& oarg, outArgs)
    {
        QtSoapType* soapArg =
            new SoapType(oarg.name(), oarg.dataType(), oarg.value());

        soapResponse.addMethodArgument(soapArg);
    }

    QString xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n" + soapResponse.toXmlString();

    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
        Ok, *mi, xml.toUtf8(), ContentType_TextXml));

    HLOG_DBG("Control message successfully handled.");
}

void HDeviceHostHttpServer::incomingUnknownGetRequest(
    HMessagingInfo* mi, const HHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString peer = peerAsStr(mi->socket());
    QString requestPath = requestHdr.path();

    HLOG_DBG(QString(
        "HTTP GET request received from [%1] to [%2].").arg(peer, requestPath));

    QUuid searchedUdn(requestPath.section('/', 1, 1));
    if (searchedUdn.isNull())
    {
        // the request did not have the UDN prefix, which means that either
        // 1) the request was for a SCPD that was defined with an absolute URL
        //    in the device description or
        // 2) the request is invalid

        HServerService* service =
            m_deviceStorage.searchServiceByScpdUrl(requestPath);

        if (service)
        {
            HLOG_DBG(QString(
                "Sending service description to [%1] as requested.").arg(peer));

            m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
                Ok, *mi, service->description().toUtf8(), ContentType_TextXml));

            return;
        }

        HLOG_WARN(QString("Responding NOT_FOUND [%1] to [%2].").arg(
            requestHdr.path(), peerAsStr(mi->socket())));

        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(NotFound, *mi));
        return;
    }

    HServerDevice* device =
        m_deviceStorage.searchDeviceByUdn(HUdn(searchedUdn), AllDevices);

    if (!device)
    {
        HLOG_WARN(QString("Responding NOT_FOUND [%1] to [%2].").arg(
            requestHdr.path(), peerAsStr(mi->socket())));

        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(NotFound, *mi));
        return;
    }

    if (requestPath.endsWith(m_ddPostFix))
    {
        HLOG_DBG(QString(
            "Sending device description to [%1] as requested.").arg(peer));

        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
            Ok, *mi, device->description().toUtf8(), ContentType_TextXml));

        return;
    }

    QString extractedRequestPart = extractRequestExludingUdn(requestPath);

    HServerService* service =
        m_deviceStorage.searchServiceByScpdUrl(device, extractedRequestPart);

    if (service)
    {
        HLOG_DBG(QString(
            "Sending service description to [%1] as requested.").arg(peer));

        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
            Ok, *mi, service->description().toUtf8(), ContentType_TextXml));

        return;
    }

    QUrl icon = m_deviceStorage.seekIcon(device, extractedRequestPart);

    if (!icon.isEmpty())
    {
        QFile iconFile(icon.toLocalFile());
        if (!iconFile.open(QIODevice::ReadOnly))
        {
            HLOG_WARN(QString("Could not open icon file.").arg(icon.toLocalFile()));
            m_httpHandler->send(mi, HHttpMessageCreator::createResponse(InternalServerError, *mi));
            return;
        }

        HLOG_DBG(QString("Sending icon to [%1] as requested.").arg(peer));

        m_httpHandler->send(mi, HHttpMessageCreator::createResponse(
            Ok, *mi, iconFile.readAll(), ContentType_TextXml));

        return;
    }

    HLOG_WARN(QString("Responding NOT_FOUND [%1] to [%2].").arg(
        requestHdr.path(), peerAsStr(mi->socket())));

    m_httpHandler->send(mi, HHttpMessageCreator::createResponse(NotFound, *mi));
}

bool HDeviceHostHttpServer::sendComplete(HHttpAsyncOperation* op)
{
    HOpInfo opInfo;
    QList<QPair<QPointer<HHttpAsyncOperation>, HOpInfo> >::iterator it = m_ops.begin();
    for(; it != m_ops.end(); ++it)
    {
        if (it->first == op)
        {
            opInfo = it->second;
            break;
        }
    }

    if (opInfo.isValid())
    {
        if (opInfo.m_service->isEvented() && !opInfo.m_req.isRenewal())
        {
            // by now the UnicastRemoteClient for the subscriber is created if everything
            // went well and we can attempt to send the initial event message

            m_eventNotifier.initialNotify(
                opInfo.m_subscriber, op->takeMessagingInfo());
        }

        m_ops.erase(it);
        return false;
    }

    return true;
}

}
}
