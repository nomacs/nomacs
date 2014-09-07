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

#include "hstatevariables_setupdata.h"

#include "../general/hupnp_global_p.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HStateVariablesSetupData
 ******************************************************************************/
HStateVariablesSetupData::HStateVariablesSetupData(
    DefaultInclusionPolicy policy) :
        m_setupData(), m_defaultInclusionPolicy(policy)
{
}

bool HStateVariablesSetupData::insert(const HStateVariableInfo& setupData)
{
    if (m_setupData.contains(setupData.name()))
    {
        return false;
    }

    m_setupData.insert(setupData.name(), setupData);
    return true;
}

bool HStateVariablesSetupData::remove(const QString& stateVarName)
{
    if (m_setupData.contains(stateVarName))
    {
        m_setupData.remove(stateVarName);
        return true;
    }

    return false;
}

HStateVariablesSetupData::DefaultInclusionPolicy
    HStateVariablesSetupData::defaultInclusionPolicy() const
{
    return m_defaultInclusionPolicy;
}

bool HStateVariablesSetupData::setInclusionRequirement(
    const QString& name, HInclusionRequirement incReq)
{
    if (m_setupData.contains(name))
    {
        HStateVariableInfo setupInfo = m_setupData.value(name);
        setupInfo.setInclusionRequirement(incReq);
        m_setupData.insert(name, setupInfo);
        return true;
    }

    return false;
}

HStateVariableInfo HStateVariablesSetupData::get(
    const QString& stateVarName) const
{
    return m_setupData.value(stateVarName);
}

bool HStateVariablesSetupData::contains(const QString& name) const
{
    return m_setupData.contains(name);
}

QSet<QString> HStateVariablesSetupData::names() const
{
    return m_setupData.keys().toSet();
}

qint32 HStateVariablesSetupData::size() const
{
    return m_setupData.size();
}

bool HStateVariablesSetupData::isEmpty() const
{
    return m_setupData.isEmpty();
}

}
}
