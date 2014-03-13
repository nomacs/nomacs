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

#ifndef HSTORAGEVOLUME_H_
#define HSTORAGEVOLUME_H_

#include <HUpnpAv/HContainer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HStorageVolumePrivate;

/*!
 * \brief This class represents all, or a partition of,
 * some physical storage unit of a single type (as indicated by storageMedium())
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.storageVolume.
 *
 * \headerfile hstoragevolume.h HStorageVolume
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HStorageVolume :
    public HContainer
{
Q_OBJECT
H_DISABLE_COPY(HStorageVolume)
H_DECLARE_PRIVATE(HStorageVolume)

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
    HStorageVolume(const QString& clazz = sClass(), CdsType cdsType = sType());
    HStorageVolume(HStorageVolumePrivate&);

    // Documented in HClonable
    virtual HStorageVolume* newInstance() const;

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
    HStorageVolume(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HStorageVolume();

    /*!
     * \brief Returns the total capacity in bytes of the storage represented by the
     * container.
     *
     * upnp:storageTotal, ContentDirectory:3, Appendix B.6.1.
     *
     * \return he total capacity in bytes of the storage represented by the
     * container.
     *
     * \sa setStorageTotal()
     */
    qint64 storageTotal() const;

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
     * \brief Returns the total free capacity in bytes of the storage represented by
     * the container.
     *
     * upnp:storageFree, ContentDirectory:3, Appendix B.6.3
     *
     * \return The total free capacity in bytes of the storage represented by
     * the container.
     *
     * \sa setStorageFree()
     */
    qint64 storageFree() const;

    /*!
     * \brief Indicates the type of storage medium used for the content.
     *
     * upnp:storageMedium, ContentDirectory:3, Appendix B.6.5.
     *
     * \return The type of storage medium used for the content.
     *
     * \sa setStorageMedium()
     */
    HStorageMedium storageMedium() const;

    /*!
     * \brief Specifies the total capacity in bytes of the storage represented by the
     * container.
     *
     * \param arg specifies the total capacity in bytes of the storage represented by the
     * container.
     *
     * \sa storageTotal()
     */
    void setStorageTotal(qint64 arg);

    /*!
     * \brief Specifies the combined space in bytes used by all the objects held
     * in storage represented by this container.
     *
     * \param arg specifies the combined space in bytes used by all the objects held
     * in storage represented by this container.
     *
     * \sa storageUsed()
     */
    void setStorageUsed(qint64 arg);

    /*!
     * \brief Specifies the total free capacity in bytes of the storage represented by
     * the container.
     *
     * \param arg specifies the total free capacity in bytes of the storage represented by
     * the container.
     *
     * \sa storageFree()
     */
    void setStorageFree(qint64 arg);

    /*!
     * \brief Specifies the type of storage medium used for the content.
     *
     * \param arg specifies the type of storage medium used for the content.
     *
     * \sa storageMedium()
     */
    void setStorageMedium(const HStorageMedium& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return StorageVolume; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.storageVolume"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HStorageVolume* create() { return new HStorageVolume(); }
};

}
}
}

#endif /* HSTORAGEVOLUME_H_ */
