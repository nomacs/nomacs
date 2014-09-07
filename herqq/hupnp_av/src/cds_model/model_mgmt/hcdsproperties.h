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

#ifndef HCDS_PROPERTIES_H_
#define HCDS_PROPERTIES_H_

#include <HUpnpAv/HUpnpAv>

class QMutex;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsPropertiesPrivate;

/*!
 * \brief This class is used to contain information of CDS object properties.
 *
 * \headerfile hcdsproperties.h HCdsProperties
 *
 * \ingroup hupnp_av_cds_om_mgmt
 *
 * \remarks This class \b is thread-safe.
 */
class H_UPNP_AV_EXPORT HCdsProperties
{
H_DISABLE_COPY(HCdsProperties)

private:

    HCdsPropertiesPrivate* h_ptr;

    static HCdsProperties* s_instance;
    static QMutex* s_instanceLock;

    HCdsProperties();
   ~HCdsProperties();

public:

   /*!
     * \brief Returns the single instance of the class.
     *
     * \return The single instance of the class.
     */
    static const HCdsProperties& instance();

    /*!
     * \brief This enumeration defines all the supported CDS properties.
     */
    enum Property
    {
        /*!
         * This value is used in error scenarios and when the property is not
         * known by HUPnPAv.
         */
        undefined = 0,

        /*!
         * DIDL-Lite, \@id.
         */
        dlite_id,

        /*!
         * DIDL-Lite, \@parentID.
         */
        dlite_parentId,

        /*!
         * DIDL-Lite, \@restricted.
         */
        dlite_restricted,

        /*!
         * DIDL-Lite, \@res.
         */
        dlite_res,

        /*!
         * DIDL-Lite, \@refID.
         */
        dlite_refId,

        /*!
         * DIDL-Lite, \@childCount.
         */
        dlite_childCount,

        /*!
         * DIDL-Lite, \@searchable.
         */
        dlite_searchable,

        /*!
         * DIDL-Lite, \@neverPlayable.
         */
        dlite_neverPlayable,

        /*!
         * dc:title.
         */
        dc_title,

        /*!
         * dc:creator.
         */
        dc_creator,

        /*!
         * dc:description.
         */
        dc_description,

        /*!
         * dc:publisher.
         */
        dc_publisher,

        /*!
         * dc:date.
         */
        dc_date,

        /*!
         * dc:rights.
         */
        dc_rights,

        /*!
         * dc:relation.
         */
        dc_relation,

        /*!
         * dc_language.
         */
        dc_language,

        /*!
         * dc_contributor.
         */
        dc_contributor,

        /*!
         * upnp:class.
         */
        upnp_class,

        /*!
         * upnp:writeStatus.
         */
        upnp_writeStatus,

        /*!
         * upnp:objectUpdateID.
         */
        upnp_objectUpdateID,

        /*!
         * upnp:bookmarkID.
         */
        upnp_bookmarkID,

        /*!
         * upnp:longDescription.
         */
        upnp_longDescription,

        /*!
         * upnp:rating.
         */
        upnp_rating,

        /*!
         * upnp:album.
         */
        upnp_album,

        /*!
         * upnp:genre.
         */
        upnp_genre,

        /*!
         * upnp:artist.
         */
        upnp_artist,

        /*!
         * upnp:originalTrackNumber.
         */
        upnp_originalTrackNumber,

        /*!
         * upnp:producer.
         */
        upnp_producer,

        /*!
         * upnp:actor.
         */
        upnp_actor,

        /*!
         * upnp:playList.
         */
        upnp_playList,

        /*!
         * upnp:director.
         */
        upnp_director,

        /*!
         * upnp:playbackCount.
         */
        upnp_playbackCount,

        /*!
         * upnp:lastPlaybackTime.
         */
        upnp_lastPlaybackTime,

        /*!
         * upnp:lastPlaybackPosition.
         */
        upnp_lastPlaybackPosition,

        /*!
         * upnp:recordedStartDateTime.
         */
        upnp_recordedStartDateTime,

        /*!
         * upnp:recordedDuration.
         */
        upnp_recordedDuration,

        /*!
         * upnp:recordedDayOfWeek.
         */
        upnp_recordedDayOfWeek,

        /*!
         * upnp:srsRecordScheduleID.
         */
        upnp_srsRecordScheduleID,

        /*!
         * upnp:srsRecordTaskID.
         */
        upnp_srsRecordTaskID,

        /*!
         * upnp:author.
         */
        upnp_author,

        /*!
         * upnp:storageMedium.
         */
        upnp_storageMedium,

        /*!
         * upnp:storageTotal.
         */
        upnp_storageTotal,

        /*!
         * upnp:storageUsed.
         */
        upnp_storageUsed,

        /*!
         * upnp:storageFree.
         */
        upnp_storageFree,

        /*!
         * upnp:storageMaxPartition.
         */
        upnp_storageMaxPartition,

        /*!
         * upnp:containerUpdateID.
         */
        upnp_containerUpdateID,

        /*!
         * upnp:totalDeletedChildCount.
         */
        upnp_totalDeletedChildCount,

        /*!
         * upnp:createClass.
         */
        upnp_createClass,

        /*!
         * upnp:searchClass.
         */
        upnp_searchClass,

        /*!
         * upnp:region.
         */
        upnp_region,

        /*!
         * upnp:radioCallSign.
         */
        upnp_radioCallSign,

        /*!
         * upnp:radioStationID.
         */
        upnp_radioStationID,

        /*!
         * upnp:radioBand.
         */
        upnp_radioBand,

        /*!
         * upnp:channelNr.
         */
        upnp_channelNr,

        /*!
         * upnp:signalStrength.
         */
        upnp_signalStrength,

        /*!
         * upnp:signalLocked.
         */
        upnp_signalLocked,

        /*!
         * upnp:tuned.
         */
        upnp_tuned,

        /*!
         * upnp:recordable.
         */
        upnp_recordable,

        /*!
         * upnp:dvdRegionCode.
         */
        upnp_dvdRegionCode,

        /*!
         * upnp:channelName.
         */
        upnp_channelName,

        /*!
         * upnp:scheduledStartTime.
         */
        upnp_scheduledStartTime,

        /*!
         * upnp:scheduledEndTime.
         */
        upnp_scheduledEndTime,

        /*!
         * upnp:scheduledDuration.
         */
        upnp_scheduledDuration,

        /*!
         * upnp:programTitle.
         */
        upnp_programTitle,

        /*!
         * upnp:seriesTitle.
         */
        upnp_seriesTitle,

        /*!
         * upnp:episodeCount.
         */
        upnp_episodeCount,

        /*!
         * upnp:episodeNumber.
         */
        upnp_episodeNumber,

        /*!
         * upnp:icon.
         */
        upnp_icon,

        /*!
         * upnp:callSign.
         */
        upnp_callSign,

        /*!
         * upnp:price.
         */
        upnp_price,

        /*!
         * upnp:payPerView.
         */
        upnp_payPerView,

        /*!
         * upnp:bookmarkedObjectID.
         */
        upnp_bookmarkedObjectID,

        /*!
         * upnp:deviceUdn.
         */
        upnp_deviceUdn,

        /*!
         * upnp:stateVariableCollection.
         */
        upnp_stateVariableCollection,

        /*!
         * upnp:channelGroupName.
         */
        upnp_channelGroupName,

        /*!
         * upnp:epgProviderName.
         */
        upnp_epgProviderName,

        /*!
         * upnp:dateTimeRange.
         */
        upnp_dateTimeRange,

        /*!
         * upnp:serviceProvider.
         */
        upnp_serviceProvider,

        /*!
         * upnp:programID.
         */
        upnp_programID,

        /*!
         * upnp:seriesID.
         */
        upnp_seriesID,

        /*!
         * upnp:channelID.
         */
        upnp_channelID,

        /*!
         * upnp:programCode.
         */
        upnp_programCode,

        /*!
         * upnp:episodeType.
         */
        upnp_episodeType,

        /*!
         * upnp:networkAffiliation.
         */
        upnp_networkAffiliation,

        /*!
         * upnp:foreignMetadata.
         */
        upnp_foreignMetadata,

        /*!
         * upnp:artistDiscographyURI.
         */
        upnp_artistDiscographyURI,

        /*!
         * upnp:lyricsURI.
         */
        upnp_lyricsURI,

        /*!
         * upnp:albumArtURI.
         */
        upnp_albumArtURI,

        /*!
         * upnp:toc
         */
        upnp_toc,

        /*!
         * upnp:userAnnotation.
         */
        upnp_userAnnotation,

        /*!
         * DIDL-Lite \@desc.
         */
        dlite_desc
        // 90
    };

    /*!
     * \brief Returns information of the specified property.
     *
     * \param property specifies the CDS property.
     *
     * \return information of the specified property.
     */
    const HCdsPropertyInfo& get(Property property) const;

    /*!
     * \brief Returns information of the specified property.
     *
     * \param property specifies the CDS property.
     *
     * \return information of the specified property.
     */
    const HCdsPropertyInfo& get(const QString& property) const;
};

}
}
}

#endif /* HCDS_PROPERTIES_H_ */
