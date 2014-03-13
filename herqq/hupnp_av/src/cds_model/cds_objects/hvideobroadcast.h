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

#ifndef HVIDEOBROADCAST_H_
#define HVIDEOBROADCAST_H_

#include <HUpnpAv/HVideoItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HVideoBroadcastPrivate;

/*!
 * \brief This class represents a continuous stream from a video broadcast.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.videoItem.videoBroadcast.
 *
 * \headerfile hvideobroadcast.h HVideoBroadcast
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HVideoBroadcast :
    public HVideoItem
{
Q_OBJECT
H_DISABLE_COPY(HVideoBroadcast)
H_DECLARE_PRIVATE(HVideoBroadcast)

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
    HVideoBroadcast(const QString& clazz = sClass(), CdsType cdsType = sType());
    HVideoBroadcast(HVideoBroadcastPrivate&);

    // Documented in HClonable
    virtual HVideoBroadcast* newInstance() const;

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
    HVideoBroadcast(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HVideoBroadcast();

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
     * \brief Returns the relative strength of the signal that is used to retrieve
     * the content of the item.
     *
     * upnp:signalStrenth, ContentDirectory, Appendix B.12.1.
     *
     * \return The relative strength of the signal that is used to retrieve
     * the content of the item.
     *
     * \sa setSignalStrength()
     */
    qint32 signalStrength() const;

    /*!
     * \brief Indicates whether the signal strength is sufficient for a hardware to
     * lock onto the signal at the current frequency.
     *
     * upnp:signalLocked, ContentDirectory, Appendix B.12.2.
     *
     * \return \e true if the signal strength is sufficient for a hardware to
     * lock onto the signal at the current frequency.
     *
     * \sa setSignalLocked()
     */
    bool signalLocked() const;

    /*!
     * \brief Indicates whether a hardware resource is currently tuned to retrieve
     * the content represented by this item.
     *
     * upnp:tuned, ContentDirectory, Appendix B.12.3.
     *
     * \return \e true if a hardware resource is currently tuned to retrieve
     * the content represented by this item.
     *
     * \sa setTuned()
     */
    bool tuned() const;

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
     * \brief Specifies the number of the broadcast channel.
     *
     * \param arg specifies the number of the broadcast channel.
     *
     * \sa channelNr()
     */
    void setChannelNr(qint32 arg);

    /*!
     * \brief Specifies the relative strength of the signal that is used to retrieve
     * the content of the item.
     *
     * \param arg specifies the relative strength of the signal that is used to retrieve
     * the content of the item.
     *
     * \sa signalStrength()
     */
    void setSignalStrength(qint32 arg);

    /*!
     * \brief Specifies whether the signal strength is sufficient for a hardware to
     * lock onto the signal at the current frequency.
     *
     * \param arg specifies whether the signal strength is sufficient for a hardware to
     * lock onto the signal at the current frequency.
     *
     * \sa signalLocked()
     */
    void setSignalLocked(bool arg);

    /*!
     * \brief Specifies whether a hardware resource is currently tuned to retrieve
     * the content represented by this item.
     *
     * \param arg specifies whether a hardware resource is currently tuned to retrieve
     * the content represented by this item.
     *
     * \sa tuned()
     */
    void setTuned(bool arg);

    /*!
     * \brief Specifies whether the object can be used for recording purposes.
     *
     * \param arg specifies whether the object can be used for recording purposes.
     *
     * \sa recordable()
     */
    void setRecordable(bool arg);

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
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return VideoBroadcast; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.videoItem.videoBroadcast"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HVideoBroadcast* create() { return new HVideoBroadcast(); }
};

}
}
}

#endif /* HVIDEOBROADCAST_H_ */
