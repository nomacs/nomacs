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

#include "hsortinfo.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HSortModifier
 ******************************************************************************/
HSortModifier::HSortModifier() :
    m_typeAsString(toString(AscendingByValue)), m_type(AscendingByValue)
{
}

HSortModifier::HSortModifier(Type arg) :
    m_typeAsString(toString(arg)), m_type(arg)
{
}

HSortModifier::HSortModifier(const QString& arg) :
    m_typeAsString(), m_type()
{
    QString trimmed = arg.trimmed();
    m_typeAsString = trimmed;
    m_type = fromString(trimmed);
}

bool HSortModifier::ascending(bool* ok) const
{
	bool b = false, determined = false;
	switch(m_type)
	{
	case Undefined:
		break;
	case AscendingByTime:
	case AscendingByValue:
		b = true;
		determined = true;
		break;
	case DescendingByTime:
	case DescendingByValue:
		determined = true;
		break;
	case VendorDefined:
	default:
		if (m_typeAsString.endsWith('+'))
		{
			b = true;
			determined = true;
		}
		else if (m_typeAsString.endsWith('-'))
		{
			determined = true;
		}
		break;
	}

    if (ok)
	{
		*ok = determined;
	}

	return b;
}

QString HSortModifier::toString(Type type)
{
    QString retVal;
    switch(type)
    {
    case AscendingByValue:
        retVal = "+";
        break;
    case AscendingByTime:
        retVal = "TIME+";
        break;
    case DescendingByValue:
        retVal = "-";
        break;
    case DescendingByTime:
        retVal = "TIME-";
        break;
    default:
        break;
    }
    return retVal;
}

HSortModifier::Type HSortModifier::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare("+", Qt::CaseInsensitive) == 0)
    {
        retVal = AscendingByValue;
    }
    else if (type.compare("-", Qt::CaseInsensitive) == 0)
    {
        retVal = DescendingByValue;
    }
    else if (type.compare("TIME+", Qt::CaseInsensitive) == 0)
    {
        retVal = AscendingByTime;
    }
    else if (type.compare("TIME-", Qt::CaseInsensitive) == 0)
    {
        retVal = DescendingByTime;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HSortModifier& obj1, const HSortModifier& obj2)
{
    return obj1.toString() == obj2.toString();
}

/*******************************************************************************
 * HSortInfoPrivate
 ******************************************************************************/
class HSortInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HSortInfoPrivate)

public:

    QString m_property;
    HSortModifier m_modifier;
};

/*******************************************************************************
 * HSortInfo
 ******************************************************************************/
HSortInfo::HSortInfo() :
    h_ptr(new HSortInfoPrivate())
{
}

HSortInfo::HSortInfo(const QString& property, const HSortModifier& modifier) :
    h_ptr(new HSortInfoPrivate())
{
    h_ptr->m_property = property.trimmed();
    h_ptr->m_modifier = modifier;
}

HSortInfo::HSortInfo(const HSortInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HSortInfo& HSortInfo::operator =(const HSortInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HSortInfo::~HSortInfo()
{
}

bool HSortInfo::isValid() const
{
    return !property().isEmpty();
}

QString HSortInfo::property() const
{
    return h_ptr->m_property;
}

HSortModifier HSortInfo::sortModifier() const
{
    return h_ptr->m_modifier;
}

void HSortInfo::setProperty(const QString& arg)
{
    h_ptr->m_property = arg.trimmed();
}

void HSortInfo::setSortModifier(const HSortModifier& arg)
{
    h_ptr->m_modifier = arg;
}

bool operator==(const HSortInfo& obj1, const HSortInfo& obj2)
{
    return obj1.property() == obj2.property() &&
           obj1.sortModifier() == obj2.sortModifier();
}

}
}
}
