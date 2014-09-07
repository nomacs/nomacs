/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HLASTCHANGEINFO_H_
#define HLASTCHANGEINFO_H_

#include <HUpnpAv/HUpnpAv>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsLastChangeInfoPrivate;

/*!
 * \brief This class is used to contain information of a LastChange event.
 *
 * \headerfile hcds_lastchange_info.h HCdsLastChangeInfo
 *
 * \ingroup hupnp_av_cds_browsing
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HMediaBrowser
 */
class H_UPNP_AV_EXPORT HCdsLastChangeInfo
{
public:

    /*!
     * \brief This enumeration defines the different event types used in LastChange
     * Data Format.
     */
    enum EventType
    {
        /*!
         * The event type is not defined.
         */
        Undefined = 0,

        /*!
         * A new object was added.
         */
        ObjectAdded,

        /*!
         * An existing object was modified.
         */
        ObjectModified,

        /*!
         * An object was deleted.
         */
        ObjectDeleted,
    };

private:

    HCdsLastChangeInfoPrivate* h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HCdsLastChangeInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param objectId specifies the ID of the object.
     *
     * \param eventType specifies the type of the event.
     *
     * \sa isValid()
     */
    HCdsLastChangeInfo(const QString& objectId, EventType eventType);

    /*!
     * \brief Destroys the instance.
     */
    ~HCdsLastChangeInfo();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HCdsLastChangeInfo(const HCdsLastChangeInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HCdsLastChangeInfo& operator=(const HCdsLastChangeInfo&);

    /*!
     * \brief Specifies the ID of the object.
     *
     * \param id specifies the ID of the object.
     *
     * \sa objectId()
     */
    void setObjectId(const QString& id);

    /*!
     * \brief Specifies the type of the event.
     *
     * \param type specifies the type of the event.
     *
     * \sa eventType()
     */
    void setEventType(EventType type);

    /*!
     * \brief Specifies the ID of the parent object.
     *
     * \param id specifies the ID of the parent object.
     *
     * \sa parentId()
     */
    void setParentId(const QString& id);

    /*!
     * \brief Specifies the value of the upnp:class property.
     *
     * \param objClass specifies the value of the upnp:class property.
     *
     * \sa objectClass()
     */
    void setObjectClass(const QString& objClass);

    /*!
     * Specifies the value of the SystemUpdateID state variable that
     * resulted when the object was modified.
     *
     * \param arg specifies the value of the SystemUpdateID state variable that
     * resulted when the object was modified.
     *
     * \sa updateId()
     */
    void setUpdateId(quint32 arg);

    /*!
     * Specifies whether the change was part of a sub-tree operation.
     *
     * \param set specifies whether the change was part of a sub-tree operation.
     *
     * \sa stUpdate()
     */
    void setStUpdate(bool set);

    /*!
     * \brief Indicates the validity of the object.
     *
     * \return \e true in case the object is valid, i.e. at least objectId() and
     * eventType() are defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the ID of the object.
     *
     * \return The ID of the object.
     *
     * \sa setObjectId()
     */
    QString objectId() const;

    /*!
     * \brief Returns the type of the event.
     *
     * \return The type of the event.
     *
     * \sa setEventType()
     */
    EventType eventType() const;

    /*!
     * \brief Returns the ID of the parent object.
     *
     * \return The ID of the parent object.
     *
     * \sa setParentId()
     */
    QString parentId() const;

    /*!
     * \brief Returns the value of the upnp:class property.
     *
     * \return the value of the upnp:class property.
     *
     * \sa setObjectClass()
     */
    QString objectClass() const;

    /*!
     * Returns the value of the SystemUpdateID state variable that
     * resulted when the object was modified.
     *
     * \return the value of the SystemUpdateID state variable that
     * resulted when the object was modified.
     *
     * \sa setUpdateId()
     */
    quint32 updateId() const;

    /*!
     * Indicates whether the change was part of a sub-tree operation.
     *
     * \return \e true if the change was part of a sub-tree operation.
     *
     * \sa setStUpdate();
     */
    bool stUpdate() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HCdsLastChangeInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HCdsLastChangeInfo& obj1, const HCdsLastChangeInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HCdsLastChangeInfo
 */
inline bool operator!=(const HCdsLastChangeInfo& obj1, const HCdsLastChangeInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HLASTCHANGEINFO_H_ */
