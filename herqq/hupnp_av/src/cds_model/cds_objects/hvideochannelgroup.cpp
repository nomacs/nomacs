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

#include "hvideochannelgroup.h"
#include "hvideochannelgroup_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HVideoChannelGroupPrivate
 ******************************************************************************/
HVideoChannelGroupPrivate::HVideoChannelGroupPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HChannelGroupPrivate(clazz, cdsType)
{
}

/*******************************************************************************
 * HVideoChannelGroup
 ******************************************************************************/
HVideoChannelGroup::HVideoChannelGroup(const QString& clazz, CdsType cdsType) :
    HChannelGroup(*new HVideoChannelGroupPrivate(clazz, cdsType))
{
}

HVideoChannelGroup::HVideoChannelGroup(HVideoChannelGroupPrivate& dd) :
    HChannelGroup(dd)
{
}

HVideoChannelGroup::HVideoChannelGroup(
    const QString& title, const QString& parentId, const QString& id) :
        HChannelGroup(*new HVideoChannelGroupPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HVideoChannelGroup::~HVideoChannelGroup()
{
}

HVideoChannelGroup* HVideoChannelGroup::newInstance() const
{
    return new HVideoChannelGroup();
}

}
}
}
