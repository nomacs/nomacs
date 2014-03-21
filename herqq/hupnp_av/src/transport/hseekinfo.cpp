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

#include "hseekinfo.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HSeekMode
 ******************************************************************************/
HSeekMode::HSeekMode() :
    m_type(Unknown), m_typeAsString()
{
}

HSeekMode::HSeekMode(Type type) :
    m_type(type), m_typeAsString(toString(type))
{
}

HSeekMode::HSeekMode(const QString& arg) :
    m_type(), m_typeAsString()
{
    QString trimmed = arg.trimmed();
    m_type = fromString(trimmed);
    m_typeAsString = trimmed;
}

QString HSeekMode::toString() const
{
    return m_typeAsString;
}

QString HSeekMode::toString(Type type)
{
    QString retVal;
    switch(type)
    {
    case TrackNr:
        retVal = "TRACK_NR";
        break;
    case AbsTime:
        retVal = "ABS_TIME";
        break;
    case RelTime:
        retVal = "REL_TIME";
        break;
    case AbsCount:
        retVal = "ABS_COUNT";
        break;
    case RelCount:
        retVal = "REL_COUNT";
        break;
    case ChannelFreq:
        retVal = "CHANNEL_FREQ";
        break;
    case TapeIndex:
        retVal = "TAPE-INDEX";
        break;
    case RelTapeIndex:
        retVal = "REL_TAPE-INDEX";
        break;
    case Frame:
        retVal = "FRAME";
        break;
    case RelFrame:
        retVal = "REL_FRAME";
        break;
    case Unknown:
    case VendorDefined:
    default:
        break;
    }
    return retVal;
}

HSeekMode::Type HSeekMode::fromString(const QString& type)
{
    Type retVal = Unknown;
    if (type.compare("TRACK_NR", Qt::CaseInsensitive) == 0)
    {
        retVal = TrackNr;
    }
    else if (type.compare("ABS_TIME", Qt::CaseInsensitive) == 0)
    {
        retVal = AbsTime;
    }
    else if (type.compare("REL_TIME", Qt::CaseInsensitive) == 0)
    {
        retVal = RelTime;
    }
    else if (type.compare("ABS_COUNT", Qt::CaseInsensitive) == 0)
    {
        retVal = AbsCount;
    }
    else if (type.compare("REL_COUNT", Qt::CaseInsensitive) == 0)
    {
        retVal = RelCount;
    }
    else if (type.compare("CHANNEL_FREQ", Qt::CaseInsensitive) == 0)
    {
        retVal = ChannelFreq;
    }
    else if (type.compare("TAPE-INDEX", Qt::CaseInsensitive) == 0)
    {
        retVal = TapeIndex;
    }
    else if (type.compare("REL_TAPE-INDEX", Qt::CaseInsensitive) == 0)
    {
        retVal = RelTapeIndex;
    }
    else if (type.compare("FRAME", Qt::CaseInsensitive) == 0)
    {
        retVal = Frame;
    }
    else if (type.compare("REL_FRAME", Qt::CaseInsensitive) == 0)
    {
        retVal = RelFrame;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HSeekMode& obj1, const HSeekMode& obj2)
{
    return obj1.toString() == obj2.toString();
}

/*******************************************************************************
 * HSeekInfoPrivate
 ******************************************************************************/
class HSeekInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HSeekInfoPrivate)
public:

    HSeekMode m_unit;
    QString m_target;

    HSeekInfoPrivate() : m_unit(), m_target()
    {
    }
};

/*******************************************************************************
 * HSeekInfo
 ******************************************************************************/
HSeekInfo::HSeekInfo() :
    h_ptr(new HSeekInfoPrivate())
{
}

HSeekInfo::HSeekInfo(const HSeekMode& unit, const QString& target) :
    h_ptr(new HSeekInfoPrivate())
{
    h_ptr->m_unit = unit;
    h_ptr->m_target = target;
}

HSeekInfo::HSeekInfo(const HSeekInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

HSeekInfo& HSeekInfo::operator=(const HSeekInfo& other)
{
    Q_ASSERT(this != &other);
    h_ptr = other.h_ptr;
    return *this;
}

HSeekInfo::~HSeekInfo()
{
}

bool HSeekInfo::isValid() const
{
    return unit().isValid();
}

HSeekMode HSeekInfo::unit() const
{
    return h_ptr->m_unit;
}

QString HSeekInfo::target() const
{
    return h_ptr->m_target;
}

bool operator==(const HSeekInfo& obj1, const HSeekInfo& obj2)
{
    return obj1.target() == obj2.target() &&
           obj1.unit() == obj2.unit();
}

}
}
}
