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

#ifndef DATAITEM_DISPLAY_H_
#define DATAITEM_DISPLAY_H_

#include "controlpoint_navigatoritem.h"

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HUpnp>

#include <QList>
#include <QPair>
#include <QtCore/QString>
#include <QAbstractTableModel>

class QVariant;
class QModelIndex;

//
//
//
class DataItemDisplay :
    public QAbstractTableModel
{
Q_OBJECT
H_DISABLE_COPY(DataItemDisplay)
friend class NavItemVisitor;

private:

    class NavItemVisitor :
        public ControlPointNavigatorItemVisitor
    {
    H_DISABLE_COPY(NavItemVisitor)
    private:

        DataItemDisplay* m_owner;

    public:

        NavItemVisitor(DataItemDisplay* owner);
        virtual ~NavItemVisitor();

        virtual void visit(ActionItem*);
        virtual void visit(ServiceItem*);
        virtual void visit(DeviceItem*);
        virtual void visit(StateVariableItem*);
    };

    QList<QPair<QString, QString> > m_modelData;

    Herqq::Upnp::HUdn m_rootDeviceUdn;

public:

    DataItemDisplay(QObject* parent = 0);
    virtual ~DataItemDisplay();

    void deviceRemoved(const Herqq::Upnp::HUdn&);

    void setData(ControlPointNavigatorItem*);

    Qt::ItemFlags flags(const QModelIndex& index) const;

    QVariant data(
        const QModelIndex& index, int role = Qt::DisplayRole) const;

    QVariant headerData(
        int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount   (const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
};

#endif // DATAITEM_DISPLAY_H_
