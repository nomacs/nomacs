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

#ifndef HMUSICALBUM_H_
#define HMUSICALBUM_H_

#include <HUpnpAv/HAlbum>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMusicAlbumPrivate;

/*!
 * \brief This class represents an ordered collection of music tracks and albums.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.album.musicAlbum.
 *
 * \headerfile hmusicalbum.h HMusicAlbum
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMusicAlbum :
    public HAlbum
{
Q_OBJECT
H_DISABLE_COPY(HMusicAlbum)
H_DECLARE_PRIVATE(HMusicAlbum)

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
    HMusicAlbum(const QString& clazz = sClass(), CdsType cdsType = sType());
    HMusicAlbum(HMusicAlbumPrivate&);

    // Documented in HClonable
    virtual HMusicAlbum* newInstance() const;

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
    HMusicAlbum(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMusicAlbum();

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
     * \brief Returns the URIs where the album art can be retrieved.
     *
     * upnp:albumArtUrls, ContentDirectory:3, Appendix B.5.1.
     *
     * \return The URIs where the album art can be retrieved.
     *
     * \sa setAlbumArtUrls()
     */
    QList<QUrl> albumArtUrls() const;

    /*!
     * \brief Returns the Table of Contents of the object.
     *
     * upnp:toc, ContentDirectory:3, Appendix B.14.3.
     *
     * \return The Table of Contents of the object.
     *
     * \sa toc()
     */
    QString toc() const;

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
     * \brief Sets the producers of this item.
     *
     * \param arg specifies the producers of this item.
     *
     * \sa producers()
     */
    void setProducers(const QStringList& arg);

    /*!
     * \brief Specifies the URIs where the album art can be retrieved.
     *
     * \param arg specifies the URIs where the album art can be retrieved.
     *
     * \sa albumArtUrls()
     */
    void setAlbumArtUrls(const QList<QUrl>& arg);

    /*!
     * \brief Specifies the Table of Contents of the object.
     *
     * \param arg specifies the Table of Contents of the object.
     *
     * \sa toc()
     */
    void setToc(const QString& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return MusicAlbum; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.album.musicAlbum"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HMusicAlbum* create() { return new HMusicAlbum(); }
};

}
}
}

#endif /* HMUSICALBUM_H_ */
