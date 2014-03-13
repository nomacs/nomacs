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

#include "hsearchresult.h"

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HSearchResultPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HSearchResultPrivate)

public:

    QString m_result;
    quint32 m_numberReturned;
    quint32 m_totalMatches;
    quint32 m_updateId;

    HSearchResultPrivate() :
        m_result(), m_numberReturned(0), m_totalMatches(0), m_updateId(0)
    {
    }

    HSearchResultPrivate(
        const QString& result, quint32 numberReturned, quint32 totalMatches,
        quint32 updateId) :
            m_result(result), m_numberReturned(numberReturned),
            m_totalMatches(totalMatches), m_updateId(updateId)
    {
    }
};

HSearchResult::HSearchResult() :
    h_ptr(new HSearchResultPrivate())
{
}

HSearchResult::HSearchResult(
    const QString& result, quint32 numberReturned, quint32 totalMatches,
    quint32 updateId) :
        h_ptr(new HSearchResultPrivate(result, numberReturned, totalMatches, updateId))
{
}

HSearchResult::HSearchResult(const HSearchResult& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

HSearchResult& HSearchResult::operator=(const HSearchResult& other)
{
    Q_ASSERT(this != &other);
    h_ptr = other.h_ptr;
    return *this;
}

HSearchResult::~HSearchResult()
{
}

QString HSearchResult::result() const
{
    return h_ptr->m_result;
}

quint32 HSearchResult::numberReturned() const
{
    return h_ptr->m_numberReturned;
}

quint32 HSearchResult::totalMatches() const
{
    return h_ptr->m_totalMatches;
}

quint32 HSearchResult::updateId() const
{
    return h_ptr->m_updateId;
}

bool operator==(const HSearchResult& obj1, const HSearchResult& obj2)
{
    return obj1.numberReturned() == obj2.numberReturned() &&
           obj1.result() == obj2.result() &&
           obj1.totalMatches() == obj2.totalMatches() &&
           obj1.updateId() == obj2.updateId();
}

}
}
}
