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

#ifndef HSERVERSERVICE_P_H_
#define HSERVERSERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hservice_p.h>

#include <HUpnpCore/HServerAction>
#include <HUpnpCore/HServerService>
#include <HUpnpCore/HServerStateVariable>

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HServerService
//
class H_UPNP_CORE_EXPORT HServerServicePrivate :
    public HServicePrivate<HServerService, HServerAction, HServerStateVariable>
{
H_DECLARE_PUBLIC(HServerService)
H_DISABLE_COPY(HServerServicePrivate)

public: // methods

    HServerServicePrivate();
    virtual ~HServerServicePrivate();

    ReturnValue updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);
};

}
}

#endif /* HSERVERSERVICE_P_H_ */
