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

#ifndef HMUSICGENRE_H_
#define HMUSICGENRE_H_

#include <HUpnpAv/HGenreContainer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMusicGenrePrivate;

/*!
 * \brief This class represents a \c genre \c container where the genre indicates a
 * <em>style of music</em>.
 *
 * A musicGenre container can
 * contain objects of class musicArtist, musicAlbum, audioItem or sub-musicgenres
 * of the same class (for example, Rock contains Alternative Rock).
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.genre.musicGenre.
 *
 * \headerfile hmusicgenre.h HMusicGenre
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMusicGenre :
    public HGenreContainer
{
Q_OBJECT
H_DISABLE_COPY(HMusicGenre)
H_DECLARE_PRIVATE(HMusicGenre)

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
    HMusicGenre(const QString& clazz = sClass(), CdsType cdsType = sType());
    HMusicGenre(HMusicGenrePrivate&);

    // Documented in HClonable
    virtual HMusicGenre* newInstance() const;

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
    HMusicGenre(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMusicGenre();

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return MusicGenre; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.genre.musicGenre"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HMusicGenre* create() { return new HMusicGenre(); }
};

}
}
}

#endif /* HMUSICGENRE_H_ */
