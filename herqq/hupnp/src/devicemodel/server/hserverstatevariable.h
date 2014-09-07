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

#ifndef HSERVER_STATEVARIABLE_H_
#define HSERVER_STATEVARIABLE_H_

#include <HUpnpCore/HUpnpDataTypes>

#include <QtCore/QObject>
#include <QtCore/QVariant>

class QStringList;

namespace Herqq
{

namespace Upnp
{

class HStateVariablePrivate;

/*!
 * \brief This is a class that represents a server-side UPnP state variable.
 *
 * \c %HServerStateVariable is a core component of the HUPnP's server-side \ref hupnp_devicemodel
 * and it models a UPnP state variable. The UPnP Device Architecture specifies a
 * UPnP state variable as an item or aspect that models state in a service.
 * In a way a state variable is an abstraction to a member variable inside a
 * UPnP service.
 *
 * A state variable can be \e evented in which case it notifies interested listeners
 * of changes in its value. You can see if a state variable is evented by checking
 * the HStateVariableInfo object using info() and you can connect to the signal
 * valueChanged() to be notified when the value of the state variable changes.
 * Note, only evented state variables emit the valueChanged() signal.
 *
 * \headerfile hserverstatevariable.h HServerStateVariable
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HServerStateVariable
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HServerStateVariable :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HServerStateVariable)

protected:

    HStateVariablePrivate* h_ptr;

    /*!
     * \brief Creates a new instance.
     *
     * \param info specifies information of the state variable. This is usually
     * read from a service description document.
     *
     * \param parent specifies the UPnP service instance that contains this
     * state variable.
     */
    HServerStateVariable(const HStateVariableInfo& info, HServerService* parent);

public:

    /*!
     * \brief Destroys the instance.
     *
     * An \c %HServerStateVariable is always destroyed by the
     * \c %HServerService that contains it when it is being deleted.
     * Further, unless you hold the ownership of the \c %HServerStateVariable
     * instance, you should never destroy it.
     */
    virtual ~HServerStateVariable() = 0;

    /*!
     * \brief Returns the HServerService that contains this state variable.
     *
     * \return The HServerService that contains this state variable.
     *
     * \warning the pointer is guaranteed to point to a valid object as long
     * as the \c %HServerStateVariable exists, which ultimately is as long as the
     * containing root device exists.
     */
    HServerService* parentService() const;

    /*!
     * \brief Returns the current value of the state variable.
     *
     * \return The current value of the state variable.
     */
    QVariant value() const;

    /*!
     * \brief Returns information about the state variable.
     *
     * \return information about the state variable. This information is often read
     * from a service description document.
     */
    const HStateVariableInfo& info() const;

    /*!
     * Changes the value of the state variable. If the instance is evented,
     * the valueChanged() signal is emitted after the value has been changed.
     *
     * \param newValue specifies the new value of the state variable. The new value
     * must have the same underlying data type as the previous value
     * (and the default value). If the new value has different data type, the value
     * is not changed, no event is sent and false is returned.
     *
     * \retval true in case the new value was successfully set.
     * \retval false in case the new value could not be set.
     *
     * \remarks the new value will be set if the value:
     *  - does not violate the defined constraints
     *  - has the same variant type or the type of the new value can be converted
     *  to the same variant type
     *  - is not QVariant::Invalid
     */
    bool setValue(const QVariant& newValue);

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when the value of the state variable has changed.
     *
     * \param source specifies the state variable that sent the event.
     *
     * \param event specifies information about the event that occurred.
     *
     * \remarks This signal has thread affinity to the thread where the object
     * resides. Do not connect to this signal from other threads.
     */
    void valueChanged(
        Herqq::Upnp::HServerStateVariable* source,
        const Herqq::Upnp::HStateVariableEvent& event);
};

}
}

#endif /* HSERVER_STATEVARIABLE_H_ */
