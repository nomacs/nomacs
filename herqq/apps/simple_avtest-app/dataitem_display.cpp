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

#include "dataitem_display.h"

#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HServiceInfo>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HClientService>

#include <HUpnpAv/HItem>
#include <HUpnpAv/HResource>
#include <HUpnpAv/HContainer>
#include <HUpnpAv/HProtocolInfo>
#include <HUpnpAv/HMediaBrowser>
#include <HUpnpAv/HCdsDataSource>
#include <HUpnpAv/HConnectionInfo>
#include <HUpnpAv/HMediaRendererAdapter>
#include <HUpnpAv/HContentDirectoryAdapter>

#include <QSet>
#include <QUrl>

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

/*******************************************************************************
 * DisplayDataRow
 *******************************************************************************/
DisplayDataRow::DisplayDataRow(const QStringList& columnData, Type type) :
    m_columns(columnData), m_type(type)
{
    Q_ASSERT(!columnData.isEmpty());
}

DisplayDataRow::~DisplayDataRow()
{
}

/*******************************************************************************
 * DataItemDisplay
 *******************************************************************************/
DataItemDisplay::NavItemVisitor::NavItemVisitor(DataItemDisplay* owner) :
    m_owner(owner)
{
    Q_ASSERT(m_owner);
}

DataItemDisplay::NavItemVisitor::~NavItemVisitor()
{
}

void DataItemDisplay::NavItemVisitor::visit(RendererItem* item)
{
    Q_ASSERT(item);

    m_owner->clearModel();

    HClientDevice* device = item->renderer()->device();
    HDeviceInfo deviceInfo = device->info();

    m_owner->m_rootDeviceUdn = device->rootDevice()->info().udn();
    m_owner->m_columns.clear();
    m_owner->m_columns.insert(0, "Name");
    m_owner->m_columns.insert(1, "Value");

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Friendly name,%1").arg(deviceInfo.friendlyName()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Device type,%1").arg(deviceInfo.deviceType().toString()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Model name,%1").arg(deviceInfo.modelName()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Manufacturer,%1").arg(deviceInfo.manufacturer()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "UDN,%1").arg(deviceInfo.udn().toString()).split(",")));

    QList<QUrl> locations = device->locations();
    for (qint32 i = 0; i < locations.size(); ++i)
    {
        m_owner->m_modelData.push_back(
            new DisplayDataRow(QString(
                "Device description URL,%1").arg(
                    locations.at(i).toString()).split(",")));
    }

    m_owner->reset();
}

void DataItemDisplay::NavItemVisitor::visit(ConnectionItem* item)
{
    Q_ASSERT(item);
    m_owner->clearModel();

    HConnection* conn = item->connection();
    HConnectionInfo info = conn->info();

    m_owner->m_columns.clear();
    m_owner->m_columns.insert(0, "Name");
    m_owner->m_columns.insert(1, "Value");

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "AV Transport ID,%1").arg(info.avTransportId()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Rendering Control ID,%1").arg(info.rcsId()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Protocol Info,%1").arg(info.protocolInfo().toString()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString("Status,%1").arg(
            HConnectionManagerInfo::statusToString(info.status())).split(",")));

    m_owner->reset();
}

void DataItemDisplay::NavItemVisitor::visit(CdsContainerItem* item)
{
    Q_ASSERT(item);

    m_owner->clearModel();

    m_owner->m_columns.clear();
    m_owner->m_columns.insert(0, "Title");
    m_owner->m_columns.insert(1, "Id");
    m_owner->m_columns.insert(2, "Class");
    m_owner->m_columns.insert(3, "Locations");

    HCdsDataSource* ds = item->dataSource();
    QSet<QString> childIds = item->container()->childIds();
    HItems items = ds->findItems(childIds);
    foreach(HItem* item, items)
    {
        HResources resources = item->resources();
        QString resAsStr;
        for(int i = 0; i < resources.size() - 1; ++i)
        {
            resAsStr.append(resources[i].location().toString()).append(";");
        }
        if (resources.size())
        {
            resAsStr.append(resources[resources.size()-1].location().toString());
        }

        m_owner->m_modelData.push_back(
            new DisplayDataRow(
                QString("%1,%2,%3,%4").arg(
                    item->title(),
                    item->id(),
                    item->clazz(),
                    resAsStr).split(","),
                DisplayDataRow::CdsItem));
    }

    m_owner->reset();
}

void DataItemDisplay::NavItemVisitor::visit(ContentDirectoryItem* item)
{
    Q_ASSERT(item);
    m_owner->clearModel();

    HClientDevice* clientDevice =
        item->browser()->contentDirectory()->service()->parentDevice();

    HDeviceInfo deviceInfo = clientDevice->info();

    m_owner->m_rootDeviceUdn = deviceInfo.udn();
    m_owner->m_columns.clear();
    m_owner->m_columns.insert(0, "Name");
    m_owner->m_columns.insert(1, "Value");

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Friendly name,%1").arg(deviceInfo.friendlyName()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Device type,%1").arg(deviceInfo.deviceType().toString()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Model name,%1").arg(deviceInfo.modelName()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "Manufacturer,%1").arg(deviceInfo.manufacturer()).split(",")));

    m_owner->m_modelData.push_back(
        new DisplayDataRow(QString(
            "UDN,%1").arg(deviceInfo.udn().toString()).split(",")));

    QList<QUrl> locations = clientDevice->locations();
    for (qint32 i = 0; i < locations.size(); ++i)
    {
        m_owner->m_modelData.push_back(
            new DisplayDataRow(QString(
                "Device description URL,%1").arg(
                    locations.at(i).toString()).split(",")));
    }

    m_owner->reset();
}

DataItemDisplay::DataItemDisplay(QObject* parent) :
    QAbstractTableModel(parent),
        m_modelData(), m_rootDeviceUdn(), m_columns()
{
}

DataItemDisplay::~DataItemDisplay()
{
    clearModel();
}

void DataItemDisplay::clearModel()
{
    qDeleteAll(m_modelData);
    m_modelData.clear();
}

void DataItemDisplay::setData(ControlPointNavigatorItem* navItem)
{
    NavItemVisitor visitor(this);
    navItem->accept(&visitor);
}

DisplayDataRow* DataItemDisplay::displayDataRow(qint32 row) const
{
    return m_modelData.at(row);
}

void DataItemDisplay::deviceRemoved(const Herqq::Upnp::HUdn& udn)
{
    if (udn == m_rootDeviceUdn)
    {
        clearModel();
        reset();
    }
}

Qt::ItemFlags DataItemDisplay::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant DataItemDisplay::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant::Invalid;
    }

    if (role == Qt::DisplayRole)
    {
        return m_modelData.at(index.row())->column(index.column());
    }

    return QVariant::Invalid;
}

QVariant DataItemDisplay::headerData (
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal && section < m_columns.size())
        {
            return m_columns.at(section);
        }
    }

    return QVariant::Invalid;
}

int DataItemDisplay::rowCount(const QModelIndex& /*parent*/) const
{
    return m_modelData.size();
}

int DataItemDisplay::columnCount(const QModelIndex& /*parent*/) const
{
    return m_columns.size();
}
