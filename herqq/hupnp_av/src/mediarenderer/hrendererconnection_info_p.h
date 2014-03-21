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

#ifndef HRENDERERCONNECTION_INFO_P_H_
#define HRENDERERCONNECTION_INFO_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hrendererconnection_info.h"

#include "../common/hstoragemedium.h"

#include "../transport/hduration.h"
#include "../transport/hplaymode.h"
#include "../transport/hseekinfo.h"
#include "../transport/hmediainfo.h"
#include "../transport/hpositioninfo.h"
#include "../transport/htransportinfo.h"
#include "../transport/htransportaction.h"
#include "../transport/havtransport_info.h"
#include "../transport/htransportsettings.h"
#include "../transport/hdevicecapabilities.h"
#include "../transport/hrecordmediumwritestatus.h"

#include "../renderingcontrol/hchannel.h"

#include <HUpnpCore/HFunctor>

#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QHash>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HChannelInformation
{
private:

    HChannel m_channel;

public:

    quint16 volume;
    qint16 volumeDb;
    qint16 minVolumeDb, maxVolumeDb;
    bool muted, loudness;

    HChannelInformation(const HChannel& ch);

    inline const HChannel& channel() const { return m_channel; }
};

typedef Functor<void, H_TYPELIST_2(const QString&, const HChannel&)> ValueSetter;
typedef Functor<QString, H_TYPELIST_1(const HChannel&)> ValueGetter;

//
//
//
class HRendererConnectionInfoPrivate
{
H_DISABLE_COPY(HRendererConnectionInfoPrivate)

public:

    void setCurrentTransportActions(const QString&, const HChannel&);
    void setCurrentMediaCategory(const QString&, const HChannel&);
    void setDrmState(const QString&, const HChannel&);
    void setPossiblePlaybackStorageMedia(const QString&, const HChannel&);
    void setPossibleRecordStorageMedia(const QString&, const HChannel&);
    void setPossibleRecordQualityModes(const QString&, const HChannel&);
    void setNumberOfTracks(const QString&, const HChannel&);
    void setCurrentMediaDuration(const QString&, const HChannel&);
    void setAVTransportURI(const QString&, const HChannel&);
    void setAVTransportURIMetaData(const QString&, const HChannel&);
    void setNextAVTransportURI(const QString&, const HChannel&);
    void setNextAVTransportURIMetaData(const QString&, const HChannel&);
    void setPlaybackStorageMedium(const QString&, const HChannel&);
    void setRecordStorageMedium(const QString&, const HChannel&);
    void setRecordMediumWriteStatus(const QString&, const HChannel&);
    void setCurrentTrack(const QString&, const HChannel&);
    void setCurrentTrackDuration(const QString&, const HChannel&);
    void setCurrentTrackMetaData(const QString&, const HChannel&);
    void setCurrentTrackURI(const QString&, const HChannel&);
    void setRelativeTimePosition(const QString&, const HChannel&);
    void setAbsoluteTimePosition(const QString&, const HChannel&);
    void setRelativeCounterPosition(const QString&, const HChannel&);
    void setAbsoluteCounterPosition(const QString&, const HChannel&);
    void setTransportState(const QString&, const HChannel&);
    void setTransportStatus(const QString&, const HChannel&);
    void setTransportPlaySpeed(const QString&, const HChannel&);
    void setCurrentPlayMode(const QString&, const HChannel&);
    void setCurrentRecordQualityMode(const QString&, const HChannel&);

    void setBrightness(const QString&, const HChannel&);
    void setContrast(const QString&, const HChannel&);
    void setSharpness(const QString&, const HChannel&);
    void setRedVideoGain(const QString&, const HChannel&);
    void setGreenVideoGain(const QString&, const HChannel&);
    void setBlueVideoGain(const QString&, const HChannel&);
    void setRedVideoBlackLevel(const QString&, const HChannel&);
    void setGreenVideoBlackLevel(const QString&, const HChannel&);
    void setBlueVideoBlackLevel(const QString&, const HChannel&);
    void setColorTemperature(const QString&, const HChannel&);
    void setHorizontalKeystone(const QString&, const HChannel&);
    void setVerticalKeystone(const QString&, const HChannel&);
    void setMute(const QString&, const HChannel&);
    void setVolume(const QString&, const HChannel&);
    void setVolumeDB(const QString&, const HChannel&);
    void setLoudness(const QString&, const HChannel&);

    QString getCurrentTransportActions(const HChannel& ch = HChannel()) const;
    QString getCurrentMediaCategory(const HChannel& ch = HChannel()) const;
    QString getDrmState(const HChannel& ch = HChannel()) const;
    QString getPossiblePlaybackStorageMedia(const HChannel& ch = HChannel()) const;
    QString getPossibleRecordStorageMedia(const HChannel& ch = HChannel()) const;
    QString getPossibleRecordQualityModes(const HChannel& ch = HChannel()) const;
    QString getNumberOfTracks(const HChannel& ch = HChannel()) const;
    QString getCurrentMediaDuration(const HChannel& ch = HChannel()) const;
    QString getAVTransportURI(const HChannel& ch = HChannel()) const;
    QString getAVTransportURIMetaData(const HChannel& ch = HChannel()) const;
    QString getNextAVTransportURI(const HChannel& ch = HChannel()) const;
    QString getNextAVTransportURIMetaData(const HChannel& ch = HChannel()) const;
    QString getPlaybackStorageMedium(const HChannel& ch = HChannel()) const;
    QString getRecordStorageMedium(const HChannel& ch = HChannel()) const;
    QString getRecordMediumWriteStatus(const HChannel& ch = HChannel()) const;
    QString getCurrentTrack(const HChannel& ch = HChannel()) const;
    QString getCurrentTrackDuration(const HChannel& ch = HChannel()) const;
    QString getCurrentTrackMetaData(const HChannel& ch = HChannel()) const;
    QString getCurrentTrackURI(const HChannel& ch = HChannel()) const;
    QString getRelativeTimePosition(const HChannel& ch = HChannel()) const;
    QString getAbsoluteTimePosition(const HChannel& ch = HChannel()) const;
    QString getRelativeCounterPosition(const HChannel& ch = HChannel()) const;
    QString getAbsoluteCounterPosition(const HChannel& ch = HChannel()) const;
    QString getTransportState(const HChannel& ch = HChannel()) const;
    QString getTransportStatus(const HChannel& ch = HChannel()) const;
    QString getTransportPlaySpeed(const HChannel& ch = HChannel()) const;
    QString getCurrentPlayMode(const HChannel& ch = HChannel()) const;
    QString getCurrentRecordQualityMode(const HChannel& ch = HChannel()) const;

    QString getBrightness(const HChannel&) const;
    QString getContrast(const HChannel&) const;
    QString getSharpness(const HChannel&) const;
    QString getRedVideoGain(const HChannel&) const;
    QString getGreenVideoGain(const HChannel&) const;
    QString getBlueVideoGain(const HChannel&) const;
    QString getRedVideoBlackLevel(const HChannel&) const;
    QString getGreenVideoBlackLevel(const HChannel&) const;
    QString getBlueVideoBlackLevel(const HChannel&) const;
    QString getColorTemperature(const HChannel&) const;
    QString getHorizontalKeystone(const HChannel&) const;
    QString getVerticalKeystone(const HChannel&) const;
    QString getMute(const HChannel&) const;
    QString getVolume(const HChannel&) const;
    QString getVolumeDB(const HChannel&) const;
    QString getLoudness(const HChannel&) const;

public:

    HRendererConnectionInfo* q_ptr;
    HRendererConnection* m_parent;

    QHash<QString, ValueSetter> m_valueSetters;
    QHash<QString, ValueGetter> m_valueGetters;

    // AVT
    QSet<HTransportAction> m_transportActions;
    HAvTransportInfo::DrmState m_drmState;
    HDeviceCapabilities m_deviceCapabilities;
    HMediaInfo m_mediaInfo;
    HPositionInfo m_positionInfo;
    HTransportInfo m_transportInfo;
    HTransportSettings m_transportSettings;

    // RCS
    QSet<QString> m_presets;
    QHash<HRendererConnectionInfo::RcsAttribute, quint16> m_rcsAttributes;
    qint16 m_horizontalKeystone, m_verticalKeystone;
    QList<HChannelInformation*> m_channelInfo;

    HRendererConnectionInfoPrivate();
    ~HRendererConnectionInfoPrivate();

    HChannelInformation* checkAndAddChannel(const HChannel&);
    HChannelInformation* getChannel(const HChannel&) const;
};

}
}
}

#endif /* HRENDERERCONNECTION_INFO_P_H_ */
