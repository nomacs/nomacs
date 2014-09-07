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

#include "hmediainfo.h"
#include "hduration.h"
#include "hrecordmediumwritestatus.h"

#include "../common/hstoragemedium.h"

#include <QtCore/QUrl>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMediaInfoPrivate
 ******************************************************************************/
class HMediaInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HMediaInfoPrivate)

public:

    quint32 m_numOfTracks;
    HDuration m_duration;
    QUrl m_currentUri;
    QString m_currentMetadata;
    QUrl m_nextUri;
    QString m_nextMetadata;
    HStorageMedium m_playMedium;
    HStorageMedium m_recordMedium;
    HRecordMediumWriteStatus m_writeStatus;
    HMediaInfo::MediaCategory m_type;

    HMediaInfoPrivate();

    HMediaInfoPrivate(
        quint32 numOfTracks,
        const HDuration& duration,
        const QUrl& currentUri,
        const QString& currentMetadata,
        const QUrl& nextUri,
        const QString& nextMetadata,
        const HStorageMedium& playMedium,
        const HStorageMedium& recordMedium,
        const HRecordMediumWriteStatus& writeStatus,
        HMediaInfo::MediaCategory type = HMediaInfo::Undefined);
};

HMediaInfoPrivate::HMediaInfoPrivate() :
    m_numOfTracks(), m_duration(), m_currentUri(), m_currentMetadata(),
    m_nextUri(), m_nextMetadata(), m_playMedium(),
    m_recordMedium(), m_writeStatus(), m_type(HMediaInfo::Undefined)
{
}

HMediaInfoPrivate::HMediaInfoPrivate(
    quint32 numOfTracks,
    const HDuration& duration,
    const QUrl& currentUri,
    const QString& currentMetadata,
    const QUrl& nextUri,
    const QString& nextMetadata,
    const HStorageMedium& playMedium,
    const HStorageMedium& recordMedium,
    const HRecordMediumWriteStatus& writeStatus,
    HMediaInfo::MediaCategory type) :
        m_numOfTracks(numOfTracks),
        m_duration(duration),
        m_currentUri(currentUri),
        m_currentMetadata(currentMetadata),
        m_nextUri(nextUri),
        m_nextMetadata(nextMetadata),
        m_playMedium(playMedium),
        m_recordMedium(recordMedium),
        m_writeStatus(writeStatus),
        m_type(type)
{
}

/*******************************************************************************
 * HMediaInfo
 ******************************************************************************/
HMediaInfo::HMediaInfo() :
    h_ptr(new HMediaInfoPrivate())
{
}

HMediaInfo::HMediaInfo(const QUrl& currentUri, const QString& currentMetadata) :
    h_ptr(new HMediaInfoPrivate())
{
    h_ptr->m_currentUri = currentUri;
    h_ptr->m_currentMetadata = currentMetadata;
}

HMediaInfo::HMediaInfo(
    quint32 numOfTracks,
    const HDuration& duration,
    const QUrl& currentUri,
    const QString& currentMetadata,
    const QUrl& nextUri,
    const QString& nextMetadata,
    const HStorageMedium& playMedium,
    const HStorageMedium& recordMedium,
    const HRecordMediumWriteStatus& writeStatus,
    MediaCategory type) :
        h_ptr(new HMediaInfoPrivate(
                numOfTracks, duration, currentUri, currentMetadata, nextUri,
                nextMetadata, playMedium, recordMedium, writeStatus, type))
{
}

HMediaInfo::~HMediaInfo()
{
}

HMediaInfo::HMediaInfo(const HMediaInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HMediaInfo& HMediaInfo::operator=(const HMediaInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

QString HMediaInfo::toString(MediaCategory category)
{
    QString retVal;
    switch(category)
    {
    case Undefined:
        break;
    case NoMedia:
        retVal = "NO_MEDIA";
        break;
    case TrackAware:
        retVal = "TRACK_AWARE";
        break;
    case TrackUnaware:
        retVal = "TRACK_UNAWARE";
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return retVal;
}

HMediaInfo::MediaCategory HMediaInfo::mediaCategoryFromString(
    const QString& category)
{
    MediaCategory retVal = Undefined;
    if (category.compare("NO_MEDIA", Qt::CaseInsensitive) == 0)
    {
        retVal = NoMedia;
    }
    else if (category.compare("TRACK_AWARE", Qt::CaseInsensitive) == 0)
    {
        retVal = TrackAware;
    }
    else if (category.compare("TRACK_UNAWARE", Qt::CaseInsensitive) == 0)
    {
        retVal = TrackUnaware;
    }
    return retVal;
}

bool HMediaInfo::isValid() const
{
    return h_ptr->m_currentUri.isValid() && !h_ptr->m_currentUri.isEmpty();
}

HMediaInfo::MediaCategory HMediaInfo::mediaCategory() const
{
    return h_ptr->m_type;
}

quint32 HMediaInfo::numberOfTracks() const
{
    return h_ptr->m_numOfTracks;
}

HDuration HMediaInfo::mediaDuration() const
{
    return h_ptr->m_duration;
}

QUrl HMediaInfo::currentUri() const
{
    return h_ptr->m_currentUri;
}

QString HMediaInfo::currentUriMetadata() const
{
    return h_ptr->m_currentMetadata;
}

QUrl HMediaInfo::nextUri() const
{
    return h_ptr->m_nextUri;
}

QString HMediaInfo::nextUriMetadata() const
{
    return h_ptr->m_nextMetadata;
}

HStorageMedium HMediaInfo::playMedium() const
{
    return h_ptr->m_playMedium;
}

HStorageMedium HMediaInfo::recordMedium() const
{
    return h_ptr->m_recordMedium;
}

HRecordMediumWriteStatus HMediaInfo::writeStatus() const
{
    return h_ptr->m_writeStatus;
}

void HMediaInfo::setMediaCategory(MediaCategory arg)
{
    h_ptr->m_type = arg;
}

void HMediaInfo::setNumberOfTracks(quint32 arg)
{
    h_ptr->m_numOfTracks = arg;
}

void HMediaInfo::setMediaDuration(const HDuration& arg)
{
    h_ptr->m_duration = arg;
}

void HMediaInfo::setCurrentUri(const QUrl& arg)
{
    h_ptr->m_currentUri = arg;
}

void HMediaInfo::setCurrentUriMetadata(const QString &arg)
{
    h_ptr->m_currentMetadata = arg;
}

void HMediaInfo::setNextUri(const QUrl& arg)
{
    h_ptr->m_nextUri = arg;
}

void HMediaInfo::setNextUriMetadata(const QString& arg)
{
    h_ptr->m_nextMetadata = arg;
}

void HMediaInfo::setPlayMedium(const HStorageMedium& arg)
{
    h_ptr->m_playMedium = arg;
}

void HMediaInfo::setRecordMedium(const HStorageMedium& arg)
{
    h_ptr->m_recordMedium = arg;
}

void HMediaInfo::setWriteStatus(const HRecordMediumWriteStatus &arg)
{
    h_ptr->m_writeStatus = arg;
}

bool operator==(const HMediaInfo& obj1, const HMediaInfo& obj2)
{
    return obj1.mediaCategory() == obj2.mediaCategory() &&
           obj1.currentUri() == obj2.currentUri() &&
           obj1.currentUriMetadata() == obj2.currentUriMetadata() &&
           obj1.mediaDuration() == obj2.mediaDuration() &&
           obj1.nextUri() == obj2.nextUri() &&
           obj1.nextUriMetadata() == obj2.nextUriMetadata() &&
           obj1.numberOfTracks() == obj2.numberOfTracks() &&
           obj1.playMedium() == obj2.playMedium() &&
           obj1.recordMedium() == obj2.recordMedium() &&
           obj1.writeStatus() == obj2.writeStatus();
}

}
}
}
