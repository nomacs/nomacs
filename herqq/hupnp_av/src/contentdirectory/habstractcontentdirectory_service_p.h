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

#ifndef HABSTRACTCONTENTDIRECTORY_SERVICE_P_H_
#define HABSTRACTCONTENTDIRECTORY_SERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/private/hserverservice_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAbstractContentDirectoryService;

//
// Implementation details of HAbstractContentDirectoryService
//
class H_UPNP_AV_EXPORT HAbstractContentDirectoryServicePrivate :
    public HServerServicePrivate
{
H_DECLARE_PUBLIC(HAbstractContentDirectoryService)
H_DISABLE_COPY(HAbstractContentDirectoryServicePrivate)

private:

    qint32 getSearchCapabilities(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getSortCapabilities(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getSortExtensionCapabilities(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getFeatureList(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getSystemUpdateID(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getServiceResetToken(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 browse(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 search(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 createObject(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 destroyObject(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 updateObject(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 moveObject(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 importResource(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 exportResource(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 deleteResource(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 stopTransferResource(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getTransferProgress(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 createReference(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 freeFormQuery(
        const HActionArguments& inArgs, HActionArguments* outArgs);

    qint32 getFreeFormQueryCapabilities(
        const HActionArguments& inArgs, HActionArguments* outArgs);

public: // methods

    HAbstractContentDirectoryServicePrivate();
    virtual ~HAbstractContentDirectoryServicePrivate();
};

}
}
}

#endif /* HABSTRACTCONTENTDIRECTORY_SERVICE_P_H_ */
