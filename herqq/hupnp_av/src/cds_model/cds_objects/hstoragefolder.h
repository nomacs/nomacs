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

#ifndef HSTORAGEFOLDER_H_
#define HSTORAGEFOLDER_H_

#include <HUpnpAv/HContainer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HStorageFolderPrivate;

/*!
 * \brief This class represents a collection of objects stored on some
 * storage medium.
 *
 * The storage folder container MAY be writable, indicating
 * whether new items can be created as children of the storage folder container
 * or whether existing child items can be removed. If the parent container is not
 * writable, then the storage folder container itself cannot be writable.
 * A storage folder container MAY contain other objects, except a storage system
 * container or a storage volume container. A storage folder container MUST either
 * be a child of the root container or a child of another storage system container, a
 * storage volume container or a storageFolder container.
 * Examples of storage folder instances are:
 *
 * \li a directory on a Hard Disk Drive
 * \li a directory on CD-Rom, etc.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.storageFolder.
 *
 * \headerfile hstoragefolder.h HStorageFolder
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HStorageFolder :
    public HContainer
{
Q_OBJECT
H_DISABLE_COPY(HStorageFolder)
H_DECLARE_PRIVATE(HStorageFolder)

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
    HStorageFolder(const QString& clazz = sClass(), CdsType cdsType = sType());
    HStorageFolder(HStorageFolderPrivate&);

    // Documented in HClonable
    virtual HStorageFolder* newInstance() const;

    // Documented in HObject
    virtual bool validate() const;

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
    HStorageFolder(
        const QString& title,
        const QString& parentId = QString(),
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HStorageFolder();

    /*!
     * \brief Returns the combined space in bytes used by all the objects held
     * in storage represented by this container.
     *
     * upnp:storageUsed, ContentDirectory:3, Appendix B.6.2.
     *
     * \return The combined space in bytes used by all the objects held
     * in storage represented by this container.
     *
     * \sa setStorageUsed()
     */
    qint64 storageUsed() const;

    /*!
     * \brief Specifies the combined space in bytes used by all the objects held
     * in storage represented by this container.
     *
     * \param arg specifies the combined space in bytes used by all the objects held
     * in storage represented by this container.
     *
     * \sa storageUsed()
     */
    void setStorageUsed(quint32 arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return StorageFolder; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.storageFolder"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HStorageFolder* create() { return new HStorageFolder(); }
};

}
}
}

#endif /* HSTORAGEFOLDER_H_ */
