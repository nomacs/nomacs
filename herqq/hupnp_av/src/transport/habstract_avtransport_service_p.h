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

#ifndef HABSTRACT_AVTRANSPORT_SERVICE_P_H_
#define HABSTRACT_AVTRANSPORT_SERVICE_P_H_

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
//
//
class HAbstractTransportServicePrivate :
    public HServerServicePrivate
{
H_DISABLE_COPY(HAbstractTransportServicePrivate)
H_DECLARE_PUBLIC(HAbstractTransportService)

public:

    HAbstractTransportServicePrivate();
    virtual ~HAbstractTransportServicePrivate();

    qint32 setAVTransportURI(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 setNextAVTransportURI(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getMediaInfo(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getMediaInfo_ext(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getTransportInfo(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getPositionInfo(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getDeviceCapabilities(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getTransportSettings(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 stop(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 play(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 pause(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 record(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 seek(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 next(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 previous(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 setPlayMode(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 setRecordQualityMode(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getCurrentTransportActions(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getDRMState(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 getStateVariables(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    qint32 setStateVariables(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);
};

}
}
}

#endif /* HABSTRACT_AVTRANSPORT_SERVICE_P_H_ */
