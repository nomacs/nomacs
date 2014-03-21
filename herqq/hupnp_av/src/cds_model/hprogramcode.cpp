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

#include "hprogramcode.h"

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HProgramCode>("Herqq::Upnp::Av::HProgramCode");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HProgramCodePrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HProgramCodePrivate)
public:
    QString m_value, m_type;
};

HProgramCode::HProgramCode() :
    h_ptr(new HProgramCodePrivate())
{
}

HProgramCode::HProgramCode(const QString& value, const QString& type) :
    h_ptr(new HProgramCodePrivate())
{
    QString valueTrimmed = value.trimmed();
    QString typeTrimmed = type.trimmed();
    if (!valueTrimmed.isEmpty())
    {
        int index = typeTrimmed.indexOf("_");
        if (index > 3 && !typeTrimmed.mid(index+1).isEmpty())
        {
            // You really cannot express a valid domain in less than four characters.
            h_ptr->m_value = valueTrimmed;
            h_ptr->m_type = typeTrimmed;
        }
    }
}

HProgramCode::~HProgramCode()
{
}

HProgramCode::HProgramCode(const HProgramCode& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HProgramCode& HProgramCode::operator =(const HProgramCode& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

bool HProgramCode::isValid() const
{
    return !value().isEmpty();
}

QString HProgramCode::value() const
{
    return h_ptr->m_value;
}

QString HProgramCode::type() const
{
    return h_ptr->m_type;
}

bool operator==(const HProgramCode& obj1, const HProgramCode& obj2)
{
    return obj1.value() == obj2.value() &&
           obj2.type() == obj2.type();
}

}
}
}

