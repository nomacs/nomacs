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

#include "haudioprogram.h"
#include "haudioprogram_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../../common/hradioband.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAudioProgramPrivate
 ******************************************************************************/
HAudioProgramPrivate::HAudioProgramPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HEpgItemPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_radioCallSign));
    insert(inst.get(HCdsProperties::upnp_radioStationID));
    insert(inst.get(HCdsProperties::upnp_radioBand));
}

/*******************************************************************************
 * HAudioProgram
 ******************************************************************************/
HAudioProgram::HAudioProgram(const QString& clazz, CdsType cdsType) :
    HEpgItem(*new HAudioProgramPrivate(clazz, cdsType))
{
}

HAudioProgram::HAudioProgram(HAudioProgramPrivate& dd) :
    HEpgItem(dd)
{
}

HAudioProgram::HAudioProgram(
    const QString& title, const QString& parentId, const QString& id) :
        HEpgItem(*new HAudioProgramPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HAudioProgram::~HAudioProgram()
{
}

HAudioProgram* HAudioProgram::newInstance() const
{
    return new HAudioProgram();
}

void HAudioProgram::setRadioCallSign(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioCallSign, arg);
}

void HAudioProgram::setRadioStationId(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioStationID, arg);
}

void HAudioProgram::setRadioBand(const HRadioBand& arg)
{
    setCdsProperty(HCdsProperties::upnp_radioBand, QVariant::fromValue(arg));
}

QString HAudioProgram::radioCallSign() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioCallSign, &value);
    return value.toString();
}

QString HAudioProgram::radioStationId() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioStationID, &value);
    return value.toString();
}

HRadioBand HAudioProgram::radioBand() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_radioBand, &value);
    return value.value<HRadioBand>();
}

}
}
}
