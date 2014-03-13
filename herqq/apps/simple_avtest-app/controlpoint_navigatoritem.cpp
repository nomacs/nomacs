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

#include "controlpoint_navigatoritem.h"

#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HStateVariableInfo>

#include <HUpnpAv/HContainer>
#include <HUpnpAv/HMediaBrowser>
#include <HUpnpAv/HConnectionInfo>
#include <HUpnpAv/HMediaRendererAdapter>
#include <HUpnpAv/HContentDirectoryAdapter>

#include <QVariant>

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

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

ControlPointNavigatorItem* ControlPointNavigatorItem::child(int row) const
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
        for(int i = 0; i < m_parentItem->m_childItems.size(); ++i)
        {
            if (m_parentItem->m_childItems.at(i) == this)
            {
                return i;
            }
        }
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
    return "Root";
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
 * RendererItem
 ******************************************************************************/
RendererItem::RendererItem(
    HMediaRendererAdapter* renderer, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_renderer(renderer)
{
    Q_ASSERT(m_renderer);
}

RendererItem::~RendererItem()
{
}

QVariant RendererItem::data(int /*column*/) const
{
    return m_renderer->device()->info().friendlyName();
}

void RendererItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}

/*******************************************************************************
 * ConnectionItem
 ******************************************************************************/
ConnectionItem::ConnectionItem(
    HConnection* connection, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_connection(connection)
{
    Q_ASSERT(m_connection);
}

ConnectionItem::~ConnectionItem()
{
}

QVariant ConnectionItem::data(int /*column*/) const
{
    if (!m_connection)
    {
        return QVariant();
    }

    return QString("Id:%1").arg(
        QString::number(m_connection->info().connectionId()));
}

void ConnectionItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}

/*******************************************************************************
 * CdsContainerItem
 ******************************************************************************/
CdsContainerItem::CdsContainerItem(
    HContainer* container,
    HCdsDataSource* dataSource,
    ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent),
            m_container(container), m_dataSource(dataSource)
{
    Q_ASSERT(m_container);
    Q_ASSERT(m_dataSource);
}

CdsContainerItem::~CdsContainerItem()
{
}

QVariant CdsContainerItem::data(int /*column*/) const
{
    return m_container->title();
}

void CdsContainerItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}

/*******************************************************************************
 * ContentDirectoryItem
 ******************************************************************************/
ContentDirectoryItem::ContentDirectoryItem(
    HMediaBrowser* browser, ControlPointNavigatorItem* parent) :
        ControlPointNavigatorItem(parent), m_browser(browser)
{
    Q_ASSERT(m_browser);
}

ContentDirectoryItem::~ContentDirectoryItem()
{
    delete m_browser;
}

QVariant ContentDirectoryItem::data(int /*column*/) const
{
    return m_browser->contentDirectory()->service()->parentDevice()->info().friendlyName();
}

void ContentDirectoryItem::accept(ControlPointNavigatorItemVisitor* visitor)
{
    visitor->visit(this);
}
