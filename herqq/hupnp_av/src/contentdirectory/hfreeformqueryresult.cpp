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

#include "hfreeformqueryresult.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HFreeFormQueryResult::HFreeFormQueryResult() :
    m_queryResult(), m_updateId(0)
{
}

HFreeFormQueryResult::HFreeFormQueryResult(
    const QString& queryResult, quint32 updateId) :
        m_queryResult(queryResult), m_updateId(updateId)
{
}

bool HFreeFormQueryResult::isValid() const
{
    return !m_queryResult.isEmpty();
}

bool operator==(
    const HFreeFormQueryResult& obj1, const HFreeFormQueryResult& obj2)
{
    return obj1.queryResult() == obj2.queryResult() &&
           obj1.updateId() == obj2.updateId();
}

}
}
}
