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

#ifndef HRENDERINGCONTROL_ADAPTER_P_H_
#define HRENDERINGCONTROL_ADAPTER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/private/hclientservice_adapter_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HRenderingControlAdapterPrivate :
    public HClientServiceAdapterPrivate
{
H_DECLARE_PUBLIC(HRenderingControlAdapter)
H_DISABLE_COPY(HRenderingControlAdapterPrivate)

public:

    quint32 m_instanceId;

    HRenderingControlAdapterPrivate();
    virtual ~HRenderingControlAdapterPrivate();

    bool listPresets(HClientAction*, const HClientActionOp&);
    bool selectPreset(HClientAction*, const HClientActionOp&);
    bool getBrightness(HClientAction*, const HClientActionOp&);
    bool setBrightness(HClientAction*, const HClientActionOp&);
    bool getContrast(HClientAction*, const HClientActionOp&);
    bool setContrast(HClientAction*, const HClientActionOp&);
    bool getSharpness(HClientAction*, const HClientActionOp&);
    bool setSharpness(HClientAction*, const HClientActionOp&);
    bool getRedVideoGain(HClientAction*, const HClientActionOp&);
    bool setRedVideoGain(HClientAction*, const HClientActionOp&);
    bool getGreenVideoGain(HClientAction*, const HClientActionOp&);
    bool setGreenVideoGain(HClientAction*, const HClientActionOp&);
    bool getBlueVideoGain(HClientAction*, const HClientActionOp&);
    bool setBlueVideoGain(HClientAction*, const HClientActionOp&);
    bool getRedVideoBlackLevel(HClientAction*, const HClientActionOp&);
    bool setRedVideoBlackLevel(HClientAction*, const HClientActionOp&);
    bool getGreenVideoBlackLevel(HClientAction*, const HClientActionOp&);
    bool setGreenVideoBlackLevel(HClientAction*, const HClientActionOp&);
    bool getBlueVideoBlackLevel(HClientAction*, const HClientActionOp&);
    bool setBlueVideoBlackLevel(HClientAction*, const HClientActionOp&);
    bool getColorTemperature(HClientAction*, const HClientActionOp&);
    bool setColorTemperature(HClientAction*, const HClientActionOp&);
    bool getHorizontalKeystone(HClientAction*, const HClientActionOp&);
    bool setHorizontalKeystone(HClientAction*, const HClientActionOp&);
    bool getVerticalKeystone(HClientAction*, const HClientActionOp&);
    bool setVerticalKeystone(HClientAction*, const HClientActionOp&);
    bool getMute(HClientAction*, const HClientActionOp&);
    bool setMute(HClientAction*, const HClientActionOp&);
    bool getVolume(HClientAction*, const HClientActionOp&);
    bool setVolume(HClientAction*, const HClientActionOp&);
    bool getVolumeDB(HClientAction*, const HClientActionOp&);
    bool setVolumeDB(HClientAction*, const HClientActionOp&);
    bool getVolumeDBRange(HClientAction*, const HClientActionOp&);
    bool getLoudness(HClientAction*, const HClientActionOp&);
    bool setLoudness(HClientAction*, const HClientActionOp&);
    bool getStateVariables(HClientAction*, const HClientActionOp&);
    bool setStateVariables(HClientAction*, const HClientActionOp&);
};

}
}
}

#endif /* HRENDERINGCONTROL_ADAPTER_P_H_ */
