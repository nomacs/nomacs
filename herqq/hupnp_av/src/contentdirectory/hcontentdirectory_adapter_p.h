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

#ifndef HCONTENTDIRECTORY_P_H_
#define HCONTENTDIRECTORY_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/private/hclientservice_adapter_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
// Implementation details of HContentDirectoryAdapter.
//
class H_UPNP_AV_EXPORT HContentDirectoryAdapterPrivate :
    public HClientServiceAdapterPrivate
{
H_DECLARE_PUBLIC(HContentDirectoryAdapter)
H_DISABLE_COPY(HContentDirectoryAdapterPrivate)

public:

    HContentDirectoryAdapterPrivate();
    virtual ~HContentDirectoryAdapterPrivate();

    bool getSearchCapabilities(
        HClientAction*, const HClientActionOp&);

    bool getSortCapabilities(
        HClientAction*, const HClientActionOp&);

    bool getSortExtensionCapabilities(
        HClientAction*, const HClientActionOp&);

    bool getFeatureList(
        HClientAction*, const HClientActionOp&);

    bool getSystemUpdateID(
        HClientAction*, const HClientActionOp&);

    bool getServiceResetToken(
        HClientAction*, const HClientActionOp&);

    bool browse(
        HClientAction*, const HClientActionOp&);

    bool search(
        HClientAction*, const HClientActionOp&);

    bool createObject(
        HClientAction*, const HClientActionOp&);

    bool destroyObject(
        HClientAction*, const HClientActionOp&);

    bool updateObject(
        HClientAction*, const HClientActionOp&);

    bool moveObject(
        HClientAction*, const HClientActionOp&);

    bool importResource(
        HClientAction*, const HClientActionOp&);

    bool exportResource(
        HClientAction*, const HClientActionOp&);

    bool deleteResource(
        HClientAction*, const HClientActionOp&);

    bool stopTransferResource(
        HClientAction*, const HClientActionOp&);

    bool getTransferProgress(
        HClientAction*, const HClientActionOp&);

    bool createReference(
        HClientAction*, const HClientActionOp&);

    bool freeFormQuery(
        HClientAction*, const HClientActionOp&);

    bool getFreeFormQueryCapabilities(
        HClientAction*, const HClientActionOp&);
};

}
}
}

#endif /* HCONTENTDIRECTORY_P_H_ */
