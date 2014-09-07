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

#include "hclientactionop.h"
#include "hclientactionop_p.h"
#include "hclientaction_p.h"

#include "../hasyncop.h"
#include "../hactionarguments.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientActionOpPrivate
 *******************************************************************************/
HClientActionOpPrivate::HClientActionOpPrivate() :
    HAsyncOpPrivate(HAsyncOpPrivate::genId()),
        m_inArgs(), m_outArgs(), m_runner(0)
{
}

HClientActionOpPrivate::~HClientActionOpPrivate()
{
}

/*******************************************************************************
 * HClientActionOp
 *******************************************************************************/
HClientActionOp::HClientActionOp() :
    HAsyncOp(*new HClientActionOpPrivate())
{
}

HClientActionOp::HClientActionOp(
    qint32 returnCode, const QString& errorDescription) :
        HAsyncOp(returnCode, errorDescription, *new HClientActionOpPrivate())
{
}

HClientActionOp::HClientActionOp(const HActionArguments& inArgs) :
    HAsyncOp(*new HClientActionOpPrivate())
{
    H_D(HClientActionOp);
    h->m_inArgs = inArgs;
}

HClientActionOp::HClientActionOp(const HClientActionOp& other) :
    HAsyncOp(other)
{
}

HClientActionOp::~HClientActionOp()
{
}

void HClientActionOp::abort()
{
    H_D(HClientActionOp);
    if (h->m_runner)
    {
        h->m_runner->abort(id());
    }
}

HClientActionOp& HClientActionOp::operator=(const HClientActionOp& other)
{
    Q_ASSERT(&other != this);
    HAsyncOp::operator=(other);
    return *this;
}

const HActionArguments& HClientActionOp::inputArguments() const
{
    const H_D(HClientActionOp);
    return h->m_inArgs;
}

const HActionArguments& HClientActionOp::outputArguments() const
{
    const H_D(HClientActionOp);
    return h->m_outArgs;
}

void HClientActionOp::setOutputArguments(const HActionArguments& outArgs)
{
    H_D(HClientActionOp);
    h->m_outArgs = outArgs;
}

}
}
