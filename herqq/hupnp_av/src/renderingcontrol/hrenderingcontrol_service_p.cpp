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

#include "hrenderingcontrol_service_p.h"
#include "hrenderingcontrol_info.h"
#include "hvolumedbrange_result.h"
#include "hchannel.h"

#include "../mediarenderer/hmediarenderer_info.h"
#include "../mediarenderer/hmediarenderer_device_p.h"
#include "../mediarenderer/hrendererconnection.h"
#include "../mediarenderer/hrendererconnection_info.h"

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HStateVariablesSetupData>

#include <HUpnpCore/private/hlogger_p.h>

#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamWriter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRenderingControlService
 ******************************************************************************/
HRenderingControlService::HRenderingControlService() :
    m_owner(0), m_lastId(-1)
{
}

HRenderingControlService::~HRenderingControlService()
{
}

bool HRenderingControlService::init(HMediaRendererDevice* owner)
{
    Q_ASSERT(owner);
    m_owner = owner;
    return true;
}

qint32 HRenderingControlService::listPresets(
    quint32 instanceId, QStringList* currentPresetNameList)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentPresetNameList =
        mediaConnection->rendererConnectionInfo()->presets().toList();

    return UpnpSuccess;
}

qint32 HRenderingControlService::selectPreset(
    quint32 instanceId, const QString& presetName)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->selectPreset(presetName.trimmed());
}

qint32 HRenderingControlService::getBrightness(
    quint32 instanceId, quint16* currentBrightness)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentBrightness =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::Brightness);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setBrightness(
    quint32 instanceId, quint16 desiredBrightness)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::Brightness, desiredBrightness);
}

qint32 HRenderingControlService::getContrast(
    quint32 instanceId, quint16* currentContrast)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentContrast =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::Contrast);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setContrast(
    quint32 instanceId, quint16 desiredContrast)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::Contrast, desiredContrast);
}

qint32 HRenderingControlService::getSharpness(
    quint32 instanceId, quint16* currentSharpness)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentSharpness =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::Sharpness);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setSharpness(
    quint32 instanceId, quint16 desiredSharpness)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::Sharpness, desiredSharpness);
}

qint32 HRenderingControlService::getRedVideoGain(
    quint32 instanceId, quint16* currentRedVideoGain)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentRedVideoGain =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::RedVideoGain);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setRedVideoGain(
    quint32 instanceId, quint16 desiredRedVideoGain)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::RedVideoGain, desiredRedVideoGain);
}

qint32 HRenderingControlService::getGreenVideoGain(
    quint32 instanceId, quint16* currentGreenVideoGain)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentGreenVideoGain =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::GreenVideoGain);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setGreenVideoGain(
    quint32 instanceId, quint16 desiredGreenVideoGain)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::GreenVideoGain, desiredGreenVideoGain);
}

qint32 HRenderingControlService::getBlueVideoGain(
    quint32 instanceId, quint16* currentBlueVideoGain)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentBlueVideoGain =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::BlueVideoGain);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setBlueVideoGain(
    quint32 instanceId, quint16 desiredBlueVideoGain)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::BlueVideoGain, desiredBlueVideoGain);
}

qint32 HRenderingControlService::getRedVideoBlackLevel(
    quint32 instanceId, quint16* currentRedVideoBlackLevel)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentRedVideoBlackLevel =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::RedVideoBlackLevel);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setRedVideoBlackLevel(
    quint32 instanceId, quint16 desiredRedVideoBlackLevel)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::RedVideoBlackLevel, desiredRedVideoBlackLevel);
}

qint32 HRenderingControlService::getGreenVideoBlackLevel(
    quint32 instanceId, quint16* currentGreenVideoBlackLevel)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentGreenVideoBlackLevel =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::GreenVideoBlackLevel);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setGreenVideoBlackLevel(
    quint32 instanceId, quint16 desiredGreenVideoBlackLevel)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::GreenVideoBlackLevel, desiredGreenVideoBlackLevel);
}

qint32 HRenderingControlService::getBlueVideoBlackLevel(
    quint32 instanceId, quint16* currentBlueVideoBlackLevel)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentBlueVideoBlackLevel =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::BlueVideoBlackLevel);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setBlueVideoBlackLevel(
    quint32 instanceId, quint16 desiredBlueVideoBlackLevel)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::BlueVideoBlackLevel, desiredBlueVideoBlackLevel);
}

qint32 HRenderingControlService::getColorTemperature(
    quint32 instanceId, quint16* currentColorTemperature)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentColorTemperature =
        mediaConnection->rendererConnectionInfo()->rcsValue(HRendererConnectionInfo::ColorTemperature);

    return UpnpSuccess;
}

qint32 HRenderingControlService::setColorTemperature(
    quint32 instanceId, quint16 desiredColorTemperature)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setRcsValue(
        HRendererConnectionInfo::ColorTemperature, desiredColorTemperature);
}

qint32 HRenderingControlService::getHorizontalKeystone(
    quint32 instanceId, qint16* currentHorizontalKeyStone)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentHorizontalKeyStone =
        mediaConnection->rendererConnectionInfo()->horizontalKeystone();

    return UpnpSuccess;
}

qint32 HRenderingControlService::setHorizontalKeystone(
    quint32 instanceId, qint16 desiredHorizontalKeyStone)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setHorizontalKeystone(desiredHorizontalKeyStone);
}

qint32 HRenderingControlService::getVerticalKeystone(
    quint32 instanceId, qint16* currentVerticalKeyStone)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    *currentVerticalKeyStone =
        mediaConnection->rendererConnectionInfo()->verticalKeystone();

    return UpnpSuccess;
}

qint32 HRenderingControlService::setVerticalKeystone(
    quint32 instanceId, qint16 desiredVerticalKeyStone)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setVerticalKeystone(desiredVerticalKeyStone);
}

qint32 HRenderingControlService::getMute(
    quint32 instanceId, const HChannel& channel, bool* currentlyMuted)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    bool ok = false;
    *currentlyMuted =
        mediaConnection->rendererConnectionInfo()->muted(channel, &ok);

    if (!ok)
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    return UpnpSuccess;
}

qint32 HRenderingControlService::setMute(
    quint32 instanceId, const HChannel& channel, bool desiredMute)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setMute(channel, desiredMute);
}

qint32 HRenderingControlService::getVolume(
    quint32 instanceId, const HChannel& channel, quint16* currentVolume)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    bool ok = false;

    *currentVolume =
        mediaConnection->rendererConnectionInfo()->volume(channel, &ok);

    if (!ok)
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    return UpnpSuccess;
}

qint32 HRenderingControlService::setVolume(
    quint32 instanceId, const HChannel& channel, quint16 desiredVolume)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setVolume(channel, desiredVolume);
}

qint32 HRenderingControlService::getVolumeDB(
    quint32 instanceId, const HChannel& channel, qint16* currentVolumeDb)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    bool ok = false;

    *currentVolumeDb =
        mediaConnection->rendererConnectionInfo()->volumeDb(channel, &ok);

    if (!ok)
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    return UpnpSuccess;
}

qint32 HRenderingControlService::setVolumeDB(
    quint32 instanceId, const HChannel& channel, qint16 desiredVolumeDb)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setVolumeDb(channel, desiredVolumeDb);
}

qint32 HRenderingControlService::getVolumeDBRange(
    quint32 instanceId, const HChannel& channel, HVolumeDbRangeResult* result)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    bool ok = false;
    *result = mediaConnection->rendererConnectionInfo()->volumeDbRange(channel, &ok);

    if (!ok)
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    return UpnpSuccess;
}

qint32 HRenderingControlService::getLoudness(
    quint32 instanceId, const HChannel& channel, bool* loudnessOn)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    bool ok = false;

    *loudnessOn =
        mediaConnection->rendererConnectionInfo()->loudnessEnabled(channel, &ok);

    if (!ok)
    {
        return HRenderingControlInfo::InvalidChannel;
    }

    return UpnpSuccess;
}

qint32 HRenderingControlService::setLoudness(
    quint32 instanceId, const HChannel& channel, bool loudnessOn)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    return mediaConnection->setLoudness(channel, loudnessOn);
}

qint32 HRenderingControlService::getStateVariables(
    quint32 instanceId, const QSet<QString>& stateVariableNames,
    QString* stateVariableValuePairs)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT(stateVariableValuePairs);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    QString retVal;
    QXmlStreamWriter writer(&retVal);

    writer.setCodec("UTF-8");
    writer.writeStartDocument();
    writer.writeStartElement("stateVariableValuePairs");
    writer.writeDefaultNamespace("urn:schemas-upnp-org:av:avs");
    writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    writer.writeAttribute("xsi:schemaLocation",
        "urn:schemas-upnp-org:av:avs" \
        "http://www.upnp.org/schemas/av/avs.xsd");

    QSet<QString> stateVarNames;
    if (stateVariableNames.contains("*"))
    {
        stateVarNames = HRenderingControlInfo::stateVariablesSetupData().names();
        QSet<QString>::iterator it = stateVarNames.begin();
        for(; it != stateVarNames.end();)
        {
            if (it->startsWith("A_ARG") || *it == "LastChange" ||
                *it == "PresetNameList")
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
        bool ok = false;
        svName = svName.trimmed();

        if (HRendererConnectionInfo::hasChannelAssociated(svName))
        {
            foreach(const HChannel& ch, HChannel::allChannels())
            {
                QString value = mediaConnection->value(svName, ch, &ok);
                if (ok && !value.isEmpty())
                {
                    writer.writeStartElement("stateVariable");
                    writer.writeAttribute("variableName", svName);
                    writer.writeAttribute("channel", ch.toString());
                    writer.writeCharacters(value);
                    writer.writeEndElement();
                }
            }
        }
        else
        {
            QString value = mediaConnection->value(svName, &ok);
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
                return HRenderingControlInfo::InvalidStateVariableList;
            }
        }
    }
    writer.writeEndElement();

    *stateVariableValuePairs = retVal;
    return UpnpSuccess;
}

qint32 HRenderingControlService::setStateVariables(
    quint32 instanceId, const HUdn& renderingControlUdn,
    const HResourceType& serviceType, const HServiceId& serviceId,
    const QString& stateVariableValuePairs, QStringList* stateVariableList)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT(stateVariableList);

    HRendererConnection* mediaConnection =
        m_owner->findConnectionByRcsId(instanceId);

    if (!mediaConnection)
    {
        return HRenderingControlInfo::InvalidInstanceId;
    }

    if (renderingControlUdn.isValid(LooseChecks) &&
        renderingControlUdn != parentDevice()->info().udn())
    {
        HLOG_WARN(QString("setStateVariables() invoked with invalid UDN"
                          "[%1]").arg(renderingControlUdn.toString()));

        return UpnpInvalidArgs;
    }
    else if (serviceType.isValid() &&
            !serviceType.compare(HRenderingControlInfo::supportedServiceType(), HResourceType::Ignore))
    {
        return HRenderingControlInfo::InvalidServiceType;
    }
    else if (serviceId.isValid(LooseChecks) &&
             serviceId != HMediaRendererInfo::defaultRenderingControlId())
    {
        return HRenderingControlInfo::InvalidServiceId;
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
                QString channel = attrs.value("channel").toString();
                QString svName = attrs.value("variableName").toString();
                QString value = reader.readElementText().trimmed();

                if (mediaConnection->setValue(svName, channel, value))
                {
                    stateVariableList->append(svName);
                }
                else
                {
                    HLOG_WARN(QString("Could not set the value of state variable [%1]").arg(svName));
                    stateVariableList->removeDuplicates();
                    return HRenderingControlInfo::InvalidStateVariableValue;
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

qint32 HRenderingControlService::nextId()
{
    return ++m_lastId;
}

}
}
}
