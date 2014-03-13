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

#ifndef HMEDIAINFO_H_
#define HMEDIAINFO_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QSharedDataPointer>

class QUrl;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMediaInfoPrivate;

/*!
 * \brief This is a class used to contain media related state information of a virtual
 * AV Transport instance.
 *
 * \headerfile hmediainfo.h HMediaInfo
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractTransportService::getMediaInfo(),
 * HAbstractTransportService::getMediaInfo_ext(),
 * HAvTransportAdapter::getMediaInfo(),
 * HAvTransportAdapter::getMediaInfo_ext()
 */
class H_UPNP_AV_EXPORT HMediaInfo
{
private:

    QSharedDataPointer<HMediaInfoPrivate> h_ptr;

public:

    /*!
     * \brief This enumeration is used to indicate whether particular media is
     * track-aware.
     *
     * An example of a track-aware media is a CD and an example of track-unaware
     * media is a VHS-tape.
     *
     * \sa mediaCategoryToString(), mediaCategoryFromString()
     */
    enum MediaCategory
    {
        /*!
         * The category is not defined.
         *
         * This value is used in error scenarios, such as when querying this
         * information is not supported.
         */
        Undefined = 0,

        /*!
         * No media present.
         */
        NoMedia,

        /*!
         * The media is track aware. An example of this is a CD.
         */
        TrackAware,

        /*!
         * The media is track-unaware. An example of this is a VHS-tape.
         */
        TrackUnaware
    };

    /*!
     * \brief Returns a string representation of the specified value.
     *
     * \return a string representation of the specified value.
     */
    static QString toString(MediaCategory category);

    /*!
     * \brief Returns a WriteStatus value corresponding to the specified string.
     *
     * \return a WriteStatus value corresponding to the specified string.
     */
    static MediaCategory mediaCategoryFromString(const QString& category);

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HMediaInfo();

    /*!
     * Creates a new instance.
     *
     * \param currentUri specifies the location of the media content.
     *
     * \param currentMetadata specifies a DIDL-Lite XML fragment containing
     * metadata of the current media. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa isValid()
     */
    HMediaInfo(const QUrl& currentUri, const QString& currentMetadata = QString());

    /*!
     * Creates a new, invalid instance.
     *
     * \param numOfTracks specifies the number of tracks the media contains.
     *
     * \param duration specifies the duration of the media content.
     *
     * \param currentUri specifies the location of the media content.
     *
     * \param currentMetadata specifies a DIDL-Lite XML fragment containing
     * metadata of the current media. The format is specified in the
     * ContentDirectory specification.
     *
     * \param nextUri specifies the location of the next media to be played
     * once the playing of the current media finishes.
     *
     * \param nextMetadata specifies a DIDL-Lite XML fragment containing
     * metadata of the next media to be played.
     * The format is specified in the ContentDirectory specification.
     *
     * \param playMedium specifies the storage medium of the resource located
     * at \a currentUri.
     *
     * \param recordMedium specifies the storage medium where the
     * resource located at \a currentUri will be recorded if record
     * procedure is initiated.
     *
     * \param writeStatus indicates the write protection status of the
     * current media.
     *
     * \param type indicates whether the current media is track-aware.
     *
     * \sa isValid()
     */
    HMediaInfo(
        quint32 numOfTracks,
        const HDuration& duration,
        const QUrl& currentUri,
        const QString& currentMetadata,
        const QUrl& nextUri,
        const QString& nextMetadata,
        const HStorageMedium& playMedium,
        const HStorageMedium& recordMedium,
        const HRecordMediumWriteStatus& writeStatus,
        MediaCategory type = Undefined);

    /*!
     * \brief Destroys the instance.
     */
    ~HMediaInfo();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HMediaInfo(const HMediaInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HMediaInfo& operator=(const HMediaInfo&);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. at least the
     * currentUri() is defined.
     */
    bool isValid() const;

    /*!
     * \brief Indicates whether the media is track-aware.
     *
     * \return The media category indicating whether the media is track-aware.
     *
     * \sa setMediaCategory()
     */
    MediaCategory mediaCategory() const;

    /*!
     * \brief Returns the number of tracks the media contains.
     *
     * \return The number of tracks the media contains.
     *
     * \sa setNumberOfTracks()
     */
    quint32 numberOfTracks() const;

    /*!
     * \brief Returns the duration of the media content.
     *
     * \return The duration of the media content.
     *
     * \sa setMediaDuration()
     */
    HDuration mediaDuration() const;

    /*!
     * \brief Returns the location of the current media content.
     *
     * \return The location of the current media content.
     *
     * \sa setCurrentUri()
     */
    QUrl currentUri() const;

    /*!
     * \brief Returns the metadata associated with the current media.
     *
     * \return a DIDL-Lite XML fragment containing
     * metadata of the current media. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa setCurrentUriMetadata()
     */
    QString currentUriMetadata() const;

    /*!
     * \brief Returns the location of the next media to be played
     * once the playing of the current media finishes.
     *
     * \return The location of the next media to be played
     * once the playing of the current media finishes.
     *
     * \sa setNextUri()
     */
    QUrl nextUri() const;

    /*!
     * \brief Returns the metadata associated with the next media to be played.
     *
     * \return a DIDL-Lite XML fragment containing
     * metadata of the next media to be played.
     * The format is specified in the ContentDirectory specification.
     *
     * \sa setNextUriMetadata()
     */
    QString nextUriMetadata() const;

    /*!
     * \brief Indicates the medium at which the current media is stored.
     *
     * \return The storage medium of the resource located
     * at \a currentUri().
     *
     * \sa setPlayMedium()
     */
    HStorageMedium playMedium() const;

    /*!
     * \brief Indicates the medium used for recording.
     *
     * \return The storage medium where the
     * resource located at \a currentUri() will be recorded if record
     * procedure is initiated.
     *
     * \sa setRecordMedium()
     */
    HStorageMedium recordMedium() const;

    /*!
     * \brief Returns the write protection status of the media.
     *
     * \return The write protection status of the media.
     *
     * \sa setWriteStatus()
     */
    HRecordMediumWriteStatus writeStatus() const;

    /*!
     * \brief Sets the location of the current media content.
     *
     * \param arg specifies the location of the current media content.
     *
     * \sa currentUri()
     */
    void setCurrentUri(const QUrl& arg);

    /*!
     * \brief Sets the metadata associated with the current media.
     *
     * \param arg specifies a DIDL-Lite XML fragment containing
     * metadata of the current media. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa currentUriMetadata()
     */
    void setCurrentUriMetadata(const QString& arg);

    /*!
     * \brief Specifies the media category indicating whether the media is track-aware.
     *
     * \param arg specifies the media category indicating whether the media is track-aware.
     *
     * \sa mediaCategory()
     */
    void setMediaCategory(MediaCategory arg);

    /*!
     * \brief Sets the duration of the media content.
     *
     * \param arg specifies the duration of the media content.
     *
     * \sa mediaDuration()
     */
    void setMediaDuration(const HDuration& arg);

    /*!
     * \brief Sets the location of the next media to be played
     * once the playing of the current media finishes.
     *
     * \param arg specifies the location of the next media to be played
     * once the playing of the current media finishes.
     *
     * \sa nextUri()
     */
    void setNextUri(const QUrl& arg);

    /*!
     * \brief Sets the metadata associated with the next media to be played.
     *
     * \param arg specifies a DIDL-Lite XML fragment containing
     * metadata of the next media to be played.
     * The format is specified in the ContentDirectory specification.
     *
     * \sa nextUriMetadata()
     */
    void setNextUriMetadata(const QString& arg);

    /*!
     * \brief Sets the number of tracks the media contains.
     *
     * \param arg the number of tracks the media contains.
     *
     * \sa numberOfTracks()
     */
    void setNumberOfTracks(quint32 arg);

    /*!
     * \brief Sets the medium at which the current media is stored.
     *
     * \param arg specifies the storage medium of the resource located
     * at \a currentUri().
     *
     * \sa playMedium()
     */
    void setPlayMedium(const HStorageMedium& arg);

    /*!
     * \brief Sets the medium used for recording.
     *
     * \param arg specifies the storage medium where the
     * resource located at \a currentUri() will be recorded if record
     * procedure is initiated.
     *
     * \sa recordMedium()
     */
    void setRecordMedium(const HStorageMedium& arg);

    /*!
     * \brief Sets the write protection status of the media.
     *
     * \param arg specifies the write protection status of the media.
     *
     * \sa writeStatus()
     */
    void setWriteStatus(const HRecordMediumWriteStatus& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HMediaInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HMediaInfo& obj1, const HMediaInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HMediaInfo
 */
inline bool operator!=(const HMediaInfo& obj1, const HMediaInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HMEDIAINFO_H_ */
