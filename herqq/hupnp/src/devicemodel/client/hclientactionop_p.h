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

#ifndef HCLIENTACTIONOP_P_H_
#define HCLIENTACTIONOP_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hasyncop_p.h>
#include <HUpnpCore/HActionArguments>

#include <QtCore/QPointer>

namespace Herqq
{

namespace Upnp
{

class HClientActionPrivate;

class HClientActionOpPrivate :
    public HAsyncOpPrivate
{
public:

    HActionArguments m_inArgs, m_outArgs;
    QPointer<HClientActionPrivate> m_runner;

public:

    HClientActionOpPrivate();
    virtual ~HClientActionOpPrivate();
};

}
}

#endif /* HCLIENTACTIONOP_P_H_ */
