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

#include "hmediaserver_device_p.h"
#include "hmediaserver_deviceconfiguration.h"

#include "../cds_model/datasource/hcds_datasource.h"
#include "../contentdirectory/hcontentdirectory_serviceconfiguration.h"
#include "../connectionmanager/hconnectionmanager_serviceconfiguration.h"

#include <QtCore/QEvent>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMediaServerDevice
 *******************************************************************************/
HMediaServerDevice::HMediaServerDevice(
    const HMediaServerDeviceConfiguration& configuration) :
        m_configuration(0)
{
    Q_ASSERT_X(configuration.isValid(), "", "Valid MediaServer configuration has to be provided");
    m_configuration = configuration.clone();
}

HMediaServerDevice::~HMediaServerDevice()
{
    delete m_configuration;
}

bool HMediaServerDevice::event(QEvent* e)
{
    if (e->type() == QEvent::ThreadChange)
    {
        Q_ASSERT_X(m_configuration->contentDirectoryConfiguration()->hasOwnershipOfDataSource(),
                   "",
                   "Cannot move this instance to different thread, since this instance does "
                   "not have the ownership of the configured Data Source.");
        return true;
    }
    return false;
}

bool HMediaServerDevice::finalizeInit(QString* errDescr)
{
    HAbstractCdsDataSource* dataSource =
        m_configuration->contentDirectoryConfiguration()->dataSource();

    Q_ASSERT_X(dataSource->thread() == thread(), "",
               "Data source has to live in the same thread with the media server");

    // Note, the order of service initialization should be this, since the initialization
    // of ContentDirectory might enable TCO and the initialization of ConnectionManager
    // changes the content directory objects, which would result in modifications
    // of the LastChange state variable.

    HConnectionManagerSourceService* cm =
        qobject_cast<HConnectionManagerSourceService*>(connectionManager());

    cm->setSourceProtocolInfo(
        m_configuration->connectionManagerConfiguration()->supportedContentInfo());

    if (!cm || !cm->init())
    {
        if (errDescr)
        {
            *errDescr = "Failed to initialize ConnectionManager";
        }

        return false;
    }

    if (!dataSource->isInitialized() && !dataSource->init())
    {
        if (errDescr)
        {
            *errDescr = "Failed to initialize the data source";
        }

        return false;
    }

    HContentDirectoryService* cds =
        qobject_cast<HContentDirectoryService*>(contentDirectory());

    if (!cds || !cds->init())
    {
        if (errDescr)
        {
            *errDescr = "Failed to initialize ContentDirectory";
        }

        return false;
    }

    if (m_configuration->contentDirectoryConfiguration()->hasOwnershipOfDataSource())
    {
        m_configuration->contentDirectoryConfiguration()->dataSource()->setParent(this);
    }

    return true;
}

HContentDirectoryService* HMediaServerDevice::contentDirectory() const
{
    return static_cast<HContentDirectoryService*>(
        HAbstractMediaServerDevice::contentDirectory());
}

HConnectionManagerSourceService* HMediaServerDevice::connectionManager() const
{
    return static_cast<HConnectionManagerSourceService*>(
        HAbstractMediaServerDevice::connectionManager());
}

}
}
}
