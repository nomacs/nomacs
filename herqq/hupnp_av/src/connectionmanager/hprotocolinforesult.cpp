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

#include "hprotocolinforesult.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HProtocolInfoResult::HProtocolInfoResult() :
    m_source(), m_sink()
{
}

HProtocolInfoResult::HProtocolInfoResult(
    const HProtocolInfos& source, const HProtocolInfos& sink) :
        m_source(source), m_sink(sink)
{
}

bool HProtocolInfoResult::isEmpty() const
{
    return m_source.isEmpty() && m_sink.isEmpty();
}

bool HProtocolInfoResult::setSource(const HProtocolInfos& arg)
{
    foreach(const HProtocolInfo& info, arg)
    {
        if (!info.isValid())
        {
            return false;
        }
    }
    m_source = arg;
    return true;
}

bool HProtocolInfoResult::setSink(const HProtocolInfos& arg)
{
    foreach(const HProtocolInfo& info, arg)
    {
        if (!info.isValid())
        {
            return false;
        }
    }
    m_sink = arg;
    return true;
}

bool operator==(const HProtocolInfoResult& obj1, const HProtocolInfoResult& obj2)
{
    return obj1.sink() == obj2.sink() &&
           obj1.source() == obj2.source();
}

}
}
}
