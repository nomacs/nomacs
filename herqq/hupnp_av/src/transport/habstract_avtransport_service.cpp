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

#include "habstract_avtransport_service.h"
#include "habstract_avtransport_service_p.h"

#include "hduration.h"
#include "hplaymode.h"
#include "hseekinfo.h"
#include "hmediainfo.h"
#include "hpositioninfo.h"
#include "htransportinfo.h"
#include "htransportstate.h"
#include "htransportaction.h"
#include "htransportsettings.h"
#include "hrecordqualitymode.h"
#include "hdevicecapabilities.h"
#include "hrecordmediumwritestatus.h"

#include "../common/hstoragemedium.h"
#include "../mediarenderer/hrendererconnection_info.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HActionArguments>

#include <QtCore/QSet>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractTransportServicePrivate
 ******************************************************************************/
HAbstractTransportServicePrivate::HAbstractTransportServicePrivate()
{
}

HAbstractTransportServicePrivate::~HAbstractTransportServicePrivate()
{
}

qint32 HAbstractTransportServicePrivate::setAVTransportURI(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QString currentUri = inArgs.value("CurrentURI").toString();
    QString metadata = inArgs.value("CurrentURIMetaData").toString();

    return q->setAVTransportURI(instanceId, currentUri, metadata);
}

qint32 HAbstractTransportServicePrivate::setNextAVTransportURI(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QString currentUri = inArgs.value("NextURI").toString();
    QString metadata = inArgs.value("NextURIMetaData").toString();

    return q->setNextAVTransportURI(instanceId, currentUri, metadata);
}

qint32 HAbstractTransportServicePrivate::getMediaInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    HMediaInfo arg;
    qint32 retVal = q->getMediaInfo(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("NrTracks", arg.numberOfTracks());
        outArgs->setValue("MediaDuration", arg.mediaDuration().toString());
        outArgs->setValue("CurrentURI", arg.currentUri().toString());
        outArgs->setValue("CurrentURIMetaData", arg.currentUriMetadata());
        outArgs->setValue("NextURI", arg.nextUri().toString());
        outArgs->setValue("NextURIMetaData", arg.nextUriMetadata());
        outArgs->setValue("PlayMedium", arg.playMedium().toString());
        outArgs->setValue("RecordMedium", arg.recordMedium().toString());
        outArgs->setValue("WriteStatus", arg.writeStatus().toString());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getMediaInfo_ext(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    HMediaInfo arg;
    qint32 retVal = q->getMediaInfo_ext(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentType", HMediaInfo::toString(arg.mediaCategory()));
        outArgs->setValue("NrTracks", arg.numberOfTracks());
        outArgs->setValue("MediaDuration", arg.mediaDuration().toString());
        outArgs->setValue("CurrentURI", arg.currentUri().toString());
        outArgs->setValue("CurrentURIMetaData", arg.currentUriMetadata());
        outArgs->setValue("NextURI", arg.nextUri().toString());
        outArgs->setValue("NextURIMetaData", arg.nextUriMetadata());
        outArgs->setValue("PlayMedium", arg.playMedium().toString());
        outArgs->setValue("RecordMedium", arg.recordMedium().toString());
        outArgs->setValue("WriteStatus", arg.writeStatus().toString());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getTransportInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    HTransportInfo arg;
    qint32 retVal = q->getTransportInfo(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentTransportState", arg.state().toString());
        outArgs->setValue("CurrentTransportStatus", arg.status().toString());
        outArgs->setValue("CurrentSpeed", arg.speed());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getPositionInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    HPositionInfo arg;
    qint32 retVal = q->getPositionInfo(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("Track", arg.track());
        outArgs->setValue("TrackDuration", arg.trackDuration().toString());
        outArgs->setValue("TrackMetaData", arg.trackMetadata());
        outArgs->setValue("TrackURI", arg.trackUri().toString());
        outArgs->setValue("RelTime", arg.relativeTimePosition().toString());
        outArgs->setValue("AbsTime", arg.absoluteTimePosition().toString());
        outArgs->setValue("RelCount", arg.relativeCounterPosition());
        outArgs->setValue("AbsCount", arg.absoluteCounterPosition());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getDeviceCapabilities(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    HDeviceCapabilities arg;
    qint32 retVal = q->getDeviceCapabilities(instanceId, &arg);
    if (retVal != UpnpSuccess)
    {
        return retVal;
    }

    QStringList tmp;
    foreach(const HStorageMedium& medium, arg.playMedia())
    {
        tmp.append(medium.toString());
    }

    outArgs->setValue("PlayMedia", tmp.join(","));

    tmp.clear();
    foreach(const HStorageMedium& medium, arg.recordMedia())
    {
        tmp.append(medium.toString());
    }

    outArgs->setValue("RecMedia", tmp.join(","));

    tmp.clear();
    foreach(const HRecordQualityMode& mode, arg.recordQualityModes())
    {
        tmp.append(mode.toString());
    }

    outArgs->setValue("RecQualityModes", tmp.join(","));

    return UpnpSuccess;
}

qint32 HAbstractTransportServicePrivate::getTransportSettings(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    HTransportSettings settings;
    qint32 retVal = q->getTransportSettings(instanceId, &settings);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("PlayMode", settings.playMode().toString());
        outArgs->setValue("RecQualityMode", settings.recordQualityMode().toString());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::stop(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    return q->stop(instanceId);
}

qint32 HAbstractTransportServicePrivate::play(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QString playSpeed = inArgs.value("Speed").toString();

    return q->play(instanceId, playSpeed);
}

qint32 HAbstractTransportServicePrivate::pause(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    return q->pause(instanceId);
}

qint32 HAbstractTransportServicePrivate::record(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    return q->record(instanceId);
}

qint32 HAbstractTransportServicePrivate::seek(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QString unitAsStr = inArgs.value("Unit").toString();
    QString target = inArgs.value("Target").toString();

    HSeekInfo arg(unitAsStr, target);
    return q->seek(instanceId, arg);
}

qint32 HAbstractTransportServicePrivate::next(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    return q->next(instanceId);
}

qint32 HAbstractTransportServicePrivate::previous(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    return q->previous(instanceId);
}

qint32 HAbstractTransportServicePrivate::setPlayMode(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HPlayMode playMode = inArgs.value("NewPlayMode").toString();

    return q->setPlayMode(instanceId, playMode);
}

qint32 HAbstractTransportServicePrivate::setRecordQualityMode(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QString recQualityMode =
        inArgs.value("NewRecordQualityMode").toString();

    return q->setRecordQualityMode(instanceId, HRecordQualityMode(recQualityMode));
}

qint32 HAbstractTransportServicePrivate::getCurrentTransportActions(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    QSet<HTransportAction> arg;
    qint32 retVal = q->getCurrentTransportActions(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        QStringList tmp;
        foreach(const HTransportAction& action, arg)
        {
            tmp.append(action.toString());
        }

        outArgs->setValue("Actions", tmp.join(","));
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getDRMState(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    HAvTransportInfo::DrmState arg;
    qint32 retVal = q->getDrmState(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentDRMState", HAvTransportInfo::drmStateToString(arg));
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getStateVariables(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QSet<QString> svNames = inArgs.value("StateVariableList").toString().split(",").toSet();

    QString arg;
    qint32 retVal = q->getStateVariables(instanceId, svNames, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("StateVariableValuePairs", arg);
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::setStateVariables(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HUdn udn = inArgs.value("RenderingControlUDN").toString();
    HResourceType rt = HResourceType(inArgs.value("ServiceType").toString());
    HServiceId sid = inArgs.value("ServiceId").toString();
    QString svValuePairs = inArgs.value("StateVariableValuePairs").toString();

    QStringList arg;
    qint32 retVal = q->setStateVariables(
        instanceId, udn, rt, sid, svValuePairs, &arg);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("StateVariableList", arg.join(","));
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractTransportService
 ******************************************************************************/
HAbstractTransportService::HAbstractTransportService(
    HAbstractTransportServicePrivate& dd) :
        HServerService(dd)
{
}

HAbstractTransportService::HAbstractTransportService() :
    HServerService(*new HAbstractTransportServicePrivate())
{
}

HAbstractTransportService::~HAbstractTransportService()
{
}

HAbstractTransportService::HActionInvokes
    HAbstractTransportService::createActionInvokes()
{
    H_D(HAbstractTransportService);

    HActionInvokes retVal;

    retVal.insert(
        "SetAVTransportURI",
        HActionInvoke(h, &HAbstractTransportServicePrivate::setAVTransportURI));

    retVal.insert(
        "SetNextAVTransportURI",
        HActionInvoke(h, &HAbstractTransportServicePrivate::setNextAVTransportURI));

    retVal.insert(
        "GetMediaInfo",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getMediaInfo));

    retVal.insert(
        "GetMediaInfo_Ext",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getMediaInfo_ext));

    retVal.insert(
        "GetTransportInfo",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getTransportInfo));

    retVal.insert(
        "GetPositionInfo",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getPositionInfo));

    retVal.insert(
        "GetDeviceCapabilities",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getDeviceCapabilities));

    retVal.insert(
        "GetTransportSettings",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getTransportSettings));

    retVal.insert("Stop",
        HActionInvoke(h, &HAbstractTransportServicePrivate::stop));

    retVal.insert("Play",
        HActionInvoke(h, &HAbstractTransportServicePrivate::play));

    retVal.insert(
        "Pause",
        HActionInvoke(h, &HAbstractTransportServicePrivate::pause));

    retVal.insert(
        "Record",
        HActionInvoke(h, &HAbstractTransportServicePrivate::record));

    retVal.insert(
        "Seek",
        HActionInvoke(h, &HAbstractTransportServicePrivate::seek));

    retVal.insert(
        "Next",
        HActionInvoke(h, &HAbstractTransportServicePrivate::next));

    retVal.insert(
        "Previous",
        HActionInvoke(h, &HAbstractTransportServicePrivate::previous));

    retVal.insert(
        "SetPlayMode",
        HActionInvoke(h, &HAbstractTransportServicePrivate::setPlayMode));

    retVal.insert(
        "SetRecordQualityMode",
        HActionInvoke(h, &HAbstractTransportServicePrivate::setRecordQualityMode));

    retVal.insert(
        "GetCurrentTransportActions",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getCurrentTransportActions));

    retVal.insert(
        "GetDRMState",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getDRMState));

    retVal.insert(
        "GetStateVariables",
        HActionInvoke(h, &HAbstractTransportServicePrivate::getStateVariables));

    retVal.insert(
        "SetStateVariables",
        HActionInvoke(h, &HAbstractTransportServicePrivate::setStateVariables));

    return retVal;
}

qint32 HAbstractTransportService::setNextAVTransportURI(
    quint32 /*instanceId*/, const QUrl& /*nextUri*/,
    const QString& /*nextUriMetaData*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::pause(quint32 /*instanceId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::record(quint32 /*instanceId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::getCurrentTransportActions(
    quint32 /*instanceId*/, QSet<HTransportAction>* /*retVal*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::getDrmState(
    quint32 /*instanceId*/, HAvTransportInfo::DrmState* /*retVal*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::getStateVariables(
    quint32 /*instanceId*/,
    const QSet<QString>& /*stateVariableNames*/,
    QString* /*stateVariableValuePairs*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::setStateVariables(
    quint32 /*instanceId*/,
    const HUdn& /*avTransportUdn*/,
    const HResourceType& /*serviceType*/,
    const HServiceId& /*serviceId*/,
    const QString& /*stateVariableValuePairs*/,
    QStringList* /*stateVariableList*/)
{
    return UpnpOptionalActionNotImplemented;
}

}
}
}
