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

#ifndef HASYNCOP_H_
#define HASYNCOP_H_

#include <HUpnpCore/HUpnp>

class QString;

namespace Herqq
{

namespace Upnp
{

class HAsyncOpPrivate;

/*!
 * \brief This abstract class is used as a base for identifying an asynchronous
 * operation and detail information of it.
 *
 * Some HUPnP components provide an asynchronous interface for running possible
 * long-standing operations. A most notable example of this is the client-side
 * action invocation initiated with HClientAction::beginInvoke(). In cases
 * like this, the class running the operation returns a derivative of this class,
 * which is used to identify and describe the running operation.
 *
 * \section Usage
 *
 * The component that runs an asynchronous operation provides an instance
 * derived from this class when the operation is started. A copy of that instance is
 * provided also when the component signals the operation is complete.
 * The provided instance uniquely identifies the operation, carries information
 * whether the operation eventually succeeded or not and it may contain an error
 * description in case of an error.
 *
 * For example:
 *
 * \code
 *
 * HClientActionOp op = someActionObject->beginInvoke();
 *
 * //
 * // The operation completes, after which you can:
 * //
 *
 * int retVal = op.returnValue();
 * // retrieve a return value indicating whether the operation succeeded.
 *
 * QString errDescr = op.errorDescription();
 * // retrieve an error description if the operation failed.
 *
 * \endcode
 *
 * \note HAsyncOp and any derivative class provided by HUPnP use \e explicit
 * \e sharing, which basically means that every copy of an instance references
 * the same underlying data and any change to that data is visible to all of the
 * copies.
 *
 * \headerfile hasyncop.h HAsyncOp
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HClientActionOp
 */
class H_UPNP_CORE_EXPORT HAsyncOp
{
H_DECLARE_PRIVATE(HAsyncOp)
friend H_UPNP_CORE_EXPORT bool operator==(const HAsyncOp&, const HAsyncOp&);

protected:

    HAsyncOpPrivate* h_ptr;

    //
    // \internal
    //
    HAsyncOp(HAsyncOpPrivate&);

    //
    // \internal
    //
    HAsyncOp(qint32 returnCode, const QString& errorDescription,
            HAsyncOpPrivate& dd);

    /*!
     * Creates a new valid instance.
     *
     * Creates a new valid instance, i.e isNull() always returns \e false.
     *
     * \sa isNull()
     */
    HAsyncOp();

    /*!
     * Creates a new invalid instance.
     *
     * \param returnCode specifies the return code.
     *
     * \param errorDescription specifies a human-readable description of the error
     * that occurred.
     */
    HAsyncOp(qint32 returnCode, const QString& errorDescription);

    /*!
     * \brief Copy constructor.
     *
     * Creates a shallow copy of \a other increasing the reference count of
     * \a other.
     */
    HAsyncOp(const HAsyncOp&);

    /*!
     * \brief Assignment operator.
     *
     * Switches this instance to refer to the contents of \a other increasing the
     * reference count of \a other.
     */
    HAsyncOp& operator=(const HAsyncOp&);

public:

    /*!
     * \brief Destroys the instance.
     *
     * Decreases the reference count or destroys the instance once the reference
     * count drops to zero.
     */
    virtual ~HAsyncOp() = 0;

    /*!
     * \brief Returns a human readable error description.
     *
     * \return a human readable error description, if any.
     *
     * \sa setErrorDescription()
     */
    QString errorDescription() const;

    /*!
     * \brief Sets a human readable error description.
     *
     * \param arg specifies the human readable error description.
     *
     * \sa errorDescription()
     */
    void setErrorDescription(const QString& arg);

    /*!
     * \brief Returns the return value of the asynchronous operation.
     *
     * \sa setReturnValue()
     */
    int returnValue() const;

    /*!
     * \brief Sets the return value of the asynchronous operation.
     *
     * \param returnValue specifies the return value of the asynchronous operation.
     *
     * \sa returnValue()
     */
    void setReturnValue(int returnValue);

    /*!
     * \brief Returns an identifier of the asynchronous operation.
     *
     * \return an identifier of the asynchronous operation. The identifier
     * is "unique" within the process where the library is loaded. More specifically,
     * the ID is monotonically incremented and it is allowed to overflow.
     */
    unsigned int id() const;

    /*!
     * \brief Indicates whether the object identifies an asynchronous operation.
     *
     * \retval true in case the object does not identify an asynchronous operation.
     * This is usually the case when an operation was not successfully started.
     * \retval false in case the object identifies an asynchronous operation.
     */
    bool isNull() const;

    /*!
     * \brief Indicates whether the object identifies an asynchronous operation.
     *
     * This is a convenience method and it is semantically equivalent with isNull().
     *
     * \retval true in case the object does not identify an asynchronous operation.
     * This is usually the case when an operation was not successfully started.
     * \retval false in case the object identifies an asynchronous operation.
     */
    inline bool operator!() const
    {
        return isNull();
    }

    /*!
     * Aborts the execution of the operation.
     *
     * Aborts the execution of the operation.
     *
     * \remarks
     * It is up to the implementation to decide whether to implement this. The
     * default implementation does nothing.
     */
    virtual void abort();
};

}
}

#endif /* HASYNCOP_H_ */
