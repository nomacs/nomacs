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

#ifndef HCLIENTADAPTER_P_H_
#define HCLIENTADAPTER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HResourceType>
#include <HUpnpCore/HClientAction>
#include <HUpnpCore/HClientActionOp>
#include <HUpnpCore/HClientAdapterOp>

#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtCore/QScopedPointer>

namespace Herqq
{

namespace Upnp
{

typedef QList<QPair<HAbstractClientAdapterOp*, HAsyncOp*> > HClientAdapterOps;

//
//
//
class H_UPNP_CORE_EXPORT HAbstractClientAdapterPrivate
{
H_DISABLE_COPY(HAbstractClientAdapterPrivate)

protected:

    HClientAdapterOps m_ops;

public:

    QByteArray m_loggingIdentifier;
    QScopedPointer<HDeviceModelInfoProvider> m_infoProvider;
    HResourceType m_resourceType;
    QString m_lastErrorDescription;

public:

    HAbstractClientAdapterPrivate(const HResourceType& rt) :
        m_ops(), m_loggingIdentifier(), m_infoProvider(), m_resourceType(rt),
        m_lastErrorDescription()
    {
    }

    virtual ~HAbstractClientAdapterPrivate()
    {
        for(int i = 0; i < m_ops.size(); ++i)
        {
            delete m_ops.at(i).first;
            m_ops.at(i).second->abort();
            delete m_ops.at(i).second;
        }
    }

    template<typename RetValOp, typename RunningOp>
    void addOp(const RetValOp& op1, const RunningOp& op2)
    {
        m_ops.append(QPair<HAbstractClientAdapterOp*, HAsyncOp*>(new RetValOp(op1), new RunningOp(op2)));
    }

    template<typename ValueType>
    HClientAdapterOp<ValueType> addOp(const HClientActionOp& op)
    {
        HClientAdapterOp<ValueType>* retVal = new HClientAdapterOp<ValueType>();
        retVal->setReturnValue(op.returnValue());
        retVal->setErrorDescription(op.errorDescription());
        m_ops.append(QPair<HAbstractClientAdapterOp*, HAsyncOp*>(
            static_cast<HAbstractClientAdapterOp*>(retVal), new HClientActionOp(op)));
        return *retVal;
    }

    inline void remove(int i)
    {
        delete m_ops.at(i).first;
        delete m_ops.at(i).second;
        m_ops.removeAt(i);
    }

    template<typename RetValOp>
    RetValOp takeOp(const HAsyncOp& op)
    {
        for(int i = 0; i < m_ops.size(); ++i)
        {
            if (op.id() == m_ops.at(i).second->id())
            {
                RetValOp retVal = *static_cast<RetValOp*>(m_ops.at(i).first);
                retVal.setReturnValue(op.returnValue());
                retVal.setErrorDescription(op.errorDescription());
                remove(i);
                return retVal;
            }
        }
        return RetValOp();
    }

    template<typename ValueType>
    HClientAdapterOp<ValueType> takeOp(
        const HClientActionOp& op, const ValueType& value)
    {
        HClientAdapterOp<ValueType> retVal;
        for(int i = 0; i < m_ops.size(); ++i)
        {
            if (m_ops.at(i).second->id() == op.id())
            {
                retVal = *static_cast<HClientAdapterOp<ValueType>*>(m_ops.at(i).first);
                retVal.setValue(value);
                retVal.setReturnValue(op.returnValue());
                retVal.setErrorDescription(op.errorDescription());
                remove(i);
                break;
            }
        }
        return retVal;
    }

    HClientAdapterOpNull takeOp(const HClientActionOp& op)
    {
        HClientAdapterOpNull retVal;
        for(int i = 0; i < m_ops.size(); ++i)
        {
            if (m_ops.at(i).second->id() == op.id())
            {
                retVal = *static_cast<HClientAdapterOpNull*>(m_ops.at(i).first);
                retVal.setReturnValue(op.returnValue());
                retVal.setErrorDescription(op.errorDescription());
                remove(i);
                break;
            }
        }
        return retVal;
    }

    template<typename ValueType>
    HClientAdapterOp<ValueType> beginInvoke(
        HClientAction* actionToInvoke, const HActionArguments& inArgs,
        const HActionInvokeCallback& cb)
    {
        HClientActionOp op = actionToInvoke->beginInvoke(inArgs, cb, 0);
        return addOp<ValueType>(op);
    }

    HClientAdapterOpNull beginInvoke(
        HClientAction* actionToInvoke, const HActionArguments& inArgs,
        const HActionInvokeCallback& cb)
    {
        HClientActionOp op = actionToInvoke->beginInvoke(inArgs, cb, 0);
        return addOp<HNullValue>(op);
    }
};

}
}

#endif /* HCLIENTADAPTER_P_H_ */
