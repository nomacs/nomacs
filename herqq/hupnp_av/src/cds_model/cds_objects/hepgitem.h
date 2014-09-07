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

#ifndef HEPGITEM_H_
#define HEPGITEM_H_

#include <HUpnpAv/HItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HEpgItemPrivate;

/*!
 * \brief This class represents a program such as a single radio show,
 * a single TV show or a series of programs.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.epgItem.
 *
 * \headerfile hepgitem.h HEpgItem
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HEpgItem :
    public HItem
{
Q_OBJECT
H_DISABLE_COPY(HEpgItem)
H_DECLARE_PRIVATE(HEpgItem)

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
    HEpgItem(const QString& clazz = sClass(), CdsType cdsType = sType());
    HEpgItem(HEpgItemPrivate&);

    // Documented in HClonable
    virtual HEpgItem* newInstance() const;

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
    HEpgItem(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

   /*!
     * \brief Destroys the instance.
     *
     * \brief Destroys the instance.
     */
    virtual ~HEpgItem();

    /*!
     * \brief Returns the user friendly name of the channel group.
     *
     * upnp:channelGroupName, ContentDirectory:3, Appendix B.9.1.
     *
     * \return The user friendly name of the channel group.
     *
     * \sa setChannelGroupName()
     */
    HChannelGroupName channelGroupName() const;

    /*!
     * \brief Returns the name of the Electronic Program Guide service provider.
     *
     * upnp:epgProviderName, ContentDirectory:3, Appendix B.9.7.
     *
     * \return The name of the Electronic Program Guide service provider.
     *
     * \sa setEpgProviderName()
     */
    QString epgProviderName() const;

    /*!
     * \brief Returns the user friendly name of the service provider of this content.
     *
     * upnp:serviceProvider, ContentDirectory:3, Appendix B.9.4.
     *
     * \return The user friendly name of the service provider of this content.
     *
     * \sa setServiceProvider()
     */
    QString serviceProvider() const;

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
     * \brief Returns the number of the broadcast channel.
     *
     * upnp:channelNr, ContentDirectory, Appendix B.11.1.
     *
     * \return The number of the broadcast channel.
     *
     * \sa setChannelNr()
     */
    qint32 channelNr() const;

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
     * \brief Returns the unique ID of a program.
     *
     * upnp:programID, ContentDirectory:3, Appendix B.8.3.
     *
     * \return The unique ID of a program.
     *
     * \sa setProgramId()
     */
    HMatchingId programId() const;

    /*!
     * \brief Returns the unique ID of a series.
     *
     * upnp:seriesID, ContentDirectory:3, Appendix B.8.4.
     *
     * \return The unique ID of a series.
     *
     * \sa setProgramId()
     */
    HMatchingId seriesId() const;

    /*!
     * \brief Returns either the source channel, or a channel that is associated with
     * the content item.
     *
     * upnp:channelID, ContentDirectory:3, Appendix B.8.5.
     *
     * \return The source channel, if the content was created via a
     * \c ScheduledRecording service. Otherwise the returned value indicates the
     * channel that is associated with the content.
     *
     * \sa setChannelId()
     */
    HChannelId channelId() const;

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
    qint32 episodeCount() const;

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
     * \brief Returns a unique program code.
     *
     * upnp:programCode, ContentDirectory:3, Appendix B.8.8.
     *
     * \return a unique program code.
     *
     * \sa setProgramCode()
     */
    HProgramCode programCode() const;

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
     * \brief Indicates the broadcast novelty.
     *
     * upnp:episodeType, ContentDirectory:3, Appendix B.8.10.
     *
     * \return The broadcast novelty.
     */
    HEpisodeType episodeType() const;

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
     * \brief Returns the authors of this item.
     *
     * upnp:author, ContentDirectory:3, Appendix B.3.3.
     *
     * \return The authors of this item.
     *
     * \sa setAuthors()
     */
    QList<HPersonWithRole> authors() const;

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
     * \brief Returns the broadcast station call sign of the associated broadcast
     * channel.
     *
     * upnp:callSign, ContentDirectory:3, Appendix B.9.2.
     *
     * \return The broadcast station call sign of the associated broadcast
     * channel.
     *
     * \sa setCallSign()
     */
    QString callSign() const;

    /*!
     * \brief Returns the name of the broadcast network or distribution network
     * associated with this content.
     *
     * upnp:networkAffiliation, ContentDirectory:3, Appendix B.9.3.
     *
     * \return The name of the broadcast network or distribution network
     * associated with this content.
     *
     * \sa setNetworkAffiliation()
     */
    QString networkAffiliation() const;

    /*!
     * \brief Returns the price of the content item.
     *
     * upnp:price, ContentDirectory:3, Appendix B.9.5.
     *
     * \return The price of the content item.
     *
     * \sa setPrice()
     */
    HPrice price() const;

    /*!
     * \brief Indicates whether the content is pay-per-view.
     *
     * upnp:payPerView, ContentDirectory:3, Appendix B.9.6.
     *
     * \return \e true if the content is pay-per-view.
     *
     * \sa setPayPerView()
     */
    bool payPerView() const;

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
     * \brief Returns a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * upnp:icon, ContentDirectory:3, Appendix B.7.3.
     *
     * \return a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * \sa setIcon()
     */
    QUrl icon() const;

    /*!
     * \brief Returns some information of the region, associated with the source of the
     * object.
     *
     * upnp:region, ContentDirectory, Appendix B.7.4.
     *
     * \return some information of the region, associated with the source of the
     * object.
     *
     * \sa setRegion()
     */
    QString region() const;

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
     * \brief Indicates if the object can be used for recording purposes.
     *
     * upnp:recordable, ContentDirectory:3, Appendix B.7.16.
     *
     * \return \e true if the content represented by this object can potentially
     * be used for recording purposes.
     *
     * \sa setRecordable()
     */
    bool recordable() const;

    /*!
     * \brief Returns additional metadata of the object.
     *
     * upnp:foreignMetadata, ContentDirectory:3, Appendix B.16.1.
     *
     * \return additional metadata of the object.
     *
     * \sa setForeignMetadata()
     */
    HForeignMetadata foreignMetadata() const;

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
     * \brief Specifies the user friendly name of the channel group.
     *
     * \param arg specifies the user friendly name of the channel group.
     *
     * \sa channelGroupName()
     */
    void setChannelGroupName(const HChannelGroupName& arg);

    /*!
     * \brief Specifies the name of the Electronic Program Guide service provider.
     *
     * \param arg specifies the name of the Electronic Program Guide service provider.
     *
     * \sa epgProviderName()
     */
    void setEpgProviderName(const QString& arg);

    /*!
     * \brief Specifies the user friendly name of the service provider of this content.
     *
     * \param arg specifies the user friendly name of the service provider of this content.
     *
     * \sa serviceProvider()
     */
    void setServiceProvider(const QString& arg);

    /*!
     * \brief Specifies the the user-friendly name of the associated broadcast channel.
     *
     * \param arg specifies the user-friendly name of the associated broadcast channel.
     *
     * \sa channelName()
     */
    void setChannelName(const QString& arg);

    /*!
     * \brief Specifies the number of the broadcast channel.
     *
     * \param arg specifies the number of the broadcast channel.
     *
     * \sa channelNr()
     */
    void setChannelNr(qint32 arg);

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
     * \brief Specifies the unique ID of a program.
     *
     * \param arg specifies the unique ID of a program.
     *
     * \sa programId()
     */
    void setProgramId(const HMatchingId& arg);

    /*!
     * \brief Specifies the unique ID of a series.
     *
     * \param arg specifies the unique ID of a series.
     *
     * \sa seriesId()
     */
    void setSeriesId(const HMatchingId& arg);

    /*!
     * \brief Specifies either the source channel, or a channel that is associated with
     * the content item.
     *
     * \param arg specifies either the source channel, or a channel that is associated with
     * the content item.
     *
     * \sa channelId()
     */
    void setChannelId(const HChannelId& arg);

     /*!
     * \brief Specifies the the total number of episodes in the series to which this content
     * belongs.
     *
     * \param arg specifies the total number of episodes in the series to which this content
     * belongs.
     *
     * \sa episodeCount()
     */
    void setEpisodeCount(qint32 arg);

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
     * \brief Specifies a unique program code.
     *
     * \param arg specifies a unique program code.
     *
     * \sa programCode()
     */
    void setProgramCode(const HProgramCode& arg);

    /*!
     * \brief Specifies the viewer ratings of this item.
     *
     * \param arg specifies the viewer ratings of this item.
     *
     * \sa ratings()
     */
    void setRatings(const QList<HRating>& arg);

    /*!
     * \brief Specifies the broadcast novelty.
     *
     * \param arg specifies the broadcast novelty.
     *
     * \sa episodeType()
     */
    void setEpisodeType(const HEpisodeType& arg);

    /*!
     * \brief Specifies the genres to which this item belongs.
     *
     * \param arg specifies the genres to which this item belongs.
     *
     * \sa genres()
     */
    void setGenres(const QList<HGenre>& arg);

    /*!
     * \brief Sets the artists of this item.
     *
     * \param arg specifies the artists of this item.
     *
     * \sa artists()
     */
    void setArtists(const QList<HPersonWithRole>& arg);

    /*!
     * \brief Sets the actors of this item.
     *
     * \param arg specifies the actors of this item.
     *
     * \sa actors()
     */
    void setActors(const QList<HPersonWithRole>& arg);

    /*!
     * \brief Sets the authors of this item.
     *
     * \param arg specifies the authors of this item.
     *
     * \sa authors()
     */
    void setAuthors(const QList<HPersonWithRole>& arg);

    /*!
     * \brief Sets the producers of this item.
     *
     * \param arg specifies the producers of this item.
     *
     * \sa producers()
     */
    void setProducers(const QStringList& arg);

    /*!
     * \brief Sets the directors of this item.
     *
     * \param arg specifies the directors of this item.
     *
     * \sa directors()
     */
    void setDirectors(const QStringList& arg);

    /*!
     * \brief Sets the publishers of this item.
     *
     * \param arg specifies the publishers of this item.
     *
     * \sa publishers()
     */
    void setPublishers(const QStringList& arg);

    /*!
     * \brief Sets the contributors of this item.
     *
     * \param arg specifies the contributors of this item.
     *
     * \sa contributors()
     */
    void setContributors(const QStringList& arg);

    /*!
     * \brief Specifies the broadcast station call sign of the associated broadcast
     * channel.
     *
     * \param arg specifies the broadcast station call sign of the associated broadcast
     * channel.
     *
     * \sa callSign()
     */
    void setCallSign(const QString& arg);

    /*!
     * \brief Specifies the name of the broadcast network or distribution network
     * associated with this content.
     *
     * \param arg specifies the name of the broadcast network or distribution network
     * associated with this content.
     *
     * \sa networkAffiliation()
     */
    void setNetworkAffiliation(const QString& arg);

    /*!
     * \brief Specifies the price of the content item.
     *
     * \param arg specifies the price of the content item.
     *
     * \sa price()
     */
    void setPrice(const HPrice& arg);

    /*!
     * \brief Specifies whether the content is pay-per-view.
     *
     * \param arg specifies whether the content is pay-per-view.
     *
     * \sa payPerView()
     */
    void setPayPerView(bool arg);

     /*!
     * \brief Specifies a brief description of the content item.
     *
     * \param arg specifies a brief description of the content item.
     *
     * \sa description()
     */
    void setDescription(const QString& arg);

    /*!
     * \brief Specifies a few lines of description of the content item.
     *
     * \param arg specifies a few lines of description of the content item.
     *
     * \sa longDescription()
     */
    void setLongDescription(const QString& arg);

    /*!
     * \brief Specifies a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * \param arg specifies a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * \sa icon()
     */
    void setIcon(const QUrl& arg);

    /*!
     * \brief Specifies some information of the region, associated with the source of the
     * object.
     *
     * \param arg specifies some information of the region, associated with the source of the
     * object.
     *
     * \sa region()
     */
    void setRegion(const QString& arg);

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
     * \brief Specifies whether the object can be used for recording purposes.
     *
     * \param arg specifies whether the object can be used for recording purposes.
     *
     * \sa recordable()
     */
    void setRecordable(bool arg);

    /*!
     * \brief Specifies additional metadata of the object.
     *
     * \param arg specifies additional metadata of the object.
     *
     * \sa foreignMetadata()
     */
    void setForeignMetadata(const HForeignMetadata& arg) ;

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return EpgItem; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.epgItem"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HEpgItem* create() { return new HEpgItem(); }
};

}
}
}

#endif /* HEPGITEM_H_ */
