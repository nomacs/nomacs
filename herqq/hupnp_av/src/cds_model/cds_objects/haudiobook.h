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

#ifndef HAUDIOBOOK_H_
#define HAUDIOBOOK_H_

#include <HUpnpAv/HAudioItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAudioBookPrivate;

/*!
 * \brief This class represents audio content that is the narration of a book
 * (as opposed to, for example, a news broadcast or a song)
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.audioItem.audioBook.
 *
 * \headerfile haudiobook.h HAudioBook
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAudioBook :
    public HAudioItem
{
Q_OBJECT
H_DISABLE_COPY(HAudioBook)
H_DECLARE_PRIVATE(HAudioBook)

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
    HAudioBook(const QString& clazz = sClass(), CdsType cdsType = sType());
    HAudioBook(HAudioBookPrivate&);

    // Documented in HClonable
    virtual HAudioBook* newInstance() const;

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
    HAudioBook(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAudioBook();

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
     * \brief Returns the producers of this item.
     *
     * upnp:producer, ContentDirectory:3, Appendix B.3.4.
     *
     * \return The producers of this item.
     *
     * \sa setProducers()
     */
    QStringList producers() const;

    /*!
     * \brief Returns the contributors of this item.
     *
     * dc:contributor, ContentDirectory:3, Appendix B.3.7.
     *
     * \return The contributors of this item.
     *
     * \sa setContributors()
     */
    QStringList contributors() const;

    /*!
     * \brief Returns the primary date of the content.
     *
     * dc:date, ContentDirectory:3, Appendix B.7.6.
     *
     * \return The primary date of the content.
     */
    QDateTime date() const;

    /*!
     * \brief Specifies the type of storage medium used for the content.
     *
     * \param arg specifies the type of storage medium used for the content.
     *
     * \sa storageMedium()
     */
    void setStorageMedium(const HStorageMedium& arg);

    /*!
     * \brief Sets the producers of this item.
     *
     * \param arg specifies the producers of this item.
     *
     * \sa producers()
     */
    void setProducers(const QStringList& arg);

    /*!
     * \brief Specifies the contributors of this item.
     *
     * \param arg specifies the contributors of this item.
     *
     * \sa contributors()
     */
    void setContributors(const QStringList& arg);

    /*!
     * \brief Specifies the primary date of the content.
     *
     * \param arg specifies the primary date of the content.
     *
     * \sa date()
     */
    void setDate(const QDateTime& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return AudioBook; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.audioItem.audioBook"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HAudioBook* create() { return new HAudioBook(); }
};

}
}
}

#endif /* HAUDIOBOOK_H_ */
