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

#ifndef HRENDERINGCONTROL_ADAPTER_H_
#define HRENDERINGCONTROL_ADAPTER_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClientServiceAdapter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HRenderingControlAdapterPrivate;

/*!
 * \brief This is a convenience class for using a RenderingControl service.
 *
 * This is a convenience class that provides a simple asynchronous API for
 * accessing server-side RenderingControl service from the client-side.
 * The class can be instantiated with a HClientService that
 * provides the mandatory functionality of a RenderingControl.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-RenderingControl-v2-Service.pdf>
 * UPnP RenderingControl:2 specification</a>.
 *
 * \headerfile HRenderingControlAdapter.h HRenderingControlAdapter
 *
 * \ingroup hupnp_av_rcs
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HRenderingControlAdapter :
    public HClientServiceAdapter
{
Q_OBJECT
H_DISABLE_COPY(HRenderingControlAdapter)
H_DECLARE_PRIVATE(HRenderingControlAdapter)

private Q_SLOTS:

    void lastChange(
        const Herqq::Upnp::HClientStateVariable*,
        const Herqq::Upnp::HStateVariableEvent&);

protected:

    virtual bool prepareService(HClientService* service);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param instanceId specifies the ID of the virtual RenderingControl.
     *
     * \param specifies the \c QObject parent.
     */
    HRenderingControlAdapter(quint32 instanceId, QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HRenderingControlAdapter();

    /*!
     * \brief Retrieves a list of the currently defined presets.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), listPresetsCompleted()
     */
    HClientAdapterOp<QStringList> listPresets();

    /*!
     * Restores the values of the state variables associated with the
     * specified preset.
     *
     * \param presetName specifies the preset.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), selectPresetCompleted()
     */
    HClientAdapterOpNull selectPreset(const QString& presetName);

    /*!
     * \brief Retrieves the current value of the Brightness state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getBrightnessCompleted()
     */
    HClientAdapterOp<quint16> getBrightness();

    /*!
     * \brief Sets the value of the Brightness state variable.
     *
     * \param desiredBrightness specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setBrightnessCompleted()
     */
    HClientAdapterOpNull setBrightness(quint16 desiredBrightness);

    /*!
     * \brief Retrieves the current value of the Contrast state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getContrastCompleted()
     */
    HClientAdapterOp<quint16> getContrast();

    /*!
     * \brief Sets the value of the Contrast state variable.
     *
     * \param desiredContrast specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setContrastCompleted()
     */
    HClientAdapterOpNull setContrast(quint16 desiredContrast);

    /*!
     * \brief Retrieves the current value of the Sharpness state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getSharpnessCompleted()
     */
    HClientAdapterOp<quint16> getSharpness();

    /*!
     * \brief Sets the value of the Sharpness state variable.
     *
     * \param desiredSharpness specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setSharpnessCompleted()
     */
    HClientAdapterOpNull setSharpness(quint16 desiredSharpness);

    /*!
     * \brief Retrieves the current value of the RedVideoGain state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getRedVideoGainCompleted()
     */
    HClientAdapterOp<quint16> getRedVideoGain();

    /*!
     * \brief Sets the value of the RedVideoGain state variable.
     *
     * \param desiredRedVideoGain specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setRedVideoGainCompleted()
     */
    HClientAdapterOpNull setRedVideoGain(quint16 desiredRedVideoGain);

    /*!
     * \brief Retrieves the current value of the GreenVideoGain state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getGreenVideoGainCompleted()
     */
    HClientAdapterOp<quint16> getGreenVideoGain();

    /*!
     * \brief Sets the value of the GreenVideoGain state variable.
     *
     * \param desiredGreenVideoGain specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setGreenVideoGainCompleted()
     */
    HClientAdapterOpNull setGreenVideoGain(quint16 desiredGreenVideoGain);

    /*!
     * \brief Retrieves the current value of the BlueVideoGain state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getBlueVideoGainCompleted()
     */
    HClientAdapterOp<quint16> getBlueVideoGain();

    /*!
     * \brief Sets the value of the BlueVideoGain state variable.
     *
     * \param desiredBlueVideoGain specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setBlueVideoGainCompleted()
     */
    HClientAdapterOpNull setBlueVideoGain(quint16 desiredBlueVideoGain);

    /*!
     * \brief Retrieves the current value of the RedVideoBlackLevel state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getRedVideoBlackLevelCompleted()
     */
    HClientAdapterOp<quint16> getRedVideoBlackLevel();

    /*!
     * \brief Sets the value of the RedVideoBlackLevel state variable.
     *
     * \param desiredRedVideoBlackLevel specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setRedVideoBlackLevelCompleted()
     */
    HClientAdapterOpNull setRedVideoBlackLevel(quint16 desiredRedVideoBlackLevel);

    /*!
     * \brief Retrieves the current value of the GreenVideoBlackLevel state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getGreenVideoBlackLevelCompleted()
     */
    HClientAdapterOp<quint16> getGreenVideoBlackLevel();

    /*!
     * \brief Sets the value of the GreenVideoBlackLevel state variable.
     *
     * \param desiredGreenVideoBlackLevel specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setGreenVideoBlackLevelCompleted()
     */
    HClientAdapterOpNull setGreenVideoBlackLevel(quint16 desiredGreenVideoBlackLevel);

    /*!
     * \brief Retrieves the current value of the BlueVideoBlackLevel state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getBlueVideoBlackLevelCompleted()
     */
    HClientAdapterOp<quint16> getBlueVideoBlackLevel();

    /*!
     * \brief Sets the value of the BlueVideoBlackLevel state variable.
     *
     * \param desiredBlueVideoBlackLevel specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setBlueVideoBlackLevelCompleted()
     */
    HClientAdapterOpNull setBlueVideoBlackLevel(quint16 desiredBlueVideoBlackLevel);

    /*!
     * \brief Sets the value of the ColorTemperature state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getColorTemperatureCompleted()
     */
    HClientAdapterOp<quint16> getColorTemperature();

    /*!
     * \brief Sets the value of the ColorTemperature state variable.
     *
     * \param desiredColorTemperature specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setColorTemperatureCompleted()
     */
    HClientAdapterOpNull setColorTemperature(quint16 desiredColorTemperature);

    /*!
     * \brief Retrieves the current value of the HorizontalKeystone state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getHorizontalKeystoneCompleted()
     */
    HClientAdapterOp<quint16> getHorizontalKeystone();

    /*!
     * \brief Sets the value of the HorizontalKeystone state variable.
     *
     * \param desiredHorizontalKeyStone specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setHorizontalKeystoneCompleted()
     */
    HClientAdapterOpNull setHorizontalKeystone(qint16 desiredHorizontalKeyStone);

    /*!
     * \brief Retrieves the current value of the VerticalKeystone state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getVerticalKeystoneCompleted()
     */
    HClientAdapterOp<quint16> getVerticalKeystone();

    /*!
     * \brief Sets the value of the VerticalKeystone state variable.
     *
     * \param desiredVerticalKeyStone specifies the new value.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setVerticalKeystoneCompleted()
     */
    HClientAdapterOpNull setVerticalKeystone(qint16 desiredVerticalKeyStone);

   /*!
     * \brief Retrieves the value of the Mute setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getMuteCompleted()
     */
    HClientAdapterOp<bool> getMute(const HChannel& channel);

    /*!
     * \brief Sets the value of the Mute setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \param desiredMute specifies whether the channel should be muted.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setMuteCompleted()
     */
    HClientAdapterOpNull setMute(const HChannel& channel, bool desiredMute);

    /*!
     * \brief Retrieves the value of the Volume setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getVolumeCompleted()
     */
    HClientAdapterOp<quint16> getVolume(const HChannel& channel);

    /*!
     * \brief Sets the value of the Volume setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \param desiredVolume specifies the desired volume for the specified
     * channel.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setVolumeCompleted()
     */
    HClientAdapterOpNull setVolume(const HChannel& channel, quint16 desiredVolume);

    /*!
     * \brief Retrieves the value of the VolumeDB setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getVolumeDBCompleted()
     */
    HClientAdapterOp<qint16> getVolumeDB(const HChannel& channel);

    /*!
     * \brief Sets the value of the VolumeDB setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \param desiredVolumeDb specifies the desired volume for the specified
     * channel in units of 1/256 decibels.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setVolumeDBCompleted()
     */
    HClientAdapterOpNull setVolumeDB(
        const HChannel& channel, qint16 desiredVolumeDb);

    /*!
     * \brief Retrieves the valid range for the VolumeDB state variable.
     *
     * \param channel specifies the channel.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getVolumeDBRangeCompleted()
     */
    HClientAdapterOp<HVolumeDbRangeResult> getVolumeDBRange(const HChannel& channel);

    /*!
     * \brief Retrieves the value of the Loudness setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getLoudnessCompleted()
     */
    HClientAdapterOp<bool> getLoudness(const HChannel& channel);

    /*!
     * \brief Sets the value of the Loudness setting of the specified channel.
     *
     * \param channel specifies the channel.
     *
     * \param loudnessOn specifies whether loudness should be enabled on the
     * specified channel.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setLoudnessCompleted()
     */
    HClientAdapterOpNull setLoudness(const HChannel& channel, bool loudnessOn);

    /*!
     * \brief Retrieves the values of the specified state variables.
     *
     * \param stateVariableNames specifies the names of the state variables, which
     * values are to be returned. If the list contains a single item "*", values
     * of all supported state variables are returned, except \c LastChange,
     * \c PresetNameList and any \c A_ARG_TYPE_xxx variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getStateVariablesCompleted()
     */
    HClientAdapterOp<QString> getStateVariables(const QSet<QString>& stateVariableNames);

    /*!
     * \brief Sets the values of the specified state variables.
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
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setStateVariablesCompleted()
     */
    HClientAdapterOp<QStringList> setStateVariables(
        const HUdn& renderingControlUdn,
        const HResourceType& serviceType, const HServiceId& serviceId,
        const QString& stateVariableValuePairs);

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when listPresets() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * listPresets().
     *
     * The return value of the operation upon success is a \c QStringList that
     * contains the preset names.
     *
     * \sa listPresets()
     */
    void listPresetsCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QStringList>& op);

    /*!
     * \brief This signal is emitted when selectPreset() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * selectPreset().
     *
     * \sa selectPreset()
     */
    void selectPresetCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getBrightness() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getBrightness().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getBrightness()
     */
    void getBrightnessCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setBrightness() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setBrightness().
     *
     * \sa setBrightness()
     */
    void setBrightnessCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getContrast() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getContrast().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getContrast()
     */
    void getContrastCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setContrast() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setContrast().
     *
     * \sa setContrast()
     */
    void setContrastCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getSharpness() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getSharpness().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getSharpness()
     */
    void getSharpnessCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setSharpness() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setSharpness().
     *
     * \sa setSharpness()
     */
    void setSharpnessCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getRedVideoGain() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getRedVideoGain().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getRedVideoGain()
     */
    void getRedVideoGainCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setRedVideoGain() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setRedVideoGain().
     *
     * \sa setRedVideoGain()
     */
    void setRedVideoGainCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getGreenVideoGain() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getGreenVideoGain().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getGreenVideoGain()
     */
    void getGreenVideoGainCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setGreenVideoGain() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setGreenVideoGain().
     *
     * \sa setGreenVideoGain()
     */
    void setGreenVideoGainCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getBlueVideoGain() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getBlueVideoGain().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getBlueVideoGain()
     */
    void getBlueVideoGainCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setBlueVideoGain() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setBlueVideoGain().
     *
     * \sa setBlueVideoGain()
     */
    void setBlueVideoGainCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getRedVideoBlackLevel() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getRedVideoBlackLevel().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getRedVideoBlackLevel()
     */
    void getRedVideoBlackLevelCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setRedVideoBlackLevel() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setRedVideoBlackLevel().
     *
     * \sa setRedVideoBlackLevel()
     */
    void setRedVideoBlackLevelCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getGreenVideoBlackLevel() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getGreenVideoBlackLevel().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getGreenVideoBlackLevel()
     */
    void getGreenVideoBlackLevelCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setGreenVideoBlackLevel() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setGreenVideoBlackLevel().
     *
     * \sa setGreenVideoBlackLevel()
     */
    void setGreenVideoBlackLevelCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getBlueVideoBlackLevel() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getBlueVideoBlackLevel().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getBlueVideoBlackLevel()
     */
    void getBlueVideoBlackLevelCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setBlueVideoBlackLevel() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setBlueVideoBlackLevel().
     *
     * \sa setBlueVideoBlackLevel()
     */
    void setBlueVideoBlackLevelCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getColorTemperature() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getColorTemperature().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getColorTemperature()
     */
    void getColorTemperatureCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setColorTemperature() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setColorTemperature().
     *
     * \sa setColorTemperature()
     */
    void setColorTemperatureCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getHorizontalKeystone() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getHorizontalKeystone().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getHorizontalKeystone()
     */
    void getHorizontalKeystoneCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setHorizontalKeystone() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setHorizontalKeystone().
     *
     * \sa setHorizontalKeystone()
     */
    void setHorizontalKeystoneCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getVerticalKeystone() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getVerticalKeystone().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getVerticalKeystone()
     */
    void getVerticalKeystoneCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setVerticalKeystone() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setVerticalKeystone().
     *
     * \sa setVerticalKeystone()
     */
    void setVerticalKeystoneCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getMute() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getMute().
     *
     * The return value of the operation upon success is a \c bool, which indicates
     * the queried audio channel is muted.
     *
     * \sa getMute()
     */
    void getMuteCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<bool>& op);

    /*!
     * \brief This signal is emitted when getVolume() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getVolume().
     *
     * The return value of the operation upon success is a \c quint16, which will contain
     * the current value of the state variable.
     *
     * \sa getVolume()
     */
    void getVolumeCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint16>& op);

    /*!
     * \brief This signal is emitted when setMute() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setMute().
     *
     * \sa setMute()
     */
    void setMuteCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when setVolume() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setVolume().
     *
     * \sa setVolume()
     */
    void setVolumeCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getVolumeDB() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getVolumeDB().
     *
     * The return value of the operation upon success is a \c quint16, which will
     * contain the current volume of the specified channel in units of
     * 1/256 decibels.
     *
     * \sa getVolumeDB()
     */
    void getVolumeDBCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<qint16>& op);

    /*!
     * \brief This signal is emitted when setVolumeDB() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setVolumeDB().
     *
     * \sa setVolumeDB()
     */
    void setVolumeDBCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getVolumeDBRange() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getVolumeDBRange().
     *
     * \sa getVolumeDBRange()
     */
    void getVolumeDBRangeCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HVolumeDbRangeResult>& op);

    /*!
     * \brief This signal is emitted when getLoudness() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getLoudness().
     *
     * The return value of the operation upon success is a \c bool, indicates
     * if the queried channel has loudness enabled.
     *
     * \sa getLoudness()
     */
    void getLoudnessCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<bool>& op);

    /*!
     * \brief This signal is emitted when setLoudness() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setLoudness().
     *
     * \sa setLoudness()
     */
    void setLoudnessCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getStateVariables() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getStateVariables().
     *
     * The return value of the operation upon success is a \c QString,
     * which contains a XML document following
     * <a href="http://www.upnp.org/schemas/av/avs-v2.xsd">this schema</a>.
     *
     * \sa getStateVariables()
     */
    void getStateVariablesCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QString>& op);

    /*!
     * \brief This signal is emitted when setStateVariables() has completed.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setStateVariables().
     *
     * \sa setStateVariables()
     */
    void setStateVariablesCompleted(
        Herqq::Upnp::Av::HRenderingControlAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QStringList>& op);

    /*!
     * \brief This signal is emitted when a \e Last \e Change event has been
     * received from the virtual RenderingControlService instance.
     *
     * \param source specifies the HRenderingControlAdapter instance that
     * sent the event.
     *
     * \param data specifies the <em>LastChange XML Document</em> following
     * the schema available <a href="http://www.upnp.org/schemas/av/rcs-event-v1.xsd">here</a>.
     */
    void lastChangeReceived(
        Herqq::Upnp::Av::HRenderingControlAdapter* source, const QString& data);
};

}
}
}

#endif /* HRENDERINGCONTROL_ADAPTER_H_ */
