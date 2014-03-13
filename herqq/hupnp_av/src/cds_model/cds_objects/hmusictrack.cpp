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

#include "hmusictrack.h"
#include "hmusictrack_p.h"
#include "../hpersonwithrole.h"
#include "../../common/hstoragemedium.h"

#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QDateTime>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMusicTrackPrivate
 ******************************************************************************/
HMusicTrackPrivate::HMusicTrackPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HAudioItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_artist));
    insert(inst.get(HCdsProperties::upnp_album));
    insert(inst.get(HCdsProperties::upnp_originalTrackNumber));
    insert(inst.get(HCdsProperties::upnp_playList));
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::dc_contributor));
    insert(inst.get(HCdsProperties::dc_date));
}

/*******************************************************************************
 * HMusicTrack
 ******************************************************************************/
HMusicTrack::HMusicTrack(const QString& clazz, CdsType cdsType) :
    HAudioItem(*new HMusicTrackPrivate(clazz, cdsType))
{
}

HMusicTrack::HMusicTrack(
    const QString& title, const QString& parentId, const QString& id) :
        HAudioItem(*new HMusicTrackPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HMusicTrack::~HMusicTrack()
{
}

HMusicTrack* HMusicTrack::newInstance() const
{
    return new HMusicTrack();
}

void HMusicTrack::setArtists(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_artist, toList(arg));
}

void HMusicTrack::setAlbums(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_album, arg);
}

void HMusicTrack::setOriginalTrackNumber(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_originalTrackNumber, arg);
}

void HMusicTrack::setPlaylists(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_playList, arg);
}

void HMusicTrack::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HMusicTrack::setContributors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_contributor, arg);
}

void HMusicTrack::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

QList<HPersonWithRole> HMusicTrack::artists() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_artist, &value);
    return toList<HPersonWithRole>(value.toList());
}

QStringList HMusicTrack::albums() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_album, &value);
    return value.toStringList();
}

qint32 HMusicTrack::originalTrackNumber() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_originalTrackNumber, &value);
    return value.toInt();
}

QStringList HMusicTrack::playlists() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_playList, &value);
    return value.toStringList();
}

HStorageMedium HMusicTrack::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QStringList HMusicTrack::contributors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_contributor, &value);
    return value.toStringList();
}

QDateTime HMusicTrack::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

}
}
}
