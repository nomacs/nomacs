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

#ifndef HAVTRANSPORT_ADAPTER_P_H_
#define HAVTRANSPORT_ADAPTER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hclientservice_adapter_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HAvTransportAdapterPrivate :
    public HClientServiceAdapterPrivate
{
H_DECLARE_PUBLIC(HAvTransportAdapter)
H_DISABLE_COPY(HAvTransportAdapterPrivate)

public:

    quint32 m_instanceId;

    HAvTransportAdapterPrivate();
    virtual ~HAvTransportAdapterPrivate();

    bool setAVTransportURI(HClientAction*, const HClientActionOp&);
    bool setNextAVTransportURI(HClientAction*, const HClientActionOp&);
    bool getMediaInfo(HClientAction*, const HClientActionOp&);
    bool getMediaInfo_ext(HClientAction*, const HClientActionOp&);
    bool getTransportInfo(HClientAction*, const HClientActionOp&);
    bool getPositionInfo(HClientAction*, const HClientActionOp&);
    bool getDeviceCapabilities(HClientAction*, const HClientActionOp&);
    bool getTransportSettings(HClientAction*, const HClientActionOp&);
    bool stop(HClientAction*, const HClientActionOp&);
    bool play(HClientAction*, const HClientActionOp&);
    bool pause(HClientAction*, const HClientActionOp&);
    bool record(HClientAction*, const HClientActionOp&);
    bool seek(HClientAction*, const HClientActionOp&);
    bool next(HClientAction*, const HClientActionOp&);
    bool previous(HClientAction*, const HClientActionOp&);
    bool setPlayMode(HClientAction*, const HClientActionOp&);
    bool setRecordQualityMode(HClientAction*, const HClientActionOp&);
    bool getCurrentTransportActions(HClientAction*, const HClientActionOp&);
    bool getDRMState(HClientAction*, const HClientActionOp&);
    bool getStateVariables(HClientAction*, const HClientActionOp&);
    bool setStateVariables(HClientAction*, const HClientActionOp&);
};

}
}
}

#endif /* HAVTRANSPORT_ADAPTER_P_H_ */
