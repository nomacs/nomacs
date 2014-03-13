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

#ifndef HCLIENTACTION_P_H_
#define HCLIENTACTION_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hclientactionop.h"

#include "../hexecargs.h"
#include "../hactionarguments.h"
#include "../hactioninvoke_callback.h"
#include "../../dataelements/hactioninfo.h"

#include <QtCore/QUrl>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtCore/QPointer>
#include <QtCore/QScopedPointer>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

namespace Herqq
{

namespace Upnp
{

class HInvocationInfo;
class HDefaultClientAction;

//
// Class for relaying action invocations across the network to the real
// HClientAction objects instantiated by control points
//
class HActionProxy :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HActionProxy)

private:

    QList<QUrl> m_locations;
    QUrl m_lastUsedLocation;

    qint32 m_iNextLocationToTry;
    // the device locations and the index the next connection attempt should try
    // these are the places to which the action invocation requests are sent

    QNetworkAccessManager& m_nam;
    QPointer<QNetworkReply> m_reply;

    HDefaultClientAction* m_owner;

    HActionArguments m_inArgs;

private:

    void invocationDone(qint32 rc, const HActionArguments* outArgs = 0);
    void deleteReply();

private slots:

    void locationsChanged();
    void error(QNetworkReply::NetworkError);
    void finished();

public:

    HActionProxy(QNetworkAccessManager&, HDefaultClientAction* owner);
    virtual ~HActionProxy();

    bool send();
    void abort();

    inline void setInputArgs(const HActionArguments& inArgs)
    {
        m_inArgs = inArgs;
    }

    inline bool invocationInProgress() const { return m_reply; }
};

//
// Implementation details of HClientAction
//
class HClientActionPrivate :
    public QObject
{
Q_OBJECT
H_DECLARE_PUBLIC(HClientAction)
H_DISABLE_COPY(HClientActionPrivate)

public:

    void invokeCompleted(int rc, const HActionArguments* outArgs = 0);

public:

    const QByteArray m_loggingIdentifier;

    HClientAction* q_ptr;
    QScopedPointer<HActionInfo> m_info;

    HActionProxy* m_proxy;
    QQueue<HInvocationInfo> m_invocations;

public:

    HClientActionPrivate();
    ~HClientActionPrivate();

    bool setInfo(const HActionInfo&);
    void abort(unsigned int id);
};

//
//
//
class HClientActionOp_ :
    public HClientActionOp
{
H_DECLARE_PRIVATE(HClientActionOp);
public:
    HClientActionOp_();
    HClientActionOp_(const HActionArguments& inArgs);
    void setRunner(HClientActionPrivate* runner);
};

//
//
//
class HInvocationInfo
{

public:

    HActionInvokeCallback callback;
    HExecArgs execArgs;

    HActionArguments m_inArgs;
    HClientActionOp_ m_invokeId;

    inline HInvocationInfo() : callback(), execArgs(), m_inArgs(), m_invokeId() { }
    inline ~HInvocationInfo() { }

    inline HInvocationInfo(
        const HActionArguments& inArgs,
        const HActionInvokeCallback& cb,
        const HExecArgs& eargs) :
            callback(cb),
            execArgs(eargs),
            m_inArgs(inArgs),
            m_invokeId(inArgs)
    {
    }
};

}
}

#endif /* HCLIENTACTION_P_H_ */
