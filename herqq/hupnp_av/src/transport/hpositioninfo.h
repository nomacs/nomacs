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

#ifndef HPOSITIONINFO_H_
#define HPOSITIONINFO_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QSharedDataPointer>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HPositionInfoPrivate;

/*!
 * \brief This is a class used to contain position information of a track and media of a
 * virtual AV Transport instance.
 *
 * \headerfile hpositioninfo.h HPositionInfo
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractTransportService::getPositionInfo(),
 * HAvTransportAdapter::getPositionInfo()
 */
class H_UPNP_AV_EXPORT HPositionInfo
{
private:

    QSharedDataPointer<HPositionInfoPrivate> h_ptr;

public:

    /*!
     * Creates a new invalid instance.
     *
     * \sa isValid()
     */
    HPositionInfo();

    /*!
     * Creates a new instance instance.
     *
     * \param trackUri specifies a reference to the track as a URI.
     *
     * \param trackMetadata specifies a DIDL-Lite XML fragment containing
     * metadata of the track. The format is specified in the
     * ContentDirectory specification.
     *
     * \sa isValid()
     */
    HPositionInfo(const QUrl& trackUri, const QString& trackMetadata = QString());

    /*!
     * \brief Creates a new instance.
     *
     * \param track specifies the track number.
     *
     * \param trackDuration specifies the duration of the track.
     *
     * \param trackMetadata specifies a DIDL-Lite XML fragment containing
     * metadata of the track. The format is specified in the
     * ContentDirectory specification.
     *
     * \param trackUri specifies a reference to the track as a URI.
     *
     * \param relTimePos specifies the relative position in terms of time.
     * For track-aware media, this is measured from the beginning of the track
     * and the value is always positive. For track-unaware media, this is
     * measured from a zero reference point on the media and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.23.
     *
     * \param absTimePos specifies the absolute position in terms of time,
     * always measured from the beginning of the media. The value is always positive.
     *
     * \param relCounterPos specifies relative position in terms of a
     * dimensionless counter. For track-aware media, this is measurd from the
     * beginning of the track and the value is always positive.
     * For track-unaware media, this is measured from a zero reference point
     * on the media and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.25.
     *
     * \param absCounterPos specifies the absolute position in terms of a
     * dimensionless counter, always measured from the beginning of the media.
     * The value is always positive.
     */
    HPositionInfo(
        quint32 track,
        const HDuration& trackDuration,
        const QString& trackMetadata,
        const QUrl& trackUri,
        const HDuration& relTimePos,
        const HDuration& absTimePos,
        qint32 relCounterPos,
        quint32 absCounterPos);

    /*!
     * \brief Destroys the instance.
     */
    ~HPositionInfo();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HPositionInfo(const HPositionInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HPositionInfo& operator=(const HPositionInfo&);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. at least trackUri() is
     * appropriately defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the track number.
     *
     * \return The track number.
     */
    quint32 track() const;

    /*!
     * \brief Returns the duration of the track.
     *
     * \return The duration of the track.
     */
    HDuration trackDuration() const;

    /*!
     * \brief Returns metadata of the track.
     *
     * \return a DIDL-Lite XML fragment containing
     * metadata of the track. The format is specified in the
     * ContentDirectory specification.
     */
    QString trackMetadata() const;

    /*!
     * \brief Returns a reference to the track as a URI.
     *
     * \return a reference to the track as a URI.
     */
    QUrl trackUri() const;

    /*!
     * \brief Returns the relative position in terms of time.
     *
     * \return The relative position in terms of time.
     * For track-aware media, this is measured from the beginning of the track
     * and the value is always positive. For track-unaware media, this is
     * measured from a zero reference point on the media and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.23.
     */
    HDuration relativeTimePosition() const;

    /*!
     * \brief Returns the absolute position in terms of time.
     *
     * \return The absolute position in terms of time,
     * always measured from the beginning of the media. The value is always positive.
     */
    HDuration absoluteTimePosition() const;

    /*!
     * \brief Returns the relative position in terms of dimensionless counter.
     *
     * \return relative position in terms of a
     * dimensionless counter. For track-aware media, this is measurd from the
     * beginning of the track and the value is always positive.
     * For track-unaware media, this is measured from a zero reference point
     * on the media and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.25.
     */
    qint32 relativeCounterPosition() const;

    /*!
     * \brief Returns the absolute position in terms of dimensionless counter.
     *
     * \return The absolute position in terms of a
     * dimensionless counter, always measured from the beginning of the media.
     * The value is always positive.
     */
    qint32 absoluteCounterPosition() const;

    /*!
     * \brief Sets the track number.
     *
     * \param arg specifies the track number.
     */
    void setTrack(quint32 arg);

    /*!
     * \brief Sets the duration of the track.
     *
     * \param arg specifies the duration of the track.
     */
    void setTrackDuration(const HDuration& arg);

    /*!
     * \brief Sets the metadata of the track.
     *
     * \param arg specifies a DIDL-Lite XML fragment containing
     * metadata of the track. The format is specified in the
     * ContentDirectory specification.
     */
    void setTrackMetadata(const QString& arg);

    /*!
     * \brief Sets a reference to the track as a URI.
     *
     * \param arg specifies a reference to the track as a URI.
     */
    void setTrackUri(const QUrl& arg);

    /*!
     * \brief Sets the relative position in terms of time.
     *
     * \param arg specifies the relative position in terms of time.
     * For track-aware media, this is measured from the beginning of the track
     * and the value is always positive. For track-unaware media, this is
     * measured from a zero reference point on the media and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.23.
     */
    void setRelativeTimePosition(const HDuration& arg);

    /*!
     * \brief Sets the absolute position in terms of time.
     *
     * \param arg specifies the absolute position in terms of time,
     * always measured from the beginning of the media. The value is always positive.
     */
    void setAbsoluteTimePosition(const HDuration& arg);

    /*!
     * \brief Sets the relative position in terms of dimensionless counter.
     *
     * \param arg specifies relative position in terms of a
     * dimensionless counter. For track-aware media, this is measurd from the
     * beginning of the track and the value is always positive.
     * For track-unaware media, this is measured from a zero reference point
     * on the media and this can be negative.
     * For more information, see AVTransport:2 specification, section 2.2.25.
     */
    void setRelativeCounterPosition(qint32 arg);

    /*!
     * \brief Sets the absolute position in terms of dimensionless counter.
     *
     * \param arg specifies the absolute position in terms of a
     * dimensionless counter, always measured from the beginning of the media.
     * The value is always positive.
     */
    void setAbsoluteCounterPosition(quint32 arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HPositionInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HPositionInfo& obj1, const HPositionInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HPositionInfo
 */
inline bool operator!=(const HPositionInfo& obj1, const HPositionInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HPOSITIONINFO_H_ */
