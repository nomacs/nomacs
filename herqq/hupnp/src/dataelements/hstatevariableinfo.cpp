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

#include "hstatevariableinfo.h"
#include "hstatevariableinfo_p.h"

#include "../general/hupnp_global_p.h"
#include "../general/hupnp_datatypes_p.h"

#include "../utils/hmisc_utils_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HStateVariableInfoPrivate
 ******************************************************************************/
HStateVariableInfoPrivate::HStateVariableInfoPrivate() :
    m_name(),
    m_dataType(HUpnpDataTypes::Undefined),
    m_variantDataType(QVariant::Invalid),
    m_defaultValue(),
    m_eventingType(HStateVariableInfo::NoEvents),
    m_allowedValueList(),
    m_allowedValueRange(),
    m_inclusionRequirement(InclusionRequirementUnknown),
    m_maxRate(-1),
    m_version(-1)
{
}

bool HStateVariableInfoPrivate::isWithinAllowedRange(
    const QVariant& value, QString* errDescr)
{
    Q_ASSERT(!m_allowedValueRange.isNull());
    Q_ASSERT(HUpnpDataTypes::isNumeric(m_dataType));

    if (HUpnpDataTypes::isRational(m_dataType))
    {
        qreal tmp = value.toDouble();
        if (tmp < m_allowedValueRange.minimum().toDouble() ||
            tmp > m_allowedValueRange.maximum().toDouble())
        {
            if (errDescr)
            {
                *errDescr = QString(
                    "Value [%1] is not within the specified allowed values range.").arg(
                        value.toString());
            }
            return false;
        }
    }
    else
    {
        qlonglong tmp = value.toLongLong();
        if (tmp < m_allowedValueRange.minimum().toLongLong() ||
            tmp > m_allowedValueRange.maximum().toLongLong())
        {
            if (errDescr)
            {
                *errDescr = QString(
                    "Value [%1] is not within the specified allowed values range.").arg(
                        value.toString());
            }
            return false;
        }
    }

    return true;
}

bool HStateVariableInfoPrivate::checkValue(
    const QVariant& value, QVariant* acceptableValue, QString* errDescr) const
{
    QVariant tmp(value);

    if (m_dataType == HUpnpDataTypes::Undefined)
    {
        if (errDescr)
        {
            *errDescr = QString(
                "Data type of the state variable [%1] is not defined.").arg(
                m_name);
        }
        return false;
    }

    if (value.type() != m_variantDataType)
    {
        if (m_variantDataType == QVariant::Url)
        {
            // for some reason, QVariant does not provide automatic conversion between
            // QUrl and other types (this includes QString, unfortunately) and until it does,
            // this has to be handled as a special case.
            QUrl valueAsUrl(value.toString());
            if (!valueAsUrl.isValid())
            {
                if (errDescr)
                {
                    *errDescr = QString(
                        "Invalid value for a URL type: [%1]").arg(
                            value.toString());
                }
                return false;
            }

            tmp = valueAsUrl;
        }
        else if (!tmp.convert(m_variantDataType))
        {
            if (errDescr)
            {
                *errDescr = "Data type mismatch.";
            }
            return false;
        }
    }

    if (m_dataType == HUpnpDataTypes::string && m_allowedValueList.size())
    {
        if (m_allowedValueList.indexOf(value.toString()) < 0)
        {
            if (errDescr)
            {
                *errDescr = QString(
                    "Value [%1] is not included in the allowed values list.").arg(
                        value.toString());
            }
            return false;
        }
    }
    else if (HUpnpDataTypes::isRational(m_dataType) && !m_allowedValueRange.isNull())
    {
        qreal tmp = value.toDouble();
        if (tmp < m_allowedValueRange.minimum().toDouble() ||
            tmp > m_allowedValueRange.maximum().toDouble())
        {
            if (errDescr)
            {
                *errDescr = QString(
                    "Value [%1] is not within the specified allowed values range.").arg(
                        value.toString());
            }
            return false;
        }
    }
    else if (HUpnpDataTypes::isNumeric(m_dataType) && !m_allowedValueRange.isNull())
    {
        qlonglong tmp = value.toLongLong();
        if (tmp < m_allowedValueRange.minimum().toLongLong() ||
            tmp > m_allowedValueRange.maximum().toLongLong())
        {
            if (errDescr)
            {
                *errDescr = QString(
                    "Value [%1] is not within the specified allowed values range.").arg(
                        value.toString());
            }
            return false;
        }
    }

    *acceptableValue = tmp;
    return true;
}

bool HStateVariableInfoPrivate::setName(const QString& name, QString* err)
{
    if (verifyName(name, err))
    {
        m_name = name;
        return true;
    }

    return false;
}

bool HStateVariableInfoPrivate::setDataType(
    HUpnpDataTypes::DataType arg, QString* err)
{
    if (arg == HUpnpDataTypes::Undefined)
    {
        if (err)
        {
            *err = "Data type was undefined";
        }
        return false;
    }

    m_dataType = arg;
    m_variantDataType = HUpnpDataTypes::convertToVariantType(m_dataType);
    m_defaultValue = QVariant(m_variantDataType);

    return true;
}

bool HStateVariableInfoPrivate::setDefaultValue(
    const QVariant& defVal, QString* err)
{
    if (defVal.isNull() || !defVal.isValid() ||
       ((m_dataType == HUpnpDataTypes::string && m_allowedValueList.size()) &&
        defVal.toString().isEmpty()))
    {
        // according to the UDA, default value is OPTIONAL.
        return true;
    }

    QVariant acceptableValue;
    if (checkValue(defVal, &acceptableValue, err))
    {
        m_defaultValue = acceptableValue;
        return true;
    }

    return false;
}

bool HStateVariableInfoPrivate::setAllowedValueList(
    const QStringList& allowedValueList, QString* err)
{
    if (m_dataType != HUpnpDataTypes::string)
    {
        if (err)
        {
            *err = "The data type of the state variable has to be [string]";
        }

        return false;
    }

    m_allowedValueList = allowedValueList;
    if (!allowedValueList.empty() && !allowedValueList.contains(m_defaultValue.toString()))
    {
        m_defaultValue = QVariant(QVariant::String);
    }

    return true;
}

bool HStateVariableInfoPrivate::setAllowedValueRange(
    const QVariant& minimumValue, const QVariant& maximumValue,
    const QVariant& stepValue, QString* err)
{
    if (!HUpnpDataTypes::isNumeric(m_dataType))
    {
        if (err)
        {
            *err = "Cannot define allowed value range when the data type "
                    "of the state variable is not numeric";
        }
        return false;
    }

    HValueRange valueRange;
    bool ok = HValueRange::fromVariant(
        m_variantDataType, minimumValue, maximumValue, stepValue,
        &valueRange, err);

    if (!ok)
    {
        return false;
    }

    m_allowedValueRange = valueRange;
    if (!isWithinAllowedRange(m_defaultValue))
    {
        m_defaultValue = QVariant(m_variantDataType);
    }

    return true;
}

/*******************************************************************************
 * HStateVariableInfo
 ******************************************************************************/
HStateVariableInfo::HStateVariableInfo() :
    h_ptr(new HStateVariableInfoPrivate())
{
}

HStateVariableInfo::HStateVariableInfo(
    const QString& name,
    HUpnpDataTypes::DataType datatype,
    HInclusionRequirement inclusionReq,
    QString* err) :
        h_ptr(new HStateVariableInfoPrivate())
{
    QScopedPointer<HStateVariableInfoPrivate> hptr(
        new HStateVariableInfoPrivate());

    if (!hptr->setName(name, err))
    {
        return;
    }

    if (!hptr->setDataType(datatype, err))
    {
        return;
    }

    hptr->m_eventingType = NoEvents;
    hptr->m_inclusionRequirement = inclusionReq;

    h_ptr = hptr.take();
}

HStateVariableInfo::HStateVariableInfo(
    const QString& name,
    HUpnpDataTypes::DataType datatype,
    EventingType eventingType,
    HInclusionRequirement inclusionReq,
    QString* err) :
        h_ptr(new HStateVariableInfoPrivate())
{
    QScopedPointer<HStateVariableInfoPrivate> hptr(
        new HStateVariableInfoPrivate());

    if (!hptr->setName(name, err))
    {
        return;
    }

    if (!hptr->setDataType(datatype, err))
    {
        return;
    }

    hptr->m_eventingType = eventingType;
    hptr->m_inclusionRequirement = inclusionReq;

    h_ptr = hptr.take();
}

HStateVariableInfo::HStateVariableInfo(
    const QString& name,
    HUpnpDataTypes::DataType datatype,
    const QVariant& defaultValue,
    EventingType eventingType,
    HInclusionRequirement inclusionReq,
    QString* err) :
        h_ptr(new HStateVariableInfoPrivate())
{
    QScopedPointer<HStateVariableInfoPrivate> hptr(
        new HStateVariableInfoPrivate());

    if (!hptr->setName(name, err))
    {
        return;
    }

    if (!hptr->setDataType(datatype, err))
    {
        return;
    }

    if (!hptr->setDefaultValue(defaultValue, err))
    {
        return;
    }

    hptr->m_eventingType = eventingType;
    hptr->m_inclusionRequirement = inclusionReq;

    h_ptr = hptr.take();
}

HStateVariableInfo::HStateVariableInfo(
    const QString& name,
    const QVariant& defaultValue,
    const QStringList& allowedValueList,
    EventingType eventingType,
    HInclusionRequirement inclusionReq,
    QString* err) :
        h_ptr(new HStateVariableInfoPrivate())
{
    QScopedPointer<HStateVariableInfoPrivate> hptr(
        new HStateVariableInfoPrivate());

    if (!hptr->setName(name, err))
    {
        return;
    }

    if (!hptr->setDataType(HUpnpDataTypes::string))
    {
        return;
    }

    if (!hptr->setDefaultValue(defaultValue, err))
    {
        return;
    }

    if (!hptr->setAllowedValueList(allowedValueList, err))
    {
        return;
    }

    hptr->m_eventingType = eventingType;
    hptr->m_inclusionRequirement = inclusionReq;

    h_ptr = hptr.take();
}

HStateVariableInfo::HStateVariableInfo(
    const QString& name,
    HUpnpDataTypes::DataType datatype,
    const QVariant& defaultValue,
    const QVariant& minimumValue,
    const QVariant& maximumValue,
    const QVariant& stepValue,
    EventingType eventingType,
    HInclusionRequirement inclusionReq,
    QString* err) :
        h_ptr(new HStateVariableInfoPrivate())
{
    QScopedPointer<HStateVariableInfoPrivate> hptr(
        new HStateVariableInfoPrivate());

    if (!hptr->setName(name, err))
    {
        return;
    }

    if (!hptr->setDataType(datatype, err))
    {
        return;
    }

    if (!hptr->setDefaultValue(defaultValue, err))
    {
        return;
    }

    if (!hptr->setAllowedValueRange(minimumValue, maximumValue, stepValue, err))
    {
        return;
    }

    hptr->m_eventingType = eventingType;
    hptr->m_inclusionRequirement = inclusionReq;

    h_ptr = hptr.take();
}

HStateVariableInfo::HStateVariableInfo(const HStateVariableInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HStateVariableInfo& HStateVariableInfo::operator=(
    const HStateVariableInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HStateVariableInfo::~HStateVariableInfo()
{
}

qint32 HStateVariableInfo::version() const
{
    return h_ptr->m_version;
}

void HStateVariableInfo::setVersion(qint32 version)
{
    h_ptr->m_version = version < 0 ? -1 : version;
}

QString HStateVariableInfo::name() const
{
    return h_ptr->m_name;
}

HInclusionRequirement HStateVariableInfo::inclusionRequirement() const
{
    return h_ptr->m_inclusionRequirement;
}

void HStateVariableInfo::setInclusionRequirement(HInclusionRequirement arg)
{
    h_ptr->m_inclusionRequirement = arg;
}

qint32 HStateVariableInfo::maxEventRate() const
{
    return h_ptr->m_maxRate;
}

void HStateVariableInfo::setMaxEventRate(qint32 arg)
{
    if (h_ptr->m_eventingType != NoEvents)
    {
        h_ptr->m_maxRate = arg < 0 ? -1 : arg;
    }
}

HUpnpDataTypes::DataType HStateVariableInfo::dataType() const
{
    return h_ptr->m_dataType;
}

HStateVariableInfo::EventingType HStateVariableInfo::eventingType() const
{
    return h_ptr->m_eventingType;
}

void HStateVariableInfo::setEventingType(HStateVariableInfo::EventingType arg)
{
    h_ptr->m_eventingType = arg;
}

QStringList HStateVariableInfo::allowedValueList() const
{
    return h_ptr->m_allowedValueList;
}

bool HStateVariableInfo::setAllowedValueList(
    const QStringList& allowedValueList)
{
    return h_ptr->setAllowedValueList(allowedValueList);
}

QVariant HStateVariableInfo::minimumValue() const
{
    return h_ptr->m_allowedValueRange.minimum();
}

QVariant HStateVariableInfo::maximumValue() const
{
    return h_ptr->m_allowedValueRange.maximum();
}

QVariant HStateVariableInfo::stepValue() const
{
    return h_ptr->m_allowedValueRange.step();
}

bool HStateVariableInfo::setAllowedValueRange(
    const QVariant& minimumValue, const QVariant& maximumValue,
    const QVariant& stepValue, QString* err)
{
    return h_ptr->setAllowedValueRange(
        minimumValue, maximumValue, stepValue, err);
}

QVariant HStateVariableInfo::defaultValue() const
{
    return h_ptr->m_defaultValue;
}

bool HStateVariableInfo::setDefaultValue(const QVariant& defVal, QString* err)
{
    return h_ptr->setDefaultValue(defVal, err);
}

bool HStateVariableInfo::isConstrained() const
{
    return !h_ptr->m_allowedValueList.isEmpty() ||
           !h_ptr->m_allowedValueRange.isNull();
}

bool HStateVariableInfo::isValidValue(
    const QVariant& value, QVariant* convertedValue, QString* err) const
{
    QVariant acceptableValue;
    if (h_ptr->checkValue(value, &acceptableValue, err))
    {
        if (convertedValue)
        {
            *convertedValue = acceptableValue;
        }
        return true;
    }

    return false;
}

bool HStateVariableInfo::isValid() const
{
    return !h_ptr->m_name.isEmpty();
}

bool operator==(const HStateVariableInfo& arg1, const HStateVariableInfo& arg2)
{
    return arg1.h_ptr->m_name == arg2.h_ptr->m_name &&
           arg1.h_ptr->m_maxRate == arg2.h_ptr->m_maxRate &&
           arg1.h_ptr->m_version == arg2.h_ptr->m_version &&
           arg1.h_ptr->m_dataType == arg2.h_ptr->m_dataType &&
           arg1.h_ptr->m_defaultValue == arg2.h_ptr->m_defaultValue &&
           arg1.h_ptr->m_eventingType == arg2.h_ptr->m_eventingType &&
           arg1.h_ptr->m_allowedValueList == arg2.h_ptr->m_allowedValueList &&
           arg1.h_ptr->m_allowedValueRange == arg2.h_ptr->m_allowedValueRange &&
           arg1.h_ptr->m_inclusionRequirement == arg2.h_ptr->m_inclusionRequirement;
}

quint32 qHash(const HStateVariableInfo& key)
{
    QByteArray data = key.name().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
