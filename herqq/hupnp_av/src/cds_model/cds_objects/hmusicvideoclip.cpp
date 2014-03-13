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

#include "hmusicvideoclip.h"
#include "hmusicvideoclip_p.h"
#include "../hscheduledtime.h"
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
 * HMusicVideoClipPrivate
 ******************************************************************************/
HMusicVideoClipPrivate::HMusicVideoClipPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HVideoItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_artist));
    insert(inst.get(HCdsProperties::upnp_storageMedium));
    insert(inst.get(HCdsProperties::upnp_album));
    insert(inst.get(HCdsProperties::upnp_scheduledStartTime));
    insert(inst.get(HCdsProperties::upnp_scheduledEndTime));
    insert(inst.get(HCdsProperties::upnp_director));
    insert(inst.get(HCdsProperties::dc_contributor));
    insert(inst.get(HCdsProperties::dc_date));
}

/*******************************************************************************
 * HMusicVideoClip
 ******************************************************************************/
HMusicVideoClip::HMusicVideoClip(const QString& clazz, CdsType cdsType) :
    HVideoItem(*new HMusicVideoClipPrivate(clazz, cdsType))
{
}

HMusicVideoClip::HMusicVideoClip(HMusicVideoClipPrivate& dd) :
    HVideoItem(dd)
{
}

HMusicVideoClip::HMusicVideoClip(
    const QString& title, const QString& parentId, const QString& id) :
        HVideoItem(*new HMusicVideoClipPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HMusicVideoClip::~HMusicVideoClip()
{
}

HMusicVideoClip* HMusicVideoClip::newInstance() const
{
    return new HMusicVideoClip();
}

void HMusicVideoClip::setArtists(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_artist, toList(arg));
}

void HMusicVideoClip::setStorageMedium(const HStorageMedium& arg)
{
    setCdsProperty(HCdsProperties::upnp_storageMedium, QVariant::fromValue(arg));
}

void HMusicVideoClip::setAlbums(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_album, arg);
}

void HMusicVideoClip::setScheduledStartTime(const HScheduledTime& arg)
{
    setCdsProperty(HCdsProperties::upnp_scheduledStartTime, QVariant::fromValue(arg));
}

void HMusicVideoClip::setScheduledEndTime(const HScheduledTime& arg)
{
    setCdsProperty(HCdsProperties::upnp_scheduledEndTime, QVariant::fromValue(arg));
}

void HMusicVideoClip::setDirectors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_director, arg);
}

void HMusicVideoClip::setContributors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_contributor, arg);
}

void HMusicVideoClip::setDate(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::dc_date, arg);
}

QList<HPersonWithRole> HMusicVideoClip::artists() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_artist, &value);
    return toList<HPersonWithRole>(value.toList());
}

HStorageMedium HMusicVideoClip::storageMedium() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_storageMedium, &value);
    return value.value<HStorageMedium>();
}

QStringList HMusicVideoClip::albums() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_album, &value);
    return value.toStringList();
}

HScheduledTime HMusicVideoClip::scheduledStartTime() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_scheduledStartTime, &value);
    return value.value<HScheduledTime>();
}

HScheduledTime HMusicVideoClip::scheduledEndTime() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_scheduledEndTime, &value);
    return value.value<HScheduledTime>();
}

QStringList HMusicVideoClip::directors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_director, &value);
    return value.toStringList();
}

QStringList HMusicVideoClip::contributors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_contributor, &value);
    return value.toStringList();
}

QDateTime HMusicVideoClip::date() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_date, &value);
    return value.toDateTime();
}

}
}
}
