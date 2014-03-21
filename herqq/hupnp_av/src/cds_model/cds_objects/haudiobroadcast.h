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

#ifndef HAUDIOBROADCAST_H_
#define HAUDIOBROADCAST_H_

#include <HUpnpAv/HAudioItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAudioBroadcastPrivate;

/*!
 * \brief This class represents a continuous stream from an audio broadcast.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.audioItem.audioBroadcast .
 *
 * \headerfile haudiobroadcast.h HAudioBroadcast
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAudioBroadcast :
    public HAudioItem
{
Q_OBJECT
H_DISABLE_COPY(HAudioBroadcast)
H_DECLARE_PRIVATE(HAudioBroadcast)

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
    HAudioBroadcast(const QString& clazz = sClass(), CdsType cdsType = sType());
    HAudioBroadcast(HAudioBroadcastPrivate&);

    // Documented in HClonable
    virtual HAudioBroadcast* newInstance() const;

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
    HAudioBroadcast(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAudioBroadcast();

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
     * upnp:recordable, ContentDirectory:3, Appendix, B.7.16.
     *
     * \return \e true if the content represented by this object can potentially
     * be used for recording purposes.
     *
     * \sa setRecordable()
     */
    bool recordable() const;

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
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return AudioBroadcast; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.audioItem.audioBroadcast"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HAudioBroadcast* create() { return new HAudioBroadcast(); }
};

}
}
}

#endif /* HAUDIOBROADCAST_H_ */
