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

#include "hcds_dlite_serializer.h"
#include "hcds_dlite_serializer_p.h"
#include "hcdsproperty_db.h"
#include "hcdsproperty.h"

#include "../cds_objects/hitem.h"
#include "../cds_objects/halbum.h"
#include "../cds_objects/hmovie.h"
#include "../cds_objects/hphoto.h"
#include "../cds_objects/hperson.h"
#include "../cds_objects/htextitem.h"
#include "../cds_objects/haudioitem.h"
#include "../cds_objects/haudiobook.h"
#include "../cds_objects/hcontainer.h"
#include "../cds_objects/haudiobook.h"
#include "../cds_objects/hvideoitem.h"
#include "../cds_objects/hmusicgenre.h"
#include "../cds_objects/hmoviegenre.h"
#include "../cds_objects/hmusictrack.h"
#include "../cds_objects/hmusicalbum.h"
#include "../cds_objects/hphotoalbum.h"
#include "../cds_objects/hmusicartist.h"
#include "../cds_objects/hphotoalbum.h"
#include "../cds_objects/hepgcontainer.h"
#include "../cds_objects/hchannelgroup.h"
#include "../cds_objects/hbookmarkitem.h"
#include "../cds_objects/hplaylistitem.h"
#include "../cds_objects/haudioprogram.h"
#include "../cds_objects/hvideoprogram.h"
#include "../cds_objects/hstoragesystem.h"
#include "../cds_objects/hstoragevolume.h"
#include "../cds_objects/hstoragefolder.h"
#include "../cds_objects/hgenrecontainer.h"
#include "../cds_objects/hvideobroadcast.h"
#include "../cds_objects/haudiobroadcast.h"
#include "../cds_objects/hmusicvideoclip.h"
#include "../cds_objects/hbookmarkfolder.h"
#include "../cds_objects/haudiochannelgroup.h"
#include "../cds_objects/hvideochannelgroup.h"
#include "../cds_objects/hplaylistcontainer.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <QtCore/QSet>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

/*!
 * \defgroup hupnp_av_cds_om_mgmt Object Model Management
 * \ingroup hupnp_av_cds
 *
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

namespace
{
void addNamespaces(QXmlStreamReader& reader)
{
    QXmlStreamNamespaceDeclaration didl(
        "DIDL-Lite", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
    QXmlStreamNamespaceDeclaration dc(
        "dc", "http://purl.org/dc/elements/1.1/");
    QXmlStreamNamespaceDeclaration upnp(
        "upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
    QXmlStreamNamespaceDeclaration xsi(
        "xsi", "http://www.w3.org/2001/XMLSchema-instance");

    reader.addExtraNamespaceDeclaration(didl);
    reader.addExtraNamespaceDeclaration(dc);
    reader.addExtraNamespaceDeclaration(upnp);
    reader.addExtraNamespaceDeclaration(xsi);
}

QString saveItemToXml(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.name() == "item" || reader.name() == "container");

    QString retVal;
    QXmlStreamWriter writer(&retVal);

    writer.writeStartElement(reader.name().toString());

    while(!reader.atEnd() && reader.readNext())
    {
        switch(reader.tokenType())
        {
        case QXmlStreamReader::StartElement:
            if (reader.name() == "item" || reader.name() == "container")
            {
                goto end;
            }

            writer.writeStartElement(reader.qualifiedName().toString());
            writer.writeAttributes(reader.attributes());
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name() == "item" || reader.name() == "container")
            {
                goto end;
            }

            writer.writeEndElement();
            break;

        case QXmlStreamReader::Characters:
            writer.writeCharacters(reader.text().toString());
            break;

        default:
            continue;
        }
    }

end:
    writer.writeEndElement();

    return retVal;
}
}

/*******************************************************************************
 * HCdsDidlLiteSerializerPrivate
 ******************************************************************************/
namespace
{
inline HObject* createItem          () { return HItem::create(); }
inline HObject* createImageItem     () { return HImageItem::create(); }
inline HObject* createPhoto         () { return HPhoto::create(); }
inline HObject* createAudioItem     () { return HAudioItem::create(); }
inline HObject* createMusicTrack    () { return HMusicTrack::create(); }
inline HObject* createAudioBroadcast() { return HAudioBroadcast::create(); }
inline HObject* createAudioBook     () { return HAudioBook::create(); }
inline HObject* createVideoItem     () { return HVideoItem::create(); }
inline HObject* createMovie         () { return HMovie::create(); }
inline HObject* createVideoBroadcast() { return HVideoBroadcast::create(); }
inline HObject* createMusicVideoClip() { return HMusicVideoClip::create(); }
inline HObject* createPlaylistItem  () { return HPlaylistItem::create(); }
inline HObject* createTextItem      () { return HTextItem::create(); }
inline HObject* createBookmarkItem  () { return HBookmarkItem::create(); }
inline HObject* createEpgItem       () { return HEpgItem::create(); }
inline HObject* createAudioProgram  () { return HAudioProgram::create(); }
inline HObject* createVideoProgram  () { return HVideoProgram::create(); }
inline HObject* createContainer     () { return HContainer::create(); }
inline HObject* createPerson        () { return HPerson::create(); }
inline HObject* createMusicArtist   () { return HMusicArtist::create(); }
inline HObject* createPlaylistContainer() { return HPlaylistContainer::create(); }
inline HObject* createAlbum         () { return HAlbum::create(); }
inline HObject* createMusicAlbum    () { return HMusicAlbum::create(); }
inline HObject* createPhotoAlbum    () { return HPhotoAlbum::create(); }
inline HObject* createGenreContainer() { return HGenreContainer::create(); }
inline HObject* createMusicGenre    () { return HMusicGenre::create(); }
inline HObject* createMovieGenre    () { return HMovieGenre::create(); }
inline HObject* createChannelGroup  () { return HChannelGroup::create(); }
inline HObject* createAudioChannelGroup() { return HAudioChannelGroup::create(); }
inline HObject* createVideoChannelGroup() { return HVideoChannelGroup::create(); }
inline HObject* createEpgContainer  () { return HEpgContainer::create(); }
inline HObject* createStorageSystem () { return HStorageSystem::create(); }
inline HObject* createStorageVolume () { return HStorageVolume::create(); }
inline HObject* createStorageFolder () { return HStorageFolder::create(); }
inline HObject* createBookmarkFolder() { return HBookmarkFolder::create(); }
}

HCdsDidlLiteSerializerPrivate::HCdsDidlLiteSerializerPrivate() :
    m_creatorFunctions(), m_lastErrorDescription()
{
    m_creatorFunctions.insert(HItem::sClass(), createItem);
    m_creatorFunctions.insert(HImageItem::sClass(), createImageItem);
    m_creatorFunctions.insert(HPhoto::sClass(), createPhoto);
    m_creatorFunctions.insert(HAudioItem::sClass(), createAudioItem);
    m_creatorFunctions.insert(HMusicTrack::sClass(), createMusicTrack);
    m_creatorFunctions.insert(HAudioBroadcast::sClass(), createAudioBroadcast);
    m_creatorFunctions.insert(HAudioBook::sClass(), createAudioBook);
    m_creatorFunctions.insert(HVideoItem::sClass(), createVideoItem);
    m_creatorFunctions.insert(HMovie::sClass(), createMovie);
    m_creatorFunctions.insert(HVideoBroadcast::sClass(), createVideoBroadcast);
    m_creatorFunctions.insert(HMusicVideoClip::sClass(), createMusicVideoClip);
    m_creatorFunctions.insert(HPlaylistItem::sClass(), createPlaylistItem);
    m_creatorFunctions.insert(HTextItem::sClass(), createTextItem);
    m_creatorFunctions.insert(HBookmarkItem::sClass(), createBookmarkItem);
    m_creatorFunctions.insert(HEpgItem::sClass(), createEpgItem);
    m_creatorFunctions.insert(HAudioProgram::sClass(), createAudioProgram);
    m_creatorFunctions.insert(HVideoProgram::sClass(), createVideoProgram);

    m_creatorFunctions.insert(HContainer::sClass(), createContainer);
    m_creatorFunctions.insert(HPerson::sClass(), createPerson);
    m_creatorFunctions.insert(HMusicArtist::sClass(), createMusicArtist);
    m_creatorFunctions.insert(HPlaylistContainer::sClass(), createPlaylistContainer);
    m_creatorFunctions.insert(HAlbum::sClass(), createAlbum);
    m_creatorFunctions.insert(HMusicAlbum::sClass(), createMusicAlbum);
    m_creatorFunctions.insert(HPhotoAlbum::sClass(), createPhotoAlbum);
    m_creatorFunctions.insert(HGenreContainer::sClass(), createGenreContainer);
    m_creatorFunctions.insert(HMusicGenre::sClass(), createMusicGenre);
    m_creatorFunctions.insert(HMovieGenre::sClass(), createMovieGenre);
    m_creatorFunctions.insert(HChannelGroup::sClass(), createChannelGroup);
    m_creatorFunctions.insert(HAudioChannelGroup::sClass(), createAudioChannelGroup);
    m_creatorFunctions.insert(HVideoChannelGroup::sClass(), createVideoChannelGroup);
    m_creatorFunctions.insert(HEpgContainer::sClass(), createEpgContainer);
    m_creatorFunctions.insert(HStorageSystem::sClass(), createStorageSystem);
    m_creatorFunctions.insert(HStorageVolume::sClass(), createStorageVolume);
    m_creatorFunctions.insert(HStorageFolder::sClass(), createStorageFolder);
    m_creatorFunctions.insert(HBookmarkFolder::sClass(), createBookmarkFolder);
}

HCdsDidlLiteSerializerPrivate::~HCdsDidlLiteSerializerPrivate()
{
}

bool HCdsDidlLiteSerializerPrivate::serializePropertyFromAttribute(
    HObject* object, const QString& xmlTokenName, const QString& attributeValue)
{
    HLOG(H_AT, H_FUN);

    QString cdsName = QString("@%1").arg(xmlTokenName);
    if (!object->hasCdsProperty(cdsName))
    {
        return false;
    }

    HCdsProperty prop = HCdsPropertyDb::instance().property(cdsName);
    if (!prop.isValid())
    {
        return false;
    }

    HCdsPropertyHandler hnd = prop.handler();

    QVariant value(attributeValue);
    value.convert(prop.info().defaultValue().type());
    /*if (!hnd.inSerializer()(xmlTokenName, &value, 0))
    {
        return false;
    }*/

    HValidator validator = hnd.validator();
    if (validator && !validator(value))
    {
        return false;
    }

    if (!object->setCdsProperty(cdsName, value))
    {
        return false;
    }

    return true;
}

bool HCdsDidlLiteSerializerPrivate::serializePropertyFromElement(
    HObject* object, const QString& propertyName, QXmlStreamReader& reader)
{
    HLOG(H_AT, H_FUN);

    if (!object->hasCdsProperty(propertyName))
    {
        return false;
    }

    HCdsProperty prop = HCdsPropertyDb::instance().property(propertyName);

    if (!prop.isValid())
    {
        return object->serialize(propertyName, 0, &reader);
    }

    QVariant value;
    HCdsPropertyHandler hnd = prop.handler();
    if (hnd.inSerializer()(propertyName, &value, &reader) == false)
    {
        return false;
    }

    HValidator validator = hnd.validator();
    if (validator && !validator(value))
    {
        return false;
    }

    if (prop.info().propertyFlags() & HCdsPropertyInfo::MultiValued)
    {
        QVariant tmp;
        object->getCdsProperty(propertyName, &tmp);
        QVariantList values = tmp.toList();
        values.append(value);
        return object->setCdsProperty(propertyName, values);
    }

    return object->setCdsProperty(propertyName, value);
}

bool HCdsDidlLiteSerializerPrivate::serializeProperty(
    const HObject& object, const QString& propName, const QVariant& value,
    const QSet<QString>& filter, QXmlStreamWriter& writer)
{
    HLOG(H_AT, H_FUN);

    if (!object.isCdsPropertySet(propName) ||
        !object.isCdsPropertyActive(propName))
    {
        return false;
    }

    bool filterOk = filter.contains("*") || filter.contains(propName);

    HCdsProperty prop = HCdsPropertyDb::instance().property(propName);

    if (!prop.isValid())
    {
        if (filterOk)
        {
            if (!object.serialize(propName, value, writer))
            {
                HLOG_DBG(QString("Failed to serialize property: [%1]").arg(propName));
            }
        }
        else
        {
            HLOG_DBG(QString("Failed to serialize property: [%1]").arg(propName));
        }
        return false;
    }

    HCdsPropertyInfo::PropertyFlags pflags = prop.info().propertyFlags();
    if (!filterOk && (!(pflags & HCdsPropertyInfo::Mandatory)))
    {
        return false;
    }
    else if (pflags & HCdsPropertyInfo::MultiValued)
    {
        foreach(const QVariant& var, value.toList())
        {
            if (prop.handler().outSerializer()(propName, var, writer) == false)
            {
                HLOG_DBG(QString("Failed to serialize property: [%1]").arg(propName));
            }
        }
    }
    else
    {
        if (prop.handler().outSerializer()(propName, value, writer) == false)
        {
            HLOG_DBG(QString("Failed to serialize property: [%1]").arg(propName));
        }
    }

    return true;
}

bool HCdsDidlLiteSerializerPrivate::serializeObject(
    const HObject& object, const QSet<QString>& filter,
    QXmlStreamWriter& writer)
{
    HLOG(H_AT, H_FUN);

    writer.writeStartElement(HObject::isItem(object.type()) ? "item" : "container");

    QHash<QString, QVariant> cdsProps = object.cdsProperties();

    QSet<QString> didlLiteDepProps =
        HCdsPropertyDb::instance().didlLiteDependentProperties();

    foreach(const QString& arg, didlLiteDepProps)
    {
        if (cdsProps.contains(arg))
        {
            serializeProperty(object, arg, cdsProps.value(arg), filter, writer);
            cdsProps.remove(arg);
        }
    }

    QHash<QString, QVariant>::const_iterator ci = cdsProps.constBegin();
    for(; ci != cdsProps.constEnd(); ++ci)
    {
        serializeProperty(object, ci.key(), ci.value(), filter, writer);
    }

    writer.writeEndElement();

    return true;
}

HObject* HCdsDidlLiteSerializerPrivate::parseObject(
    QXmlStreamReader& reader, HCdsDidlLiteSerializer::XmlType xtype)
{
    HLOG(H_AT, H_FUN);

    QXmlStreamAttributes attrs = reader.attributes();

    QString itemData = saveItemToXml(reader);

    QXmlStreamReader itemReader(itemData);

    if (xtype == HCdsDidlLiteSerializer::Document)
    {
        addNamespaces(itemReader);
    }
    else
    {
        itemReader.setNamespaceProcessing(false);
    }

    while(!itemReader.atEnd() && itemReader.readNext())
    {
        if (itemReader.tokenType() == QXmlStreamReader::StartElement)
        {
            if (itemReader.qualifiedName() != "upnp:class")
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        QString clazz = itemReader.readElementText();
        HObjectCreator creator = m_creatorFunctions.value(clazz);
        if (!creator)
        {
            m_lastErrorDescription =  QString("Unknown class: [%1]").arg(clazz);
            return 0;
        }

        QScopedPointer<HObject> object(creator());

        foreach(const QXmlStreamAttribute& attr, attrs)
        {
            serializePropertyFromAttribute(
                object.data(), attr.name().toString(), attr.value().toString());
        }

        bool tcoEnabled = false;

        itemReader.clear();
        if (xtype == HCdsDidlLiteSerializer::Document)
        {
            addNamespaces(itemReader);
        }
        else
        {
            itemReader.setNamespaceProcessing(false);
        }
        itemReader.addData(itemData);

        while(!itemReader.atEnd() && itemReader.readNext())
        {
            if (itemReader.tokenType() == QXmlStreamReader::StartElement)
            {
                QString name = itemReader.qualifiedName().toString();

                if (name == "upnp:class")
                {
                    continue;
                }
                else if (!serializePropertyFromElement(object.data(), name, itemReader))
                {
                    if (name == "item" || name == "container")
                    {
                        continue;
                    }
                    else
                    {
                        HLOG_DBG(QString("Couldn't serialize property: %1").arg(name));
                    }
                }
                else if (!tcoEnabled)
                {
                    const HCdsPropertyInfo& info = HCdsProperties::instance().get(name);
                    if (info.isValid() && (
                            info.type() == HCdsProperties::upnp_objectUpdateID ||
                            info.type() == HCdsProperties::upnp_containerUpdateID ||
                            info.type() == HCdsProperties::upnp_totalDeletedChildCount)
                        )
                    {
                        tcoEnabled = true;
                    }
                }
            }
        }

        if (tcoEnabled)
        {
            object->setTrackChangesOption(true);
        }

        return object->validate() ? object.take() : 0;
    }

    return 0;
}

void HCdsDidlLiteSerializerPrivate::writeDidlLiteDocumentInfo(
    QXmlStreamWriter& writer)
{
    writer.setCodec("UTF-8");
    writer.writeStartDocument();
    writer.writeStartElement("DIDL-Lite");
    writer.writeDefaultNamespace("urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
    writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    writer.writeAttribute("xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
    writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    writer.writeAttribute("xsi:schemaLocation",
                      "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/ " \
                      "http://www.upnp.org/schemas/av/didl-lite.xsd " \
                      "urn:schemas-upnp-org:metadata-1-0/upnp/ " \
                      "http://www.upnp.org/schemas/av/upnp.xsd");
}

/*******************************************************************************
 * HCdsDidlLiteSerializer
 ******************************************************************************/
HCdsDidlLiteSerializer::HCdsDidlLiteSerializer() :
    h_ptr(new HCdsDidlLiteSerializerPrivate())
{
}

HCdsDidlLiteSerializer::~HCdsDidlLiteSerializer()
{
    delete h_ptr;
}

QString HCdsDidlLiteSerializer::lastErrorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

bool HCdsDidlLiteSerializer::serializeFromXml(
    const QString& didlLiteDoc, HObjects* retVal, XmlType inputType)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(retVal);

    QXmlStreamReader reader(didlLiteDoc);

    if (inputType == Document)
    {
        addNamespaces(reader);
        if (reader.readNextStartElement())
        {
            if (reader.name().compare("DIDL-Lite", Qt::CaseInsensitive) != 0)
            {
                h_ptr->m_lastErrorDescription = "Missing mandatory DIDL-Lite element";
                return false;
            }
        }
    }
    else
    {
        reader.setNamespaceProcessing(false);
    }

    HObjects tmp;
    while(!reader.atEnd() && reader.readNext())
    {
        switch(reader.tokenType())
        {
        case QXmlStreamReader::StartElement:
            {
                QStringRef name = reader.name();
                if (name == "item" || name == "container")
                {
                     HObject* obj = h_ptr->parseObject(reader, inputType);
                     if (!obj)
                     {
                         return false;
                     }
                     tmp.append(obj);
                }
            }
            break;
        default:
            break;
        }
    }

    if (reader.error() != QXmlStreamReader::NoError)
    {
        h_ptr->m_lastErrorDescription =
            QString("Parse failed: [%1]").arg(reader.errorString());

        return false;
    }

    *retVal = tmp;
    return true;
}

QString HCdsDidlLiteSerializer::serializeToXml(
    const HObject& object, XmlType xmlType)
{
    QSet<QString> filter; filter.insert("*");
    return serializeToXml(object, filter, xmlType);
}

QString HCdsDidlLiteSerializer::serializeToXml(
    const HObject& object, const QSet<QString>& filter, XmlType xmlType)
{
    QString retVal;
    QXmlStreamWriter writer(&retVal);

    if (xmlType == Document)
    {
        h_ptr->writeDidlLiteDocumentInfo(writer);
    }

    if (!h_ptr->serializeObject(object, filter, writer))
    {
        return "";
    }

    if (xmlType == Document)
    {
        writer.writeEndDocument();
    }

    return retVal;
}

QString HCdsDidlLiteSerializer::serializeToXml(const HObjects& objects)
{
    QSet<QString> filter; filter.insert("*");
    return serializeToXml(objects, filter);
}

QString HCdsDidlLiteSerializer::serializeToXml(
    const HObjects& objects, const QSet<QString>& filter)
{
    QString retVal;
    QXmlStreamWriter writer(&retVal);

    h_ptr->writeDidlLiteDocumentInfo(writer);

    foreach(const HObject* obj, objects)
    {
        if (!h_ptr->serializeObject(*obj, filter, writer))
        {
            return "";
        }
    }

    writer.writeEndDocument();

    return retVal;
}

}
}
}
