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

#ifndef HPLAYLISTCONTAINER_H_
#define HPLAYLISTCONTAINER_H_

#include <HUpnpAv/HContainer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HPlaylistContainerPrivate;

/*!
 * \brief This class represents a collection of objects, which may be a mixture of
 * video, audio and images and is typically created by a user.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.playlistContainer.
 *
 * \headerfile hplaylistcontainer.h HPlaylistContainer
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HPlaylistContainer :
    public HContainer
{
Q_OBJECT
H_DISABLE_COPY(HPlaylistContainer)
H_DECLARE_PRIVATE(HPlaylistContainer)

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
    HPlaylistContainer(const QString& clazz = sClass(), CdsType cdsType = sType());
    HPlaylistContainer(HPlaylistContainerPrivate&);

    // Documented in HClonable
    virtual HPlaylistContainer* newInstance() const;

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
    HPlaylistContainer(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HPlaylistContainer();

    /*!
     * \brief Retrieves the artists of this item.
     *
     * upnp:artist, ContentDirectory:3, Appendix B.3.1.
     *
     * \return The artists of this item.
     *
     * \sa setArtists()
     */
    QList<HPersonWithRole> artists() const;

    /*!
     * \brief Returns the genres to which this item belongs.
     *
     * upnp:genre, ContentDirectory:3, Appendix B.4.1.
     *
     * \return The genres to which this item belongs.
     *
     * \sa setGenres()
     */
    QList<HGenre> genres() const;

    /*!
     * \brief Returns a few lines of description of the content item.
     *
     * upnp:longDescripion, ContentDirectory:3, Appendix B.7.2.
     *
     * \return a few lines of description of the content item.
     *
     * \sa setLongDescription()
     */
    QString longDescription() const;

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
     * \brief Returns a brief description of the content item.
     *
     * dc:description, ContentDirectory:3, Appendix B.7.1.
     *
     * \return a brief description of the content item.
     *
     * \sa setDescription()
     */
    QString description() const;

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
     * \brief Returns the languages used in the content.
     *
     * dc:language, ContentDirectory:3, Appendix B.7.7.
     *
     * \return The languages used in the content.
     *
     * \sa setLanguages()
     */
    QStringList languages() const;

    /*!
     * \brief Returns descriptive information of legal rights held in over this resource.
     *
     * dc:rights, ContentDirectory:3, Appendix B.7.5.
     *
     * \return descriptive information of legal rights held in over this resource.
     *
     * \sa setRights()
     */
    QStringList rights() const;

    /*!
     * \brief Sets the artists of this item.
     *
     * \param arg specifies the artists of this item.
     *
     * \sa artists()
     */
    void setArtists(const QList<HPersonWithRole>& arg);

    /*!
     * \brief Specifies the genres to which this item belongs.
     *
     * \param arg specifies the genres to which this item belongs.
     *
     * \sa genres()
     */
    void setGenres(const QList<HGenre>& arg);

    /*!
     * \brief Specifies a few lines of description of the content item.
     *
     * \param arg specifies a few lines of description of the content item.
     *
     * \sa longDescription()
     */
    void setLongDescription(const QString& arg);

    /*!
     * \brief Sets the producers of this item.
     *
     * \param arg specifies the producers of this item.
     *
     * \sa producers()
     */
    void setProducers(const QStringList& arg);

    /*!
     * \brief Specifies the type of storage medium used for the content.
     *
     * \param arg specifies the type of storage medium used for the content.
     *
     * \sa storageMedium()
     */
    void setStorageMedium(const HStorageMedium& arg);

     /*!
     * \brief Specifies a brief description of the content item.
     *
     * \param arg specifies a brief description of the content item.
     *
     * \sa description()
     */
    void setDescription(const QString& arg);

    /*!
     * \brief Sets the contributors of this item.
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
     * \brief Specifies the languages used in the content.
     *
     * \param arg specifies the languages used in the content.
     *
     * \sa languages()
     */
    void setLanguages(const QStringList& arg);

    /*!
     * \brief Specifies descriptive information of legal rights held in over this
     * resource.
     *
     * \param arg specifies descriptive information of legal rights held in
     * over this resource.
     *
     * \sa rights()
     */
    void setRights(const QStringList& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return PlaylistContainer; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.playlistContainer"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HPlaylistContainer* create() { return new HPlaylistContainer(); }
};

}
}
}

#endif /* HPLAYLISTCONTAINER_H_ */
