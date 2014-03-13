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

#include "hepgitem.h"
#include "hepgitem_p.h"

#include "../hprice.h"
#include "../hgenre.h"
#include "../hchannel_id.h"
#include "../hmatching_id.h"
#include "../hprogramcode.h"
#include "../hscheduledtime.h"
#include "../hpersonwithrole.h"
#include "../hforeignmetadata.h"
#include "../hchannelgroupname.h"

#include "../../common/hrating.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HEpgItemPrivate
 ******************************************************************************/
HEpgItemPrivate::HEpgItemPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_channelGroupName));
    insert(inst.get(HCdsProperties::upnp_epgProviderName));
    insert(inst.get(HCdsProperties::upnp_serviceProvider));
    insert(inst.get(HCdsProperties::upnp_channelName));
    insert(inst.get(HCdsProperties::upnp_channelNr));
    insert(inst.get(HCdsProperties::upnp_programTitle));
    insert(inst.get(HCdsProperties::upnp_seriesTitle));
    insert(inst.get(HCdsProperties::upnp_programID));
    insert(inst.get(HCdsProperties::upnp_seriesID));
    insert(inst.get(HCdsProperties::upnp_channelID));
    insert(inst.get(HCdsProperties::upnp_episodeCount));
    insert(inst.get(HCdsProperties::upnp_episodeNumber));
    insert(inst.get(HCdsProperties::upnp_programCode));
    insert(inst.get(HCdsProperties::upnp_rating));
    insert(inst.get(HCdsProperties::upnp_episodeType));
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_artist));
    insert(inst.get(HCdsProperties::upnp_actor));
    insert(inst.get(HCdsProperties::upnp_author));
    insert(inst.get(HCdsProperties::upnp_producer));
    insert(inst.get(HCdsProperties::upnp_director));
    insert(inst.get(HCdsProperties::dc_publisher));
    insert(inst.get(HCdsProperties::dc_contributor));
    insert(inst.get(HCdsProperties::upnp_callSign));
    insert(inst.get(HCdsProperties::upnp_networkAffiliation));
    insert(inst.get(HCdsProperties::upnp_price));
    insert(inst.get(HCdsProperties::upnp_payPerView));
    insert(inst.get(HCdsProperties::upnp_epgProviderName));
    insert(inst.get(HCdsProperties::dc_description));
    insert(inst.get(HCdsProperties::upnp_longDescription));
    insert(inst.get(HCdsProperties::upnp_icon));
    insert(inst.get(HCdsProperties::upnp_region));
    insert(inst.get(HCdsProperties::dc_rights));
    insert(inst.get(HCdsProperties::dc_language));
    insert(inst.get(HCdsProperties::dc_relation));
    insert(inst.get(HCdsProperties::upnp_scheduledStartTime));
    insert(inst.get(HCdsProperties::upnp_scheduledEndTime));
    insert(inst.get(HCdsProperties::upnp_recordable));
    insert(inst.get(HCdsProperties::upnp_foreignMetadata));
}

/*******************************************************************************
 * HEpgItem
 ******************************************************************************/
HEpgItem::HEpgItem(const QString& clazz, CdsType cdsType) :
    HItem(*new HEpgItemPrivate(clazz, cdsType))
{
}

HEpgItem::HEpgItem(HEpgItemPrivate& dd) :
    HItem(dd)
{
}

HEpgItem::HEpgItem(
    const QString& title, const QString& parentId, const QString& id) :
        HItem(*new HEpgItemPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HEpgItem::~HEpgItem()
{
}

HEpgItem* HEpgItem::newInstance() const
{
    return new HEpgItem();
}

HChannelGroupName HEpgItem::channelGroupName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelGroupName, &value);
    return value.value<HChannelGroupName>();
}

QString HEpgItem::epgProviderName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_epgProviderName, &value);
    return value.toString();
}

QString HEpgItem::serviceProvider() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_serviceProvider, &value);
    return value.toString();
}

QString HEpgItem::channelName() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelName, &value);
    return value.toString();
}

qint32 HEpgItem::channelNr() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelNr, &value);
    return value.toInt();
}

QString HEpgItem::programTitle() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_programTitle, &value);
    return value.toString();
}

QString HEpgItem::seriesTitle() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_seriesTitle, &value);
    return value.toString();
}

HMatchingId HEpgItem::programId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_programID, &value);
    return value.value<HMatchingId>();
}

HMatchingId HEpgItem::seriesId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_seriesID, &value);
    return value.value<HMatchingId>();
}

HChannelId HEpgItem::channelId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_channelID, &value);
    return value.value<HChannelId>();
}

qint32 HEpgItem::episodeCount() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_episodeCount, &value);
    return value.toInt();
}

quint32 HEpgItem::episodeNumber() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_episodeNumber, &value);
    return value.toUInt();
}

HProgramCode HEpgItem::programCode() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_programCode, &value);
    return value.value<HProgramCode>();
}

QList<HRating> HEpgItem::ratings() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_rating, &value);
    return toList<HRating>(value.toList());
}

HEpisodeType HEpgItem::episodeType() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_episodeType, &value);
    return value.value<HEpisodeType>();
}

QList<HGenre> HEpgItem::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QList<HPersonWithRole> HEpgItem::artists() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_artist, &value);
    return toList<HPersonWithRole>(value.toList());
}

QList<HPersonWithRole> HEpgItem::actors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_actor, &value);
    return toList<HPersonWithRole>(value.toList());
}

QList<HPersonWithRole> HEpgItem::authors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_author, &value);
    return toList<HPersonWithRole>(value.toList());
}

QStringList HEpgItem::producers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_producer, &value);
    return value.toStringList();
}

QStringList HEpgItem::directors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_director, &value);
    return value.toStringList();
}

QStringList HEpgItem::publishers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_publisher, &value);
    return value.toStringList();
}

QStringList HEpgItem::contributors() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_contributor, &value);
    return value.toStringList();
}

QString HEpgItem::callSign() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_callSign, &value);
    return value.toString();
}

QString HEpgItem::networkAffiliation() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_networkAffiliation, &value);
    return value.toString();
}

HPrice HEpgItem::price() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_price, &value);
    return value.value<HPrice>();
}

bool HEpgItem::payPerView() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_payPerView, &value);
    return value.toBool();
}

QString HEpgItem::description() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_description, &value);
    return value.toString();
}

QString HEpgItem::longDescription() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_longDescription, &value);
    return value.toString();
}

QUrl HEpgItem::icon() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_icon, &value);
    return value.toUrl();
}

QString HEpgItem::region() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_region, &value);
    return value.toString();
}

QStringList HEpgItem::rights() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_rights, &value);
    return value.toStringList();
}

QStringList HEpgItem::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

QList<QUrl> HEpgItem::relations() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_relation, &value);
    return toList<QUrl>(value.toList());
}

HScheduledTime HEpgItem::scheduledStartTime() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_scheduledStartTime, &value);
    return value.value<HScheduledTime>();
}

HScheduledTime HEpgItem::scheduledEndTime() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_scheduledEndTime, &value);
    return value.value<HScheduledTime>();
}

bool HEpgItem::recordable() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_recordable, &value);
    return value.toBool();
}

HForeignMetadata HEpgItem::foreignMetadata() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_foreignMetadata, &value);
    return value.value<HForeignMetadata>();
}

void HEpgItem::setChannelGroupName(const HChannelGroupName& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelGroupName, QVariant::fromValue(arg));
}

void HEpgItem::setEpgProviderName(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_epgProviderName, arg);
}

void HEpgItem::setServiceProvider(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_serviceProvider, arg);
}

void HEpgItem::setChannelName(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelName, arg);
}

void HEpgItem::setChannelNr(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_channelNr, arg);
}

void HEpgItem::setProgramTitle(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_programTitle, arg);
}

void HEpgItem::setSeriesTitle(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_seriesTitle, arg);
}

void HEpgItem::setProgramId(const HMatchingId& arg)
{
    setCdsProperty(HCdsProperties::upnp_programID, QVariant::fromValue(arg));
}

void HEpgItem::setSeriesId(const HMatchingId& arg)
{
    setCdsProperty(HCdsProperties::upnp_seriesID, QVariant::fromValue(arg));
}

void HEpgItem::setChannelId(const HChannelId& arg)
{
    setCdsProperty(HCdsProperties::upnp_channelID, QVariant::fromValue(arg));
}

void HEpgItem::setEpisodeCount(qint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_episodeCount, arg);
}

void HEpgItem::setEpisodeNumber(quint32 arg)
{
    setCdsProperty(HCdsProperties::upnp_episodeNumber, arg);
}

void HEpgItem::setProgramCode(const HProgramCode& arg)
{
    setCdsProperty(HCdsProperties::upnp_programCode, QVariant::fromValue(arg));
}

void HEpgItem::setRatings(const QList<HRating>& arg)
{
    setCdsProperty(HCdsProperties::upnp_rating, toList(arg));
}

void HEpgItem::setEpisodeType(const HEpisodeType& arg)
{
    setCdsProperty(HCdsProperties::upnp_episodeType, QVariant::fromValue(arg));
}

void HEpgItem::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HEpgItem::setArtists(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_artist, toList(arg));
}

void HEpgItem::setActors(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_actor, toList(arg));
}

void HEpgItem::setAuthors(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_author, toList(arg));
}

void HEpgItem::setProducers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_producer, arg);
}

void HEpgItem::setDirectors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_director, arg);
}

void HEpgItem::setPublishers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_publisher, arg);
}

void HEpgItem::setContributors(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_contributor, arg);
}

void HEpgItem::setCallSign(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_callSign, arg);
}

void HEpgItem::setNetworkAffiliation(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_networkAffiliation, arg);
}

void HEpgItem::setPrice(const HPrice& arg)
{
    setCdsProperty(HCdsProperties::upnp_price, QVariant::fromValue(arg));
}

void HEpgItem::setPayPerView(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_payPerView, arg);
}

void HEpgItem::setDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::dc_description, arg);
}

void HEpgItem::setLongDescription(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_longDescription, arg);
}

void HEpgItem::setIcon(const QUrl& arg)
{
    setCdsProperty(HCdsProperties::upnp_icon, arg);
}

void HEpgItem::setRegion(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_region, arg);
}

void HEpgItem::setRights(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_rights, arg);
}

void HEpgItem::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

void HEpgItem::setRelations(const QList<QUrl>& arg)
{
    setCdsProperty(HCdsProperties::dc_relation, toList(arg));
}

void HEpgItem::setScheduledStartTime(const HScheduledTime& arg)
{
    setCdsProperty(HCdsProperties::upnp_scheduledStartTime, QVariant::fromValue(arg));
}

void HEpgItem::setScheduledEndTime(const HScheduledTime& arg)
{
    setCdsProperty(HCdsProperties::upnp_scheduledEndTime, QVariant::fromValue(arg));
}

void HEpgItem::setRecordable(bool arg)
{
    setCdsProperty(HCdsProperties::upnp_recordable, arg);
}

void HEpgItem::setForeignMetadata(const HForeignMetadata& arg)
{
    setCdsProperty(HCdsProperties::upnp_foreignMetadata, QVariant::fromValue(arg));
}

}
}
}
