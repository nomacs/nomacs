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

#include "hrenderingcontrol_info.h"

#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HActionsSetupData>
#include <HUpnpCore/HStateVariablesSetupData>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av_rcs RenderingControl
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's RenderingControl
 * functionality.
 */

/*******************************************************************************
 * HRenderingControlInfo
 ******************************************************************************/
HRenderingControlInfo::HRenderingControlInfo()
{
}

HRenderingControlInfo::~HRenderingControlInfo()
{
}

const HResourceType& HRenderingControlInfo::supportedServiceType()
{
    static const HResourceType retVal("urn:schemas-upnp-org:service:RenderingControl:2");
    return retVal;
}

HActionsSetupData HRenderingControlInfo::actionsSetupData()
{
    HActionsSetupData retVal;

    retVal.insert(HActionSetup("ListPresets"));
    retVal.insert(HActionSetup("SelectPreset"));
    retVal.insert(HActionSetup("GetBrightness", InclusionOptional));
    retVal.insert(HActionSetup("SetBrightness", InclusionOptional));
    retVal.insert(HActionSetup("GetContrast", InclusionOptional));
    retVal.insert(HActionSetup("SetContrast", InclusionOptional));
    retVal.insert(HActionSetup("GetSharpness", InclusionOptional));
    retVal.insert(HActionSetup("SetSharpness", InclusionOptional));
    retVal.insert(HActionSetup("GetRedVideoGain", InclusionOptional));
    retVal.insert(HActionSetup("SetRedVideoGain", InclusionOptional));
    retVal.insert(HActionSetup("GetGreenVideoGain", InclusionOptional));
    retVal.insert(HActionSetup("SetGreenVideoGain", InclusionOptional));
    retVal.insert(HActionSetup("GetBlueVideoGain", InclusionOptional));
    retVal.insert(HActionSetup("SetBlueVideoGain", InclusionOptional));
    retVal.insert(HActionSetup("GetRedVideoBlackLevel", InclusionOptional));
    retVal.insert(HActionSetup("SetRedVideoBlackLevel", InclusionOptional));
    retVal.insert(HActionSetup("GetGreenVideoBlackLevel", InclusionOptional));
    retVal.insert(HActionSetup("SetGreenVideoBlackLevel", InclusionOptional));
    retVal.insert(HActionSetup("GetBlueVideoBlackLevel", InclusionOptional));
    retVal.insert(HActionSetup("SetBlueVideoBlackLevel", InclusionOptional));
    retVal.insert(HActionSetup("GetColorTemperature", InclusionOptional));
    retVal.insert(HActionSetup("SetColorTemperature", InclusionOptional));
    retVal.insert(HActionSetup("GetHorizontalKeystone", InclusionOptional));
    retVal.insert(HActionSetup("SetHorizontalKeystone", InclusionOptional));
    retVal.insert(HActionSetup("GetVerticalKeystone", InclusionOptional));
    retVal.insert(HActionSetup("SetVerticalKeystone", InclusionOptional));
    retVal.insert(HActionSetup("GetMute", InclusionOptional));
    retVal.insert(HActionSetup("SetMute", InclusionOptional));
    retVal.insert(HActionSetup("GetVolume", InclusionOptional));
    retVal.insert(HActionSetup("SetVolume", InclusionOptional));
    retVal.insert(HActionSetup("GetVolumeDB", InclusionOptional));
    retVal.insert(HActionSetup("SetVolumeDB", InclusionOptional));
    retVal.insert(HActionSetup("GetVolumeDBRange", InclusionOptional));
    retVal.insert(HActionSetup("GetLoudness", InclusionOptional));
    retVal.insert(HActionSetup("SetLoudness", InclusionOptional));

    HActionSetup setup("GetStateVariables", InclusionOptional);
    setup.setVersion(2);
    retVal.insert(setup);

    setup = HActionSetup("SetStateVariables", InclusionOptional);
    setup.setVersion(2);
    retVal.insert(setup);

    return retVal;
}

HStateVariablesSetupData HRenderingControlInfo::stateVariablesSetupData()
{
    HStateVariablesSetupData retVal;

    retVal.insert(HStateVariableInfo("LastChange", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("PresetNameList", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("Brightness", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("Contrast", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("Sharpness", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("RedVideoGain", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("GreenVideoGain", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("BlueVideoGain", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("RedVideoBlackLevel", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("BlueVideoBlackLevel", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("ColoTemperature", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("HorizontalKeystone", HUpnpDataTypes::i2, InclusionOptional));
    retVal.insert(HStateVariableInfo("VerticalKeystone", HUpnpDataTypes::i2, InclusionOptional));
    retVal.insert(HStateVariableInfo("Mute", HUpnpDataTypes::boolean, InclusionOptional));
    retVal.insert(HStateVariableInfo("Volume", HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo("VolumeDB", HUpnpDataTypes::i2, InclusionOptional));
    retVal.insert(HStateVariableInfo("Loudness", HUpnpDataTypes::boolean, InclusionOptional));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_Channel", HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_InstanceID", HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo("A_ARG_TYPE_PresetName", HUpnpDataTypes::string));

    HStateVariableInfo info("A_ARG_TYPE_DeviceUDN", HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo("A_ARG_TYPE_ServiceType", HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo("A_ARG_TYPE_ServiceID", HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo("A_ARG_TYPE_StateVariableValuePairs", HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo("A_ARG_TYPE_StateVariableList", HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    return retVal;
}

}
}
}
