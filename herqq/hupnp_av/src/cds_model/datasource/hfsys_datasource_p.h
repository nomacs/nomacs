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

#ifndef HFILESYSTEM_DATASOURCE_P_H_
#define HFILESYSTEM_DATASOURCE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "habstract_cds_datasource_p.h"

#include <QtCore/QScopedPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class H_UPNP_AV_EXPORT HFileSystemDataSourcePrivate :
    public HAbstractCdsDataSourcePrivate
{
H_DISABLE_COPY(HFileSystemDataSourcePrivate)
H_DECLARE_PUBLIC(HFileSystemDataSource)

public: // attributes

    QHash<QString, QString> m_itemPaths;
    // key == object id, value == file system path

    QScopedPointer<HCdsFileSystemReader> m_fsysReader;

public: // methods

    using HAbstractCdsDataSourcePrivate::add;

    HFileSystemDataSourcePrivate();
    HFileSystemDataSourcePrivate(const HFileSystemDataSourceConfiguration&);

    virtual ~HFileSystemDataSourcePrivate();

    bool add(
        HCdsObjectData* item,
        HFileSystemDataSource::AddFlag addFlag=HFileSystemDataSource::AddNewOnly);

    bool add(
        const QList<HCdsObjectData*> items,
        HFileSystemDataSource::AddFlag addFlag=HFileSystemDataSource::AddNewOnly);

    inline HFileSystemDataSourceConfiguration* configuration() const
    {
        return static_cast<HFileSystemDataSourceConfiguration*>(m_configuration.data());
    }
};

}
}
}

#endif /* HFILESYSTEM_DATASOURCE_P_H_ */
