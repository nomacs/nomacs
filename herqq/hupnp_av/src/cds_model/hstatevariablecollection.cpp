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

#include "hstatevariablecollection.h"

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HStateVariableCollection>(
        "Herqq::Upnp::Av::HStateVariableCollection");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HStateVariableData
 ******************************************************************************/
HStateVariableData::HStateVariableData() :
    m_svName(), m_svValue(), m_channel()
{
}

HStateVariableData::HStateVariableData(
    const QString& name, const QString& value, const HChannel& channel) :
        m_svName(name.trimmed()), m_svValue(value), m_channel(channel)
{
}

bool HStateVariableData::isValid() const
{
    return !name().isEmpty();
}

void HStateVariableData::setName(const QString& arg)
{
    m_svName = arg.trimmed();
}

void HStateVariableData::setValue(const QString& arg)
{
    m_svValue = arg;
}

void HStateVariableData::setChannel(const HChannel& arg)
{
    m_channel = arg;
}

bool operator==(const HStateVariableData& obj1, const HStateVariableData& obj2)
{
    return obj1.name() == obj2.name() &&
           obj1.value() == obj2.value() &&
           obj1.channel() == obj2.channel();
}

/*******************************************************************************
 * HStateVariableCollectionPrivate
 ******************************************************************************/
class HStateVariableCollectionPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HStateVariableCollectionPrivate)

public:
    QString m_serviceName;
    HStateVariableCollection::RcsInstanceType m_rcsInstanceType;
    QList<HStateVariableData> m_stateVariables;

    HStateVariableCollectionPrivate() :
        m_serviceName(), m_rcsInstanceType(HStateVariableCollection::Undefined),
        m_stateVariables()
    {
    }
};

/*******************************************************************************
 * HStateVariableCollection
 ******************************************************************************/
HStateVariableCollection::HStateVariableCollection() :
    h_ptr(new HStateVariableCollectionPrivate())
{
}

HStateVariableCollection::HStateVariableCollection(
    const QString& serviceName, RcsInstanceType rcsInstanceType) :
        h_ptr(new HStateVariableCollectionPrivate())
{
    h_ptr->m_serviceName = serviceName.trimmed();
    if (!h_ptr->m_serviceName.isEmpty())
    {
        h_ptr->m_rcsInstanceType = rcsInstanceType;
    }
}

HStateVariableCollection::HStateVariableCollection(
    const HStateVariableCollection& other) :
        h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HStateVariableCollection& HStateVariableCollection::operator=(
    const HStateVariableCollection& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HStateVariableCollection::~HStateVariableCollection()
{
}

bool HStateVariableCollection::isValid() const
{
    return !serviceName().isEmpty();
}

QString HStateVariableCollection::serviceName() const
{
    return h_ptr->m_serviceName;
}

HStateVariableCollection::RcsInstanceType HStateVariableCollection::rcsInstanceType() const
{
    return h_ptr->m_rcsInstanceType;
}

QList<HStateVariableData> HStateVariableCollection::stateVariables() const
{
    return h_ptr->m_stateVariables;
}

void HStateVariableCollection::setRcsInstanceType(RcsInstanceType arg)
{
    h_ptr->m_rcsInstanceType = arg;
}

void HStateVariableCollection::setStateVariables(
    const QList<HStateVariableData>& arg)
{
    h_ptr->m_stateVariables = arg;
}

void HStateVariableCollection::add(const HStateVariableData& arg)
{
    if (arg.isValid())
    {
        h_ptr->m_stateVariables.append(arg);
    }
}

QString HStateVariableCollection::toString(RcsInstanceType type)
{
    QString retVal;
    switch(type)
    {
    case PreMix:
        retVal = "pre-mix";
        break;
    case PostMix:
        retVal = "post-mix";
        break;
    default:
        break;
    }
    return retVal;
}

HStateVariableCollection::RcsInstanceType HStateVariableCollection::fromString(
    const QString& type)
{
    RcsInstanceType retVal = Undefined;
    if (type.compare("pre-mix", Qt::CaseInsensitive) == 0)
    {
        retVal = PreMix;
    }
    else if (type.compare("post-mix", Qt::CaseInsensitive) == 0)
    {
        retVal = PostMix;
    }
    return retVal;
}

bool operator==(const HStateVariableCollection& obj1, const HStateVariableCollection& obj2)
{
    return obj1.serviceName() == obj2.serviceName() &&
           obj1.rcsInstanceType() == obj2.rcsInstanceType();
}

}
}
}

