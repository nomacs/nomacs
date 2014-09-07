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

#include "hthreadpool_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HRunnable
 ******************************************************************************/
HRunnable::HRunnable() :
    m_status(NotStarted), m_statusMutex(), m_statusWait(),
    m_owner(0), m_doNotInform(false)
{
}

HRunnable::~HRunnable()
{
    if (!m_doNotInform)
    {
        m_owner->exiting(this);
    }
}

void HRunnable::signalTaskComplete()
{
    QMutexLocker locker(&m_statusMutex);
    Q_ASSERT(m_status == RunningTask);
    m_status = WaitingNewTask;
    m_statusWait.wakeOne();
}

void HRunnable::signalExit()
{
    QMutexLocker locker(&m_statusMutex);
    if (m_status == Exiting)
    {
        return;
    }
    m_status = Exiting;
    m_statusWait.wakeAll();
}

HRunnable::Status HRunnable::wait()
{
    QMutexLocker locker(&m_statusMutex);
    Q_ASSERT(m_status != NotStarted);
    for(;;)
    {
        if (m_status == Exiting || m_status == WaitingNewTask)
        {
            return m_status;
        }

        m_statusWait.wait(&m_statusMutex, 100);
    }

    return Exiting;
}

bool HRunnable::setupNewTask()
{
    QMutexLocker locker(&m_statusMutex);
    if (m_status == Exiting)
    {
        return false;
    }

    m_status = RunningTask;
    return true;
}

/*******************************************************************************
 * HThreadPool
 ******************************************************************************/
HThreadPool::HThreadPool(QObject* parent) :
    QObject(parent),
        m_threadPool(new QThreadPool(this))
{
}

HThreadPool::~HThreadPool()
{
    shutdown();
}

void HThreadPool::exiting(HRunnable* runnable)
{
    QMutexLocker locker(&m_runnablesMutex);
    QList<HRunnable*>::iterator it = m_runnables.begin();
    for (; it != m_runnables.end(); ++it)
    {
        if (*it == runnable)
        {
            m_runnables.erase(it);
            return;
        }
    }
}

void HThreadPool::start(HRunnable* runnable)
{
    Q_ASSERT(runnable);
    Q_ASSERT(runnable->m_status == HRunnable::NotStarted);
    Q_ASSERT(!runnable->m_owner);

    runnable->m_status = HRunnable::WaitingNewTask;
    runnable->m_owner = this;

    QMutexLocker locker(&m_runnablesMutex);
    m_runnables.append(runnable);
    locker.unlock();

    m_threadPool->start(runnable);
}

void HThreadPool::shutdown()
{
    QMutexLocker locker(&m_runnablesMutex);
    for (int i = 0; i < m_runnables.size(); ++i)
    {
        m_runnables.at(i)->m_doNotInform = true;
        m_runnables.at(i)->signalExit();
    }
    m_runnables.clear();
    locker.unlock();

    m_threadPool->waitForDone();
}

}
}
