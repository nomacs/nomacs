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

#ifndef HCDS_SERIALIZER_P_H_
#define HCDS_SERIALIZER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpAv/HCdsDidlLiteSerializer>

#include <QtCore/QHash>
#include <QtCore/QString>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
// Implementation details of HCdsDidlLiteSerializer.
//
class HCdsDidlLiteSerializerPrivate
{
public:

    typedef HObject* (*HObjectCreator)();

    QHash<QString, HObjectCreator> m_creatorFunctions;

    QString m_lastErrorDescription;

    HCdsDidlLiteSerializerPrivate();
    ~HCdsDidlLiteSerializerPrivate();

    bool serializePropertyFromAttribute(
        HObject* object, const QString& xmlTokenName,
        const QString& attributeValue);

    bool serializePropertyFromElement(
        HObject* object, const QString& xmlTokenName,
        QXmlStreamReader& reader);

    HObject* parseObject(QXmlStreamReader&, HCdsDidlLiteSerializer::XmlType);

    void writeDidlLiteDocumentInfo(QXmlStreamWriter&);

    bool serializeProperty(
        const HObject& object, const QString& propName, const QVariant& value,
        const QSet<QString>& filter, QXmlStreamWriter& writer);

    bool serializeObject(
        const HObject&, const QSet<QString>&, QXmlStreamWriter&);
};

}
}
}

#endif /* HCDS_SERIALIZER_P_H_ */
