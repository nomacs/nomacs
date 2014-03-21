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

#include "hdevicebuild_p.h"
#include "hcontrolpoint_p.h"

#include "../../devicemodel/client/hdefault_clientdevice_p.h"

#include "../../general/hlogger_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * DeviceBuildTask
 ******************************************************************************/
DeviceBuildTask::~DeviceBuildTask()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    if (m_createdDevice.data())
    {
        m_createdDevice->deleteLater();
    }

    m_createdDevice.take();
}

HDefaultClientDevice* DeviceBuildTask::createdDevice()
{
    return m_createdDevice.take();
}

void DeviceBuildTask::run()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    QString err;
    QScopedPointer<HDefaultClientDevice> device;
    device.reset(
        m_owner->buildDevice(m_locations[0], m_cacheControlMaxAge, &err));
    // the returned device is a fully built root device containing every
    // embedded device and service advertised in the device and service descriptions
    // otherwise, the creation failed
    if (!device.data())
    {
        HLOG_WARN(QString("Couldn't create a device: %1").arg(err));

        m_completionValue = -1;
        m_errorString = err;
    }
    else
    {
        device->moveToThread(m_owner->thread());

        m_completionValue = 0;
        m_createdDevice.swap(device);
    }

    emit done(m_udn);
}

/*******************************************************************************
 * DeviceBuildTasks
 ******************************************************************************/
DeviceBuildTasks::DeviceBuildTasks() :
    m_builds()
{
}

DeviceBuildTasks::~DeviceBuildTasks()
{
    qDeleteAll(m_builds);
}

DeviceBuildTask* DeviceBuildTasks::get(const HUdn& udn) const
{
    QList<DeviceBuildTask*>::const_iterator ci = m_builds.constBegin();

    for(; ci != m_builds.constEnd(); ++ci)
    {
        if ((*ci)->udn() == udn)
        {
            return *ci;
        }
    }

    return 0;
}

void DeviceBuildTasks::remove(const HUdn& udn)
{
    QList<DeviceBuildTask*>::iterator i = m_builds.begin();

    for(; i != m_builds.end(); ++i)
    {
        if ((*i)->udn() == udn)
        {
            delete (*i);
            m_builds.erase(i);
            return;
        }
    }

    Q_ASSERT(false);
}

void DeviceBuildTasks::add(DeviceBuildTask* arg)
{
    Q_ASSERT(arg);
    m_builds.push_back(arg);
}

QList<DeviceBuildTask*> DeviceBuildTasks::values() const
{
    return m_builds;
}

}
}
