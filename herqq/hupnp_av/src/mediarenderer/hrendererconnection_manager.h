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

#ifndef HRENDERERCONNECTION_MANAGER_H_
#define HRENDERERCONNECTION_MANAGER_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HConnectionInfo>

#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HRendererConnectionManagerPrivate;

/*!
 * This class is used to create HRendererConnection instances.
 *
 * \headerfile hrendererconnection_manager.h HRendererConnectionManager
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HRendererConnection
 */
class H_UPNP_AV_EXPORT HRendererConnectionManager :
    public QObject
{
Q_OBJECT

private Q_SLOTS:

    void destroyed_(QObject*);

protected:

    HRendererConnectionManagerPrivate* h_ptr;

    /*!
     * Creates a new instance.
     *
     * \param parent specifies the parent \c QObject, if any.
     */
    HRendererConnectionManager(QObject* parent = 0);

    /*!
     * Creates a new HRendererConnection instance.
     *
     * \param cmService specifies the Connection Manager to which
     * the connection is to be created.
     *
     * \param connectionInfo specifies information of the connection
     * to be created.
     *
     * \return a new HRendererConnection instance or a null pointer if no
     * instance could be created.
     *
     * \note When this function returns a valid HRendererConnection,
     * the base HRendererConnectionManager instance checks if \c QObject
     * parent has been set to the renderer connection and if not, assigns itself
     * as the parent of the new connection. However, you can assign the parent
     * yourself and you can delete the renderer connection at will.
     */
    virtual HRendererConnection* doCreate(
        HAbstractConnectionManagerService* cmService,
        HConnectionInfo* connectionInfo) = 0;

public:

    /*!
     * Destroys the instance and every HRendererConnection instance it owns.
     *
     * \note You can change the \c QObject parent of a HRendererConnection instance
     * to modify this behavior.
     */
    virtual ~HRendererConnectionManager();

    /*!
     * Creates a new HRendererConnection instance.
     *
     * \param cmService specifies the Connection Manager to which
     * the connection is to be created.
     *
     * \param connectionInfo specifies information of the connection
     * to be created.
     *
     * \return a new HRendererConnection instance or a null pointer if no
     * instance could be created.
     */
    HRendererConnection* createAndAdd(
        HAbstractConnectionManagerService* cmService,
        const HConnectionInfo& connectionInfo);

    /*!
     * Attempts to remove the specified connection from the manager instance.
     *
     * \param cmService specifies the Connection Manager which owns the connection.
     *
     * \param id specifies the connection ID to be removed.
     *
     */
    bool removeConnection(
        const HAbstractConnectionManagerService* cmService, qint32 id);

    /*!
     * Returns an HRendererConnection instance managed by this manager that
     * has the specified connectionId.
     *
     * \param cid specifies the connection ID.
     *
     * \return an HRendererConnection instance managed by this manager that
     * has the specified connectionId.
     */
    HRendererConnection* connection(
        HAbstractConnectionManagerService* cmService, qint32 cid) const;

    /*!
     * Returns the connections owned by the specified Connection Manager.
     *
     * \return the connections owned by the specified Connection Manager.
     */
    QList<HRendererConnection*> connections(HAbstractConnectionManagerService* cmService) const;

    /*!
     * Instructs the instance to "dispose" a particular renderer connection and
     * remove it from the control of the manager.
     *
     * \param connectionId specifies the connection ID.
     *
     * \return \e true if a renderer connection matching the specified ID
     * was found and removed.
     *
     * \note This does not delete the renderer connection instance.
     *
     * \sa HRendererConnection::disposed()
     */
    bool connectionComplete(
        HAbstractConnectionManagerService* cmService, qint32 connectionId);

Q_SIGNALS:

    /*!
     * This signal is emitted when a new HRendererConnection has been added
     * into the control of this instance.
     *
     * \param connectionInfo specifies information of the new HRendererConnection
     * instance.
     *
     * \sa connectionRemoved()
     */
    void connectionAdded(
        Herqq::Upnp::Av::HAbstractConnectionManagerService* cmService,
        Herqq::Upnp::Av::HConnectionInfo connectionInfo);

    /*!
     * This signal is emitted when a HRendererConnection has been removed
     * from the control of this instance.
     *
     * \param connectionId specifies the connection ID of the HRendererConnection
     * instance that was removed.
     *
     * \sa connectionAdded()
     */
    void connectionRemoved(
        Herqq::Upnp::Av::HAbstractConnectionManagerService* cmService, qint32 connectionId);

};

}
}
}

#endif /* HRENDERERCONNECTION_MANAGER_H_ */
