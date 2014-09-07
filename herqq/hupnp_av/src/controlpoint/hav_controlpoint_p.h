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

#ifndef HAV_CONTROLPOINT_P_H_
#define HAV_CONTROLPOINT_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../hav_fwd.h"
#include "../hav_defs.h"

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HControlPoint>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class H_UPNP_AV_EXPORT HAvControlPointPrivate :
    public HControlPoint
{
Q_OBJECT
H_DISABLE_COPY(HAvControlPointPrivate)

private:

    virtual bool acceptResource(
        const HDiscoveryType& usn, const HEndpoint& source);

    template<typename T>
    static T* searchAdapter(const QList<T*> adapters, HClientDevice* device)
    {
        T* retVal = 0;
        foreach(T* adapter, adapters)
        {
            if (adapter->device()->info().udn() == device->info().udn())
            {
                retVal = adapter;
                break;
            }
        }
        return retVal;
    }

private Q_SLOTS:

    void rootDeviceOnline_(Herqq::Upnp::HClientDevice* device);
    void rootDeviceOffline_(Herqq::Upnp::HClientDevice* device);

public:

    using HControlPoint::configuration;
    using HControlPoint::setError;

    HAvControlPointConfiguration* m_configuration;

    HMediaServerAdapters m_mediaServers;
    HMediaRendererAdapters m_mediaRenderers;

    HAvControlPointPrivate(const HControlPointConfiguration&, HAvControlPoint*);
    virtual ~HAvControlPointPrivate();
};

}
}
}

#endif /* HAV_CONTROLPOINT_P_H_ */
