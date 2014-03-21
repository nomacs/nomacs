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

#include "hrendererconnection_info.h"
#include "hrendererconnection_info_p.h"

#include "../hav_global_p.h"

#include "../transport/hrecordqualitymode.h"
#include "../renderingcontrol/hvolumedbrange_result.h"

#include <HUpnpCore/private/hmisc_utils_p.h>

#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HRendererConnectionEventInfo>("Herqq::Upnp::Av::HRendererConnectionEventInfo");
    return true;
}

static bool regMetaT = registerMetaTypes();

/*******************************************************************************
 * HChannelInformation
 ******************************************************************************/
HChannelInformation::HChannelInformation(const HChannel& ch) :
    m_channel(ch), volume(0), volumeDb(0), minVolumeDb(0), maxVolumeDb(0),
    muted(false), loudness(false)
{
}

/*******************************************************************************
 * HRendererConnectionEventInfoPrivate
 ******************************************************************************/
class HRendererConnectionEventInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HRendererConnectionEventInfoPrivate)

public:

    QString m_variableName;
    QString m_oldValue;
    QString m_newValue;
    HChannel m_channel;
};

/*******************************************************************************
 * HRendererConnectionEventInfo
 ******************************************************************************/
HRendererConnectionEventInfo::HRendererConnectionEventInfo() :
    h_ptr(new HRendererConnectionEventInfoPrivate())
{
}

HRendererConnectionEventInfo::HRendererConnectionEventInfo(
    const QString& propName,
    const QString& newValue) :
        h_ptr(new HRendererConnectionEventInfoPrivate())
{
    h_ptr->m_variableName = propName.trimmed();
    h_ptr->m_newValue = newValue;
}

HRendererConnectionEventInfo::HRendererConnectionEventInfo(
    const HRendererConnectionEventInfo& other) :
        h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HRendererConnectionEventInfo& HRendererConnectionEventInfo::operator=(
    const HRendererConnectionEventInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HRendererConnectionEventInfo::~HRendererConnectionEventInfo()
{
}

bool HRendererConnectionEventInfo::isValid() const
{
    return !h_ptr->m_variableName.isEmpty() &&
            h_ptr->m_oldValue != h_ptr->m_newValue;
}

QString HRendererConnectionEventInfo::propertyName() const
{
    return h_ptr->m_variableName;
}

QString HRendererConnectionEventInfo::oldValue() const
{
    return h_ptr->m_oldValue;
}

QString HRendererConnectionEventInfo::newValue() const
{
    return h_ptr->m_newValue;
}

HChannel HRendererConnectionEventInfo::channel() const
{
    return h_ptr->m_channel;
}

void HRendererConnectionEventInfo::setPropertyName(const QString& arg)
{
    h_ptr->m_variableName = arg.trimmed();
}

void HRendererConnectionEventInfo::setOldValue(const QString& arg)
{
    h_ptr->m_oldValue = arg;
}

void HRendererConnectionEventInfo::setNewValue(const QString& arg)
{
    h_ptr->m_newValue = arg;
}

void HRendererConnectionEventInfo::setChannel(const HChannel& arg)
{
    h_ptr->m_channel = arg;
}

bool operator==(const HRendererConnectionEventInfo& obj1, const HRendererConnectionEventInfo& obj2)
{
    return obj1.propertyName() == obj2.propertyName() &&
           obj1.newValue() == obj2.newValue() &&
           obj1.oldValue() == obj2.oldValue() &&
           obj1.channel() == obj2.channel();
}

/*******************************************************************************
 * HRendererConnectionInfoPrivate
 ******************************************************************************/
HRendererConnectionInfoPrivate::HRendererConnectionInfoPrivate() :
    q_ptr(0),
    m_parent(0),
    m_valueSetters(),
    m_valueGetters(),
    m_transportActions(),
    m_drmState(HAvTransportInfo::DrmState_Unknown),
    m_deviceCapabilities(),
    m_mediaInfo(),
    m_positionInfo(),
    m_transportInfo(),
    m_transportSettings(),
    m_presets(),
    m_rcsAttributes(),
    m_horizontalKeystone(0),
    m_verticalKeystone(0),
    m_channelInfo()
{
    m_valueSetters.insert("TransportState", ValueSetter(this, &HRendererConnectionInfoPrivate::setTransportState));
    m_valueSetters.insert("TransportStatus", ValueSetter(this, &HRendererConnectionInfoPrivate::setTransportStatus));
    m_valueSetters.insert("CurrentMediaCategory", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentMediaCategory));
    m_valueSetters.insert("PlaybackStorageMedium", ValueSetter(this, &HRendererConnectionInfoPrivate::setPlaybackStorageMedium));
    m_valueSetters.insert("RecordStorageMedium", ValueSetter(this, &HRendererConnectionInfoPrivate::setRecordStorageMedium));
    m_valueSetters.insert("PossiblePlaybackStorageMedia", ValueSetter(this, &HRendererConnectionInfoPrivate::setPossiblePlaybackStorageMedia));
    m_valueSetters.insert("PossibleRecordStorageMedia", ValueSetter(this, &HRendererConnectionInfoPrivate::setPossibleRecordStorageMedia));
    m_valueSetters.insert("CurrentPlayMode", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentPlayMode));
    m_valueSetters.insert("TransportPlaySpeed", ValueSetter(this, &HRendererConnectionInfoPrivate::setTransportPlaySpeed));
    m_valueSetters.insert("RecordMediumWriteStatus", ValueSetter(this, &HRendererConnectionInfoPrivate::setRecordMediumWriteStatus));
    m_valueSetters.insert("CurrentRecordQualityMode", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentRecordQualityMode));
    m_valueSetters.insert("PossibleRecordQualityModes", ValueSetter(this, &HRendererConnectionInfoPrivate::setPossibleRecordQualityModes));
    m_valueSetters.insert("NumberOfTracks", ValueSetter(this, &HRendererConnectionInfoPrivate::setNumberOfTracks));
    m_valueSetters.insert("CurrentTrack", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentTrack));
    m_valueSetters.insert("CurrentTrackDuration", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentTrackDuration));
    m_valueSetters.insert("CurrentMediaDuration", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentMediaDuration));
    m_valueSetters.insert("CurrentTrackMetaData", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentTrackMetaData));
    m_valueSetters.insert("CurrentTrackURI", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentTrackURI));
    m_valueSetters.insert("AVTransportURI", ValueSetter(this, &HRendererConnectionInfoPrivate::setAVTransportURI));
    m_valueSetters.insert("AVTransportURIMetaData", ValueSetter(this, &HRendererConnectionInfoPrivate::setAVTransportURIMetaData));
    m_valueSetters.insert("NextAVTransportURI", ValueSetter(this, &HRendererConnectionInfoPrivate::setNextAVTransportURI));
    m_valueSetters.insert("NextAVTransportURIMetaData", ValueSetter(this, &HRendererConnectionInfoPrivate::setNextAVTransportURIMetaData));
    m_valueSetters.insert("RelativeTimePosition", ValueSetter(this, &HRendererConnectionInfoPrivate::setRelativeTimePosition));
    m_valueSetters.insert("AbsoluteTimePosition", ValueSetter(this, &HRendererConnectionInfoPrivate::setAbsoluteTimePosition));
    m_valueSetters.insert("RelativeCounterPosition", ValueSetter(this, &HRendererConnectionInfoPrivate::setRelativeCounterPosition));
    m_valueSetters.insert("AbsoluteCounterPosition", ValueSetter(this, &HRendererConnectionInfoPrivate::setAbsoluteCounterPosition));
    m_valueSetters.insert("CurrentTransportActions", ValueSetter(this, &HRendererConnectionInfoPrivate::setCurrentTransportActions));
    m_valueSetters.insert("DRMState", ValueSetter(this, &HRendererConnectionInfoPrivate::setDrmState));

    m_valueSetters.insert("Brightness", ValueSetter(this, &HRendererConnectionInfoPrivate::setBrightness));
    m_valueSetters.insert("Contrast", ValueSetter(this, &HRendererConnectionInfoPrivate::setContrast));
    m_valueSetters.insert("Sharpness", ValueSetter(this, &HRendererConnectionInfoPrivate::setSharpness));
    m_valueSetters.insert("RedVideoGain", ValueSetter(this, &HRendererConnectionInfoPrivate::setRedVideoGain));
    m_valueSetters.insert("GreenVideoGain", ValueSetter(this, &HRendererConnectionInfoPrivate::setGreenVideoGain));
    m_valueSetters.insert("BlueVideoGain", ValueSetter(this, &HRendererConnectionInfoPrivate::setBlueVideoGain));
    m_valueSetters.insert("RedVideoBlackLevel", ValueSetter(this, &HRendererConnectionInfoPrivate::setRedVideoBlackLevel));
    m_valueSetters.insert("GreenVideoBlackLevel", ValueSetter(this, &HRendererConnectionInfoPrivate::setGreenVideoBlackLevel));
    m_valueSetters.insert("BlueVideoBlackLevel", ValueSetter(this, &HRendererConnectionInfoPrivate::setBlueVideoBlackLevel));
    m_valueSetters.insert("ColorTemperature", ValueSetter(this, &HRendererConnectionInfoPrivate::setColorTemperature));
    m_valueSetters.insert("HorizontalKeystone", ValueSetter(this, &HRendererConnectionInfoPrivate::setHorizontalKeystone));
    m_valueSetters.insert("VerticalKeystone", ValueSetter(this, &HRendererConnectionInfoPrivate::setVerticalKeystone));
    m_valueSetters.insert("Mute", ValueSetter(this, &HRendererConnectionInfoPrivate::setMute));
    m_valueSetters.insert("Volume", ValueSetter(this, &HRendererConnectionInfoPrivate::setVolume));
    m_valueSetters.insert("VolumeDB", ValueSetter(this, &HRendererConnectionInfoPrivate::setVolumeDB));
    m_valueSetters.insert("Loudness", ValueSetter(this, &HRendererConnectionInfoPrivate::setLoudness));

    m_valueGetters.insert("TransportState", ValueGetter(this, &HRendererConnectionInfoPrivate::getTransportState));
    m_valueGetters.insert("TransportStatus", ValueGetter(this, &HRendererConnectionInfoPrivate::getTransportStatus));
    m_valueGetters.insert("CurrentMediaCategory", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentMediaCategory));
    m_valueGetters.insert("PlaybackStorageMedium", ValueGetter(this, &HRendererConnectionInfoPrivate::getPlaybackStorageMedium));
    m_valueGetters.insert("RecordStorageMedium", ValueGetter(this, &HRendererConnectionInfoPrivate::getRecordStorageMedium));
    m_valueGetters.insert("PossiblePlaybackStorageMedia", ValueGetter(this, &HRendererConnectionInfoPrivate::getPossiblePlaybackStorageMedia));
    m_valueGetters.insert("PossibleRecordStorageMedia", ValueGetter(this, &HRendererConnectionInfoPrivate::getPossibleRecordStorageMedia));
    m_valueGetters.insert("CurrentPlayMode", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentPlayMode));
    m_valueGetters.insert("TransportPlaySpeed", ValueGetter(this, &HRendererConnectionInfoPrivate::getTransportPlaySpeed));
    m_valueGetters.insert("RecordMediumWriteStatus", ValueGetter(this, &HRendererConnectionInfoPrivate::getRecordMediumWriteStatus));
    m_valueGetters.insert("CurrentRecordQualityMode", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentRecordQualityMode));
    m_valueGetters.insert("PossibleRecordQualityModes", ValueGetter(this, &HRendererConnectionInfoPrivate::getPossibleRecordQualityModes));
    m_valueGetters.insert("NumberOfTracks", ValueGetter(this, &HRendererConnectionInfoPrivate::getNumberOfTracks));
    m_valueGetters.insert("CurrentTrack", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentTrack));
    m_valueGetters.insert("CurrentTrackDuration", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentTrackDuration));
    m_valueGetters.insert("CurrentMediaDuration", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentMediaDuration));
    m_valueGetters.insert("CurrentTrackMetaData", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentTrackMetaData));
    m_valueGetters.insert("CurrentTrackURI", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentTrackURI));
    m_valueGetters.insert("AVTransportURI", ValueGetter(this, &HRendererConnectionInfoPrivate::getAVTransportURI));
    m_valueGetters.insert("AVTransportURIMetaData", ValueGetter(this, &HRendererConnectionInfoPrivate::getAVTransportURIMetaData));
    m_valueGetters.insert("NextAVTransportURI", ValueGetter(this, &HRendererConnectionInfoPrivate::getNextAVTransportURI));
    m_valueGetters.insert("NextAVTransportURIMetaData", ValueGetter(this, &HRendererConnectionInfoPrivate::getNextAVTransportURIMetaData));
    m_valueGetters.insert("RelativeTimePosition", ValueGetter(this, &HRendererConnectionInfoPrivate::getRelativeTimePosition));
    m_valueGetters.insert("AbsoluteTimePosition", ValueGetter(this, &HRendererConnectionInfoPrivate::getAbsoluteTimePosition));
    m_valueGetters.insert("RelativeCounterPosition", ValueGetter(this, &HRendererConnectionInfoPrivate::getRelativeCounterPosition));
    m_valueGetters.insert("AbsoluteCounterPosition", ValueGetter(this, &HRendererConnectionInfoPrivate::getAbsoluteCounterPosition));
    m_valueGetters.insert("CurrentTransportActions", ValueGetter(this, &HRendererConnectionInfoPrivate::getCurrentTransportActions));
    m_valueGetters.insert("DRMState", ValueGetter(this, &HRendererConnectionInfoPrivate::getDrmState));

    m_valueGetters.insert("Brightness", ValueGetter(this, &HRendererConnectionInfoPrivate::getBrightness));
    m_valueGetters.insert("Contrast", ValueGetter(this, &HRendererConnectionInfoPrivate::getContrast));
    m_valueGetters.insert("Sharpness", ValueGetter(this, &HRendererConnectionInfoPrivate::getSharpness));
    m_valueGetters.insert("RedVideoGain", ValueGetter(this, &HRendererConnectionInfoPrivate::getRedVideoGain));
    m_valueGetters.insert("GreenVideoGain", ValueGetter(this, &HRendererConnectionInfoPrivate::getGreenVideoGain));
    m_valueGetters.insert("BlueVideoGain", ValueGetter(this, &HRendererConnectionInfoPrivate::getBlueVideoGain));
    m_valueGetters.insert("RedVideoBlackLevel", ValueGetter(this, &HRendererConnectionInfoPrivate::getRedVideoBlackLevel));
    m_valueGetters.insert("GreenVideoBlackLevel", ValueGetter(this, &HRendererConnectionInfoPrivate::getGreenVideoBlackLevel));
    m_valueGetters.insert("BlueVideoBlackLevel", ValueGetter(this, &HRendererConnectionInfoPrivate::getBlueVideoBlackLevel));
    m_valueGetters.insert("ColorTemperature", ValueGetter(this, &HRendererConnectionInfoPrivate::getColorTemperature));
    m_valueGetters.insert("HorizontalKeystone", ValueGetter(this, &HRendererConnectionInfoPrivate::getHorizontalKeystone));
    m_valueGetters.insert("VerticalKeystone", ValueGetter(this, &HRendererConnectionInfoPrivate::getVerticalKeystone));
    m_valueGetters.insert("Mute", ValueGetter(this, &HRendererConnectionInfoPrivate::getMute));
    m_valueGetters.insert("Volume", ValueGetter(this, &HRendererConnectionInfoPrivate::getVolume));
    m_valueGetters.insert("VolumeDB", ValueGetter(this, &HRendererConnectionInfoPrivate::getVolumeDB));
    m_valueGetters.insert("Loudness", ValueGetter(this, &HRendererConnectionInfoPrivate::getLoudness));
}

HRendererConnectionInfoPrivate::~HRendererConnectionInfoPrivate()
{
    qDeleteAll(m_channelInfo);
}

HChannelInformation* HRendererConnectionInfoPrivate::checkAndAddChannel(const HChannel& channel)
{
    if (!channel.isValid())
    {
        return 0;
    }

    HChannelInformation* info = getChannel(channel);
    if (!info)
    {
        info = new HChannelInformation(channel);
        m_channelInfo.append(info);
    }

    return info;
}

HChannelInformation* HRendererConnectionInfoPrivate::getChannel(const HChannel& channel) const
{
    foreach(HChannelInformation* ch, m_channelInfo)
    {
        if (channel == ch->channel())
        {
            return ch;
        }
    }
    return 0;
}

void HRendererConnectionInfoPrivate::setCurrentTransportActions(const QString& value, const HChannel&)
{
    QSet<HTransportAction> actions;
    foreach(const QString& arg, value.split(','))
    {
        HTransportAction action = arg;
        if (action.isValid())
        {
            actions.insert(action);
        }
    }
    q_ptr->setCurrentTransportActions(actions);
}

void HRendererConnectionInfoPrivate::setCurrentMediaCategory(const QString& value, const HChannel&)
{
    q_ptr->setCurrentMediaCategory(HMediaInfo::mediaCategoryFromString(value));
}

void HRendererConnectionInfoPrivate::setDrmState(const QString& value, const HChannel&)
{
    q_ptr->setDrmState(HAvTransportInfo::drmStateFromString(value));
}

void HRendererConnectionInfoPrivate::setPossiblePlaybackStorageMedia(const QString& value, const HChannel&)
{
    QSet<HStorageMedium> mediums;
    foreach(const QString& arg, value.split(','))
    {
        HStorageMedium medium = arg;
        if (medium.isValid())
        {
            mediums.insert(medium);
        }
    }
    q_ptr->setPossiblePlaybackStorageMedia(mediums);
}

void HRendererConnectionInfoPrivate::setPossibleRecordStorageMedia(const QString& value, const HChannel&)
{
    QSet<HStorageMedium> mediums;
    foreach(const QString& arg, value.split(','))
    {
        HStorageMedium medium = arg;
        if (medium.isValid())
        {
            mediums.insert(medium);
        }
    }
    q_ptr->setPossibleRecordStorageMedia(mediums);
}

void HRendererConnectionInfoPrivate::setPossibleRecordQualityModes(const QString& value, const HChannel&)
{
    QSet<HRecordQualityMode> modes;
    foreach(const QString& arg, value.split(','))
    {
        HRecordQualityMode mode = arg;
        if (mode.isValid())
        {
            modes.insert(mode);
        }
    }
    q_ptr->setPossibleRecordQualityModes(modes);
}

void HRendererConnectionInfoPrivate::setNumberOfTracks(const QString& value, const HChannel&)
{
    q_ptr->setNumberOfTracks(value.toUInt());
}

void HRendererConnectionInfoPrivate::setCurrentMediaDuration(const QString& value, const HChannel&)
{
    q_ptr->setCurrentMediaDuration(value);
}

void HRendererConnectionInfoPrivate::setAVTransportURI(const QString& value, const HChannel&)
{
    q_ptr->setCurrentResourceUri(value);
}

void HRendererConnectionInfoPrivate::setAVTransportURIMetaData(const QString& value, const HChannel&)
{
    q_ptr->setCurrentResourceMetadata(value);
}

void HRendererConnectionInfoPrivate::setNextAVTransportURI(const QString& value, const HChannel&)
{
    q_ptr->setNextResourceUri(value);
}

void HRendererConnectionInfoPrivate::setNextAVTransportURIMetaData(const QString& value, const HChannel&)
{
    q_ptr->setNextResourceMetadata(value);
}

void HRendererConnectionInfoPrivate::setPlaybackStorageMedium(const QString& value, const HChannel&)
{
    q_ptr->setPlaybackStorageMedium(value);
}

void HRendererConnectionInfoPrivate::setRecordStorageMedium(const QString& value, const HChannel&)
{
    q_ptr->setRecordStorageMedium(value);
}

void HRendererConnectionInfoPrivate::setRecordMediumWriteStatus(const QString& value, const HChannel&)
{
    q_ptr->setRecordMediumWriteStatus(value);
}

void HRendererConnectionInfoPrivate::setCurrentTrack(const QString& value, const HChannel&)
{
    q_ptr->setCurrentTrack(value.toUInt());
}

void HRendererConnectionInfoPrivate::setCurrentTrackDuration(const QString& value, const HChannel&)
{
    q_ptr->setCurrentTrackDuration(value);
}

void HRendererConnectionInfoPrivate::setCurrentTrackMetaData(const QString& value, const HChannel&)
{
    q_ptr->setCurrentTrackMetadata(value);
}

void HRendererConnectionInfoPrivate::setCurrentTrackURI(const QString& value, const HChannel&)
{
    q_ptr->setCurrentTrackUri(value);
}

void HRendererConnectionInfoPrivate::setRelativeTimePosition(const QString& value, const HChannel&)
{
    q_ptr->setRelativeTimePosition(value);
}

void HRendererConnectionInfoPrivate::setAbsoluteTimePosition(const QString& value, const HChannel&)
{
    q_ptr->setAbsoluteTimePosition(value);
}

void HRendererConnectionInfoPrivate::setRelativeCounterPosition(const QString& value, const HChannel&)
{
    q_ptr->setRelativeCounterPosition(value.toUInt());
}

void HRendererConnectionInfoPrivate::setAbsoluteCounterPosition(const QString& value, const HChannel&)
{
    q_ptr->setAbsoluteCounterPosition(value.toUInt());
}

void HRendererConnectionInfoPrivate::setTransportState(const QString& value, const HChannel&)
{
    q_ptr->setTransportState(value);
}

void HRendererConnectionInfoPrivate::setTransportStatus(const QString& value, const HChannel&)
{
    q_ptr->setTransportStatus(value);
}

void HRendererConnectionInfoPrivate::setTransportPlaySpeed(const QString& value, const HChannel&)
{
    q_ptr->setTransportPlaySpeed(value);
}

void HRendererConnectionInfoPrivate::setCurrentPlayMode(const QString& value, const HChannel&)
{
    q_ptr->setCurrentPlayMode(value);
}

void HRendererConnectionInfoPrivate::setCurrentRecordQualityMode(const QString& value, const HChannel&)
{
    q_ptr->setCurrentRecordQualityMode(value);
}

void HRendererConnectionInfoPrivate::setBrightness(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::Brightness, value.toUShort());
}

void HRendererConnectionInfoPrivate::setContrast(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::Contrast, value.toUShort());
}

void HRendererConnectionInfoPrivate::setSharpness(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::Sharpness, value.toUShort());
}

void HRendererConnectionInfoPrivate::setRedVideoGain(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::RedVideoGain, value.toUShort());
}

void HRendererConnectionInfoPrivate::setGreenVideoGain(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::GreenVideoGain, value.toUShort());
}

void HRendererConnectionInfoPrivate::setBlueVideoGain(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::BlueVideoGain, value.toUShort());
}

void HRendererConnectionInfoPrivate::setRedVideoBlackLevel(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::RedVideoBlackLevel, value.toUShort());
}

void HRendererConnectionInfoPrivate::setGreenVideoBlackLevel(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::GreenVideoBlackLevel, value.toUShort());
}

void HRendererConnectionInfoPrivate::setBlueVideoBlackLevel(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::BlueVideoBlackLevel, value.toUShort());
}

void HRendererConnectionInfoPrivate::setColorTemperature(const QString& value, const HChannel&)
{
    q_ptr->setRcsValue(HRendererConnectionInfo::ColorTemperature, value.toUShort());
}

void HRendererConnectionInfoPrivate::setHorizontalKeystone(const QString& value, const HChannel&)
{
    q_ptr->setHorizontalKeystone(value.toShort());
}

void HRendererConnectionInfoPrivate::setVerticalKeystone(const QString& value, const HChannel&)
{
    q_ptr->setVerticalKeystone(value.toShort());
}

void HRendererConnectionInfoPrivate::setMute(const QString& value, const HChannel& ch)
{
    bool ok = false;
    q_ptr->setMute(ch, toBool(value, &ok));
    Q_UNUSED(ok)
}

void HRendererConnectionInfoPrivate::setVolume(const QString& value, const HChannel& ch)
{
    q_ptr->setVolume(ch, value.toUShort());
}

void HRendererConnectionInfoPrivate::setVolumeDB(const QString& value, const HChannel& ch)
{
    q_ptr->setVolumeDb(ch, value.toShort());
}

void HRendererConnectionInfoPrivate::setLoudness(const QString& value, const HChannel& ch)
{
    bool ok = false;
    q_ptr->setLoudness(ch, toBool(value, &ok));
    Q_UNUSED(ok)
}

QString HRendererConnectionInfoPrivate::getCurrentMediaCategory(const HChannel&) const
{
    return HMediaInfo::toString(m_mediaInfo.mediaCategory());
}

QString HRendererConnectionInfoPrivate::getCurrentTransportActions(const HChannel&) const
{
    return strToCsvString(m_transportActions);
}

QString HRendererConnectionInfoPrivate::getDrmState(const HChannel&) const
{
    return HAvTransportInfo::drmStateToString(m_drmState);
}

QString HRendererConnectionInfoPrivate::getPossiblePlaybackStorageMedia(const HChannel&) const
{
    return strToCsvString(m_deviceCapabilities.playMedia());
}

QString HRendererConnectionInfoPrivate::getPossibleRecordStorageMedia(const HChannel&) const
{
    return strToCsvString(m_deviceCapabilities.recordMedia());
}

QString HRendererConnectionInfoPrivate::getPossibleRecordQualityModes(const HChannel&) const
{
    return strToCsvString(m_deviceCapabilities.recordQualityModes());
}

QString HRendererConnectionInfoPrivate::getNumberOfTracks(const HChannel&) const
{
    return QString::number(m_mediaInfo.numberOfTracks());
}

QString HRendererConnectionInfoPrivate::getCurrentMediaDuration(const HChannel&) const
{
    return m_mediaInfo.mediaDuration().toString();
}

QString HRendererConnectionInfoPrivate::getAVTransportURI(const HChannel&) const
{
    return m_mediaInfo.currentUri().toString();
}

QString HRendererConnectionInfoPrivate::getAVTransportURIMetaData(const HChannel&) const
{
    return m_mediaInfo.currentUriMetadata();
}

QString HRendererConnectionInfoPrivate::getNextAVTransportURI(const HChannel&) const
{
    return m_mediaInfo.nextUri().toString();
}

QString HRendererConnectionInfoPrivate::getNextAVTransportURIMetaData(const HChannel&) const
{
    return m_mediaInfo.nextUriMetadata();
}

QString HRendererConnectionInfoPrivate::getPlaybackStorageMedium(const HChannel&) const
{
    return m_mediaInfo.playMedium().toString();
}

QString HRendererConnectionInfoPrivate::getRecordStorageMedium(const HChannel&) const
{
    return m_mediaInfo.recordMedium().toString();
}

QString HRendererConnectionInfoPrivate::getRecordMediumWriteStatus(const HChannel&) const
{
    return m_mediaInfo.writeStatus().toString();
}

QString HRendererConnectionInfoPrivate::getCurrentTrack(const HChannel&) const
{
    return QString::number(m_positionInfo.track());
}

QString HRendererConnectionInfoPrivate::getCurrentTrackDuration(const HChannel&) const
{
    return m_positionInfo.trackDuration().toString();
}

QString HRendererConnectionInfoPrivate::getCurrentTrackMetaData(const HChannel&) const
{
    return m_positionInfo.trackMetadata();
}

QString HRendererConnectionInfoPrivate::getCurrentTrackURI(const HChannel&) const
{
    return m_positionInfo.trackUri().toString();
}

QString HRendererConnectionInfoPrivate::getRelativeTimePosition(const HChannel&) const
{
    return m_positionInfo.relativeTimePosition().toString();
}

QString HRendererConnectionInfoPrivate::getAbsoluteTimePosition(const HChannel&) const
{
    return m_positionInfo.absoluteTimePosition().toString();
}

QString HRendererConnectionInfoPrivate::getRelativeCounterPosition(const HChannel&) const
{
    return QString::number(m_positionInfo.relativeCounterPosition());
}

QString HRendererConnectionInfoPrivate::getAbsoluteCounterPosition(const HChannel&) const
{
    return QString::number(m_positionInfo.absoluteCounterPosition());
}

QString HRendererConnectionInfoPrivate::getTransportState(const HChannel&) const
{
    return m_transportInfo.state().toString();
}

QString HRendererConnectionInfoPrivate::getTransportStatus(const HChannel&) const
{
    return m_transportInfo.status().toString();
}

QString HRendererConnectionInfoPrivate::getTransportPlaySpeed(const HChannel&) const
{
    return m_transportInfo.speed();
}

QString HRendererConnectionInfoPrivate::getCurrentPlayMode(const HChannel&) const
{
    return m_transportSettings.playMode().toString();
}

QString HRendererConnectionInfoPrivate::getCurrentRecordQualityMode(const HChannel&) const
{
    return m_transportSettings.recordQualityMode().toString();
}

QString HRendererConnectionInfoPrivate::getBrightness(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::Brightness));
}

QString HRendererConnectionInfoPrivate::getContrast(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::Contrast));
}

QString HRendererConnectionInfoPrivate::getSharpness(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::Sharpness));
}

QString HRendererConnectionInfoPrivate::getRedVideoGain(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::RedVideoGain));
}

QString HRendererConnectionInfoPrivate::getGreenVideoGain(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::GreenVideoGain));
}

QString HRendererConnectionInfoPrivate::getBlueVideoGain(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::BlueVideoGain));
}

QString HRendererConnectionInfoPrivate::getRedVideoBlackLevel(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::RedVideoBlackLevel));
}

QString HRendererConnectionInfoPrivate::getGreenVideoBlackLevel(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::GreenVideoBlackLevel));
}

QString HRendererConnectionInfoPrivate::getBlueVideoBlackLevel(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::BlueVideoBlackLevel));
}

QString HRendererConnectionInfoPrivate::getColorTemperature(const HChannel&) const
{
    return QString::number(m_rcsAttributes.value(HRendererConnectionInfo::ColorTemperature));
}

QString HRendererConnectionInfoPrivate::getHorizontalKeystone(const HChannel&) const
{
    return QString::number(m_horizontalKeystone);
}

QString HRendererConnectionInfoPrivate::getVerticalKeystone(const HChannel&) const
{
    return QString::number(m_verticalKeystone);
}

QString HRendererConnectionInfoPrivate::getMute(const HChannel& ch) const
{
    HChannelInformation* chInfo = getChannel(ch);
    return chInfo ? (chInfo->muted ? "1" : "0") : QString();
}

QString HRendererConnectionInfoPrivate::getVolume(const HChannel& ch) const
{
    HChannelInformation* chInfo = getChannel(ch);
    return chInfo ? QString::number(chInfo->volume) : QString();
}

QString HRendererConnectionInfoPrivate::getVolumeDB(const HChannel& ch) const
{
    HChannelInformation* chInfo = getChannel(ch);
    return chInfo ? QString::number(chInfo->volumeDb) : QString();
}

QString HRendererConnectionInfoPrivate::getLoudness(const HChannel& ch) const
{
    HChannelInformation* chInfo = getChannel(ch);
    return chInfo ? (chInfo->loudness ? "1" : "0") : QString();
}

/*******************************************************************************
 * HRendererConnectionInfo
 ******************************************************************************/
HRendererConnectionInfo::HRendererConnectionInfo(
    HRendererConnection* owner) :
        QObject(reinterpret_cast<QObject*>(owner)),
            h_ptr(new HRendererConnectionInfoPrivate())
{
    h_ptr->q_ptr = this;
    h_ptr->m_parent = owner;
}

HRendererConnectionInfo::~HRendererConnectionInfo()
{
    delete h_ptr;
}

HRendererConnection* HRendererConnectionInfo::connection() const
{
    return h_ptr->m_parent;
}

HTransportState HRendererConnectionInfo::transportState() const
{
    return h_ptr->m_transportInfo.state();
}

HTransportStatus HRendererConnectionInfo::transportStatus() const
{
    return h_ptr->m_transportInfo.status();
}

HMediaInfo::MediaCategory HRendererConnectionInfo::currentMediaCategory() const
{
    return h_ptr->m_mediaInfo.mediaCategory();
}

HStorageMedium HRendererConnectionInfo::playbackStorageMedium() const
{
    return h_ptr->m_mediaInfo.playMedium();
}

HStorageMedium HRendererConnectionInfo::recordStorageMedium() const
{
    return h_ptr->m_mediaInfo.recordMedium();
}

QSet<HStorageMedium> HRendererConnectionInfo::possiblePlaybackStorageMedia() const
{
    return h_ptr->m_deviceCapabilities.playMedia();
}

QSet<HStorageMedium> HRendererConnectionInfo::possibleRecordStorageMedia() const
{
    return h_ptr->m_deviceCapabilities.recordMedia();
}

HPlayMode HRendererConnectionInfo::currentPlayMode() const
{
    return h_ptr->m_transportSettings.playMode();
}

QString HRendererConnectionInfo::transportPlaySpeed() const
{
    return h_ptr->m_transportInfo.speed();
}

HRecordMediumWriteStatus HRendererConnectionInfo::recordMediumWriteStatus() const
{
    return h_ptr->m_mediaInfo.writeStatus();
}

HRecordQualityMode HRendererConnectionInfo::currentRecordQualityMode() const
{
    return h_ptr->m_transportSettings.recordQualityMode();
}

QSet<HRecordQualityMode> HRendererConnectionInfo::possibleRecordQualityModes() const
{
    return h_ptr->m_deviceCapabilities.recordQualityModes();
}

quint32 HRendererConnectionInfo::numberOfTracks() const
{
    return h_ptr->m_mediaInfo.numberOfTracks();
}

quint32 HRendererConnectionInfo::currentTrack() const
{
    return h_ptr->m_positionInfo.track();
}

HDuration HRendererConnectionInfo::currentTrackDuration() const
{
    return h_ptr->m_positionInfo.trackDuration();
}

HDuration HRendererConnectionInfo::currentMediaDuration() const
{
    return h_ptr->m_mediaInfo.mediaDuration();
}

QString HRendererConnectionInfo::currentTrackMetadata() const
{
    return h_ptr->m_positionInfo.trackMetadata();
}

QUrl HRendererConnectionInfo::currentTrackUri() const
{
    return h_ptr->m_positionInfo.trackUri();
}

QUrl HRendererConnectionInfo::currentResourceUri() const
{
    return h_ptr->m_mediaInfo.currentUri();
}

QString HRendererConnectionInfo::currentResourceMetadata() const
{
    return h_ptr->m_mediaInfo.currentUriMetadata();
}

QUrl HRendererConnectionInfo::nextResourceUri() const
{
    return h_ptr->m_mediaInfo.nextUri();
}

QString HRendererConnectionInfo::nextResourceMetadata() const
{
    return h_ptr->m_mediaInfo.nextUriMetadata();
}

HDuration HRendererConnectionInfo::relativeTimePosition() const
{
    return h_ptr->m_positionInfo.relativeTimePosition();
}

HDuration HRendererConnectionInfo::absoluteTimePosition() const
{
    return h_ptr->m_positionInfo.absoluteTimePosition();
}

qint32 HRendererConnectionInfo::relativeCounterPosition() const
{
    return h_ptr->m_positionInfo.relativeCounterPosition();
}

qint32 HRendererConnectionInfo::absoluteCounterPosition() const
{
    return h_ptr->m_positionInfo.absoluteCounterPosition();
}

QSet<HTransportAction> HRendererConnectionInfo::currentTransportActions() const
{
    return h_ptr->m_transportActions;
}

HAvTransportInfo::DrmState HRendererConnectionInfo::drmState() const
{
    return h_ptr->m_drmState;
}

HDeviceCapabilities HRendererConnectionInfo::deviceCapabilities() const
{
    return h_ptr->m_deviceCapabilities;
}

HMediaInfo HRendererConnectionInfo::mediaInfo() const
{
    return h_ptr->m_mediaInfo;
}

HPositionInfo HRendererConnectionInfo::positionInfo() const
{
    return h_ptr->m_positionInfo;
}

HTransportInfo HRendererConnectionInfo::transportInfo() const
{
    return h_ptr->m_transportInfo;
}

HTransportSettings HRendererConnectionInfo::transportSettings() const
{
    return h_ptr->m_transportSettings;
}

QSet<QString> HRendererConnectionInfo::presets() const
{
    return h_ptr->m_presets;
}

QString HRendererConnectionInfo::rcsAttributeToString(RcsAttribute arg)
{
    QString retVal = "";
    switch(arg)
    {
    case Brightness:
        retVal = "Brightness";
        break;
    case Contrast:
        retVal = "Contrast";
        break;
    case Sharpness:
        retVal = "Sharpness";
        break;
    case RedVideoGain:
        retVal = "RedVideoGain";
        break;
    case GreenVideoGain:
        retVal = "GreenVideoGain";
        break;
    case BlueVideoGain:
        retVal = "BlueVideoGain";
        break;
    case RedVideoBlackLevel:
        retVal = "RedVideoBlackLevel";
        break;
    case GreenVideoBlackLevel:
        retVal = "GreenVideoBlackLevel";
        break;
    case BlueVideoBlackLevel:
        retVal = "BlueVideoBlackLevel";
        break;
    case ColorTemperature:
        retVal = "ColorTemperature";
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return retVal;
}

quint16 HRendererConnectionInfo::rcsValue(RcsAttribute attr) const
{
    return h_ptr->m_rcsAttributes.value(attr);
}

qint16 HRendererConnectionInfo::horizontalKeystone() const
{
    return h_ptr->m_horizontalKeystone;
}

qint16 HRendererConnectionInfo::verticalKeystone() const
{
    return h_ptr->m_verticalKeystone;
}

bool HRendererConnectionInfo::muted(const HChannel& channel, bool* ok) const
{
    HChannelInformation* info = h_ptr->getChannel(channel);
    if (!info)
    {
        if (ok) { *ok = false; }
        return false;
    }
    if (ok) { *ok = true; }
    return info->muted;
}

quint16 HRendererConnectionInfo::volume(const HChannel& channel, bool* ok) const
{
    HChannelInformation* info = h_ptr->getChannel(channel);
    if (!info)
    {
        if (ok) { *ok = false; }
        return false;
    }
    if (ok) { *ok = true; }
    return info->volume;
}

qint16 HRendererConnectionInfo::volumeDb(const HChannel& channel, bool* ok) const
{
    HChannelInformation* info = h_ptr->getChannel(channel);
    if (!info)
    {
        if (ok) { *ok = false; }
        return false;
    }
    if (ok) { *ok = true; }
    return info->volumeDb;
}

HVolumeDbRangeResult HRendererConnectionInfo::volumeDbRange(
    const HChannel& channel, bool* ok) const
{
    HVolumeDbRangeResult retVal;
    HChannelInformation* info = h_ptr->getChannel(channel);
    if (ok)
    {
        *ok = info;
    }
    if (info)
    {
        retVal = HVolumeDbRangeResult(info->minVolumeDb, info->maxVolumeDb);
    }
    return retVal;
}

bool HRendererConnectionInfo::loudnessEnabled(const HChannel& channel, bool* ok) const
{
    HChannelInformation* info = h_ptr->getChannel(channel);
    if (!info)
    {
        if (ok) { *ok = false; }
        return false;
    }
    if (ok) { *ok = true; }
    return info->loudness;
}

void HRendererConnectionInfo::setTransportState(const HTransportState& arg)
{
    if (arg != transportState())
    {
        h_ptr->m_transportInfo.setState(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "TransportState", h_ptr->getTransportState()));
    }
}

void HRendererConnectionInfo::setTransportStatus(const HTransportStatus& arg)
{
    if (arg != transportStatus())
    {
        h_ptr->m_transportInfo.setStatus(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "TransportStatus", h_ptr->getTransportStatus()));
    }
}

void HRendererConnectionInfo::setCurrentMediaCategory(HMediaInfo::MediaCategory arg)
{
    if (arg != currentMediaCategory())
    {
        h_ptr->m_mediaInfo.setMediaCategory(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentMediaCategory", h_ptr->getCurrentMediaCategory()));
    }
}

void HRendererConnectionInfo::setPlaybackStorageMedium(const HStorageMedium& arg)
{
    if (arg != playbackStorageMedium())
    {
        h_ptr->m_mediaInfo.setPlayMedium(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "PlaybackStorageMedium", h_ptr->getPlaybackStorageMedium()));
    }
}

void HRendererConnectionInfo::setRecordStorageMedium(const HStorageMedium& arg)
{
    if (arg != recordStorageMedium())
    {
        h_ptr->m_mediaInfo.setRecordMedium(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "RecordStorageMedium", h_ptr->getRecordStorageMedium()));
    }
}

void HRendererConnectionInfo::setPossiblePlaybackStorageMedia(
    const QSet<HStorageMedium>& arg)
{
    if (arg != possiblePlaybackStorageMedia())
    {
        h_ptr->m_deviceCapabilities.setPlayMedia(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "PossiblePlaybackStorageMedia", h_ptr->getPossiblePlaybackStorageMedia()));
    }
}

void HRendererConnectionInfo::setPossibleRecordStorageMedia(
    const QSet<HStorageMedium>& arg)
{
    if (arg != possibleRecordStorageMedia())
    {
        h_ptr->m_deviceCapabilities.setRecordMedia(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "PossibleRecordStorageMedia", h_ptr->getPossibleRecordStorageMedia()));
    }
}

void HRendererConnectionInfo::setCurrentPlayMode(const HPlayMode& arg)
{
    if (arg != currentPlayMode())
    {
        h_ptr->m_transportSettings.setPlayMode(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentPlayMode", h_ptr->getCurrentPlayMode()));
    }
}

void HRendererConnectionInfo::setTransportPlaySpeed(const QString& arg)
{
    if (arg != transportPlaySpeed())
    {
        h_ptr->m_transportInfo.setSpeed(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "TransportPlaySpeed", h_ptr->getTransportPlaySpeed()));
    }
}

void HRendererConnectionInfo::setRecordMediumWriteStatus(
    const HRecordMediumWriteStatus& arg)
{
    if (arg != recordMediumWriteStatus())
    {
        h_ptr->m_mediaInfo.setWriteStatus(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "RecordMediumWriteStatus", h_ptr->getRecordMediumWriteStatus()));
    }
}

void HRendererConnectionInfo::setCurrentRecordQualityMode(
    const HRecordQualityMode& arg)
{
    if (arg != currentRecordQualityMode())
    {
        h_ptr->m_transportSettings.setRecordQualityMode(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentRecordQualityMode", h_ptr->getCurrentRecordQualityMode()));
    }
}

void HRendererConnectionInfo::setPossibleRecordQualityModes(
    const QSet<HRecordQualityMode>& arg)
{
    if (arg != possibleRecordQualityModes())
    {
        h_ptr->m_deviceCapabilities.setRecordQualityModes(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "PossibleRecordQualityModes", h_ptr->getPossibleRecordQualityModes()));
    }
}

void HRendererConnectionInfo::setNumberOfTracks(quint32 arg)
{
    if (arg != numberOfTracks())
    {
        h_ptr->m_mediaInfo.setNumberOfTracks(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "NumberOfTracks", h_ptr->getNumberOfTracks()));
    }
}

void HRendererConnectionInfo::setCurrentTrack(quint32 arg)
{
    if (arg != currentTrack())
    {
        h_ptr->m_positionInfo.setTrack(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentTrack", h_ptr->getCurrentTrack()));
    }
}

void HRendererConnectionInfo::setCurrentTrackDuration(const HDuration& arg)
{
    if (arg != currentTrackDuration())
    {
        h_ptr->m_positionInfo.setTrackDuration(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentTrackDuration", h_ptr->getCurrentTrackDuration()));
    }
}

void HRendererConnectionInfo::setCurrentMediaDuration(const HDuration& arg)
{
    if (arg != currentMediaDuration())
    {
        h_ptr->m_mediaInfo.setMediaDuration(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentMediaDuration", h_ptr->getCurrentMediaDuration()));
    }
}

void HRendererConnectionInfo::setCurrentTrackMetadata(const QString& arg)
{
    if (arg != currentTrackMetadata())
    {
        h_ptr->m_positionInfo.setTrackMetadata(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentTrackMetaData", h_ptr->getCurrentTrackMetaData()));
    }
}

void HRendererConnectionInfo::setCurrentTrackUri(const QUrl& arg)
{
    if (arg != currentTrackUri())
    {
        h_ptr->m_positionInfo.setTrackUri(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentTrackURI", h_ptr->getCurrentTrackURI()));
    }
}

void HRendererConnectionInfo::setCurrentResourceUri(const QUrl& arg)
{
    if (arg != currentResourceUri())
    {
        h_ptr->m_mediaInfo.setCurrentUri(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "AVTransportURI", h_ptr->getAVTransportURI()));
    }
}

void HRendererConnectionInfo::setCurrentResourceMetadata(const QString& arg)
{
    if (arg != currentResourceMetadata())
    {
        h_ptr->m_mediaInfo.setCurrentUriMetadata(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "AVTransportURIMetaData", h_ptr->getAVTransportURIMetaData()));
    }
}

void HRendererConnectionInfo::setNextResourceUri(const QUrl& arg)
{
    if (arg != nextResourceUri())
    {
        h_ptr->m_mediaInfo.setNextUri(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "NextAVTransportURI", h_ptr->getNextAVTransportURI()));
    }
}

void HRendererConnectionInfo::setNextResourceMetadata(const QString& arg)
{
    if (arg != nextResourceMetadata())
    {
        h_ptr->m_mediaInfo.setNextUriMetadata(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "NextAVTransportURIMetaData", h_ptr->getNextAVTransportURIMetaData()));
    }
}

void HRendererConnectionInfo::setRelativeTimePosition(const HDuration& arg)
{
    if (arg != relativeTimePosition())
    {
        h_ptr->m_positionInfo.setRelativeTimePosition(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "RelativeTimePosition", h_ptr->getRelativeTimePosition()));
    }
}

void HRendererConnectionInfo::setAbsoluteTimePosition(const HDuration& arg)
{
    if (arg != absoluteTimePosition())
    {
        h_ptr->m_positionInfo.setAbsoluteTimePosition(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "AbsoluteTimePosition", h_ptr->getAbsoluteTimePosition()));
    }
}

void HRendererConnectionInfo::setRelativeCounterPosition(qint32 arg)
{
    if (arg != relativeCounterPosition())
    {
        h_ptr->m_positionInfo.setRelativeCounterPosition(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "RelativeCounterPosition", h_ptr->getRelativeCounterPosition()));
    }
}

void HRendererConnectionInfo::setAbsoluteCounterPosition(qint32 arg)
{
    if (arg != absoluteCounterPosition())
    {
        h_ptr->m_positionInfo.setAbsoluteCounterPosition(arg);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "AbsoluteCounterPosition", h_ptr->getAbsoluteCounterPosition()));
    }
}

void HRendererConnectionInfo::setCurrentTransportActions(
    const QSet<HTransportAction>& arg)
{
    if (arg != currentTransportActions())
    {
        h_ptr->m_transportActions = arg;
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "CurrentTransportActions", h_ptr->getCurrentTransportActions()));
    }
}

void HRendererConnectionInfo::setDrmState(HAvTransportInfo::DrmState arg)
{
    if (arg != drmState())
    {
        h_ptr->m_drmState = arg;
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "DRMState", h_ptr->getDrmState()));
    }
}

void HRendererConnectionInfo::setDeviceCapabilities(
    const HDeviceCapabilities& arg)
{
    setPossiblePlaybackStorageMedia(arg.playMedia());
    setPossibleRecordQualityModes(arg.recordQualityModes());
    setPossibleRecordStorageMedia(arg.recordMedia());
}

void HRendererConnectionInfo::setMediaInfo(const HMediaInfo& arg)
{
    setCurrentResourceUri(arg.currentUri());
    setCurrentResourceMetadata(arg.currentUriMetadata());
    setCurrentMediaCategory(arg.mediaCategory());
    setCurrentMediaDuration(arg.mediaDuration());
    setNextResourceUri(arg.nextUri());
    setNextResourceMetadata(arg.nextUriMetadata());
    setNumberOfTracks(arg.numberOfTracks());
    setPlaybackStorageMedium(arg.playMedium());
    setRecordStorageMedium(arg.recordMedium());
    setRecordMediumWriteStatus(arg.writeStatus());
}

void HRendererConnectionInfo::setPositionInfo(const HPositionInfo& arg)
{
    setAbsoluteCounterPosition(arg.absoluteCounterPosition());
    setAbsoluteTimePosition(arg.absoluteTimePosition());
    setRelativeCounterPosition(arg.relativeCounterPosition());
    setRelativeTimePosition(arg.relativeTimePosition());
    setCurrentTrack(arg.track());
    setCurrentTrackDuration(arg.trackDuration());
    setCurrentTrackMetadata(arg.trackMetadata());
    setCurrentTrackUri(arg.trackUri());
}

void HRendererConnectionInfo::setTransportInfo(const HTransportInfo& arg)
{
    setTransportPlaySpeed(arg.speed());
    setTransportState(arg.state());
    setTransportStatus(arg.status());
}

void HRendererConnectionInfo::setTransportSettings(const HTransportSettings& arg)
{
    setCurrentPlayMode(arg.playMode());
    setCurrentRecordQualityMode(arg.recordQualityMode());
}

void HRendererConnectionInfo::setPresets(const QSet<QString>& presets)
{
    h_ptr->m_presets = presets;
}

void HRendererConnectionInfo::setRcsValue(RcsAttribute attr, quint16 value)
{
    if (h_ptr->m_rcsAttributes.value(attr) != value)
    {
        h_ptr->m_rcsAttributes.insert(attr, value);
        emit propertyChanged(this, HRendererConnectionEventInfo(
            rcsAttributeToString(attr), QString::number(value)));
    }
}

void HRendererConnectionInfo::setHorizontalKeystone(qint16 value)
{
    if (horizontalKeystone() != value)
    {
        h_ptr->m_horizontalKeystone = value;
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "HorizontalKeystone", QString::number(value)));
    }
}

void HRendererConnectionInfo::setVerticalKeystone(qint16 value)
{
    if (verticalKeystone() != value)
    {
        h_ptr->m_verticalKeystone = value;
        emit propertyChanged(this, HRendererConnectionEventInfo(
            "VerticalKeystone", QString::number(value)));
    }
}

bool HRendererConnectionInfo::setMute(const HChannel& channel, bool enabled)
{
    HChannelInformation* info = 0;
    if ((info = h_ptr->checkAndAddChannel(channel)))
    {
        if (info->muted != enabled)
        {
            info->muted = enabled;
            emit propertyChanged(this, HRendererConnectionEventInfo(
                "Mute", enabled ? "1" : "0"));
        }
    }
    return info;
}

bool HRendererConnectionInfo::setVolume(const HChannel& channel, quint16 value)
{
    HChannelInformation* info = 0;
    if ((info = h_ptr->checkAndAddChannel(channel)))
    {
        if (info->volume != value)
        {
            info->volume = value;
            emit propertyChanged(this, HRendererConnectionEventInfo(
                "Volume", QString::number(value)));
        }
    }
    return info;
}

bool HRendererConnectionInfo::setVolumeDb(const HChannel& channel, qint16 value)
{
    HChannelInformation* info = 0;
    if ((info = h_ptr->checkAndAddChannel(channel)))
    {
        if (info->volumeDb != value)
        {
            info->volumeDb = value;
            emit propertyChanged(this, HRendererConnectionEventInfo(
                "VolumeDB", QString::number(value)));
        }
    }
    return info;
}

bool HRendererConnectionInfo::setLoudness(const HChannel& channel, bool enabled)
{
    HChannelInformation* info = 0;
    if ((info = h_ptr->checkAndAddChannel(channel)))
    {
        if (info->loudness != enabled)
        {
            info->loudness = enabled;
            emit propertyChanged(this, HRendererConnectionEventInfo(
                "Loudness", enabled ? "1" : "0"));
        }
    }
    return info;
}

QString HRendererConnectionInfo::value(const QString& svName, bool* ok) const
{
    return value(svName, HChannel(), ok);
}

QString HRendererConnectionInfo::value(
    const QString& svName, const HChannel& channel, bool* ok) const
{
    if (h_ptr->m_valueGetters.contains(svName))
    {
        if (ok) { *ok = true; }
        ValueGetter getter = h_ptr->m_valueGetters.value(svName);
        return getter(channel);
    }

    if (ok) { *ok = false; }
    return QString();
}

bool HRendererConnectionInfo::setValue(const QString& svName, const QString& value)
{
    return setValue(svName, HChannel(), value);
}

bool HRendererConnectionInfo::setValue(
    const QString& svName, const HChannel& channel, const QString& value)
{
    if (h_ptr->m_valueSetters.contains(svName))
    {
        ValueSetter setter = h_ptr->m_valueSetters.value(svName);
        setter(value, channel);
        return true;
    }
    return false;
}

bool HRendererConnectionInfo::hasChannelAssociated(const QString& svName)
{
    bool retVal = false;
    QString trimmed = svName.trimmed();
    if (trimmed.compare("Mute", Qt::CaseInsensitive) == 0)
    {
        retVal = true;
    }
    else if (trimmed.compare("Volume", Qt::CaseInsensitive) == 0)
    {
        retVal = true;
    }
    else if (trimmed.compare("VolumeDB", Qt::CaseInsensitive) == 0)
    {
        retVal = true;
    }
    else if (trimmed.compare("Loudness", Qt::CaseInsensitive) == 0)
    {
        retVal = true;
    }
    return retVal;
}

}
}
}
