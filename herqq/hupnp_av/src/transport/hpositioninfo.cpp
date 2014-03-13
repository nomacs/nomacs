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

#include "hpositioninfo.h"
#include "hduration.h"

#include <QtCore/QUrl>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPositionInfoPrivate
 ******************************************************************************/
class HPositionInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HPositionInfoPrivate)

public:

    quint32 m_track;
    HDuration m_trackDuration;
    QString m_trackMetadata;
    QUrl m_trackUri;
    HDuration m_relTimePos;
    HDuration m_absTimePos;
    qint32 m_relCounterPos;
    quint32 m_absCounterPos;

    HPositionInfoPrivate();

    HPositionInfoPrivate(
        quint32 track,
        const HDuration& trackDuration,
        const QString& trackMetadata,
        const QUrl& trackUri,
        const HDuration& relTimePos,
        const HDuration& absTimePos,
        qint32 relCounterPos,
        quint32 absCounterPos);
};

HPositionInfoPrivate::HPositionInfoPrivate() :
    m_track(0), m_trackDuration(), m_trackMetadata(), m_trackUri(),
    m_relTimePos(), m_absTimePos(), m_relCounterPos(0), m_absCounterPos(0)
{
}

HPositionInfoPrivate::HPositionInfoPrivate(
    quint32 track,
    const HDuration& trackDuration,
    const QString& trackMetadata,
    const QUrl& trackUri,
    const HDuration& relTimePos,
    const HDuration& absTimePos,
    qint32 relCounterPos,
    quint32 absCounterPos) :
        m_track(track),
        m_trackDuration(trackDuration),
        m_trackMetadata(trackMetadata),
        m_trackUri(trackUri),
        m_relTimePos(relTimePos),
        m_absTimePos(absTimePos),
        m_relCounterPos(relCounterPos),
        m_absCounterPos(absCounterPos)
{
}


/*******************************************************************************
 * HPositionInfo
 ******************************************************************************/
HPositionInfo::HPositionInfo() :
    h_ptr(new HPositionInfoPrivate())
{
}

HPositionInfo::HPositionInfo(const QUrl& trackUri, const QString& trackMetadata) :
    h_ptr(new HPositionInfoPrivate())
{
    h_ptr->m_trackUri = trackUri;
    h_ptr->m_trackMetadata = trackMetadata;
}

HPositionInfo::HPositionInfo(
    quint32 track,
    const HDuration& trackDuration,
    const QString& trackMetadata,
    const QUrl& trackUri,
    const HDuration& relTimePos,
    const HDuration& absTimePos,
    qint32 relCounterPos,
    quint32 absCounterPos) :
        h_ptr(new HPositionInfoPrivate(
                track, trackDuration, trackMetadata,
                trackUri, relTimePos, absTimePos, relCounterPos, absCounterPos))
{
}

HPositionInfo::~HPositionInfo()
{
}

HPositionInfo::HPositionInfo(const HPositionInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HPositionInfo& HPositionInfo::operator=(const HPositionInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

bool HPositionInfo::isValid() const
{
    return h_ptr->m_trackUri.isValid() && !h_ptr->m_trackUri.isEmpty();
}

quint32 HPositionInfo::track() const
{
    return h_ptr->m_track;;
}

HDuration HPositionInfo::trackDuration() const
{
    return h_ptr->m_trackDuration;
}

QString HPositionInfo::trackMetadata() const
{
    return h_ptr->m_trackMetadata;
}

QUrl HPositionInfo::trackUri() const
{
    return h_ptr->m_trackUri;
}

HDuration HPositionInfo::relativeTimePosition() const
{
    return h_ptr->m_relTimePos;
}

HDuration HPositionInfo::absoluteTimePosition() const
{
    return h_ptr->m_absTimePos;
}

qint32 HPositionInfo::relativeCounterPosition() const
{
    return h_ptr->m_relCounterPos;
}

qint32 HPositionInfo::absoluteCounterPosition() const
{
    return h_ptr->m_absCounterPos;
}

void HPositionInfo::setTrack(quint32 arg)
{
    h_ptr->m_track = arg;
}

void HPositionInfo::setTrackDuration(const HDuration& arg)
{
    h_ptr->m_trackDuration = arg;
}

void HPositionInfo::setTrackMetadata(const QString& arg)
{
    h_ptr->m_trackMetadata = arg;
}

void HPositionInfo::setTrackUri(const QUrl& arg)
{
    h_ptr->m_trackUri = arg;
}

void HPositionInfo::setRelativeTimePosition(const HDuration& arg)
{
    h_ptr->m_relTimePos = arg;
}

void HPositionInfo::setAbsoluteTimePosition(const HDuration& arg)
{
    h_ptr->m_absTimePos = arg;
}

void HPositionInfo::setRelativeCounterPosition(qint32 arg)
{
    h_ptr->m_relCounterPos = arg;
}

void HPositionInfo::setAbsoluteCounterPosition(quint32 arg)
{
    h_ptr->m_absCounterPos = arg;
}

bool operator==(const HPositionInfo& obj1, const HPositionInfo& obj2)
{
    return obj1.absoluteCounterPosition() == obj2.absoluteCounterPosition() &&
           obj1.absoluteTimePosition() == obj2.absoluteTimePosition() &&
           obj1.relativeCounterPosition() == obj2.relativeCounterPosition() &&
           obj1.relativeTimePosition() == obj2.relativeTimePosition() &&
           obj1.track() == obj2.track() &&
           obj1.trackDuration() == obj2.trackDuration() &&
           obj1.trackMetadata() == obj2.trackMetadata() &&
           obj1.trackUri() == obj2.trackUri();
}

}
}
}
