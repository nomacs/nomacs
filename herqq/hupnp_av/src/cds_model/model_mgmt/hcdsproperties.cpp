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

#include "hcdsproperties.h"

#include <HUpnpAv/HStateVariableCollection>
#include <HUpnpAv/HCdsPropertyInfo>
#include <HUpnpAv/HChannelGroupName>
#include <HUpnpAv/HForeignMetadata>
#include <HUpnpAv/HContentDuration>
#include <HUpnpAv/HScheduledTime>
#include <HUpnpAv/HDateTimeRange>
#include <HUpnpAv/HProgramCode>
#include <HUpnpAv/HMatchingId>
#include <HUpnpAv/HDeviceUdn>
#include <HUpnpAv/HRadioBand>
#include <HUpnpAv/HChannelId>
#include <HUpnpAv/HObject>
#include <HUpnpAv/HPrice>

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QMutexLocker>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HCdsProperties* HCdsProperties::s_instance = 0;
QMutex* HCdsProperties::s_instanceLock = new QMutex();

class HCdsPropertiesPrivate
{
H_DISABLE_COPY(HCdsPropertiesPrivate)

public:

    QVector<const HCdsPropertyInfo*> m_propertyInfos;
    QHash<const QString, const HCdsPropertyInfo*> m_propertyInfosHash;

    inline HCdsPropertiesPrivate(){}
    inline ~HCdsPropertiesPrivate()
    {
        qDeleteAll(m_propertyInfos);
    }

    inline void insert(HCdsPropertyInfo* obj)
    {
        Q_ASSERT(obj);
        m_propertyInfos.append(obj);
        m_propertyInfosHash.insert(obj->name(), obj);
    }
};

namespace
{
bool lessThan(const HCdsPropertyInfo* obj1, const HCdsPropertyInfo* obj2)
{
     return obj1->name() < obj2->name();
}
}

HCdsProperties::HCdsProperties() :
    h_ptr(new HCdsPropertiesPrivate())
{
    h_ptr->m_propertyInfos.reserve(92);

    HCdsPropertyInfo* obj = new HCdsPropertyInfo(HCdsPropertyInfo::empty());
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "@id", HCdsProperties::dlite_id, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "@parentID", HCdsProperties::dlite_id, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "@restricted", HCdsProperties::dlite_restricted, QVariant::Bool,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "res", HCdsProperties::dlite_res, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::MultiValued);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "@refID", HCdsProperties::dlite_refId, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "@childCount", HCdsProperties::dlite_childCount, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "@searchable", HCdsProperties::dlite_searchable, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "@neverPlayable", HCdsProperties::dlite_neverPlayable, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:title", HCdsProperties::dc_title, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::Mandatory);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:creator", HCdsProperties::dc_creator, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:description", HCdsProperties::dc_description, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:publisher", HCdsProperties::dc_publisher, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::MultiValued);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:date", HCdsProperties::dc_date, QVariant::DateTime, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:rights", HCdsProperties::dc_rights, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:relation", HCdsProperties::dc_relation, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:language", HCdsProperties::dc_language, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "dc:contributor", HCdsProperties::dc_contributor, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:class", HCdsProperties::upnp_class, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:writeStatus", HCdsProperties::upnp_writeStatus, QVariant::fromValue(HObject::UnknownWriteStatus),
        HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:objectUpdateID", HCdsProperties::upnp_objectUpdateID, QVariant::UInt,
        HCdsPropertyInfo::StandardType | HCdsPropertyInfo::Disableable);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:bookmarkID", HCdsProperties::upnp_bookmarkID, QVariant::StringList, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:longDescription", HCdsProperties::upnp_longDescription, QVariant::String, HCdsPropertyInfo::StandardType);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:rating", HCdsProperties::upnp_rating, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:album", HCdsProperties::upnp_album, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:genre", HCdsProperties::upnp_genre, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:artist", HCdsProperties::upnp_artist, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:originalTrackNumber", HCdsProperties::upnp_originalTrackNumber, QVariant::UInt, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:producer", HCdsProperties::upnp_producer, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:actor", HCdsProperties::upnp_actor, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:playList", HCdsProperties::upnp_playList, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:director", HCdsProperties::upnp_director, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:playbackCount", HCdsProperties::upnp_playbackCount, QVariant::UInt, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:lastPlaybackTime", HCdsProperties::upnp_lastPlaybackTime, QVariant::DateTime, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:lastPlaybackPosition", HCdsProperties::upnp_lastPlaybackPosition, QVariant::fromValue(HContentDuration()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:recordedStartDateTime", HCdsProperties::upnp_recordedStartDateTime, QVariant::DateTime, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:recordedDuration", HCdsProperties::upnp_recordedDuration, QVariant::fromValue(HContentDuration()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:recordedDayOfWeek", HCdsProperties::upnp_recordedDayOfWeek, QVariant::fromValue(Undefined_DayOfWeek),
        HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:srsRecordScheduleID", HCdsProperties::upnp_srsRecordScheduleID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:srsRecordTaskID", HCdsProperties::upnp_srsRecordTaskID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:author", HCdsProperties::upnp_author, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:storageMedium", HCdsProperties::upnp_storageMedium, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:storageTotal", HCdsProperties::upnp_storageTotal, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:storageUsed", HCdsProperties::upnp_storageUsed, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:storageFree", HCdsProperties::upnp_storageFree, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:storageMaxPartition", HCdsProperties::upnp_storageMaxPartition, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:containerUpdateID", HCdsProperties::upnp_containerUpdateID, QVariant::String,
        HCdsPropertyInfo::StandardType | HCdsPropertyInfo::Disableable);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:totalDeletedChildCount", HCdsProperties::upnp_totalDeletedChildCount, QVariant::UInt,
        HCdsPropertyInfo::StandardType | HCdsPropertyInfo::Disableable);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:createClass", HCdsProperties::upnp_createClass, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:searchClass", HCdsProperties::upnp_searchClass, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:region", HCdsProperties::upnp_region, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:radioCallSign", HCdsProperties::upnp_radioCallSign, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:radioStationID", HCdsProperties::upnp_radioStationID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:radioBand", HCdsProperties::upnp_radioBand, QVariant::fromValue(HRadioBand()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:channelNr", HCdsProperties::upnp_channelNr, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:signalStrength", HCdsProperties::upnp_signalStrength, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:signalLocked", HCdsProperties::upnp_signalLocked, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:tuned", HCdsProperties::upnp_tuned, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:recordable", HCdsProperties::upnp_recordable, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:DVDRegionCode", HCdsProperties::upnp_dvdRegionCode, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:channelName", HCdsProperties::upnp_channelName, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:scheduledStartTime", HCdsProperties::upnp_scheduledStartTime, QVariant::fromValue(HScheduledTime()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:scheduledEndTime", HCdsProperties::upnp_scheduledEndTime, QVariant::fromValue(HScheduledTime()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:scheduledDuration", HCdsProperties::upnp_scheduledDuration, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:programTitle", HCdsProperties::upnp_programTitle, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:seriesTitle", HCdsProperties::upnp_seriesTitle, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:episodeCount", HCdsProperties::upnp_episodeCount, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:episodeNumber", HCdsProperties::upnp_episodeNumber, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:icon", HCdsProperties::upnp_icon, QVariant::Url, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:callSign", HCdsProperties::upnp_callSign, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:price", HCdsProperties::upnp_price, QVariant::fromValue(HPrice()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:payPerView", HCdsProperties::upnp_payPerView, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:bookmarkedObjectID", HCdsProperties::upnp_bookmarkedObjectID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:deviceUDN", HCdsProperties::upnp_deviceUdn, QVariant::fromValue(HDeviceUdn()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:stateVariableCollection", HCdsProperties::upnp_stateVariableCollection,
        QVariant::fromValue(HStateVariableCollection()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:channelGroupName", HCdsProperties::upnp_channelGroupName, QVariant::fromValue(HChannelGroupName()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:epgProviderName", HCdsProperties::upnp_epgProviderName, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:dateTimeRange", HCdsProperties::upnp_dateTimeRange, QVariant::fromValue(HDateTimeRange()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:serviceProvider", HCdsProperties::upnp_serviceProvider, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:programID", HCdsProperties::upnp_programID, QVariant::fromValue(HMatchingId()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:seriesID", HCdsProperties::upnp_seriesID, QVariant::fromValue(HMatchingId()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:channelID", HCdsProperties::upnp_channelID, QVariant::fromValue(HChannelId()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:programCode", HCdsProperties::upnp_programCode, QVariant::fromValue(HProgramCode()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:episodeType", HCdsProperties::upnp_episodeType, QVariant::fromValue(HEpisodeType()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:networkAffiliation", HCdsProperties::upnp_networkAffiliation, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:foreignMetadata", HCdsProperties::upnp_foreignMetadata, QVariant::fromValue(HForeignMetadata()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:artistDiscographyURI", HCdsProperties::upnp_artistDiscographyURI, QVariant::Url, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:lyricsURI", HCdsProperties::upnp_lyricsURI, QVariant::List, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:albumArtURI", HCdsProperties::upnp_albumArtURI, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::MultiValued);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:toc", HCdsProperties::upnp_toc, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        "upnp:userAnnotation", HCdsProperties::upnp_userAnnotation, QVariant::List, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create("desc", HCdsProperties::dlite_desc, QVariant::List, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);
}

HCdsProperties::~HCdsProperties()
{
    delete h_ptr;
}

const HCdsProperties& HCdsProperties::instance()
{
    QMutexLocker locker(s_instanceLock);
    if (!s_instance)
    {
        s_instance = new HCdsProperties();
    }
    return *s_instance;
}

const HCdsPropertyInfo& HCdsProperties::get(Property property) const
{
    return *(*(h_ptr->m_propertyInfos.begin()+property));
}

const HCdsPropertyInfo& HCdsProperties::get(const QString& property) const
{
    if (h_ptr->m_propertyInfosHash.contains(property))
    {
        return *h_ptr->m_propertyInfosHash.value(property);
    }
    return HCdsPropertyInfo::empty();
}

}
}
}
