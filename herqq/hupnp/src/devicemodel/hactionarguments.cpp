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

#include "hactionarguments.h"
#include "hactionarguments_p.h"

#include "../general/hupnp_global_p.h"

#include <QtCore/QUrl>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{


/*******************************************************************************
 * HActionArgumentPrivate
 *******************************************************************************/
class HActionArgumentPrivate :
    public QSharedData
{
public:

    QString m_name;
    HStateVariableInfo m_stateVariableInfo;
    QVariant m_value;

    HActionArgumentPrivate();
};

HActionArgumentPrivate::HActionArgumentPrivate() :
    m_name(), m_stateVariableInfo(), m_value()
{
}

/*******************************************************************************
 * HActionArgument
 *******************************************************************************/
HActionArgument::HActionArgument() :
    h_ptr(new HActionArgumentPrivate())
{
}

HActionArgument::HActionArgument(
    const QString& name, const HStateVariableInfo& stateVariableInfo,
    QString* err) :
        h_ptr(new HActionArgumentPrivate())
{
    if (!verifyName(name, err))
    {
        return;
    }
    else if (!stateVariableInfo.isValid())
    {
        if (err)
        {
            *err = "The provided state variable information object was not valid";
        }
        return;
    }

    h_ptr->m_name = name;
    h_ptr->m_value = stateVariableInfo.defaultValue();
    h_ptr->m_stateVariableInfo = stateVariableInfo;
}

HActionArgument::HActionArgument(const HActionArgument& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HActionArgument& HActionArgument::operator=(const HActionArgument& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HActionArgument::~HActionArgument()
{
}

void HActionArgument::detach()
{
    h_ptr.detach();
}

QString HActionArgument::name() const
{
    return h_ptr->m_name;
}

const HStateVariableInfo& HActionArgument::relatedStateVariable() const
{
    return h_ptr->m_stateVariableInfo;
}

HUpnpDataTypes::DataType HActionArgument::dataType() const
{
    return h_ptr->m_stateVariableInfo.dataType();
}

QVariant HActionArgument::value() const
{
    return h_ptr->m_value;
}

bool HActionArgument::setValue(const QVariant& value)
{
    QVariant convertedValue;
    if (isValid() && h_ptr->m_stateVariableInfo.isValidValue(value, &convertedValue))
    {
        h_ptr->m_value = convertedValue;
        return true;
    }

    return false;
}

bool HActionArgument::isValid() const
{
    return !h_ptr->m_name.isEmpty();
}

bool HActionArgument::operator!() const
{
    return !isValid();
}

QString HActionArgument::toString() const
{
    if (!isValid())
    {
        return "";
    }

    return QString("%1: %2").arg(
             name(),
             dataType() == HUpnpDataTypes::uri ? value().toUrl().toString() :
             value().toString());
}

bool HActionArgument::isValidValue(const QVariant& value)
{
    return isValid() && h_ptr->m_stateVariableInfo.isValidValue(value);
}

bool operator==(const HActionArgument& arg1, const HActionArgument& arg2)
{
    return arg1.h_ptr->m_name == arg2.h_ptr->m_name &&
           arg1.h_ptr->m_value == arg2.h_ptr->m_value &&
           arg1.h_ptr->m_stateVariableInfo == arg2.h_ptr->m_stateVariableInfo;
}

bool operator!=(const HActionArgument& arg1, const HActionArgument& arg2)
{
    return !(arg1 == arg2);
}

/*******************************************************************************
 * HActionArgumentsPrivate
 *******************************************************************************/
HActionArgumentsPrivate::HActionArgumentsPrivate()
{
}

HActionArgumentsPrivate::HActionArgumentsPrivate(
    const QVector<HActionArgument>& args)
{
    QVector<HActionArgument>::const_iterator ci = args.constBegin();

    for (; ci != args.constEnd(); ++ci)
    {
        append(*ci);
    }
}

/*******************************************************************************
 * HActionArguments
 *******************************************************************************/
HActionArguments::HActionArguments() :
    h_ptr(new HActionArgumentsPrivate())
{
}

HActionArguments::HActionArguments(const QVector<HActionArgument>& args) :
    h_ptr(HActionArgumentsPrivate::copy(args))
{
}

HActionArguments::~HActionArguments()
{
    delete h_ptr;
}

HActionArguments::HActionArguments(const HActionArguments& other) :
    h_ptr(HActionArgumentsPrivate::copy(other))
{
    Q_ASSERT(&other != this);
}

HActionArguments& HActionArguments::operator=(const HActionArguments& other)
{
    Q_ASSERT(&other != this);
    delete h_ptr;
    h_ptr = HActionArgumentsPrivate::copy(other);
    return *this;
}

bool HActionArguments::contains(const QString& argumentName) const
{
    return h_ptr->m_arguments.contains(argumentName);
}

HActionArgument HActionArguments::get(qint32 index) const
{
    return h_ptr->m_argumentsOrdered.at(index);
}

HActionArgument HActionArguments::get(const QString& argumentName) const
{
    return h_ptr->m_arguments.value(argumentName);
}

HActionArguments::const_iterator HActionArguments::constBegin() const
{
    return h_ptr->m_argumentsOrdered.constBegin();
}

HActionArguments::const_iterator HActionArguments::constEnd() const
{
    return h_ptr->m_argumentsOrdered.constEnd();
}

HActionArguments::iterator HActionArguments::begin()
{
    return h_ptr->m_argumentsOrdered.begin();
}

HActionArguments::iterator HActionArguments::end()
{
    return h_ptr->m_argumentsOrdered.end();
}

HActionArguments::const_iterator HActionArguments::begin() const
{
    return h_ptr->m_argumentsOrdered.begin();
}

HActionArguments::const_iterator HActionArguments::end() const
{
    return h_ptr->m_argumentsOrdered.end();
}

qint32 HActionArguments::size() const
{
    return h_ptr->m_argumentsOrdered.size();
}

HActionArgument HActionArguments::operator[](qint32 index) const
{
    return h_ptr->m_argumentsOrdered.at(index);
}

HActionArgument HActionArguments::operator[](const QString& argName) const
{
    return h_ptr->m_arguments.value(argName);
}

QStringList HActionArguments::names() const
{
    return h_ptr->m_arguments.keys();
}

bool HActionArguments::isEmpty() const
{
    return h_ptr->m_argumentsOrdered.isEmpty();
}

void HActionArguments::clear()
{
    h_ptr->m_arguments.clear();
    h_ptr->m_argumentsOrdered.clear();
}

bool HActionArguments::remove(const QString& name)
{
    if (h_ptr->m_arguments.contains(name))
    {
        h_ptr->m_arguments.remove(name);
        HActionArguments::iterator it = h_ptr->m_argumentsOrdered.begin();
        for(; it != h_ptr->m_argumentsOrdered.end(); ++it)
        {
            if (it->name() == name)
            {
                h_ptr->m_argumentsOrdered.erase(it);
                return true;
            }
        }
    }

    return false;
}

bool HActionArguments::append(const HActionArgument& arg)
{
    if (!arg.isValid())
    {
        return false;
    }
    else if (h_ptr->m_arguments.contains(arg.name()))
    {
        return false;
    }

    h_ptr->m_arguments.insert(arg.name(), arg);
    h_ptr->m_argumentsOrdered.append(arg);

    return true;
}

QVariant HActionArguments::value(const QString& name, bool* ok) const
{
    QVariant retVal;

    if (h_ptr->m_arguments.contains(name))
    {
        retVal = h_ptr->m_arguments.value(name).value();
        if (ok) { *ok = true; }
    }
    else
    {
        if (ok) { *ok = false; }
    }

    return retVal;
}

bool HActionArguments::setValue(const QString& name, const QVariant& value)
{
    if (h_ptr->m_arguments.contains(name))
    {
        return h_ptr->m_arguments[name].setValue(value);
    }

    return false;
}

QString HActionArguments::toString() const
{
    QString retVal;

    HActionArguments::const_iterator ci = constBegin();
    for (; ci != constEnd(); ++ci)
    {
        retVal.append(ci->toString()).append("\n");
    }

    return retVal;
}

void swap(HActionArguments& a, HActionArguments& b)
{
    std::swap(a.h_ptr, b.h_ptr);
}

bool operator==(const HActionArguments& arg1, const HActionArguments& arg2)
{
    if (arg1.h_ptr->m_argumentsOrdered.size() !=
        arg2.h_ptr->m_argumentsOrdered.size())
    {
        return false;
    }

    qint32 size = arg1.h_ptr->m_argumentsOrdered.size();
    for(qint32 i = 0; i < size; ++i)
    {
        if (arg1.h_ptr->m_argumentsOrdered.at(i) !=
            arg2.h_ptr->m_argumentsOrdered.at(i))
        {
            return false;
        }
    }

    return true;
}

}
}
