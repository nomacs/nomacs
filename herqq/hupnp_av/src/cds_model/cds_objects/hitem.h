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

#ifndef HITEM_H_
#define HITEM_H_

#include <HUpnpAv/HObject>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HItemPrivate;

/*!
 * \brief This class is used to represent individual content objects.
 *
 * \brief This class represents \e individual content objects, which refers to
 * objects that cannot contain other objects. For instance, such an item is a
 * a music track or a photo. The class identifier specified by the AV Working
 * Committee is \c object.item.
 *
 * \headerfile hitem.h HItem
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HItem :
    public HObject
{
Q_OBJECT
H_DISABLE_COPY(HItem)
H_DECLARE_PRIVATE(HItem)

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
    HItem(const QString& clazz = sClass(), CdsType cdsType = sType());

    //
    // \internal
    //
    HItem(HItemPrivate& dd);

    // Documented in HClonable
    virtual HItem* newInstance() const;

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
    HItem(
        const QString& title,
        const QString& parentId = QString(),
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HItem();

    /*!
     * \brief Returns the bookmark IDs.
     *
     * As defined in the ContentDirectory:3 specification, this is an object ID
     * of a bookmark item that is associated with this content item and that
     * marks a specific location with its content.
     *
     * upnp:bookmarkID, ContentDirectory:3, Appendix B.13.2.
     *
     * \return The bookmark IDs.
     *
     * \sa setBookmarkIds()
     */
    QStringList bookmarkIds() const;

    /*!
     * \brief Returns the ID of the referenced item in case this item is a reference
     * item.
     *
     * \return The ID of the referenced item in case this item is a reference
     * item. Otherwise the return value is empty.
     *
     * \sa setRefId()
     */
    QString refId() const;

    /*!
     * \brief Indicates if the item is a reference item.
     *
     * \return \e true in case the item is a reference item.
     */
    bool isRef() const;

    /*!
     * Checks if any of the HResource objects contained by this item has
     * specified a MIME type.
     *
     * \return \e true in case any of the HResource objects contained by this
     * item has specified a MIME type.
     */
    bool hasContentFormat() const;

    /*!
     * \brief Specifies the book mark IDs.
     *
     * \param bookmarkId specifies the book mark IDs.
     *
     * \sa bookmarkIds()
     */
    void setBookmarkIds(const QStringList& bookmarkIds);

    /*!
     * \brief Specifies the ID that this item references.
     *
     * \param referenceId specifies the ID that this item references.
     */
    void setRefId(const QString& referenceId);

    /*!
     * Modifies the HResource objects contained by this item to contain the
     * specified MIME type.
     *
     * This is a convenience method for modifying the content format of
     * HProtocolInfo objects contained by the HResource objects.
     *
     * \param mimeType specifies the MIME type to be set to the contained
     * HResource objects.
     *
     * \sa resources()
     */
    void setContentFormat(const QString& mimeType);

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item"; }

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return Item; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HItem* create() { return new HItem(); }
};

}
}
}

#endif /* HITEM_H_ */
