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

#include "hresourcetype.h"
#include "../utils/hmisc_utils_p.h"

#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

HResourceType::HResourceType() :
    m_type(Undefined), m_resourceElements()
{
}

HResourceType::HResourceType(const QString& resourceTypeAsStr) :
    m_type(Undefined), m_resourceElements()
{
    qint32 flags = 0;

    QStringList tmp = resourceTypeAsStr.simplified().split(":");
    if (tmp.size() != 5)
    {
        return;
    }

    if (tmp[0] != "urn")
    {
        return;
    }

    tmp[1] = tmp[1].simplified();
    if (tmp[1].isEmpty())
    {
        return;
    }
    if (tmp[1].compare("schemas-upnp-org") != 0)
    {
        flags = 0x01;
        tmp[1] = tmp[1].replace('.', '-');
    }
    else
    {
        flags = 0x02;
    }

    tmp[2] = tmp[2].simplified();
    if (tmp[2].compare("device") == 0)
    {
        flags |= 0x04;
    }
    else if (tmp[2].compare("service") == 0)
    {
        flags |= 0x08;
    }
    else
    {
        return;
    }

    tmp[3] = tmp[3].simplified();
    if (tmp[3].isEmpty())
    {
        return;
    }

    bool ok = false;
    tmp[4].toInt(&ok);
    if (!ok)
    {
        return;
    }

    switch(flags)
    {
    case 0x05:
        m_type = VendorSpecifiedDeviceType;
        break;
    case 0x06:
        m_type = StandardDeviceType;
        break;
    case 0x09:
        m_type = VendorSpecifiedServiceType;
        break;
    case 0x0a:
        m_type = StandardServiceType;
        break;
    default:
        Q_ASSERT(false);
        return;
    }

    m_resourceElements = tmp;
}

HResourceType::~HResourceType()
{
}

QString HResourceType::toString(Tokens tokens) const
{
    if (!isValid())
    {
        return QString();
    }

    QString retVal;
    bool appendDelim = false;
    if (tokens & UrnPrefix)
    {
        retVal.append("urn:");
    }

    if (tokens & Domain)
    {
        retVal.append(m_resourceElements[1]);
        appendDelim = true;
    }

    if (tokens & Type)
    {
        if (appendDelim) { retVal.append(':'); }
        retVal.append(m_resourceElements[2]);
        appendDelim = true;
    }

    if (tokens & TypeSuffix)
    {
        if (appendDelim) { retVal.append(':'); }
        retVal.append(m_resourceElements[3]);
        appendDelim = true;
    }

    if (tokens & Version)
    {
        if (appendDelim) { retVal.append(':'); }
        retVal.append(m_resourceElements[4]);
    }

    return retVal;
}

qint32 HResourceType::version() const
{
    if (!isValid())
    {
        return -1;
    }

    return m_resourceElements[4].toInt();
}

bool HResourceType::compare(
    const HResourceType& other, VersionMatch versionMatch) const
{
    if (other.isValid() != isValid())
    {
        return false;
    }
    else if (!isValid())
    {
        return true;
    }

    switch (versionMatch)
    {
    case Ignore:
        break;
    case Exact:
        if (other.version() != version())
        {
            return false;
        }
        break;
    case Inclusive:
        if (version() > other.version())
        {
            return false;
        }
        break;
    case EqualOrGreater:
        if (version() < other.version())
        {
            return false;
        }
        break;
    default:
        Q_ASSERT(false);
    }

    for(qint32 i = 0; i < m_resourceElements.size() - 1; ++i)
    {
        if (m_resourceElements[i] != other.m_resourceElements[i])
        {
            return false;
        }
    }

    return true;
}

bool operator==(const HResourceType& arg1, const HResourceType& arg2)
{
    return arg1.m_resourceElements == arg2.m_resourceElements;
}

quint32 qHash(const HResourceType& key)
{
    QByteArray data = key.toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
