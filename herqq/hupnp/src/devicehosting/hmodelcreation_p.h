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

#ifndef HMODELCREATOR_P_H_
#define HMODELCREATOR_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../general/hupnp_global.h"
#include "../utils/hfunctor.h"

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

//
//
//
typedef Functor<bool, H_TYPELIST_3(const QUrl&, const QUrl&, QString*)>
    ServiceDescriptionFetcher;

//
//
//
typedef Functor<bool, H_TYPELIST_3(const QUrl&, const QUrl&, QByteArray*)>
    IconFetcher;

//
// A class that contains information for the creation of HUPnP's device model
// This information is set by the HDeviceHost and HControlPoint according
// to their needs
//
class HModelCreationArgs
{
public:

    HModelCreationArgs();
    virtual ~HModelCreationArgs() = 0;

    QString m_deviceDescription;
    QList<QUrl> m_deviceLocations;

    ServiceDescriptionFetcher m_serviceDescriptionFetcher;
    // provides the possibility of defining how the service description is
    // retrieved

    qint32 m_deviceTimeoutInSecs;
    IconFetcher m_iconFetcher;
    QByteArray m_loggingIdentifier;
};

}
}

#endif /* HMODELCREATOR_P_H_ */
