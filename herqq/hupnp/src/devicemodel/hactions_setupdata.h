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

#ifndef HACTIONS_SETUPDATA_H_
#define HACTIONS_SETUPDATA_H_

#include <HUpnpCore/HActionInvoke>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

class HActionSetupPrivate;

/*!
 * \brief This class is used to specify information that can be used to setup
 * an HServerAction or validate a UPnP action.
 *
 * \headerfile hactions_setupdata.h HActionSetup
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HActionsSetupData, HClientAction, HServerAction
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HActionSetup
{
private:

    QSharedDataPointer<HActionSetupPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HActionSetup();

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the name of the action. If the name value contains
     * special characters other than hyphens or dots the instance will be
     * invalid and name() will be empty.
     *
     * \param incReq specifies the \e inclusion \e requirement of the action.
     *
     * \sa isValid()
     *
     * \remarks the version() is set to 1.
     */
    explicit HActionSetup(
        const QString& name,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the name of the action. If the name value contains
     * special characters other than hyphens or dots the instance will be
     * invalid and name() will be empty.
     *
     * \param version specifies the UPnP service version in which the action
     * was first specified.
     *
     * \param incReq specifies the \e inclusion \e requirement of the action.
     *
     * \sa isValid()
     */
    HActionSetup(
        const QString& name,
        int version,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \a other.
     */
    HActionSetup(const HActionSetup&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \a other to this.
     */
    HActionSetup& operator=(const HActionSetup&);

    /*!
     * \brief Destroys the instance.
     */
    ~HActionSetup();

    /*!
     * \brief Returns the setup information of the action's input arguments.
     *
     * \return The setup information of the action's input arguments.
     *
     * \sa setInputArguments()
     */
    const HActionArguments& inputArguments() const;

    /*!
     * \brief Returns the setup information of the action's output arguments.
     *
     * \return The setup information of the action's output arguments.
     *
     * \sa setOutputArguments()
     */
    const HActionArguments& outputArguments() const;

    /*!
     * \brief Returns the <em>inclusion requirement</em> of the action.
     *
     * \return The <em>inclusion requirement</em> of the action.
     *
     * \sa setInclusionRequirement()
     */
    HInclusionRequirement inclusionRequirement() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, that is,
     * the name(), version() and the inclusionRequirement() are properly defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of the action.
     *
     * \return The name of the action.
     *
     * \sa setName()
     */
    QString name() const;

    /*!
     * \brief Returns the UPnP service version in which the action
     * was first specified.
     *
     * \return The UPnP service version in which the action
     * was first specified.
     *
     * \sa setVersion()
     */
    int version() const;

    /*!
     * \brief Specifies the action's input arguments.
     *
     * \param args specifies the setup information for the action's input arguments.
     *
     * \sa inputArguments()
     */
    void setInputArguments(const HActionArguments& args);

    /*!
     * \brief Specifies the action's output arguments.
     *
     * \param args specifies the setup information for the action's output arguments.
     *
     * \sa outputArguments()
     */
    void setOutputArguments(const HActionArguments& args);

    /*!
     * \brief Sets the name of the action.
     *
     * \param name specifies the name of the action.
     *
     * \param err is a pointer to a \c QString that contains an error description
     * in case the name could not be set. This is an optional parameter.
     *
     * \return \e true in case the specified name was successfully set.
     *
     * \sa name()
     */
    bool setName(const QString& name, QString* err = 0);

     /*!
     * \brief Sets the <em>inclusion requirement</em> of the action.
     *
     * \param arg specifies the <em>inclusion requirement</em> of the action.
     *
     * \sa inclusionRequirement()
     */
    void setInclusionRequirement(HInclusionRequirement arg);

    /*!
     * \brief Specifies the UPnP service version in which the action
     * was first specified.
     *
     * \param version specifies the UPnP service version in which the action
     * was first specified.
     *
     * \sa version()
     */
    void setVersion(int version);
};

/*!
 * \brief This class is used to specify information that can be used to setup
 * HServerAction instances or generally validate the actions of a UPnP service.
 *
 * \headerfile hactions_setupdata.h HActionsSetupData
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HActionSetup
 */
class H_UPNP_CORE_EXPORT HActionsSetupData
{

private:

    QHash<QString, HActionSetup> m_actionSetupInfos;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HActionsSetupData();

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \return \e true in case the item was added. The item will not be added
     * if the instance already contains an item with the
     * same name as \a newItem or the \a newItem is invalid.
     *
     * \sa remove()
     */
    bool insert(const HActionSetup& newItem);

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
     * \brief Retrieves an action setup object.
     *
     * \param name specifies the name of the item to be retrieved.
     *
     * \return The item with the specified name. Note that the returned item
     * is invalid, i.e. HActionSetup::isValid() returns false in case no item
     * with the specified name was found.
     *
     * \sa contains()
     */
    HActionSetup get(const QString& name) const;

    /*!
     * This is a convenience method for setting the inclusion requirement
     * element of an item.
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

    /*!
     * \brief Indicates if the instance contains an item with the specified name.
     *
     * \param name specifies the name of the item.
     *
     * \return \e true when the instance contains an item with the specified name.
     *
     * \sa get()
     */
    inline bool contains(const QString& name) const
    {
        return m_actionSetupInfos.contains(name);
    }

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
    inline qint32 size() const
    {
        return m_actionSetupInfos.size();
    }

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true in case the instance has no items.
     */
    inline bool isEmpty() const
    {
        return m_actionSetupInfos.isEmpty();
    }

    /*!
     * Removes every contained object.
     */
    inline void clear()
    {
        m_actionSetupInfos.clear();
    }
};

}
}

#endif /* HACTIONS_SETUPDATA_H_ */
