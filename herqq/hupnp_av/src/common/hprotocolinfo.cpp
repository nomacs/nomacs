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

#include "hprotocolinfo.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HProtocolInfoPrivate
 ******************************************************************************/
class HProtocolInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HProtocolInfoPrivate)

public:

    QString m_protocol, m_network, m_contentFormat, m_additionalInfo;

    HProtocolInfoPrivate() :
        m_protocol(), m_network(), m_contentFormat(), m_additionalInfo()
    {
    }
};

/*******************************************************************************
 * HProtocolInfo
 ******************************************************************************/
HProtocolInfo::HProtocolInfo() :
    h_ptr(new HProtocolInfoPrivate())
{
}

HProtocolInfo::HProtocolInfo(const QString& arg) :
    h_ptr(new HProtocolInfoPrivate())
{
    QStringList tmp = arg.split(":");
    if (tmp.size() != 4)
    {
        return;
    }

    setProtocol(tmp[0]);
    setNetwork(tmp[1]);
    setContentFormat(tmp[2]);
    setAdditionalInfo(tmp[3]);
}

HProtocolInfo::HProtocolInfo(
    const QString& protocol, const QString& network,
    const QString& contentFormat, const QString& additionalInfo) :
        h_ptr(new HProtocolInfoPrivate())
{
    setProtocol(protocol);
    setNetwork(network);
    setContentFormat(contentFormat);
    setAdditionalInfo(additionalInfo);
}

HProtocolInfo::~HProtocolInfo()
{
}

HProtocolInfo::HProtocolInfo(const HProtocolInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

HProtocolInfo& HProtocolInfo::operator=(const HProtocolInfo& other)
{
    Q_ASSERT(this != &other);
    h_ptr = other.h_ptr;
    return *this;
}

QString HProtocolInfo::protocol() const
{
    return h_ptr->m_protocol;
}

QString HProtocolInfo::network() const
{
    return h_ptr->m_network;
}

QString HProtocolInfo::contentFormat() const
{
    return h_ptr->m_contentFormat;
}

QString HProtocolInfo::additionalInfo() const
{
    return h_ptr->m_additionalInfo;
}

void HProtocolInfo::setProtocol(const QString& arg)
{
    if (!arg.contains(':'))
    {
        h_ptr->m_protocol = arg.trimmed();
    }
}

void HProtocolInfo::setNetwork(const QString& arg)
{
    if (!arg.contains(':'))
    {
        h_ptr->m_network = arg.trimmed();
    }
}

void HProtocolInfo::setContentFormat(const QString& arg)
{
    if (!arg.contains(':'))
    {
        h_ptr->m_contentFormat = arg.trimmed();
    }
}

void HProtocolInfo::setAdditionalInfo(const QString& arg)
{
    if (!arg.contains(':'))
    {
        h_ptr->m_additionalInfo = arg.trimmed();
    }
}

bool HProtocolInfo::isEmpty() const
{
    return protocol().isEmpty() && network().isEmpty() &&
           contentFormat().isEmpty() && additionalInfo().isEmpty();
}

bool HProtocolInfo::isValid() const
{
    return !protocol().isEmpty() && !network().isEmpty() &&
           !contentFormat().isEmpty() && !additionalInfo().isEmpty();
}

QString HProtocolInfo::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    return QString("%1:%2:%3:%4").arg(
        protocol(), network(), contentFormat(), additionalInfo());
}

HProtocolInfo HProtocolInfo::createUsingWildcards()
{
    return HProtocolInfo("*:*:*:*");
}

bool operator==(const HProtocolInfo& obj1, const HProtocolInfo& obj2)
{
    return obj1.protocol() == obj2.protocol() &&
           obj1.network() == obj2.network() &&
           obj1.contentFormat() == obj2.contentFormat() &&
           obj1.additionalInfo() == obj2.additionalInfo();
}

}
}
}
