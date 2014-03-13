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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "device_window.h"
#include "controlpoint_window.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
        m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::changeEvent(QEvent* e)
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

void MainWindow::deviceWindowClosed()
{
    m_ui->hostDeviceButton->setEnabled(true);
}

void MainWindow::on_hostDeviceButton_clicked()
{
    DeviceWindow* dw = new DeviceWindow(this);

    bool ok = connect(dw, SIGNAL(closed()), dw, SLOT(deleteLater()));

    Q_ASSERT(ok); Q_UNUSED(ok);

    ok = connect(dw, SIGNAL(closed()), this, SLOT(deviceWindowClosed()));

    Q_ASSERT(ok);

    dw->show();

    m_ui->hostDeviceButton->setEnabled(false);
}

void MainWindow::on_startControlPointButton_clicked()
{
    ControlPointWindow* cpw = new ControlPointWindow(this);

    bool ok = connect(cpw, SIGNAL(closed()), cpw, SLOT(deleteLater()));

    Q_ASSERT(ok); Q_UNUSED(ok);

    cpw->show();
}
