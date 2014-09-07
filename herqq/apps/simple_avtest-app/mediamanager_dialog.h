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

#ifndef MEDIAMANAGER_DIALOG_H_
#define MEDIAMANAGER_DIALOG_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClientAdapterOp>

#include <QtGui/QDialog>

namespace Ui {
    class MediaManagerDialog;
}

//
//
//
class MediaManagerDialog :
    public QDialog
{
Q_OBJECT

private:

    Ui::MediaManagerDialog* ui;
    Herqq::Upnp::Av::HAvControlPoint* m_controlPoint;
    Herqq::Upnp::Av::HCdsDataSource* m_dataSource;
    QString m_objectId;

    enum State
    {
        Stopped,
        Connecting,
        SettingUp,
        Playing,
        Stopping
    };

    Herqq::Upnp::Av::HMediaRendererAdapter* m_renderer;
    QScopedPointer<Herqq::Upnp::Av::HConnection> m_connection;

    State m_state;

    Herqq::Upnp::HClientAdapterOp<qint32> m_connectionRetrieveOp;

private slots:

    void on_mediaRenderersCb_currentIndexChanged(int index);

    void mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter*);
    void mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter*);

    void connectionReady(Herqq::Upnp::Av::HMediaRendererAdapter*, qint32);

    void invalidated(Herqq::Upnp::Av::HConnection* source);

    void avTransportStateChanged(
        Herqq::Upnp::Av::HConnection* source,
        const Herqq::Upnp::Av::HAvtLastChangeInfos& info);

    void renderingControlStateChanged(
        Herqq::Upnp::Av::HConnection* source,
        const Herqq::Upnp::Av::HRcsLastChangeInfos& info);

    void playCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    void stopCompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull &op);

    void setAVTransportURICompleted(
        Herqq::Upnp::Av::HAvTransportAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    void on_playButton_clicked();
    void on_stopButton_clicked();

private:

    void populateRenderersList();
    void populateItemsList();
    void enableControl(bool enable);
    void getConnection();

public:

    MediaManagerDialog(
        Herqq::Upnp::Av::HAvControlPoint*, Herqq::Upnp::Av::HCdsDataSource*,
        const QString& objectId, QWidget* parent = 0);

    virtual ~MediaManagerDialog();
};

#endif // MEDIAMANAGER_DIALOG_H_
