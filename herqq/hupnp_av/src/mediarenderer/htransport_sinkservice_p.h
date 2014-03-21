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

#ifndef HTRANSPORT_SINKSERVICE_P_H_
#define HTRANSPORT_SINKSERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../transport/habstract_avtransport_service.h"

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
class HTransportSinkService :
    public HAbstractTransportService
{
Q_OBJECT
H_DISABLE_COPY(HTransportSinkService)

private:

    HMediaRendererDevice* m_owner;

    int m_lastId;

public:

    HTransportSinkService();
    virtual ~HTransportSinkService();

    bool init(HMediaRendererDevice* owner);

    virtual qint32 setAVTransportURI(
        quint32 instanceId, const QUrl& currentUri,
        const QString& currentUriMetaData);

    virtual qint32 setNextAVTransportURI(
        quint32 instanceId, const QUrl& nextUri,
        const QString& nextUriMetaData);

    virtual qint32 getMediaInfo(quint32 instanceId, HMediaInfo*);
    virtual qint32 getMediaInfo_ext(quint32 instanceId, HMediaInfo*);
    virtual qint32 getTransportInfo(quint32 instanceId, HTransportInfo*);
    virtual qint32 getPositionInfo(quint32 instanceId, HPositionInfo*);

    virtual qint32 getDeviceCapabilities(
        quint32 instanceId, HDeviceCapabilities*);

    virtual qint32 getTransportSettings(
        quint32 instanceId, HTransportSettings*);

    virtual qint32 stop(quint32 instanceId);
    virtual qint32 play(quint32 instanceId, const QString&);
    virtual qint32 pause(quint32 instanceId);
    virtual qint32 record(quint32 instanceId);
    virtual qint32 seek(quint32 instanceId, const HSeekInfo&);
    virtual qint32 next(quint32 instanceId);
    virtual qint32 previous(quint32 instanceId);

    virtual qint32 setPlayMode(quint32 instanceId, const HPlayMode&);

    virtual qint32 setRecordQualityMode(
        quint32 instanceId, const HRecordQualityMode&);

    virtual qint32 getCurrentTransportActions(
        quint32 instanceId, QSet<HTransportAction>*);

    virtual qint32 getDrmState(
        quint32 instanceId, HAvTransportInfo::DrmState* drmState);

    virtual qint32 getStateVariables(
        quint32 instanceId, const QSet<QString>& stateVariables,
        QString* stateVariableValuePairs);

    virtual qint32 setStateVariables(
        quint32 instanceId, const HUdn& avtUdn,
        const HResourceType& serviceType, const HServiceId& serviceId,
        const QString& stateVariableValuePairs, QStringList* stateVariableList);

    qint32 nextId();
};

}
}
}

#endif /* HTRANSPORT_SINKSERVICE_P_H_ */
