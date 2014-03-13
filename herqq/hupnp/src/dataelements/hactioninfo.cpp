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

#include "hactioninfo.h"
#include "hactioninfo_p.h"

#include "../general/hupnp_global_p.h"
#include "../utils/hmisc_utils_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionInfoPrivate
 ******************************************************************************/
HActionInfoPrivate::HActionInfoPrivate() :
    m_name(), m_inclusionRequirement(InclusionRequirementUnknown),
    m_inputArguments(), m_outputArguments(), m_hasRetValArg(false)
{
}

/*******************************************************************************
 * HActionInfo
 ******************************************************************************/
HActionInfo::HActionInfo() :
    h_ptr(new HActionInfoPrivate())
{
}

HActionInfo::HActionInfo(
    const QString& name, HInclusionRequirement ireq, QString* err) :
        h_ptr(new HActionInfoPrivate())
{
    if (!verifyName(name, err))
    {
        return;
    }

    h_ptr->m_name = name;
    h_ptr->m_inclusionRequirement = ireq;
}

HActionInfo::HActionInfo(
    const QString& name, const HActionArguments& inputArguments,
    const HActionArguments& outputArguments, bool hasRetVal,
    HInclusionRequirement ireq, QString* err) :
        h_ptr(new HActionInfoPrivate())
{
    if (!verifyName(name, err))
    {
        return;
    }

    if (!outputArguments.size() && hasRetVal)
    {
        if (err)
        {
            *err = "Cannot contain a return value when no output arguments "
                   "are specified";
        }
        return;
    }

    h_ptr->m_name = name;

    h_ptr->m_inputArguments = inputArguments;
    h_ptr->m_outputArguments = outputArguments;

    h_ptr->m_hasRetValArg = hasRetVal;
    h_ptr->m_inclusionRequirement = ireq;
}

HActionInfo::HActionInfo(const HActionInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HActionInfo::~HActionInfo()
{
}

HActionInfo& HActionInfo::operator=(const HActionInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

QString HActionInfo::name() const
{
    return h_ptr->m_name;
}

const HActionArguments& HActionInfo::inputArguments() const
{
    return h_ptr->m_inputArguments;
}

const HActionArguments& HActionInfo::outputArguments() const
{
    return h_ptr->m_outputArguments;
}

QString HActionInfo::returnArgumentName() const
{
    return h_ptr->m_hasRetValArg ? h_ptr->m_outputArguments.get(0).name() : "";
}

HInclusionRequirement HActionInfo::inclusionRequirement() const
{
    return h_ptr->m_inclusionRequirement;
}

bool HActionInfo::isValid() const
{
    return !h_ptr->m_name.isEmpty();
}

bool operator==(const HActionInfo& arg1, const HActionInfo& arg2)
{
    return arg1.h_ptr->m_name == arg2.h_ptr->m_name &&
           arg1.h_ptr->m_hasRetValArg == arg2.h_ptr->m_hasRetValArg &&
           arg1.h_ptr->m_inclusionRequirement == arg2.h_ptr->m_inclusionRequirement &&
           arg1.h_ptr->m_inputArguments == arg2.h_ptr->m_inputArguments &&
           arg1.h_ptr->m_outputArguments == arg2.h_ptr->m_outputArguments;
}

quint32 qHash(const HActionInfo& key)
{
    QByteArray data = key.name().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
