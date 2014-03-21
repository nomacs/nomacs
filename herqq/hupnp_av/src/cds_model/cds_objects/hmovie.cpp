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

#include "hmovie.h"
#include "hmovie_p.h"
#include "../hscheduledtime.h"
#include "../hcontentduration.h"
#include "../../common/hstoragemedium.h"
#include "../model_mgmt/hcdsproperties.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMoviePrivate
 ******************************************************************************/
HMoviePrivate::HMoviePrivate(const QString& clazz, HObject::CdsType cdsType) :
    HVideoItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::upnp_dvdRegionCode));
    insert(inst.get(HCdsProperties::upnp_channelName));
    insert(inst.get(HCdsProperties::upnp_scheduledStartTime));
    insert(inst.get(HCdsProperties::upnp_scheduledEndTime));
    insert(inst.get(HCdsProperties::upnp_scheduledDuration));
    insert(inst.get(HCdsProperties::upnp_programTitle));
    insert(inst.get(HCdsProperties::upnp_seriesTitle));
    insert(inst.get(HCdsProperties::upnp_episodeCount));
    insert(inst.get(HCdsProperties::upnp_episodeNumber));
}

/*******************************************************************************
 * HMovie
 ******************************************************************************/
HMovie::HMovie(const QString& clazz, CdsType cdsType) :
    HVideoItem(*new HMoviePrivate(clazz, cdsType))
{
}

HMovie::HMovie(HMoviePrivate& dd) :
    HVideoItem(dd)
{
}

HMovie::HMovie(
    const QString& title, const QString& parentId, const QString& id) :
        HVideoItem(*new HMoviePrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HMovie::~HMovie()
{
}

HMovie* HMovie::newInstance() const
{
    return new HMovie();
}

void HMovie::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HMovie::setDvdRegionCode(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_dvdRegionCode, arg);
}

void HMovie::setChannelName(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelName, arg);
}

void HMovie::setScheduledStartTime(const HScheduledTime& arg)
{
    setCdsProperty(
        HCdsProperties::upnp_scheduledStartTime,
        QVariant::fromValue(arg));
}

void HMovie::setScheduledEndTime(const HScheduledTime& arg)
{
    setCdsProperty(
        HCdsProperties::upnp_scheduledEndTime,
        QVariant::fromValue(arg));
}

void HMovie::setScheduledDuration(const HContentDuration& arg)
{
    setCdsProperty(
        HCdsProperties::upnp_scheduledDuration,
        QVariant::fromValue(arg));
}

void HMovie::setProgramTitle(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_programTitle, arg);
}

void HMovie::setSeriesTitle(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_seriesTitle, arg);
}

void HMovie::setEpisodeCount(quint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_episodeCount, arg);
}

void HMovie::setEpisodeNumber(quint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_episodeNumber, arg);
}

HStorageMedium HMovie::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

qint32 HMovie::dvdRegionCode() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_dvdRegionCode, &value);
    return value.toInt();
}

QString HMovie::channelName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelName, &value);
    return value.toString();
}

HScheduledTime HMovie::scheduledStartTime() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_scheduledStartTime, &value);
    return value.value<HScheduledTime>();
}

HScheduledTime HMovie::scheduledEndTime() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_scheduledEndTime, &value);
    return value.value<HScheduledTime>();
}

HContentDuration HMovie::scheduledDuration() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_scheduledDuration, &value);
    return value.value<HContentDuration>();
}

QString HMovie::programTitle() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_programTitle, &value);
    return value.toString();
}

QString HMovie::seriesTitle() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_seriesTitle, &value);
    return value.toString();
}

quint32 HMovie::episodeCount() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_episodeCount, &value);
    return value.toUInt();
}

quint32 HMovie::episodeNumber() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_episodeNumber, &value);
    return value.toUInt();
}

}
}
}
