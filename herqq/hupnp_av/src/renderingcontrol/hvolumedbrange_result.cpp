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

#include "hvolumedbrange_result.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HVolumeDbRangeResult::HVolumeDbRangeResult() :
    m_minValue(0), m_maxValue(0)
{
}

HVolumeDbRangeResult::HVolumeDbRangeResult(qint16 min, qint16 max) :
    m_minValue(min), m_maxValue(max)
{
}

bool operator==(
    const HVolumeDbRangeResult& obj1, const HVolumeDbRangeResult& obj2)
{
    return obj1.minValue() == obj2.minValue() &&
           obj2.maxValue() == obj2.maxValue();
}

}
}
}
