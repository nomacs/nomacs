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

#include "hcontentdirectory_adapter.h"
#include "hcontentdirectory_adapter_p.h"
#include "hcontentdirectory_info.h"

#include "../hav_devicemodel_infoprovider.h"

#include <HUpnpCore/HActionInfo>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/HStateVariableEvent>
#include <HUpnpCore/HClientStateVariable>

#include <HUpnpCore/private/hlogger_p.h>

#include <QtCore/QUrl>
#include <QtCore/QSet>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HContentDirectoryAdapterPrivate
 ******************************************************************************/
HContentDirectoryAdapterPrivate::HContentDirectoryAdapterPrivate() :
    HClientServiceAdapterPrivate(HContentDirectoryInfo::supportedServiceType())
{
}

HContentDirectoryAdapterPrivate::~HContentDirectoryAdapterPrivate()
{
}

bool HContentDirectoryAdapterPrivate::getSearchCapabilities(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QStringList capabilities =
        op.outputArguments().value("SearchCaps").toString().split(",");

    capabilities.removeAll("");

    emit q->getSearchCapabilitiesCompleted(q, takeOp(op, capabilities));

    return false;
}

bool HContentDirectoryAdapterPrivate::getSortCapabilities(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QStringList capabilities =
        op.outputArguments().value("SortCaps").toString().split(",");

    emit q->getSortCapabilitiesCompleted(q, takeOp(op, capabilities));

    return false;
}

bool HContentDirectoryAdapterPrivate::getSortExtensionCapabilities(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QStringList capabilities =
        op.outputArguments().value("SortExtensionCaps").toString().split(",");

    emit q->getSortExtensionCapabilitiesCompleted(q, takeOp(op, capabilities));

    return false;
}

bool HContentDirectoryAdapterPrivate::getFeatureList(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QString features = op.outputArguments().value("FeatureList").toString();

    emit q->getFeatureListCompleted(q, takeOp(op, features));

    return false;
}

bool HContentDirectoryAdapterPrivate::getSystemUpdateID(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    quint32 updateId = op.outputArguments().value("Id").toUInt();
    emit q->getSystemUpdateIdCompleted(q, takeOp(op, updateId));

    return false;
}

bool HContentDirectoryAdapterPrivate::getServiceResetToken(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QString resetToken =
        op.outputArguments().value("ResetToken").toString();

    emit q->getServiceResetTokenCompleted(q, takeOp(op, resetToken));

    return false;
}

bool HContentDirectoryAdapterPrivate::browse(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    HSearchResult searchResult;
    if (op.returnValue() == UpnpSuccess)
    {
        HActionArguments outArgs = op.outputArguments();
        QString result = outArgs.value("Result").toString();
        quint32 numReturned = outArgs.value("NumberReturned").toUInt();
        quint32 totalMatches = outArgs.value("TotalMatches").toUInt();
        quint32 updateId = outArgs.value("UpdateID").toUInt();

        searchResult = HSearchResult(result, numReturned, totalMatches, updateId);
    }
    emit q->browseCompleted(q, takeOp(op, searchResult));

    return false;
}

bool HContentDirectoryAdapterPrivate::search(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    HSearchResult searchResult;
    if (op.returnValue() == UpnpSuccess)
    {
        HActionArguments outArgs = op.outputArguments();
        QString result = outArgs.value("Result").toString();
        quint32 numReturned = outArgs.value("NumberReturned").toUInt();
        quint32 totalMatches = outArgs.value("TotalMatches").toUInt();
        quint32 updateId = outArgs.value("UpdateID").toUInt();

        searchResult = HSearchResult(result, numReturned, totalMatches, updateId);
    }
    emit q->searchCompleted(q, takeOp(op, searchResult));

    return false;
}

bool HContentDirectoryAdapterPrivate::createObject(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    HActionArguments outArgs = op.outputArguments();

    QString objectId = outArgs.value("ObjectID").toString();
    QString result = outArgs.value("Result").toString();

    HCreateObjectResult coResult(objectId, result);

    emit q->createObjectCompleted(q, takeOp(op, coResult));

    return false;
}

bool HContentDirectoryAdapterPrivate::destroyObject(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);
    emit q->destroyObjectCompleted(q, takeOp(op));
    return false;
}

bool HContentDirectoryAdapterPrivate::updateObject(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    emit q->updateObjectCompleted(q, takeOp(op));

    return false;
}

bool HContentDirectoryAdapterPrivate::moveObject(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QString newObjectId =
        op.outputArguments().value("NewObjectId").toString();

    emit q->moveObjectCompleted(q, takeOp(op, newObjectId));

    return false;
}

bool HContentDirectoryAdapterPrivate::importResource(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    quint32 transferId = op.outputArguments().value("TransferID").toUInt();
    emit q->importResourceCompleted(q, takeOp(op, transferId));

    return false;
}

bool HContentDirectoryAdapterPrivate::exportResource(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    quint32 transferId = op.outputArguments().value("TransferID").toUInt();
    emit q->exportResourceCompleted(q, takeOp(op, transferId));

    return false;
}

bool HContentDirectoryAdapterPrivate::deleteResource(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    emit q->deleteResourceCompleted(q, takeOp(op));

    return false;
}

bool HContentDirectoryAdapterPrivate::stopTransferResource(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    emit q->stopTransferResourceCompleted(q, takeOp(op));

    return false;
}

bool HContentDirectoryAdapterPrivate::getTransferProgress(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    HTransferProgressInfo info;
    if (op.returnValue() == UpnpSuccess)
    {
        HActionArguments outArgs = op.outputArguments();

        HTransferProgressInfo::Status ts =
            HTransferProgressInfo::fromString(
                op.outputArguments().value("TransferStatus").toString());

        quint32 tl = outArgs.value("TransferLength").toUInt();
        quint32 tt = outArgs.value("TransferTotal").toUInt();

        info = HTransferProgressInfo(tl, ts, tt);
    }
    emit q->getTransferProgressCompleted(q, takeOp(op, info));

    return false;
}

bool HContentDirectoryAdapterPrivate::createReference(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QString newId = op.outputArguments().value("NewID").toString();
    emit q->createReferenceCompleted(q, takeOp(op, newId));

    return false;
}

bool HContentDirectoryAdapterPrivate::freeFormQuery(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    HActionArguments outArgs = op.outputArguments();

    QString queryResult = outArgs.value("QueryResult").toString();
    quint32 updateId = outArgs.value("UpdateID").toUInt();

    HFreeFormQueryResult result(queryResult, updateId);

    emit q->freeFormQueryCompleted(q, takeOp(op, result));

    return false;
}

bool HContentDirectoryAdapterPrivate::getFreeFormQueryCapabilities(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HContentDirectoryAdapter);

    QString ffqCapabilities =
        op.outputArguments().value("FFQCapabilities").toString();

    emit q->getFreeFormQueryCapabilitiesCompleted(
        q, takeOp(op, ffqCapabilities));

    return false;
}

/*******************************************************************************
 * HContentDirectoryAdapter
 ******************************************************************************/
HContentDirectoryAdapter::HContentDirectoryAdapter(QObject* parent) :
    HClientServiceAdapter(*new HContentDirectoryAdapterPrivate(), parent)
{
    HAvDeviceModelInfoProvider infoProvider;
    setDeviceModelInfoProvider(infoProvider);
}

HContentDirectoryAdapter::~HContentDirectoryAdapter()
{
}

void HContentDirectoryAdapter::lastChange(
    const HClientStateVariable*, const HStateVariableEvent& event)
{
    emit lastChangeReceived(this, event.newValue().toString());
}

bool HContentDirectoryAdapter::prepareService(HClientService* service)
{
    const HClientStateVariable* lastChange = service->stateVariables().value("LastChange");
    if (lastChange)
    {
        bool ok = connect(
            lastChange,
            SIGNAL(valueChanged(const Herqq::Upnp::HClientStateVariable*,Herqq::Upnp::HStateVariableEvent)),
            this,
            SLOT(lastChange(const Herqq::Upnp::HClientStateVariable*,Herqq::Upnp::HStateVariableEvent)));
        Q_ASSERT(ok); Q_UNUSED(ok)
    }
    return true;
}

bool HContentDirectoryAdapter::isLastChangeEnabled() const
{
    return h_ptr->m_service->stateVariables().contains("LastChange");
}

HClientAdapterOp<QStringList> HContentDirectoryAdapter::getSearchCapabilities()
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetSearchCapabilities", &rc);
    if (!action)
    {
        return HClientAdapterOp<QStringList>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    return h_ptr->beginInvoke<QStringList>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getSearchCapabilities));
}

HClientAdapterOp<QStringList> HContentDirectoryAdapter::getSortCapabilities()
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetSortCapabilities", &rc);
    if (!action)
    {
        return HClientAdapterOp<QStringList>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    return h_ptr->beginInvoke<QStringList>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getSortCapabilities));
}

HClientAdapterOp<QStringList> HContentDirectoryAdapter::getSortExtensionCapabilities()
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetSortExtensionCapabilities", &rc);
    if (!action)
    {
        return HClientAdapterOp<QStringList>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    return h_ptr->beginInvoke<QStringList>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getSortExtensionCapabilities));
}

HClientAdapterOp<QString> HContentDirectoryAdapter::getFeatureList()
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetFeatureList", &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    return h_ptr->beginInvoke<QString>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getFeatureList));
}

HClientAdapterOp<quint32> HContentDirectoryAdapter::getSystemUpdateId()
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetSystemUpdateID", &rc);
    if (!action)
    {
        return HClientAdapterOp<quint32>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    return h_ptr->beginInvoke<quint32>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getSystemUpdateID));
}

HClientAdapterOp<QString> HContentDirectoryAdapter::getServiceResetToken()
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetServiceResetToken", &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();

    return h_ptr->beginInvoke<QString>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getServiceResetToken));
}

HClientAdapterOp<HSearchResult> HContentDirectoryAdapter::browse(
    const QString& objectId,
    HContentDirectoryInfo::BrowseFlag browseFlag,
    const QSet<QString>& filter,
    quint32 startingIndex,
    quint32 requestedCount,
    const QStringList& sortCriteria)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Browse", &rc);
    if (!action)
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ObjectID", objectId))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("BrowseFlag", HContentDirectoryInfo::browseFlagToString(browseFlag)))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("Filter", QStringList(filter.toList()).join(",")))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("StartingIndex", startingIndex))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("RequestedCount", requestedCount))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("SortCriteria", sortCriteria.join(",")))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<HSearchResult>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::browse));
}

HClientAdapterOp<HSearchResult> HContentDirectoryAdapter::search(
    const QString& containerId,
    const QString& searchCriteria,
    const QSet<QString>& filter,
    quint32 startingIndex,
    quint32 requestedCount,
    const QStringList& sortCriteria)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Search", &rc);
    if (!action)
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ContainerID", containerId))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("SearchCriteria", searchCriteria))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("Filter", QStringList(filter.toList()).join(",")))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("StartingIndex", startingIndex))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("RequestedCount", requestedCount))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("SortCriteria", sortCriteria.join(",")))
    {
        return HClientAdapterOp<HSearchResult>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<HSearchResult>(
            action, inArgs,
            HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::search));
}

HClientAdapterOp<HCreateObjectResult> HContentDirectoryAdapter::createObject(
    const QString& containerId, const QString& elements)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("CreateObject", &rc);
    if (!action)
    {
        return HClientAdapterOp<HCreateObjectResult>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ContainerID", containerId))
    {
        return HClientAdapterOp<HCreateObjectResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("Elements", elements))
    {
        return HClientAdapterOp<HCreateObjectResult>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<HCreateObjectResult>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::createObject));
}

HClientAdapterOpNull HContentDirectoryAdapter::destroyObject(const QString& objectId)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("DestroyObject", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ObjectID", objectId))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::destroyObject));
}

HClientAdapterOpNull HContentDirectoryAdapter::updateObject(
    const QString& objectId,
    const QStringList& currentTagValues,
    const QStringList& newTagValues)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("UpdateObject", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ObjectID", objectId))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("CurrentTagValue", currentTagValues.join(",")))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("NewTagValue", newTagValues.join(",")))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::updateObject));
}

HClientAdapterOp<QString> HContentDirectoryAdapter::moveObject(
    const QString& objectId, const QString& newParentId)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("MoveObject", &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ObjectID", objectId))
    {
        return HClientAdapterOp<QString>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("NewParentID", newParentId))
    {
        return HClientAdapterOp<QString>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<QString>(
        action, inArgs, HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::moveObject));
}

HClientAdapterOp<quint32> HContentDirectoryAdapter::importResource(
    const QUrl& source, const QUrl& destination)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("ImportResource", &rc);
    if (!action)
    {
        return HClientAdapterOp<quint32>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("SourceURI", source))
    {
        return HClientAdapterOp<quint32>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("DestinationURI", destination))
    {
        return HClientAdapterOp<quint32>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<quint32>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::importResource));
}

HClientAdapterOp<quint32> HContentDirectoryAdapter::exportResource(
    const QUrl& source, const QUrl& destination)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("ExportResource", &rc);
    if (!action)
    {
        return HClientAdapterOp<quint32>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("SourceURI", source))
    {
        return HClientAdapterOp<quint32>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("DestinationURI", destination))
    {
        return HClientAdapterOp<quint32>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<quint32>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::exportResource));
}

HClientAdapterOpNull HContentDirectoryAdapter::deleteResource(const QUrl& resourceUrl)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("DeleteResource", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ResourceURI", resourceUrl))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::deleteResource));
}

HClientAdapterOpNull HContentDirectoryAdapter::stopTransferResource(quint32 transferId)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("StopTransferResource", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("TransferID", transferId);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::stopTransferResource));
}

HClientAdapterOp<HTransferProgressInfo> HContentDirectoryAdapter::getTransferProgress(quint32 transferId)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetTransferProgress", &rc);
    if (!action)
    {
        return HClientAdapterOp<HTransferProgressInfo>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("TransferID", transferId);

    return h_ptr->beginInvoke<HTransferProgressInfo>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getTransferProgress));
}

HClientAdapterOp<QString> HContentDirectoryAdapter::createReference(
    const QString& containerId, const QString& objectId)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("CreateReference", &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ContainerID", containerId))
    {
        return HClientAdapterOp<QString>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("ObjectID", objectId))
    {
        return HClientAdapterOp<QString>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<QString>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::createReference));
}

HClientAdapterOp<HFreeFormQueryResult> HContentDirectoryAdapter::freeFormQuery(
    const QString& containerId, quint32 cdsView, const QString& queryRequest)
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("FreeFormQuery", &rc);
    if (!action)
    {
        return HClientAdapterOp<HFreeFormQueryResult>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue("ContainerID", containerId))
    {
        return HClientAdapterOp<HFreeFormQueryResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("CDSView", cdsView))
    {
        return HClientAdapterOp<HFreeFormQueryResult>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("QueryRequest", queryRequest))
    {
        return HClientAdapterOp<HFreeFormQueryResult>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<HFreeFormQueryResult>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::freeFormQuery));
}

HClientAdapterOp<QString> HContentDirectoryAdapter::getFreeFormQueryCapabilities()
{
    H_D(HContentDirectoryAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetFreeFormQueryCapabilities", &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();

    return h_ptr->beginInvoke<QString>(
        action, inArgs,
        HActionInvokeCallback(h, &HContentDirectoryAdapterPrivate::getFreeFormQueryCapabilities));
}

}
}
}
