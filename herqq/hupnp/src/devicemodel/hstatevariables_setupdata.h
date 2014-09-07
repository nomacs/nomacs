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

#ifndef HSTATEVARIABLES_SETUPDATA_H_
#define HSTATEVARIABLES_SETUPDATA_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HStateVariableInfo>

#include <QtCore/QHash>
#include <QtCore/QString>

template<typename T>
class QSet;

namespace Herqq
{

namespace Upnp
{

/*!
 * \brief This class is used to specify information that can be used to validate
 * UPnP state variables.
 *
 * \headerfile hstatevariables_setupdata.h HStateVariablesSetupData
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HStateVariablesSetupData
{

public:

    /*!
     * \brief This enumeration specifies the actions the HUPnP device model builder
     * should take when it encounters an unknown state variable definition in a
     * service description file.
     */
    enum DefaultInclusionPolicy
    {
        /*!
         * The unknown state variable should be accepted.
         */
        Accept,

        /*!
         * The unknown state variable should be rejected. In this case the build
         * of a device tree is aborted.
         */
        Deny
    };

private:

    QHash<QString, HStateVariableInfo> m_setupData;
    DefaultInclusionPolicy m_defaultInclusionPolicy;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \param defIncPol specifies the default inclusion policy for state variables
     * that are \b not contained in this instance.
     *
     * \sa isEmpty(), defaultInclusionPolicy()
     */
    HStateVariablesSetupData(DefaultInclusionPolicy defIncPol = Accept);

    /*!
     * \brief Returns the default inclusion policy.
     *
     * The default inclusion policy specifies the action to take when a
     * state variable definition in a service description file does not map
     * to any HStateVariableInfo object contained within this instance.
     *
     * \return The default inclusion policy.
     */
    DefaultInclusionPolicy defaultInclusionPolicy() const;

    /*!
     * \brief Indicates if the instance contains an item that has the
     * specified name.
     *
     * \param name specifies the name of the item.
     *
     * \return \e true when the instance contains an item that
     * has the specified name.
     *
     * \sa get(), isEmpty()
     */
    bool contains(const QString& name) const;

    /*!
     * \brief Retrieves an item.
     *
     * \param name specifies the name of the item to be retrieved.
     *
     * \return The item with the specified name. Note that the returned item
     * is invalid, i.e. HStateVariableInfo::isValid() returns false in case no item
     * with the specified name was found.
     *
     * \sa contains(), isEmpty()
     */
    HStateVariableInfo get(const QString& name) const;

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true in case the instance has no items.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the names of the contained items.
     *
     * \return The names of the contained items.
     */
    QSet<QString> names() const;

    /*!
     * \brief Returns the number of contained items.
     *
     * \return The number of contained items.
     */
    qint32 size() const;

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \return \e true in case the item was added. The item will not be added
     * if the instance already contains an item that has the
     * same name as the \c newItem.
     *
     * \sa remove()
     */
    bool insert(const HStateVariableInfo& newItem);

    /*!
     * Removes an existing item.
     *
     * \param name specifies the name of the item to be removed.
     *
     * \return \e true in case the item was found and removed.
     *
     * \sa insert()
     */
    bool remove(const QString& name);

    /*!
     * \brief Sets the inclusion requirement element of an item.
     *
     * \param name specifies the name of the item.
     *
     * \param incReq specifies the inclusion requirement value.
     *
     * \return \e true when the item was found and the inclusion requirement
     * element was set.
     */
    bool setInclusionRequirement(
        const QString& name, HInclusionRequirement incReq);
};

}
}

#endif /* HSTATEVARIABLES_SETUPDATA_H_ */
