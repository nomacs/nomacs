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

#include "hprepareforconnection_result.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPrepareForConnectionResult
 ******************************************************************************/
HPrepareForConnectionResult::HPrepareForConnectionResult() :
    m_connectionId(-1), m_avTransportId(-1), m_rcsId(-1)
{
}

HPrepareForConnectionResult::HPrepareForConnectionResult(
    qint32 connectionId, qint32 avTransportId, qint32 rcsId) :
        m_connectionId(-1), m_avTransportId(-1), m_rcsId(-1)
{
    setConnectionId(connectionId);
    setAvTransportId(avTransportId);
    setRcsId(rcsId);
}

bool HPrepareForConnectionResult::isValid() const
{
    return m_connectionId >= 0;
}

void HPrepareForConnectionResult::setConnectionId(qint32 arg)
{
    if (arg < -1) { arg = -1; }
    m_connectionId = arg;
}

void HPrepareForConnectionResult::setAvTransportId(qint32 arg)
{
    if (arg < -1) { arg = -1; }
    m_avTransportId = arg;
}

void HPrepareForConnectionResult::setRcsId(qint32 arg)
{
    if (arg < -1) { arg = -1; }
    m_rcsId = arg;
}

bool operator==(const HPrepareForConnectionResult& obj1, const HPrepareForConnectionResult& obj2)
{
    return obj1.avTransportId() == obj2.avTransportId() &&
           obj1.connectionId() == obj2.connectionId() &&
           obj1.rcsId() == obj2.rcsId();
}

}
}
}
