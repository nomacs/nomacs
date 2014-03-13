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

#include "controlpoint_window.h"
#include "ui_controlpoint.h"

#include "dataitem_display.h"
#include "mediamanager_dialog.h"
#include "controlpoint_navigator.h"
#include "controlpoint_navigatoritem.h"

#include <HUpnpCore/HDeviceInfo>

#include <HUpnpAv/HContainer>
#include <HUpnpAv/HMediaBrowser>
#include <HUpnpAv/HAvControlPoint>
#include <HUpnpAv/HMediaServerAdapter>
#include <HUpnpAv/HMediaRendererAdapter>
#include <HUpnpAv/HAvControlPointConfiguration>

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

ControlPointWindow::ControlPointWindow(QWidget* parent) :
    QMainWindow(parent),
        m_ui(new Ui::ControlPointWindow), m_controlPoint(0),
        m_controlpointNavigator(0), m_dataItemDisplay(0)
{
    m_ui->setupUi(this);

    HAvControlPointConfiguration configuration;
    // The controlpoint can be configured, but here it isn't done, so we could have
    // used the constructor that doesn't take HAvControlPointConfiguration object.
    m_controlPoint = new HAvControlPoint(configuration, this);

    bool ok = connect(
        m_controlPoint,
        SIGNAL(mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter*)),
        this,
        SLOT(mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter*)));

    Q_ASSERT(ok);

    ok = connect(
        m_controlPoint,
        SIGNAL(mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter*)),
        this,
        SLOT(mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter*)));

    Q_ASSERT(ok);

    ok = connect(
        m_controlPoint,
        SIGNAL(mediaServerOnline(Herqq::Upnp::Av::HMediaServerAdapter*)),
        this,
        SLOT(mediaServerOnline(Herqq::Upnp::Av::HMediaServerAdapter*)));

    Q_ASSERT(ok);

    ok = connect(
        m_controlPoint,
        SIGNAL(mediaServerOffline(Herqq::Upnp::Av::HMediaServerAdapter*)),
        this,
        SLOT(mediaServerOffline(Herqq::Upnp::Av::HMediaServerAdapter*)));

    Q_ASSERT(ok);

    m_controlpointNavigator = new ControlPointNavigator(this);
    m_ui->navigatorTreeView->setModel(m_controlpointNavigator);

    m_dataItemDisplay = new DataItemDisplay(this);
    m_ui->dataTableView->setModel(m_dataItemDisplay);

    ok = m_controlPoint->init();
    Q_ASSERT(ok);
}

ControlPointWindow::~ControlPointWindow()
{
    delete m_ui;
    delete m_controlpointNavigator;
    delete m_dataItemDisplay;
    delete m_controlPoint;
}

void ControlPointWindow::mediaRendererOnline(HMediaRendererAdapter* newDevice)
{
    m_controlpointNavigator->mediaRendererOnline(newDevice);
}

void ControlPointWindow::mediaRendererOffline(HMediaRendererAdapter* adapter)
{
    m_dataItemDisplay->deviceRemoved(adapter->device()->info().udn());
    m_controlpointNavigator->mediaRendererOffline(adapter);
}

void ControlPointWindow::mediaServerOnline(HMediaServerAdapter* newDevice)
{
    HMediaBrowser* browser = m_controlpointNavigator->mediaServerOnline(newDevice);
    bool ok = connect(
        browser,
        SIGNAL(objectsBrowsed(Herqq::Upnp::Av::HMediaBrowser*,QSet<QString>)),
        this,
        SLOT(objectsBrowsed(Herqq::Upnp::Av::HMediaBrowser*,QSet<QString>)));
    Q_ASSERT(ok); Q_UNUSED(ok)
}

void ControlPointWindow::mediaServerOffline(HMediaServerAdapter* adapter)
{
    m_dataItemDisplay->deviceRemoved(adapter->device()->info().udn());
    m_controlpointNavigator->mediaServerOffline(adapter);
}

void ControlPointWindow::changeEvent(QEvent* e)
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

void ControlPointWindow::closeEvent(QCloseEvent*)
{
    emit closed();
}

void ControlPointWindow::on_navigatorTreeView_clicked(const QModelIndex& index)
{
    ControlPointNavigatorItem* navItem =
        static_cast<ControlPointNavigatorItem*>(index.internalPointer());

    if (navItem)
    {
        m_dataItemDisplay->setData(navItem);

        CdsContainerItem* containerItem = dynamic_cast<CdsContainerItem*>(navItem);
        if (containerItem && containerItem->container()->childIds().isEmpty())
        {
            ContentDirectoryItem* cdItem = 0;
            ControlPointNavigatorItem* tmpItem = containerItem->parent();
            for(;;)
            {
                cdItem = dynamic_cast<ContentDirectoryItem*>(tmpItem);
                if (cdItem)
                {
                    break;
                }
                else
                {
                    tmpItem = tmpItem->parent();
                }
            }
            cdItem->browser()->browse(HBrowseParams(
                containerItem->container()->id(), HBrowseParams::ObjectAndDirectChildren));
        }
    }
}

void ControlPointWindow::objectsBrowsed(HMediaBrowser*, const QSet<QString>&)
{
    QModelIndex curIndex = m_ui->navigatorTreeView->currentIndex();
    if (curIndex.isValid())
    {
        ControlPointNavigatorItem* navItem =
            static_cast<ControlPointNavigatorItem*>(curIndex.internalPointer());

        if (navItem)
        {
            m_dataItemDisplay->setData(navItem);
        }
    }
}

void ControlPointWindow::on_dataTableView_doubleClicked(const QModelIndex& index)
{
    DisplayDataRow* row = m_dataItemDisplay->displayDataRow(index.row());
    if (row->type() != DisplayDataRow::CdsItem)
    {
        return;
    }

    ControlPointNavigatorItem* navItem =
        static_cast<ControlPointNavigatorItem*>(
            m_ui->navigatorTreeView->currentIndex().internalPointer());

    if (navItem)
    {
        CdsContainerItem* containerItem = dynamic_cast<CdsContainerItem*>(navItem);
        if (containerItem)
        {
            MediaManagerDialog* dlg =
                new MediaManagerDialog(
                    m_controlPoint, containerItem->dataSource(), row->column(1), this);

            bool ok = connect(dlg, SIGNAL(finished(int)), dlg, SLOT(deleteLater()));
            Q_ASSERT(ok); Q_UNUSED(ok)

            dlg->show();
        }
    }
}
