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

#ifndef HAV_GLOBAL_P_H_
#define HAV_GLOBAL_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <QtCore/QList>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

template<typename T>
QString numToCsvString(const QList<T>& list)
{
    QString retVal;

    qint32 size = list.size();
    for(qint32 i = 0; i < size - 1; ++i)
    {
        retVal.append(QString::number(list[i])).append(',');
    }
    if (size > 0)
    {
        retVal.append(QString::number(list[size - 1]));
    }

    return retVal;
}

template<typename T>
QString strToCsvString(const T& collection)
{
    QString retVal;

    if (collection.isEmpty()) { return retVal; }

    typename T::const_iterator ci = collection.constBegin();
    for(; ci != collection.constEnd()-1; ++ci)
    {
        retVal.append(ci->toString()).append(',');
    }
    retVal.append(ci->toString());

    return retVal;
}

template<typename T>
QString strToCsvString(const QList<T>& list)
{
    QString retVal;

    qint32 size = list.size();
    for(qint32 i = 0; i < size - 1; ++i)
    {
        retVal.append(list[i].toString()).append(',');
    }
    if (size > 0)
    {
        retVal.append(list[size - 1].toString());
    }

    return retVal;
}

}
}
}

#endif /* HAV_GLOBAL_P_H_ */
