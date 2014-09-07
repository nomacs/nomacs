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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui {
    class MainWindow;
}

//
// Main window for the test application.
//
class MainWindow :
    public QMainWindow
{
Q_OBJECT
Q_DISABLE_COPY(MainWindow)

public:

    explicit MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

protected:

    virtual void changeEvent(QEvent*);

private:

    Ui::MainWindow* m_ui;

private slots:

    void on_startControlPoint_clicked();
    void on_startMediaServer_clicked();
    void on_startMediaRenderer_clicked();

    void rendererWindowClosed();
    void serverWindowClosed();
};

#endif // MAINWINDOW_H
