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

#include "htransport_sinkservice_p.h"
#include "hmediarenderer_device_p.h"
#include "hmediarenderer_info.h"
#include "hrendererconnection_info.h"
#include "hrendererconnection.h"

#include "../transport/hmediainfo.h"
#include "../transport/hpositioninfo.h"
#include "../transport/htransportinfo.h"
#include "../transport/htransportaction.h"
#include "../transport/htransportsettings.h"
#include "../transport/hdevicecapabilities.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HStateVariablesSetupData>

#include <QtCore/QSet>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamWriter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HTransportSinkService
 ******************************************************************************/
HTransportSinkService::HTransportSinkService() :
    HAbstractTransportService(),
        m_owner(0), m_lastId(-1)
{
}

HTransportSinkService::~HTransportSinkService()
{
}

bool HTransportSinkService::init(HMediaRendererDevice* owner)
{
    Q_ASSERT(owner);
    m_owner = owner;
    return true;
}

qint32 HTransportSinkService::setAVTransportURI(
    quint32 instanceId, const QUrl& currentUri,
    const QString& currentUriMetaData)
{
    if (currentUri.isEmpty() || !currentUri.isValid())
    {
        return UpnpInvalidArgs;
    }

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    const HRendererConnectionInfo* info = mediaConnection->rendererConnectionInfo();
    if (info->mediaInfo().currentUri() == currentUri)
    {
        return HAvTransportInfo::ContentBusy;
    }

    if (currentUri.host().isEmpty())
    {
        QString localPath = currentUri.toLocalFile();
        if (!QFile::exists(localPath) && !QDir(localPath).exists())
        {
            return HAvTransportInfo::ResourceNotFound;
        }
        // TODO
    }

    return mediaConnection->setResource(currentUri, currentUriMetaData);
}

qint32 HTransportSinkService::setNextAVTransportURI(
    quint32 instanceId, const QUrl& nextUri, const QString& nextUriMetaData)
{
    if (nextUri.isEmpty() || !nextUri.isValid())
    {
        return UpnpInvalidArgs;
    }

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    const HRendererConnectionInfo* info = mediaConnection->rendererConnectionInfo();
    if (info->mediaInfo().currentUri() == nextUri)
    {
        return HAvTransportInfo::ContentBusy;
    }
    else if (info->mediaInfo().nextUri() == nextUri)
    {
        return UpnpSuccess;
    }

    if (nextUri.host().isEmpty())
    {
        QString localPath = nextUri.toLocalFile();
        if (!QFile::exists(localPath) && !QDir(localPath).exists())
        {
            return HAvTransportInfo::ResourceNotFound;
        }
        // TODO
    }

    return mediaConnection->setNextResource(nextUri, nextUriMetaData);
}

qint32 HTransportSinkService::getMediaInfo(quint32 instanceId, HMediaInfo* retVal)
{
    Q_ASSERT(retVal);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    *retVal = mediaConnection->rendererConnectionInfo()->mediaInfo();
    return UpnpSuccess;
}

qint32 HTransportSinkService::getMediaInfo_ext(
    quint32 instanceId, HMediaInfo* retVal)
{
    return getMediaInfo(instanceId, retVal);
}

qint32 HTransportSinkService::getTransportInfo(
    quint32 instanceId, HTransportInfo* retVal)
{
    Q_ASSERT(retVal);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    *retVal = mediaConnection->rendererConnectionInfo()->transportInfo();
    return UpnpSuccess;
}

qint32 HTransportSinkService::getPositionInfo(
    quint32 instanceId, HPositionInfo* retVal)
{
    Q_ASSERT(retVal);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    *retVal = mediaConnection->rendererConnectionInfo()->positionInfo();
    return UpnpSuccess;
}

qint32 HTransportSinkService::getDeviceCapabilities(
    quint32 instanceId, HDeviceCapabilities* retVal)
{
    Q_ASSERT(retVal);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    *retVal = mediaConnection->rendererConnectionInfo()->deviceCapabilities();
    return UpnpSuccess;
}

qint32 HTransportSinkService::stop(quint32 instanceId)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->stop();
}

qint32 HTransportSinkService::play(
    quint32 instanceId, const QString& speed)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->play(speed);
}

qint32 HTransportSinkService::pause(quint32 instanceId)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->pause();
}

qint32 HTransportSinkService::record(quint32 instanceId)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->record();
}

qint32 HTransportSinkService::seek(quint32 instanceId, const HSeekInfo& seekInfo)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->seek(seekInfo);
}

qint32 HTransportSinkService::next(quint32 instanceId)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->next();
}

qint32 HTransportSinkService::previous(quint32 instanceId)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->previous();
}

qint32 HTransportSinkService::setPlayMode(
    quint32 instanceId, const HPlayMode& playMode)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->setPlaymode(playMode);
}

qint32 HTransportSinkService::setRecordQualityMode(
    quint32 instanceId,  const HRecordQualityMode& rqMode)
{
    HRendererConnection* mediaConnection =
        m_owner->findConnectionByAvTransportId(instanceId);

    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    return mediaConnection->setRecordQualityMode(rqMode);
}

qint32 HTransportSinkService::getTransportSettings(
    quint32 instanceId, HTransportSettings* retVal)
{
    Q_ASSERT(retVal);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    *retVal = mediaConnection->rendererConnectionInfo()->transportSettings();
    return UpnpSuccess;
}

qint32 HTransportSinkService::getCurrentTransportActions(
    quint32 instanceId, QSet<HTransportAction>* retVal)
{
    Q_ASSERT(retVal);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    *retVal = mediaConnection->rendererConnectionInfo()->currentTransportActions();
    return UpnpSuccess;
}

qint32 HTransportSinkService::getDrmState(
    quint32 instanceId, HAvTransportInfo::DrmState* retVal)
{
    Q_ASSERT(retVal);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    *retVal = mediaConnection->rendererConnectionInfo()->drmState();
    return UpnpSuccess;
}

//namespace
//{
//void addNamespaces(QXmlStreamReader& reader)
//{
//    QXmlStreamNamespaceDeclaration def(
//        "", "urn:schemas-upnp-org:av:avs");
//    QXmlStreamNamespaceDeclaration xsi(
//        "xsi", "http://www.w3.org/2001/XMLSchema-instance");
//    QXmlStreamNamespaceDeclaration xsiSchemaLocation(
//        "xsi:schemaLocation", "urn:schemas-upnp-org:av:avs\r\nhttp://www.upnp.org/schemas/av/avs.xsd");

//    reader.addExtraNamespaceDeclaration(def);
//    reader.addExtraNamespaceDeclaration(xsi);
//    reader.addExtraNamespaceDeclaration(xsiSchemaLocation);
//}
//}

qint32 HTransportSinkService::getStateVariables(
    quint32 instanceId, const QSet<QString>& stateVariableNames,
    QString* stateVariableValuePairs)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT(stateVariableValuePairs);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    QString retVal;
    QXmlStreamWriter writer(&retVal);

    writer.setCodec("UTF-8");
    writer.writeStartDocument();
    writer.writeStartElement("stateVariableValuePairs");
    writer.writeDefaultNamespace("urn:schemas-upnp-org:av:avs");
    writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    writer.writeAttribute("xsi:schemaLocation",
        "urn:schemas-upnp-org:av:avs " \
        "http://www.upnp.org/schemas/av/avs.xsd");

    QSet<QString> stateVarNames;
    if (stateVariableNames.contains("*"))
    {
        stateVarNames = HAvTransportInfo::stateVariablesSetupData().names();
        QSet<QString>::iterator it = stateVarNames.begin();
        for(; it != stateVarNames.end();)
        {
            if (it->startsWith("A_ARG") || *it == "LastChange")
            {
                it = stateVarNames.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    else
    {
        stateVarNames = stateVariableNames;
    }

    foreach(QString svName, stateVarNames)
    {
        svName = svName.trimmed();
        if (svName.compare("LastChange", Qt::CaseInsensitive) == 0 ||
            svName.startsWith("A_ARG", Qt::CaseInsensitive))
        {
            return HAvTransportInfo::InvalidStateVariableList;
        }

        bool ok = false;
        QString value = mediaConnection->rendererConnectionInfo()->value(svName, &ok);
        if (ok)
        {
            writer.writeStartElement("stateVariable");
            writer.writeAttribute("variableName", svName);
            writer.writeCharacters(value);
            writer.writeEndElement();
        }
        else
        {
            HLOG_WARN(QString("Could not get the value of state variable [%1]").arg(svName));
            return HAvTransportInfo::InvalidStateVariableList;
        }
    }
    writer.writeEndElement();

    *stateVariableValuePairs = retVal;
    return UpnpSuccess;
}

qint32 HTransportSinkService::setStateVariables(
    quint32 instanceId, const HUdn& avtUdn,
    const HResourceType& serviceType, const HServiceId& serviceId,
    const QString& stateVariableValuePairs, QStringList* stateVariableList)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT(stateVariableList);

    HRendererConnection* mediaConnection = m_owner->findConnectionByAvTransportId(instanceId);
    if (!mediaConnection)
    {
        return HAvTransportInfo::InvalidInstanceId;
    }

    if (avtUdn.isValid(LooseChecks) && avtUdn != parentDevice()->info().udn())
    {
        HLOG_WARN(QString("setStateVariables() invoked with invalid UDN [%1]").arg(avtUdn.toString()));
        return UpnpInvalidArgs;
    }
    else if (serviceType.isValid() &&
            !HAvTransportInfo::supportedServiceType().compare(serviceType, HResourceType::Inclusive))
    {
        return HAvTransportInfo::InvalidServiceType;
    }
    else if (serviceId.isValid(LooseChecks) &&
             serviceId != HMediaRendererInfo::defaultAvTransportId())
    {
        return HAvTransportInfo::InvalidServiceId;
    }

    QXmlStreamReader reader(stateVariableValuePairs.trimmed());
    //addNamespaces(reader);

    if (reader.readNextStartElement())
    {
        if (reader.name().compare("stateVariableValuePairs", Qt::CaseInsensitive) != 0)
        {
            return UpnpInvalidArgs;
        }
    }
    else
    {
        if (reader.error() != QXmlStreamReader::NoError)
        {
            HLOG_WARN(QString("XML parse failed: %1").arg(reader.errorString()));
        }
        return UpnpInvalidArgs;
    }

    stateVariableList->clear();
    while(!reader.atEnd() && reader.readNextStartElement())
    {
        QStringRef name = reader.name();
        if (name == "stateVariable")
        {
            QXmlStreamAttributes attrs = reader.attributes();
            if (!attrs.hasAttribute(QString("variableName")))
            {
                HLOG_WARN(QString("Ignoring state variable value pair definition that lacks the [variableName] attribute."));
            }
            else
            {
                QString svName = attrs.value("variableName").toString();
                QString value = reader.readElementText().trimmed();

                if (mediaConnection->setValue(svName, value))
                {
                    stateVariableList->append(svName);
                }
                else
                {
                    HLOG_WARN(QString("Could not set the value of state variable [%1]").arg(svName));
                    stateVariableList->removeDuplicates();
                    return HAvTransportInfo::InvalidStateVariableValue;
                }
            }
        }
        else
        {
            HLOG_WARN(QString("Encountered unknown XML element: [%1]").arg(name.toString()));
        }
    }

    stateVariableList->removeDuplicates();
    return UpnpSuccess;
}

qint32 HTransportSinkService::nextId()
{
    return ++m_lastId;
}

}
}
}
