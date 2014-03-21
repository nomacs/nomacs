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

#ifndef CONTROLPOINT_WINDOW_H
#define CONTROLPOINT_WINDOW_H

#include <HUpnpCore/HUpnp>

#include <QModelIndex>
#include <QtGui/QMainWindow>

class DataItemDisplay;
class ControlPointNavigator;

namespace Ui {
    class ControlPointWindow;
}

//
//
//
class ControlPointWindow :
    public QMainWindow
{
Q_OBJECT
Q_DISABLE_COPY(ControlPointWindow)

private:

    Ui::ControlPointWindow* m_ui;

    Herqq::Upnp::HControlPoint* m_controlPoint;

    ControlPointNavigator* m_controlpointNavigator;

    DataItemDisplay* m_dataItemDisplay;

private:

    void connectToEvents(Herqq::Upnp::HClientDevice*);

protected:

    virtual void changeEvent(QEvent*);
    virtual void closeEvent(QCloseEvent*);

private slots:

    void rootDeviceOnline(Herqq::Upnp::HClientDevice*);
    void rootDeviceOffline(Herqq::Upnp::HClientDevice*);

    void stateVariableChanged(
        const Herqq::Upnp::HClientStateVariable*,
        const Herqq::Upnp::HStateVariableEvent&);

    void on_navigatorTreeView_doubleClicked(QModelIndex);
    void on_navigatorTreeView_clicked(QModelIndex);

public:

    explicit ControlPointWindow(QWidget* parent = 0);
    virtual ~ControlPointWindow();

Q_SIGNALS:

    void contentSourceRemoved(Herqq::Upnp::HClientDevice*);
    void closed();
};

#endif // CONTROLPOINT_WINDOW_H
