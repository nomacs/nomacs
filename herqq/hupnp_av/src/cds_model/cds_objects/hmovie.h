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

#ifndef HMOVIE_H_
#define HMOVIE_H_

#include <HUpnpAv/HVideoItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HMoviePrivate;

/*!
 * \brief This class represents content that is a movie.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.videoItem.movie .
 *
 * \headerfile hmove.h HMovie
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMovie :
    public HVideoItem
{
Q_OBJECT
H_DISABLE_COPY(HMovie)
H_DECLARE_PRIVATE(HMovie)

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
    HMovie(const QString& clazz = sClass(), CdsType cdsType = sType());
    HMovie(HMoviePrivate&);

    // Documented in HClonable
    virtual HMovie* newInstance() const;

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
    HMovie(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMovie();

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
     * \brief Returns the DVD region code.
     *
     * upnp:DVDRegionCode, ContentDirectory:3, Appendix B.14.1.
     *
     * \return The DVD region code.
     *
     * \sa setDvdRegionCode()
     */
    qint32 dvdRegionCode() const;

    /*!
     * \brief Returns the user-friendly name of the associated broadcast channel.
     *
     * upnp:channelName, ContentDirectory:3, Appendix B.11.2.
     *
     * \return The user-friendly name of the associated broadcast channel.
     *
     * \sa setChannelName()
     */
    QString channelName() const;

    /*!
     * \brief Returns the start time of a scheduled program.
     *
     * \return The start time of a scheduled program.
     *
     * \sa setScheduledStartTime()
     */
    HScheduledTime scheduledStartTime() const;

    /*!
     * \brief Returns the end time of a scheduled program.
     *
     * \return The end time of a scheduled program.
     *
     * \sa setScheduledEndTime()
     */
    HScheduledTime scheduledEndTime() const;

    /*!
     * \brief Indicates the scheduled duration of the movie.
     *
     * upnp:scheduledDuration, ContentDirectory:3, Appendix B.11.5.
     *
     * \return The scheduled duration of the movie.
     *
     * \sa setScheduledDuration()
     */
    HContentDuration scheduledDuration() const;

    /*!
     * \brief Returns the name of the program.
     *
     * upnp:programTitle, ContentDirectory:3, Appendix B.8.2.
     *
     * \return The name of the program.
     *
     * \sa setProgramTitle()
     */
    QString programTitle() const;

    /*!
     * \brief Returns the name of the series.
     *
     * upnp:seriesTitle, ContentDirectory:3, Appendix B.8.2.
     *
     * \return The name of the series.
     *
     * \sa setSeriesTitle()
     */
    QString seriesTitle() const;

    /*!
     * \brief Returns the total number of episodes in the series to which this content
     * belongs.
     *
     * upnp:episodeCount, ContentDiretory:3, Appendix B.8.6.
     *
     * \return The total number of episodes in the series to which this content
     * belongs.
     *
     * \sa setEpisodeCount()
     */
    quint32 episodeCount() const;

    /*!
     * \brief Returns the episode number within the series to which this content
     * belongs.
     *
     * upnp:episodeNumber, ContentDirectory:3, Appendix B.8.7.
     *
     * \return The episode number within the series to which this content
     * belongs.
     *
     * \sa setEpisodeNumber()
     */
    quint32 episodeNumber() const;

    /*!
     * \brief Specifies the type of storage medium used for the content.
     *
     * \param arg specifies the type of storage medium used for the content.
     *
     * \sa storageMedium()
     */
    void setStorageMedium(const HStorageMedium& arg);

    /*!
     * \brief Specifies the DVD region code.
     *
     * \param arg specifies DVD region code.
     *
     * \sa dvdRegionCode()
     */
    void setDvdRegionCode(qint32 arg);

    /*!
     * \brief Specifies the the user-friendly name of the associated broadcast channel.
     *
     * \param arg specifies the user-friendly name of the associated broadcast channel.
     *
     * \sa channelName()
     */
    void setChannelName(const QString& arg);

    /*!
     * \brief Specifies the the start time of a scheduled program.
     *
     * \param arg specifies the start time of a scheduled program.
     *
     * \sa scheduledStartTime()
     */
    void setScheduledStartTime(const HScheduledTime& arg);

    /*!
     * \brief Specifies the the end time of a scheduled program.
     *
     * \param arg specifies the end time of a scheduled program.
     *
     * \sa scheduledEndTime()
     */
    void setScheduledEndTime(const HScheduledTime& arg);

    /*!
     * \brief Specifies the scheduled duration of the movie.
     *
     * \param arg specifies the scheduled duration of the movie.
     *
     * \sa scheduledDuration()
     */
    void setScheduledDuration(const HContentDuration& arg);

    /*!
     * \brief Specifies the name of the program.
     *
     * \param arg specifies the name of the program.
     *
     * \sa programTitle()
     */
    void setProgramTitle(const QString& arg);

    /*!
     * \brief Specifies the the name of the series.
     *
     * \param arg specifies the name of the series.
     *
     * \sa seriesTitle()
     */
    void setSeriesTitle(const QString& arg);

    /*!
     * \brief Specifies the the total number of episodes in the series to which this content
     * belongs.
     *
     * \param arg specifies the total number of episodes in the series to which this content
     * belongs.
     *
     * \sa episodeCount()
     */
    void setEpisodeCount(quint32 arg);

    /*!
     * \brief Specifies the the episode number within the series to which this content
     * belongs.
     *
     * \param arg specifies the episode number within the series to which this content
     * belongs.
     *
     * \sa episodeNumber()
     */
    void setEpisodeNumber(quint32 arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return Movie; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.videoItem.movie"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HMovie* create() { return new HMovie(); }
};

}
}
}

#endif /* HMOVIE_H_ */
