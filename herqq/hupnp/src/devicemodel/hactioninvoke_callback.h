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

#ifndef HACTIONINVOKE_CALLBACK_H_
#define HACTIONINVOKE_CALLBACK_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HFunctor>

/*!
 * \file
 */

namespace Herqq
{

namespace Upnp
{

/*!
 * This is a type definition for a <em>callable entity</em> that is used
 * as a callback for signaling the completion of an HClientAction invocation.
 *
 * You can create \c %HActionInvokeCallback objects using normal functions,
 * functors and member functions that follow the signature of
 *
 * <tt>
 *
 * bool function(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&);
 *
 * </tt>
 *
 * <h3>Parameters</h3>
 * \li The first parameter is a type of an "ID" of the asynchronous action invocation.
 * \li The second parameter specifies the output arguments of the action invocation,
 * which may be empty.
 *
 * <h3>Return value</h3>
 * The return value indicates if the invoked HClientAction should emit an
 * HClientAction::invokeComplete() after the callback has returned.
 * \li \b true indicates that the HClientAction should sent the corresponding event.
 *
 * The following example demonstrates how you can instantiate the \c %HActionInvokeCallback
 * for a normal function, functor and a member function.
 *
 * \code
 *
 * #include <HUpnpCore/HClientAction>
 *
 * #include "myclass.h" // your code that contains declaration for MyClass
 *
 * namespace
 * {
 * bool freefun(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)
 * {
 *     return true;
 * }
 *
 * class MyFunctor
 * {
 * public:
 *     bool operator()(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)
 *     {
 *         return true;
 *     }
 * };
 * }
 *
 * bool MyClass::memfun(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)
 * {
 *     return true;
 * }
 *
 * void MyClass::example()
 * {
 *     Herqq::Upnp::HActionInvokeCallback usingFreeFunction(freefun);
 *
 *     MyFunctor myfunc;
 *     Herqq::Upnp::HActionInvokeCallback usingFunctor(myfunc);
 *
 *     Herqq::Upnp::HActionInvokeCallback usingMemberFunction(this, &MyClass::memfun);
 * }
 *
 * \endcode
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (actionInvokeCallbackObject) { ... } </tt>
 *
 * \headerfile hactioninvoke_callback.h HActionInvokeCallback
 *
 * \ingroup hupnp_devicemodel
 */
typedef Functor<bool, H_TYPELIST_2(
    HClientAction*, const HClientActionOp&)> HActionInvokeCallback;

}
}

#endif /* HACTIONINVOKE_CALLBACK_H_ */
