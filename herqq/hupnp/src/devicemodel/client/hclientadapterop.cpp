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

#include "hclientadapterop.h"
#include "../hasyncop_p.h"

namespace Herqq
{

namespace Upnp
{

class HAbstractClientAdapterOpPrivate :
    public HAsyncOpPrivate
{
public:

    HAbstractClientAdapterOpPrivate() :
        HAsyncOpPrivate(genId())
    {
    }
};

HAbstractClientAdapterOp::HAbstractClientAdapterOp() :
    HAsyncOp(*new HAbstractClientAdapterOpPrivate())
{
}

HAbstractClientAdapterOp::HAbstractClientAdapterOp(
    qint32 returnCode, const QString& errorDescription) :
        HAsyncOp(returnCode, errorDescription, *new HAbstractClientAdapterOpPrivate())
{
}

HAbstractClientAdapterOp::HAbstractClientAdapterOp(
    const HAbstractClientAdapterOp& other) :
        HAsyncOp(other)
{
}

HAbstractClientAdapterOp& HAbstractClientAdapterOp::operator=(
    const HAbstractClientAdapterOp& other)
{
    return static_cast<HAbstractClientAdapterOp&>(HAsyncOp::operator=(other));
}

}
}
