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

#ifndef HEPGCONTAINER_H_
#define HEPGCONTAINER_H_

#include <HUpnpAv/HContainer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HEpgContainerPrivate;

/*!
 * \brief This class represents a program guide container, which may contain
 * any kind of objects for EPG information.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.epgContainer.
 *
 * \headerfile hepgcontainer.h HEpgContainer
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HEpgContainer :
    public HContainer
{
Q_OBJECT
H_DISABLE_COPY(HEpgContainer)
H_DECLARE_PRIVATE(HEpgContainer)

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
    HEpgContainer(const QString& clazz = sClass(), CdsType cdsType = sType());
    HEpgContainer(HEpgContainerPrivate&);

    // Documented in HClonable
    virtual HEpgContainer* newInstance() const;

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
    HEpgContainer(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HEpgContainer();

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
     * \brief Returns the call sign of the source radio station.
     *
     *  upnp:radioCallSign, ContentDirectory, Appendix B.10.1
     *
     * \return The call sign of the source radio station.
     *
     * \sa setRadioCallSign()
     */
    QString radioCallSign() const;

    /*!
     * \brief Returns some identification information of the radio station.
     *
     * upnp:radioStationID, ContentDirectory, Appendix B.10.2.
     *
     * \return some identification information of the radio station.
     *
     * \sa setRadioStationId()
     */
    QString radioStationId() const;

    /*!
     * \brief Returns the radio band type of the radio station.
     *
     * upnp:radioBand, ContentDirectory, Appendix B.10.3.
     *
     * \return The radio band type of the radio station.
     *
     * \sa setRadioBand()
     */
    HRadioBand radioBand() const;

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
     * \brief Returns the time range within which all items in this container belong.
     *
     * upnp:dateTimeRange, ContentDirectory:3, Appendix B.9.8.
     *
     * \return The time range within which all items in this container belong.
     *
     * \sa setDateTimeRange()
     */
    HDateTimeRange dateTimeRange() const;

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
     * \brief Specifies either the source channel, or a channel that is associated with
     * the content item.
     *
     * \param arg specifies either the source channel, or a channel that is associated with
     * the content item.
     *
     * \sa channelId()
     */
    void setChannelId(const HMatchingId& arg);

    /*!
     * \brief Specifies the call sign of the source radio station.
     *
     * \param arg specifies the call sign of the source radio station.
     *
     * \sa radioCallSign()
     */
    void setRadioCallSign(const QString& arg);

    /*!
     * \brief Specifies some identification information of the radio station.
     *
     * \param arg specifies some identification information of the radio station.
     *
     * \sa radioStationId()
     */
    void setRadioStationId(const QString& arg);

    /*!
     * \brief Specifies the radio band type of the radio station.
     *
     * \param arg specifies the radio band type of the radio station.
     *
     * \sa radioBand()
     */
    void setRadioBand(const HRadioBand& arg);

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
     * \brief Specifies the time range within which all items in this container belong.
     *
     * \param arg specifies the time range within which all items in this container belong.
     *
     * \sa dateTimeRange()
     */
    void setDateTimeRange(const HDateTimeRange& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return EpgContainer; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.epgContainer"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HEpgContainer* create() { return new HEpgContainer(); }
};

}
}
}

#endif /* HEPGCONTAINER_H_ */
