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

#ifndef HCONNECTIONMANAGER_P_H_
#define HCONNECTIONMANAGER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hclientservice_adapter_p.h>

#include <QtCore/QHash>

namespace Herqq
{

namespace Upnp
{

namespace Av
{


//
// Implementation details of HConnectionManagerAdapter
//
class H_UPNP_AV_EXPORT HConnectionManagerAdapterPrivate :
    public HClientServiceAdapterPrivate
{
H_DECLARE_PUBLIC(HConnectionManagerAdapter)
H_DISABLE_COPY(HConnectionManagerAdapterPrivate)

public:

    QHash<quint32, qint32> m_connectionIds;

    HConnectionManagerAdapterPrivate();
    virtual ~HConnectionManagerAdapterPrivate();

    bool getProtocolInfo         (HClientAction*, const HClientActionOp&);
    bool prepareForConnection    (HClientAction*, const HClientActionOp&);
    bool connectionComplete      (HClientAction*, const HClientActionOp&);
    bool getCurrentConnectionIDs (HClientAction*, const HClientActionOp&);
    bool getCurrentConnectionInfo(HClientAction*, const HClientActionOp&);
};

}
}
}

#endif /* HCONNECTIONMANAGER_P_H_ */
