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

#include "hactions_setupdata.h"
#include "hactionarguments.h"
#include "../general/hupnp_global_p.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionSetupPrivate
 ******************************************************************************/
class HActionSetupPrivate :
    public QSharedData
{
public:

    QString m_name;
    int m_version;
    HInclusionRequirement m_inclusionRequirement;
    HActionArguments m_inputArgs;
    HActionArguments m_outputArgs;

    HActionSetupPrivate() :
        m_name(), m_version(0),
        m_inclusionRequirement(InclusionRequirementUnknown), m_inputArgs(),
        m_outputArgs()
    {
    }
};

/*******************************************************************************
 * HActionSetup
 ******************************************************************************/
HActionSetup::HActionSetup() :
    h_ptr(new HActionSetupPrivate())
{
}

HActionSetup::HActionSetup(const QString& name, HInclusionRequirement ireq) :
    h_ptr(new HActionSetupPrivate())
{
    setName(name);

    h_ptr->m_version = 1;
    h_ptr->m_inclusionRequirement = ireq;
}

HActionSetup::HActionSetup(
    const QString& name, int version, HInclusionRequirement ireq) :
        h_ptr(new HActionSetupPrivate())
{
    setName(name);

    h_ptr->m_version = version;
    h_ptr->m_inclusionRequirement = ireq;
}

HActionSetup::~HActionSetup()
{
}

HActionSetup::HActionSetup(const HActionSetup& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HActionSetup& HActionSetup::operator=(const HActionSetup& other)
{
    Q_ASSERT(&other != this);

    h_ptr = other.h_ptr;
    return *this;
}

const HActionArguments& HActionSetup::inputArguments() const
{
    return h_ptr->m_inputArgs;
}

const HActionArguments& HActionSetup::outputArguments() const
{
    return h_ptr->m_outputArgs;
}

HInclusionRequirement HActionSetup::inclusionRequirement() const
{
    return h_ptr->m_inclusionRequirement;
}

bool HActionSetup::isValid() const
{
    return !h_ptr->m_name.isEmpty() && h_ptr->m_version > 0 &&
            h_ptr->m_inclusionRequirement != InclusionRequirementUnknown;
}

QString HActionSetup::name() const
{
    return h_ptr->m_name;
}

int HActionSetup::version() const
{
    return h_ptr->m_version;
}

void HActionSetup::setInputArguments(const HActionArguments& args)
{
    h_ptr->m_inputArgs = args;
}

void HActionSetup::setOutputArguments(const HActionArguments& args)
{
    h_ptr->m_outputArgs = args;
}

void HActionSetup::setInclusionRequirement(HInclusionRequirement arg)
{
    h_ptr->m_inclusionRequirement = arg;
}

void HActionSetup::setVersion(int version)
{
    h_ptr->m_version = version;
}

bool HActionSetup::setName(const QString& name, QString* err)
{
    if (verifyName(name, err))
    {
        h_ptr->m_name = name;
        return true;
    }

    return false;
}

/*******************************************************************************
 * HActionsSetupData
 ******************************************************************************/
HActionsSetupData::HActionsSetupData() :
    m_actionSetupInfos()
{
}

bool HActionsSetupData::insert(const HActionSetup& setupInfo)
{
    if (m_actionSetupInfos.contains(setupInfo.name()) || !setupInfo.isValid())
    {
        return false;
    }

    m_actionSetupInfos.insert(setupInfo.name(), setupInfo);
    return true;
}

bool HActionsSetupData::remove(const QString& actionName)
{
    if (m_actionSetupInfos.contains(actionName))
    {
        m_actionSetupInfos.remove(actionName);
        return true;
    }

    return false;
}

HActionSetup HActionsSetupData::get(const QString& actionName) const
{
    return m_actionSetupInfos.value(actionName);
}

bool HActionsSetupData::setInclusionRequirement(
    const QString& actionName, HInclusionRequirement incReq)
{
    if (m_actionSetupInfos.contains(actionName))
    {
        HActionSetup setupInfo = m_actionSetupInfos.value(actionName);
        setupInfo.setInclusionRequirement(incReq);
        m_actionSetupInfos.insert(actionName, setupInfo);
        return true;
    }

    return false;
}

QSet<QString> HActionsSetupData::names() const
{
    return m_actionSetupInfos.keys().toSet();
}

}
}
