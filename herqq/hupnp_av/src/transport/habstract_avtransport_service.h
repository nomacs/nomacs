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

#ifndef HABSTRACT_AVTRANSPORT_SERVICE_H_
#define HABSTRACT_AVTRANSPORT_SERVICE_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HAvTransportInfo>

#include <HUpnpCore/HServerService>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAbstractTransportServicePrivate;

/*!
 * This is an abstract base class for implementing the standardized UPnP service type
 * AVTransport:2.
 *
 * \brief This class marshals the action invocations through the HServerAction interface to
 * virtual methods. It is an ideal choice for a base class when you wish to implement
 * the AVTransport service in full.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-AVTransport-v2-Service.pdf>
 * UPnP AVTransport:2 specification</a>.
 *
 * \headerfile habstracttransport_service.h HAbstractTransportService
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAbstractTransportService :
    public HServerService
{
Q_OBJECT
H_DISABLE_COPY(HAbstractTransportService)
H_DECLARE_PRIVATE(HAbstractTransportService)

protected:

    /*!
     * \brief Creates a new instance.
     */
    HAbstractTransportService();

    //
    // \internal
    //
    HAbstractTransportService(HAbstractTransportServicePrivate& dd);

    // Documented in HServerService
    virtual HActionInvokes createActionInvokes();

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAbstractTransportService() = 0;

    /*!
     * \brief Sets the URI of the resource to be controlled by the specified
     * AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param currentUri specifies the URI of the resource to be controlled.
     *
     * \param currentUriMetadata specifies CDS metadata associated with the
     * resource, using a DIDL-Lite XML Fragment. This parameter is optional.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 setAVTransportURI(
        quint32 instanceId, const QUrl& currentUri,
        const QString& currentUriMetadata = QString()) = 0;

    /*!
     * \brief Sets the URI of the next resource to be controlled by the specified
     * AVTransport instance after the playback of the current resource has
     * finished.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param nextUri specifies the URI of the resource to be controlled.
     *
     * \param nextUriMetadata specifies CDS metadata associated with the
     * resource, using a DIDL-Lite XML Fragment. This parameter is optional.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setNextAVTransportURI(
        quint32 instanceId, const QUrl& nextUri,
        const QString& nextUriMetadata = QString());

    /*!
     * \brief Retrieves information associated with the current media of the specified
     * virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to HMediaInfo, which will contain the
     * result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 getMediaInfo(quint32 instanceId, HMediaInfo* outArg) = 0;

    /*!
     * \brief Retrieves information associated with the current media of the specified
     * virtual AVTransport instance.
     *
     * The only difference between this action and getMediaInfo() is that this
     * action may also return a value for HMediaInfo::currentType().
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to HMediaInfo, which will contain the
     * result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 getMediaInfo_ext(quint32 instanceId, HMediaInfo* outArg) = 0;

    /*!
     * \brief Retrieves information associated with the current transport state
     * of the specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to HTransportInfo, which will contain the
     * result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 getTransportInfo(quint32 instanceId, HTransportInfo* outArg) = 0;

    /*!
     * \brief Retrieves information associated with the current position of the
     * transport of the specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to HPositionInfo, which will contain the
     * result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 getPositionInfo(quint32 instanceId, HPositionInfo* outArg) = 0;

    /*!
     * \brief Retrieves information of the device capabilities of the
     * specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to HDeviceCapabilities, which
     * will contain the result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 getDeviceCapabilities(
        quint32 instanceId, HDeviceCapabilities* outArg) = 0;

    /*!
     * \brief Retrieves information of various settings, such as the play mode of the
     * specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to HTransportSettings, which
     * will contain the result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 getTransportSettings(
        quint32 instanceId, HTransportSettings*) = 0;

    /*!
     * \brief Stops the progression of the current resource associated with the
     * specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 stop(quint32 instanceId) = 0;

    /*!
     * \brief Begins the playback of the current resource associated with the
     * specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param speed specifies the speed of the playback.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 play(quint32 instanceId, const QString& speed) = 0;

    /*!
     * \brief Pauses the ongoing playback of the current resource associated with the
     * specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 pause(quint32 instanceId);

   /*!
     * \brief Begins recording on the specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 record(quint32 instanceId);

    /*!
     * \brief Seeks through the current resource of the specified
     * virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param seekInfo specifies information about the seek operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 seek(quint32 instanceId, const HSeekInfo& seekInfo) = 0;

    /*!
     * \brief Advances the current resource of the specified
     * virtual AVTransport instance to the next track.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 next(quint32 instanceId) = 0;

    /*!
     * \brief Advances the current resource of the specified
     * virtual AVTransport instance to the previous track.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     */
    virtual qint32 previous(quint32 instanceId) = 0;

    /*!
     * \brief Sets the play mode of the specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param playMode specifies the new play mode.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setPlayMode(quint32 instanceId, const HPlayMode& playMode) = 0;

    /*!
     * \brief Sets the record quality mode of the specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param rqMode specifies the new record quality mode.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setRecordQualityMode(
        quint32 instanceId, const HRecordQualityMode& rqMode) = 0;

    /*!
     * \brief Returns the value of the \c CurrentTransportActions state variable of the
     * specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to a list of HTransportAction instances,
     * which will contain the result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getCurrentTransportActions(
        quint32 instanceId, QSet<HTransportAction>* outArg);

    /*!
     * \brief Retrieves information of the current DRM state of the
     * specified virtual AVTransport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param outArg specifies a pointer to HAvTransportInfo::DrmState,
     * which will contain the result of the operation when the operation is successful.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getDrmState(
        quint32 instanceId, HAvTransportInfo::DrmState* outArg);

    /*!
     * \brief Returns the values of the specified state variables of the specified
     * virtual AV Transport instance.
     *
     * \param instanceId specifies the virtual AVTransport instance.
     *
     * \param stateVariableNames specifies the names of the state variables, which
     * values are to be returned. If the set contains a single item "*", values
     * of all supported state variables are returned, except \c LastChange
     * and any \c A_ARG_TYPE_xxx variable.
     *
     * \param stateVariableValuePairs specifies a pointer to a \c QString,
     * which will contain the result. The result is a XML document following
     * <a href="http://www.upnp.org/schemas/av/avs-v2.xsd">this schema</a>.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 getStateVariables(
        quint32 instanceId, const QSet<QString>& stateVariableNames,
        QString* stateVariableValuePairs);

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
     * \param stateVariableList specifies a pointer to a \c QStringList,
     * which contains the names of all the state variables that were successfully
     * modified.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HAvTransportInfo::ErrorCode
     *
     * \remarks This action is optional.
     */
    virtual qint32 setStateVariables(
        quint32 instanceId, const HUdn& avtUdn,
        const HResourceType& serviceType, const HServiceId& serviceId,
        const QString& stateVariableValuePairs, QStringList* stateVariableList);
};

}
}
}

#endif /* HABSTRACT_AVTRANSPORT_SERVICE_H_ */
