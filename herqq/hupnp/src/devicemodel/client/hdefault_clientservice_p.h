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

#ifndef HDEFAULT_CLIENTSERVICE_P_H_
#define HDEFAULT_CLIENTSERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HClientService>

#include <QtCore/QPair>

namespace Herqq
{

namespace Upnp
{

class HDefaultClientDevice;
class HDefaultClientStateVariable;

//
// Default implementation of HClientService
//
class HDefaultClientService :
    public HClientService
{
H_DISABLE_COPY(HDefaultClientService)

public:

    HDefaultClientService(const HServiceInfo&, HDefaultClientDevice* parentDevice);
    void addAction(HClientAction*);
    void addStateVariable(HDefaultClientStateVariable*);
    void setDescription(const QString& description);

    bool updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);

    HDefaultClientDevice* parentDevice() const;
};

}
}

#endif /* HDEFAULT_CLIENTSERVICE_P_H_ */
