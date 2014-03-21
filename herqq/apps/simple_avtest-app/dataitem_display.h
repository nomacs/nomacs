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

#ifndef DATAITEM_DISPLAY_H_
#define DATAITEM_DISPLAY_H_

#include "controlpoint_navigatoritem.h"

#include <HUpnpCore/HUdn>
#include <HUpnpAv/HUpnpAv>

#include <QList>
#include <QPair>
#include <QtCore/QString>
#include <QAbstractTableModel>

class QVariant;
class QModelIndex;

//
//
//
class DisplayDataRow
{
H_DISABLE_COPY(DisplayDataRow)

public:

    enum Type
    {
        Normal,
        CdsItem
    };

private:

    QStringList m_columns;
    Type m_type;

public:

    DisplayDataRow(const QStringList& columnData, Type type = Normal);
    virtual ~DisplayDataRow();

    inline QString column(qint32 index)
    {
        return m_columns.at(index);
    }

    inline Type type() const
    {
        return m_type;
    }
};

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

        virtual void visit(RendererItem*);
        virtual void visit(ConnectionItem*);
        virtual void visit(CdsContainerItem*);
        virtual void visit(ContentDirectoryItem*);
    };

    QList<DisplayDataRow*> m_modelData;
    void clearModel();

    Herqq::Upnp::HUdn m_rootDeviceUdn;
    QStringList m_columns;

public:

    DataItemDisplay(QObject* parent = 0);
    virtual ~DataItemDisplay();

    void deviceRemoved(const Herqq::Upnp::HUdn&);

    void setData(ControlPointNavigatorItem*);

    DisplayDataRow* displayDataRow(qint32 row) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    QVariant data(
        const QModelIndex& index, int role = Qt::DisplayRole) const;

    QVariant headerData(
        int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount   (const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
};

#endif // DATAITEM_DISPLAY_H_
