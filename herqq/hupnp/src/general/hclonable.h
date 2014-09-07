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

#ifndef HCLONABLE_H_
#define HCLONABLE_H_

#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

/*!
 * \brief This class defines an interface for cloning instances of polymorphic classes.
 *
 * \headerfile hclonable.h HClonable
 *
 * \remarks This class is thread-safe.
 *
 * \ingroup hupnp_common
 */
class H_UPNP_CORE_EXPORT HClonable
{
H_DISABLE_COPY(HClonable)

protected:

    /*!
     * Clones the contents of this to the \c target object.
     *
     * Every derived class that introduces member variables that should be
     * copied as part of a cloning operation \b should override this method.
     * The implementation should be something along these lines:
     *
     * \code
     * void MyClonable::doClone(HClonable* target) const
     * {
     *    MyClonable* myClonable = dynamic_cast<MyClonable*>(target);
     *    if (!myClonable)
     *    {
     *        return;
     *    }
     *
     *    BaseClassOfMyClonable::doClone(target);
     *
     *    // copy the variables introduced in *this* MyClonable
     *    // instance to "myClonable".
     * }
     * \endcode
     *
     * \param target specifies the target object to which the contents of
     * \c this instance are cloned.
     */
    virtual void doClone(HClonable* target) const;

    /*!
     * \brief Creates a new instance.
     *
     * This method is used as part of object cloning. Because of that, it is
     * important that every concrete (non-abstract) descendant class overrides
     * this method regardless of the type location in the inheritance tree:
     *
     * \code
     * MyClonable* MyClonable::newInstance() const
     * {
     *     return new MyClonable();
     * }
     * \endcode
     *
     * \remarks
     * \li the object has to be heap-allocated and
     * \li the ownership of the object is passed to the caller.
     */
    virtual HClonable* newInstance() const = 0;

public:

    /*!
     * \brief Creates a new instance.
     */
    HClonable();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HClonable();

    /*!
     * \brief Returns a deep copy of the instance.
     *
     * \return a deep copy of the instance.
     *
     * \remarks
     * \li the ownership of the returned object is transferred to the caller.
     */
    virtual HClonable* clone() const;
};

}
}

#endif /* HCLONABLE_H_ */
