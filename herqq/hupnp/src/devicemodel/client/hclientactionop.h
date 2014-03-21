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

#ifndef HCLIENTACTION_OP_H_
#define HCLIENTACTION_OP_H_

#include <HUpnpCore/HAsyncOp>

namespace Herqq
{

namespace Upnp
{

class HClientActionOpPrivate;

/*!
 * \brief This class is used to identify a client-side action invocation and detail
 * information of it.
 *
 * When you call HClientAction::beginInvoke() you get an instance of this class
 * that uniquely identifies the asynchronous operation within the running process.
 * Once the operation completes and the HClientAction::invokeComplete()
 * signal is sent, you get a copy of the instance that was provided by the \c beginInvoke().
 * You can use either of the objects and any other copy you may have made to query
 * the UPnP return code of the operation by calling returnValue(). You can call
 * inputArguments() to get the arguments you provided to the
 * HClientAction::beginInvoke() and you can call outputArguments() to get any
 * output arguments the action invocation may have returned.
 *
 * \headerfile hclientactionop.h HClientActionOp
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientAction, HAsyncOp
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HClientActionOp :
    public HAsyncOp
{
H_DECLARE_PRIVATE(HClientActionOp);

public:

    /*!
     * \brief Creates a new instance.
     */
    HClientActionOp();

    /*!
     * Creates a new invalid instance.
     *
     * \param returnCode specifies the return code.
     *
     * \param errorDescription specifies a human-readable description of the error
     * that occurred.
     */
    HClientActionOp(qint32 returnCode, const QString& errorDescription);

    /*!
     * \brief Creates a new instance based on the provided values.
     *
     * \param inArgs specifies the input arguments of the action invocation.
     */
    HClientActionOp(const HActionArguments& inArgs);

    /*!
     * \brief Copy constructor.
     *
     * Copies the contents of the \c other to this.
     */
    HClientActionOp(const HClientActionOp&);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HClientActionOp();

    // Documented in HAsyncOp
    virtual void abort();

    /*!
     * \brief Assigns the contents of the other object to this.
     *
     * \return reference to this object.
     */
    HClientActionOp& operator=(const HClientActionOp&);

    /*!
     * \brief Returns the input arguments of the action invocation.
     *
     * \return The input arguments of the action invocation.
     */
    const HActionArguments& inputArguments() const;

    /*!
     * \brief Returns the output arguments of the action invocation.
     *
     * \return The output arguments of the action invocation.
     */
    const HActionArguments& outputArguments() const;

    /*!
     * \brief Sets the output arguments of the action invocation.
     *
     * \param outArgs
     */
    void setOutputArguments(const HActionArguments& outArgs);
};

}
}

#endif /* HCLIENTACTION_OP_H_ */
