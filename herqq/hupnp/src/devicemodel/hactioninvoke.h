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

#ifndef HACTIONINVOKE_H_
#define HACTIONINVOKE_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HFunctor>

/*!
 * \file
 * This file contains the type definition and usage documentation for the functor
 * used in action invocation.
 */

namespace Herqq
{

namespace Upnp
{

/*!
 * This is a type definition for a <em>callable entity</em> that is used
 * for HServerAction invocation.
 *
 * You can create \c %HActionInvoke objects using normal functions, functors and
 * member functions that follow the signature of
 *
 * <tt>
 *
 * qint32 function(const Herqq::Upnp::HActionArguments& inArgs,
 *                 Herqq::Upnp::HActionArguments* outArgs = 0);
 *
 * </tt>
 *
 * The following example demonstrates how you can instantiate the \c %HActionInvoke
 * for a normal function, functor and a member function.
 *
 * \code
 *
 * #include <HActionInvoke>
 * #include <HUpnpCore/HActionArguments>
 *
 * #include "myclass.h" // your code that contains declaration for MyClass
 *
 * namespace
 * {
 * qint32 freefun(
 *      const Herqq::Upnp::HActionArguments& inArgs,
 *      Herqq::Upnp::HActionArguments* outArgs)
 * {
 *     return 0;
 * }
 *
 * class MyFunctor
 * {
 * public:
 *     qint32 operator()(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs)
 *     {
 *         return 0;
 *     }
 * };
 * }
 *
 * qint32 MyClass::memfun(
 *      const Herqq::Upnp::HActionArguments& inArgs,
 *      Herqq::Upnp::HActionArguments* outArgs = 0)
 * {
 * }
 *
 * void MyClass::example()
 * {
 *     Herqq::Upnp::HActionInvoke usingFreeFunction(freefun);
 *
 *     MyFunctor myfunc;
 *     Herqq::Upnp::HActionInvoke usingFunctor(myfunc);
 *
 *     Herqq::Upnp::HActionInvoke usingMemberFunction(this, &MyClass::memfun);
 * }
 *
 * \endcode
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (actionInvokeObject) { ... } </tt>
 *
 * \headerfile hactioninvoke.h HActionInvoke
 *
 * \ingroup hupnp_devicemodel
 */
typedef Functor<int, H_TYPELIST_2(
    const Herqq::Upnp::HActionArguments&,
    Herqq::Upnp::HActionArguments*)> HActionInvoke;

}
}

#endif /* HACTIONINVOKE_H_ */
