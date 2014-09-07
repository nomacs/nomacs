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

#ifndef HMEDIARENDERER_CONNECTION_H_
#define HMEDIARENDERER_CONNECTION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HRendererConnectionInfo>
#include <HUpnpAv/HConnectionManagerInfo>

#include <QtCore/QObject>

class QUrl;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HRendererConnectionPrivate;
class HRendererConnectionController;

/*!
 * This class defines an interface for rendering media content.
 *
 * This is a server-side class from which users derive custom classes,
 * instantiate them and provide those instances to HUPnPAv, which at appropriate
 * times uses them. The purpose of this class is to simplify the process of writing custom
 * MediaRenderers by allowing the user to focus on actual content rendering, rather
 * than to the details of AVTransport and RenderingControl specifications.
 *
 * \headerfile hrendererconnection_manager.h HRendererConnection
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HRendererConnectionManager, HAvDeviceModelCreator, HMediaRendererDeviceConfiguration
 */
class H_UPNP_AV_EXPORT HRendererConnection :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HRendererConnection)
friend class HRendererConnectionManager;

private:

    HRendererConnectionPrivate* h_ptr;

    void init(HAbstractConnectionManagerService*, HConnectionInfo*);

    void dispose();

protected:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject.
     */
    HRendererConnection(QObject* parent = 0);

    /*!
     * Returns the Connection Manager Service to which this connection belongs.
     *
     * \return the Connection Manager Service to which this connection belongs.
     */
    HAbstractConnectionManagerService* service() const;

    /*!
     * Provides the implementation for play().
     *
     * Every descendant has to override this.
     *
     * \param speed specifies the speed of the playback relative to normal speed.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doPlay(const QString& speed) = 0;

    /*!
     * Provides the implementation for stop().
     *
     * Every descendant has to override this.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doStop() = 0;

    /*!
     * Provides the implementation for pause().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doPause();

    /*!
     * Provides the implementation for record().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doRecord();

    /*!
     * Provides the implementation for seek().
     *
     * Every descendant has to override this.
     *
     * \param seekInfo specifies information about the seek operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doSeek(const HSeekInfo& seekInfo) = 0;

    /*!
     * Provides the implementation for next().
     *
     * Every descendant has to override this.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doNext() = 0;

    /*!
     * Provides the implementation for previous().
     *
     * Every descendant has to override this.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doPrevious() = 0;

    /*!
     * Provides the implementation for setPlayMode().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param newMode specifies the new play mode.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doSetPlayMode(const HPlayMode& newMode);

    /*!
     * Provides the implementation for setRecordQualityMode().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param newMode specifies the new new record quality mode.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doSetRecordQualityMode(const HRecordQualityMode& newMode);

    /*!
     * \brief Provides the implementation for setResource().
     *
     * \param resourceUri specifies the location of the new resource.
     *
     * \param cdsMetadata specifies optional metadata about the resource.
     * This is null when no metadata is available.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doSetResource(
        const QUrl& resourceUri, HObject* cdsMetadata = 0) = 0;

    /*!
     * \brief Provides the implementation for setNextResource().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param resourceUri specifies the location of the new resource.
     *
     * \param cdsMetadata specifies optional metadata about the resource.
     * This is null when no metadata is available.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 doSetNextResource(
        const QUrl& resourceUri, HObject* cdsMetadata = 0);

    /*!
     * \brief Provides the implementation for selectPreset().
     *
     * Every descendant has to override this.
     *
     * \param presetName specifies the preset.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSelectPreset(const QString& presetName) = 0;

    /*!
     * \brief Provides the implementation for setRcsValue().
     *
     * \param rcsAttrib specifies the RenderingControl property.
     *
     * \param newValue specifies the new value of the property.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSetRcsValue(
        HRendererConnectionInfo::RcsAttribute rcsAttrib, quint16 newValue);

    /*!
     * \brief Provides the implementation for setHorizontalKeystone().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param desiredHorizontalKeyStone specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSetHorizontalKeystone(qint16 desiredHorizontalKeyStone);

    /*!
     * \brief Provides the implementation for setVerticalKeystone().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param desiredVerticalKeyStone specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSetVerticalKeystone(qint16 desiredHorizontalKeyStone);

    /*!
     * \brief Provides the implementation for setMute().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param channel specifies the audio channel.
     *
     * \param muted specifies whether the channel is muted.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSetMute(const HChannel& channel, bool muted);

    /*!
     * \brief Provides the implementation for setVolume().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param channel specifies the audio channel.
     *
     * \param volume specifies the target volume.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSetVolume(const HChannel& channel, quint16 volume);

    /*!
     * \brief Provides the implementation for setVolumeDb().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param channel specifies the audio channel.
     *
     * \param volume specifies the target volume.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSetVolumeDb(const HChannel& channel, qint16 volume);

    /*!
     * \brief Provides the implementation for setLoudness().
     *
     * This method models an optional UPnP action and because of that it does not
     * have to be overridden.
     *
     * \param channel specifies the audio channel.
     *
     * \param enabled specifies whether the loudness is enabled.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    virtual qint32 doSetLoudness(const HChannel& channel, bool enabled);

    /*!
     * \brief Returns an object for getting and setting
     * information about the capabilities and runtime status of the
     * renderer connection instance.
     *
     * \return an object for getting and setting
     * information about the capabilities and runtime status of the
     * renderer connection instance.
     */
    HRendererConnectionInfo* writableRendererConnectionInfo();

    /*!
     * \brief Sets the content formats the connection supports.
     *
     * \param contentFormats specifies the content formats the connection supports.
     */
    void setContentFormat(const QString& contentFormats);

    /*!
     * Sets the additional information element of the protocol info.
     *
     * \param additionalInfo specifies the additional information element of the
     * protocol info.
     */
    void setAdditionalInfo(const QString& additionalInfo);

    /*!
     * \brief Specifies the status of the connection.
     *
     * \param arg specifies the status of the connection.
     */
    void setConnectionStatus(HConnectionManagerInfo::ConnectionStatus connectionStatus);

    /*!
     * Provides an opportunity to do post-construction initialization routines
     * in derived classes.
     */
    virtual void finalizeInit();

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HRendererConnection() = 0;

    /*!
     * \brief Returns an object for retrieving
     * information about the capabilities and runtime status of the
     * media manager instance.
     *
     * \return an object for retrieving
     * information about the capabilities and runtime status of the
     * media manager instance.
     */
    const HRendererConnectionInfo* rendererConnectionInfo() const;

    /*!
     * \brief Returns an object for getting information about the
     * connection managed by the UPnP A/V ConnectionManager.
     *
     * \return an object for getting information about the
     * connection managed by the UPnP A/V ConnectionManager.
     */
    const HConnectionInfo* connectionInfo() const;

    /*!
     * \brief Begins the playback of the current resource.
     *
     * \param speed specifies the speed of the playback relative to normal speed.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 play(const QString& speed);

    /*!
     * \brief Stops the progression of the current resource.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 stop();

    /*!
     * \brief Pauses the ongoing playback of the current resource associated with the
     * specified virtual AVTransport instance.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 pause();

    /*!
     * \brief Begins recording on the specified virtual AVTransport instance.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 record();

    /*!
     * \brief Seeks through the current resource of the specified
     * virtual AVTransport instance.
     *
     * \param seekInfo specifies information about the seek operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 seek(const HSeekInfo& seekInfo);

    /*!
     * \brief Advances the current resource of the specified
     * virtual AVTransport instance to the next track.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 next();

    /*!
     * \brief Advances the current resource of the specified
     * virtual AVTransport instance to the previous track.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 previous();

    /*!
     * \brief Sets the play mode of the specified virtual AVTransport instance.
     *
     * \param newMode specifies the new play mode.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 setPlaymode(const HPlayMode& newMode);

    /*!
     * \brief Sets the record quality mode of the specified virtual AVTransport instance.
     *
     * \param newMode specifies the new record quality mode.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 setRecordQualityMode(const HRecordQualityMode& newMode);

    /*!
     * \brief Indicates if the instance accepts the current resource to be changed to
     * the specified resource.
     *
     * \param resourceUri specifies the location of the new resource.
     *
     * \param resourceMetadata specifies CDS metadata associated with the
     * resource, using a DIDL-Lite XML Fragment. This parameter is optional.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 setResource(
        const QUrl& resourceUri, const QString& resourceMetadata = QString());

    /*!
     * \brief Indicates if the instance accepts the current resource to be changed to
     * the specified resource.
     *
     * \param resourceUri specifies the location of the new resource.
     *
     * \param cdsMetadata specifies CDS metadata associated with the
     * resource, using a DIDL-Lite XML Fragment. This parameter is optional.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    qint32 setNextResource(
        const QUrl& resourceUri, const QString& resourceMetadata = QString());

    /*!
     * \brief Restores the values of the state variables associated with the
     * specified preset.
     *
     * \param presetName specifies the preset.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 selectPreset(const QString& presetName);

    /*!
     * \brief Specifies the value for the specified RenderingControl property.
     *
     * \param rcsAttrib specifies the RenderingControl property.
     *
     * \param newValue specifies the new value of the property.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 setRcsValue(
        HRendererConnectionInfo::RcsAttribute rcsAttrib, quint16 newValue);

    /*!
     * \brief Specifies the horizontal keystone.
     *
     * \param desiredHorizontalKeyStone specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 setHorizontalKeystone(qint16 desiredHorizontalKeyStone);

    /*!
     * \brief Specifies the vertical keystone.
     *
     * \param desiredVerticalKeyStone specifies the new value.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 setVerticalKeystone(qint16 desiredHorizontalKeyStone);

    /*!
     * \brief Specifies whether the specified audio channel is muted.
     *
     * \param channel specifies the audio channel.
     *
     * \param muted specifies whether the channel is muted.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 setMute(const HChannel& channel, bool muted);

    /*!
     * \brief Specifies the volume of the specified audio channel.
     *
     * \param channel specifies the audio channel.
     *
     * \param volume specifies the target volume.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 setVolume(const HChannel& channel, quint16 volume);

    /*!
     * \brief Specifies the volume of the specified audio channel.
     *
     * \param channel specifies the audio channel.
     *
     * \param volume specifies the target volume.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 setVolumeDb(const HChannel& channel, qint16 volume);

    /*!
     * \brief Specifies whether the loudness is enabled on the specified audio channel.
     *
     * \param channel specifies the audio channel.
     *
     * \param enabled specifies whether the loudness is enabled.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HRenderingControlInfo::ErrorCode
     */
    qint32 setLoudness(const HChannel& channel, bool enabled);

    /*!
     * \brief Returns the value of the specified property (state variable).
     *
     * \param svName specifies the name of the property (state variable), which
     * value should be returned.
     *
     * \param ok is a pointer to \c bool, which value is set to \e true, if the value
     * property was found. This is optional.
     *
     * \return the value of the specified property (state variable).
     *
     * \sa setValue()
     */
    QString value(const QString& svName, bool* ok = 0) const;

    /*!
     * \brief Returns the value of the specified property (state variable).
     *
     * \param svName specifies the name of the property (state variable), which
     * value should be returned.
     *
     * \param channel specifies the audio channel of which the property value is retrieved.
     *
     * \param ok is a pointer to \c bool, which value is set to \e true, if the value
     * property was found. This is optional.
     *
     * \return the value of the specified property (state variable).
     *
     * \sa setValue()
     */
    QString value(const QString& svName, const HChannel& channel, bool* ok = 0) const;

    /*!
     * \brief Specifies the value of the specified property (state variable).
     *
     * \param svName specifies the name of the property (state variable), which
     * value should be set.
     *
     * \param value specifies the new value of the property.
     *
     * \return \e true if the value of the specified property was set.
     *
     * \sa value()
     */
    bool setValue(const QString& svName, const QString& value);

    /*!
     * \brief Specifies the value of the specified property (state variable).
     *
     * \param svName specifies the name of the property (state variable), which
     * value should be set.
     *
     * \param channel specifies the audio channel of which property should be set.
     *
     * \param value specifies the new value of the property.
     *
     * \return \e true if the value of the specified property was set.
     *
     * \sa value()
     */
    bool setValue(const QString& svName, const HChannel& channel, const QString& value);

Q_SIGNALS:

    /*!
     *
     */
    void disposed(Herqq::Upnp::Av::HRendererConnection* source);
};

}
}
}

#endif /* HMEDIARENDERER_CONNECTION_H_ */
