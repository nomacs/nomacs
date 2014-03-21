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

#include "hrecordqualitymode.h"

#include <HUpnpCore/private/hmisc_utils_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRecordQualityMode
 ******************************************************************************/
HRecordQualityMode::HRecordQualityMode() :
    m_mode(Undefined), m_modeAsString()
{
}

HRecordQualityMode::HRecordQualityMode(Type mode) :
    m_mode(mode), m_modeAsString(toString(mode))
{
}

HRecordQualityMode::HRecordQualityMode(const QString& mode) :
    m_mode(fromString(mode)), m_modeAsString(mode)
{
    QString trimmed = mode.trimmed();
    m_mode = fromString(trimmed);
    m_modeAsString = trimmed;
}

QString HRecordQualityMode::toString(Type mode)
{
    QString retVal;
    switch(mode)
    {
    case Ep:
        retVal = "0:EP";
        break;
    case Lp:
        retVal = "1:LP";
        break;
    case Sp:
        retVal = "2:SP";
        break;
    case Basic:
        retVal = "0:BASIC";
        break;
    case Medium:
        retVal = "1:MEDIUM";
        break;
    case High:
        retVal = "2:HIGH";
        break;
    case NotImplemented:
        retVal = "NOT_IMPLEMENTED";
        break;
    default:
        break;
    }
    return retVal;
}

HRecordQualityMode::Type HRecordQualityMode::fromString(const QString& mode)
{
    Type retVal = Undefined;
    if (mode.compare("0:EP", Qt::CaseInsensitive) == 0)
    {
        retVal = Ep;
    }
    else if (mode.compare("1:LP", Qt::CaseInsensitive) == 0)
    {
        retVal = Lp;
    }
    else if (mode.compare("2:SP", Qt::CaseInsensitive) == 0)
    {
        retVal = Sp;
    }
    else if (mode.compare("0:BASIC", Qt::CaseInsensitive) == 0)
    {
        retVal = Basic;
    }
    else if (mode.compare("1:MEDIUM", Qt::CaseInsensitive) == 0)
    {
        retVal = Medium;
    }
    else if (mode.compare("2:HIGH", Qt::CaseInsensitive) == 0)
    {
        retVal = High;
    }
    else if (mode.compare("NOT_IMPLEMENTED", Qt::CaseInsensitive) == 0)
    {
        retVal = NotImplemented;
    }
    else if (!mode.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HRecordQualityMode& obj1, const HRecordQualityMode& obj2)
{
    return obj1.toString() == obj2.toString();
}

quint32 qHash(const HRecordQualityMode& key)
{
    QString str = key.toString();
    return hash(key.toString().toLocal8Bit(), str.size());
}

}
}
}
