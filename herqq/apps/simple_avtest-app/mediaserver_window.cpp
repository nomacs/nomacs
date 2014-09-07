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

#include "mediaserver_window.h"
#include "ui_mediaserver_window.h"

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HDeviceHostConfiguration>

#include <HUpnpAv/HItem>
#include <HUpnpAv/HRootDir>
#include <HUpnpAv/HContainer>
#include <HUpnpAv/HAvDeviceModelCreator>
#include <HUpnpAv/HMediaServerDeviceConfiguration>

#include <HUpnpAv/HFileSystemDataSource>
#include <HUpnpAv/HContentDirectoryServiceConfiguration>

#include <QDir>
#include <QFileDialog>

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

/*******************************************************************************
 * MediaServerWindow
 *******************************************************************************/
MediaServerWindow::MediaServerWindow(QWidget *parent) :
    QMainWindow(parent),
        m_ui(new Ui::MediaServerWindow), m_deviceHost(0),
        m_datasource(0)
{
    m_ui->setupUi(this);

    // 1) Configure a data source
    HFileSystemDataSourceConfiguration datasourceConfig;
    // Here you could configure the data source in more detail if needed. For example,
    // you could add "root directories" to the configuration and the data source
    // would scan those directories for media content upon initialization.
    m_datasource = new HFileSystemDataSource(datasourceConfig);

    // 2) Configure ContentDirectoryService by providing it access to the desired data source.
    HContentDirectoryServiceConfiguration cdsConfig;
    cdsConfig.setDataSource(m_datasource, false);

    // 3) Configure MediaServer by giving it the ContentDirectoryService configuration.
    HMediaServerDeviceConfiguration mediaServerConfig;
    mediaServerConfig.setContentDirectoryConfiguration(cdsConfig);

    // 4) Setup the "Device Model Cretor" that HUPnP will use to create
    // appropriate UPnP A/V device and service instances. Here you provide the
    // MediaServer configuration HUPnP will pass to the MediaServer device instance.
    HAvDeviceModelCreator creator;
    creator.setMediaServerConfiguration(mediaServerConfig);

    // 5) Setup the HDeviceHost with desired configuration info.
    HDeviceConfiguration config;
    config.setPathToDeviceDescription("./descriptions/herqq_mediaserver_description.xml");
    config.setCacheControlMaxAge(180);

    HDeviceHostConfiguration hostConfiguration;
    hostConfiguration.setDeviceModelCreator(creator);
    hostConfiguration.add(config);

    // 6) Initialize the HDeviceHost.
    m_deviceHost = new HDeviceHost(this);
    if (!m_deviceHost->init(hostConfiguration))
    {
        Q_ASSERT_X(false, "", m_deviceHost->errorDescription().toLocal8Bit());
    }
}

MediaServerWindow::~MediaServerWindow()
{
    delete m_ui;
    delete m_datasource;
}

void MediaServerWindow::changeEvent(QEvent* e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MediaServerWindow::closeEvent(QCloseEvent*)
{
    emit closed();
}

void MediaServerWindow::on_addContentButton_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Open Directory");
    if (!dirName.isEmpty())
    {
        HRootDir::ScanMode smode =
            m_ui->scanRecursivelyCheckbox->checkState() == Qt::Checked ?
                HRootDir::RecursiveScan : HRootDir::SingleDirectoryScan;

        HRootDir rd(dirName, smode);
        if (m_datasource->add(rd) >= 0)
        {
            int rc = m_ui->sharedItemsTable->rowCount();
            m_ui->sharedItemsTable->insertRow(rc);

            QTableWidgetItem* newItemScanType =
                new QTableWidgetItem(
                    smode == HRootDir::RecursiveScan ? "Yes" : "No");

            newItemScanType->setFlags(Qt::ItemIsEnabled);

            m_ui->sharedItemsTable->setItem(rc, 0, newItemScanType);

            QTableWidgetItem* newItemWatchType = new QTableWidgetItem("No");
            newItemWatchType->setFlags(Qt::ItemIsEnabled);

            m_ui->sharedItemsTable->setItem(rc, 1, newItemWatchType);

            QTableWidgetItem* newItem =
                new QTableWidgetItem(rd.dir().absolutePath());

            newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            m_ui->sharedItemsTable->setItem(rc, 2, newItem);
        }
    }
}

void MediaServerWindow::addContenDlgtFinished()
{
    m_ui->addContentButton->setEnabled(true);
}

void MediaServerWindow::on_addItemButton_clicked()
{
    QStringList fullPaths = QFileDialog::getOpenFileNames(this, "Open File(s)");
    if (!fullPaths.isEmpty())
    {
        foreach(const QString& fullPath, fullPaths)
        {
            QStringList parts = fullPath.split(QDir::separator(), QString::SkipEmptyParts);

            if (parts.isEmpty())
            {
                m_datasource->add(fullPath, "0");
            }
            else
            {
                QString lastParentContainerId = "0";
                for(int i = 0; i < parts.count() - 1; ++i)
                {
                    HContainer* container =
                        m_datasource->findContainerWithTitle(parts[i]);

                    if (!container)
                    {
                        container = new HContainer(parts[i], lastParentContainerId);
                        m_datasource->add(container);
                    }

                    lastParentContainerId = container->id();
                }

                m_datasource->add(parts.last(), lastParentContainerId);
            }

            int rc = m_ui->sharedItemsTable->rowCount();
            m_ui->sharedItemsTable->insertRow(rc);

            QTableWidgetItem* newItemScanType = new QTableWidgetItem("No");

            newItemScanType->setFlags(Qt::ItemIsEnabled);

            m_ui->sharedItemsTable->setItem(rc, 0, newItemScanType);

            QTableWidgetItem* newItemWatchType = new QTableWidgetItem("No");
            newItemWatchType->setFlags(Qt::ItemIsEnabled);

            m_ui->sharedItemsTable->setItem(rc, 1, newItemWatchType);

            QTableWidgetItem* newItem = new QTableWidgetItem(fullPath);

            newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            m_ui->sharedItemsTable->setItem(rc, 2, newItem);
        }
    }
}
