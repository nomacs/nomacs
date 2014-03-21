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

#include "controlpoint_navigatoritem.h"

#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HActionInfo>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HServiceInfo>
#include <HUpnpCore/HClientAction>
#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HStateVariableInfo>
#include <HUpnpCore/HClientStateVariable>

#include <QUrl>
#include <QList>
#include <QPair>
#include <QImage>
#include <QVariant>

using namespace Herqq::Upnp;

/*******************************************************************************
 * ControlPointNavigatorItem
 ******************************************************************************/
ControlPointNavigatorItem::ControlPointNavigatorItem(
    ControlPointNavigatorItem *parent) :
        m_childItems(), m_parentItem(parent)
{
}

ControlPointNavigatorItem::~ControlPointNavigatorItem()
{
    qDeleteAll(m_childItems);
}

void ControlPointNavigatorItem::appendChild(ControlPointNavigatorItem* item)
{
    Q_ASSERT(item);
    m_childItems.append(item);
}

void ControlPointNavigatorItem::removeChild(qint32 index)
{
    Q_ASSERT(index < m_childItems.size());
    delete m_childItems.at(index);
    m_childItems.removeAt(index);
}

ControlPointNavigatorItem* ControlPointNavigatorItem::child(int row)
{
    return m_childItems.value(row);
}

int ControlPointNavigatorItem::childCount() const
{
    return m_childItems.count();
}

int ControlPointNavigatorItem::columnCount() const
{
    return 1;
}

ControlPointNavigatorItem* ControlPointNavigatorItem::parent()
{
    return m_parentItem;
}

int ControlPointNavigatorItem::row() const
{
    if (m_parentItem)
    {
        return m_parentItem->m_childItems.indexOf(
            const_cast<ControlPointNavigatorItem*>(this));
    }

    return 0;
}

int ControlPointNavigatorItem::rowCount() const
{
    qint32 rowCount = childCount();
    foreach(ControlPointNavigatorItem* child, m_childItems)
    {
        rowCount += child->rowCount();
    }

    return rowCount;
}

/*******************************************************************************
 * RootItem
 ******************************************************************************/
RootItem::RootItem(ControlPointNavigatorItem* parent) :
    ControlPointNavigatorItem(parent)
{
}

RootItem::~RootItem()
{
}

QVariant RootItem::data (int /*column*/) const
{
    return "Devices";
}

void RootItem::accept(ControlPointNavigatorItemVisitor* /*visitor*/)
{
}

/*******************************************************************************
 * ContainerItem
 ******************************************************************************/
ContainerItem::ContainerItem(
    const QString& name, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_name(name)
{
}

ContainerItem::~ContainerItem()
{
}

QVariant ContainerItem::data (int /*column*/) const
{
    return m_name;
}

void ContainerItem::accept(ControlPointNavigatorItemVisitor* /*visitor*/)
{
}

/*******************************************************************************
 * DeviceItem
 ******************************************************************************/
DeviceItem::DeviceItem(
    HClientDevice* device, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_device(device)
{
    Q_ASSERT(m_device);
}

DeviceItem::~DeviceItem()
{
}

QVariant DeviceItem::data(int /*column*/) const
{
    return m_device->info().friendlyName();
}

void DeviceItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}

/*******************************************************************************
 * ServiceItem
 ******************************************************************************/
ServiceItem::ServiceItem(
    HClientService* service, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_service(service)
{
    Q_ASSERT(service);
}

ServiceItem::~ServiceItem()
{
}

QVariant ServiceItem::data (int /*column*/) const
{
    return m_service->info().serviceId().toString();
}

void ServiceItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}

/*******************************************************************************
 * ActionItem
 ******************************************************************************/
ActionItem::ActionItem(
    HClientAction* action, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_action(action)
{
    Q_ASSERT(action);
}

ActionItem::~ActionItem()
{
}

QVariant ActionItem::data (int /*column*/) const
{
    return m_action->info().name();
}

void ActionItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}

/*******************************************************************************
 * StateVariableItem
 ******************************************************************************/
StateVariableItem::StateVariableItem(
    const HClientStateVariable* stateVar, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_stateVar(stateVar)
{
    Q_ASSERT(stateVar);
}

StateVariableItem::~StateVariableItem()
{
}

QVariant StateVariableItem::data (int /*column*/) const
{
    return m_stateVar->info().name();
}

void StateVariableItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}
