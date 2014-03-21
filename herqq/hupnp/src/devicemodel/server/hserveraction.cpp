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

#include "hserveraction.h"
#include "hserveraction_p.h"
#include "hdefault_serveraction_p.h"

#include "../hactionarguments.h"

#include "../../general/hlogger_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServerActionPrivate
 ******************************************************************************/
HServerActionPrivate::HServerActionPrivate() :
    q_ptr(0), m_info(), m_actionInvoke()
{
}

HServerActionPrivate::~HServerActionPrivate()
{
}

bool HServerActionPrivate::setInfo(const HActionInfo& info)
{
    if (!info.isValid())
    {
        return false;
    }

    m_info.reset(new HActionInfo(info));
    return true;
}

/*******************************************************************************
 * HServerAction
 ******************************************************************************/
HServerAction::HServerAction(const HActionInfo& info, HServerService* parent) :
    QObject(reinterpret_cast<QObject*>(parent)),
        h_ptr(new HServerActionPrivate())
{
    Q_ASSERT_X(parent, H_AT, "Parent service must be defined.");
    Q_ASSERT_X(info.isValid(), H_AT, "Action information must be defined.");

    h_ptr->m_info.reset(new HActionInfo(info));
    h_ptr->q_ptr = this;
}

HServerAction::~HServerAction()
{
    delete h_ptr;
}

HServerService* HServerAction::parentService() const
{
    return reinterpret_cast<HServerService*>(parent());
}

const HActionInfo& HServerAction::info() const
{
    return *h_ptr->m_info;
}

qint32 HServerAction::invoke(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    *outArgs = h_ptr->m_info->outputArguments();
    return h_ptr->m_actionInvoke(inArgs, outArgs);
}

/*******************************************************************************
 * HDefaultServerAction
 ******************************************************************************/
HDefaultServerAction::HDefaultServerAction(
    const HActionInfo& info, const HActionInvoke& inv, HServerService* parent) :
        HServerAction(info, parent)
{
    Q_ASSERT(inv);
    h_ptr->m_actionInvoke = inv;
}

}
}
