/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HDEVICEHOST_SSDP_HANDLER_P_H_
#define HDEVICEHOST_SSDP_HANDLER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../hdevicestorage_p.h"

#include "../../ssdp/hssdp.h"
#include "../../ssdp/hssdp_p.h"
#include "../../ssdp/hdiscovery_messages.h"

#include "../../socket/hendpoint.h"

#include <QtCore/QList>

namespace Herqq
{

namespace Upnp
{

class HServerDevice;
class HDeviceHostSsdpHandler;
class HServerDeviceController;

//
//
//
class HDelayedWriter :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HDelayedWriter)

private:

    HDeviceHostSsdpHandler& m_ssdp;
    QList<HDiscoveryResponse> m_responses;
    HEndpoint m_source;
    qint32 m_msecs;

protected:

    void timerEvent(QTimerEvent*);

public:

    HDelayedWriter(
        HDeviceHostSsdpHandler&,
        const QList<HDiscoveryResponse>&,
        const HEndpoint& source,
        qint32 msecs);

    void run();

Q_SIGNALS:

    void sent();
};

//
//
//
class HDeviceHostSsdpHandler :
    public HSsdp
{
H_DISABLE_COPY(HDeviceHostSsdpHandler)

private:

    HDeviceStorage<HServerDevice, HServerService, HServerDeviceController>& m_deviceStorage;

private:

    void processSearchRequest(
        const HServerDevice*, const QUrl& deviceLocation,
        QList<HDiscoveryResponse>*);

    bool processSearchRequest_AllDevices(
        const HDiscoveryRequest&, const HEndpoint&,
        QList<HDiscoveryResponse>*);

    bool processSearchRequest_RootDevice(
        const HDiscoveryRequest&, const HEndpoint&,
        QList<HDiscoveryResponse>*);

    bool processSearchRequest_specificDevice(
        const HDiscoveryRequest&, const HEndpoint&,
        QList<HDiscoveryResponse>*);

    bool processSearchRequest_deviceType(
        const HDiscoveryRequest&, const HEndpoint&,
        QList<HDiscoveryResponse>*);

    bool processSearchRequest_serviceType(
        const HDiscoveryRequest&, const HEndpoint&,
        QList<HDiscoveryResponse>*);

protected:

    virtual bool incomingDiscoveryRequest(
        const HDiscoveryRequest&, const HEndpoint&, DiscoveryRequestMethod);

public:

    HDeviceHostSsdpHandler(
        const QByteArray& loggingIdentifier,
        HDeviceStorage<HServerDevice, HServerService, HServerDeviceController>&,
        QObject* parent = 0);

    virtual ~HDeviceHostSsdpHandler();

    inline const QByteArray& loggingIdentifier() const
    {
        return h_ptr->m_loggingIdentifier;
    }
};

}
}

#endif /* HDEVICEHOST_SSDP_HANDLER_P_H_ */
