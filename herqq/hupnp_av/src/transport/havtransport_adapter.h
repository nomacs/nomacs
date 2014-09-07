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

#ifndef HAVTRANSPORT_ADAPTER_H_
#define HAVTRANSPORT_ADAPTER_H_

#include <HUpnpAv/HMediaInfo>
#include <HUpnpAv/HPositionInfo>
#include <HUpnpAv/HTransportInfo>
#include <HUpnpAv/HAvTransportInfo>
#include <HUpnpAv/HTransportAction>
#include <HUpnpAv/HTransportSettings>
#include <HUpnpAv/HDeviceCapabilities>

#include <HUpnpCore/HClientAdapterOp>
#include <HUpnpCore/HClientServiceAdapter>

#include <QtCore/QSet>
#include <QtCore/QStringList>

class QUrl;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAvTransportAdapterPrivate;

/*!
 * \brief This is a convenience class for using a AVTransport service.
 *
 * This is a convenience class that provides a simple asynchronous API for
 * accessing server-side AVTransport service from the client-side.
 * The class can be instantiated with a HClientService that
 * provides the mandatory functionality of a AVTransport.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-AVTransport-v2-Service.pdf>
 * UPnP AVTransport:2 specification</a>.
 *
 * \headerfile HAvTransportAdapter.h HAvTransportAdapter
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAvTransportAdapter :
    public HClientServiceAdapter
{
Q_OBJECT
H_DISABLE_COPY(HAvTransportAdapter)
H_DECLARE_PRIVATE(HAvTransportAdapter)

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
     * \param instanceId specifies the ID of the virtual AVTransport.
     *
     * \param parent specifies the \c QObject parent.
     *
     * \sa isReady()
     */
    HAvTransportAdapter(quint32 instanceId, QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAvTransportAdapter();

    /*!
     * Moves an CDS object within the service to a different location in the
     * CDS object hierarchy.
     *
     * \brief Sets the URI of the resource to be controlled by the specified
     * AVTransport instance.
     *
     * \param currentUri specifies the URI of the resource to be controlled.
     *
     * \param currentUriMetadata specifies CDS metadata associated with the
     * resource, using a DIDL-Lite XML Fragment. This parameter is optional.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setAVTransportURICompleted()
     */
    HClientAdapterOpNull setAVTransportURI(
        const QUrl& currentUri, const QString& currentUriMetaData);

    /*!
     * \brief Sets the URI of the next resource to be controlled by the specified
     * AVTransport instance after the playback of the current resource has
     * finished.
     *
     * \param nextUri specifies the URI of the resource to be controlled.
     *
     * \param nextUriMetadata specifies CDS metadata associated with the
     * resource, using a DIDL-Lite XML Fragment. This parameter is optional.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setNextAVTransportURICompleted()
     */
    HClientAdapterOpNull setNextAVTransportURI(
        const QUrl& nextUri, const QString& nextUriMetaData);

    /*!
     * \brief Retrieves information associated with the current media.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getMediaInfo_extCompleted()
     */
    HClientAdapterOp<HMediaInfo> getMediaInfo();

    /*!
     * \brief Retrieves information associated with the current media.
     *
     * The only difference between this action and getMediaInfo() is that this
     * action may also return a value for HMediaInfo::currentType().
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getMediaInfo_extCompleted()
     */
    HClientAdapterOp<HMediaInfo> getMediaInfo_ext();

    /*!
     * \brief Retrieves information associated with the current transport state.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getTransportSettingsCompleted()
     */
    HClientAdapterOp<HTransportInfo> getTransportInfo();

     /*!
     * \brief Retrieves information associated with the current position.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getPositionInfoCompleted()
     */
    HClientAdapterOp<HPositionInfo> getPositionInfo();

    /*!
     * \brief Retrieves information of the device capabilities.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getDeviceCapabilitiesCompleted()
     */
    HClientAdapterOp<HDeviceCapabilities> getDeviceCapabilities();

    /*!
     * \brief Retrieves information of various settings, such as the play mode.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getTransportSettingsCompleted()
     */
    HClientAdapterOp<HTransportSettings> getTransportSettings();

    /*!
     * Stops the progression of the current resource.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), stopCompleted()
     */
    HClientAdapterOpNull stop();

    /*!
     * Begins the playback of the current resource.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), playCompleted()
     */
    HClientAdapterOpNull play(const QString&);

    /*!
     * Pauses the ongoing playback of the current resource.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), pauseCompleted()
     */
    HClientAdapterOpNull pause();

    /*!
     * Begins recording on the specified virtual AVTransport instance.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), recordCompleted()
     */
    HClientAdapterOpNull record();

    /*!
     * Seeks through the current resource.
     *
     * \param arg specifies the necessary information to run the operation.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), seekCompleted()
     */
    HClientAdapterOpNull seek(const HSeekInfo& arg);

    /*!
     * Advances the current resource to the next track.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), nextCompleted()
     */
    HClientAdapterOpNull next();

    /*!
     * Advances the current resource to the previous track.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), previousCompleted()
     */
    HClientAdapterOpNull previous();

    /*!
     * \brief Sets the play mode.
     *
     * \param playMode specifies the play mode.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setPlayModeCompleted()
     */
    HClientAdapterOpNull setPlayMode(const HPlayMode& playMode);

    /*!
     * \brief Sets the record quality mode.
     *
     * \param rqMode specifies the record quality mode.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), setRecordQualityModeCompleted()
     */
    HClientAdapterOpNull setRecordQualityMode(const HRecordQualityMode& rqMode);

    /*!
     * \brief Returns the value of the \c CurrentTransportActions state variable.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getCurrentTransportActionsCompleted()
     */
    HClientAdapterOp<QSet<HTransportAction> > getCurrentTransportActions();

    /*!
     * \brief Retrieves information of the current DRM state.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getDrmStateCompleted()
     */
    HClientAdapterOp<HAvTransportInfo::DrmState> getDrmState();

    /*!
     * \brief Retrieves names and values of the specified state variables.
     *
     * \param stateVariableNames specifies the names of the state variables, which
     * values are to be returned. If the set contains a single item "*", values
     * of all supported state variables are returned, except \c LastChange
     * and any \c A_ARG_TYPE_xxx variable.
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
     * \param instanceId specifies the virtual AV Transport instance.
     *
     * \param avtUdn specifies the UPnP device that owns the
     * target AV Transport service.
     *
     * \param serviceType specifies the service type of the target AV Transport.
     *
     * \param serviceId specifies the serviceId of the target AV Transport.
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
        const HUdn& avtUdn, const HResourceType& serviceType,
        const HServiceId& serviceId, const QString& stateVariableValuePairs);

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when setAVTransportURI() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setAVTransportURI().
     *
     * \sa setAVTransportURI()
     */
    void setAVTransportURICompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when setNextAVTransportURI() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setNextAVTransportURI().
     *
     * \sa setNextAVTransportURI()
     */
    void setNextAVTransportURICompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getMediaInfo() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getMediaInfo().
     *
     * \sa getMediaInfo()
     */
    void getMediaInfoCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HMediaInfo>& op);

    /*!
     * \brief This signal is emitted when getMediaInfo_ext() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getMediaInfo_ext().
     *
     * \sa getMediaInfo_ext()
     */
    void getMediaInfo_extCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HMediaInfo>& op);

    /*!
     * \brief This signal is emitted when getTransportInfo() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getTransportInfo().
     *
     * \sa getTransportInfo()
     */
    void getTransportInfoCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HTransportInfo>& op);

    /*!
     * \brief This signal is emitted when getPositionInfo() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getPositionInfo().
     *
     * \sa getPositionInfo()
     */
    void getPositionInfoCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HPositionInfo>& op);

    /*!
     * \brief This signal is emitted when getDeviceCapabilities() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getDeviceCapabilities().
     *
     * \sa getDeviceCapabilities()
     */
    void getDeviceCapabilitiesCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HDeviceCapabilities>& op);

    /*!
     * \brief This signal is emitted when getTransportSettings() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getTransportSettings().
     *
     * \sa getTransportSettings()
     */
    void getTransportSettingsCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HTransportSettings>& op);

    /*!
     * \brief This signal is emitted when stop() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * stop().
     *
     * \sa stop()
     */
    void stopCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when play() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * play().
     *
     * \sa play()
     */
    void playCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when pause() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * pause().
     *
     * \sa pause()
     */
    void pauseCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when record() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * record().
     *
     * \sa record()
     */
    void recordCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when seek() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * seek().
     *
     * \sa seek()
     */
    void seekCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when next() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * next().
     *
     * \sa next()
     */
    void nextCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when previous() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * previous().
     *
     * \sa previous()
     */
    void previousCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when setPlayMode() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setPlayMode().
     *
     * \sa setPlayMode()
     */
    void setPlayModeCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when setRecordQualityMode() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setRecordQualityMode().
     *
     * \sa setRecordQualityMode()
     */
    void setRecordQualityModeCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getCurrentTransportActions() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getCurrentTransportActions().
     *
     * \sa getCurrentTransportActions()
     */
    void getCurrentTransportActionsCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QSet<Herqq::Upnp::Av::HTransportAction> >& op);

    /*!
     * \brief This signal is emitted when getDrmState() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getDrmState().
     *
     * \sa getDrmState()
     */
    void getDrmStateCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HAvTransportInfo::DrmState>& op);

    /*!
     * \brief This signal is emitted when getStateVariables() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
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
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QString>& op);

    /*!
     * \brief This signal is emitted when setStateVariables() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * setStateVariables().
     *
     * \sa setStateVariables()
     */
    void setStateVariablesCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QStringList>& op);

    /*!
     * \brief This signal is emitted when a \e Last \e Change event has been
     * received from the virtual AvTransportService instance.
     *
     * \param source specifies the HAvTransportAdapter instance that
     * sent the event.
     *
     * \param data specifies the <em>LastChange XML Document</em> following
     * the schema available <a href="http://www.upnp.org/schemas/av/avt-event-v2.xsd">here</a>.
     */
    void lastChangeReceived(
        Herqq::Upnp::Av::HAvTransportAdapter* source, const QString& data);
};

}
}
}

#endif /* HAVTRANSPORT_ADAPTER_H_ */
