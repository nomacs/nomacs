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

#include "controlpoint_window.h"
#include "invokeactiondialog.h"
#include "ui_controlpoint.h"

#include "dataitem_display.h"
#include "controlpoint_navigator.h"
#include "controlpoint_navigatoritem.h"

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HControlPoint>
#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HStateVariableInfo>
#include <HUpnpCore/HStateVariableEvent>
#include <HUpnpCore/HClientStateVariable>
#include <HUpnpCore/HControlPointConfiguration>

using namespace Herqq::Upnp;

ControlPointWindow::ControlPointWindow(QWidget* parent) :
    QMainWindow(parent),
        m_ui(new Ui::ControlPointWindow), m_controlPoint(0),
        m_controlpointNavigator(0), m_dataItemDisplay(0)
{
    m_ui->setupUi(this);

    HControlPointConfiguration configuration;
    //configuration.setDesiredSubscriptionTimeout(30);
    // you can uncomment this ^^ to test subscription renewal with UPnP devices
    // that respect the subscription timeout requests of control points
    m_controlPoint = new HControlPoint(configuration, this);

    bool ok = connect(
        m_controlPoint,
        SIGNAL(rootDeviceOnline(Herqq::Upnp::HClientDevice*)),
        this,
        SLOT(rootDeviceOnline(Herqq::Upnp::HClientDevice*)));

    Q_ASSERT(ok);

    ok = connect(
        m_controlPoint,
        SIGNAL(rootDeviceOffline(Herqq::Upnp::HClientDevice*)),
        this,
        SLOT(rootDeviceOffline(Herqq::Upnp::HClientDevice*)));

    Q_ASSERT(ok);

    m_controlpointNavigator = new ControlPointNavigator(this);
    m_ui->navigatorTreeView->setModel(m_controlpointNavigator);

    m_dataItemDisplay = new DataItemDisplay(this);
    m_ui->dataTableView->setModel(m_dataItemDisplay);

    m_controlPoint->init();
}

ControlPointWindow::~ControlPointWindow()
{
    delete m_ui;
    delete m_controlpointNavigator;
    delete m_dataItemDisplay;
    delete m_controlPoint;
}

void ControlPointWindow::connectToEvents(HClientDevice* device)
{
    HClientServices services = device->services();
    for (qint32 i = 0; i < services.size(); ++i)
    {
        HClientStateVariables stateVars = services[i]->stateVariables();
        foreach(const QString& name, stateVars.keys())
        {
            bool ok = connect(
                stateVars.value(name),
                SIGNAL(valueChanged(const Herqq::Upnp::HClientStateVariable*,
                                    Herqq::Upnp::HStateVariableEvent)),
                this,
                SLOT(stateVariableChanged(const Herqq::Upnp::HClientStateVariable*,
                                          Herqq::Upnp::HStateVariableEvent)));

            Q_ASSERT(ok); Q_UNUSED(ok)
        }
    }

    QList<HClientDevice*> embeddedDevices = device->embeddedDevices();
    for(qint32 i = 0; i < embeddedDevices.size(); ++i)
    {
        connectToEvents(embeddedDevices[i]);
    }
}

void ControlPointWindow::stateVariableChanged(
    const HClientStateVariable* source, const HStateVariableEvent& event)
{
    m_ui->status->append(QString(
        "State variable [%1] changed value from [%2] to [%3]").arg(
            source->info().name(), event.previousValue().toString(),
            event.newValue().toString()));
}

void ControlPointWindow::rootDeviceOnline(HClientDevice* newDevice)
{
    m_controlpointNavigator->rootDeviceOnline(newDevice);
    connectToEvents(newDevice);
}

void ControlPointWindow::rootDeviceOffline(HClientDevice* device)
{
    m_controlpointNavigator->rootDeviceOffline(device);
    m_dataItemDisplay->deviceRemoved(device->info().udn());
    emit contentSourceRemoved(device);
}

void ControlPointWindow::changeEvent(QEvent *e)
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

void ControlPointWindow::on_navigatorTreeView_clicked(QModelIndex index)
{
    ControlPointNavigatorItem* navItem =
        static_cast<ControlPointNavigatorItem*>(index.internalPointer());

    if (!navItem)
    {
        return;
    }

    m_dataItemDisplay->setData(navItem);
}

void ControlPointWindow::on_navigatorTreeView_doubleClicked(QModelIndex index)
{
    ControlPointNavigatorItem* navItem =
        static_cast<ControlPointNavigatorItem*>(index.internalPointer());

    if (!navItem)
    {
        return;
    }

    ActionItem* ai = dynamic_cast<ActionItem*>(navItem);
    if (ai)
    {
        InvokeActionDialog* dlg = new InvokeActionDialog(ai->action(), this);

        bool ok = connect(dlg, SIGNAL(finished(int)), dlg, SLOT(deleteLater()));
        Q_ASSERT(ok); Q_UNUSED(ok)

        ok = connect(
            this, SIGNAL(contentSourceRemoved(Herqq::Upnp::HClientDevice*)),
            dlg , SLOT(contentSourceRemoved(Herqq::Upnp::HClientDevice*)));
        Q_ASSERT(ok);

        dlg->show();
    }
}
