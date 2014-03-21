/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HMEDIARENDERER_DEVICE_P_H_
#define HMEDIARENDERER_DEVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HServerDevice>
#include <HUpnpCore/private/hserverdevice_p.h>

#include "hrendererconnection.h"
#include "htransport_sinkservice_p.h"
#include "habstractmediarenderer_device.h"
#include "hconnectionmanager_sinkservice_p.h"
#include "hmediarenderer_deviceconfiguration.h"
#include "../renderingcontrol/hrenderingcontrol_service_p.h"

#include <QtCore/QTimer>
#include <QtCore/QPointer>
#include <QtNetwork/QNetworkReply>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HInstanceEvents
{
H_DISABLE_COPY(HInstanceEvents)

public:

    const qint32 m_instanceId;
    QHash<QString, QPair<QString, QString> > m_changedProperties;
    // The first component of the pair is the "val" attribute and the second
    // component is the "channel", if any.

    HInstanceEvents(qint32 instanceId) :
        m_instanceId(instanceId)
    {
    }
};

//
//
//
class HMediaRendererDevice :
    public HAbstractMediaRendererDevice
{
Q_OBJECT
H_DISABLE_COPY(HMediaRendererDevice)

private:

    HMediaRendererDeviceConfiguration* m_configuration;

    QTimer m_timer;
    QList<HInstanceEvents*> m_avtInstanceEvents;
    QList<HInstanceEvents*> m_rcsInstanceEvents;

private Q_SLOTS:

    void timeout();

    void propertyChanged(
        Herqq::Upnp::Av::HRendererConnectionInfo* source,
        const Herqq::Upnp::Av::HRendererConnectionEventInfo& eventInfo);

    void rendererConnectionRemoved(
        Herqq::Upnp::Av::HAbstractConnectionManagerService*, qint32);

protected:

    virtual bool event(QEvent*);
    virtual bool finalizeInit(QString* errDescription);

public:

    HMediaRendererDevice(const HMediaRendererDeviceConfiguration& configuration);
    virtual ~HMediaRendererDevice();

    HMediaRendererDeviceConfiguration* configuration() const;

    virtual HRenderingControlService* renderingControl() const;
    virtual HConnectionManagerSinkService* connectionManager() const;
    virtual HTransportSinkService* avTransport() const;

    qint32 prepareForConnection(HConnectionInfo*);

    qint32 connectionComplete(qint32 connectionId);

    HRendererConnection* createRendererConnection(const HConnectionInfo&);

    HRendererConnection* findConnectionByAvTransportId(qint32 id) const;
    HRendererConnection* findConnectionByRcsId(qint32 id) const;
};

}
}
}

#endif /* HMEDIARENDERER_DEVICE_P_H_ */
