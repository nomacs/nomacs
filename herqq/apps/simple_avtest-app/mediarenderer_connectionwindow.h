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

#ifndef MEDIARENDERER_CONNECTIONWINDOW_H
#define MEDIARENDERER_CONNECTIONWINDOW_H

#include "rendererconnections.h"

#include <HUpnpAv/HUpnpAv>

#include <QtGui/QWidget>
#include <QtCore/QPointer>

namespace Ui {
    class MediaRendererConnectionWindow;
}

class QNetworkAccessManager;

class MediaRendererConnectionWindow :
    public QWidget
{
Q_OBJECT

private:

    Ui::MediaRendererConnectionWindow *ui;

    QPointer<CustomRendererConnection> m_rendererConnection;
    QNetworkAccessManager& m_nam;

private Q_SLOTS:

    void disposed(Herqq::Upnp::Av::HRendererConnection*);

protected:

    virtual void closeEvent(QCloseEvent*);
    virtual void resizeEvent(QResizeEvent*);

public:

    explicit MediaRendererConnectionWindow(
        const QString& contentFormat, QNetworkAccessManager&,
        QWidget* parent = 0);

    virtual ~MediaRendererConnectionWindow();

    Herqq::Upnp::Av::HRendererConnection* rendererConnection() const;
};

#endif // MEDIARENDERER_CONNECTIONWINDOW_H
