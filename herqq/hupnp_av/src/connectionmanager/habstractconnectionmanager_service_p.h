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

#ifndef HABSTRACTCONNECTIONMANAGER_SERVICE_P_H_
#define HABSTRACTCONNECTIONMANAGER_SERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hserverservice_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
// Implementation details of HAbstractConnectionManagerService
//
class HAbstractConnectionManagerServicePrivate :
    public HServerServicePrivate
{
H_DECLARE_PUBLIC(HAbstractConnectionManagerService)
H_DISABLE_COPY(HAbstractConnectionManagerServicePrivate)

public:

    HAbstractConnectionManagerServicePrivate();
    virtual ~HAbstractConnectionManagerServicePrivate();

    qint32 getProtocolInfo(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 prepareForConnection(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 connectionComplete(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getCurrentConnectionIDs(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getCurrentConnectionInfo(
        const HActionArguments& inArgs, HActionArguments* outArgs);
};

}
}
}

#endif /* HABSTRACTCONNECTIONMANAGER_SERVICE_P_H_ */
