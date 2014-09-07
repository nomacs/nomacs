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

#include "mediamanager_dialog.h"
#include "ui_mediamanager_dialog.h"

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HClientDevice>

#include <HUpnpAv/HItem>
#include <HUpnpAv/HResource>
#include <HUpnpAv/HContainer>
#include <HUpnpAv/HConnection>
#include <HUpnpAv/HCdsDataSource>
#include <HUpnpAv/HAvControlPoint>
#include <HUpnpAv/HAvtLastChangeInfo>
#include <HUpnpAv/HRcsLastChangeInfo>
#include <HUpnpAv/HConnectionManagerId>


#include <HUpnpAv/HAvTransportAdapter>
#include <HUpnpAv/HMediaRendererAdapter>
#include <HUpnpAv/HConnectionManagerAdapter>

#include <QtCore/QUrl>

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

MediaManagerDialog::MediaManagerDialog(
    HAvControlPoint* cp, HCdsDataSource* dataSource, const QString& objectId,
    QWidget* parent) :
        QDialog(parent),
            ui(new Ui::MediaManagerDialog), m_controlPoint(cp),
            m_dataSource(dataSource), m_objectId(objectId), m_renderer(0),
            m_connection(0), m_state(Stopped)
{
    Q_ASSERT(m_dataSource);
    Q_ASSERT(m_controlPoint);
    Q_ASSERT(!m_objectId.isEmpty());

    ui->setupUi(this);

    bool ok = connect(
        m_controlPoint,
        SIGNAL(mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter*)),
        this,
        SLOT(mediaRendererOnline(Herqq::Upnp::Av::HMediaRendererAdapter*)));

    Q_ASSERT(ok);

    ok = connect(
        m_controlPoint,
        SIGNAL(mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter*)),
        this,
        SLOT(mediaRendererOffline(Herqq::Upnp::Av::HMediaRendererAdapter*)));

    Q_ASSERT(ok);

    populateRenderersList();
    populateItemsList();
}

MediaManagerDialog::~MediaManagerDialog()
{
    delete ui;
}

void MediaManagerDialog::mediaRendererOnline(HMediaRendererAdapter* newDevice)
{
    const HDeviceInfo& dinfo = newDevice->device()->info();
    qint32 index = ui->mediaRenderersCb->findData(
        dinfo.udn().toSimpleUuid(), Qt::UserRole);

    if (index < 0)
    {
        ui->mediaRenderersCb->addItem(dinfo.friendlyName(), dinfo.udn().toSimpleUuid());
        enableControl(true);
    }
}

void MediaManagerDialog::mediaRendererOffline(HMediaRendererAdapter* device)
{
    qint32 index = ui->mediaRenderersCb->findData(
        device->device()->info().udn().toSimpleUuid(), Qt::UserRole);

    if (index >= 0)
    {
        ui->mediaRenderersCb->removeItem(index);
    }

    if (ui->mediaRenderersCb->count() <= 0)
    {
        enableControl(false);
    }
}

void MediaManagerDialog::populateRenderersList()
{
    HMediaRendererAdapters renderers = m_controlPoint->mediaRenderers();
    foreach(HMediaRendererAdapter* renderer, renderers)
    {
        ui->mediaRenderersCb->addItem(
            renderer->device()->info().friendlyName(),
            renderer->device()->info().udn().toSimpleUuid());
    }

    enableControl(renderers.size() > 0);
}

void MediaManagerDialog::populateItemsList()
{
    HObject* obj = m_dataSource->findObject(m_objectId);
    if (obj->isItem())
    {
        QListWidgetItem* newItem = new QListWidgetItem(obj->title());
        newItem->setData(Qt::UserRole, m_objectId);
        ui->itemsList->addItem(newItem);
    }
    else
    {
        HContainer* cont = obj->asContainer();
        QSet<QString> childIds = cont->childIds();
        foreach(const QString& childId, childIds)
        {
            HItem* item = m_dataSource->findItem(childId);
            if (item)
            {
                QListWidgetItem* newItem = new QListWidgetItem(item->title());
                newItem->setData(Qt::UserRole, childId);
                ui->itemsList->addItem(newItem);
            }
        }
    }

    ui->itemsList->setCurrentRow(0);
}

void MediaManagerDialog::enableControl(bool enable)
{
    if (enable)
    {
        if (m_state == Stopped)
        {
            ui->playButton->setEnabled(enable);
            ui->mediaRenderersCb->setEnabled(enable);
            ui->stopButton->setEnabled(!enable);
        }
        else
        {
            ui->playButton->setEnabled(!enable);
            ui->mediaRenderersCb->setEnabled(!enable);
            ui->stopButton->setEnabled(enable);
        }
    }
    else
    {
        ui->playButton->setEnabled(false);
        ui->mediaRenderersCb->setEnabled(false);
        ui->stopButton->setEnabled(false);
    }
}

void MediaManagerDialog::getConnection()
{
    Q_ASSERT(!m_connection && m_renderer);
    Q_ASSERT(m_state == Stopped);

    if (m_renderer->connectionManager()->implementsAction("PrepareForConnection"))
    {
        QListWidgetItem* curItem = ui->itemsList->currentItem();
        QVariant id = curItem->data(Qt::UserRole);
        HItem* cdsItem = m_dataSource->findItem(id.toString());
        Q_ASSERT(cdsItem);

        if (cdsItem->resources().size() > 0)
        {
            m_connectionRetrieveOp =
                m_renderer->prepareNewConnection(
                    cdsItem->resources().at(0).protocolInfo(),
                    HConnectionManagerId());

            if (m_connectionRetrieveOp.isNull())
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        m_connectionRetrieveOp = m_renderer->getConnection(0);
    }

    m_state = Connecting;
}

void MediaManagerDialog::connectionReady(HMediaRendererAdapter* source, qint32 cid)
{
    if (m_state != Connecting ||
        m_connectionRetrieveOp.returnValue() != UpnpSuccess ||
        m_connectionRetrieveOp.value() != cid)
    {
        return;
    }

    Q_ASSERT(!m_connection);
    m_connection.reset(source->takeConnection(m_connectionRetrieveOp.value()));
    m_connection->setAutoCloseConnection(true);
    Q_ASSERT(m_connection);

    QListWidgetItem* curItem = ui->itemsList->currentItem();
    QVariant curItemId = curItem->data(Qt::UserRole);
    HItem* cdsItem = m_dataSource->findItem(curItemId.toString());

    HClientAdapterOpNull op =
        m_connection->transport()->setAVTransportURI(
            cdsItem->resources().at(0).location(), "");

    Q_ASSERT(!op.isNull()); Q_UNUSED(op)

    bool ok = connect(
        m_connection->transport(),
        SIGNAL(setAVTransportURICompleted(Herqq::Upnp::Av::HAvTransportAdapter*, Herqq::Upnp::HClientAdapterOpNull)),
        this,
        SLOT(setAVTransportURICompleted(Herqq::Upnp::Av::HAvTransportAdapter*, Herqq::Upnp::HClientAdapterOpNull)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        m_connection->transport(),
        SIGNAL(playCompleted(Herqq::Upnp::Av::HAvTransportAdapter*, Herqq::Upnp::HClientAdapterOpNull)),
        this,
        SLOT(playCompleted(Herqq::Upnp::Av::HAvTransportAdapter*, Herqq::Upnp::HClientAdapterOpNull)));
    Q_ASSERT(ok);

    ok = connect(
        m_connection->transport(),
        SIGNAL(stopCompleted(Herqq::Upnp::Av::HAvTransportAdapter*, Herqq::Upnp::HClientAdapterOpNull)),
        this,
        SLOT(stopCompleted(Herqq::Upnp::Av::HAvTransportAdapter*, Herqq::Upnp::HClientAdapterOpNull)));
    Q_ASSERT(ok);

    ok = connect(
        m_connection.data(),
        SIGNAL(invalidated(Herqq::Upnp::Av::HConnection*)),
        this,
        SLOT(invalidated(Herqq::Upnp::Av::HConnection*)));
    Q_ASSERT(ok);

    ok = connect(
        m_connection.data(),
        SIGNAL(avTransportStateChanged(Herqq::Upnp::Av::HConnection*,Herqq::Upnp::Av::HAvtLastChangeInfos)),
        this,
        SLOT(avTransportStateChanged(Herqq::Upnp::Av::HConnection*,Herqq::Upnp::Av::HAvtLastChangeInfos)));
    Q_ASSERT(ok);

    ok = connect(
        m_connection.data(),
        SIGNAL(renderingControlStateChanged(Herqq::Upnp::Av::HConnection*,Herqq::Upnp::Av::HRcsLastChangeInfos)),
        this,
        SLOT(renderingControlStateChanged(Herqq::Upnp::Av::HConnection*,Herqq::Upnp::Av::HRcsLastChangeInfos)));
    Q_ASSERT(ok);

    m_state = SettingUp;
}

void MediaManagerDialog::invalidated(Herqq::Upnp::Av::HConnection* source)
{
    Q_ASSERT(source == m_connection.data()); Q_UNUSED(source)
    m_connection->setAutoCloseConnection(false);
    // When the connection is invalidated at server-side there's no "need" to
    // explicitly close it from client-side. It would actually cause an
    // action invocation error.
    m_connection.reset(0);
    m_state = Stopped;
    enableControl(true);
}

void MediaManagerDialog::avTransportStateChanged(
    HConnection*, const HAvtLastChangeInfos& infos)
{
    foreach(const HAvtLastChangeInfo& info, infos)
    {
        if (info.propertyName().compare("TransportState", Qt::CaseInsensitive) == 0)
        {
            HTransportState state(info.value().toString());
            if (state.isValid())
            {
                switch (state.type())
                {
                case HTransportState::Playing:
                    if (m_state != Playing)
                    {
                        m_state = Playing;
                        enableControl(true);
                    }
                    break;

                case HTransportState::Stopped:
                    if (m_state != Stopped)
                    {
                        m_state = Stopped;
                        enableControl(true);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void MediaManagerDialog::renderingControlStateChanged(
    HConnection*, const HRcsLastChangeInfos&)
{
}

void MediaManagerDialog::playCompleted(
    HAvTransportAdapter*, const HClientAdapterOpNull&)
{
    enableControl(true);
}

void MediaManagerDialog::stopCompleted(
    HAvTransportAdapter*, const HClientAdapterOpNull&)
{
    if (m_state != Stopped)
    {
        m_state = Stopped;
        enableControl(true);
    }
}

void MediaManagerDialog::setAVTransportURICompleted(
    HAvTransportAdapter*, const HClientAdapterOpNull&)
{
    m_connection->transport()->play("1");
    m_state = Playing;
    enableControl(true);
}

void MediaManagerDialog::on_playButton_clicked()
{
    enableControl(false);

    if (!m_renderer)
    {
        QVariant udn = ui->mediaRenderersCb->itemData(
            ui->mediaRenderersCb->currentIndex());

        HMediaRendererAdapter* renderer =
            m_controlPoint->mediaRenderer(udn.toString());

        if (renderer)
        {
            m_renderer = renderer;

            Q_ASSERT(!m_connection);

            bool ok = connect(
                m_renderer,
                SIGNAL(connectionReady(Herqq::Upnp::Av::HMediaRendererAdapter*, qint32)),
                this,
                SLOT(connectionReady(Herqq::Upnp::Av::HMediaRendererAdapter*, qint32)));
            Q_ASSERT(ok); Q_UNUSED(ok)

            getConnection();
        }

        return;
    }
    else if (m_connection)
    {
        m_connection->transport()->play("1");
        m_state = Playing;
    }
    else
    {
        getConnection();
    }

    enableControl(true);
}

void MediaManagerDialog::on_stopButton_clicked()
{
    if (m_state == Playing)
    {
        Q_ASSERT(m_connection);

        m_connection->transport()->stop();
        m_state = Stopping;

        enableControl(false);
    }
}

void MediaManagerDialog::on_mediaRenderersCb_currentIndexChanged(int)
{
    if (m_connection)
    {
        m_connection.reset(0);
        m_renderer = 0;
    }
}
