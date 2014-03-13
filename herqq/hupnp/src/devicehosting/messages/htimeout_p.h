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

#ifndef HTIMEOUT_H_
#define HTIMEOUT_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <QtCore/QtGlobal>

class QString;

namespace Herqq
{

namespace Upnp
{

//
//
//
class HTimeout
{
friend bool operator==(const HTimeout&, const HTimeout&);

private:

    qint32 m_value;

public:

    HTimeout();
    explicit HTimeout(qint32);
    explicit HTimeout(const QString&);

    ~HTimeout();

    HTimeout& operator=(qint32);
    HTimeout& operator=(const QString&);

    inline qint32 value() const
    {
        return m_value;
    }

    QString toString() const;

    inline bool isInfinite() const
    {
        return m_value == -1;
    }
};

bool operator==(const HTimeout&, const HTimeout&);
inline bool operator!=(const HTimeout& obj1, const HTimeout& obj2)
{
    return !(obj1 == obj2);
}

}
}

#endif /* HTIMEOUT_H_ */
