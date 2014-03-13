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

#ifndef HCONTROLPOINT_CONFIGURATION_P_H_
#define HCONTROLPOINT_CONFIGURATION_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../utils/hglobal.h"

#include <QtCore/QList>
#include <QtNetwork/QHostAddress>

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HControlPointConfiguration class
//
class H_UPNP_CORE_EXPORT HControlPointConfigurationPrivate
{
H_DISABLE_COPY(HControlPointConfigurationPrivate)

public: // attributes

    bool m_subscribeToEvents;
    qint32 m_desiredSubscriptionTimeout;
    bool m_autoDiscovery;
    QList<QHostAddress> m_networkAddresses;

public: // methods

    HControlPointConfigurationPrivate();
    virtual ~HControlPointConfigurationPrivate();

    virtual HControlPointConfigurationPrivate* clone() const;
};

}
}

#endif /* HCONTROLPOINT_CONFIGURATION_P_H_ */
