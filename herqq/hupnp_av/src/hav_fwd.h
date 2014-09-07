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

#ifndef HAV_FWD_H_
#define HAV_FWD_H_

template<typename T>
class QSet;

template<typename T>
class QList;

template<typename T, typename U>
class QHash;

class QString;
class QVariant;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/////////// CDS objects
class HItem;
class HPhoto;
class HAlbum;
class HMovie;
class HObject;
class HPerson;
class HEpgItem;
class HTextItem;
class HContainer;
class HImageItem;
class HVideoItem;
class HAudioBook;
class HAudioItem;
class HMovieGenre;
class HMusicAlbum;
class HMusicGenre;
class HMusicTrack;
class HPhotoAlbum;
class HEpgContainer;
class HAudioProgram;
class HVideoProgram;
class HBookmarkItem;
class HChannelGroup;
class HPlaylistItem;
class HStorageFolder;
class HStorageSystem;
class HStorageVolume;
class HVideoBroadcast;
class HGenreContainer;
class HAudioBroadcast;
class HBookMarkFolder;
class HPlaylistContainer;
class HAudioChannelGroup;
class HVideoChannelGroup;

class HObjectEventInfo;
class HContainerEventInfo;

/*!
 * Type definition for a list of pointers to HObject instances.
 *
 * \ingroup hupnp_av_cds
 *
 * \sa HObject
 */
typedef QList<HObject*> HObjects;

/*!
 * Type definition for a list of pointers to HItem instances.
 *
 * \ingroup hupnp_av_cds
 *
 * \sa HItem
 */
typedef QList<HItem*> HItems;

/*!
 * Type definition for a list of pointers to HContainer instances.
 *
 * \ingroup hupnp_av_cds
 *
 * \sa HContainer
 */
typedef QList<HContainer*> HContainers;

//////////

///////// CDS model handling
class HCdsProperty;
class HCdsPropertyDb;
class HCdsDataSource;
class HCdsProperties;
class HCdsFileSystemReader;
class HFileSystemDataSource;
class HAbstractCdsDataSource;
class HCdsDidlLiteSerializer;
class HCdsDataSourceConfiguration;
class HFileSystemDataSourceConfiguration;
////////

///////// Media Server
class HMediaServerAdapter;
class HAbstractMediaServerDevice;
class HMediaServerDeviceConfiguration;

/*!
 * This is a type definition for a list of pointers to HMediaServerAdapter instances.
 *
 * \ingroup hupnp_av_mediaserver
 *
 * \sa HMediaServerAdapter
 */
typedef QList<HMediaServerAdapter*> HMediaServerAdapters;
////////

///////// Media Renderer
class HConnection;
class HAvtLastChangeInfo;
class HRcsLastChangeInfo;
class HRendererConnection;
class HMediaRendererAdapter;
class HRendererConnectionInfo;
class HAbstractMediaRendererDevice;
class HMediaRendererDeviceConfiguration;

class HRendererConnectionManager;
class HVolumeDbRangeResult;
class HRendererConnectionEventInfo;

/*!
 * This is a type definition for a list of pointers to HConnection instances.
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \sa HConnection
 */
typedef QList<HConnection*> HConnections;

/*!
 * This is a type definition for a list of pointers to HMediaRendererAdapter instances.
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \sa HMediaRendererAdapter
 */
typedef QList<HMediaRendererAdapter*> HMediaRendererAdapters;
////////

///////// Content Directory
class HContentDirectoryAdapter;
class HContentDirectoryService;
class HAbstractContentDirectoryService;
class HContentDirectoryServiceConfiguration;

class HCreateObjectResult;
class HFreeFormQueryResult;
////////

///////// Rendering Control
class HRenderingControlAdapter;
class HRenderingControlService;
class HAbstractRenderingControlService;
////////

///////// AV Transport
class HAvTransportAdapter;
class HAbstractTransportService;

class HPlayMode;
class HTransportInfo;
class HTransportState;
class HTransportStatus;
class HTransportAction;
class HRecordQualityMode;
class HTransportSettings;
class HTransportInstance;
////////

///////// Connection Manager
class HConnectionManagerAdapter;
class HAbstractConnectionManagerService;
class HConnectionManagerServiceConfiguration;

class HProtocolInfoResult;
////////

///////// Scheduled Recording
class HScheduledRecordingAdapter;
class HScheduledRecordingService;
////////

// AV control point
class HAvControlPoint;
class HAvControlPointConfiguration;
//////////

class HBrowseParams;
class HMediaBrowser;
class HCdsLastChangeInfo;
class HAvDeviceModelCreator;
class HAvDeviceModelInfoProvider;
class HPrepareForConnectionResult;

class HDuration;
class HGenre;
class HPrice;
class HRating;
class HChannel;
class HRootDir;
class HResource;
class HSeekInfo;
class HSortInfo;
class HChannelId;
class HRadioBand;
class HMediaInfo;
class HMatchingId;
class HDeviceUdn;
class HProgramCode;
class HCdsClassInfo;
class HProtocolInfo;
class HPositionInfo;
class HSearchResult;
class HDateTimeRange;
class HCdsObjectData;
class HScheduledTime;
class HStorageMedium;
class HPersonWithRole;
class HConnectionInfo;
class HContentDuration;
class HForeignMetadata;
class HChannelGroupName;
class HPropertyCallbacks;
class HDeviceCapabilities;
class HConnectionManagerId;
class HTransferProgressInfo;
class HStateVariableCollection;
class HRecordMediumWriteStatus;

class HCdsPropertyInfo;

/*!
 * This is a type definition for a list of HCdsLastChangeInfo objects.
 *
 * \ingroup hupnp_av_cds_browsing
 *
 * \sa HCdsLastChangeInfo
 */
typedef QList<HCdsLastChangeInfo> HCdsLastChangeInfos;

/*!
 * This is a type definition for a list of HRcsLastChangeInfo objects.
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \sa HRcsLastChangeInfo
 */
typedef QList<HRcsLastChangeInfo> HRcsLastChangeInfos;

/*!
 * This is a type definition for a list of HAvtLastChangeInfo objects.
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \sa HAvtLastChangeInfo
 */
typedef QList<HAvtLastChangeInfo> HAvtLastChangeInfos;

/*!
 * This is a type definition for a list of HProtocolInfo objects.
 *
 * \ingroup hupnp_av_common
 *
 * \sa HProtocolInfo
 */
typedef QList<HProtocolInfo> HProtocolInfos;

/*!
 * This is a type definition for a list of HRootDir objects.
 *
 * \ingroup hupnp_av_cds_ds
 *
 * \sa HRootDir
 */
typedef QList<HRootDir> HRootDirs;

/*!
 * This is a type definition for a list of HResource objects.
 *
 * \ingroup hupnp_av_common
 *
 * \sa HResource
 */
typedef QList<HResource> HResources;

/*!
 * This is a type definition for a hash table, which contains CDS property
 * information. \e Key is a property name of a CDS object and \e value is the
 * value of the property.
 */
typedef QHash<QString, QVariant> HCdsPropertyMap;

}
}
}

#endif /* HAV_FWD_H_ */
