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

#ifndef HSTATEVARIABLE_EVENT_H_
#define HSTATEVARIABLE_EVENT_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QVariant>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

class HStateVariableEventPrivate;

/*!
 * \brief This is a class used to transfer state variable event information.
 *
 * \headerfile hstatevariable_event.h HStateVariableEvent
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientStateVariable, HServerStateVariable
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HStateVariableEvent
{
private:

    QSharedDataPointer<HStateVariableEventPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HStateVariableEvent();

    /*!
     * Creates a new instance based on the provided values.
     *
     * \param previousValue specifies the value before the change.
     *
     * \param newValue specifies the value of the state variable at the time
     * the event was generated.
     *
     * \sa isEmpty()
     */
    HStateVariableEvent(
        const QVariant& previousValue,
        const QVariant& newValue);

    /*!
     * \brief Copy constructor.
     *
     * Copies the contents of the \c other to this.
     */
    HStateVariableEvent(const HStateVariableEvent&);

    /*!
     * \brief Destroys the instance.
     */
    ~HStateVariableEvent();

    /*!
     * Assigns the contents of the other object to this.
     *
     * \return reference to this object.
     */
    HStateVariableEvent& operator=(const HStateVariableEvent&);

    /*!
     * \brief Indicates if the instance is empty.
     *
     * \return \e true if the instance is empty, which means that none of its
     * attributes contain a valid non-empty value.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the previous value of the state variable.
     *
     * \return The previous value of the state variable.
     */
    QVariant previousValue() const;

    /*!
     * \brief Returns the new, changed value of the state variable.
     *
     * \return The new, changed value of the state variable.
     */
    QVariant newValue() const;
};

}
}

#endif /* HSTATEVARIABLE_EVENT_H_ */
