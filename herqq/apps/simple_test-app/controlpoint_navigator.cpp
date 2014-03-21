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

#include "controlpoint_navigator.h"
#include "controlpoint_navigatoritem.h"

#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HClientService>

#include <QVariant>
#include <QModelIndex>

using namespace Herqq::Upnp;

ControlPointNavigator::ControlPointNavigator(QObject* parent) :
    QAbstractItemModel(parent),
        m_rootItem(new RootItem())
{
}

ControlPointNavigator::~ControlPointNavigator()
{
    delete m_rootItem;
}

ControlPointNavigatorItem* ControlPointNavigator::buildModel(
    HClientDevice* device, ControlPointNavigatorItem* parentItem)
{
    DeviceItem* deviceItem = new DeviceItem(device, parentItem);

    HClientServices services(device->services());
    for(qint32 i = 0; i < services.size(); ++i)
    {
        HClientService* service = services.at(i);
        ServiceItem* serviceItem = new ServiceItem(service, deviceItem);

        ContainerItem* stateVariablesItem =
            new ContainerItem("State Variables", serviceItem);

        HClientStateVariables stateVars = service->stateVariables();
        foreach(const QString& name, stateVars.keys())
        {
            StateVariableItem* stateVarItem =
                new StateVariableItem(stateVars.value(name), stateVariablesItem);

            stateVariablesItem->appendChild(stateVarItem);
        }

        ContainerItem* actionsItem =
            new ContainerItem("Actions", serviceItem);

        HClientActions actions = service->actions();
        foreach(const QString& name, actions.keys())
        {
            ActionItem* actionItem =
                new ActionItem(actions.value(name), actionsItem);

            actionsItem->appendChild(actionItem);
        }

        serviceItem->appendChild(stateVariablesItem);
        serviceItem->appendChild(actionsItem);

        deviceItem->appendChild(serviceItem);
    }

    foreach(HClientDevice* embeddedDevice, device->embeddedDevices())
    {
        ControlPointNavigatorItem* childItem =
            buildModel(embeddedDevice, deviceItem);

        deviceItem->appendChild(childItem);
    }

    return deviceItem;
}

void ControlPointNavigator::rootDeviceOnline(HClientDevice* newDevice)
{
    ControlPointNavigatorItem* childItem = buildModel(newDevice, m_rootItem);

    beginInsertRows(
        QModelIndex(), m_rootItem->childCount(), m_rootItem->childCount());

    m_rootItem->appendChild(childItem);

    endInsertRows();
}

void ControlPointNavigator::rootDeviceOffline(HClientDevice* device)
{
    for(qint32 i = 0; i < m_rootItem->childCount(); ++i)
    {
        DeviceItem* deviceItem = static_cast<DeviceItem*>(m_rootItem->child(i));
        if (deviceItem->device() == device)
        {
            beginRemoveRows(QModelIndex(), i, i);

            m_rootItem->removeChild(i);

            endRemoveRows();

            break;
        }
    }
}

int ControlPointNavigator::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return static_cast<ControlPointNavigatorItem*>(
            parent.internalPointer())->columnCount();
    }
    else
    {
        return m_rootItem->columnCount();
    }
}

QVariant ControlPointNavigator::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    ControlPointNavigatorItem *item =
        static_cast<ControlPointNavigatorItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags ControlPointNavigator::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ControlPointNavigator::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return m_rootItem->data(section);
    }

    return QVariant();
}

QModelIndex ControlPointNavigator::index(
    int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    ControlPointNavigatorItem* parentItem;

    if (!parent.isValid())
    {
        parentItem = m_rootItem;
    }
    else
    {
        parentItem =
            static_cast<ControlPointNavigatorItem*>(parent.internalPointer());
    }

    ControlPointNavigatorItem* childItem = parentItem->child(row);
    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex ControlPointNavigator::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    ControlPointNavigatorItem* childItem  =
        static_cast<ControlPointNavigatorItem*>(index.internalPointer());

    ControlPointNavigatorItem* parentItem = childItem->parent();

    if (parentItem == m_rootItem)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int ControlPointNavigator::rowCount(const QModelIndex& parent) const
{
    ControlPointNavigatorItem* parentItem;
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        parentItem = m_rootItem;
    }
    else
    {
        parentItem =
            static_cast<ControlPointNavigatorItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}
