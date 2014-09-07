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

#ifndef HOBJECT_P_H_
#define HOBJECT_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpAv/HObject>
#include <HUpnpAv/HCdsPropertyInfo>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QLinkedList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

template<typename T>
QList<QVariant> toList(const QList<T>& values)
{
    QList<QVariant> retVal;
    foreach(const T& value, values)
    {
        retVal.append(QVariant::fromValue(value));
    }
    return retVal;
}

template<typename T>
QList<T> toList(const QList<QVariant>& values)
{
    QList<T> retVal;
    foreach(const QVariant& value, values)
    {
        retVal.append(value.value<T>());
    }
    return retVal;
}

//
//
//
class HObjectPrivate
{
H_DISABLE_COPY(HObjectPrivate)

public:

    QHash<QString, QVariant> m_properties;
    HObject::CdsType m_cdsType;
    QLinkedList<QString> m_disabledProperties;

    HObjectPrivate(const QString& clazz, HObject::CdsType cdsType);
    virtual ~HObjectPrivate();

    inline void insert(const HCdsPropertyInfo& arg)
    {
        m_properties.insert(arg.name(), arg.defaultValue());
    }

    inline void insert(const QString& arg, const QVariant& var)
    {
        m_properties.insert(arg, var);
    }
};

}
}
}

#endif /* HOBJECT_P_H_ */
