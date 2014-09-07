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

#ifndef HABSTRACTRENDERINGCONTROL_SERVICE_H_
#define HABSTRACTRENDERINGCONTROL_SERVICE_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HServerService>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAbstractRenderingControlServicePrivate;

/*!
 * This is an abstract base class for implementing the standardized UPnP service type
 * RenderingControl:2.
 *
 * \brief This class marshals the action invocations through the HServerAction interface to
 * virtual methods. It is an ideal choice for a base class when you wish to implement
 * the RenderingControl service in full.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-RenderingControl-v2-Service.pdf>
 * UPnP RenderingControl:2 specification</a>.
 *
 * \headerfile hrenderingcontrol_service.h HRenderingControlService
 *
 * \ingroup hupnp_av_rcs
 *
 * \remarks This class is not thread-safe.
 */

class H_UPNP_AV_EXPORT HAbstractRenderingControlService :
    public HServerService
{
Q_OBJECT
H_DECLARE_PRIVATE(HAbstractRenderingControlService)
H_DISABLE_COPY(HAbstractRenderingControlService)

protected:

    /*!
     * \brief Creates a new instance.
     */
    HAbstractRenderingControlService();

    //
    // \internal
    //
    HAbstractRenderingControlService(HAbstractRenderingControlServicePrivate& dd);

    // Documented in HServerService
    virtual HActionInvokes createActionInvokes();

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAbstractRenderingControlService() = 0;

    /*!
     * \brief Retrieves a list of the currently defined presets.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentPresetNameList specifies a pointer to a \c QStringList that
     * contains the preset names.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 listPresets(
        quint32 instanceId, QStringList* currentPresetNameList) = 0;

    /*!
     * \brief Restores the values of the state variables associated with the
     * specified preset.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param presetName specifies the preset.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 selectPreset(
        quint32 instanceId, const QString& presetName) = 0;

    /*!
     * \brief Retrieves the current value of the Brightness state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentBrightness specifies a pointer to quint16, which will contain
     * the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getBrightness(
        quint32 instanceId, quint16* currentBrightness);

    /*!
     * \brief Sets the value of the Brightness state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredBrightness specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setBrightness(
        quint32 instanceId, quint16 desiredBrightness);

    /*!
     * \brief Retrieves the current value of the Contrast state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentContrast specifies a pointer to \c quint16, which will contain
     * the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getContrast(
        quint32 instanceId, quint16* currentContrast);

    /*!
     * \brief Sets the value of the Contrast state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredContrast specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setContrast(
        quint32 instanceId, quint16 desiredContrast);

    /*!
     * \brief Retrieves the current value of the Sharpness state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentSharpness specifies a pointer to \c quint16, which will contain
     * the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getSharpness(
        quint32 instanceId, quint16* currentSharpness);

    /*!
     * \brief Sets the value of the Sharpness state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredSharpness specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setSharpness(
        quint32 instanceId, quint16 desiredSharpness);

    /*!
     * \brief Retrieves the current value of the RedVideoGain state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentRedVideoGain specifies a pointer to \c quint16,
     * which will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getRedVideoGain(
        quint32 instanceId, quint16* currentRedVideoGain);

    /*!
     * \brief Sets the value of the RedVideoGain state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredRedVideoGain specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setRedVideoGain(
        quint32 instanceId, quint16 desiredRedVideoGain);

    /*!
     * \brief Retrieves the current value of the GreenVideoGain state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentGreenVideoGain specifies a pointer to \c quint16,
     * which will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getGreenVideoGain(
        quint32 instanceId, quint16* currentGreenVideoGain);

    /*!
     * \brief Sets the value of the GreenVideoGain state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredGreenVideoGain specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setGreenVideoGain(
        quint32 instanceId, quint16 desiredGreenVideoGain);

    /*!
     * \brief Retrieves the current value of the BlueVideoGain state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentBlueVideoGain specifies a pointer to \c quint16,
     * which will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getBlueVideoGain(
        quint32 instanceId, quint16* currentBlueVideoGain);

    /*!
     * \brief Sets the value of the BlueVideoGain state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredBlueVideoGain specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setBlueVideoGain(
        quint32 instanceId, quint16 desiredBlueVideoGain);

    /*!
     * \brief Retrieves the current value of the RedVideoBlackLevel state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentRedVideoBlackLevel specifies a pointer to \c quint16, which
     * will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getRedVideoBlackLevel(
        quint32 instanceId, quint16* currentRedVideoBlackLevel);

    /*!
     * \brief Sets the value of the RedVideoBlackLevel state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredRedVideoBlackLevel specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setRedVideoBlackLevel(
        quint32 instanceId, quint16 desiredRedVideoBlackLevel);

    /*!
     * \brief Retrieves the current value of the GreenVideoBlackLevel state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentGreenVideoBlackLevel specifies a pointer to \c quint16,
     * which will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getGreenVideoBlackLevel(
        quint32 instanceId, quint16* currentGreenVideoBlackLevel);

    /*!
     * \brief Sets the value of the GreenVideoBlackLevel state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredGreenVideoBlackLevel specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setGreenVideoBlackLevel(
        quint32 instanceId, quint16 desiredGreenVideoBlackLevel);

    /*!
     * \brief Retrieves the current value of the BlueVideoBlackLevel state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentBlueVideoBlackLevel specifies a pointer to \c quint16, which
     * will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getBlueVideoBlackLevel(
        quint32 instanceId, quint16* currentBlueVideoBlackLevel);

    /*!
     * \brief Sets the value of the BlueVideoBlackLevel state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredBlueVideoBlackLevel specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setBlueVideoBlackLevel(
        quint32 instanceId, quint16 desiredBlueVideoBlackLevel);

    /*!
     * \brief Retrieves the current value of the ColorTemperature state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentColorTemperature specifies a pointer to \c quint16, which
     * will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getColorTemperature(
        quint32 instanceId, quint16* currentColorTemperature);

    /*!
     * \brief Sets the value of the ColorTemperature state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredColorTemperature specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setColorTemperature(
        quint32 instanceId, quint16 desiredColorTemperature);

    /*!
     * \brief Retrieves the current value of the HorizontalKeystone state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentHorizontalKeyStone specifies a pointer to \c quint16,
     * which will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getHorizontalKeystone(
        quint32 instanceId, qint16* currentHorizontalKeyStone);

    /*!
     * \brief Sets the value of the HorizontalKeystone state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredHorizontalKeyStone specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setHorizontalKeystone(
        quint32 instanceId, qint16 desiredHorizontalKeyStone);

    /*!
     * \brief Retrieves the current value of the VerticalKeystone state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param currentVerticalKeyStone specifies a pointer to \c quint16,
     * which will contain the current value of the state variable.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getVerticalKeystone(
        quint32 instanceId, qint16* currentVerticalKeyStone);

    /*!
     * \brief Sets the value of the VerticalKeystone state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param desiredVerticalKeyStone specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setVerticalKeystone(
        quint32 instanceId, qint16 desiredVerticalKeyStone);

    /*!
     * \brief Retrieves the value of the Mute setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param channel specifies the channel.
     *
     * \param currentlyMuted specifies a pointer to \c bool, which will indicate
     * if the specified channel is muted.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getMute(
        quint32 instanceId, const HChannel& channel, bool* currentlyMuted);

    /*!
     * \brief Sets the value of the Mute setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param channel specifies the channel.
     *
     * \param desiredMute specifies whether the channel should be muted.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setMute(
        quint32 instanceId, const HChannel& channel, bool desiredMute);

    /*!
     * \brief Retrieves the value of the Volume setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param channel specifies the channel.
     *
     * \param currentVolume specifies a pointer to \c quint16, which will contain
     * the current volume of the specified channel.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getVolume(
        quint32 instanceId, const HChannel& channel, quint16* currentVolume);

    /*!
     * \brief Sets the value of the Volume setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param channel specifies the channel.
     *
     * \param desiredVolume specifies the desired volume for the specified
     * channel.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setVolume(
        quint32 instanceId, const HChannel& channel, quint16 desiredVolume);

    /*!
     * \brief Retrieves the value of the VolumeDB setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param channel specifies the channel.
     *
     * \param currentVolumeDb specifies a pointer to \c quint16, which will
     * contain the current volume of the specified channel in units of
     * 1/256 decibels.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getVolumeDB(
        quint32 instanceId, const HChannel& channel, qint16* currentVolumeDb);

    /*!
     * \brief Sets the value of the VolumeDB setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param specifies the channel.
     *
     * \param desiredVolumeDb specifies the desired volume for the specified
     * channel in units of 1/256 decibels.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setVolumeDB(
        quint32 instanceId, const HChannel& channel, qint16 desiredVolumeDb);

    /*!
     * \brief Retrieves the valid range for the VolumeDB state variable.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param specifies the channel.
     *
     * \param result specifies a pointer to \c HVolumeDbRangeResult, which will
     * contain the result of the operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getVolumeDBRange(
        quint32 instanceId, const HChannel& channel, HVolumeDbRangeResult*);

    /*!
     * \brief Retrieves the value of the Loudness setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param channel specifies the channel.
     *
     * \param loudnessOn specifies a pointer to \c bool, which will indicate
     * whether loudness is enabled on the specified channel.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getLoudness(
        quint32 instanceId, const HChannel& channel, bool* loudnessOn);

    /*!
     * \brief Sets the value of the Loudness setting of the specified channel.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param specifies the channel.
     *
     * \param loudnessOn specifies whether loudness should be enabled on the
     * specified channel.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setLoudness(
        quint32 instanceId, const HChannel& channel, bool loudnessOn);

    /*!
     * \brief Returns the values of the specified state variables of the specified
     * virtual RenderingControl instance.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param stateVariableNames specifies the names of the state variables, which
     * values are to be returned. If the list contains a single item "*", values
     * of all supported state variables are returned, except \c LastChange,
     * \c PresetNameList and any \c A_ARG_TYPE_xxx variable.
     *
     * \param stateVariableValuePairs specifies a pointer to a \c QString,
     * which will contain the result. The result is a XML document following
     * <a href="http://www.upnp.org/schemas/av/avs-v2.xsd">this schema</a>.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getStateVariables(
        quint32 instanceId, const QSet<QString>& stateVariableNames,
        QString* stateVariableValuePairs);

    /*!
     * \brief Sets the values of the specified state variables.
     *
     * \param instanceId specifies the virtual RenderingControl instance.
     *
     * \param renderingControlUdn specifies the UPnP device that owns the
     * target RenderingControl service.
     *
     * \param serviceType specifies the service type of the target RenderingControl.
     *
     * \param serviceId specifies the serviceId of the target RenderingControl.
     *
     * \param stateVariableValuePairs specifies the state variables and their
     * desired values. This is a XML document following
     * <a href="http://www.upnp.org/schemas/av/avs-v2.xsd">this schema</a>.
     *
     * \param stateVariableList specifies a pointer to a \c QStringList,
     * which contains the names of all the state variables that were successfully
     * modified.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setStateVariables(
        quint32 instanceId, const HUdn& renderingControlUdn,
        const HResourceType& serviceType, const HServiceId& serviceId,
        const QString& stateVariableValuePairs, QStringList* stateVariableList);
};

}
}
}

#endif /* HABSTRACTRENDERINGCONTROL_SERVICE_H_ */
