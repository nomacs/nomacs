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

#ifndef HVIDEOITEM_H_
#define HVIDEOITEM_H_

#include <HUpnpAv/HItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HVideoItemPrivate;

/*!
 * \brief This class represents content intended for viewing.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.videoItem .
 *
 * \headerfile hvideoitem.h HVideoItem
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HVideoItem :
    public HItem
{
Q_OBJECT
H_DISABLE_COPY(HVideoItem)
H_DECLARE_PRIVATE(HVideoItem)

protected:

    /*!
     * Constructs a new instance.
     *
     * \param clazz specifies the class identifier.
     *
     * \param cdsType specifies CDS type.
     */
    HVideoItem(const QString& clazz = sClass(), CdsType cdsType = sType());
    HVideoItem(HVideoItemPrivate&);

    // Documented in HClonable
    virtual HVideoItem* newInstance() const;

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
    HVideoItem(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HVideoItem();

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
     * \brief Returns the viewer ratings of this item.
     *
     * upnp:rating, ContentDirectory, Appendix B.8.9.
     *
     * \return The viewer ratings of this item.
     *
     * \sa setRatings()
     */
    QList<HRating> ratings() const;

    /*!
     * \brief Retrieves the actors of this item.
     *
     * upnp:actor, ContentDirectory:3, Appendix B.3.1.
     *
     * \return The actors of this item.
     *
     * \sa setActors()
     */
    QList<HPersonWithRole> actors() const;

    /*!
     * \brief Returns the directors of this item.
     *
     * upnp:director, ContentDirectory:3, Appendix B.3.5.
     *
     * \return The directors of this item.
     *
     * \sa setDirectors()
     */
    QStringList directors() const;

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
     * \brief Returns the publishers of this item.
     *
     * dc:publisher, ContentDirectory:3, Appendix B.3.6.
     *
     * \return The publishers of this item.
     *
     * \sa setPublishers()
     */
    QStringList publishers() const;

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
     * \brief Returns the URLs of resources to which this object relates.
     *
     * dc:language, ContentDirectory:3, Appendix B.5.4.
     *
     * \return The URLs of resources to which this object relates.
     *
     * \sa setRelations()
     */
    QList<QUrl> relations() const;

    /*!
     * \brief Returns the number of times the content has been played.
     *
     * upnp:playbackCount, ContentDirectory:3, Appendix B.7.8.
     *
     * \return The number of times the content has been played.
     *
     * \sa setPlaybackCount()
     */
    qint32 playbackCount() const;

    /*!
     * \brief Returns the date time when the item was played last.
     *
     * upnp:lastPlaybackTime, ContentDirectory:3, Appendix B.7.9.
     *
     * \return The date time when the item was played last.
     *
     * \sa setLastPlaybackTime()
     */
    QDateTime lastPlaybackTime() const;

    /*!
     * \brief Returns the time offset within the content where the last playback
     * was suspended.
     *
     * upnp:lastPlaybackPosition, ContentDirectory:3, Appendix B.7.10.
     *
     * \return The time offset within the content where the last playback
     * was suspended.
     *
     * \sa setLastPlaybackPosition()
     */
    HContentDuration lastPlaybackPosition() const;

    /*!
     * \brief Returns the day of the week when the recording started.
     *
     * upnp:recordedDayOfWeek, ContentDirectory:3, Appendix B.7.13.
     *
     * \return The day of the week when the recording started.
     *
     * \sa setRecordedDayOfWeek()
     */
    HDayOfWeek recordedDayOfWeek() const;

    /*!
     * \brief Returns the ID of the srs:recordSchedule object that was used to
     * create this recorded content.
     *
     * upnp:srsRecordScheduleID, ContentDirectory:3, Appendix B.7.14.
     *
     * \return The ID of the srs:recordSchedule object that was used to
     * create this recorded content.
     *
     * \sa setSrsRecordScheduleId()
     */
    QString srsRecordScheduleId() const;

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
     * \brief Specifies the viewer ratings of this item.
     *
     * \param arg specifies the viewer ratings of this item.
     *
     * \sa ratings()
     */
    void setRatings(const QList<HRating>& arg);

    /*!
     * \brief Sets the actors of this item.
     *
     * \param arg specifies the actors of this item.
     *
     * \sa actors()
     */
    void setActors(const QList<HPersonWithRole>& arg);

    /*!
     * \brief Sets the directors of this item.
     *
     * \param arg specifies the directors of this item.
     *
     * \sa directors()
     */
    void setDirectors(const QStringList& arg);

     /*!
     * \brief Specifies a brief description of the content item.
     *
     * \param arg specifies a brief description of the content item.
     *
     * \sa description()
     */
    void setDescription(const QString& arg);

    /*!
     * \brief Sets the publishers of this item.
     *
     * \param arg specifies the publishers of this item.
     *
     * \sa publishers()
     */
    void setPublishers(const QStringList& arg);

    /*!
     * \brief Specifies the languages used in the content.
     *
     * \param arg specifies the languages used in the content.
     *
     * \sa languages()
     */
    void setLanguages(const QStringList& arg);

    /*!
     * \brief Specifies the URLs of resources to which this object relates.
     *
     * \param arg specifies the URLs of resources to which this object relates.
     *
     * \sa relations()
     */
    void setRelations(const QList<QUrl>& arg);

    /*!
     * \brief Specifies the number of times the content has been played.
     *
     * \param arg specifies the number of times the content has been played.
     *
     * \sa playbackCount()
     */
    void setPlaybackCount(qint32 arg);

    /*!
     * \brief Specifies the date time when the item was played last.
     *
     * \param arg specifies the date time when the item was played last.
     *
     * \sa lastPlaybackTime()
     */
    void setLastPlaybackTime(const QDateTime& arg);

    /*!
     * \brief Specifies the time offset within the content where the last playback
     * was suspended.
     *
     * \param arg specifies the time offset within the content where the last playback
     * was suspended.
     *
     * \sa lastPlaybackPosition()
     */
    void setLastPlaybackPosition(const HContentDuration& arg);

    /*!
     * \brief Specifies the day of the week when the recording started.
     *
     * \param arg specifies the day of the week when the recording started.
     *
     * \sa recordedDayOfWeek()
     */
    void setRecordedDayOfWeek(HDayOfWeek arg);

    /*!
     * \brief Specifies the ID of the srs:recordSchedule object that was used to
     * create this recorded content.
     *
     * \param arg specifies the ID of the srs:recordSchedule object that was used to
     * create this recorded content.
     *
     * \sa srsRecordScheduleId()
     */
    void setSrsRecordScheduleId(const QString& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return VideoItem; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.videoItem"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HVideoItem* create() { return new HVideoItem(); }
};

}
}
}

#endif /* HVIDEOITEM_H_ */
