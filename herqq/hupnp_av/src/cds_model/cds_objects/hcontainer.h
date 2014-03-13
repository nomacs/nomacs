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

#ifndef HCONTAINER_H_
#define HCONTAINER_H_

#include <HUpnpAv/HObject>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HContainerEventInfoPrivate;

/*!
 * \brief This class is used to contain information of an HContainer modification event.
 *
 * \headerfile hcontainer.h HContainerEventInfo
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HContainer
 */
class H_UPNP_AV_EXPORT HContainerEventInfo
{
private:

    QSharedDataPointer<HContainerEventInfoPrivate> h_ptr;

public:

    /*!
     * \brief This enumeration defines the different event types instances of this class
     * are used to depict.
     */
    enum EventType
    {
        /*!
         * This event type is not defined.
         */
        Undefined,

        /*!
         * This value is used when a child object has been added into
         * this container.
         */
        ChildAdded,

        /*!
         * This value is used when a child object has been removed from
         * this container.
         */
        ChildRemoved,

        /*!
         * This value is used when a child object of a container has been
         * modified.
         */
        ChildModified
    };

    /*!
     * \brief Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HContainerEventInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the type of the event.
     *
     * \param childId specifies the ID of the child object that was the "target"
     * of this event.
     */
    HContainerEventInfo(
        EventType type, const QString& childId, quint32 updateId = 0);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HContainerEventInfo(const HContainerEventInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HContainerEventInfo& operator=(const HContainerEventInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HContainerEventInfo();

    /*!
     * \brief Indicates if the instance contains any data.
     *
     * \return \e true if the instance does not contain any data.
     */
    bool isValid() const;

    /*!
     * \brief Returns the type of the event.
     *
     * \return The type of the event.
     *
     * \sa setType()
     */
    EventType type() const;

    /*!
     * \brief Returns the ID of the child object that was the "target"
     * of this event.
     *
     * \return The ID of the child object that was the "target"
     * of this event.
     *
     * \sa setChildId()
     */
    QString childId() const;

    /*!
     * \brief Returns the value of the state variable \c SystemUpdateId at the time this
     * event was generated.
     *
     * \return the value of the state variable \c SystemUpdateId at the time this
     * event was generated.
     *
     * \sa setUpdateId()
     */
    quint32 updateId() const;

    /*!
     * \brief Specifies the type of the event.
     *
     * \param type specifies the type of the event.
     *
     * \sa type()
     */
    void setType(EventType type);

    /*!
     * \brief Specifies the ID of the child object that was the "target"
     * of this event.
     *
     * \param arg specifies the ID of the child object that was the "target"
     * of this event.
     *
     * \sa childId()
     */
    void setChildId(const QString& arg);

    /*!
     * \brief Specifies the value of the state variable \c SystemUpdateId at the time this
     * event was generated.
     *
     * \param arg specifies the value of the state variable \c SystemUpdateId at the time this
     * event was generated.
     *
     * \sa updateId()
     */
    void setUpdateId(quint32 arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HContainerEventInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HContainerEventInfo&, const HContainerEventInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HContainerEventInfo
 */
inline bool operator!=(const HContainerEventInfo& obj1, const HContainerEventInfo& obj2)
{
    return !(obj1 == obj2);
}

class HContainerPrivate;

/*!
 * \brief This class is used to represent a collection of individual content objects
 * and other collections of individual content objects.
 *
 * The class identifier specified by the AV Working
 * Committee is \c object.container .
 *
 * \headerfile hcontainer.h HContainer
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HContainer :
    public HObject
{
Q_OBJECT
H_DISABLE_COPY(HContainer)
H_DECLARE_PRIVATE(HContainer)

protected:

    /*!
     * Constructs a new instance.
     *
     * \param clazz specifies the UPnP class of the object. This cannot be empty.
     *
     * \param cdsType specifies the CDS type of the object. This cannot be
     * HObject::UndefinedCdsType.
     *
     * \sa isInitialized()
     */
    HContainer(const QString& clazz = sClass(), CdsType cdsType = sType());

    //
    // \internal
    //
    HContainer(HContainerPrivate&);

    /*!
     * Specifies the number of child objects the container is supposed to contain.
     *
     * \param arg specifies the number of child objects the container is supposed to contain.
     *
     * \sa expectedChildCount()
     */
    void setExpectedChildCount(quint32 arg);

    virtual bool validate() const;

    // Documented in HClonable
    virtual HContainer* newInstance() const;
    // Documented in HClonable
    virtual void doClone(HClonable* target) const;

public:

     /*!
     * \brief Creates a new instance.
     *
     * \param title specifies the title of the object.
     *
     * \param parentId specifies the ID of the object that contains this
     * object. If the object has no parent, this has to be left empty.
     *
     * \param id specifies the ID of this object. If this is not specified,
     * a unique identifier within the running process is created for the object.
     *
     * \sa isValid()
     */
    HContainer(
        const QString& title,
        const QString& parentId = QString(),
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HContainer();

    /*!
     * \brief Returns the IDs of the individual content objects that this container
     * contains.
     *
     * \return The IDs of the individual content objects that this container
     * contains.
     *
     * \sa setChildIds(), expectedChildCount()
     */
    QSet<QString> childIds() const;

    /*!
     * \brief Indicates if a search can be performed to this container.
     *
     * \return \e true if a search can be performed to this container.
     *
     * \sa setSearchable()
     */
    bool searchable() const;

    /*!
     * \brief Returns the container update ID.
     *
     * \return The container update ID, which is the value of the state variable
     * \c SystemUpdateID at the time of the most recent \e Container \e Modification
     * of this instance.
     *
     * \sa setContainerUpdateId()
     */
    quint32 containerUpdateId() const;

    /*!
     * \brief Returns the total number of child objects that have been deleted
     * from this container since the last initialization.
     *
     * \return the total number of child objects that have been deleted
     * from this container since the last initialization.
     *
     * \sa setTotalDeletedChildCount()
     */
    quint32 totalDeletedChildCount() const;

    /*!
     * \brief Returns the class types that can be created within this container.
     *
     * \return The class types that can be created within this container.
     *
     * \sa setCreateClassInfos()
     */
    QList<HCdsClassInfo> createClassInfos() const;

    /*!
     * \brief Returns the class types that can be used in searches targeted to this
     * container.
     *
     * \return The class types that can be used in searches targeted to this
     * container.
     *
     * \sa setSearchClassInfos(), ContentDirectory:3, Appendix B.1.11.
     */
    QList<HCdsClassInfo> searchClassInfos() const;

    /*!
     * Returns the number of child objects the container is supposed to contain.
     *
     * At server-side the expected child count always equals to the size of
     * childIds(). However, at client-side the number of child objects of a
     * container are may be known before the actual child objects are available.
     * Further, child objects may be added to a container object progressively
     * and because of that non-zero size of childIds() does not mean that every
     * child object of a container has been retrieved.
     *
     * You should never consider the size of childIds() as the number of all child
     * objects of a container at client-side, unless the size equals to the
     * value this method returns.
     *
     * \return the number of child objects the container is supposed to contain.
     *
     * \sa setExpectedChildCount(), childIds()
     */
    quint32 expectedChildCount() const;

    /*!
     * Indicates if the container contains the specified child ID.
     *
     * \param childId specifies the child ID to check.
     *
     * \return \e true if the container contains the specified child ID.
     */
    bool hasChildId(const QString& childId) const;

    /*!
     * \brief Sets the IDs of the individual content objects that this container contains.
     *
     * \param childIds specifies the IDs of the individual content objects that this
     * container contains.
     */
    void setChildIds(const QSet<QString>& childIds);

    /*!
     * Adds the specified IDs of the individual content objects to this container.
     *
     * \param childIds specifies the IDs of the individual content objects to
     * be added to this container.
     */
    void addChildIds(const QSet<QString>& childIds);

    /*!
     * Adds an ID of an individual content object to this container.
     *
     * \param childId specifies the ID of the individual content object to
     * be added to this container.
     */
    void addChildId(const QString& childId);

    /*!
     * Removes the specified child ID.
     *
     * \param childId specifies the child ID to be removed.
     */
    void removeChildId(const QString& childId);

    /*!
     * Removes the specified child IDs.
     *
     * \param childIds specifies the child IDs to be removed.
     */
    void removeChildIds(const QSet<QString>& childIDs);

    /*!
     * \brief Specifies the class types that can be created within this
     * container.
     *
     * \param arg specifies the class types that can be created within this
     * container.
     *
     * \sa searchClassInfos()
     */
    void setCreateClassInfos(const QList<HCdsClassInfo>& arg);

    /*!
     * \brief Specifies the container update ID.
     *
     * \param arg specifies the container update ID.
     *
     * \sa containerUpdateId()
     */
    void setContainerUpdateId(quint32 arg);

    /*!
     * \brief Specifies whether a search can be performed to this container.
     *
     * \param arg specifies whether a search can be performed to this container.
     *
     * \sa searchable()
     */
    void setSearchable(bool arg);

    /*!
     * \brief Specifies class types that can be used in searches targeted to this
     * container.
     *
     * \param arg specifies the class types that can be used in searches
     * targeted to this container.
     *
     * \sa searchClassInfos()
     */
    void setSearchClassInfos(const QList<HCdsClassInfo>& arg);

    /*!
     * Specifies the total number of child objects that have been deleted
     * from this container since the last initialization.
     *
     * \param arg specifies the total number of child objects that have been deleted
     * from this container since the last initialization.
     *
     * \sa totalDeletedChildCount()
     */
    void setTotalDeletedChildCount(quint32 arg);

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container"; }

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return Container; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HContainer* create() { return new HContainer(); }

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when a container has been modified.
     *
     * \param source specifies the HContainer that sent the event.
     *
     * \param eventInfo specifies information of the \e Container \e Modification that
     * occurred.
     *
     * \note Since HContainer knows only the IDs of its child objects, HContainer
     * does not monitor its child objects for changes. Because of this, an
     * HContainer instance will never emit this signal when a child object
     * has been modified. However, this type of functionality is provided by
     * HAbstractCdsDataSource.
     *
     * \sa HAbstractCdsDataSource
     */
    void containerModified(
        Herqq::Upnp::Av::HContainer* source,
        const Herqq::Upnp::Av::HContainerEventInfo& eventInfo);
};

}
}
}

#endif /* HCONTAINER_H_ */
