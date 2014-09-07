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

#ifndef HEXECARGS_H_
#define HEXECARGS_H_

#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

/*!
 * \brief This class is used to specify information used to control the execution of
 * an asynchronous operation and the notification of its completion.
 *
 * \headerfile hexecargs.h HExecArgs
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is thread-safe.
 */
class H_UPNP_CORE_EXPORT HExecArgs
{
friend H_UPNP_CORE_EXPORT bool operator==(const HExecArgs&, const HExecArgs&);

public:

    /*!
     * \brief This enumeration specifies how the asynchronous operation should be run.
     */
    enum ExecType
    {
        /*!
         * This value indicates that the operation should be run normally and
         * its completion or failure should be signaled normally.
         */
        Normal,

        /*!
         * This value indicates that the operation should be dispatched to be run,
         * but its completion or failure isn't signaled.
         *
         * This value is useful in situations where the result of the operation
         * isn't interesting.
         */
        FireAndForget
    };

private:

    volatile ExecType m_execType;

public:

   /*!
    * \brief Creates a new instance.
    *
    * \param type
    */
   explicit HExecArgs(ExecType type=Normal);

   /*!
    * \brief Destroys the instance.
    *
    * \brief Destroys the instance.
    */
   ~HExecArgs();

   /*!
    * \brief Indicates how the operation should be run and its completion or failure
    * be signaled.
    *
    * \return a value indicating how the operation should be run and its
    * completion or failure be signaled.
    *
    * \sa setExecType()
    */
   inline ExecType execType() const { return m_execType; }

   /*!
    * \brief Sets the value indicating how the operation should be run and its
    * completion or failure be signaled.
    *
    * \param type specifies the value indicating how the operation should be
    * run and its completion or failure be signaled.
    *
    * \sa execType()
    */
   inline void setExecType(ExecType type) { m_execType = type; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HExecArgs
 */
H_UPNP_CORE_EXPORT bool operator==(const HExecArgs&, const HExecArgs&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HExecArgs
 */
inline bool operator!=(const HExecArgs& obj1, const HExecArgs& obj2)
{
    return !(obj1 == obj2);
}

}
}

#endif /* HEXECARGS_H_ */
