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

#include "habstractcontentdirectory_service.h"
#include "habstractcontentdirectory_service_p.h"

#include "hsearchresult.h"
#include "hcreateobjectresult.h"
#include "hfreeformqueryresult.h"
#include "htransferprogressinfo.h"

#include "../common/hprotocolinfo.h"
#include "../cds_model/hsortinfo.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HServerStateVariable>

#include <QtCore/QSet>
#include <QtCore/QUuid>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractContentDirectoryServicePrivate
 ******************************************************************************/
HAbstractContentDirectoryServicePrivate::HAbstractContentDirectoryServicePrivate() :
    HServerServicePrivate()
{
}

HAbstractContentDirectoryServicePrivate::~HAbstractContentDirectoryServicePrivate()
{
}

qint32 HAbstractContentDirectoryServicePrivate::getSearchCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QStringList searchCapabilities;
    qint32 retVal = q->getSearchCapabilities(&searchCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("SearchCaps", searchCapabilities.join(","));
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getSortCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QStringList sortCapabilities;
    qint32 retVal = q->getSortCapabilities(&sortCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("SortCaps", sortCapabilities.join(","));
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getSortExtensionCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QStringList sortExtCapabilities;
    qint32 retVal = q->getSortExtensionCapabilities(&sortExtCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("SortExtensionCaps", sortExtCapabilities.join(","));
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getFeatureList(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString featureList;
    qint32 retVal = q->getFeatureList(&featureList);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("FeatureList", featureList);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getSystemUpdateID(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 systemUpdateId;
    qint32 retVal = q->getSystemUpdateId(&systemUpdateId);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("Id", systemUpdateId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getServiceResetToken(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString serviceResetToken;
    qint32 retVal = q->getServiceResetToken(&serviceResetToken);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("ResetToken", serviceResetToken);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::browse(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HSearchResult result;
    qint32 retVal = q->browse(
           inArgs.value("ObjectID").toString(),
           HContentDirectoryInfo::browseFlagFromString(inArgs.value("BrowseFlag").toString()),
           inArgs.value("Filter").toString().split(',').toSet(),
           inArgs.value("StartingIndex").toUInt(),
           inArgs.value("RequestedCount").toUInt(),
           inArgs.value("SortCriteria").toString().split(',', QString::SkipEmptyParts),
           &result);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("Result", result.result());
        outArgs->setValue("NumberReturned", result.numberReturned());
        outArgs->setValue("TotalMatches", result.totalMatches());
        outArgs->setValue("UpdateID", result.updateId());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::search(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HSearchResult result;
    qint32 retVal = q->search(
          inArgs.value("ContainerID").toString(),
          inArgs.value("SearchCriteria").toString(),
          inArgs.value("Filter").toString().split(',').toSet(),
          inArgs.value("StartingIndex").toUInt(),
          inArgs.value("RequestedCount").toUInt(),
          inArgs.value("SortCriteria").toString().split(',', QString::SkipEmptyParts),
          &result);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("Result", result.result());
        outArgs->setValue("NumberReturned", result.numberReturned());
        outArgs->setValue("TotalMatches", result.totalMatches());
        outArgs->setValue("UpdateID", result.updateId());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::createObject(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HCreateObjectResult result;
    qint32 retVal = q->createObject(
        inArgs.value("ContainerID").toString(),
        inArgs.value("Elements").toString(),
        &result);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("ObjectID", result.objectId());
        outArgs->setValue("Result", result.result());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::destroyObject(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);
    return q->destroyObject(inArgs.value("ObjectID").toString());
}

qint32 HAbstractContentDirectoryServicePrivate::updateObject(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);
    return q->updateObject(
        inArgs.value("ObjectID").toString(),
        inArgs.value("CurrentTagValue").toString().split(','),
        inArgs.value("NewTagValue").toString().split(','));
}

qint32 HAbstractContentDirectoryServicePrivate::moveObject(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString newObjectId;
    qint32 retVal = q->moveObject(
        inArgs.value("ObjectID").toString(),
        inArgs.value("NewParentID").toString(),
        &newObjectId);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("NewObjectId", newObjectId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::importResource(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    quint32 transferId;
    qint32 retVal = q->importResource(
        inArgs.value("SourceURI").toUrl(),
        inArgs.value("DestinationURI").toUrl(),
        &transferId);

    if (retVal == UpnpSuccess && outArgs)
    {
        outArgs->setValue("TransferID", transferId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::exportResource(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    quint32 transferId;
    qint32 retVal = q->exportResource(
        inArgs.value("SourceURI").toUrl(),
        inArgs.value("DestinationURI").toUrl(),
        &transferId);

    if (retVal == UpnpSuccess && outArgs)
    {
        outArgs->setValue("TransferID", transferId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::deleteResource(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);
    return q->deleteResource(inArgs.value("ResourceURI").toUrl());
}

qint32 HAbstractContentDirectoryServicePrivate::stopTransferResource(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);
    return q->stopTransferResource(inArgs.value("TransferID").toUInt());
}

qint32 HAbstractContentDirectoryServicePrivate::getTransferProgress(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HTransferProgressInfo info;
    qint32 retVal = q->getTransferProgress(inArgs.value("TransferID").toUInt(), &info);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("TransferStatus", info.status());
        outArgs->setValue("TransferLength", info.length());
        outArgs->setValue("TransferTotal", info.total());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::createReference(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString newId;
    qint32 retVal = q->createReference(
       inArgs.value("ContainerID").toString(),
       inArgs.value("ObjectID").toString(),
       &newId);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("NewID", newId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::freeFormQuery(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HFreeFormQueryResult queryResult;
    qint32 retVal = q->freeFormQuery(
        inArgs.value("ContainerID").toString(),
        inArgs.value("CDSView").toUInt(),
        inArgs.value("QueryRequest").toString(),
        &queryResult);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("QueryResult", queryResult.queryResult());
        outArgs->setValue("UpdateID", queryResult.updateId());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getFreeFormQueryCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString ffqCapabilities;
    qint32 retVal = q->getFreeFormQueryCapabilities(&ffqCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("FFQCapabilities", ffqCapabilities);
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractContentDirectoryService
 ******************************************************************************/
HAbstractContentDirectoryService::HAbstractContentDirectoryService(
    HAbstractContentDirectoryServicePrivate& dd) :
        HServerService(dd)
{
}

HAbstractContentDirectoryService::HAbstractContentDirectoryService() :
    HServerService(*new HAbstractContentDirectoryServicePrivate())
{
}

HAbstractContentDirectoryService::~HAbstractContentDirectoryService()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
}

HServerService::HActionInvokes HAbstractContentDirectoryService::createActionInvokes()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HAbstractContentDirectoryService);

    HActionInvokes retVal;

    retVal.insert("GetSearchCapabilities",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSearchCapabilities));

    retVal.insert("GetSortCapabilities",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSortCapabilities));

    retVal.insert("GetSortExtensionCapabilities",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSortExtensionCapabilities));

    retVal.insert("GetFeatureList",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getFeatureList));

    retVal.insert("GetSystemUpdateID",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSystemUpdateID));

    retVal.insert("GetServiceResetToken",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getServiceResetToken));

    retVal.insert("Browse",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::browse));

    retVal.insert("Search",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::search));

    retVal.insert("CreateObject",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::createObject));

    retVal.insert("DestroyObject",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::destroyObject));

    retVal.insert("UpdateObject",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::updateObject));

    retVal.insert("MoveObject",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::moveObject));

    retVal.insert("ImportResource",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::importResource));

    retVal.insert("ExportResource",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::exportResource));

    retVal.insert("DeleteResource",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::deleteResource));

    retVal.insert("StopTransferResource",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::stopTransferResource));

    retVal.insert("GetTransferProgress",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getTransferProgress));

    retVal.insert("CreateReference",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::createReference));

    retVal.insert("FreeFormQuery",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::freeFormQuery));

    retVal.insert("GetFreeFormQueryCapabilities",
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getFreeFormQueryCapabilities));

    return retVal;
}

bool HAbstractContentDirectoryService::finalizeInit(QString* errDescription)
{
    if (!HServerService::finalizeInit(errDescription))
    {
        return false;
    }

    stateVariables().value("ServiceResetToken")->setValue(
        QUuid::createUuid().toString().remove("{").remove("}"));

    return true;
}

qint32 HAbstractContentDirectoryService::getSortExtensionCapabilities(
    QStringList* /*oarg*/) const
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::search(
    const QString& /*containerId*/,
    const QString& /*searchCriteria*/,
    const QSet<QString>& /*filter*/,
    quint32 /*startingIndex*/,
    quint32 /*requestedCount*/,
    const QStringList& /*sortCriteria*/,
    HSearchResult* /*result*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::createObject(
    const QString& /*containerId*/,
    const QString& /*elements*/,
    HCreateObjectResult*)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::destroyObject(const QString& /*objectId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::updateObject(
    const QString& /*objectId*/,
    const QStringList& /*currentTagValues*/,
    const QStringList& /*newTagValues*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::moveObject(
    const QString& /*objectId*/,
    const QString& /*newParentId*/,
    QString* /*newObjectId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::importResource(
    const QUrl& /*source*/, const QUrl& /*destination*/, quint32* /*transferId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::exportResource(
    const QUrl& /*source*/, const QUrl& /*destination*/, quint32* /*transferId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::deleteResource(const QUrl& /*resourceUrl*/)
{return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::stopTransferResource(quint32 /*transferId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::getTransferProgress(
    quint32 /*transferId*/, HTransferProgressInfo* /*transferInfo*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::createReference(
    const QString& /*containerId*/, const QString& /*objectId*/,
    QString* /*newId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::freeFormQuery(
    const QString& /*containerId*/, quint32 /*cdsView*/,
    const QString& /*queryRequest*/, HFreeFormQueryResult*)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::getFreeFormQueryCapabilities(
    QString* /*ffqCapabilities*/)
{
    return UpnpOptionalActionNotImplemented;
}

}
}
}
