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

#include "controlpoint_navigator.h"
#include "controlpoint_navigatoritem.h"

#include <HUpnpAv/HContainer>
#include <HUpnpAv/HProtocolInfo>
#include <HUpnpAv/HMediaBrowser>
#include <HUpnpAv/HCdsDataSource>
#include <HUpnpAv/HMediaServerAdapter>
#include <HUpnpAv/HMediaRendererAdapter>
#include <HUpnpAv/HContentDirectoryAdapter>

#include <HUpnpCore/HClientService>

#include <QSet>
#include <QVariant>
#include <QEventLoop>

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

ControlPointNavigator::ControlPointNavigator(QObject* parent) :
    QAbstractItemModel(parent),
        m_rootItem(new RootItem()), m_renderersItem(), m_serversItem()
{
    m_renderersItem = new ContainerItem("Media Renderers", m_rootItem);
    m_rootItem->appendChild(m_renderersItem);

    m_serversItem = new ContainerItem("Media Servers", m_rootItem);
    m_rootItem->appendChild(m_serversItem);
}

ControlPointNavigator::~ControlPointNavigator()
{
    delete m_rootItem;
}

void ControlPointNavigator::mediaRendererOnline(HMediaRendererAdapter* mediaRenderer)
{
    HClientAdapterOp<qint32> op = mediaRenderer->getCurrentConnections();
    if (op.isNull())
    {
        return;
    }

    bool ok = connect(
        mediaRenderer,
        SIGNAL(connectionReady(Herqq::Upnp::Av::HMediaRendererAdapter*, qint32)),
        this,
        SLOT(connectionReady(Herqq::Upnp::Av::HMediaRendererAdapter*, qint32)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        mediaRenderer,
        SIGNAL(error(Herqq::Upnp::Av::HMediaRendererAdapter*,
                     Herqq::Upnp::HClientAdapterOp<qint32>)),
        this,
        SLOT(error(Herqq::Upnp::Av::HMediaRendererAdapter*,
                   Herqq::Upnp::HClientAdapterOp<qint32>)));
    Q_ASSERT(ok);

    RendererItem* rendererItem = new RendererItem(mediaRenderer, m_renderersItem);

    beginInsertRows(
        QModelIndex(), m_rootItem->childCount(), m_rootItem->childCount());

    m_renderersItem->appendChild(rendererItem);

    endInsertRows();
}

void ControlPointNavigator::error(Herqq::Upnp::Av::HMediaRendererAdapter*,
    const Herqq::Upnp::HClientAdapterOp<qint32>& op)
{
    Q_UNUSED(op)
}

void ControlPointNavigator::connectionReady(HMediaRendererAdapter* source, qint32 id)
{
    RendererItem* parentItem = 0;
    for(qint32 i = 0; i < m_renderersItem->childCount(); ++i)
    {
        RendererItem* deviceItem =
            static_cast<RendererItem*>(m_renderersItem->child(i));

        if (deviceItem->renderer() == source)
        {
            parentItem = deviceItem;
            break;
        }
    }

    if (parentItem)
    {
        HConnection* connection = source->connection(id);
        ConnectionItem* connectionItem = new ConnectionItem(connection, parentItem);

        bool ok = connect(
            connection, SIGNAL(destroyed(QObject*)),
            this, SLOT(connectionDestroyed(QObject*)));
        Q_ASSERT(ok); Q_UNUSED(ok)

        beginInsertRows(
            QModelIndex(), m_rootItem->childCount(), m_rootItem->childCount());

        parentItem->appendChild(connectionItem);

        endInsertRows();
    }
}

void ControlPointNavigator::connectionDestroyed(QObject* conn)
{
    for(qint32 i = 0; i < m_renderersItem->childCount(); ++i)
    {
        RendererItem* rendererItem =
            static_cast<RendererItem*>(m_renderersItem->child(i));

        for (int j = 0; j < rendererItem->childCount(); ++j)
        {
            ConnectionItem* conItem =
                static_cast<ConnectionItem*>(rendererItem->child(j));

            if (!conItem->connection() || conItem->connection() == conn)
            {
                beginRemoveRows(index(j, 0, renderersIndex()), j, j);
                rendererItem->removeChild(j);
                endRemoveRows();
                return;
            }
        }
    }
}

void ControlPointNavigator::mediaRendererOffline(HMediaRendererAdapter* device)
{
    for(qint32 i = 0; i < m_renderersItem->childCount(); ++i)
    {
        RendererItem* rendererItem =
            static_cast<RendererItem*>(m_renderersItem->child(i));

        if (rendererItem->renderer() == device)
        {
            beginRemoveRows(renderersIndex(), i, i);
            m_renderersItem->removeChild(i);
            endRemoveRows();
            break;
        }
    }
}

HMediaBrowser* ControlPointNavigator::mediaServerOnline(HMediaServerAdapter* device)
{
    HMediaBrowser* browser = new HMediaBrowser(this);
    if (!browser->reset(device->contentDirectory(), false))
    {
        return 0;
    }

    bool ok = connect(
        browser,
        SIGNAL(objectsBrowsed(Herqq::Upnp::Av::HMediaBrowser*,QSet<QString>)),
        this,
        SLOT(objectsBrowsed(Herqq::Upnp::Av::HMediaBrowser*,QSet<QString>)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        browser,
        SIGNAL(browseFailed(Herqq::Upnp::Av::HMediaBrowser*)),
        this,
        SLOT(browseFailed(Herqq::Upnp::Av::HMediaBrowser*)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    if (!browser->browseRoot())
    {
        delete browser; browser = 0;
    }
    else
    {
        ContentDirectoryItem* cdItem =
            new ContentDirectoryItem(browser, m_serversItem);

        beginInsertRows(QModelIndex(), m_rootItem->childCount(), m_rootItem->childCount());
        m_serversItem->appendChild(cdItem);
        endInsertRows();
    }

    return browser;
}

CdsContainerItem* ControlPointNavigator::findParentContainer(
    CdsContainerItem* parent, const QString& id)
{
    for (int i = 0; i < parent->childCount(); ++i)
    {
        CdsContainerItem* item = static_cast<CdsContainerItem*>(parent->child(i));
        if (item->container()->id() == id)
        {
            return item;
        }
        else
        {
            item = findParentContainer(item, id);
            if (item)
            {
                return item;
            }
        }
    }
    return 0;
}

namespace
{
bool hasContainerItem(const CdsContainerItem* item, const HContainer* container)
{
    for(int i = 0; i < item->childCount(); ++i)
    {
        const CdsContainerItem* cdsContainerItem =
            dynamic_cast<const CdsContainerItem*>(item->child(i));

        if (cdsContainerItem && cdsContainerItem->container() == container)
        {
            return true;
        }
        else if (hasContainerItem(cdsContainerItem, container))
        {
            return true;
        }
    }
    return false;
}
}

void ControlPointNavigator::objectsBrowsed(
    HMediaBrowser* browser, const QSet<QString>& ids)
{
    HCdsDataSource* dataSource = browser->dataSource();

    ContentDirectoryItem* cdsItem = 0;
    for(int i = 0; i < m_serversItem->childCount(); ++i)
    {
        cdsItem = static_cast<ContentDirectoryItem*>(m_serversItem->child(i));
        if (cdsItem->browser() == browser)
        {
            break;
        }
    }
    Q_ASSERT(cdsItem);

    foreach(const QString& id, ids)
    {
        HContainer* container = dataSource->findContainer(id);
        if (!container)
        {
            continue;
        }

        if (id != "0")
        {
            CdsContainerItem* rootItem =
                static_cast<CdsContainerItem*>(cdsItem->child(0));

            if (!rootItem || hasContainerItem(rootItem, container))
            {
                continue;
            }

            if ("0" == container->parentId())
            {
                CdsContainerItem* newContainerItem =
                    new CdsContainerItem(container, dataSource, rootItem);

                rootItem->appendChild(newContainerItem);
            }
            else
            {
                CdsContainerItem* item =
                    findParentContainer(rootItem, container->parentId());

                if (item)
                {
                    CdsContainerItem* newContainerItem =
                        new CdsContainerItem(container, dataSource, item);

                    item->appendChild(newContainerItem);
                }
            }
        }
        else
        {
            CdsContainerItem* newContainerItem =
                new CdsContainerItem(container, dataSource, cdsItem);

            cdsItem->appendChild(newContainerItem);
        }
    }

    beginInsertRows(QModelIndex(), m_rootItem->childCount(), m_rootItem->childCount());
    endInsertRows();
}

void ControlPointNavigator::browseFailed(HMediaBrowser*)
{
}

void ControlPointNavigator::mediaServerOffline(HMediaServerAdapter* device)
{
    for(qint32 i = 0; i < m_serversItem->childCount(); ++i)
    {
        ContentDirectoryItem* cdItem =
            static_cast<ContentDirectoryItem*>(m_serversItem->child(i));

        if (cdItem->browser()->contentDirectory()->service()->parentDevice() == device->device())
        {
            beginRemoveRows(serversIndex(), i, i);
            m_serversItem->removeChild(i);
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

    ControlPointNavigatorItem* item =
        static_cast<ControlPointNavigatorItem*>(index.internalPointer());

    Q_ASSERT(item);

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

    ControlPointNavigatorItem* parentItem = 0;
    if (!parent.isValid())
    {
        parentItem = m_rootItem;
    }
    else
    {
        parentItem =
            static_cast<ControlPointNavigatorItem*>(parent.internalPointer());
    }
    Q_ASSERT(parentItem);

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

QModelIndex ControlPointNavigator::parent(const QModelIndex& itemIndex) const
{
    if (!itemIndex.isValid() || !itemIndex.internalPointer())
    {
        return QModelIndex();
    }

    ControlPointNavigatorItem* childItem  =
        static_cast<ControlPointNavigatorItem*>(itemIndex.internalPointer());

    ControlPointNavigatorItem* parentItem = childItem->parent();
    Q_ASSERT(parentItem);

    if (parentItem == m_rootItem)
    {
        return QModelIndex();
    }

    return createIndex(0, 0, parentItem);
}

int ControlPointNavigator::rowCount(const QModelIndex& parent) const
{
    ControlPointNavigatorItem* parentItem = 0;

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

    Q_ASSERT(parentItem);

    return parentItem->childCount();
}
