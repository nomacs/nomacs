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

#ifndef MEDIARENDERER_WINDOW_H_
#define MEDIARENDERER_WINDOW_H_

#include <HUpnpCore/HDeviceHost>
#include <HUpnpAv/HRendererConnectionManager>

#include <QtGui/QMainWindow>

class QNetworkAccessManager;

namespace Ui {
    class MediaRendererWindow;
}

class MediaRendererWindow;

//
//
//
class RendererConnectionManager :
    public Herqq::Upnp::Av::HRendererConnectionManager
{
private:

    MediaRendererWindow* m_owner;

protected:

    virtual Herqq::Upnp::Av::HRendererConnection* doCreate(
        Herqq::Upnp::Av::HAbstractConnectionManagerService* service,
        Herqq::Upnp::Av::HConnectionInfo* cinfo);

public:

    RendererConnectionManager(MediaRendererWindow* owner);
};

//
//
//
class MediaRendererWindow :
    public QMainWindow
{
Q_OBJECT
friend class RendererConnectionManager;

private Q_SLOTS:

    void currentConnectionIDsChanged(Herqq::Upnp::HServerStateVariable*);

    void propertyChanged(
        Herqq::Upnp::Av::HRendererConnectionInfo*,
        const Herqq::Upnp::Av::HRendererConnectionEventInfo&);

private:

    Ui::MediaRendererWindow *m_ui;
    Herqq::Upnp::HDeviceHost* m_deviceHost;

    Herqq::Upnp::Av::HAbstractMediaRendererDevice* m_mediaRenderer;
    RendererConnectionManager* m_mm;
    QNetworkAccessManager* m_nam;

protected:

    virtual void changeEvent(QEvent*);
    virtual void closeEvent(QCloseEvent*);

public:

    MediaRendererWindow(QWidget *parent = 0);
    virtual ~MediaRendererWindow();

Q_SIGNALS:

    void closed();
};

#endif // MEDIARENDERER_WINDOW_H_
