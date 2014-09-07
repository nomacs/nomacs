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

#ifndef HABSTRACTRENDERINGCONTROL_SERVICE_P_H_
#define HABSTRACTRENDERINGCONTROL_SERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hserverservice_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{
//
// Implementation details of HConnectionManagerService
//
class HAbstractRenderingControlServicePrivate :
    public HServerServicePrivate
{
H_DISABLE_COPY(HAbstractRenderingControlServicePrivate)
H_DECLARE_PUBLIC(HAbstractRenderingControlService)

public:

    HAbstractRenderingControlServicePrivate();
    virtual ~HAbstractRenderingControlServicePrivate();

    qint32 listPresets(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 selectPreset(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getBrightness(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setBrightness(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getContrast(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setContrast(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getSharpness(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setSharpness(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getRedVideoGain(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setRedVideoGain(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getGreenVideoGain(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setGreenVideoGain(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getBlueVideoGain(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setBlueVideoGain(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getRedVideoBlackLevel(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setRedVideoBlackLevel(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getGreenVideoBlackLevel(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setGreenVideoBlackLevel(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getBlueVideoBlackLevel(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setBlueVideoBlackLevel(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getColorTemperature(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setColorTemperature(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getHorizontalKeystone(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setHorizontalKeystone(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getVerticalKeystone(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setVerticalKeystone(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getMute(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setMute(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getVolume(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setVolume(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getVolumeDB(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setVolumeDB(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getVolumeDBRange(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getLoudness(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setLoudness(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getStateVariables(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 setStateVariables(
        const HActionArguments& inArgs, HActionArguments* outArgs);
};

}
}
}

#endif /* HABSTRACTRENDERINGCONTROL_SERVICE_P_H_ */
