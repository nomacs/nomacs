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

#ifndef HRENDERERCONNECTION_INFO_H_
#define HRENDERERCONNECTION_INFO_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HMediaInfo>
#include <HUpnpAv/HAvTransportInfo>

#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HRendererConnectionEventInfoPrivate;

/*!
 * \brief This class is used to contain information of HRendererConnectionInfo events.
 *
 * \headerfile hrendererconnection_info.h HRendererConnectionEventInfo
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HRendererConnectionInfo
 */
class H_UPNP_AV_EXPORT HRendererConnectionEventInfo
{
private:

    QSharedDataPointer<HRendererConnectionEventInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HRendererConnectionEventInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param propName specifies the name of a property, which value has changed.
     *
     * \param newValue specifies the new value of the property.
     *
     * \sa isValid()
     */
    HRendererConnectionEventInfo(const QString& propName, const QString& newValue);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HRendererConnectionEventInfo(const HRendererConnectionEventInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HRendererConnectionEventInfo& operator=(const HRendererConnectionEventInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HRendererConnectionEventInfo();

    /*!
     * Indicates if the instance is valid.
     *
     * \return \e true if the instance is valid, i.e. propertyName() is defined
     * and the oldValue() and newValue() are different.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of the property that has changed.
     *
     * \return the name of the property that has changed.
     *
     * \sa setPropertyName()
     */
    QString propertyName() const;

    /*!
     * \brief Returns the value of the property before the change.
     *
     * \return the value of the property before the change.
     *
     * \sa setOldValue()
     */
    QString oldValue() const;

    /*!
     * \brief Returns the new value of the property.
     *
     * \return the new value of the property.
     *
     * \sa setNewValue()
     */
    QString newValue() const;

    /*!
     * \brief Returns the associated audio channel of the property value, if any.
     *
     * \return the associated audio channel of the property value, if any.
     *
     * \sa setChannel()
     */
    HChannel channel() const;

    /*!
     * \brief Specifies the name of the property that has changed.
     *
     * \param arg specifies the name of the property that has changed.
     *
     * \sa propertyName()
     */
    void setPropertyName(const QString& arg);

    /*!
     * \brief Specifies the value of the property before the change.
     *
     * \param arg specifies the value of the property before the change.
     *
     * \sa oldValue()
     */
    void setOldValue(const QString& arg);

    /*!
     * \brief Specifies the new value of the property.
     *
     * \param arg specifies the new value of the property.
     *
     * \sa newValue()
     */
    void setNewValue(const QString& arg);

    /*!
     * \brief Specifies the associated audio channel of the property value.
     *
     * \param arg specifies the associated audio channel of the property value.
     *
     * \sa channel()
     */
    void setChannel(const HChannel& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HRendererConnectionEventInfo
 */
H_UPNP_AV_EXPORT bool operator==(
    const HRendererConnectionEventInfo&, const HRendererConnectionEventInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HRendererConnectionEventInfo
 */
inline bool operator!=(
    const HRendererConnectionEventInfo& obj1, const HRendererConnectionEventInfo& obj2)
{
    return !(obj1 == obj2);
}

class HRendererConnectionInfoPrivate;

/*!
 * \brief This is a class used to contain capability and state information
 * of an HRendererConnection instance.
 *
 * \headerfile hrendererconnection_info.h HRendererConnectionInfo
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HRendererConnection
 */
class H_UPNP_AV_EXPORT HRendererConnectionInfo :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HRendererConnectionInfo)

protected:

    HRendererConnectionInfoPrivate* h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \param owner specifies the owner of this instance.
     *
     * \sa isValid()
     */
    HRendererConnectionInfo(HRendererConnection* owner);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HRendererConnectionInfo();

    /*!
     * Returns the media renderer connection that owns this instance.
     *
     * \return the media renderer connection that owns this instance.
     */
    HRendererConnection* connection() const;

    // AVT related

    /*!
     * \brief Returns the Transport State of the current resource.
     *
     * \return The Transport State of the current resource.
     *
     * \sa setTransportState()
     */
    HTransportState transportState() const;

    /*!
     * \brief Returns the Transport Status of the current resource.
     *
     * \return The Transport Status of the current resource.
     *
     * \sa setTransportStatus()
     */
    HTransportStatus transportStatus() const;

    /*!
     * \brief Indicates whether the current resource is track-aware.
     *
     * \return a value indicating whether the current resource is track-aware.
     *
     * \sa setCurrentMediaCategory()
     */
    HMediaInfo::MediaCategory currentMediaCategory() const;

    /*!
     * \brief Indicates the storage medium at which the current resource is stored.
     *
     * \return The storage medium at which the current resource is stored.
     *
     * \sa setPlaybackStorageMedium()
     */
    HStorageMedium playbackStorageMedium() const;

    /*!
     * \brief Indicates the storage medium where the current resource will be stored
     * when recording.
     *
     * \return The storage medium where the current resource will be recorded
     * if the recording is started.
     *
     * \sa setRecordStorageMedium()
     */
    HStorageMedium recordStorageMedium() const;

    /*!
     * \brief Returns the supported medias for playback.
     *
     * \return The supported medias for playback.
     *
     * \sa setPossiblePlaybackStorageMedia()
     */
    QSet<HStorageMedium> possiblePlaybackStorageMedia() const;

    /*!
     * \brief Returns the supported medias for recording.
     *
     * \return The supported medias for recording.
     *
     * \sa setPossibleRecordStorageMedia()
     */
    QSet<HStorageMedium> possibleRecordStorageMedia() const;

    /*!
     * \brief Returns the current play mode.
     *
     * \return The current play mode.
     *
     * \sa setCurrentPlayMode()
     */
    HPlayMode currentPlayMode() const;

    /*!
     * \brief Returns the transport play speed.
     *
     * \return a rational fraction indicating the speed relative to normal speed.
     *
     * \sa setTransportPlaySpeed()
     */
    QString transportPlaySpeed() const;

    /*!
     * \brief Returns the write protection status of the current resource.
     *
     * \return The write protection status of the current resource.
     *
     * \sa setRecordMediumWriteStatus()
     */
    HRecordMediumWriteStatus recordMediumWriteStatus() const;

    /*!
     * \brief Returns the current record quality mode.
     *
     * \return The current record quality mode.
     *
     * \sa setCurrentRecordQualityMode()
     */
    HRecordQualityMode currentRecordQualityMode() const;

    /*!
     * \brief Returns the possible recording quality modes the device supports.
     *
     * \return The possible recording quality modes the device supports.
     *
     * \sa setPossibleRecordQualityModes()
     */
    QSet<HRecordQualityMode> possibleRecordQualityModes() const;

    /*!
     * \brief Returns the number of tracks the current resource contains.
     *
     * \return The number of tracks the current resource contains.
     *
     * \sa setNumberOfTracks()
     */
    quint32 numberOfTracks() const;

    /*!
     * \brief Returns the current track number.
     *
     * \return The current track number.
     *
     * \sa setCurrentTrack()
     */
    quint32 currentTrack() const;

    /*!
     * \brief Returns the duration of the current track.
     *
     * \return The duration of the current track.
     *
     * \sa setCurrentTrackDuration()
     */
    HDuration currentTrackDuration() const;

    /*!
     * \brief Returns the duration of the current resource.
     *
     * \return The duration of the current resource.
     *
     * \sa setCurrentMediaDuration()
     */
    HDuration currentMediaDuration() const;

    /*!
     * \brief Returns the metadata of the current track.
     *
     * \return a DIDL-Lite XML fragment containing
     * the metadata of the current track. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa setCurrentTrackMetadata()
     */
    QString currentTrackMetadata() const;

    /*!
     * \brief Returns a reference to the current track as a URI.
     *
     * \return a reference to the current track as a URI.
     *
     * \sa setCurrentTrackUri()
     */
    QUrl currentTrackUri() const;

    /*!
     * \brief Returns the location of the current resource.
     *
     * \return The location of the current resource.
     *
     * \sa setCurrentResourceUri()
     */
    QUrl currentResourceUri() const;

    /*!
     * \brief Returns the metadata associated with the current resource.
     *
     * \return a DIDL-Lite XML fragment containing
     * metadata of the current resource. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa setCurrentResourceMetadata()
     */
    QString currentResourceMetadata() const;

    /*!
     * \brief Returns the location of the next resource to be played
     * once the playing of the current resource finishes.
     *
     * \return The location of the next resource to be played
     * once the playing of the current resource finishes.
     *
     * \sa setNextResourceUri()
     */
    QUrl nextResourceUri() const;

    /*!
     * \brief Returns the metadata associated with the next resource to be played.
     *
     * \return a DIDL-Lite XML fragment containing
     * metadata of the next resource to be played. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa setNextResourceMetadata()
     */
    QString nextResourceMetadata() const;

    /*!
     * \brief Returns the current position in the current track, in terms of
     * time, measured from the beginning of the current \b track.
     *
     * \return The current position in the current track, in terms of
     * time, measured from the beginning of the current \b track. For track-aware media,
     * this is measured from the beginning of the track
     * and the value is always positive. For track-unaware media, this is
     * measured from a zero reference point on the media and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.23.
     *
     * \sa setRelativeTimePosition()
     */
    HDuration relativeTimePosition() const;

    /*!
     * \brief Returns the current position, in terms of time, measured from the
     * beginning of the current \b resource.
     *
     * \return The absolute position in terms of time,
     * measured from the beginning of the current \b resource.
     * The range of this is from "00:00:00" to the duration of the current
     * resource. The value is always positive.
     *
     * \sa setAbsoluteTimePosition()
     */
    HDuration absoluteTimePosition() const;

    /*!
     * \brief Returns the current position in the current track, in terms of a
     * dimensionless counter, measured from the beginning of the current \b track.
     *
     * \return The current position in the current track, in terms of a
     * dimensionless counter, measured from the beginning of the current \b track.
     * For track-aware media, this is measured from the
     * beginning of the track and the value is always positive.
     * For track-unaware media, this is measured from a zero reference point
     * on the current resource and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.25.
     *
     * \sa setRelativeCounterPosition()
     */
    qint32 relativeCounterPosition() const;

    /*!
     * \brief Returns the current position in terms of dimensionless counter, measured
     * from the beginning of the current \b resource.
     *
     * \return The absolute position in terms of a
     * dimensionless counter, measured from the beginning of the current
     * \b resource. This value is always positive.
     *
     * \sa setAbsoluteCounterPosition()
     */
    qint32 absoluteCounterPosition() const;

    /*!
     * \brief Returns the transport actions that can be successfully invoked for the
     * current resource.
     *
     * \return The transport actions that can be successfully invoked for the
     * current resource.
     *
     * \sa setCurrentTransportActions()
     */
    QSet<HTransportAction> currentTransportActions() const;

    /*!
     * \brief Returns the current state of DRM-controlled content.
     *
     * \return The current state of DRM-controlled content.
     *
     * \sa setDrmState()
     */
    HAvTransportInfo::DrmState drmState() const;

    /*!
     * \brief Returns media-related information of the current resource.
     *
     * This is a convenience method for retrieving all the
     * media-related information of the current resource.
     *
     * \return media-related information of the current resource.
     *
     * \sa setMediaInfo()
     */
    HMediaInfo mediaInfo() const;

    /*!
     * \brief Returns runtime status information.
     *
     * This is a convenience method for retrieving information that describe
     * the status of the \e current \e operation targeting the current resource.
     *
     * \return runtime status information.
     *
     * \sa setTransportInfo()
     */
    HTransportInfo transportInfo() const;

    /*!
     * \brief Returns position information of the current operation targeting
     * the current resource.
     *
     * This is a convenience method for retrieving position information.
     *
     * \return position information of the current operation targeting
     * the current resource.
     *
     * \sa setPositionInfo()
     */
    HPositionInfo positionInfo() const;

    /*!
     * \brief Returns the capabilities of this media manager.
     *
     * This is a convenience method for retrieving all the capability information.
     *
     * \return The capabilities of this media manager.
     *
     * \sa setDeviceCapabilities()
     */
    HDeviceCapabilities deviceCapabilities() const;

    /*!
     * \brief Returns information on various settings.
     *
     * This is a convenience method for retrieving information on various
     * transport settings.
     *
     * \return information on various settings.
     *
     * \sa setTransportSettings()
     */
    HTransportSettings transportSettings() const;

    // RCS Related

    /*!
     * \brief Returns the currently defined presets.
     *
     * \return The currently defined presets.
     *
     * \sa setPresets()
     */
    QSet<QString> presets() const;

    /*!
     * \brief This enumeration defines the RenderingControl properties, which values
     * are representable with the range of unsigned short integer (16 bytes).
     */
    enum RcsAttribute
    {
        /*!
         * The brightness setting of an associated display device.
         */
        Brightness,

        /*!
         * The contrast setting of an associated display device.
         */
        Contrast,

        /*!
         * The sharpness setting of an associated display device.
         */
        Sharpness,

        /*!
         * The red gain control setting of an associated display device.
         */
        RedVideoGain,

        /*!
         * The green gain control setting of an associated display device.
         */
        GreenVideoGain,

        /*!
         * The blue gain control setting of an associated display device.
         */
        BlueVideoGain,

        /*!
         * The setting of minimum output intensity of red of an associated display device.
         */
        RedVideoBlackLevel,

        /*!
         * The setting of minimum output intensity of green of an associated display device.
         */
        GreenVideoBlackLevel,

        /*!
         * The setting of minimum output intensity of blue of an associated display device.
         */
        BlueVideoBlackLevel,

        /*!
         * The setting of color quality of white of an associated display device.
         */
        ColorTemperature
    };

    /*!
     * \brief Returns a string representation of the specified RcsAttribute value.
     *
     * \param arg specifies the RcsAttribute value to be converted.
     *
     * \return a string representation of the specified RcsAttribute value.
     */
    static QString rcsAttributeToString(RcsAttribute arg);

    /*!
     * \brief Returns the current value of the specified setting.
     *
     * \param attr specifies setting.
     *
     * \return the current value of the specified setting.
     *
     * \sa setRcsValue()
     */
    quint16 rcsValue(RcsAttribute attr) const;

    /*!
     * \brief Returns the current value of horizontal keystone setting.
     *
     * \return the current value of horizontal keystone setting.
     *
     * \sa setHorizontalKeystone()
     */
    qint16 horizontalKeystone() const;

    /*!
     * \brief Returns the current value of vertical keystone setting.
     *
     * \return the current value of vertical keystone setting.
     *
     * \sa setVerticalKeystone()
     */
    qint16 verticalKeystone() const;

    /*!
     * \brief Indicates if the specified audio channel is currently muted.
     *
     * \param channel specifies the audio channel.
     *
     * \param ok specifies a pointer to \c bool, which is set to \e true if the
     * specified \a channel exists.
     *
     * \return \e true if the specified audio channel is currently muted.
     *
     * \sa setMute()
     */
    bool muted(const HChannel& channel, bool* ok = 0) const;

    /*!
     * \brief Returns the current volume setting of the specified audio channel.
     *
     * \param channel specifies the audio channel.
     *
     * \param ok specifies a pointer to \c bool, which is set to \e true if the
     * specified \a channel exists.
     *
     * \return the current volume setting of the specified audio channel.
     *
     * \sa setVolume()
     */
    quint16 volume(const HChannel& channel, bool* ok = 0) const;

    /*!
     * \brief Returns the current volume setting in decibels of the specified audio channel.
     *
     * \param channel specifies the audio channel.
     *
     * \param ok specifies a pointer to \c bool, which is set to \e true if the
     * specified \a channel exists.
     *
     * \return the current volume setting in decibels of the specified audio channel.
     *
     * \sa setVolumeDb()
     */
    qint16 volumeDb(const HChannel& channel, bool* ok = 0) const;

    /*!
     * \brief Returns the VolumeDB range of the specified audio channel.
     *
     * \param specifies the audio channel.
     *
     * \param ok specifies a pointer to \c bool, which is set to \e true if the
     * specified \a channel exists.
     *
     * \return the VolumeDB range of the specified audio channel.
     */
    HVolumeDbRangeResult volumeDbRange(const HChannel& channel, bool* ok = 0) const;

    /*!
     * \brief Indicates if the specified audio channel has loudness currently enabled.
     *
     * \param channel specifies the audio channel.
     *
     * \param ok specifies a pointer to \c bool, which is set to \e true if the
     * specified \a channel exists.
     *
     * \return \e true if the specified audio channel has loudness currently enabled
     *
     * \sa setLoudness()
     */
    bool loudnessEnabled(const HChannel& channel, bool* ok = 0) const;

    // AVT related

    /*!
     * \brief Specifies the Transport State of the current resource.
     *
     * \param arg specifies the Transport State of the current resource.
     *
     * \sa transportState()
     */
    void setTransportState(const HTransportState& arg);

    /*!
     * \brief Specifies the Transport Status of the current resource.
     *
     * \param arg specifies the Transport Status of the current resource.
     *
     * \sa transportStatus()
     */
    void setTransportStatus(const HTransportStatus& arg);

    /*!
     * \brief Specifies whether the current resource is track-aware.
     *
     * \param arg specifies whether the current resource is track-aware.
     *
     * \sa currentMediaCategory()
     */
    void setCurrentMediaCategory(HMediaInfo::MediaCategory arg);

    /*!
     * \brief Specifies the storage medium at which the current resource is stored.
     *
     * \param arg specifies the storage medium at which the current resource is stored.
     *
     * \sa playbackStorageMedium()
     */
    void setPlaybackStorageMedium(const HStorageMedium& arg);

    /*!
     * \brief Specifies the storage medium where the current resource will be stored
     * when recording.
     *
     * \param arg specifies the storage medium where the current resource will be recorded
     * if the recording is started.
     *
     * \sa recordStorageMedium()
     */
    void setRecordStorageMedium(const HStorageMedium& arg);

    /*!
     * \brief Specifies the supported medias for playback.
     *
     * \param arg specifies the supported medias for playback.
     *
     * \sa possiblePlaybackStorageMedia()
     */
    void setPossiblePlaybackStorageMedia(const QSet<HStorageMedium>& arg);

    /*!
     * \brief Specifies the supported medias for recording.
     *
     * \param arg specifies the supported medias for recording.
     *
     * \sa possibleRecordStorageMedia()
     */
    void setPossibleRecordStorageMedia(const QSet<HStorageMedium>& arg);

    /*!
     * \brief Specifies the current play mode.
     *
     * \param arg specifies the current play mode.
     *
     * \sa currentPlayMode()
     */
    void setCurrentPlayMode(const HPlayMode& arg);

    /*!
     * \brief Specifies the transport play speed.
     *
     * \param arg specifies a rational fraction indicating the speed relative to normal speed.
     *
     * \sa transportPlaySpeed()
     */
    void setTransportPlaySpeed(const QString& arg);

    /*!
     * \brief Specifies the write protection status of the current resource.
     *
     * \param arg specifies the write protection status of the current resource.
     *
     * \sa recordMediumWriteStatus()
     */
    void setRecordMediumWriteStatus(const HRecordMediumWriteStatus& arg);

    /*!
     * \brief Specifies the current record quality mode.
     *
     * \param arg specifeis the current record quality mode.
     *
     * \sa currentRecordQualityMode()
     */
    void setCurrentRecordQualityMode(const HRecordQualityMode& arg);

    /*!
     * \brief Specifies the possible recording quality modes the device supports.
     *
     * \param arg specifies the possible recording quality modes the device supports.
     *
     * \sa possibleRecordQualityModes()
     */
    void setPossibleRecordQualityModes(const QSet<HRecordQualityMode>& arg);

    /*!
     * \brief Specifies the number of tracks the current resource contains.
     *
     * \param arg specifies the number of tracks the current resource contains.
     *
     * \sa numberOfTracks()
     */
    void setNumberOfTracks(quint32 arg);

    /*!
     * \brief Specifies the current track number.
     *
     * \param arg specifies the current track number.
     *
     * \sa currentTrack()
     */
    void setCurrentTrack(quint32 arg);

    /*!
     * \brief Specifies the duration of the current track.
     *
     * \param arg specifies the duration of the current track.
     *
     * \sa currentTrackDuration()
     */
    void setCurrentTrackDuration(const HDuration& arg);

    /*!
     * \brief Specifies the duration of the current resource.
     *
     * \param arg specifies the duration of the current resource.
     *
     * \sa currentMediaDuration()
     */
    void setCurrentMediaDuration(const HDuration& arg);

    /*!
     * \brief Specifies the metadata of the current track.
     *
     * \param arg specifies a DIDL-Lite XML fragment containing
     * the metadata of the current track. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa currentTrackMetadata()
     */
    void setCurrentTrackMetadata(const QString& arg);

    /*!
     * \brief Specifies a reference to the current track as a URI.
     *
     * \param arg specifies a reference to the current track as a URI.
     *
     * \sa currentTrackUri()
     */
    void setCurrentTrackUri(const QUrl& arg);

    /*!
     * \brief Specifies the location of the current resource.
     *
     * \param arg specifies the location of the current resource.
     *
     * \sa currentResourceUri()
     */
    void setCurrentResourceUri(const QUrl& arg);

    /*!
     * \brief Specifies the metadata associated with the current resource.
     *
     * \param arg specifies a DIDL-Lite XML fragment containing
     * metadata of the current resource. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa currentResourceMetadata()
     */
    void setCurrentResourceMetadata(const QString& arg);

    /*!
     * \brief Specifies the location of the next resource to be played
     * once the playing of the current resource finishes.
     *
     * \param arg specifies the location of the next resource to be played
     * once the playing of the current resource finishes.
     *
     * \sa nextResourceUri()
     */
    void setNextResourceUri(const QUrl& arg);

    /*!
     * \brief Specifies the metadata associated with the next resource to be played.
     *
     * \param arg a DIDL-Lite XML fragment containing
     * metadata of the next resource to be played. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa nextResourceMetadata()
     */
    void setNextResourceMetadata(const QString& arg);

    /*!
     * \brief Specifies the current position in the current track, in terms of
     * time, measured from the beginning of the current \b track.
     *
     * \param arg specifies the current position in the current track, in terms of
     * time, measured from the beginning of the current \b track.
     *
     * \sa relativeTimePosition()
     */
    void setRelativeTimePosition(const HDuration& arg);

    /*!
     * \brief Specifies the current position, in terms of time, measured from the
     * beginning of the current \b resource.
     *
     * \param arg specifies the absolute position in terms of time,
     * measured from the beginning of the current \b resource.
     *
     * \sa absoluteTimePosition()
     */
    void setAbsoluteTimePosition(const HDuration& arg);

    /*!
     * \brief Specifies the current position in the current track, in terms of a
     * dimensionless counter, measured from the beginning of the current \b track.
     *
     * \param arg specifies the current position in the current track, in terms of a
     * dimensionless counter, measured from the beginning of the current \b track.
     *
     * \sa relativeCounterPosition()
     */
    void setRelativeCounterPosition(qint32 arg);

    /*!
     * \brief Specifies the current position in terms of dimensionless counter, measured
     * from the beginning of the current \b resource.
     *
     * \param arg specifeis the absolute position in terms of a
     * dimensionless counter, measured from the beginning of the current
     * \b resource. This value is always positive.
     *
     * \sa absoluteCounterPosition()
     */
    void setAbsoluteCounterPosition(qint32 arg);

    /*!
     * \brief Specifies the transport actions that can be successfully invoked for the
     * current resource.
     *
     * \param arg specifies the transport actions that can be successfully invoked for the
     * current resource.
     *
     * \sa currentTransportActions()
     */
    void setCurrentTransportActions(const QSet<HTransportAction>& arg);

    /*!
     * \brief Specifies the current state of DRM-controlled content.
     *
     * \param arg specifies the current state of DRM-controlled content.
     *
     * \sa drmState()
     */
    void setDrmState(HAvTransportInfo::DrmState arg);

    /*!
     * \brief Specifies media-related information of the current resource.
     *
     * This is a convenience method for setting all the
     * media-related information of the current resource.
     *
     * \param arg specifies media-related information of the current resource.
     *
     * \sa mediaInfo()
     */
    void setMediaInfo(const HMediaInfo& arg);

    /*!
     * \brief Specifies runtime status information.
     *
     * This is a convenience method for setting information that describe
     * the status of the \e current \e operation targeting the current resource.
     *
     * \param arg specifies runtime status information.
     *
     * \sa transportInfo()
     */
    void setTransportInfo(const HTransportInfo& arg);

    /*!
     * \brief Specifies position information of the current operation targeting
     * the current resource.
     *
     * This is a convenience method for setting position information.
     *
     * \param arg specifies position information of the current operation targeting
     * the current resource.
     *
     * \sa positionInfo()
     */
    void setPositionInfo(const HPositionInfo& arg);

    /*!
     * \brief Specifies the capabilities of this media manager.
     *
     * This is a convenience method for setting all the capability information.
     *
     * \param arg specifies the capabilities of this media manager.
     *
     * \sa deviceCapabilities()
     */
    void setDeviceCapabilities(const HDeviceCapabilities& arg);

    /*!
     * \brief Specifies information on various settings.
     *
     * This is a convenience method for setting information on various
     * transport settings.
     *
     * \param arg specifies information on various settings.
     *
     * \sa transportSettings()
     */
    void setTransportSettings(const HTransportSettings& arg);

    /*!
     * \brief Specifies the currently defined presets.
     *
     * \param arg specifies the currently defined presets.
     *
     * \sa presets()
     */
    void setPresets(const QSet<QString>& presets);

    // RCS related

    /*!
     * \brief Specifies a new value value for the specified setting.
     *
     * \param attr specifies setting.
     *
     * \param value specifies the new value for the specified setting.
     *
     * \sa rcsValue()
     */
    void setRcsValue(RcsAttribute attr, quint16 value);

    /*!
     * \brief Specifies a new value for the horizontal keystone setting.
     *
     * \param value specifies a new value for the horizontal keystone setting.
     *
     * \sa horizontalKeystone()
     */
    void setHorizontalKeystone(qint16 value);

    /*!
     * \brief Specifies a new value for the horizontal keystone setting.
     *
     * \param value specifies a new value for the vertical keystone setting.
     *
     * \sa verticalKeystone()
     */
    void setVerticalKeystone(qint16 value);

    /*!
     * \brief Specifies whether the specified audio channel should be muted.
     *
     * \param channel specifies the audio channel.
     *
     * \param enabled specifies whether the specified audio channel should be muted.
     *
     * \return \e true if the specified audio channel was found.
     *
     * \sa muted()
     */
    bool setMute(const HChannel& channel, bool enabled);

    /*!
     * \brief Specifies a new volume setting for the specified audio channel.
     *
     * \param channel specifies the audio channel.
     *
     * \param value specifies a new volume setting for the specified audio channel.
     *
     * \return \e true if the specified audio channel was found.
     *
     * \sa volume()
     */
    bool setVolume(const HChannel& channel, quint16 value);

    /*!
     * \brief Specifies a new volume setting in decibels for the specified audio channel.
     *
     * \param channel specifies the audio channel.
     *
     * \param value specifies a new volume setting in decibels for the specified audio channel.
     *
     * \return \e true if the specified audio channel was found.
     *
     * \sa volumeDb()
     */
    bool setVolumeDb(const HChannel& channel, qint16 value);

    /*!
     * \brief Specifies whether the specified audio channel should have loudness enabled.
     *
     * \param channel specifies the audio channel.
     *
     * \param enabled specifies whether the specified audio channel should have loudness enabled.
     *
     * \return \e true if the specified audio channel was found.
     *
     * \sa loudnessEnabled()
     */
    bool setLoudness(const HChannel& channel, bool enabled);

    /*!
     * \brief Returns the value of the specified property.
     *
     * \param svName specifies the name of the property, which
     * value should be returned.
     *
     * \param ok is a pointer to \c bool, which value is set to \e true, if the value
     * property was found. This is optional.
     *
     * \return the value of the specified property.
     *
     * \sa setValue()
     */
    QString value(const QString& svName, bool* ok = 0) const;

    /*!
     * \brief Returns the value of the specified property.
     *
     * \param svName specifies the name of the property, which
     * value should be returned.
     *
     * \param channel specifies the audio channel of which the property value is retrieved.
     *
     * \param ok is a pointer to \c bool, which value is set to \e true, if the value
     * property was found. This is optional.
     *
     * \return the value of the specified property.
     *
     * \sa setValue()
     */
    QString value(const QString& svName, const HChannel& channel, bool* ok = 0) const;

    /*!
     * \brief Specifies a new value for the specified property.
     *
     * \param svName specifies the name of the property, which
     * value should be set.
     *
     * \param value specifies the new value for the property.
     *
     * \return \e true if the value of the specified property was set.
     *
     * \sa value()
     */
    bool setValue(const QString& svName, const QString& value);

    /*!
     * \brief Specifies a new value for the specified property.
     *
     * \param svName specifies the name of the property, which
     * value should be set.
     *
     * \param channel specifies the audio channel of which property should be set.
     *
     * \param value specifies a new value for the property.
     *
     * \return \e true if the value of the specified property was set.
     *
     * \sa value()
     */
    bool setValue(const QString& svName, const HChannel& channel, const QString& value);

    /*!
     * Indicates if the specified state variable is associated with an
     * audio channel.
     *
     * \param svName specifies the name of the state variable.
     *
     * \return \e true if the specified state variable is associated with an
     * audio channel.
     */
    static bool hasChannelAssociated(const QString& svName);

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when a value of a property has changed.
     *
     * \param source specifies the source of the event.
     *
     * \param info specifies information of the event that occurred.
     */
    void propertyChanged(
        Herqq::Upnp::Av::HRendererConnectionInfo* source,
        const Herqq::Upnp::Av::HRendererConnectionEventInfo& info);
};

}
}
}

#endif /* HRENDERERCONNECTION_INFO_H_ */
