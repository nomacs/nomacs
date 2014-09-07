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

#include "hvideoprogram.h"
#include "hvideoprogram_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HVideoProgramPrivate
 ******************************************************************************/
HVideoProgramPrivate::HVideoProgramPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HEpgItemPrivate(clazz, cdsType)
{
}

/*******************************************************************************
 * HVideoProgram
 ******************************************************************************/
HVideoProgram::HVideoProgram(const QString& clazz, CdsType cdsType) :
    HEpgItem(*new HVideoProgramPrivate(clazz, cdsType))
{
}

HVideoProgram::HVideoProgram(HVideoProgramPrivate& dd) :
    HEpgItem(dd)
{
}

HVideoProgram::HVideoProgram(
    const QString& title, const QString& parentId, const QString& id) :
        HEpgItem(*new HVideoProgramPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HVideoProgram::~HVideoProgram()
{
}

HVideoProgram* HVideoProgram::newInstance() const
{
    return new HVideoProgram();
}

}
}
}
