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

#ifndef RENDERERCONNECTIONS_H_
#define RENDERERCONNECTIONS_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpAv/HRendererConnection>

#include <phonon/MediaObject>
#include <phonon/VideoWidget>

#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>

#include <QtCore/QPointer>
#include <QtCore/QScopedPointer>

class QLabel;
class QTextEdit;
class QNetworkReply;
class QNetworkAccessManager;

//
//
//
class CustomRendererConnection :
    public Herqq::Upnp::Av::HRendererConnection
{
public:

    CustomRendererConnection(QObject* parent);
    virtual void resizeEventOccurred(const QResizeEvent&);
};

//
//
//
class RendererConnectionForImagesAndText :
    public CustomRendererConnection
{
Q_OBJECT
Q_DISABLE_COPY(RendererConnectionForImagesAndText)

public:

    enum ContentType
    {
        Images,
        Text,
        Unknown
    };

private:

    struct Image
    {
        QPixmap m_pixmap;
        QPointer<QLabel> m_label;

        Image() : m_pixmap(), m_label(0)
        {
        }

        ~Image()
        {
            if (m_label) { delete m_label; }
        }

    };

    union
    {
        QTextEdit* m_textEdit;
        Image* m_image;
    };

    ContentType m_contentType;
    QNetworkAccessManager& m_nam;
    QPointer<QNetworkReply> m_currentResource;
    QByteArray m_currentData;
    bool m_showWhenReady;
    bool m_stopped;

private:

    void show();

private Q_SLOTS:

    void finished();

protected:

    virtual qint32 doPlay(const QString& arg);
    virtual qint32 doStop();

    virtual qint32 doSeek(const Herqq::Upnp::Av::HSeekInfo& seekInfo);
    virtual qint32 doNext();
    virtual qint32 doPrevious();

    virtual qint32 doSetResource(
        const QUrl& resourceUri, Herqq::Upnp::Av::HObject* cdsObjectData = 0);

    virtual qint32 doSelectPreset(const QString& presetName);

public:

    explicit RendererConnectionForImagesAndText(
        ContentType, QNetworkAccessManager&, QWidget* parent = 0);

    virtual ~RendererConnectionForImagesAndText();

    virtual void resizeEventOccurred(const QResizeEvent&);
};

//
//
//
class DefaultRendererConnection :
    public CustomRendererConnection
{
Q_OBJECT
Q_DISABLE_COPY(DefaultRendererConnection)

private:

    Phonon::MediaObject m_mediaObject;
    QScopedPointer<Phonon::MediaSource> m_mediaSource;
    Phonon::VideoWidget* m_videoWidget;

private:

    void setupVideo();

private Q_SLOTS:

    void hasVideoChanged(bool);
    void stateChanged(Phonon::State newstate, Phonon::State oldstate);
    void tick(qint64 time);
    void totalTimeChanged(qint64 time);

protected:

    virtual qint32 doPlay(const QString& arg);
    virtual qint32 doStop();
    virtual qint32 doPause();

    virtual qint32 doSeek(const Herqq::Upnp::Av::HSeekInfo& seekInfo);
    virtual qint32 doNext();
    virtual qint32 doPrevious();

    virtual qint32 doSetResource(
        const QUrl& resourceUri, Herqq::Upnp::Av::HObject* cdsObjectData = 0);

    virtual qint32 doSelectPreset(const QString& presetName);

public:

    enum ContentType
    {
        AudioOnly,
        AudioVideo,
        Unknown
    };

    explicit DefaultRendererConnection(ContentType, QWidget* parent = 0);
    virtual ~DefaultRendererConnection();
};

#endif // RENDERERCONNECTIONS_H_
