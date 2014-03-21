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

#ifndef HMUSICARTIST_H_
#define HMUSICARTIST_H_

#include <HUpnpAv/HPerson>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMusicArtistPrivate;

/*!
 * \brief This class represents an unordered collection of objects associated
 * with a music artist.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.person.musicArtist.
 *
 * \headerfile hmusicartist.h HMusicArtist
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMusicArtist :
    public HPerson
{
Q_OBJECT
H_DISABLE_COPY(HMusicArtist)
H_DECLARE_PRIVATE(HMusicArtist)

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
    HMusicArtist(const QString& clazz = sClass(), CdsType cdsType = sType());
    HMusicArtist(HMusicArtistPrivate&);

    // Documented in HClonable
    virtual HMusicArtist* newInstance() const;

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
    HMusicArtist(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMusicArtist();

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
     * \brief Returns the URI where the discography of this artist is available.
     *
     * upnp:artistDiscographyURI, ContentDirectory:3, Appendix B.5.2.
     *
     * \return The URI where the discography of this artist is available.
     *
     * \sa artistDiscographyUri()
     */
    QUrl artistDiscographyUri() const;

    /*!
     * \brief Specifies the genres to which this item belongs.
     *
     * \param arg specifies the genres to which this item belongs.
     *
     * \sa genres()
     */
    void setGenres(const QList<HGenre>& arg);

    /*!
     * \brief Specifies the URI where the discography of this artist is available.
     *
     * \param arg specifies the URI where the discography of this artist
     * is available.
     *
     * \sa artistDiscographyUri()
     */
    void setArtistDiscographyUri(const QUrl& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return MusicArtist; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.person.musicArtist"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HMusicArtist* create() { return new HMusicArtist(); }
};

}
}
}

#endif /* HMUSICARTIST_H_ */
