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

#ifndef HDEVICEINFO_P_H_
#define HDEVICEINFO_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hudn.h"
#include "hresourcetype.h"

#include "../general/hlogger_p.h"

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QSharedData>

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HDeviceInfo
//
class HDeviceInfoPrivate :
    public QSharedData
{
HDeviceInfoPrivate& operator=(const HDeviceInfoPrivate&);

public: // attributes

    HResourceType m_deviceType;
    QString m_friendlyName;
    QString m_manufacturer;
    QUrl    m_manufacturerUrl;
    QString m_modelDescription;
    QString m_modelName;
    QString m_modelNumber;
    QUrl    m_modelUrl;
    QString m_serialNumber;
    HUdn    m_udn;
    QString m_upc;
    QUrl    m_presentationUrl;
    QList<QUrl> m_icons;

public: // methods

    HDeviceInfoPrivate();
    ~HDeviceInfoPrivate();

    bool setDeviceType(const HResourceType& deviceType);
    bool setFriendlyName(const QString& friendlyName);
    bool setManufacturer(const QString& manufacturer);
    inline bool setManufacturerUrl(const QUrl& manufacturerUrl)
    {
        m_manufacturerUrl = manufacturerUrl;
        return true;
    }

    bool setModelDescription(const QString& modelDescription);

    bool setModelName(const QString& modelName);
    bool setModelNumber(const QString& modelNumber);
    inline bool setModelUrl(const QUrl& modelUrl)
    {
        m_modelUrl = modelUrl;
        return true;
    }

    bool setSerialNumber(const QString& serialNumber);

    inline bool setUdn(const HUdn& udn, HValidityCheckLevel checkLevel)
    {
        if (!udn.isValid(checkLevel))
        {
            return false;
        }

        m_udn = udn;
        return true;
    }

    bool setUpc(const QString& upc);
    bool setIcons(const QList<QUrl>& icons);
    inline bool setPresentationUrl(const QUrl& presentationUrl)
    {
        Q_ASSERT(presentationUrl.isValid() || presentationUrl.isEmpty());
        m_presentationUrl = presentationUrl;
        return true;
    }
};

}
}

#endif /* HDEVICEINFO_P_H_ */
