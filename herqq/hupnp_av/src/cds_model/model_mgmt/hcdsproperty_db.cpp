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

#include "hcdsproperty_db.h"
#include "hcdsproperty_db_p.h"
#include "hcdsproperties.h"
#include "hcdsproperty.h"

#include "../hgenre.h"
#include "../hprice.h"
#include "../hdeviceudn.h"
#include "../hmatching_id.h"
#include "../hprogramcode.h"
#include "../hcdsclassinfo.h"
#include "../hdatetimerange.h"
#include "../hscheduledtime.h"
#include "../hpersonwithrole.h"
#include "../hcontentduration.h"
#include "../hforeignmetadata.h"
#include "../hchannelgroupname.h"
#include "../hstatevariablecollection.h"

#include "../../common/hrating.h"
#include "../../common/hprotocolinfo.h"
#include "../../common/hstoragemedium.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HResourceType>

#include <QtCore/QMutex>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HCdsPropertyDbPrivate
 ******************************************************************************/
HCdsPropertyDbPrivate::HCdsPropertyDbPrivate() :
    m_properties(), m_didlLiteDependentProperties(), m_propertiesLock()
{
}

HCdsPropertyDbPrivate::~HCdsPropertyDbPrivate()
{
}

void HCdsPropertyDbPrivate::insert(const HCdsProperty& prop)
{
    QString name = prop.info().name();
    m_properties.insert(name, prop);
    if (name.startsWith('@'))
    {
        m_didlLiteDependentProperties.insert(name);
    }
}

void HCdsPropertyDbPrivate::remove(const QString& propName)
{
    m_properties.remove(propName);
    m_didlLiteDependentProperties.remove(propName);
}

QString HCdsPropertyDbPrivate::variantAsString(const QVariant& var) const
{
    QString retVal;
    switch(var.type())
    {
    case QVariant::Bool:
        retVal = var.toBool() ? "1": "0";
        break;
    case QVariant::Date:
        retVal = var.toDate().toString(Qt::ISODate);
        break;
    case QVariant::Time:
        retVal = var.toTime().toString(Qt::ISODate);
        break;
    case QVariant::DateTime:
        retVal = var.toDateTime().toString(Qt::ISODate);
        break;
    case QVariant::StringList:
        retVal = var.toStringList().join(",");
        break;
    default:
        retVal = var.toString();
        break;
    }
    return retVal;
}

bool HCdsPropertyDbPrivate::serializeHResourceOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HResource res = value.value<HResource>();

    writer.writeStartElement(property);
    writer.writeAttribute("protocolInfo", res.protocolInfo().toString());

    QHash<QString, QString>::const_iterator ci = res.mediaInfo().constBegin();
    for(; ci != res.mediaInfo().constEnd(); ++ci)
    {
        writer.writeAttribute(ci.key(), ci.value());
    }

    if (res.trackChangesOptionEnabled())
    {
        writer.writeAttribute("updateCount", QString::number(res.updateCount()));
    }

    writer.writeCharacters(res.location().toString());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeHResourceIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    HResource retVal;

    QXmlStreamAttributes attrs = reader->attributes();
    QString location = reader->readElementText().trimmed();

    if (attrs.hasAttribute("protocolInfo"))
    {
        QString pinfo = attrs.value("protocolInfo").toString();

        retVal.setLocation(location);
        retVal.setProtocolInfo(pinfo);
    }

    if (attrs.hasAttribute("updateCount"))
    {
        QString updateCount = attrs.value("updateCount").toString();

        bool ok;
        quint32 value = updateCount.toUInt(&ok);
        if (ok)
        {
            retVal.setUpdateCount(value);
        }
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeClassInfoOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HCdsClassInfo cinfo = value.value<HCdsClassInfo>();
    if (!cinfo.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);

    if (!cinfo.name().isEmpty())
    {
        writer.writeAttribute("name", cinfo.name());
    }

    writer.writeAttribute("includeDerived", cinfo.includeDerived() ? "1" : "0");
    writer.writeCharacters(cinfo.className());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeClassInfoIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString name = attrs.value("name").toString();

    bool inc = true;
    if (attrs.hasAttribute("includeDerived"))
    {
        bool ok = false;
        inc = toBool(attrs.value("includeDerived").toString(), &ok);
        if (!ok)
        {
            HLOG_WARN("Value of attribute [includeDerived] is invalid.");
            inc = true;
        }
    }

    QString className = reader->readElementText().trimmed();

    HCdsClassInfo retVal(className, inc, name);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeRoledPersonOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HPersonWithRole person = value.value<HPersonWithRole>();
    if (!person.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("role", person.role());
    writer.writeCharacters(person.name());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeRoledPersonIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString role = attrs.value("role").toString();

    QString name = reader->readElementText().trimmed();

    HPersonWithRole retVal(name, role);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeContentDurationOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HContentDuration cd = value.value<HContentDuration>();
    if (!cd.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeCharacters(cd.toString());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeContentDurationIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    QString valueAsStr = reader->readElementText().trimmed();

    HContentDuration retVal(valueAsStr);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeWeekDayOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    writer.writeTextElement(property, toString(value.value<HDayOfWeek>()));
    return true;
}

bool HCdsPropertyDbPrivate::serializeWeekDayIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QString tmp = reader->readElementText();
    HDayOfWeek dayOfWeek = dayOfWeekFromString(tmp);

    value->setValue(dayOfWeek);
    return true;
}

bool HCdsPropertyDbPrivate::serializeMatchedIdOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HMatchingId id = value.value<HMatchingId>();
    if (!id.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("type", id.typeAsString());
    writer.writeCharacters(id.value());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeMatchedIdIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    QXmlStreamAttributes attrs = reader->attributes();
    QString type = attrs.value("type").toString();

    QString midValue = reader->readElementText().trimmed();

    HMatchingId retVal(midValue, type);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeProgramCodeOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HProgramCode pc = value.value<HProgramCode>();
    if (!pc.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("type", pc.type());
    writer.writeCharacters(pc.value());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeProgramCodeIn(
    const QString& /*property*/, QVariant* value, QXmlStreamReader* reader) const
{
    QXmlStreamAttributes attrs = reader->attributes();
    QString type = attrs.value("type").toString();

    QString valueAsStr = reader->readElementText().trimmed();

    HProgramCode retVal(valueAsStr, type);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeRatingOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HRating rating = value.value<HRating>();
    if (!rating.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("type", rating.typeAsString());
    writer.writeCharacters(rating.value());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeRatingIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    QXmlStreamAttributes attrs = reader->attributes();
    QString type = attrs.value("type").toString();

    QString ratingValue = reader->readElementText().trimmed();

    HRating retVal(ratingValue, type);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeEpTypeOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HEpisodeType ep = value.value<HEpisodeType>();

    if (ep == EpisodeTypeUndefined)
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeCharacters(toString(ep));
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeEpTypeIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    QString epValue = reader->readElementText().trimmed();

    HEpisodeType retVal = episodeTypeFromString(epValue);
    if (retVal == EpisodeTypeUndefined)
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeChGroupNameOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HChannelGroupName group = value.value<HChannelGroupName>();
    if (!group.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    if (!group.id().isEmpty())
    {
        writer.writeAttribute("id", group.id());
    }
    writer.writeCharacters(group.name());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeChGroupNameIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString id = attrs.value("id").toString();

    QString name = reader->readElementText().trimmed();

    HChannelGroupName retVal(name, id);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializePriceOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HPrice price = value.value<HPrice>();
    if (!price.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("currency", price.currency());
    writer.writeCharacters(QString::number(price.value()));
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializePriceIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString currency = attrs.value("currency").toString();

    QString valueAsStr = reader->readElementText().trimmed();

    bool ok = false;
    float valueAsFloat = valueAsStr.toFloat(&ok);
    if (!ok)
    {
        return false;
    }

    HPrice retVal(valueAsFloat, currency);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeDtRangeOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HDateTimeRange dtRange = value.value<HDateTimeRange>();
    if (!dtRange.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    if (dtRange.daylightSaving() != Unknown_DaylightSaving)
    {
        writer.writeAttribute("daylightSaving", toString(dtRange.daylightSaving()));
    }
    writer.writeCharacters(dtRange.toString());
    writer.writeEndElement();
    return true;
}

bool HCdsPropertyDbPrivate::serializeDtRangeIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString daylightSaving = attrs.value("daylightSaving").toString();

    QString valueAsStr = reader->readElementText().trimmed();

    HDateTimeRange retVal(valueAsStr, daylightSavingFromString(daylightSaving));
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeScheduledTimeOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HScheduledTime time = value.value<HScheduledTime>();
    if (!time.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("usage", HScheduledTime::toString(time.type()));
    if (time.daylightSaving() != Unknown_DaylightSaving)
    {
        writer.writeAttribute("daylightSaving", toString(time.daylightSaving()));
    }

    writer.writeCharacters(time.value().toString(Qt::ISODate));
    writer.writeEndElement();
    return true;
}

bool HCdsPropertyDbPrivate::serializeScheduledTimeIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString usage = attrs.value("usage").toString();
    QString daylightSaving = attrs.value("daylightSaving").toString();

    QString valueAsStr = reader->readElementText().trimmed();
    QDateTime dt = QDateTime::fromString(valueAsStr, Qt::ISODate);

    HScheduledTime::Type usageType =
        usage.isEmpty() ? HScheduledTime::ScheduledProgram : HScheduledTime::fromString(usage);

    HScheduledTime retVal(dt, usageType);
    if (!retVal.isValid())
    {
        return false;
    }

    retVal.setDaylightSaving(daylightSavingFromString(daylightSaving));
    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeStateInfoOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HDeviceUdn info = value.value<HDeviceUdn>();
    if (!info.isValid(StrictChecks))
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("serviceType", info.serviceType().toString());
    writer.writeAttribute("serviceId", info.serviceId().toString());
    writer.writeCharacters(info.udn().toString());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeStateInfoIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString serviceType = attrs.value("serviceType").toString();
    QString serviceId = attrs.value("serviceId").toString();

    HUdn udn = reader->readElementText().trimmed();

    HDeviceUdn retVal(udn, serviceType, serviceId);
    if (!retVal.isValid(LooseChecks))
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeSvCollectionOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HStateVariableCollection svCol = value.value<HStateVariableCollection>();
    if (!svCol.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeAttribute("serviceName", svCol.serviceName());
    writer.writeAttribute("rcsInstanceType", HStateVariableCollection::toString(svCol.rcsInstanceType()));

    QString buf;
    QXmlStreamWriter stateVariableWriter(&buf);

    stateVariableWriter.setCodec("UTF-8");
    stateVariableWriter.writeStartDocument();
    stateVariableWriter.writeStartElement("stateVariableValuePairs");
    stateVariableWriter.writeDefaultNamespace("urn:schemas-upnp-org:av:avs");
    stateVariableWriter.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    stateVariableWriter.writeAttribute("xsi:schemaLocation",
        "urn:schemas-upnp-org:av:avs" \
        "http://www.upnp.org/schemas/av/avs.xsd");

    foreach(const HStateVariableData& sv, svCol.stateVariables())
    {
        stateVariableWriter.writeStartElement("stateVariable");
        if (sv.channel().isValid())
        {
            writer.writeAttribute("channel", sv.channel().toString());
        }
        stateVariableWriter.writeAttribute("variableName", sv.name());
        stateVariableWriter.writeCharacters(sv.value());
        stateVariableWriter.writeEndElement();
    }
    stateVariableWriter.writeEndElement();

    writer.writeCharacters(buf);
    writer.writeEndElement();
    return true;
}

namespace
{
void addNamespaces(QXmlStreamReader& reader)
{
    QXmlStreamNamespaceDeclaration def(
        "", "urn:schemas-upnp-org:av:avs");
    QXmlStreamNamespaceDeclaration xsi(
        "xsi", "http://www.w3.org/2001/XMLSchema-instance");
    QXmlStreamNamespaceDeclaration xsiSchemaLocation(
        "xsi:schemaLocation", "urn:schemas-upnp-org:av:avs\r\nhttp://www.upnp.org/schemas/av/avs.xsd");

    reader.addExtraNamespaceDeclaration(def);
    reader.addExtraNamespaceDeclaration(xsi);
    reader.addExtraNamespaceDeclaration(xsiSchemaLocation);
}
}

bool HCdsPropertyDbPrivate::serializeSvCollectionIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString serviceName = attrs.value("serviceName").toString();
    QString rcsInstanceType = attrs.value("rcsInstanceType").toString();

    QString valueAsStr = reader->readElementText().trimmed();
    QXmlStreamReader stateVariableReader(valueAsStr);
    addNamespaces(stateVariableReader);

    if (stateVariableReader.readNextStartElement())
    {
        if (stateVariableReader.name().compare("stateVariableValuePairs", Qt::CaseInsensitive) != 0)
        {
            return false;
        }
    }
    else
    {
        HLOG_WARN(QString("Failed to parse stateVariableValuePairs data: %1\r\n"
                          "Document: %2").arg(
            stateVariableReader.errorString(), valueAsStr));

        return false;
    }

    QList<HStateVariableData> stateVariables;
    while(!stateVariableReader.atEnd() && stateVariableReader.readNextStartElement())
    {
        QStringRef name = stateVariableReader.name();
        if (name == "stateVariable")
        {
            QXmlStreamAttributes attrs = stateVariableReader.attributes();
            if (!attrs.hasAttribute(QString("variableName")))
            {
                HLOG_WARN(QString("Ignoring state variable definition that lacks the [variableName] attribute."));
                continue;
            }
            else
            {
                QString channel = attrs.value("channel").toString();
                QString svName = attrs.value("variableName").toString();
                QString value = stateVariableReader.readElementText().trimmed();

                HStateVariableData svData(svName, value);
                if (svData.isValid())
                {
                    svData.setChannel(channel);
                    stateVariables.append(svData);
                }
            }
        }
        else
        {
            HLOG_WARN(QString("Encountered unknown XML element: [%1]").arg(name.toString()));
        }
    }

    HStateVariableCollection retVal(
        serviceName, HStateVariableCollection::fromString(rcsInstanceType));

    if (!retVal.isValid())
    {
        return false;
    }

    retVal.setStateVariables(stateVariables);
    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeDescOut(
    const QString& /*property*/, const QVariant&, QXmlStreamWriter&) const
{
    // TODO
    return false;
}

bool HCdsPropertyDbPrivate::serializeDescIn(
    const QString&, QVariant*, QXmlStreamReader*) const
{
    // TODO
    Q_ASSERT(false);
    return false;
}

bool HCdsPropertyDbPrivate::serializeFmDataOut(
    const QString& /*property*/, const QVariant& value, QXmlStreamWriter&) const
{
    // TODO
    HForeignMetadata md = value.value<HForeignMetadata>();
    if (!md.isValid())
    {
        return false;
    }

    return true;
}

bool HCdsPropertyDbPrivate::serializeFmDataIn(
    const QString&, QVariant*, QXmlStreamReader*) const
{
    // TODO
    return false;
}

bool HCdsPropertyDbPrivate::serializeGenreElementOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HGenre genre = value.value<HGenre>();
    if (!genre.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    if (!genre.id().isEmpty())
    {
        writer.writeAttribute("id", genre.id());
    }
    if (!genre.extended().isEmpty())
    {
        writer.writeAttribute("extended", genre.extended().join(","));
    }
    writer.writeCharacters(genre.name());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeGenreElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QXmlStreamAttributes attrs = reader->attributes();
    QString id = attrs.value("id").toString();
    QString extended = attrs.value("extended").toString();

    QString name = reader->readElementText().trimmed();

    HGenre retVal(name, id, extended.isEmpty() ? QStringList() : extended.split(","));
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeStorageMediumOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    HStorageMedium medium = value.value<HStorageMedium>();
    if (!medium.isValid())
    {
        return false;
    }

    writer.writeStartElement(property);
    writer.writeCharacters(medium.toString());
    writer.writeEndElement();

    return true;
}

bool HCdsPropertyDbPrivate::serializeStorageMediumIn(
    const QString& /*property*/, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QString mediumValue = reader->readElementText().trimmed();

    HStorageMedium retVal(mediumValue);
    if (!retVal.isValid())
    {
        return false;
    }

    value->setValue(retVal);
    return true;
}

bool HCdsPropertyDbPrivate::serializeWriteStatusOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    writer.writeTextElement(property, HObject::writeStatusToString(value.value<HObject::WriteStatus>()));
    return true;
}

bool HCdsPropertyDbPrivate::serializeWriteStatusIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    HObject::WriteStatus status = HObject::writeStatusFromString(reader->readElementText());
    value->setValue(status);
    return true;
}

bool HCdsPropertyDbPrivate::serializeElementOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    writer.writeTextElement(property, variantAsString(value));
    return true;
}

bool HCdsPropertyDbPrivate::serializeMultiValuedElementOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    QVariantList list = value.toList();
    foreach(const QVariant& var, list)
    {
        writer.writeTextElement(property, variantAsString(var));
    }
    return true;
}

bool HCdsPropertyDbPrivate::serializeCDSListElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    value->setValue(reader->readElementText().split(","));
    return true;
}

bool HCdsPropertyDbPrivate::serializeElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    value->setValue(reader->readElementText());
    return true;
}

bool HCdsPropertyDbPrivate::serializeBoolElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    bool ok;
    bool boolValue = Herqq::toBool(reader->readElementText(), &ok);
    if (ok)
    {
        value->setValue(boolValue);
    }
    return ok;
}

bool HCdsPropertyDbPrivate::serializeUIntElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    bool ok;
    quint32 uintValue = reader->readElementText().toUInt(&ok);
    if (ok)
    {
        value->setValue(uintValue);
    }
    return ok;
}

bool HCdsPropertyDbPrivate::serializeULongElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    bool ok;
    quint64 uLongValue = reader->readElementText().toULongLong(&ok);
    if (ok)
    {
        value->setValue(uLongValue);
    }
    return ok;
}

bool HCdsPropertyDbPrivate::serializeIntElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    bool ok;
    qint32 intValue = reader->readElementText().toInt(&ok);
    if (ok)
    {
        value->setValue(intValue);
    }
    return ok;
}

bool HCdsPropertyDbPrivate::serializeUriElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QUrl uriValue = reader->readElementText();
    if (!uriValue.isValid() || uriValue.isEmpty())
    {
        return false;
    }
    value->setValue(uriValue);
    return true;
}

bool HCdsPropertyDbPrivate::serializeDateElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    QString tmp = reader->readElementText();
    QDateTime dateTimeValue = QDateTime::fromString(tmp, Qt::ISODate);

    if (!dateTimeValue.isValid())
    {
        return false;
    }
    value->setValue(dateTimeValue);
    return true;
}

bool HCdsPropertyDbPrivate::serializeRadiobandElementOut(
    const QString& property, const QVariant& value, QXmlStreamWriter& writer) const
{
    writer.writeTextElement(property, value.value<HRadioBand>().toString());
    return true;
}

bool HCdsPropertyDbPrivate::serializeRadiobandElementIn(
    const QString&, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value);
    Q_ASSERT(reader);

    HRadioBand radioBand = reader->readElementText();
    value->setValue(radioBand);
    return true;
}

bool HCdsPropertyDbPrivate::serializeAttributeOut(
    const QString& property, const QVariant& value,
    QXmlStreamWriter& writer) const
{
    if (property.startsWith('@'))
    {
        writer.writeAttribute(property.mid(1), variantAsString(value));
    }
    else
    {
        writer.writeAttribute(property, variantAsString(value));
    }
    return true;
}

bool HCdsPropertyDbPrivate::serializeAttributeIn(
    const QString& /*name*/, QVariant* value, QXmlStreamReader* reader) const
{
    Q_ASSERT(value); Q_UNUSED(value)
    Q_ASSERT(!reader); Q_UNUSED(reader)
    // intentional. nothing to do.
    return true;
}

bool HCdsPropertyDbPrivate::compareResources(
    const QVariant& var1, const QVariant& var2, qint32* retVal) const
{
    if (var1.type() != var2.type())
    {
        return false;
    }

    HResource res1 = var1.value<HResource>();
    HResource res2 = var2.value<HResource>();

    if (res1.location().isEmpty())
    {
        *retVal = res2.location().isEmpty() ? 0 : -1;
    }
    else if (res2.location().isEmpty())
    {
        *retVal = 1;
    }
    else
    {
        *retVal = res1.location().toString().compare(res2.location().toString());
    }

    return true;
}

bool HCdsPropertyDbPrivate::compareCaseSensitiveStrings(
    const QVariant& var1, const QVariant& var2, qint32* retVal) const
{
    Q_ASSERT(retVal);

    if (var1.type() != var2.type())
    {
        return false;
    }

    *retVal = var1.toString().compare(var2.toString());
    return true;
}

bool HCdsPropertyDbPrivate::compareUnsignedLongs(
    const QVariant& var1, const QVariant& var2, qint32* retVal) const
{
    Q_ASSERT(retVal);

    if (var1.type() != var2.type())
    {
        return false;
    }

    *retVal = var1.toULongLong() - var2.toULongLong();
    return true;
}

bool HCdsPropertyDbPrivate::compareSignedLongs(
    const QVariant& var1, const QVariant& var2, qint32* retVal) const
{
    Q_ASSERT(retVal);

    if (var1.type() != var2.type())
    {
        return false;
    }

    *retVal = var1.toLongLong() - var2.toLongLong();
    return true;
}

bool HCdsPropertyDbPrivate::compareBools(
    const QVariant& var1, const QVariant& var2, qint32* retVal) const
{
    Q_ASSERT(retVal);

    bool b1 = false, b2 = false;
    if (var1.type() != var2.type())
    {
        return false;
    }
    else if (var1.type() == QVariant::Bool)
    {
        b1 = var1.toBool();
        b2 = var2.toBool();
    }
    else
    {
        bool ok = false;
        b1 = toBool(var1.toString(), &ok);
        if (ok)
        {
            b2 = toBool(var2.toString(), &ok);
        }
        if (!ok)
        {
            return false;
        }
    }

    if (!b1)
    {
        *retVal = b2 ? -1 : 0;
    }
    else
    {
        *retVal = !b2;
    }
    return true;
}

bool HCdsPropertyDbPrivate::compareDates(
    const QVariant& var1, const QVariant& var2, qint32* retVal) const
{
    if (var1.type() != var2.type())
    {
        return false;
    }

    *retVal = var1.toDateTime() < var2.toDateTime();
    return true;
}

bool HCdsPropertyDbPrivate::notEmpty(const QVariant& var) const
{
    return !var.toString().isEmpty();
}

void HCdsPropertyDbPrivate::init()
{
    HValidator notEmptyValidator(this, &HCdsPropertyDbPrivate::notEmpty);

    HComparer caseSensitiveStrComparer(
        this, &HCdsPropertyDbPrivate::compareCaseSensitiveStrings);

    HComparer unsignedComparer(
        this, &HCdsPropertyDbPrivate::compareUnsignedLongs);

    HComparer signedComparer(
        this, &HCdsPropertyDbPrivate::compareSignedLongs);

    HComparer boolComparer(
        this, &HCdsPropertyDbPrivate::compareBools);

    HComparer dateTimeComparer(
        this, &HCdsPropertyDbPrivate::compareDates);

    HCdsPropertyHandler elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeElementIn),
        caseSensitiveStrComparer,
        HValidator()
    );

    HCdsPropertyHandler elemHandler_noEmpty = elemHandler_noValidator;
    elemHandler_noEmpty.setValidator(notEmptyValidator);

    HCdsPropertyHandler bool_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeBoolElementIn),
        boolComparer,
        HValidator()
    );

    HCdsPropertyHandler uint_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeUIntElementIn),
        unsignedComparer,
        HValidator()
    );

    HCdsPropertyHandler ulong_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeULongElementIn),
        unsignedComparer,
        HValidator()
    );

    HCdsPropertyHandler uri_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeUriElementIn),
        caseSensitiveStrComparer,
        HValidator()
    );

    HCdsPropertyHandler int_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeIntElementIn),
        signedComparer,
        HValidator()
    );

    HCdsPropertyHandler date_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeDateElementIn),
        dateTimeComparer,
        HValidator()
    );

    HCdsPropertyHandler genre_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeGenreElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeGenreElementIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler radioband_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeRadiobandElementOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeRadiobandElementIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler attribHandler_noEmpty(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeAttributeOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeAttributeIn),
        caseSensitiveStrComparer,
        notEmptyValidator
    );

    HComparer resourceComparer(this, &HCdsPropertyDbPrivate::compareResources);

    HCdsPropertyHandler resElemHandler(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeHResourceOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeHResourceIn),
        resourceComparer,
        HValidator()
    );

    HCdsPropertyHandler wsElemHandler(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeWriteStatusOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeWriteStatusIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler classInfoElemHandler(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeClassInfoOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeClassInfoIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler roledPersonElemHandler(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeRoledPersonOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeRoledPersonIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler weekDay_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeWeekDayOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeWeekDayIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler matchedId_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeMatchedIdOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeMatchedIdIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler programCode_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeProgramCodeOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeProgramCodeIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler rating_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeRatingOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeRatingIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler epType_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeEpTypeOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeEpTypeIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler contentDuration_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeContentDurationOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeContentDurationIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler chGroupName_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeChGroupNameOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeChGroupNameIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler price_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializePriceOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializePriceIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler dtRange_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeDtRangeOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeDtRangeIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler scheduledTime_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeScheduledTimeOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeScheduledTimeIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler stateInfo_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeStateInfoOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeStateInfoIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler svCollection_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeSvCollectionOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeSvCollectionIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler desc_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeDescOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeDescIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler fmData_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeFmDataOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeFmDataIn),
        HComparer(),
        HValidator()
    );

    HCdsPropertyHandler storageMedium_elemHandler_noValidator(
        HOutSerializer(this, &HCdsPropertyDbPrivate::serializeStorageMediumOut),
        HInSerializer(this, &HCdsPropertyDbPrivate::serializeStorageMediumIn),
        HComparer(),
        HValidator()
    );

    const HCdsProperties& inst = HCdsProperties::instance();

    // Base Properties
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_id), attribHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_parentId), attribHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_refId), attribHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_restricted), attribHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_searchable), attribHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_childCount), attribHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_title), elemHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_creator), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_res), resElemHandler));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_class), elemHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_searchClass), classInfoElemHandler));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_createClass), classInfoElemHandler));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_writeStatus), wsElemHandler));
    // end of base Properties

    // Contributor-related Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_artist), roledPersonElemHandler));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_actor), roledPersonElemHandler));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_author), roledPersonElemHandler));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_producer), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_director), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_publisher), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_contributor), elemHandler_noValidator));
    // end of contributor-related Properties

    // Affiliation-related Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_genre), genre_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_album), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_playList), elemHandler_noValidator));
    // end of affiliation-related Properties

    // Associated resource properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_albumArtURI), uri_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_artistDiscographyURI), uri_elemHandler_noValidator));

    insert(HCdsProperty(inst.get(HCdsProperties::upnp_lyricsURI), uri_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_relation), uri_elemHandler_noValidator));
    // end of associated resource properties

    // Storage related properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_storageTotal), ulong_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_storageUsed), ulong_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_storageFree), ulong_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_storageMaxPartition), ulong_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_storageMedium), storageMedium_elemHandler_noValidator));
    // end of storage related properties

    // General Description properties
    insert(HCdsProperty(inst.get(HCdsProperties::dc_description), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_longDescription), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_icon), uri_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_region), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_rights), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_date), date_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dc_language), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_playbackCount), uint_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_lastPlaybackTime), date_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_lastPlaybackPosition), contentDuration_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_recordedStartDateTime), date_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_recordedDuration), contentDuration_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_recordedDayOfWeek), weekDay_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_srsRecordScheduleID), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_srsRecordTaskID), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_recordable), bool_elemHandler_noValidator));
    // end of General Description properties

    // Recorded Object-related Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_programTitle), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_seriesTitle), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_programID), matchedId_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_seriesID), matchedId_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_channelID), matchedId_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_episodeCount), uint_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_episodeNumber), uint_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_programCode), programCode_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_rating), rating_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_episodeType), epType_elemHandler_noValidator));
    // End of Recorded Object-related Properties

    // User Channel and EPG Related Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_channelGroupName), chGroupName_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_callSign), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_networkAffiliation), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_serviceProvider), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_price), price_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_payPerView), bool_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_epgProviderName), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_dateTimeRange), dtRange_elemHandler_noValidator));
    // End of User Channel and EPG Related Properties

    // Radio Broadcast Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_radioCallSign), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_radioStationID), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_radioBand), radioband_elemHandler_noValidator));
    // End of Radio Broadcast Properties

    // Video Broadcast Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_channelNr), int_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_channelName), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_scheduledStartTime), scheduledTime_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_scheduledEndTime), scheduledTime_elemHandler_noValidator));

    insert(HCdsProperty(inst.get(HCdsProperties::upnp_scheduledDuration), contentDuration_elemHandler_noValidator));
    // End of Video Broadcast Properties

    // Physical Tuner Status-related Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_signalStrength), int_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_signalLocked), bool_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_tuned), bool_elemHandler_noValidator));
    // End of Physical Tuner Status-related Properties

    // Bookmark-related Properties
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_neverPlayable), attribHandler_noEmpty));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_bookmarkID), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_bookmarkedObjectID), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_deviceUdn), stateInfo_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_stateVariableCollection), svCollection_elemHandler_noValidator));
    // End of Bookmark-related Properties

    // Miscellaneous Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_dvdRegionCode), int_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_originalTrackNumber), int_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_toc), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_userAnnotation), elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::dlite_desc), desc_elemHandler_noValidator));
    // End of Miscellaneous Properties

    // Object Tracking Properties
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_containerUpdateID), uint_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_objectUpdateID), uint_elemHandler_noValidator));
    insert(HCdsProperty(inst.get(HCdsProperties::upnp_totalDeletedChildCount), uint_elemHandler_noValidator));
    // End of Object Tracking Properties

    insert(HCdsProperty(inst.get(HCdsProperties::upnp_foreignMetadata), fmData_elemHandler_noValidator));
}

/*******************************************************************************
 * HCdsPropertyDb
 ******************************************************************************/
HCdsPropertyDb* HCdsPropertyDb::s_instance = 0;
QMutex* HCdsPropertyDb::s_instanceLock = new QMutex();

HCdsPropertyDb::HCdsPropertyDb() :
    h_ptr(new HCdsPropertyDbPrivate())
{
    h_ptr->init();
}

HCdsPropertyDb::~HCdsPropertyDb()
{
    delete h_ptr;
}

HCdsPropertyDb& HCdsPropertyDb::instance()
{
    QMutexLocker locker(s_instanceLock);
    if (!s_instance)
    {
        s_instance = new HCdsPropertyDb();
    }

    return *s_instance;
}

HCdsProperty HCdsPropertyDb::property(const QString& property) const
{
    QReadLocker locker(&h_ptr->m_propertiesLock);
    return h_ptr->m_properties.value(property);
}

QSet<QString> HCdsPropertyDb::didlLiteDependentProperties() const
{
    QReadLocker locker(&h_ptr->m_propertiesLock);
    return h_ptr->m_didlLiteDependentProperties;
}

bool HCdsPropertyDb::registerProperty(const HCdsProperty& property)
{
    QWriteLocker locker(&h_ptr->m_propertiesLock);
    if (h_ptr->m_properties.contains(property.info().name()))
    {
        return false;
    }

    h_ptr->insert(property);
    return true;
}

bool HCdsPropertyDb::unregisterProperty(const QString& name)
{
    QWriteLocker locker(&h_ptr->m_propertiesLock);
    if (!h_ptr->m_properties.contains(name) ||
        h_ptr->m_properties.value(name).info().propertyFlags() &
        HCdsPropertyInfo::StandardType)
    {
        return false;
    }

    h_ptr->remove(name);
    return true;
}

}
}
}
