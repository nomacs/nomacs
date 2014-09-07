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

#include "hcreateobjectresult.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HCreateObjectResult::HCreateObjectResult() :
    m_objectId(), m_result()
{
}

HCreateObjectResult::HCreateObjectResult(
    const QString& objectId, const QString& result) :
        m_objectId(objectId.trimmed()), m_result(result.trimmed())
{
}

bool HCreateObjectResult::isValid() const
{
    return !m_objectId.isEmpty() && !m_result.isEmpty();
}

bool operator==(const HCreateObjectResult& obj1, const HCreateObjectResult& obj2)
{
    return obj1.objectId() == obj2.objectId() &&
           obj1.result() == obj2.result();
}

}
}
}
