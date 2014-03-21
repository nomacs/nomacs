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

#include "hasyncop.h"
#include "hasyncop_p.h"

#include <QtCore/QMutex>

static unsigned int s_lastInt = 0;
static QMutex s_lastIntMutex;

namespace Herqq
{

namespace Upnp
{

namespace
{
inline unsigned int getNextId()
{
    unsigned int retVal;
    s_lastIntMutex.lock();
    retVal = ++s_lastInt;
    s_lastIntMutex.unlock();
    return retVal;
}
}

unsigned int HAsyncOpPrivate::genId()
{
    return getNextId();
}

HAsyncOpPrivate::~HAsyncOpPrivate()
{
    delete m_errorDescription;
}

HAsyncOp::HAsyncOp() :
    h_ptr(new HAsyncOpPrivate(getNextId()))
{
}

HAsyncOp::HAsyncOp(HAsyncOpPrivate& dd) :
    h_ptr(&dd)
{
}

HAsyncOp::HAsyncOp(
    qint32 returnCode, const QString& errorDescription, HAsyncOpPrivate& dd) :
        h_ptr(&dd)
{
    h_ptr->m_returnValue = returnCode;
    h_ptr->m_errorDescription = new QString(errorDescription);
}

HAsyncOp::HAsyncOp(int rc, const QString& errorDescription) :
    h_ptr(new HAsyncOpPrivate())
{
    h_ptr->m_returnValue = rc;
    h_ptr->m_errorDescription = new QString(errorDescription);
}

HAsyncOp::~HAsyncOp()
{
    if (--h_ptr->m_refCount == 0)
    {
        delete h_ptr;
    }
}

HAsyncOp::HAsyncOp(const HAsyncOp& op) :
    h_ptr(op.h_ptr)
{
    Q_ASSERT(this != &op);
    ++h_ptr->m_refCount;
}

HAsyncOp& HAsyncOp::operator=(const HAsyncOp& op)
{
    Q_ASSERT(this != &op);

    if (--h_ptr->m_refCount == 0)
    {
        delete h_ptr;
    }
    h_ptr = op.h_ptr;
    ++h_ptr->m_refCount;

    return *this;
}

QString HAsyncOp::errorDescription() const
{
    return h_ptr->m_errorDescription ?
        QString(*h_ptr->m_errorDescription) : QString();
}

void HAsyncOp::setErrorDescription(const QString& arg)
{
    if (h_ptr->m_errorDescription)
    {
        delete h_ptr->m_errorDescription;
        h_ptr->m_errorDescription = 0;
    }

    h_ptr->m_errorDescription = new QString(arg);
}

int HAsyncOp::returnValue() const
{
    return h_ptr->m_returnValue;
}

void HAsyncOp::setReturnValue(int returnValue)
{
   h_ptr->m_returnValue = returnValue;
}

unsigned int HAsyncOp::id() const
{
    return h_ptr->id();
}

bool HAsyncOp::isNull() const
{
    return h_ptr->id() == 0;
}

void HAsyncOp::abort()
{
    // The default implementation does nothing.
}

}
}
