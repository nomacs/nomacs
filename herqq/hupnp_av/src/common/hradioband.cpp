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

#include "hradioband.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRadioBand
 ******************************************************************************/
HRadioBand::HRadioBand() :
    m_typeAsString(), m_type(Undefined)
{
}

HRadioBand::HRadioBand(Type arg) :
    m_typeAsString(toString(arg)), m_type(arg)
{
}

HRadioBand::HRadioBand(const QString& arg) :
    m_typeAsString(arg), m_type(fromString(arg))
{
    QString trimmed = arg.trimmed();
    m_typeAsString = trimmed;
    m_type = fromString(trimmed);
}

QString HRadioBand::toString(Type type)
{
    QString retVal;
    switch (type)
    {
    case AM:
        retVal = "AM";
        break;
    case FM:
        retVal = "FM";
        break;
    case ShortWave:
        retVal = "ShortWave";
        break;
    case Internet:
        retVal = "Internet";
        break;
    case Satellite:
        retVal = "Satellite";
        break;
    case VendorDefined:
        retVal = "VendorDefined";
        break;
    default:
        break;
    }
    return retVal;
}

HRadioBand::Type HRadioBand::fromString(const QString& arg)
{
    Type retVal = Undefined;
    if (arg.compare("am", Qt::CaseInsensitive) == 0)
    {
        retVal = AM;
    }
    else if (arg.compare("fm", Qt::CaseInsensitive) == 0)
    {
        retVal = FM;
    }
    else if (arg.compare("shortwave", Qt::CaseInsensitive) == 0)
    {
        retVal = ShortWave;
    }
    else if (arg.compare("internet", Qt::CaseInsensitive) == 0)
    {
        retVal = Internet;
    }
    else if (arg.compare("satellite", Qt::CaseInsensitive) == 0)
    {
        retVal = Satellite;
    }
    else if (!arg.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HRadioBand& obj1, const HRadioBand& obj2)
{
    return obj1.toString() == obj2.toString();
}

}
}
}
