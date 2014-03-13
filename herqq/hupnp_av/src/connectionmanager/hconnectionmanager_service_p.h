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

#ifndef HCONNECTIONMANAGER_SERVICE_P_H_
#define HCONNECTIONMANAGER_SERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hconnectioninfo.h"
#include "habstractconnectionmanager_service.h"
#include "habstractconnectionmanager_service_p.h"

#include "../common/hprotocolinfo.h"
#include <HUpnpCore/private/hserverservice_p.h>

#include <QtCore/QHash>
#include <QtCore/QSharedPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HConnectionManagerService :
    public HAbstractConnectionManagerService
{
Q_OBJECT
H_DISABLE_COPY(HConnectionManagerService)

private:

    HProtocolInfos m_sinkProtocolInfo;
    HProtocolInfos m_sourceProtocolInfo;
    QHash<quint32, QSharedPointer<HConnectionInfo> > m_currentConnectionIDs;

    int m_lastConnectionId;

private:

    void updateConnectionsList();

public:

    HConnectionManagerService();
    virtual ~HConnectionManagerService() = 0;

    virtual qint32 getProtocolInfo(HProtocolInfoResult*);

    virtual qint32 getCurrentConnectionIDs(QList<quint32>* retVal);
    virtual qint32 getCurrentConnectionInfo(
        qint32 connectionId, HConnectionInfo* retVal);

    void removeConnection(qint32 connectionId);
    void addConnection(const HConnectionInfo&);

    QSharedPointer<HConnectionInfo> createDefaultConnection(const HProtocolInfo&);

    void setSourceProtocolInfo(const HProtocolInfo&);
    void setSinkProtocolInfo(const HProtocolInfo&);
    void setSourceProtocolInfo(const HProtocolInfos&);
    void setSinkProtocolInfo(const HProtocolInfos&);

    static bool isMimetypeValid(
        const QString& contentFormat, const HProtocolInfos& protocolInfos);

    inline const HProtocolInfos& sinkProtocolInfo() const
    {
        return m_sinkProtocolInfo;
    }

    inline const HProtocolInfos& sourceProtocolInfo() const
    {
        return m_sourceProtocolInfo;
    }

    inline const QHash<quint32, QSharedPointer<HConnectionInfo> >& connectionIds() const
    {
        return m_currentConnectionIDs;
    }

    inline int& lastConnectionId()
    {
        return m_lastConnectionId;
    }
};

}
}
}

#endif /* HCONNECTIONMANAGER_SERVICE_P_H_ */
