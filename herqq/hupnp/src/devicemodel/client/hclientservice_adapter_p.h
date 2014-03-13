/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HCLIENTSERVICE_ADAPTER_P_H_
#define HCLIENTSERVICE_ADAPTER_P_H_

#include <HUpnpCore/private/hclientadapter_p.h>

namespace Herqq
{

namespace Upnp
{

class HClientServiceAdapter;

//
//
//
class H_UPNP_CORE_EXPORT HClientServiceAdapterPrivate :
    public HAbstractClientAdapterPrivate
{
H_DISABLE_COPY(HClientServiceAdapterPrivate)

public:

    HClientService* m_service;
    HClientServiceAdapter* q_ptr;
    QScopedPointer<HActionsSetupData> m_actionsSetupData;

    HClientServiceAdapterPrivate(const HResourceType&);
    virtual ~HClientServiceAdapterPrivate();

    HClientAction* getAction(const QString&, qint32* rc = 0) const;
    bool validate(HClientService*);
};

}
}

#endif /* HCLIENTSERVICE_ADAPTER_P_H_ */
