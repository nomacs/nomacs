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

#ifndef HCLIENTSERVICE_P_H_
#define HCLIENTSERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../hservice_p.h"

namespace Herqq
{

namespace Upnp
{

class HDefaultClientStateVariable;

//
// Implementation details of HClientService
//
class HClientServicePrivate :
    public HServicePrivate<HClientService, HClientAction, HDefaultClientStateVariable>
{
H_DECLARE_PUBLIC(HClientService)
H_DISABLE_COPY(HClientServicePrivate)

public: // attributes

    QHash<QString, const HClientStateVariable*> m_stateVariablesConst;

public: // methods

    HClientServicePrivate();

    virtual ~HClientServicePrivate();
    bool addStateVariable(HDefaultClientStateVariable*);

    ReturnValue updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);
};

}
}

#endif /* HCLIENTSERVICE_P_H_ */
