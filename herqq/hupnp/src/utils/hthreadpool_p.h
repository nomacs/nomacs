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

#ifndef HTHREADPOOL_P_H_
#define HTHREADPOOL_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../general/hupnp_defs.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QAtomicInt>
#include <QtCore/QThreadPool>
#include <QtCore/QWaitCondition>

namespace Herqq
{

namespace Upnp
{

class HThreadPool;

//
//
//
class HRunnable :
    public QRunnable,
    public QObject
{
H_DISABLE_COPY(HRunnable)
friend class HThreadPool;

public:

    enum Status
    {
        NotStarted,
        WaitingNewTask,
        RunningTask,
        Exiting
    };

private:

    Status m_status;
    QMutex m_statusMutex;
    QWaitCondition m_statusWait;

    HThreadPool* m_owner;
    bool m_doNotInform;

public:

    HRunnable();
    virtual ~HRunnable() = 0;

    void signalTaskComplete();
    void signalExit();

    Status wait();

    bool setupNewTask();
};

//
//
//
class HThreadPool :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HThreadPool)
friend class HRunnable;

private:

    QThreadPool* m_threadPool;
    QList<HRunnable*> m_runnables;
    QMutex m_runnablesMutex;

    void exiting(HRunnable*);

public:

    HThreadPool(QObject* parent);
    virtual ~HThreadPool();

    void start(HRunnable*);
    void shutdown();

    inline void setMaxThreadCount(qint32 count)
    {
        m_threadPool->setMaxThreadCount(count);
    }

    inline int activeThreadCount() const
    {
        return m_threadPool->activeThreadCount();
    }
};

}
}

#endif /* HTHREADPOOL_P_H_ */
