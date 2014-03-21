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

#ifndef HCDS_DATASOURCE_P_H_
#define HCDS_DATASOURCE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HObject;

//
//
//
class H_UPNP_AV_EXPORT HCdsDataSourcePrivate
{
H_DISABLE_COPY(HCdsDataSourcePrivate)
H_DECLARE_PUBLIC(HCdsDataSource)

public: // attributes

    QScopedPointer<HCdsDataSourceConfiguration> m_configuration;
    QHash<QString, HObject*> m_objectsById;

    QHash<QString, QSet<QString>*> m_objectIdsByParentId;

    bool m_initialized;

    HCdsDataSource* q_ptr;

public: // methods

    HCdsDataSourcePrivate();
    HCdsDataSourcePrivate(const HCdsDataSourceConfiguration&);
    virtual ~HCdsDataSourcePrivate();

    void add(HObject*);
};

}
}
}

#endif /* HCDS_DATASOURCE_P_H_ */
