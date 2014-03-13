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

#ifndef HVALUERANGE_P_H_
#define HVALUERANGE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace Herqq
{

namespace Upnp
{

//
// \internal
//
class HValueRange
{
private:

    QVariant m_maximum;
    QVariant m_minimum;
    QVariant m_step;

    template<typename T>
    static bool checkValues(const HValueRange& val, QString* err = 0)
    {
        T min  = val.m_minimum.value<T>();
        T max  = val.m_maximum.value<T>();
        T step = val.m_step.value<T>();

        if (min > max)
        {
            if (err)
            {
                *err = "Minimum value cannot be larger than the maximum";
            }
            return false;
        }

        if (max - min < step)
        {
            if (err)
            {
                *err = "Step value cannot be larger than the entire range";
            }
            return false;
        }

        return true;
    }

public:

    inline HValueRange (){}
    inline ~HValueRange(){}

    inline QVariant maximum() const { return m_maximum; }
    inline QVariant minimum() const { return m_minimum; }
    inline QVariant step   () const { return m_step   ; }

    inline bool isNull() const
    {
        return m_maximum.isNull();
        // if any of the attributes is null, all are null ==> the object is null
    }

    static bool fromVariant(
        QVariant::Type dataType,
        const QVariant& minimum, const QVariant& maximum, const QVariant& step,
        HValueRange* retVal, QString* err = 0)
    {
        return fromString(
            dataType, minimum.toString(), maximum.toString(), step.toString(),
            retVal, err);
    }

    static bool fromString(
        QVariant::Type dataType,
        const QString& minimum, const QString& maximum, const QString& step,
        HValueRange* retVal, QString* err = 0)
    {
        HValueRange tmpRetVal;

        tmpRetVal.m_maximum = maximum;
        if (!tmpRetVal.m_maximum.convert(dataType))
        {
            if (err) { *err = "Invalid maximum value"; }
            return false;
        }

        tmpRetVal.m_minimum = minimum;
        if (!tmpRetVal.m_minimum.convert(dataType))
        {
            if (err) { *err = "Invalid minimum value"; }
            return false;
        }

        tmpRetVal.m_step = step;
        if (!tmpRetVal.m_step.convert(dataType))
        {
            if (err) { *err = "Invalid step value"; }
            return false;
        }

        bool ok = false;
        switch(dataType)
        {
            case QVariant::Char      :
                ok = checkValues<char>(tmpRetVal, err);
                break;

            case QVariant::Int       :
                ok = checkValues<qint32>(tmpRetVal, err);
                break;

            case QVariant::LongLong  :
                ok = checkValues<qlonglong>(tmpRetVal, err);
                break;

            case QVariant::UInt      :
                ok = checkValues<quint32>(tmpRetVal, err);
                break;

            case QVariant::ULongLong :
                ok = checkValues<qulonglong>(tmpRetVal, err);
                break;

            case QVariant::Double    :
                ok = checkValues<qreal>(tmpRetVal, err);
                break;

            default:
                Q_ASSERT(false);
                if (err) { *err = "Invalid data type"; }
                return false;
        }

        if (ok) { *retVal = tmpRetVal; }
        return ok;
    }
};

bool operator==(const HValueRange& arg1, const HValueRange& arg2);
inline bool operator!=(const HValueRange& arg1, const HValueRange& arg2)
{
    return !(arg1 == arg2);
}

}
}

#endif /* HVALUERANGE_P_H_ */
