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

#ifndef HNT_H_
#define HNT_H_

#include <QtCore/QPair>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

//
//
//
class HNt
{
public:

    enum Type
    {
        Type_Undefined = 0,
        Type_UpnpEvent = 1
    };

    enum SubType
    {
        SubType_Undefined      = 0,
        SubType_UpnpPropChange = 1
    };

private:

    QPair<Type, QString>    m_typeValue;
    QPair<SubType, QString> m_subTypeValue;

public:

    HNt ();

    explicit HNt(const QString& type);
    explicit HNt(const QString& type, const QString& subTybe);
    explicit HNt(Type type);

    HNt(Type type, SubType subType);

    ~HNt();

    HNt& operator=(const QString& nt);

    QString typeToString() const;

    inline Type type() const
    {
        return m_typeValue.first;
    }

    QString subTypeToString() const;

    inline SubType subType() const
    {
        return m_subTypeValue.first;
    }

    static QString toString(Type type);
    static QString toString(SubType subType);
};


}
}

#endif /* HNT_H_ */
