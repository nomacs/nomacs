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

#ifndef HSID_H_
#define HSID_H_

#include <QtCore/QUuid>
#include <QtCore/QString>

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
// Implementation note: this class cannot enforce the requirement of a valid UUID,
// since there are UPnP software that do not generate and use valid UUIDs.
// Because of this, the class "accepts" any string. However, validity can be checked
// with the isValid(). **Do NOT change the semantics of this class**
class HSid
{
friend quint32 qHash(const HSid& key);
friend bool operator==(const HSid&, const HSid&);

private:

    QUuid m_value;
    QString m_valueAsStr;

public:

    HSid();
    explicit HSid(const QUuid&);
    explicit HSid(const QString&);
    HSid(const HSid&);

    ~HSid();

    HSid& operator=(const HSid&);
    HSid& operator=(const QUuid&);
    HSid& operator=(const QString&);

    inline QUuid value() const
    {
        return m_value;
    }

    inline QString toString() const
    {
        return m_valueAsStr;
    }

    inline bool isValid()const
    {
        return !m_value.isNull();
    }

    inline bool isEmpty() const
    {
        return m_valueAsStr.isEmpty();
    }
};

bool operator==(const HSid&, const HSid&);
inline bool operator!=(const HSid& obj1, const HSid& obj2)
{
    return !(obj1 == obj2);
}

quint32 qHash(const HSid& key);

}
}

#endif /* HSID_H_ */
