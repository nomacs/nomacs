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

#include "mediarenderer_connectionwindow.h"
#include "ui_mediarenderer_connectionwindow.h"

#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtNetwork/QNetworkAccessManager>

using namespace Herqq::Upnp::Av;

namespace
{
bool isAudio(const QString& contentFormat)
{
    return contentFormat.startsWith("audio", Qt::CaseInsensitive);
}

bool isVideo(const QString& contentFormat)
{
    return contentFormat.startsWith("video", Qt::CaseInsensitive);
}

bool isImage(const QString& contentFormat)
{
    return contentFormat.startsWith("image", Qt::CaseInsensitive);
}

bool isText(const QString& contentFormat)
{
    return contentFormat.startsWith("text", Qt::CaseInsensitive);
}
}

MediaRendererConnectionWindow::MediaRendererConnectionWindow(
    const QString& contentFormat, QNetworkAccessManager& nam, QWidget* parent) :
        QWidget(parent),
            ui(new Ui::MediaRendererConnectionWindow()), m_rendererConnection(0),
            m_nam(nam)
{
    ui->setupUi(this);
    ui->scrollArea->setWidgetResizable(true);

    if (isAudio(contentFormat))
    {
        m_rendererConnection = new DefaultRendererConnection(
            DefaultRendererConnection::AudioOnly, ui->scrollAreaWidgetContents);
    }
    else if (isVideo(contentFormat))
    {
        m_rendererConnection = new DefaultRendererConnection(
            DefaultRendererConnection::AudioVideo, ui->scrollAreaWidgetContents);
    }
    else if (isImage(contentFormat))
    {
        m_rendererConnection = new RendererConnectionForImagesAndText(
            RendererConnectionForImagesAndText::Images, m_nam, ui->scrollAreaWidgetContents);
    }
    else if (isText(contentFormat))
    {
        m_rendererConnection = new RendererConnectionForImagesAndText(
            RendererConnectionForImagesAndText::Text, m_nam, ui->scrollAreaWidgetContents);
    }
    else if (contentFormat == "*" || contentFormat.isEmpty() ||
             contentFormat == "application/octet-stream")
    {
        m_rendererConnection = new DefaultRendererConnection(
            DefaultRendererConnection::Unknown, ui->scrollAreaWidgetContents);
    }
    else
    {
        deleteLater();
        return;
    }

    bool ok = connect(
        m_rendererConnection, SIGNAL(disposed(Herqq::Upnp::Av::HRendererConnection*)),
        this, SLOT(disposed(Herqq::Upnp::Av::HRendererConnection*)));
    Q_ASSERT(ok); Q_UNUSED(ok)
}

MediaRendererConnectionWindow::~MediaRendererConnectionWindow()
{
    if (m_rendererConnection)
    {
        m_rendererConnection->deleteLater();
    }
    delete ui;
}

HRendererConnection* MediaRendererConnectionWindow::rendererConnection() const
{
    return m_rendererConnection;
}

void MediaRendererConnectionWindow::disposed(Herqq::Upnp::Av::HRendererConnection*)
{
    deleteLater();
}

void MediaRendererConnectionWindow::closeEvent(QCloseEvent* e)
{
    e->ignore();
    //deleteLater();
}

void MediaRendererConnectionWindow::resizeEvent(QResizeEvent* e)
{
    m_rendererConnection->resizeEventOccurred(*e);
}
