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

#ifndef HRENDERINGCONTROL_SERVICE_P_H_
#define HRENDERINGCONTROL_SERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpAv/HAbstractRenderingControlService>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMediaRendererDevice;

//
//
//
class HRenderingControlService :
    public HAbstractRenderingControlService
{
Q_OBJECT
H_DISABLE_COPY(HRenderingControlService)

private:

    HMediaRendererDevice* m_owner;
    int m_lastId;

public:

    HRenderingControlService();
    virtual ~HRenderingControlService();

    bool init(HMediaRendererDevice*);

    virtual qint32 listPresets(
        quint32 instanceId, QStringList* currentPresetNameList);

    virtual qint32 selectPreset(
        quint32 instanceId, const QString& presetName);

    virtual qint32 getBrightness(
        quint32 instanceId, quint16* currentBrightness);

    virtual qint32 setBrightness(
        quint32 instanceId, quint16 desiredBrightness);

    virtual qint32 getContrast(
        quint32 instanceId, quint16* currentContrast);

    virtual qint32 setContrast(
        quint32 instanceId, quint16 desiredContrast);

    virtual qint32 getSharpness(
        quint32 instanceId, quint16* currentSharpness);

    virtual qint32 setSharpness(
        quint32 instanceId, quint16 desiredSharpness);

    virtual qint32 getRedVideoGain(
        quint32 instanceId, quint16* currentRedVideoGain);

    virtual qint32 setRedVideoGain(
        quint32 instanceId, quint16 desiredRedVideoGain);

    virtual qint32 getGreenVideoGain(
        quint32 instanceId, quint16* currentGreenVideoGain);

    virtual qint32 setGreenVideoGain(
        quint32 instanceId, quint16 desiredGreenVideoGain);

    virtual qint32 getBlueVideoGain(
        quint32 instanceId, quint16* currentBlueVideoGain);

    virtual qint32 setBlueVideoGain(
        quint32 instanceId, quint16 desiredBlueVideoGain);

    virtual qint32 getRedVideoBlackLevel(
        quint32 instanceId, quint16* currentRedVideoBlackLevel);

    virtual qint32 setRedVideoBlackLevel(
        quint32 instanceId, quint16 desiredRedVideoBlackLevel);

    virtual qint32 getGreenVideoBlackLevel(
        quint32 instanceId, quint16* currentGreenVideoBlackLevel);

    virtual qint32 setGreenVideoBlackLevel(
        quint32 instanceId, quint16 desiredGreenVideoBlackLevel);

    virtual qint32 getBlueVideoBlackLevel(
        quint32 instanceId, quint16* currentBlueVideoBlackLevel);

    virtual qint32 setBlueVideoBlackLevel(
        quint32 instanceId, quint16 desiredBlueVideoBlackLevel);

    virtual qint32 getColorTemperature(
        quint32 instanceId, quint16* currentColorTemperature);

    virtual qint32 setColorTemperature(
        quint32 instanceId, quint16 desiredColorTemperature);

    virtual qint32 getHorizontalKeystone(
        quint32 instanceId, qint16* currentHorizontalKeyStone);

    virtual qint32 setHorizontalKeystone(
        quint32 instanceId, qint16 desiredHorizontalKeyStone);

    virtual qint32 getVerticalKeystone(
        quint32 instanceId, qint16* currentVerticalKeyStone);

    virtual qint32 setVerticalKeystone(
        quint32 instanceId, qint16 desiredVerticalKeyStone);

    virtual qint32 getMute(
        quint32 instanceId, const HChannel& channel, bool* currentlyMuted);

    virtual qint32 setMute(
        quint32 instanceId, const HChannel& channel, bool desiredMute);

    virtual qint32 getVolume(
        quint32 instanceId, const HChannel& channel, quint16* currentVolume);

    virtual qint32 setVolume(
        quint32 instanceId, const HChannel& channel, quint16 desiredVolume);

    virtual qint32 getVolumeDB(
        quint32 instanceId, const HChannel& channel, qint16* currentVolumeDb);

    virtual qint32 setVolumeDB(
        quint32 instanceId, const HChannel& channel, qint16 desiredVolumeDb);

    virtual qint32 getVolumeDBRange(
        quint32 instanceId, const HChannel& channel, HVolumeDbRangeResult*);

    virtual qint32 getLoudness(
        quint32 instanceId, const HChannel& channel, bool* loudnessOn);

    virtual qint32 setLoudness(
        quint32 instanceId, const HChannel& channel, bool loudnessOn);

    virtual qint32 getStateVariables(
        quint32 instanceId, const QSet<QString>& stateVariableNames,
        QString* stateVariableValuePairs);

    virtual qint32 setStateVariables(
        quint32 instanceId, const HUdn& renderingControlUdn,
        const HResourceType& serviceType, const HServiceId& serviceId,
        const QString& stateVariableValuePairs, QStringList* stateVariableList);

    qint32 nextId();
};

}
}
}

#endif /* HRENDERINGCONTROL_SERVICE_P_H_ */
