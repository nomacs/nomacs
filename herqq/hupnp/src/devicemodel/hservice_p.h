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

#ifndef HSERVICE_P_H_
#define HSERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HServiceInfo>
#include <HUpnpCore/HUpnpDataTypes>
#include <HUpnpCore/HStateVariableInfo>

#include <QtCore/QUrl>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

//
//
//
template<typename Owner, typename Action, typename StateVariable>
class H_UPNP_CORE_EXPORT HServicePrivate
{
H_DECLARE_PUBLIC(HServerService)
H_DISABLE_COPY(HServicePrivate)

public: // attributes

    HServiceInfo m_serviceInfo;
    QString m_serviceDescription;
    QString m_lastError;

    QHash<QString, Action*> m_actions;
    QHash<QString, StateVariable*> m_stateVariables;

    Owner* q_ptr;

    QByteArray m_loggingIdentifier;

    bool m_evented;

public: // methods

    HServicePrivate() :
        m_serviceInfo(),
        m_serviceDescription(),
        m_lastError(),
        m_actions(),
        m_stateVariables(),
        q_ptr(0),
        m_loggingIdentifier(),
        m_evented(false)
    {
    }

    virtual ~HServicePrivate()
    {
        qDeleteAll(m_actions);
        qDeleteAll(m_stateVariables);
    }

    bool addStateVariable(StateVariable* stateVariable)
    {
        Q_ASSERT(stateVariable);

        const HStateVariableInfo& info = stateVariable->info();
        Q_ASSERT(info.isValid());
        Q_ASSERT(!m_stateVariables.contains(info.name()));

        m_stateVariables.insert(info.name(), stateVariable);

        if (!m_evented && info.eventingType() != HStateVariableInfo::NoEvents)
        {
            m_evented = true;
        }

        return true;
    }

    bool updateVariable(const QString& stateVarName, const QVariant& value)
    {
        StateVariable* sv = m_stateVariables.value(stateVarName);
        return sv ? sv->setValue(value) : false;
    }

    enum ReturnValue
    {
        Failed,
        Ignored,
        Updated
    };

    ReturnValue updateVariables(const QList<QPair<QString, QString> >& variables)
    {
        // before modifying anything, it is better to be sure that the incoming
        // data is valid and it can be set completely.
        for (int i = 0; i < variables.size(); ++i)
        {
            StateVariable* stateVar = m_stateVariables.value(variables[i].first);

            if (!stateVar)
            {
                m_lastError = QString(
                    "Cannot update state variable: no state variable [%1]").arg(
                        variables[i].first);

                return Failed;
            }

            const HStateVariableInfo& info = stateVar->info();
            if (!info.isValidValue(
                    HUpnpDataTypes::convertToRightVariantType(
                        variables[i].second, info.dataType())))
            {
                m_lastError = QString(
                    "Cannot update state variable [%1]. New value is invalid: [%2]").
                        arg(info.name(), variables[i].second);

                return Failed;
            }
        }

        bool changed = false;
        for (int i = 0; i < variables.size(); ++i)
        {
            StateVariable* stateVar = m_stateVariables.value(variables[i].first);
            Q_ASSERT(stateVar);

            const HStateVariableInfo& info = stateVar->info();

            if (stateVar->setValue(
                    HUpnpDataTypes::convertToRightVariantType(
                        variables[i].second, info.dataType())) &&
                    !changed)
            {
                changed = true;
            }
        }

        return changed ? Updated : Ignored;
    }

    QVariant value(const QString& stateVarName, bool* ok = 0) const
    {
        if (m_stateVariables.contains(stateVarName))
        {
            if (ok) { *ok = true; }
            return m_stateVariables.value(stateVarName)->value();
        }

        if (ok) { *ok = false; }
        return QVariant();
    }

    bool setValue(const QString& stateVarName, const QVariant& value)
    {
        if (m_stateVariables.contains(stateVarName))
        {
            return m_stateVariables.value(stateVarName)->setValue(value);
        }

        return false;
    }
};

}
}

#endif /* HSERVICE_P_H_ */
