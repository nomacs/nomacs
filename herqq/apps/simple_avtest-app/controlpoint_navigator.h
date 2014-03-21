/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpAvSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP A/V (HUPnPAv) library.
 *
 *  HUpnpAvSimpleTestApp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HUpnpAvSimpleTestApp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HUpnpAvSimpleTestApp. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTROLPOINT_NAVIGATOR_H_
#define CONTROLPOINT_NAVIGATOR_H_

#include <HUpnpAv/HUpnpAv>

#include <HUpnpCore/HUpnp>

#include <QModelIndex>
#include <QAbstractItemModel>

class QVariant;

class CdsContainerItem;
class ControlPointNavigatorItem;

//
//
//
class ControlPointNavigator :
    public QAbstractItemModel
{
Q_OBJECT
H_DISABLE_COPY(ControlPointNavigator)

private:

    ControlPointNavigatorItem* m_rootItem;
    ControlPointNavigatorItem* m_renderersItem;
    ControlPointNavigatorItem* m_serversItem;

private:

    inline QModelIndex rootIndex()
    {
        return index(0, 0);
    }

    inline QModelIndex renderersIndex()
    {
        return index(0, 0, rootIndex());
    }

    inline QModelIndex serversIndex()
    {
        return index(1, 0, rootIndex());
    }

    CdsContainerItem* findParentContainer(CdsContainerItem* parent, const QString& id);

private Q_SLOTS:

    void objectsBrowsed(Herqq::Upnp::Av::HMediaBrowser* browser, const QSet<QString>&);
    void browseFailed(Herqq::Upnp::Av::HMediaBrowser*);

    void error(Herqq::Upnp::Av::HMediaRendererAdapter*,
        const Herqq::Upnp::HClientAdapterOp<qint32>&);

    void connectionReady(Herqq::Upnp::Av::HMediaRendererAdapter*, qint32 id);
    void connectionDestroyed(QObject*);

public:

    explicit ControlPointNavigator(QObject* parent = 0);
    virtual ~ControlPointNavigator();

    void mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter*);
    void mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter*);

    Herqq::Upnp::Av::HMediaBrowser* mediaServerOnline(Herqq::Upnp::Av::HMediaServerAdapter*);
    void mediaServerOffline(Herqq::Upnp::Av::HMediaServerAdapter*);

    virtual QVariant data      (const QModelIndex& index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual QVariant headerData(
        int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual QModelIndex index(
        int row, int column, const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex parent(const QModelIndex& index) const;

    virtual int rowCount   (const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

};

#endif /* CONTROLPOINT_NAVIGATOR_H_ */
