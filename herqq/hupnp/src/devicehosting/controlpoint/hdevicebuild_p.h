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

#ifndef HDEVICEBUILD_P_H
#define HDEVICEBUILD_P_H

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../general/hupnp_defs.h"
#include "../../dataelements/hudn.h"
#include "../../utils/hthreadpool_p.h"

#include <QtCore/QList>

namespace Herqq
{

namespace Upnp
{

class HServiceSubscribtion;
class HControlPointPrivate;
class HDefaultClientDevice;

//
// This class is used as a thread pool task to fetch a device description, its
// accompanying service descriptions (if any) and to build the device model.
//
class DeviceBuildTask :
    public HRunnable
{
Q_OBJECT
H_DISABLE_COPY(DeviceBuildTask)

private:

    HControlPointPrivate* m_owner;

    QAtomicInt m_completionValue;
    // 0 for success

    QString m_errorString;
    // empty when succeeded

    QScopedPointer<HDefaultClientDevice> m_createdDevice;

    const HUdn m_udn;
    const qint32 m_cacheControlMaxAge;

public:

    QList<QUrl> m_locations;

    template<typename Msg>
    DeviceBuildTask(HControlPointPrivate* owner, const Msg& msg) :
            m_owner(owner),
            m_completionValue(-1),
            m_errorString(),
            m_createdDevice(0),
            m_udn(msg.usn().udn()),
            m_cacheControlMaxAge(msg.cacheControlMaxAge()),
            m_locations()
    {
        m_locations.append(msg.location());
    }

    virtual ~DeviceBuildTask();
    // deletes the created device if it has not been retrieved

    virtual void run();

    inline HUdn udn() const { return m_udn; }

    inline qint32 completionValue() const { return m_completionValue; }

    inline QString errorString() const { return m_errorString; }
    // returns an error description only if something went wrong
    // and the build failed

    HDefaultClientDevice* createdDevice();
    // releases ownership

Q_SIGNALS:

    void done(const Herqq::Upnp::HUdn&);
};

//
// A class used by the controlpoint to store information about ongoing
// device model builds (initiated by advertisement and discovery messages)
//
class DeviceBuildTasks
{
H_DISABLE_COPY(DeviceBuildTasks)

private:

    QList<DeviceBuildTask*> m_builds;

public:

    DeviceBuildTasks();
    ~DeviceBuildTasks();

    template<typename Msg>
    DeviceBuildTask* get(const Msg& msg) const
    {
        QList<DeviceBuildTask*>::const_iterator ci = m_builds.constBegin();

        for(; ci != m_builds.constEnd(); ++ci)
        {
            if ((*ci)->udn() == msg.usn().udn())
            {
                // UDN match, we are definitely already building the device
                // the message advertises
                return *ci;
            }

            QList<QUrl>::const_iterator ci2 = (*ci)->m_locations.constBegin();

            for(; ci2 != (*ci)->m_locations.constEnd(); ++ci2)
            {
                if (*ci2 == msg.location())
                {
                    // exact "location" (the URL to device description) match.
                    // This means that we are already
                    // building the device tree, but the build started from
                    // a different advertisement message advertising another
                    // device in the tree (root & embedded devices do not share
                    // a common UDN).
                    return *ci;
                }
            }
        }

        return 0;
    }

    // ownership is not transferred
    DeviceBuildTask* get(const HUdn& udn) const;

    // also deletes the removed object
    void remove(const HUdn& udn);

    // takes ownership
    void add(DeviceBuildTask* arg);

    // ownership is not transferred
    QList<DeviceBuildTask*> values() const;
};

}
}

#endif /* HDEVICEBUILD_P_H */
