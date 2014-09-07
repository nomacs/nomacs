/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HDEVICESTATUS_H_
#define HDEVICESTATUS_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

/*!
 *
 */
class HDeviceStatus
{

private:

    qint32 m_bootId;
    qint32 m_configId;
    qint32 m_searchPort;

    bool m_online;

public:

    HDeviceStatus() :
        m_bootId(0), m_configId(0), m_searchPort(0), m_online(true)
    {
    }

    inline qint32 bootId() const { return m_bootId; }

    inline void setBootId(qint32 arg) { m_bootId = arg; }

    inline qint32 configId() const { return m_configId; }

    inline void setConfigId(qint32 arg) { m_configId = arg; }

    inline qint32 searchPort() const { return m_searchPort; }

    inline void setSearchPort(qint32 arg) { m_searchPort = arg; }

    inline bool online() const { return m_online; }
    inline void setOnline(bool arg) { m_online = arg; }
};

}
}

#endif /* HDEVICESTATUS_H_ */
