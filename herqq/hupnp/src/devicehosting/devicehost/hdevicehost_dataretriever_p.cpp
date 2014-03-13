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

#include "hdevicehost_dataretriever_p.h"

#include "../../general/hlogger_p.h"

#include <QtCore/QFile>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

DeviceHostDataRetriever::DeviceHostDataRetriever(
    const QByteArray& loggingId, const QUrl& rootDir) :
        m_loggingIdentifier(loggingId), m_rootDir(rootDir)
{
}

bool DeviceHostDataRetriever::retrieveServiceDescription(
    const QUrl& /*deviceLocation*/, const QUrl& scpdUrl, QString* retVal)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString localScpdPath = scpdUrl.toLocalFile();
    if (localScpdPath.startsWith('/'))
    {
        localScpdPath = localScpdPath.mid(1);
    }

    QString fullScpdPath = m_rootDir.toString();
    if (!fullScpdPath.endsWith('/'))
    {
        fullScpdPath.append('/');
    }
    fullScpdPath.append(localScpdPath);
    // UDA mandates that the paths inside a device description are treated relative
    // to the device description location.

    QFile file(fullScpdPath);

    HLOG_DBG(QString(
        "Attempting to open service description from [%1]").arg(fullScpdPath));

    if (!file.open(QIODevice::ReadOnly))
    {
        m_lastError =
            QString("Could not open the service description file [%1].").arg(
                fullScpdPath);

        return false;
    }

    *retVal = QString::fromUtf8(file.readAll());
    return true;
}

bool DeviceHostDataRetriever::retrieveIcon(
    const QUrl& /*devLoc*/, const QUrl& iconUrl, QByteArray* retVal)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString localIconPath = iconUrl.toLocalFile();
    if (localIconPath.startsWith('/'))
    {
        localIconPath = localIconPath.mid(1);
    }

    QString fullIconPath = m_rootDir.toString();
    if (!fullIconPath.endsWith('/'))
    {
        fullIconPath.append('/');
    }
    fullIconPath.append(localIconPath);
    // UDA mandates that the paths inside a device description are treated relative
    // to the device description location.

    HLOG_DBG(QString(
        "Attempting to open a file [%1] that should contain an icon").arg(
            fullIconPath));

    QFile iconFile(fullIconPath);
    if (!iconFile.open(QIODevice::ReadOnly))
    {
        m_lastError =
            QString("Could not open the icon file [%1]").arg(fullIconPath);

        return false;
    }

    *retVal = iconFile.readAll();
    return true;
}

bool DeviceHostDataRetriever::retrieveDeviceDescription(
    const QString& filePath, QString* retVal)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        m_lastError =
            QString("Could not open the device description file: [%1].").arg(
                filePath);

        return false;
    }

    *retVal = QString::fromUtf8(file.readAll());
    return true;

}

}
}
