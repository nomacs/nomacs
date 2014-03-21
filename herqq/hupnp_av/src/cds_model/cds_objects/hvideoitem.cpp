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

#include "hvideoitem.h"
#include "hvideoitem_p.h"

#include "../hgenre.h"
#include "../hpersonwithrole.h"
#include "../hcontentduration.h"

#include "../../common/hrating.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QUrl>
#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HVideoItemPrivate
 ******************************************************************************/
HVideoItemPrivate::HVideoItemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::upnp_producer));
    insert(inst.get(HCdsProperties::upnp_rating));
    insert(inst.get(HCdsProperties::upnp_actor));
    insert(inst.get(HCdsProperties::upnp_director));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::dc_publisher));
    insert(inst.get(HCdsProperties::dc_language));
    insert(inst.get(HCdsProperties::dc_relation));
    insert(inst.get(HCdsProperties::upnp_playbackCount));
    insert(inst.get(HCdsProperties::upnp_lastPlaybackTime));
    insert(inst.get(HCdsProperties::upnp_lastPlaybackPosition));
    insert(inst.get(HCdsProperties::upnp_recordedDayOfWeek));
    insert(inst.get(HCdsProperties::upnp_srsRecordScheduleID));
}

/*******************************************************************************
 * HVideoItem
 ******************************************************************************/
HVideoItem::HVideoItem(const QString& clazz, CdsType cdsType) :
    HItem(*new HVideoItemPrivate(clazz, cdsType))
{
}

HVideoItem::HVideoItem(HVideoItemPrivate& dd) :
    HItem(dd)
{
}

HVideoItem::HVideoItem(
    const QString& title, const QString& parentId, const QString& id) :
        HItem(*new HVideoItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HVideoItem::~HVideoItem()
{
}

HVideoItem* HVideoItem::newInstance() const
{
    return new HVideoItem();
}

void HVideoItem::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HVideoItem::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HVideoItem::setProducers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_producer, arg);
}

void HVideoItem::setRatings(const QList<HRating>& arg)
{
    setCdsProperty(HCdsProperties::upnp_rating, toList(arg));
}

void HVideoItem::setActors(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_actor, toList(arg));
}

void HVideoItem::setDirectors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_director, arg);
}

void HVideoItem::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

void HVideoItem::setPublishers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_publisher, arg);
}

void HVideoItem::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

void HVideoItem::setRelations(const QList<QUrl>& arg)
{
    setCdsProperty(HCdsProperties::dc_relation, toList(arg));
}

void HVideoItem::setPlaybackCount(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_playbackCount, arg);
}

void HVideoItem::setLastPlaybackTime(const QDateTime& arg)
{
    setCdsProperty(HCdsProperties::upnp_lastPlaybackTime, arg);
}

void HVideoItem::setLastPlaybackPosition(const HContentDuration& arg)
{
    setCdsProperty(HCdsProperties::upnp_lastPlaybackPosition, QVariant::fromValue(arg));
}

void HVideoItem::setRecordedDayOfWeek(HDayOfWeek arg)
{
    setCdsProperty(HCdsProperties::upnp_recordedDayOfWeek, QVariant::fromValue(arg));
}

void HVideoItem::setSrsRecordScheduleId(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_srsRecordScheduleID, arg);
}

QList<HGenre> HVideoItem::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QString HVideoItem::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

QStringList HVideoItem::producers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_producer, &value);
    return value.toStringList();
}

QList<HRating> HVideoItem::ratings() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_rating, &value);
    return toList<HRating>(value.toList());
}

QList<HPersonWithRole> HVideoItem::actors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_actor, &value);
    return toList<HPersonWithRole>(value.toList());
}

QStringList HVideoItem::directors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_director, &value);
    return value.toStringList();
}

QString HVideoItem::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QStringList HVideoItem::publishers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_publisher, &value);
    return value.toStringList();
}

QStringList HVideoItem::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

QList<QUrl> HVideoItem::relations() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_relation, &value);
    return toList<QUrl>(value.toList());
}

qint32 HVideoItem::playbackCount() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_playbackCount, &value);
    return value.toInt();
}

QDateTime HVideoItem::lastPlaybackTime() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_lastPlaybackTime, &value);
    return value.toDateTime();
}

HContentDuration HVideoItem::lastPlaybackPosition() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_lastPlaybackPosition, &value);
    return value.value<HContentDuration>();
}

HDayOfWeek HVideoItem::recordedDayOfWeek() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_recordedDayOfWeek, &value);
    return value.value<HDayOfWeek>();
}

QString HVideoItem::srsRecordScheduleId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_srsRecordScheduleID, &value);
    return value.toString();
}

}
}
}
