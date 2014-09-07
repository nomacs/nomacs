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

#include "hrecordmediumwritestatus.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRecordMediumWriteStatus
 ******************************************************************************/
HRecordMediumWriteStatus::HRecordMediumWriteStatus() :
    m_typeAsString(toString(Unknown)), m_type(Unknown)
{
}

HRecordMediumWriteStatus::HRecordMediumWriteStatus(Type arg) :
    m_typeAsString(toString(arg)), m_type(arg)
{
}

HRecordMediumWriteStatus::HRecordMediumWriteStatus(const QString& arg) :
    m_typeAsString(), m_type()
{
    QString trimmed = arg.trimmed();
    m_typeAsString = trimmed;
    m_type = fromString(trimmed);
}

QString HRecordMediumWriteStatus::toString(Type type)
{
    QString retVal;
    switch(type)
    {
    case Unknown:
        retVal = "UNKNOWN";
        break;
    case Writable:
        retVal = "WRITABLE";
        break;
    case Protected:
        retVal = "PROTECTED";
        break;
    case NotWritable:
        retVal = "NOT_WRITABLE";
        break;
    case NotImplemented:
        retVal = "NOT_IMPLEMENTED";
        break;
    default:
        break;
    }
    return retVal;
}

HRecordMediumWriteStatus::Type HRecordMediumWriteStatus::fromString(
    const QString& type)
{
    Type retVal = Unknown;
    if (type.compare("WRITABLE", Qt::CaseInsensitive) == 0)
    {
        retVal = Writable;
    }
    else if (type.compare("PROTECTED", Qt::CaseInsensitive) == 0)
    {
        retVal = Protected;
    }
    else if (type.compare("NOT_WRITABLE", Qt::CaseInsensitive) == 0)
    {
        retVal = NotWritable;
    }
    else if (type.compare("NOT_IMPLEMENTED", Qt::CaseInsensitive) == 0)
    {
        retVal = NotImplemented;
    }
    else if (type.compare("UNKNOWN", Qt::CaseInsensitive) != 0 && !type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HRecordMediumWriteStatus& obj1, const HRecordMediumWriteStatus& obj2)
{
    return obj1.toString() == obj2.toString();
}

}
}
}
