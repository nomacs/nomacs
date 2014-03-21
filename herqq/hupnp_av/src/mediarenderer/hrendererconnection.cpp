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

#include "hrendererconnection.h"
#include "hrendererconnection_p.h"

#include "../hav_global_p.h"

#include "../transport/hduration.h"
#include "../transport/hplaymode.h"
#include "../transport/hmediainfo.h"
#include "../transport/hpositioninfo.h"
#include "../transport/htransportinfo.h"
#include "../transport/htransportstate.h"
#include "../transport/htransportaction.h"
#include "../transport/hrecordqualitymode.h"
#include "../transport/htransportsettings.h"
#include "../transport/hdevicecapabilities.h"
#include "../transport/hrecordmediumwritestatus.h"

#include "../renderingcontrol/hchannel.h"
#include "../renderingcontrol/hrenderingcontrol_info.h"

#include "../common/hprotocolinfo.h"
#include "../connectionmanager/hconnectioninfo.h"
#include "../connectionmanager/habstractconnectionmanager_service.h"

#include "../common/hstoragemedium.h"
#include "../cds_model/cds_objects/hobject.h"
#include "../cds_model/model_mgmt/hcds_dlite_serializer.h"

#include <HUpnpCore/private/hmisc_utils_p.h>
#include <HUpnpCore/private/hlogger_p.h>

#include <QtCore/QSet>
#include <QtCore/QUrl>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRendererConnectionPrivate
 ******************************************************************************/
HRendererConnectionPrivate::HRendererConnectionPrivate() :
    m_info(0), m_connectionInfo(0), m_service(0), q_ptr(0), m_valueSetters()
{
    m_valueSetters.insert("Brightness", ValueSetter(this, &HRendererConnectionPrivate::setBrightness));
    m_valueSetters.insert("Contrast", ValueSetter(this, &HRendererConnectionPrivate::setContrast));
    m_valueSetters.insert("Sharpness", ValueSetter(this, &HRendererConnectionPrivate::setSharpness));
    m_valueSetters.insert("RedVideoGain", ValueSetter(this, &HRendererConnectionPrivate::setRedVideoGain));
    m_valueSetters.insert("GreenVideoGain", ValueSetter(this, &HRendererConnectionPrivate::setGreenVideoGain));
    m_valueSetters.insert("BlueVideoGain", ValueSetter(this, &HRendererConnectionPrivate::setBlueVideoGain));
    m_valueSetters.insert("RedVideoBlackLevel", ValueSetter(this, &HRendererConnectionPrivate::setRedVideoBlackLevel));
    m_valueSetters.insert("GreenVideoBlackLevel", ValueSetter(this, &HRendererConnectionPrivate::setGreenVideoBlackLevel));
    m_valueSetters.insert("BlueVideoBlackLevel", ValueSetter(this, &HRendererConnectionPrivate::setBlueVideoBlackLevel));
    m_valueSetters.insert("ColorTemperature", ValueSetter(this, &HRendererConnectionPrivate::setColorTemperature));
    m_valueSetters.insert("HorizontalKeystone", ValueSetter(this, &HRendererConnectionPrivate::setHorizontalKeystone));
    m_valueSetters.insert("VerticalKeystone", ValueSetter(this, &HRendererConnectionPrivate::setVerticalKeystone));
    m_valueSetters.insert("Mute", ValueSetter(this, &HRendererConnectionPrivate::setMute));
    m_valueSetters.insert("Volume", ValueSetter(this, &HRendererConnectionPrivate::setVolume));
    m_valueSetters.insert("VolumeDB", ValueSetter(this, &HRendererConnectionPrivate::setVolumeDB));
    m_valueSetters.insert("Loudness", ValueSetter(this, &HRendererConnectionPrivate::setLoudness));
}

HRendererConnectionPrivate::~HRendererConnectionPrivate()
{
    delete m_info;
}

bool HRendererConnectionPrivate::setBrightness(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::Brightness, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setContrast(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::Contrast, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setSharpness(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::Sharpness, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setRedVideoGain(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::RedVideoGain, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setGreenVideoGain(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::GreenVideoGain, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setBlueVideoGain(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::BlueVideoGain, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setRedVideoBlackLevel(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::RedVideoBlackLevel, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setGreenVideoBlackLevel(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::GreenVideoBlackLevel, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setBlueVideoBlackLevel(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::BlueVideoBlackLevel, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setColorTemperature(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setRcsValue(HRendererConnectionInfo::ColorTemperature, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setHorizontalKeystone(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setHorizontalKeystone(value.toShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setVerticalKeystone(const QString& value, const HChannel&)
{
    qint32 rc = q_ptr->setVerticalKeystone(value.toShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setMute(const QString& value, const HChannel& ch)
{
    bool ok = false;
    qint32 rc = q_ptr->setMute(ch, toBool(value, &ok));
    return ok && rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setVolume(const QString& value, const HChannel& ch)
{
    qint32 rc = q_ptr->setVolume(ch, value.toUShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setVolumeDB(const QString& value, const HChannel& ch)
{
    qint32 rc = q_ptr->setVolumeDb(ch, value.toShort());
    return rc == UpnpSuccess;
}

bool HRendererConnectionPrivate::setLoudness(const QString& value, const HChannel& ch)
{
    bool ok = false;
    qint32 rc = q_ptr->setLoudness(ch, toBool(value, &ok));
    return ok && rc == UpnpSuccess;
}

/*******************************************************************************
 * HRendererConnection
 ******************************************************************************/
HRendererConnection::HRendererConnection(QObject* parent) :
    QObject(parent),
        h_ptr(new HRendererConnectionPrivate())
{
    h_ptr->q_ptr = this;
    h_ptr->m_info = new HRendererConnectionInfo(this);

    h_ptr->m_info->setTransportStatus(HTransportStatus::OK);
    h_ptr->m_info->setTransportState(HTransportState::NoMediaPresent);

    HTransportSettings transPortSettings(h_ptr->m_info->transportSettings());
    transPortSettings.setPlayMode(HPlayMode::Normal);
    transPortSettings.setRecordQualityMode(HRecordQualityMode::NotImplemented);
    h_ptr->m_info->setTransportSettings(transPortSettings);

    h_ptr->m_info->setCurrentMediaCategory(HMediaInfo::NoMedia);

    QSet<HTransportAction> actions = HTransportAction::allActions();
    h_ptr->m_info->setCurrentTransportActions(actions);
}

HRendererConnection::~HRendererConnection()
{
    delete h_ptr;
}

void HRendererConnection::init(
    HAbstractConnectionManagerService* service,
    HConnectionInfo* connectionInfo)
{
    Q_ASSERT(!h_ptr->m_service);
    Q_ASSERT(!h_ptr->m_connectionInfo);

    Q_ASSERT(service);
    Q_ASSERT(connectionInfo);

    h_ptr->m_service = service;
    h_ptr->m_connectionInfo = connectionInfo;
}

void HRendererConnection::dispose()
{
    emit disposed(this);
}

HAbstractConnectionManagerService* HRendererConnection::service() const
{
    return h_ptr->m_service;
}

qint32 HRendererConnection::doPause()
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doRecord()
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doSetPlayMode(const HPlayMode& newMode)
{
    return newMode.type() == HPlayMode::Normal ?
        static_cast<qint32>(UpnpSuccess) :
        static_cast<qint32>(HAvTransportInfo::PlayModeNotSupported);
}

qint32 HRendererConnection::doSetRecordQualityMode(const HRecordQualityMode& newMode)
{
    Q_UNUSED(newMode);
    return HAvTransportInfo::RecordQualityNotSupported;
}

qint32 HRendererConnection::doSetNextResource(
    const QUrl& resourceUri, HObject* cdsMetadata)
{
    Q_UNUSED(resourceUri)
    Q_UNUSED(cdsMetadata)
    return UpnpSuccess;
}

qint32 HRendererConnection::doSetRcsValue(
    HRendererConnectionInfo::RcsAttribute rcsAttrib, quint16 newValue)
{
    Q_UNUSED(rcsAttrib)
    Q_UNUSED(newValue)
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doSetHorizontalKeystone(qint16 desiredHorizontalKeyStone)
{
    Q_UNUSED(desiredHorizontalKeyStone)
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doSetVerticalKeystone(qint16 desiredHorizontalKeyStone)
{
    Q_UNUSED(desiredHorizontalKeyStone)
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doSetMute(const HChannel& channel, bool muted)
{
    Q_UNUSED(channel)
    Q_UNUSED(muted)
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doSetVolume(const HChannel& channel, quint16 volume)
{
    Q_UNUSED(channel)
    Q_UNUSED(volume)
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doSetVolumeDb(const HChannel& channel, qint16 volume)
{
    Q_UNUSED(channel)
    Q_UNUSED(volume)
    return UpnpOptionalActionNotImplemented;
}

qint32 HRendererConnection::doSetLoudness(const HChannel& channel, bool enabled)
{
    Q_UNUSED(channel)
    Q_UNUSED(enabled)
    return UpnpOptionalActionNotImplemented;
}

HRendererConnectionInfo* HRendererConnection::writableRendererConnectionInfo()
{
    return h_ptr->m_info;
}

void HRendererConnection::setContentFormat(const QString& contentFormats)
{
    HProtocolInfo pi = h_ptr->m_connectionInfo->protocolInfo();
    pi.setContentFormat(contentFormats);
    h_ptr->m_connectionInfo->setProtocolInfo(pi);
}

void HRendererConnection::setAdditionalInfo(const QString& additionalInfo)
{
    HProtocolInfo pi = h_ptr->m_connectionInfo->protocolInfo();
    pi.setAdditionalInfo(additionalInfo);
    h_ptr->m_connectionInfo->setProtocolInfo(pi);
}

void HRendererConnection::setConnectionStatus(
    HConnectionManagerInfo::ConnectionStatus connectionStatus)
{
    h_ptr->m_connectionInfo->setStatus(connectionStatus);
}

void HRendererConnection::finalizeInit()
{
}

const HRendererConnectionInfo* HRendererConnection::rendererConnectionInfo() const
{
    return h_ptr->m_info;
}

const HConnectionInfo* HRendererConnection::connectionInfo() const
{
    return h_ptr->m_connectionInfo;
}

qint32 HRendererConnection::play(const QString& speed)
{
    HLOG(H_AT, H_FUN);

    switch(h_ptr->m_info->transportState().type())
    {
    case HTransportState::Stopped:
    case HTransportState::Playing:
    case HTransportState::PausedPlayback:
    case HTransportState::Transitioning:
    case HTransportState::VendorDefined:
        break;
    default:
        return HAvTransportInfo::TransitionNotAvailable;
    }

    qint32 rc = doPlay(speed);
    if (rc == UpnpSuccess)
    {
        HTransportInfo transportInfo = h_ptr->m_info->transportInfo();
        transportInfo.setSpeed(speed);
        transportInfo.setState(HTransportState::Playing);
        h_ptr->m_info->setTransportInfo(transportInfo);
    }
    return rc;
}

qint32 HRendererConnection::stop()
{
    HLOG(H_AT, H_FUN);

    if (h_ptr->m_info->transportState().type() == HTransportState::NoMediaPresent)
    {
        return HAvTransportInfo::TransitionNotAvailable;
    }

    qint32 rc = doStop();
    h_ptr->m_info->setTransportState(HTransportState::Stopped);
    return rc;
}

qint32 HRendererConnection::pause()
{
    HLOG(H_AT, H_FUN);

    bool pausedPlaying = true;
    switch(h_ptr->m_info->transportState().type())
    {
    case HTransportState::Playing:
    case HTransportState::PausedPlayback:
    case HTransportState::VendorDefined:
        break;
    case HTransportState::PausedRecording:
    case HTransportState::Recording:
        pausedPlaying = false;
        break;
    default:
        return HAvTransportInfo::TransitionNotAvailable;
    }

    qint32 rc = doPause();
    if (rc == UpnpSuccess)
    {
        h_ptr->m_info->setTransportState(pausedPlaying ?
            HTransportState::PausedPlayback : HTransportState::PausedRecording);
    }
    return rc;
}

qint32 HRendererConnection::record()
{
    HLOG(H_AT, H_FUN);

    switch(h_ptr->m_info->transportState().type())
    {
    case HTransportState::Stopped:
    case HTransportState::PausedRecording:
    case HTransportState::VendorDefined:
        break;
    default:
        return HAvTransportInfo::TransitionNotAvailable;
    }

    qint32 rc = doRecord();
    if (rc == UpnpSuccess)
    {
        h_ptr->m_info->setTransportState(HTransportState::Recording);
    }
    return rc;
}

qint32 HRendererConnection::seek(const HSeekInfo& seekInfo)
{
    HLOG(H_AT, H_FUN);

    switch(h_ptr->m_info->transportState().type())
    {
    case HTransportState::Stopped:
    case HTransportState::Playing:
    case HTransportState::VendorDefined:
        break;
    default:
        return HAvTransportInfo::TransitionNotAvailable;
    }

    qint32 rc = doSeek(seekInfo);
    if (rc == UpnpSuccess)
    {
        //info.setTransportState(HTransportState::Transitioning);
    }
    return rc;
}

qint32 HRendererConnection::next()
{
    HLOG(H_AT, H_FUN);

    switch(h_ptr->m_info->transportState().type())
    {
    case HTransportState::Stopped:
    case HTransportState::Playing:
    case HTransportState::VendorDefined:
        break;
    default:
        return HAvTransportInfo::TransitionNotAvailable;
    }

    qint32 rc = doNext();
    if (rc == UpnpSuccess)
    {
        //info.setTransportState(HTransportState::Transitioning);
    }
    return rc;
}

qint32 HRendererConnection::previous()
{
    HLOG(H_AT, H_FUN);

    switch(h_ptr->m_info->transportState().type())
    {
    case HTransportState::Stopped:
    case HTransportState::Playing:
    case HTransportState::VendorDefined:
        break;
    default:
        return HAvTransportInfo::TransitionNotAvailable;
    }

    qint32 rc = doPrevious();
    if (rc == UpnpSuccess)
    {
        //info.setTransportState(HTransportState::Transitioning);
    }
    return rc;
}

qint32 HRendererConnection::setPlaymode(const HPlayMode& newMode)
{
    HLOG(H_AT, H_FUN);

    if (!newMode.isValid())
    {
        return UpnpInvalidArgs;
    }

    qint32 rc = doSetPlayMode(newMode);
    if (rc == UpnpSuccess)
    {
        HTransportSettings settings = h_ptr->m_info->transportSettings();
        settings.setPlayMode(newMode);
        h_ptr->m_info->setTransportSettings(settings);
    }
    return rc;
}

qint32 HRendererConnection::setRecordQualityMode(const HRecordQualityMode& newMode)
{
    HLOG(H_AT, H_FUN);

    if (!newMode.isValid())
    {
        return UpnpInvalidArgs;
    }

    qint32 rc = doSetRecordQualityMode(newMode);
    if (rc == UpnpSuccess)
    {
        HTransportSettings settings = h_ptr->m_info->transportSettings();
        settings.setRecordQualityMode(newMode);
        h_ptr->m_info->setTransportSettings(settings);
    }
    return rc;
}

qint32 HRendererConnection::setResource(
    const QUrl& resourceUri, const QString& resourceMetadata)
{
    HLOG(H_AT, H_FUN);

    if (h_ptr->m_info->mediaInfo().currentUri() == resourceUri)
    {
        return HAvTransportInfo::ContentBusy;
    }

    QScopedPointer<HObject> metadata;
    if (!resourceMetadata.isEmpty())
    {
        HObjects objects;
        HCdsDidlLiteSerializer serializer;
        if (serializer.serializeFromXml(resourceMetadata, &objects))
        {
            if (objects.size())
            {
                metadata.reset(objects.at(0));
                objects.removeFirst();
                qDeleteAll(objects);
            }
        }
    }

    qint32 rc = doSetResource(resourceUri, metadata.data());
    if (rc == UpnpSuccess)
    {
        HMediaInfo mediaInfo = h_ptr->m_info->mediaInfo();
        mediaInfo.setCurrentUri(resourceUri);
        mediaInfo.setCurrentUriMetadata(resourceMetadata);
        h_ptr->m_info->setMediaInfo(mediaInfo);

        if (h_ptr->m_info->transportState().type() == HTransportState::NoMediaPresent)
        {
            h_ptr->m_info->setTransportState(HTransportState::Stopped);
        }
    }

    return rc;
}

qint32 HRendererConnection::setNextResource(
    const QUrl& resourceUri, const QString& resourceMetadata)
{
    HLOG(H_AT, H_FUN);

    if (h_ptr->m_info->mediaInfo().currentUri() == resourceUri)
    {
        return HAvTransportInfo::ContentBusy;
    }
    else if (h_ptr->m_info->mediaInfo().nextUri() == resourceUri)
    {
        return UpnpSuccess;
    }

    QScopedPointer<HObject> metadata;
    if (!resourceMetadata.isEmpty())
    {
        HObjects objects;
        HCdsDidlLiteSerializer serializer;
        if (serializer.serializeFromXml(resourceMetadata, &objects))
        {
            if (objects.size())
            {
                metadata.reset(objects.at(0));
                objects.removeFirst();
                qDeleteAll(objects);
            }
        }
    }

    qint32 rc = doSetNextResource(resourceUri, metadata.data());
    if (rc == UpnpSuccess)
    {
        HMediaInfo mediaInfo = h_ptr->m_info->mediaInfo();
        mediaInfo.setNextUri(resourceUri);
        mediaInfo.setNextUriMetadata(resourceMetadata);
        h_ptr->m_info->setMediaInfo(mediaInfo);
    }

    return rc;
}

qint32 HRendererConnection::selectPreset(const QString& presetName)
{
    HLOG(H_AT, H_FUN);

    if (!h_ptr->m_info->presets().contains(presetName))
    {
        return HRenderingControlInfo::InvalidName;
    }
    return doSelectPreset(presetName);
}

qint32 HRendererConnection::setRcsValue(
    HRendererConnectionInfo::RcsAttribute rcsAttrib, quint16 newValue)
{
    HLOG(H_AT, H_FUN);

    qint32 rc = doSetRcsValue(rcsAttrib, newValue);
    if (rc == UpnpSuccess)
    {
        h_ptr->m_info->setRcsValue(rcsAttrib, newValue);
    }
    return rc;
}

qint32 HRendererConnection::setHorizontalKeystone(qint16 desiredHorizontalKeyStone)
{
    HLOG(H_AT, H_FUN);

    qint32 rc = doSetHorizontalKeystone(desiredHorizontalKeyStone);
    if (rc == UpnpSuccess)
    {
        h_ptr->m_info->setHorizontalKeystone(desiredHorizontalKeyStone);
    }
    return rc;
}

qint32 HRendererConnection::setVerticalKeystone(qint16 desiredHorizontalKeyStone)
{
    HLOG(H_AT, H_FUN);

    qint32 rc = doSetVerticalKeystone(desiredHorizontalKeyStone);
    if (rc == UpnpSuccess)
    {
        h_ptr->m_info->setVerticalKeystone(desiredHorizontalKeyStone);
    }
    return rc;
}

qint32 HRendererConnection::setMute(const HChannel& channel, bool muted)
{
    HLOG(H_AT, H_FUN);

    if (!channel.isValid())
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    qint32 rc = doSetMute(channel, muted);
    if (rc == UpnpSuccess)
    {
        if (!h_ptr->m_info->setMute(channel, muted))
        {
            HLOG_WARN(QString(
                "Failed to update the value for [Mute] using channel: [%1]").arg(
                channel.toString()));

            return HRenderingControlInfo::InvalidChannel;
        }
    }
    return rc;
}

qint32 HRendererConnection::setVolume(const HChannel& channel, quint16 volume)
{
    HLOG(H_AT, H_FUN);

    if (!channel.isValid())
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    qint32 rc = doSetVolume(channel, volume);
    if (rc == UpnpSuccess)
    {
        if (!h_ptr->m_info->setVolume(channel, volume))
        {
            HLOG_WARN(QString(
                "Failed to update the value for [Volume] using channel: [%1]").arg(
                channel.toString()));

            return HRenderingControlInfo::InvalidChannel;
        }
    }
    return rc;
}

qint32 HRendererConnection::setVolumeDb(const HChannel& channel, qint16 volume)
{
    HLOG(H_AT, H_FUN);

    if (!channel.isValid())
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    qint32 rc = doSetVolumeDb(channel, volume);
    if (rc == UpnpSuccess)
    {
        if (!h_ptr->m_info->setVolumeDb(channel, volume))
        {
            HLOG_WARN(QString(
                "Failed to update the value for [VolumeDB] using channel: [%1]").arg(
                channel.toString()));

            return HRenderingControlInfo::InvalidChannel;
        }
    }
    return rc;
}

qint32 HRendererConnection::setLoudness(const HChannel& channel, bool enabled)
{
    HLOG(H_AT, H_FUN);

    if (!channel.isValid())
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    qint32 rc = doSetLoudness(channel, enabled);
    if (rc == UpnpSuccess)
    {
        if (!h_ptr->m_info->setLoudness(channel, enabled))
        {
            HLOG_WARN(QString(
                "Failed to update the value for [Loudness] using channel: [%1]").arg(
                channel.toString()));

            return HRenderingControlInfo::InvalidChannel;
        }
    }
    return rc;
}

QString HRendererConnection::value(const QString& svName, bool* ok) const
{
    return value(svName, HChannel(), ok);
}

QString HRendererConnection::value(
    const QString& svName, const HChannel& channel, bool* ok) const
{
    return h_ptr->m_info->value(svName, channel, ok);
}

bool HRendererConnection::setValue(const QString& svName, const QString& value)
{
    return setValue(svName, HChannel(), value);
}

bool HRendererConnection::setValue(const QString& svName, const HChannel& ch, const QString& value)
{
    if (h_ptr->m_valueSetters.contains(svName))
    {
        ValueSetter setter = h_ptr->m_valueSetters.value(svName);
        setter(value, ch);
        return true;
    }
    return h_ptr->m_info->setValue(svName, ch, value);
}

}
}
}
