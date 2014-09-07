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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mediaserver_window.h"
#include "controlpoint_window.h"
#include "mediarenderer_window.h"

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

void MainWindow::rendererWindowClosed()
{
    m_ui->startMediaRenderer->setEnabled(true);
}

void MainWindow::serverWindowClosed()
{
    m_ui->startMediaServer->setEnabled(true);
}

void MainWindow::on_startControlPoint_clicked()
{
    ControlPointWindow* cpw = new ControlPointWindow(this);

    bool ok = connect(cpw, SIGNAL(closed()), cpw, SLOT(deleteLater()));

    Q_ASSERT(ok); Q_UNUSED(ok);

    cpw->show();
}

void MainWindow::on_startMediaServer_clicked()
{
    MediaServerWindow* msw = new MediaServerWindow(this);

    bool ok = connect(msw, SIGNAL(closed()), msw, SLOT(deleteLater()));

    Q_ASSERT(ok); Q_UNUSED(ok);

    ok = connect(msw, SIGNAL(closed()), this, SLOT(serverWindowClosed()));

    Q_ASSERT(ok);

    msw->show();

    m_ui->startMediaServer->setEnabled(false);
}

void MainWindow::on_startMediaRenderer_clicked()
{
    MediaRendererWindow* mrw = new MediaRendererWindow(this);

    bool ok = connect(mrw, SIGNAL(closed()), mrw, SLOT(deleteLater()));

    Q_ASSERT(ok); Q_UNUSED(ok);

    ok = connect(mrw, SIGNAL(closed()), this, SLOT(rendererWindowClosed()));

    Q_ASSERT(ok);

    m_ui->startMediaRenderer->setEnabled(false);

    mrw->show();
}
