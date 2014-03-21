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

#include "htransferprogressinfo.h"

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HTransferProgressInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HTransferProgressInfoPrivate)

public:
    quint32 m_length;
    HTransferProgressInfo::Status m_status;
    quint32 m_total;

    HTransferProgressInfoPrivate() :
        m_length(0), m_status(HTransferProgressInfo::Error), m_total(0)
    {
    }

    HTransferProgressInfoPrivate(
        quint32 length, HTransferProgressInfo::Status status, quint32 total) :
            m_length(length), m_status(status), m_total(total)
    {
    }
};

HTransferProgressInfo::HTransferProgressInfo() :
    h_ptr(new HTransferProgressInfoPrivate())
{
}

HTransferProgressInfo::HTransferProgressInfo(
    quint32 length, Status status, quint32 total) :
        h_ptr(new HTransferProgressInfoPrivate(length, status, total))
{
}

HTransferProgressInfo::HTransferProgressInfo(const HTransferProgressInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

HTransferProgressInfo& HTransferProgressInfo::operator=(const HTransferProgressInfo& other)
{
    Q_ASSERT(this != &other);
    h_ptr = other.h_ptr;
    return *this;
}

HTransferProgressInfo::~HTransferProgressInfo()
{
}

HTransferProgressInfo::Status HTransferProgressInfo::fromString(const QString& arg)
{
    Status retVal = Error;
    if (arg.compare("IN_PROGRESS", Qt::CaseInsensitive) == 0)
    {
        retVal = InProgress;
    }
    else if (arg.compare("STOPPED", Qt::CaseInsensitive) == 0)
    {
        retVal = Stopped;
    }
    else if (arg.compare("COMPLETED", Qt::CaseInsensitive) == 0)
    {
        retVal = Completed;
    }
    return retVal;
}

bool HTransferProgressInfo::isEmpty() const
{
    return !h_ptr->m_length && !h_ptr->m_status && !h_ptr->m_total;
}

quint32 HTransferProgressInfo::length() const
{
    return h_ptr->m_length;
}

HTransferProgressInfo::Status HTransferProgressInfo::status() const
{
    return h_ptr->m_status;
}

quint32 HTransferProgressInfo::total() const
{
    return h_ptr->m_total;
}

bool operator==(
    const HTransferProgressInfo& obj1, const HTransferProgressInfo& obj2)
{
    return obj1.length() == obj2.length() &&
           obj1.status() == obj2.status() &&
           obj1.total() == obj2.total();
}

}
}
}
