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

#ifndef HMUSICTRACK_H_
#define HMUSICTRACK_H_

#include <HUpnpAv/HAudioItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMusicTrackPrivate;

/*!
 * \brief This class represents music audio content.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.audioItem.musicTrack.
 *
 * \headerfile hmusictrack.h HMusicTrack
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMusicTrack :
    public HAudioItem
{
Q_OBJECT
H_DISABLE_COPY(HMusicTrack)
H_DECLARE_PRIVATE(HMusicTrack)

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
    HMusicTrack(const QString& clazz = sClass(), CdsType cdsType = sType());
    HMusicTrack(HMusicTrackPrivate&);

    // Documented in HClonable
    virtual HMusicTrack* newInstance() const;

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
    HMusicTrack(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMusicTrack();

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
     * \brief Returns the albums to which this item belongs.
     *
     * upnp:album, ContentDirectory:3, Appendix B.4.2.
     *
     * \return The albums to which this item belongs.
     *
     * \sa setAlbums()
     */
    QStringList albums() const;

    /*!
     * \brief Returns the original track number on the original medium.
     *
     * upnp:originalTrackNumber, ContentDirectory:3, Appendix B.14.2.
     *
     * \return The original track number on the original medium.
     *
     * \sa setOriginalTrackNumber()
     */
    qint32 originalTrackNumber() const;

    /*!
     * \brief Returns the playlists to which this item belongs.
     *
     * upnp:playList, ContentDirectory:3, Appendix B.4.3.
     *
     * \return The playlists to which this item belongs.
     *
     * \sa setPlaylists()
     */
    QStringList playlists() const;

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
     * \brief Sets the artists of this item.
     *
     * \param arg specifies the artists of this item.
     *
     * \sa artists()
     */
    void setArtists(const QList<HPersonWithRole>& arg);

    /*!
     * \brief Specifies the albums to which this item belongs.
     *
     * \param arg the albums to which this item belongs.
     *
     * \sa albums()
     */
    void setAlbums(const QStringList& arg);

    /*!
     * \brief Specifies the original track number on the original medium.
     *
     * \param arg specifies the original track number on the original medium.
     *
     * \sa originalTrackNumber()
     */
    void setOriginalTrackNumber(qint32 arg);

    /*!
     * \brief Specifies the playlists to which this item belongs.
     *
     * \param arg specifies the playlists to which this item belongs.
     *
     * \sa playlists()
     */
    void setPlaylists(const QStringList& arg);

    /*!
     * \brief Specifies the type of storage medium used for the content.
     *
     * \param arg specifies the type of storage medium used for the content.
     *
     * \sa storageMedium()
     */
    void setStorageMedium(const HStorageMedium& arg);

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
    inline static CdsType sType() { return MusicTrack; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.audioItem.musicTrack"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HMusicTrack* create() { return new HMusicTrack(); }
};

}
}
}

#endif /* HMUSICTRACK_H_ */
