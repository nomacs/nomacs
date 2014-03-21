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

#include "hsid_p.h"

#include "../../utils/hmisc_utils_p.h"

namespace Herqq
{

namespace Upnp
{

HSid::HSid() :
    m_value(), m_valueAsStr()
{
}

HSid::HSid(const QUuid& sid) :
    m_value(sid), m_valueAsStr(
        QString("uuid:%1").arg(sid.toString().remove('{').remove('}')))
{
}

HSid::HSid(const HSid& other) :
    m_value(), m_valueAsStr()
{
    Q_ASSERT(&other != this);

    m_value = other.m_value;
    m_valueAsStr = other.m_valueAsStr;
}

HSid::HSid(const QString& sid) :
    m_value(), m_valueAsStr()
{
    QString tmp(sid.simplified());
    if (tmp.isEmpty())
    {
        // in essence, only "empty" strings are not acceptable. If UUIDs are not
        // enforced, there can be no "minimum" requirement for an "invalid" UUID.
        return;
    }
    else if (tmp.startsWith("uuid:", Qt::CaseInsensitive))
    {
        m_value = tmp.trimmed().mid(5);
        m_valueAsStr = tmp;
    }
    else
    {
        m_value = QUuid(tmp);
        m_valueAsStr = QString("uuid:%1").arg(tmp);
    }
}

HSid::~HSid()
{
}

HSid& HSid::operator=(const HSid& other)
{
    Q_ASSERT(&other != this);

    m_value = other.m_value;
    m_valueAsStr = other.m_valueAsStr;
    return *this;
}

HSid& HSid::operator=(const QString& other)
{
    HSid copy(other);
    *this = copy;
    return *this;
}

HSid& HSid::operator=(const QUuid& other)
{
    HSid copy(other);
    *this = copy;
    return *this;
}

bool operator==(const HSid& sid1, const HSid& sid2)
{
    return sid1.m_valueAsStr == sid2.m_valueAsStr;
}

quint32 qHash(const HSid& key)
{
    QByteArray data = key.toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
