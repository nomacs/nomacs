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

#include "havtransport_adapter.h"
#include "havtransport_adapter_p.h"

#include "hduration.h"
#include "hplaymode.h"
#include "hseekinfo.h"
#include "hmediainfo.h"
#include "hpositioninfo.h"
#include "htransportinfo.h"
#include "htransportinfo.h"
#include "htransportaction.h"
#include "hrecordqualitymode.h"
#include "htransportsettings.h"
#include "hdevicecapabilities.h"
#include "hrecordmediumwritestatus.h"

#include "../common/hstoragemedium.h"
#include "../hav_devicemodel_infoprovider.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HAsyncOp>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HActionInfo>
#include <HUpnpCore/HServiceInfo>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HClientAction>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HClientActionOp>
#include <HUpnpCore/HClientAdapterOp>
#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/HStateVariableEvent>
#include <HUpnpCore/HClientStateVariable>

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAvTransportAdapterPrivate
 ******************************************************************************/
HAvTransportAdapterPrivate::HAvTransportAdapterPrivate() :
    HClientServiceAdapterPrivate(HAvTransportInfo::supportedServiceType()),
        m_instanceId(0)
{
}

HAvTransportAdapterPrivate::~HAvTransportAdapterPrivate()
{
}

bool HAvTransportAdapterPrivate::setAVTransportURI(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->setAVTransportURICompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::setNextAVTransportURI(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->setNextAVTransportURICompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::getMediaInfo(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HMediaInfo mediaInfo;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        quint32 nrTracks = outArgs.value("NrTracks").toUInt();
        HDuration duration = outArgs.value("MediaDuration").toString();
        QUrl curUri = outArgs.value("CurrentURI").toUrl();
        QString curUriMetadata = outArgs.value("CurrentURIMetaData").toString();
        QUrl nextUri = outArgs.value("NextURI").toUrl();
        QString nextUriMetadata = outArgs.value("NextURIMetaData").toString();
        HStorageMedium playMedium = outArgs.value("PlayMedium").toString();
        HStorageMedium recMedium  = outArgs.value("RecordMedium").toString();
        HRecordMediumWriteStatus ws = outArgs.value("WriteStatus").toString();

        mediaInfo = HMediaInfo(
            nrTracks, duration, curUri, curUriMetadata, nextUri, nextUriMetadata,
            playMedium, recMedium, ws, HMediaInfo::Undefined);
    }
    emit q->getMediaInfoCompleted(q, takeOp(op, mediaInfo));

    return false;
}

bool HAvTransportAdapterPrivate::getMediaInfo_ext(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HMediaInfo mediaInfo;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        QString currentType = outArgs.value("CurrentType").toString();
        quint32 nrTracks = outArgs.value("NrTracks").toUInt();
        HDuration duration = outArgs.value("MediaDuration").toString();
        QUrl curUri = outArgs.value("CurrentURI").toUrl();
        QString curUriMetadata = outArgs.value("CurrentURIMetaData").toString();
        QUrl nextUri = outArgs.value("NextURI").toUrl();
        QString nextUriMetadata = outArgs.value("NextURIMetaData").toString();
        HStorageMedium playMedium = outArgs.value("PlayMedium").toString();
        HStorageMedium recMedium  = outArgs.value("RecordMedium").toString();
        HRecordMediumWriteStatus ws = outArgs.value("WriteStatus").toString();

        mediaInfo = HMediaInfo(
            nrTracks, duration, curUri, curUriMetadata, nextUri, nextUriMetadata,
            playMedium, recMedium, ws,
            HMediaInfo::mediaCategoryFromString(currentType));
    }
    emit q->getMediaInfo_extCompleted(q, takeOp(op, mediaInfo));

    return false;
}

bool HAvTransportAdapterPrivate::getTransportInfo(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HTransportInfo info;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        QString state = outArgs.value("CurrentTransportState").toString();
        QString status = outArgs.value("CurrentTransportStatus").toString();
        QString speed = outArgs.value("CurrentSpeed").toString();

       info = HTransportInfo(state, status, speed);
    }
    emit q->getTransportInfoCompleted(q, takeOp(op, info));

    return false;
}

bool HAvTransportAdapterPrivate::getPositionInfo(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HPositionInfo info;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        quint32 track = outArgs.value("Track").toUInt();
        HDuration trackDuration = outArgs.value("TrackDuration").toString();
        QString trackMd = outArgs.value("TrackMetaData").toString();
        QUrl trackUri = outArgs.value("TrackURI").toString();
        HDuration relTime = outArgs.value("RelTime").toString();
        HDuration absTime = outArgs.value("AbsTime").toString();
        qint32 relCountPos = outArgs.value("RelCount").toInt();
        quint32 absCountPos = outArgs.value("AbsCount").toUInt();

       info = HPositionInfo(
           track, trackDuration, trackMd, trackUri, relTime, absTime,
           relCountPos, absCountPos);
    }
    emit q->getPositionInfoCompleted(q, takeOp(op, info));

    return false;
}

bool HAvTransportAdapterPrivate::getDeviceCapabilities(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HDeviceCapabilities capabilities;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        QStringList pmedia = outArgs.value("PlayMedia").toString().split(",");
        QStringList rmedia = outArgs.value("RecMedia").toString().split(",");
        QStringList rqMode = outArgs.value("RecQualityModes").toString().split(",");

        capabilities =
            HDeviceCapabilities(pmedia.toSet(), rmedia.toSet(), rqMode.toSet());
    }
    emit q->getDeviceCapabilitiesCompleted(q, takeOp(op, capabilities));

    return false;
}

bool HAvTransportAdapterPrivate::getTransportSettings(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HTransportSettings settings;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        HPlayMode pm = outArgs.value("PlayMode").toString();
        QString rqMode = outArgs.value("RecQualityMode").toString();

        settings = HTransportSettings(pm, rqMode);
    }
    emit q->getTransportSettingsCompleted(q, takeOp(op, settings));

    return false;
}

bool HAvTransportAdapterPrivate::stop(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->stopCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::play(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->playCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::pause(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->pauseCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::record(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->recordCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::seek(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->seekCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::next(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->nextCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::previous(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->previousCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::setPlayMode(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->setPlayModeCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::setRecordQualityMode(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    emit q->setRecordQualityModeCompleted(
        q, takeOp(op));

    return false;
}

bool HAvTransportAdapterPrivate::getCurrentTransportActions(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    QSet<HTransportAction> actions;
    if (op.returnValue() == UpnpSuccess)
    {
        QStringList slist = op.outputArguments().value("Actions").toString().split(",");

        foreach(const QString& action, slist)
        {
            HTransportAction ta(action);
            if (ta.isValid())
            {
                actions.insert(ta);
            }
        }
    }
    emit q->getCurrentTransportActionsCompleted(q, takeOp(op, actions));

    return false;
}

bool HAvTransportAdapterPrivate::getDRMState(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HAvTransportInfo::DrmState drmState = HAvTransportInfo::DrmState_Unknown;
    if (op.returnValue() == UpnpSuccess)
    {
        drmState = HAvTransportInfo::drmStateFromString(
            op.outputArguments().value("CurrentDRMState").toString());
    }
    emit q->getDrmStateCompleted(q, takeOp(op, drmState));

    return false;
}

bool HAvTransportAdapterPrivate::getStateVariables(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    QString stateVariableValuePairs;
    if (op.returnValue() == UpnpSuccess)
    {
        stateVariableValuePairs =
            op.outputArguments().value("StateVariableValuePairs").toString();
    }
    emit q->getStateVariablesCompleted(q, takeOp(op, stateVariableValuePairs));

    return false;
}

bool HAvTransportAdapterPrivate::setStateVariables(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    QStringList retVal;
    if (op.returnValue() == UpnpSuccess)
    {
        retVal = op.outputArguments().value("StateVariableList").toString().split(",");
    }
    emit q->setStateVariablesCompleted(q, takeOp(op, retVal));

    return false;
}

/*******************************************************************************
 * HAvTransportAdapter
 ******************************************************************************/
HAvTransportAdapter::HAvTransportAdapter(
    quint32 instanceId, QObject* parent) :
        HClientServiceAdapter(*new HAvTransportAdapterPrivate(), parent)
{
    H_D(HAvTransportAdapter);
    h->m_instanceId = instanceId;
}

HAvTransportAdapter::~HAvTransportAdapter()
{
}

void HAvTransportAdapter::lastChange(
    const HClientStateVariable*, const HStateVariableEvent& event)
{
    emit lastChangeReceived(this, event.newValue().toString());
}

bool HAvTransportAdapter::prepareService(HClientService* service)
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

HClientAdapterOpNull HAvTransportAdapter::setAVTransportURI(
    const QUrl& currentUri, const QString& currentUriMetaData)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("SetAVTransportURI", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue("CurrentURI", currentUri))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("CurrentURIMetaData", currentUriMetaData))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setAVTransportURI));
}

HClientAdapterOpNull HAvTransportAdapter::setNextAVTransportURI(
    const QUrl& currentUri, const QString& currentUriMetaData)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("SetNextAVTransportURI", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue("NextURI", currentUri))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("NextURIMetaData", currentUriMetaData))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setNextAVTransportURI));
}

HClientAdapterOp<HMediaInfo> HAvTransportAdapter::getMediaInfo()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetMediaInfo", &rc);
    if (!action)
    {
        return HClientAdapterOp<HMediaInfo>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<HMediaInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getMediaInfo));
}

HClientAdapterOp<HMediaInfo> HAvTransportAdapter::getMediaInfo_ext()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetMediaInfo_Ext", &rc);
    if (!action)
    {
        return HClientAdapterOp<HMediaInfo>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<HMediaInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getMediaInfo_ext));
}

HClientAdapterOp<HTransportInfo> HAvTransportAdapter::getTransportInfo()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetTransportInfo", &rc);
    if (!action)
    {
        return HClientAdapterOp<HTransportInfo>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<HTransportInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getTransportInfo));
}

HClientAdapterOp<HPositionInfo> HAvTransportAdapter::getPositionInfo()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetPositionInfo", &rc);
    if (!action)
    {
        return HClientAdapterOp<HPositionInfo>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<HPositionInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getPositionInfo));
}

HClientAdapterOp<HDeviceCapabilities> HAvTransportAdapter::getDeviceCapabilities()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetDeviceCapabilities", &rc);
    if (!action)
    {
        return HClientAdapterOp<HDeviceCapabilities>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<HDeviceCapabilities>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getDeviceCapabilities));
}

HClientAdapterOp<HTransportSettings> HAvTransportAdapter::getTransportSettings()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetTransportSettings", &rc);
    if (!action)
    {
        return HClientAdapterOp<HTransportSettings>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<HTransportSettings>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getTransportSettings));
}

HClientAdapterOpNull HAvTransportAdapter::stop()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Stop", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::stop));
}

HClientAdapterOpNull HAvTransportAdapter::play(const QString& speed)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Play", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue("Speed", speed))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::play));
}

HClientAdapterOpNull HAvTransportAdapter::pause()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Pause", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::pause));
}

HClientAdapterOpNull HAvTransportAdapter::record()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Record", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::record));
}

HClientAdapterOpNull HAvTransportAdapter::seek(const HSeekInfo& info)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Seek", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue("Unit", info.unit().toString()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("Target", info.target()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::seek));
}

HClientAdapterOpNull HAvTransportAdapter::next()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Next", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::next));
}

HClientAdapterOpNull HAvTransportAdapter::previous()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("Previous", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::previous));
}

HClientAdapterOpNull HAvTransportAdapter::setPlayMode(const HPlayMode& mode)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("SetPlayMode", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue("NewPlayMode", mode.toString()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setPlayMode));
}

HClientAdapterOpNull HAvTransportAdapter::setRecordQualityMode(const HRecordQualityMode& mode)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("SetRecordQualityMode", &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue("NewRecordQualityMode", mode.toString()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setRecordQualityMode));
}

HClientAdapterOp<QSet<HTransportAction> > HAvTransportAdapter::getCurrentTransportActions()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetCurrentTransportActions", &rc);
    if (!action)
    {
        return HClientAdapterOp<QSet<HTransportAction> >::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<QSet<HTransportAction> >(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getCurrentTransportActions));
}

HClientAdapterOp<HAvTransportInfo::DrmState> HAvTransportAdapter::getDrmState()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetDRMState", &rc);
    if (!action)
    {
        return HClientAdapterOp<HAvTransportInfo::DrmState>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);

    return h_ptr->beginInvoke<HAvTransportInfo::DrmState>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getDRMState));
}

HClientAdapterOp<QString> HAvTransportAdapter::getStateVariables(
    const QSet<QString>& stateVariableNames)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("GetStateVariables", &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue(
        "StateVariableList", QStringList(stateVariableNames.toList()).join(",")))
    {
        return HClientAdapterOp<QString>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<QString>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getStateVariables));
}

HClientAdapterOp<QStringList> HAvTransportAdapter::setStateVariables(
    const HUdn& avTransportUdn, const HResourceType& serviceType,
    const HServiceId& serviceId, const QString& stateVariableValuePairs)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction("SetStateVariables", &rc);
    if (!action)
    {
        return HClientAdapterOp<QStringList>::createInvalid(rc, "");
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue("InstanceID", h->m_instanceId);
    if (!inArgs.setValue("AVTransportUDN", avTransportUdn.toSimpleUuid()))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("ServiceType", serviceType.toString()))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("ServiceId", serviceId.toString()))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, "");
    }
    if (!inArgs.setValue("StateVariableValuePairs", stateVariableValuePairs))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, "");
    }

    return h_ptr->beginInvoke<QStringList>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setStateVariables));
}

}
}
}
