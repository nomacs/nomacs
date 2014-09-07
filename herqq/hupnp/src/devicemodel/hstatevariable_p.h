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

#ifndef HSTATEVARIABLE_P_H_
#define HSTATEVARIABLE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../dataelements/hstatevariableinfo.h"

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HStateVariable
//
class HStateVariablePrivate
{
H_DISABLE_COPY(HStateVariablePrivate)

public:

    HStateVariableInfo m_info;
    QVariant m_value;

public:

    HStateVariablePrivate() : m_info(), m_value() {}
    ~HStateVariablePrivate(){}

    bool setValue(const QVariant& value, QString* err)
    {
        if (value == m_value)
        {
            if (err)
            {
                *err = QString("The new and the old value are equal: [%1]").arg(
                    value.toString());
            }
            return false;
        }

        QVariant convertedValue;
        if (m_info.isValidValue(value, &convertedValue, err))
        {
            m_value = convertedValue;
            return true;
        }

        return false;
    }
};

}
}

#endif /* UPNP_ACTION_P_H_ */
