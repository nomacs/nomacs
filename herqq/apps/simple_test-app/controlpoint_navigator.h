/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP (HUPnP) library.
 *
 *  HUpnpSimpleTestApp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HUpnpSimpleTestApp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HUpnpSimpleTestApp. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTROLPOINT_NAVIGATOR_H_
#define CONTROLPOINT_NAVIGATOR_H_

#include <HUpnpCore/HUpnp>

#include <QAbstractItemModel>

class QVariant;
class QModelIndex;

class DeviceItem;
class ControlPointNavigatorItem;

//
// Primitive tree model for displaying the HControlPoint's "data model"
//
class ControlPointNavigator :
    public QAbstractItemModel
{
Q_OBJECT
H_DISABLE_COPY(ControlPointNavigator)

private:

    ControlPointNavigatorItem* m_rootItem;

    ControlPointNavigatorItem* buildModel(
        Herqq::Upnp::HClientDevice*, ControlPointNavigatorItem*);

public:

    explicit ControlPointNavigator(QObject* parent = 0);
    virtual ~ControlPointNavigator();

    void rootDeviceOnline(Herqq::Upnp::HClientDevice*);
    void rootDeviceOffline(Herqq::Upnp::HClientDevice*);

    virtual QVariant data      (const QModelIndex& index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual QVariant headerData(
            int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;

    virtual QModelIndex index(
            int row, int column,
            const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex parent(const QModelIndex& index) const;

    virtual int rowCount   (const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

};

#endif /* CONTROLPOINT_NAVIGATOR_H_ */
