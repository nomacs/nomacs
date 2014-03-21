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

#include "havtransport_info.h"

#include <HUpnpCore/HActionSetup>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/HActionsSetupData>
#include <HUpnpCore/HStateVariablesSetupData>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av_avt AVTransport
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's AV Transport
 * functionality.
 */

/*******************************************************************************
 * HAvTransportInfo
 ******************************************************************************/
HAvTransportInfo::HAvTransportInfo()
{
}

HAvTransportInfo::~HAvTransportInfo()
{
}

QString HAvTransportInfo::drmStateToString(DrmState state)
{
    QString retVal;
    switch(state)
    {
    case DrmState_Ok:
        retVal = "OK";
        break;
    case DrmState_Unknown:
        retVal = "UNKNOWN";
        break;
    case DrmState_ProcessingContentKey:
        retVal = "PROCESSING_CONTENT_KEY";
        break;
    case DrmState_ContentKeyFailure:
        retVal = "CONTENT_KEY_FAILURE";
        break;
    case DrmState_AttemptingAuthentication:
        retVal = "ATTEMPTING_AUTHENTICATION";
        break;
    case DrmState_FailedAuthentication:
        retVal = "FAILED_AUTHENTICATION";
        break;
    case DrmState_NotAuthenticated:
        retVal = "NOT_AUTHENTICATED";
        break;
    case DrmState_DeviceRevocation:
        retVal = "DEVICE_REVOCATION";
        break;
    default:
        break;
    }
    return retVal;
}

HAvTransportInfo::DrmState
    HAvTransportInfo::drmStateFromString(const QString& state)
{
    DrmState retVal = DrmState_Unknown;
    if (state.compare("OK", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_Ok;
    }
    else if (state.compare("UNKNOWN", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_Unknown;
    }
    else if (state.compare("PROCESSING_CONTENT_KEY", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_ProcessingContentKey;
    }
    else if (state.compare("CONTENT_KEY_FAILURE", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_ContentKeyFailure;
    }
    else if (state.compare("ATTEMPTING_AUTHENTICATION", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_AttemptingAuthentication;
    }
    else if (state.compare("FAILED_AUTHENTICATION", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_FailedAuthentication;
    }
    else if (state.compare("NOT_AUTHENTICATED", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_NotAuthenticated;
    }
    else if (state.compare("DEVICE_REVOCATION", Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_DeviceRevocation;
    }
    return retVal;
}

const HResourceType& HAvTransportInfo::supportedServiceType()
{
    static HResourceType retVal("urn:schemas-upnp-org:service:AVTransport:2");
    return retVal;
}

HActionsSetupData HAvTransportInfo::actionsSetupData()
{
    HActionsSetupData retVal;

    HStateVariablesSetupData svSetupData = stateVariablesSetupData();

    HActionArguments inArgsSetup, outArgsSetup;

    // SetAVTransportURI
    HActionSetup actionSetup("SetAVTransportURI", InclusionMandatory);
    inArgsSetup.append(HActionArgument("InstanceID", svSetupData.get("A_ARG_TYPE_InstanceID")));
    inArgsSetup.append(HActionArgument("CurrentURI", svSetupData.get("AVTransportURI")));
    inArgsSetup.append(HActionArgument("CurrentURIMetaData", svSetupData.get("AVTransportURIMetaData")));
    actionSetup.setInputArguments(inArgsSetup);
    retVal.insert(actionSetup);

    // SetNextAVTransportURI
    inArgsSetup.clear();
    actionSetup = HActionSetup("SetNextAVTransportURI", InclusionOptional);
    inArgsSetup.append(HActionArgument("InstanceID", svSetupData.get("A_ARG_TYPE_InstanceID")));
    inArgsSetup.append(HActionArgument("NextURI", svSetupData.get("AVTransportURI")));
    inArgsSetup.append(HActionArgument("NextURIMetaData", svSetupData.get("AVTransportURIMetaData")));
    actionSetup.setInputArguments(inArgsSetup);
    retVal.insert(actionSetup);

    // GetMediaInfo
    inArgsSetup.clear();
    outArgsSetup.clear();
    actionSetup = HActionSetup("GetMediaInfo");
    inArgsSetup.append(HActionArgument("InstanceID", svSetupData.get("A_ARG_TYPE_InstanceID")));
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument("NrTracks", svSetupData.get("NumberOfTracks")));
    outArgsSetup.append(HActionArgument("MediaDuration", svSetupData.get("CurrentMediaDuration")));
    outArgsSetup.append(HActionArgument("CurrentURI", svSetupData.get("AVTransportURI")));
    outArgsSetup.append(HActionArgument("CurrentURIMetaData", svSetupData.get("AVTransportURIMetaData")));
    outArgsSetup.append(HActionArgument("NextURI", svSetupData.get("NextAVTransportURI")));
    outArgsSetup.append(HActionArgument("NextURIMetaData", svSetupData.get("NextAVTransportURIMetaData")));
    outArgsSetup.append(HActionArgument("PlayMedium", svSetupData.get("PlaybackStorageMedium")));
    outArgsSetup.append(HActionArgument("RecordMedium", svSetupData.get("RecordStorageMedium")));
    outArgsSetup.append(HActionArgument("WriteStatus", svSetupData.get("RecordMediumWriteStatus")));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);
    //

    // GetMediaInfo_Ext
    actionSetup = HActionSetup("GetMediaInfo_Ext", 2);
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument("CurrentType", svSetupData.get("CurrentMediaCategory")));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);
    //

    // GetTransportInfo
    outArgsSetup.clear();
    actionSetup = HActionSetup("GetTransportInfo");
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument("CurrentTransportState", svSetupData.get("TransportState")));
    outArgsSetup.append(HActionArgument("CurrentTransportStatus", svSetupData.get("TransportStatus")));
    outArgsSetup.append(HActionArgument("CurrentSpeed", svSetupData.get("TransportPlaySpeed")));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);

    // GetPositionInfo
    outArgsSetup.clear();
    actionSetup = HActionSetup("GetPositionInfo");
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument("Track", svSetupData.get("CurrentTrack")));
    outArgsSetup.append(HActionArgument("TrackDuration", svSetupData.get("CurrentTrackDuration")));
    outArgsSetup.append(HActionArgument("TrackMetaData", svSetupData.get("CurrentTrackMetaData")));
    outArgsSetup.append(HActionArgument("TrackURI", svSetupData.get("CurrentTrackURI")));
    outArgsSetup.append(HActionArgument("RelTime", svSetupData.get("RelativeTimePosition")));
    outArgsSetup.append(HActionArgument("AbsTime", svSetupData.get("AbsoluteTimePosition")));
    outArgsSetup.append(HActionArgument("RelCount", svSetupData.get("RelativeCounterPosition")));
    outArgsSetup.append(HActionArgument("AbsCount", svSetupData.get("AbsoluteCounterPosition")));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);

    // GetDeviceCapabilities
    retVal.insert(HActionSetup("GetDeviceCapabilities"));

    // GetTransportSettings
    retVal.insert(HActionSetup("GetTransportSettings"));

    // Stop
    retVal.insert(HActionSetup("Stop"));

    // Play
    retVal.insert(HActionSetup("Play"));

    // Pause
    retVal.insert(HActionSetup("Pause", InclusionOptional));

    // Record
    retVal.insert(HActionSetup("Record", InclusionOptional));

    // Seek
    retVal.insert(HActionSetup("Seek"));

    // Next
    retVal.insert(HActionSetup("Next"));

    // Previous
    retVal.insert(HActionSetup("Previous"));

    // SetPlayMode
    retVal.insert(HActionSetup("SetPlayMode", InclusionOptional));

    // SetRecordQualityMode
    retVal.insert(HActionSetup("SetRecordQualityMode", InclusionOptional));

    // GetCurrentTransportActions
    retVal.insert(HActionSetup("GetCurrentTransportActions", InclusionOptional));

    // GetDRMState
    retVal.insert(HActionSetup("GetDRMState", 2, InclusionOptional));

    // GetStateVariables
    retVal.insert(HActionSetup("GetStateVariables", 2, InclusionOptional));

    // SetStateVariables
    retVal.insert(HActionSetup("SetStateVariables", 2, InclusionOptional));

    return retVal;
}

HStateVariablesSetupData HAvTransportInfo::stateVariablesSetupData()
{
    HStateVariablesSetupData retVal;

    retVal.insert(HStateVariableInfo("TransportState", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("TransportStatus", HUpnpDataTypes::string));

    HStateVariableInfo setupData("CurrentMediaCategory", HUpnpDataTypes::string);
    setupData.setVersion(2);
    retVal.insert(setupData);

    retVal.insert(HStateVariableInfo("PlaybackStorageMedium", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("RecordStorageMedium", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("PossiblePlaybackStorageMedia", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("PossibleRecordStorageMedia", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("CurrentPlayMode", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("TransportPlaySpeed", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("RecordMediumWriteStatus", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("CurrentRecordQualityMode", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("PossibleRecordQualityModes", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("NumberOfTracks", HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo("CurrentTrack", HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo("CurrentTrackDuration", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("CurrentMediaDuration", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("CurrentTrackMetaData", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("CurrentTrackURI", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("AVTransportURI", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("AVTransportURIMetaData", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("NextAVTransportURI", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("NextAVTransportURIMetaData", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("RelativeTimePosition", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("AbsoluteTimePosition", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("RelativeCounterPosition", HUpnpDataTypes::i4));
    retVal.insert(HStateVariableInfo("AbsoluteCounterPosition", HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo("CurrentTransportActions", HUpnpDataTypes::string, InclusionOptional));
    retVal.insert(HStateVariableInfo("LastChange", HUpnpDataTypes::string));

    setupData = HStateVariableInfo("DRMState", HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    retVal.insert(HStateVariableInfo("A_ARG_TYPE_SeekMode", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_SeekTarget", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_InstanceID", HUpnpDataTypes::ui4));

    setupData = HStateVariableInfo("A_ARG_TYPE_DeviceUDN", HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo("A_ARG_TYPE_ServiceType", HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo("A_ARG_TYPE_ServiceID", HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo("A_ARG_TYPE_StateVariableValuePairs", HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo("A_ARG_TYPE_StateVariableList", HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    return retVal;
}

}
}
}
