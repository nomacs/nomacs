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

#include "habstractrenderingcontrol_service.h"
#include "habstractrenderingcontrol_service_p.h"

#include "hchannel.h"
#include "hvolumedbrange_result.h"

#include <HUpnpCore/private/hlogger_p.h>

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HActionArguments>

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractRenderingControlServicePrivate
 ******************************************************************************/
HAbstractRenderingControlServicePrivate::HAbstractRenderingControlServicePrivate() :
    HServerServicePrivate()
{
}

HAbstractRenderingControlServicePrivate::~HAbstractRenderingControlServicePrivate()
{
}

qint32 HAbstractRenderingControlServicePrivate::listPresets(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    QStringList currentPresetNameList;
    qint32 retVal = q->listPresets(instanceId, &currentPresetNameList);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentPresetNameList", currentPresetNameList.join(","));
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::selectPreset(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QString preset = inArgs.value("PresetName").toString();

    return q->selectPreset(instanceId, preset);
}

qint32 HAbstractRenderingControlServicePrivate::getBrightness(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getBrightness(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentBrightness", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setBrightness(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredBrightness").toUInt();

    return q->setBrightness(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getContrast(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getContrast(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentContrast", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setContrast(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredContrast").toUInt();

    return q->setContrast(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getSharpness(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getSharpness(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentSharpness", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setSharpness(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredSharpness").toUInt();

    return q->setSharpness(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getRedVideoGain(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getRedVideoGain(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentRedVideoGain", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setRedVideoGain(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredRedVideoGain").toUInt();

    return q->setRedVideoGain(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getGreenVideoGain(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getGreenVideoGain(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentGreenVideoGain", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setGreenVideoGain(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredGreenVideoGain").toUInt();

    return q->setGreenVideoGain(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getBlueVideoGain(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getBlueVideoGain(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentBlueVideoGain", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setBlueVideoGain(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredBlueVideoGain").toUInt();

    return q->setBlueVideoGain(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getRedVideoBlackLevel(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getRedVideoBlackLevel(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentRedVideoBlackLevel", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setRedVideoBlackLevel(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredRedVideoBlackLevel").toUInt();

    return q->setRedVideoBlackLevel(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getGreenVideoBlackLevel(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getGreenVideoBlackLevel(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentGreenVideoBlackLevel", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setGreenVideoBlackLevel(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredGreenVideoBlackLevel").toUInt();

    return q->setGreenVideoBlackLevel(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getBlueVideoBlackLevel(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getBlueVideoBlackLevel(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentBlueVideoBlackLevel", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setBlueVideoBlackLevel(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredBlueVideoBlackLevel").toUInt();

    return q->setBlueVideoBlackLevel(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getColorTemperature(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    quint16 arg;
    qint32 retVal = q->getColorTemperature(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentColorTemperature", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setColorTemperature(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredColorTemperature").toUInt();

    return q->setColorTemperature(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getHorizontalKeystone(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    qint16 arg;
    qint32 retVal = q->getHorizontalKeystone(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentHorizontalKeystone", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setHorizontalKeystone(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredHorizontalKeystone").toUInt();

    return q->setHorizontalKeystone(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getVerticalKeystone(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();

    qint16 arg;
    qint32 retVal = q->getVerticalKeystone(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentVerticalKeystone", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setVerticalKeystone(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    quint16 arg = inArgs.value("DesiredVerticalKeystone").toUInt();

    return q->setVerticalKeystone(instanceId, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getMute(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HChannel channel = inArgs.value("Channel").toString();

    bool arg;
    qint32 retVal = q->getMute(instanceId, channel, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentMute", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setMute(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    bool arg = inArgs.value("DesiredMute").toBool();
    HChannel channel = inArgs.value("Channel").toString();

    return q->setMute(instanceId, channel, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getVolume(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HChannel channel = inArgs.value("Channel").toString();

    quint16 arg;
    qint32 retVal = q->getVolume(instanceId, channel, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentVolume", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setVolume(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HChannel channel = inArgs.value("Channel").toString();
    quint16 arg = inArgs.value("DesiredVolume").toUInt();

    return q->setVolume(instanceId, channel, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getVolumeDB(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HChannel channel = inArgs.value("Channel").toString();

    qint16 arg;
    qint32 retVal = q->getVolumeDB(instanceId, channel, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentVolume", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setVolumeDB(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    qint16 arg = inArgs.value("DesiredVolume").toInt();
    HChannel channel = inArgs.value("Channel").toString();

    return q->setVolumeDB(instanceId, channel, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getVolumeDBRange(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HChannel channel = inArgs.value("Channel").toString();

    HVolumeDbRangeResult result;
    qint32 retVal = q->getVolumeDBRange(instanceId, channel, &result);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("MinValue", result.minValue());
        outArgs->setValue("MaxValue", result.maxValue());
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::getLoudness(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HChannel channel = inArgs.value("Channel").toString();

    bool arg;
    qint32 retVal = q->getLoudness(instanceId, channel, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("CurrentLoudness", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setLoudness(
    const HActionArguments& inArgs, HActionArguments*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    bool arg = inArgs.value("DesiredLoudness").toBool();
    HChannel channel = inArgs.value("Channel").toString();

    return q->setLoudness(instanceId, channel, arg);
}

qint32 HAbstractRenderingControlServicePrivate::getStateVariables(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    QSet<QString> svList = inArgs.value("StateVariableList").toString().split(",").toSet();

    QString arg;
    qint32 retVal = q->getStateVariables(instanceId, svList, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("StateVariableValuePairs", arg);
    }

    return retVal;
}

qint32 HAbstractRenderingControlServicePrivate::setStateVariables(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    H_Q(HAbstractRenderingControlService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value("InstanceID").toUInt();
    HUdn udn = inArgs.value("RenderingControlUDN").toString();
    HResourceType rt = HResourceType(inArgs.value("ServiceType").toString());
    HServiceId sid = inArgs.value("ServiceId").toString();
    QString svValuePairs = inArgs.value("StateVariableValuePairs").toString();

    QStringList arg;
    qint32 retVal = q->setStateVariables(
        instanceId, udn, rt, sid, svValuePairs, &arg);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue("StateVariableList", arg.join(","));
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractRenderingControlService
 ******************************************************************************/
HAbstractRenderingControlService::HAbstractRenderingControlService(
    HAbstractRenderingControlServicePrivate& dd) :
        HServerService(dd)
{
}

HAbstractRenderingControlService::HAbstractRenderingControlService() :
    HServerService(*new HAbstractRenderingControlServicePrivate())
{
}

HAbstractRenderingControlService::~HAbstractRenderingControlService()
{
}

HServerService::HActionInvokes HAbstractRenderingControlService::createActionInvokes()
{
    H_D(HAbstractRenderingControlService);

    HActionInvokes retVal;

    retVal.insert(
        "ListPresets",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::listPresets));

    retVal.insert(
        "SelectPreset",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::selectPreset));

    retVal.insert(
        "GetBrightness",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getBrightness));

    retVal.insert(
        "SetBrightness",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setBrightness));

    retVal.insert(
        "GetContrast",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getContrast));

    retVal.insert(
        "SetContrast",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setContrast));

    retVal.insert(
        "GetSharpness",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getSharpness));

    retVal.insert(
        "SetSharpness",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setSharpness));

    retVal.insert(
        "GetRedVideoGain",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getRedVideoGain));

    retVal.insert(
        "SetRedVideoGain",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setRedVideoGain));

    retVal.insert(
        "GetGreenVideoGain",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getGreenVideoGain));

    retVal.insert(
        "SetGreenVideoGain",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setGreenVideoGain));

    retVal.insert(
        "GetBlueVideoGain",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getBlueVideoGain));

    retVal.insert(
        "SetBlueVideoGain",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setBlueVideoGain));

    retVal.insert(
        "GetRedVideoBlackLevel",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getRedVideoBlackLevel));

    retVal.insert(
        "SetRedVideoBlackLevel",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setRedVideoBlackLevel));

    retVal.insert(
        "GetGreenVideoBlackLevel",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getGreenVideoBlackLevel));

    retVal.insert(
        "SetGreenVideoBlackLevel",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setGreenVideoBlackLevel));

    retVal.insert(
        "GetBlueVideoBlackLevel",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getBlueVideoBlackLevel));

    retVal.insert(
        "SetBlueVideoBlackLevel",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setBlueVideoBlackLevel));

    retVal.insert(
        "GetColorTemperature",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getColorTemperature));

    retVal.insert(
        "SetColorTemperature",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setColorTemperature));

    retVal.insert(
        "GetHorizontalKeystone",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getHorizontalKeystone));

    retVal.insert(
        "SetHorizontalKeystone",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setHorizontalKeystone));

    retVal.insert(
        "GetVerticalKeystone",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getVerticalKeystone));

    retVal.insert(
        "SetVerticalKeystone",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setVerticalKeystone));

    retVal.insert(
        "GetMute",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getMute));

    retVal.insert(
        "SetMute",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setMute));

    retVal.insert(
        "GetVolume",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getVolume));

    retVal.insert(
        "SetVolume",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setVolume));

    retVal.insert(
        "GetVolumeDB",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getVolumeDB));

    retVal.insert(
        "SetVolumeDB",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setVolumeDB));

    retVal.insert(
        "GetVolumeDBRange",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getVolumeDBRange));

    retVal.insert(
        "GetLoudness",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getLoudness));

    retVal.insert(
        "SetLoudness",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setLoudness));

    retVal.insert(
        "GetStateVariables",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::getStateVariables));

    retVal.insert(
        "SetStateVariables",
        HActionInvoke(h, &HAbstractRenderingControlServicePrivate::setStateVariables));

    return retVal;
}

qint32 HAbstractRenderingControlService::getBrightness(
    quint32 /*instanceId*/, quint16* /*currentBrightness*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setBrightness(
    quint32 /*instanceId*/, quint16 /*desiredBrightness*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getContrast(
    quint32 /*instanceId*/, quint16* /*currentContrast*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setContrast(
    quint32 /*instanceId*/, quint16 /*desiredContrast*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getSharpness(
    quint32 /*instanceId*/, quint16* /*currentSharpness*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setSharpness(
    quint32 /*instanceId*/, quint16 /*desiredSharpness*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getRedVideoGain(
    quint32 /*instanceId*/, quint16* /*currentRedVideoGain*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setRedVideoGain(
    quint32 /*instanceId*/, quint16 /*desiredRedVideoGain*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getGreenVideoGain(
    quint32 /*instanceId*/, quint16* /*currentGreenVideoGain*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setGreenVideoGain(
    quint32 /*instanceId*/, quint16 /*desiredGreenVideoGain*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getBlueVideoGain(
    quint32 /*instanceId*/, quint16* /*currentBlueVideoGain*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setBlueVideoGain(
    quint32 /*instanceId*/, quint16 /*desiredBlueVideoGain*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getRedVideoBlackLevel(
    quint32 /*instanceId*/, quint16* /*currentRedVideoBlackLevel*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setRedVideoBlackLevel(
    quint32 /*instanceId*/, quint16 /*desiredRedVideoBlackLevel*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getGreenVideoBlackLevel(
    quint32 /*instanceId*/, quint16* /*currentGreenVideoBlackLevel*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setGreenVideoBlackLevel(
    quint32 /*instanceId*/, quint16 /*desiredGreenVideoBlackLevel*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getBlueVideoBlackLevel(
    quint32 /*instanceId*/, quint16* /*currentBlueVideoBlackLevel*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setBlueVideoBlackLevel(
    quint32 /*instanceId*/, quint16 /*desiredBlueVideoBlackLevel*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getColorTemperature(
    quint32 /*instanceId*/, quint16* /*currentColorTemperature*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setColorTemperature(
    quint32 /*instanceId*/, quint16 /*desiredColorTemperature*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getHorizontalKeystone(
    quint32 /*instanceId*/, qint16* /*currentHorizontalKeyStone*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setHorizontalKeystone(
    quint32 /*instanceId*/, qint16 /*desiredHorizontalKeyStone*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getVerticalKeystone(
    quint32 /*instanceId*/, qint16* /*currentVerticalKeyStone*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setVerticalKeystone(
    quint32 /*instanceId*/, qint16 /*desiredVerticalKeyStone*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getMute(
    quint32 /*instanceId*/, const HChannel& /*channel*/, bool* /*currentlyMuted*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setMute(
    quint32 /*instanceId*/, const HChannel& /*channel*/, bool /*desiredMute*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getVolume(
    quint32 /*instanceId*/, const HChannel& /*channel*/, quint16* /*currentVolume*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setVolume(
    quint32 /*instanceId*/, const HChannel& /*channel*/, quint16 /*desiredVolume*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getVolumeDB(
    quint32 /*instanceId*/, const HChannel& /*channel*/, qint16* /*currentVolumeDb*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setVolumeDB(
    quint32 /*instanceId*/, const HChannel& /*channel*/, qint16 /*desiredVolumeDb*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getVolumeDBRange(
    quint32 /*instanceId*/, const HChannel& /*channel*/, HVolumeDbRangeResult*)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getLoudness(
    quint32 /*instanceId*/, const HChannel& /*channel*/, bool* /*loudnessOn*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setLoudness(
    quint32 /*instanceId*/, const HChannel& /*channel*/, bool /*loudnessOn*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::getStateVariables(
    quint32 /*instanceId*/, const QSet<QString>& /*stateVariableNames*/,
    QString* /*stateVariableValuePairs*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractRenderingControlService::setStateVariables(
    quint32 /*instanceId*/, const HUdn& /*renderingControlUdn*/,
    const HResourceType& /*serviceType*/, const HServiceId& /*serviceId*/,
    const QString& /*stateVariableValuePairs*/, QStringList* /*stateVariableList*/)
{
    return UpnpOptionalActionNotImplemented;
}

}
}
}
