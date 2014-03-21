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

#ifndef HRENDERERCONNECTION_P_H_
#define HRENDERERCONNECTION_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hrendererconnection_info.h"
#include "../connectionmanager/hconnectioninfo.h"

#include <HUpnpCore/HFunctor>

#include <QtCore/QHash>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

typedef Functor<bool, H_TYPELIST_2(const QString&, const HChannel&)> ValueSetter;

class HRendererConnection;

//
//
//
class HRendererConnectionPrivate
{
H_DISABLE_COPY(HRendererConnectionPrivate)

public:

    bool setBrightness(const QString&, const HChannel&);
    bool setContrast(const QString&, const HChannel&);
    bool setSharpness(const QString&, const HChannel&);
    bool setRedVideoGain(const QString&, const HChannel&);
    bool setGreenVideoGain(const QString&, const HChannel&);
    bool setBlueVideoGain(const QString&, const HChannel&);
    bool setRedVideoBlackLevel(const QString&, const HChannel&);
    bool setGreenVideoBlackLevel(const QString&, const HChannel&);
    bool setBlueVideoBlackLevel(const QString&, const HChannel&);
    bool setColorTemperature(const QString&, const HChannel&);
    bool setHorizontalKeystone(const QString&, const HChannel&);
    bool setVerticalKeystone(const QString&, const HChannel&);
    bool setMute(const QString&, const HChannel&);
    bool setVolume(const QString&, const HChannel&);
    bool setVolumeDB(const QString&, const HChannel&);
    bool setLoudness(const QString&, const HChannel&);

public:

    HRendererConnectionInfo* m_info;
    HConnectionInfo* m_connectionInfo;
    HAbstractConnectionManagerService* m_service;
    HRendererConnection* q_ptr;
    QHash<QString, ValueSetter> m_valueSetters;

    HRendererConnectionPrivate();
    virtual ~HRendererConnectionPrivate();
};

}
}
}

#endif /* HRENDERERCONNECTION_P_H_ */
