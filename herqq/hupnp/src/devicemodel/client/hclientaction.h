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

#ifndef HCLIENTACTION_H_
#define HCLIENTACTION_H_

#include <HUpnpCore/HActionInvokeCallback>

#include <QtCore/QObject>

class QString;

namespace Herqq
{

namespace Upnp
{

class HClientActionPrivate;

/*!
 * \brief A client-side class that represents a server-side UPnP action.
 *
 * \c %HClientAction is a core component of the HUPnP's client-side \ref hupnp_devicemodel
 * and it models a UPnP action. The UPnP Device Architecture specifies a UPnP
 * action as a command, which takes one or more input and output arguments and that
 * may have a return value.
 *
 * \brief This class is used to invoke the server-side UPnP actions from the client-side.
 * You can get information of the action using info(), which includes the action's
 * input and output arguments. You can dispatch an asynchronous action invocation
 * to the server-side using beginInvoke(). Once the server responds, invokeComplete()
 * signal is sent.
 *
 * \headerfile hclientaction.h HClientAction
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HActionInfo, HClientService
 *
 * \remarks
 * \li This class has thread-affinity.
 */
class H_UPNP_CORE_EXPORT HClientAction :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HClientAction)
H_DECLARE_PRIVATE(HClientAction)

protected:

    HClientActionPrivate* h_ptr;

    /*!
     * \brief Creates a new instance.
     *
     * \param info specifies information of the action.
     *
     * \param parent specifies the UPnP service that contains this action.
     */
    HClientAction(const HActionInfo& info, HClientService* parent);

public:

    /*!
     * \brief Destroys the instance.
     *
     * An \c %HClientAction is always destroyed by the \c %HClientAction that contains
     * it when it is being deleted. Further, unless you hold the ownership of the
     * \c %HClientAction instance, you should never destroy it.
     */
    virtual ~HClientAction() = 0;

    /*!
     * \brief Returns the parent service of the action.
     *
     * \return The parent service of the action.
     *
     * \warning the pointer is guaranteed to point to a valid object as long
     * as the \c %HClientAction exists, which ultimately is as long as the
     * containing root device exists.
     *
     * \sa HClientDevice
     */
    HClientService* parentService() const;

    /*!
     * \brief Returns information about the action that is read from the
     * service description.
     *
     * \return information about the action that is read from the
     * service description.
     */
    const HActionInfo& info() const;

    /*!
     * Schedules the action to be invoked.
     *
     * The method performs an asynchronous action invocation. The invocation
     * is placed in a queue and it will be run once it's at the head of the queue.
     *
     * Unless you specified the action to be executed as <em>fire and forget</em>,
     * the signal invokeComplete() is emitted once the invocation is complete.
     *
     * \param inArgs specifies the input arguments for the action invocation.
     *
     * \param execArgs specifies information used to control the execution of
     * the action invocation procedure. This is optional.
     *
     * \return an object that identifies the asynchronous operation. This object will
     * be sent through the invokeComplete() signal once the invocation is done.
     *
     * \sa invokeComplete()
     */
    HClientActionOp beginInvoke(
        const HActionArguments& inArgs, HExecArgs* execArgs = 0);

    /*!
     * Schedules the action to be invoked.
     *
     * The method performs an asynchronous action invocation. The invocation
     * is placed in a queue and it will be run once it's at the head of the queue.
     *
     * Unless you specified the action to be executed as <em>fire and forget</em>,
     * the specified callback is called when the invocation is complete.
     * No events are sent unless that is explicitly requested by \b returning \b
     * true from the callback function.
     *
     * The different semantics compared to the other beginInvoke()
     * method are important to notice:
     *
     * \li If a <em> completion callback </em>is valid, no event is sent unless
     * the invoker explicitly requests that.
     * \li The callback is always invoked immediately after the invocation has
     * succeeded or failed. If the callback returns \e true, the invokeComplete()
     * signal will be emitted immediately after.
     *
     * \param inArgs specifies the input arguments for the action invocation
     *
     * \param completionCallback specifies the callable entity that is called
     * once the action invocation is completed or failed. If the specified callable
     * entity is not valid and it cannot be called, the callable entity is
     * ignored and the invokeComplete() signal is sent sent instead.
     *
     * \param execArgs specifies information used to control the execution of
     * the action invocation procedure. This is optional.
     *
     * \return an object that identifies the asynchronous operation. This object will
     * be sent through the callback and the invokeComplete() once the invocation is done.
     *
     * \sa invokeComplete()
     */
    HClientActionOp beginInvoke(
        const HActionArguments& inArgs,
        const HActionInvokeCallback& completionCallback,
        HExecArgs* execArgs = 0);

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when an invocation has been successfully
     * completed or the invocation failed, unless the invocation was started
     * as <em>fire and forget</em>.
     *
     * \param source identifies the HClientAction that ran the operation.
     *
     * \param operation specifies information of the operation that completed.
     *
     * \sa beginInvoke()
     *
     * \remarks This signal has thread affinity to the thread where the object
     * resides. Do not connect to this signal from other threads.
     */
    void invokeComplete(
        Herqq::Upnp::HClientAction* source,
        const Herqq::Upnp::HClientActionOp& operation);
};

}
}

#endif /* HCLIENTACTION_H_ */
