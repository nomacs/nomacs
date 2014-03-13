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

#ifndef HAVTRANSPORT_INFO_H_
#define HAVTRANSPORT_INFO_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HUpnp>

class QString;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class provides general information of a UPnP A/V Transport and
 * information related to the operations of AV Transport
 * implementations provided by HUPnPAv.
 *
 * \headerfile havtransport_info.h HAvTransportInfo
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is thread-safe.
 */
class H_UPNP_AV_EXPORT HAvTransportInfo
{
H_FORCE_SINGLETON(HAvTransportInfo)

public:

    /*!
     * \brief This enumeration specifies the available values for DRMState state variable.
     *
     * \sa drmStateToString(), drmStateFromString()
     */
    enum DrmState
    {
        /*!
         * Verification has succeeded.
         */
        DrmState_Ok,

        /*!
         * DRM is not supported.
         */
        DrmState_Unknown,

        /*!
         * The content key was received and it is being verified.
         */
        DrmState_ProcessingContentKey,

        /*!
         * Either a content key was not provided or received by the verifier,
         * or the provided key failed verification.
         */
        DrmState_ContentKeyFailure,

        /*!
         * Authentication is in progress.
         */
        DrmState_AttemptingAuthentication,

        /*!
         * Authentication process has failed.
         */
        DrmState_FailedAuthentication,

        /*!
         * Authentication is required, but it is not yet occurred.
         */
        DrmState_NotAuthenticated,

        /*!
         * A revocation condition has been detected.
         */
        DrmState_DeviceRevocation
    };

    /*!
     * \brief Returns a string representation of the specified \a value.
     *
     * \param value specifies the DrmState value.
     *
     * \return a string representation of the specified DrmState value.
     */
    static QString drmStateToString(DrmState);

    /*!
     * \brief Returns a DrmState value corresponding to the specified string, if any.
     *
     * \param arg specifies the string.
     *
     * \return The corresponding DrmState value.
     */
    static DrmState drmStateFromString(const QString&);

    /*!
     * \brief This enumeration describes the error codes that have been
     * defined by the UPnP forum to the AVTransport:2 service.
     */
    enum ErrorCode
    {
        /*!
         * The immediate transition from current transport state to desired
         * transport state is not supported by this device.
         */
        TransitionNotAvailable = 701,

        /*!
         * The media does not contain any contents that can be played.
         */
        NoContents = 702,

        /*!
         * The media cannot be read (for example, because of dust or a scratch).
         */
        ReadError = 703,

        /*!
         * The storage format of the currently loaded media is not supported.
         * for playback by this device.
         */
        FormatNotSupportedForPlayback = 704,

        /*!
         * The transport is hold locked. (Some portable mobile devices have
         * a small mechanical toggle switch called a hold lock switch. While
         * this switch is ON (he transport is hold locked) the device is
         * guarded against operations such as accidental power on when not
         * in use, or interruption of play or record from accidental pressing of
         * a front panel button or a GUI button.)
         */
        TransportIsLocked = 705,

        /*!
         * The media cannot be written. (for example, because of dust or a scratch).
         */
        WriteError = 706,

        /*!
         * The media is write-protected or is of a not writable type.
         */
        MediaIsProtectedOrNotWritable = 707,

        /*!
         * The storage format of the currently loaded media is not supported
         * for recording by this device.
         */
        FormatNotSupportedForRecording = 708,

        /*!
         * There is no free space left on the loaded media.
         */
        MediaIsFull = 709,

        /*!
         * The specified seek mode is not supported by the device.
         */
        SeekModeNotSupported = 710,

        /*!
         * The specified seek target is not present on the media or is not
         * specified in terms of the seek mode.
         */
        IllegalSeekTarget = 711,

        /*!
         * The specified play mode is not supported by the device.
         */
        PlayModeNotSupported = 712,

        /*!
         * The specified record quality is not supported by the device.
         */
        RecordQualityNotSupported = 713,

        /*!
         * The specified resource has a MIME-type which is not supported
         * by the AVTransport service.
         */
        IllegalMimeType = 714,

        /*!
         * This indicates that the resource is already in use at this time.
         */
        ContentBusy = 715,

        /*!
         * The specified resource cannot be found in the network.
         */
        ResourceNotFound = 716,

        /*!
         * The specified playback speed is not supported by the AVTransport
         * service.
         */
        PlaySpeedNotSupported = 717,

        /*!
         * The specified InstanceID is invalid for this AVTransport.
         */
        InvalidInstanceId = 718,

        /*!
         * The action failed because an unspecified DRM error occurred.
         */
        DrmError = 719,

        /*!
         * The action failed because the content use validity interval has
         * expired.
         */
        ExpiredContent = 720,

        /*!
         * The action failed because the requested content use is disallowed.
         */
        NonAllowedUse = 721,

        /*!
         * The action failed because the allowed content uses cannot be
         * verified.
         */
        CantDetermineAllowedUses = 722,

        /*!
         * The action failed because the number of times this content has
         * been used as requested has reached the maximum allowed number
         * of uses.
         */
        ExhaustedAllowedUse = 723,

        /*!
         * The action failed because of a device authentication failure
         * between the media source device and the media sink device.
         */
        DeviceAuthenticationFailure = 724,

        /*!
         * The action failed because either the media source device or the
         * media sink device has been revoked.
         */
        DeviceRevocation = 725,

        /*!
         * Some of the variables are invalid.
         */
        InvalidStateVariableList = 726,

        /*!
         * The CSV list is not well formed.
         */
        IllFormedCsvList = 727,

        /*!
         * One of the StateVariableValuePairs contains an invalid value.
         */
        InvalidStateVariableValue = 728,

        /*!
         * The specified ServiceType is invalid.
         */
        InvalidServiceType = 729,

        /*!
         * The specified ServiceId is invalid.
         */
        InvalidServiceId = 730
    };

    /*!
     * \brief Returns the type of the latest AVTransport specification the
     * HUPnPAv implementations support.
     *
     * \return The type of the latest AVTransport specification the
     * HUPnPAv implementations support.
     */
    static const HResourceType& supportedServiceType();

    /*!
     * \brief Returns information about the actions specified up until the
     * AVTransport specification supported by HUPnPAv implementations.
     *
     * \sa supportedServiceType(), stateVariablesSetupData()
     */
    static HActionsSetupData actionsSetupData();

    /*!
     * \brief Returns information about the state variables specified up until the
     * AVTransport specification supported by HUPnPAv implementations.
     *
     * \sa supportedServiceType(), actionsSetupData()
     */
    static HStateVariablesSetupData stateVariablesSetupData();
};

}
}
}

#endif /* HAVTRANSPORT_INFO_H_ */
