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

#ifndef HSTATEVARIABLENIFO_P_H_
#define HSTATEVARIABLENIFO_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hvaluerange_p.h"
#include "hstatevariableinfo.h"
#include "../general/hupnp_global.h"
#include "../general/hupnp_datatypes.h"

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QSharedData>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HStateVariableInfo
//
class HStateVariableInfoPrivate :
    public QSharedData
{

public: // attributes

    QString                  m_name;
    HUpnpDataTypes::DataType m_dataType;
    QVariant::Type           m_variantDataType;
    QVariant                 m_defaultValue;
    HStateVariableInfo::EventingType m_eventingType;
    QStringList              m_allowedValueList;
    HValueRange              m_allowedValueRange;

    HInclusionRequirement m_inclusionRequirement;
    qint32 m_maxRate;
    qint32 m_version;

public: // methods

    HStateVariableInfoPrivate();

    bool isWithinAllowedRange(const QVariant&, QString* errDescr=0);

    bool checkValue(
        const QVariant&, QVariant* acceptableValue, QString* errDescr = 0) const;

    bool setName(const QString& name, QString* err = 0);
    bool setDataType(HUpnpDataTypes::DataType arg, QString* err = 0);
    bool setDefaultValue(const QVariant& defVal, QString* err = 0);

    bool setAllowedValueList(
        const QStringList& allowedValueList, QString* err = 0);

    bool setAllowedValueRange(
        const QVariant& minimumValue, const QVariant& maximumValue,
        const QVariant& stepValue, QString* err = 0);
};

}
}

#endif /* HSTATEVARIABLENIFO_P_H_ */
