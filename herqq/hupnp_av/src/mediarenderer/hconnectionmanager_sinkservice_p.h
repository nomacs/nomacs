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

#ifndef HCONNECTIONMANAGER_SINKSERVICE_P_H_
#define HCONNECTIONMANAGER_SINKSERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../connectionmanager/hconnectionmanager_service_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMediaRendererDevice;

//
//
//
class HConnectionManagerSinkService :
    public HConnectionManagerService
{
Q_OBJECT
H_DISABLE_COPY(HConnectionManagerSinkService)

private:

    HMediaRendererDevice* m_owner;

private slots:

    void connectionAdded(
        Herqq::Upnp::Av::HAbstractConnectionManagerService* cmService,
        const Herqq::Upnp::Av::HConnectionInfo& connectionInfo);

protected:

    virtual bool finalizeInit(QString* errDescription);

public:

    HConnectionManagerSinkService();
    virtual ~HConnectionManagerSinkService();

    bool init(HMediaRendererDevice*);

    virtual qint32 prepareForConnection(
        const HProtocolInfo& remoteProtocolInfo,
        const HConnectionManagerId& peerConnectionManager,
        qint32 peerConnectionId,
        HConnectionManagerInfo::Direction direction,
        HPrepareForConnectionResult* result);

    virtual qint32 connectionComplete(qint32 connectionId);

    virtual qint32 getProtocolInfo(HProtocolInfoResult*);

    virtual qint32 getCurrentConnectionIDs(QList<quint32>* retVal);
    virtual qint32 getCurrentConnectionInfo(
        qint32 connectionId, HConnectionInfo* retVal);

    qint32 nextId();
};

}
}
}

#endif /* HCONNECTIONMANAGER_SINKSERVICE_P_H_ */
