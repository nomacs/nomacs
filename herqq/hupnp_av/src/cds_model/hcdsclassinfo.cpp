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

#include "hcdsclassinfo.h"

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HCdsClassInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HCdsClassInfoPrivate)

public:

    QString m_name;
    QString m_className;
    bool m_includeDerived;

    HCdsClassInfoPrivate() :
        m_name(), m_className(), m_includeDerived(true)
    {
    }
};

/*******************************************************************************
 * HCdsClassInfo
 *******************************************************************************/
HCdsClassInfo::HCdsClassInfo() :
    h_ptr(new HCdsClassInfoPrivate())
{
}

HCdsClassInfo::HCdsClassInfo(const QString& className) :
    h_ptr(new HCdsClassInfoPrivate())
{
    h_ptr->m_className = className.trimmed();
}

HCdsClassInfo::HCdsClassInfo(
    const QString& className, bool includeDerived, const QString& name) :
        h_ptr(new HCdsClassInfoPrivate())
{
    QString classNameTrimmed = className.trimmed();
    if (!classNameTrimmed.isEmpty())
    {
        h_ptr->m_className = classNameTrimmed;
        h_ptr->m_name = name.trimmed();
        h_ptr->m_includeDerived = includeDerived;
    }
}

HCdsClassInfo::~HCdsClassInfo()
{
}

HCdsClassInfo::HCdsClassInfo(const HCdsClassInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HCdsClassInfo& HCdsClassInfo::operator=(const HCdsClassInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

void HCdsClassInfo::setName(const QString& arg)
{
    h_ptr->m_name = arg.trimmed();
}

void HCdsClassInfo::setIncludeDerived(bool arg)
{
    h_ptr->m_includeDerived = arg;
}

bool HCdsClassInfo::isValid() const
{
    return !h_ptr->m_className.isEmpty();
}

QString HCdsClassInfo::className() const
{
    return h_ptr->m_className;
}

QString HCdsClassInfo::name() const
{
    return h_ptr->m_name;
}

bool HCdsClassInfo::includeDerived() const
{
    return h_ptr->m_includeDerived;
}

bool operator==(const HCdsClassInfo& obj1, const HCdsClassInfo& obj2)
{
    return obj1.h_ptr->m_className == obj2.h_ptr->m_className &&
           obj1.h_ptr->m_includeDerived == obj2.h_ptr->m_includeDerived &&
           obj1.h_ptr->m_name == obj2.h_ptr->m_name;
}

}
}
}
