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

#ifndef HBOOKMARKITEM_H_
#define HBOOKMARKITEM_H_

#include <HUpnpAv/HItem>
#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HBookmarkItemPrivate;

/*!
 * \brief This class represents a piece of data that can be used to recover
 * previous state information of a AVTransport and a RenderingControl service instance.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.bookmarkItem.
 *
 * \headerfile hbookmarkitem.h HBookmarkItem
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HBookmarkItem :
    public HItem
{
Q_OBJECT
H_DISABLE_COPY(HBookmarkItem)
H_DECLARE_PRIVATE(HBookmarkItem)

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
    HBookmarkItem(const QString& clazz = sClass(), CdsType cdsType = sType());
    HBookmarkItem(HBookmarkItemPrivate&);

    // Documented in HClonable
    virtual HBookmarkItem* newInstance() const;

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
    HBookmarkItem(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HBookmarkItem();

    /*!
     * \brief Returns the object ID of the content item that is the target of this
     * bookmark.
     *
     * upnp:bookmarkedObjectId, ContentDirectory:3, Appendix B.13.3.
     *
     * \return The object ID of the content item that is the target of this
     * bookmark.
     *
     * \sa setBookmarkedObjectId()
     */
    QString bookmarkedObjectId() const;

    /*!
     * \brief Indicates whether the bookmarked content item will \b ever have normal
     * playable content.
     *
     * @neverPlayble, ContentDirectory:3, Appendix B.13.1.
     *
     * \return \e true if the bookmarked content item will \b never have
     * normal playable content.
     *
     * \sa setNeverPlayable()
     */
    bool neverPlayable() const;

    /*!
     * \brief Returns identifying information about the device, which state information
     * is captured in stateVariableCollection().
     *
     * upnp:deviceUDN, ContentDirectory:3, Appendix B.13.4.
     *
     * \return identifying information about the device, which state information
     * is captured in stateVariableCollection().
     *
     * \sa setDeviceUdn()
     */
    HDeviceUdn deviceUdn() const;

    /*!
     * \brief Returns the primary date of the content.
     *
     * dc:date, ContentDirectory:3, Appendix B.7.6.
     *
     * \return The primary date of the content.
     */
    QDateTime date() const;

    /*!
     * \brief Returns the state variable information stored to this book mark.
     *
     * upnp:stateVariableCollection, ContentDirectory:3, Appendix B.13.5.
     *
     * \return The state variable information stored to this book mark.
     *
     * \sa setStateVariableCollection()
     */
    HStateVariableCollection stateVariableCollection() const;

    /*!
     * \brief Specifies the object ID of the content item that is the target of this
     * bookmark.
     *
     * \param arg specifies the object ID of the content item that is the target of this
     * bookmark.
     *
     * \sa bookmarkedObjectId()
     */
    void setBookmarkedObjectId(const QString& arg);

    /*!
     * \brief Specifies whether the bookmarked content item will \b ever have normal
     * playable content.
     *
     * \param arg specifies whether the bookmarked content item will \b ever have normal
     * playable content.
     *
     * \sa neverPlayable()
     */
    void setNeverPlayable(bool arg);

    /*!
     * \brief Specifies information about the device, which state information
     * is captured in stateVariableCollection().
     *
     * \param arg specifies information about the device, which state information
     * is captured in stateVariableCollection().
     *
     * \sa deviceUdn()
     */
    void setDeviceUdn(const HDeviceUdn& arg);

    /*!
     * \brief Specifies the primary date of the content.
     *
     * \param arg specifies the primary date of the content.
     *
     * \sa date()
     */
    void setDate(const QDateTime& arg);

    /*!
     * \brief Specifies the state variable information stored to this book mark.
     *
     * \param arg specifies the state variable information stored to this book mark.
     *
     * \sa stateVariableCollection()
     */
    void setStateVariableCollection(const HStateVariableCollection& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return BookmarkItem; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.bookmarkItem"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HBookmarkItem* create() { return new HBookmarkItem(); }
};

}
}
}

#endif /* HBOOKMARKITEM_H_ */
