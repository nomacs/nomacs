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

#ifndef CONTROLPOINT_WINDOW_H
#define CONTROLPOINT_WINDOW_H

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClientDevice>

#include <QtCore/QModelIndex>
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

    Herqq::Upnp::Av::HAvControlPoint* m_controlPoint;

    ControlPointNavigator* m_controlpointNavigator;

    DataItemDisplay* m_dataItemDisplay;

protected:

    virtual void changeEvent(QEvent*);
    virtual void closeEvent(QCloseEvent*);

private slots:

    void on_dataTableView_doubleClicked(const QModelIndex& index);
    void mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter*);
    void mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter*);

    void mediaServerOnline(Herqq::Upnp::Av::HMediaServerAdapter*);
    void mediaServerOffline(Herqq::Upnp::Av::HMediaServerAdapter*);

    void on_navigatorTreeView_clicked(const QModelIndex&);

    void objectsBrowsed(Herqq::Upnp::Av::HMediaBrowser*, const QSet<QString>&);

public:

    explicit ControlPointWindow(QWidget* parent = 0);
    virtual ~ControlPointWindow();

Q_SIGNALS:

    void contentSourceRemoved(Herqq::Upnp::HClientDevice*);
    void closed();
};

#endif // CONTROLPOINT_WINDOW_H
