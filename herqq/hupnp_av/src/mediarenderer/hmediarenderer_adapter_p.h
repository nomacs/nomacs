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

#ifndef HMEDIARENDERER_ADAPTER_P_H_
#define HMEDIARENDERER_ADAPTER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hclientdevice_adapter_p.h>

#include "../common/hprotocolinfo.h"
#include "../connectionmanager/hconnectionmanager_id.h"

#include <QtCore/QHash>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HPrepareForConnectionOp :
    public HClientAdapterOp<qint32>
{
H_DISABLE_ASSIGN(HPrepareForConnectionOp)

private:

    const HProtocolInfo m_protocolInfo;
    const HConnectionManagerId m_peerCm;

public:

    HPrepareForConnectionOp(){}
    HPrepareForConnectionOp(const HPrepareForConnectionOp& other);
    HPrepareForConnectionOp(const HProtocolInfo&, const HConnectionManagerId& peerCm);

    inline const HProtocolInfo& protocolInfo() const
    {
        return m_protocolInfo;
    }

    inline const HConnectionManagerId& peerConnectionManager() const
    {
        return m_peerCm;
    }
};

//
// Implementation details of HMediaRendererAdapter.
//
class HMediaRendererAdapterPrivate :
    public HClientDeviceAdapterPrivate
{
H_DECLARE_PUBLIC(HMediaRendererAdapter)
H_DISABLE_COPY(HMediaRendererAdapterPrivate)

private:

    void cacheConnection(HConnection*);

public:

    QScopedPointer<HConnectionManagerAdapter> m_cm;
    HClientService* m_transportService;
    HClientService* m_renderingControlService;

    QHash<qint32, HConnection*> m_cachedConnections;

    HMediaRendererAdapterPrivate();

    virtual ~HMediaRendererAdapterPrivate();

    void connectSignals();
};

}
}
}

#endif /* HMEDIARENDERER_ADAPTER_P_H_ */
