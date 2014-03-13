/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hnt_p.h"

namespace Herqq
{

namespace Upnp
{

HNt::HNt() :
    m_typeValue(qMakePair(Type_Undefined   , QString(""))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
}

HNt::HNt(const QString& type) :
    m_typeValue(qMakePair(Type_Undefined, QString(""))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
    if (type.compare("upnp:event", Qt::CaseInsensitive) == 0)
    {
        m_typeValue.first  = Type_UpnpEvent;
        m_typeValue.second = "upnp:event";
    }
}

HNt::HNt(const QString& type, const QString& subtype) :
    m_typeValue(qMakePair(Type_Undefined, QString(""))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
    if (type.compare("upnp:event", Qt::CaseInsensitive) == 0)
    {
        m_typeValue.first  = Type_UpnpEvent;
        m_typeValue.second = "upnp:event";
    }

    if (subtype.compare("upnp:propchange", Qt::CaseInsensitive) == 0)
    {
        m_subTypeValue.first  = SubType_UpnpPropChange;
        m_subTypeValue.second = "upnp:propchange";
    }
}

HNt::HNt(Type type) :
    m_typeValue(qMakePair(type, toString(type))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
}

HNt::HNt(Type type, SubType subType) :
    m_typeValue(qMakePair(type, toString(type))),
    m_subTypeValue(qMakePair(subType, toString(subType)))
{
}

HNt::~HNt()
{
}

HNt& HNt::operator=(const QString& nt)
{
    HNt copy(nt);
    *this = copy;
    return *this;
}

QString HNt::typeToString() const
{
    return m_typeValue.second;
}

QString HNt::subTypeToString() const
{
    return m_subTypeValue.second;
}

QString HNt::toString(Type type)
{
    switch(type)
    {
    case Type_Undefined:
        return "";
    case Type_UpnpEvent:
        return "upnp:event";
    }

    return "";
}

QString HNt::toString(SubType subType)
{
    switch(subType)
    {
    case SubType_Undefined:
        return "";
    case SubType_UpnpPropChange:
        return "upnp:propchange";
    }

    return "";
}

}
}
