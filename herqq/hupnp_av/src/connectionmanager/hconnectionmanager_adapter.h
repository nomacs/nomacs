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

#ifndef HCONNECTIONMANAGER_H_
#define HCONNECTIONMANAGER_H_

#include <HUpnpAv/HConnectionManagerInfo>

#include <HUpnpCore/HClientServiceAdapter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HConnectionManagerAdapterPrivate;

/*!
 * \brief This is a convenience class for using a ConnectionManager service.
 *
 * This class that provides a simple asynchronous API for
 * accessing server-side ConnectionManager service from the client-side.
 * The class can be instantiated with a HClientService that
 * provides the mandatory functionality of a ConnectionManager.
 *
 * For more information,
 * see <a href=http://upnp.org/specs/av/UPnP-av-ConnectionManager-v2-Service.pdf>
 * UPnP ConnectionManager:2 specification</a>.
 *
 * \headerfile hconnectionmanager.h HConnectionManagerAdapter
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HConnectionManagerAdapter :
    public HClientServiceAdapter
{
Q_OBJECT
H_DISABLE_COPY(HConnectionManagerAdapter)
H_DECLARE_PRIVATE(HConnectionManagerAdapter)

private Q_SLOTS:

    void sourceProtocolInfoChanged_(
        const Herqq::Upnp::HClientStateVariable* source,
        const Herqq::Upnp::HStateVariableEvent& event);

    void sinkProtocolInfoChanged_(
        const Herqq::Upnp::HClientStateVariable* source,
        const Herqq::Upnp::HStateVariableEvent& event);

    void currentConnectionIDsChanged_(
        const Herqq::Upnp::HClientStateVariable* source,
        const Herqq::Upnp::HStateVariableEvent& event);

protected:

    virtual bool prepareService(HClientService* service);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the \c QObject parent.
     */
    HConnectionManagerAdapter(QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HConnectionManagerAdapter();

    /*!
     * \brief Retrieves the protocol related information this instance supports in its
     * current state.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is null, i.e. HClientAdapterOp<HProtocolInfoResult>::isNull() returns
     * \e true in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getProtocolInfoCompleted()
     */
    HClientAdapterOp<HProtocolInfoResult> getProtocolInfo();

    /*!
     * \brief Prepares the device for the purpose of sending or receiving data.
     *
     * \param remoteProtocolInfo specifies the protocol-related information
     * that \b must be used to transfer the content.
     *
     * \param peerConnectionManager specifies the ConnectionManager service on
     * the other side of the connection.
     *
     * \param peerConnectionId specifies the ID of the connection the peer
     * ConnectionManager has allocated for this connection. This can be set to
     * \c -1 if the ID is not known. This is the case if the peer does not
     * implement this action or prepareForConnection() has not been invoked on
     * that ConnectionManager yet.
     *
     * \param direction specifies the direction to which data will be sent
     * using the new connection.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), prepareForConnectionCompleted()
     */
    HClientAdapterOp<HPrepareForConnectionResult> prepareForConnection(
        const HProtocolInfo& remoteProtocolInfo,
        const HConnectionManagerId& peerConnectionManager,
        qint32 peerConnectionId,
        HConnectionManagerInfo::Direction direction);

    /*!
     * \brief Informs the device that a previously \e prepared connection is no longer
     * needed.
     *
     * \param connectionId specifies the ID of the connection.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), connectionCompleteCompleted()
     */
    HClientAdapterOpNull connectionComplete(qint32 connectionId);

    /*!
     * \brief Returns the IDs of currently active connections.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getCurrentConnectionIDsCompleted()
     */
    HClientAdapterOp<QList<quint32> > getCurrentConnectionIDs();

    /*!
     * \brief Returns information of the specified connection.
     *
     * \param connectionId specifies the connection.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getCurrentConnectionInfoCompleted()
     */
    HClientAdapterOp<HConnectionInfo> getCurrentConnectionInfo(qint32 connectionId);

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when getProtocolInfo() has completed.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getProtocolInfo().
     *
     * \sa getProtocolInfo()
     */
    void getProtocolInfoCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HProtocolInfoResult>& op);

    /*!
     * \brief This signal is emitted when prepareForConnection() has completed.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * prepareForConnection().
     *
     * \sa prepareForConnection()
     */
    void prepareForConnectionCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HPrepareForConnectionResult>& op);

    /*!
     * \brief This signal is emitted when connectionComplete() has completed.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * connectionComplete().
     *
     * \sa connectionComplete()
     */
    void connectionCompleteCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getCurrentConnectionIDs() has completed.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getCurrentConnectionIDs().
     *
     * \sa getCurrentConnectionIDs()
     */
    void getCurrentConnectionIDsCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QList<quint32> >& op);

    /*!
     * \brief This signal is emitted when getCurrentConnectionInfo() has completed.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getCurrentConnectionInfo().
     *
     * \sa getCurrentConnectionInfo()
     */
    void getCurrentConnectionInfoCompleted(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HConnectionInfo>& op);

    /*!
     * \brief This signal is emitted when the SourceProtocolInfo state variable
     * changes at the server side.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param newValue specifies the new value of the SourceProtocolInfo state
     * variable.
     *
     * \sa sinkProtocolInfoChanged()
     */
    void sourceProtocolInfoChanged(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const Herqq::Upnp::Av::HProtocolInfos& newValue);

    /*!
     * \brief This signal is emitted when the SinkProtocolInfo state variable
     * changes at the server side.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param newValue specifies the new value of the SinkProtocolInfo state
     * variable.
     *
     * \sa sourceProtocolInfoChanged()
     */
    void sinkProtocolInfoChanged(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const Herqq::Upnp::Av::HProtocolInfos& newValue);

    /*!
     * \brief This signal is emitted when the CurrentConnectionIDs state variable
     * changes at the server-side.
     *
     * \param source specifies the HConnectionManagerAdapter instance that
     * sent the event.
     *
     * \param currentIds specifies the currently active connection ID values.
     */
    void currentConnectionIdsChanged(
        Herqq::Upnp::Av::HConnectionManagerAdapter* source,
        const QList<quint32>& currentIds);
};

}
}
}

#endif /* HCONNECTIONMANAGER_H_ */
