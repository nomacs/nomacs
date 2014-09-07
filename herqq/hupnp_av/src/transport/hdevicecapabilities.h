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

#ifndef HDEVICECAPABILITIES_H_
#define HDEVICECAPABILITIES_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HDeviceCapabilitiesPrivate;

/*!
 * \brief This class contains information about the capabilities of a MediaRenderer
 * device.
 *
 * \headerfile hdevicecapabilities.h HDeviceCapabilities
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HDeviceCapabilities
{
private:

    QSharedDataPointer<HDeviceCapabilitiesPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HDeviceCapabilities();

    /*!
     * \brief Creates a new instance.
     *
     * \param playMedia specifies the supported medias for playback.
     *
     * \param recMedia specifies supported medias for recording.
     *
     * \param reqQualityModes specifies the possible recording quality modes.
     *
     * \sa isValid()
     */
    HDeviceCapabilities(
        const QSet<QString>& playMedia,
        const QSet<QString>& recMedia,
        const QSet<QString>& reqQualityModes);

    /*!
     * \brief Creates a new instance.
     *
     * \param playMedia specifies the supported medias for playback.
     *
     * \param recMedia specifies supported medias for recording.
     *
     * \param reqQualityModes specifies the possible recording quality modes.
     *
     * \sa isValid()
     */
    HDeviceCapabilities(
        const QSet<HStorageMedium>& playMedia,
        const QSet<HStorageMedium>& recMedia,
        const QSet<HRecordQualityMode>& reqQualityModes);

    /*!
     * \brief Destroys the instance.
     */
    ~HDeviceCapabilities();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HDeviceCapabilities(const HDeviceCapabilities&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HDeviceCapabilities& operator=(const HDeviceCapabilities&);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e.
     * playMedia() contains at least one entry, or recordMedia() contains
     * at least one entry and recordQualityModes() contains at least one entry.
     */
    bool isValid() const;

    /*!
     * \brief Returns the supported medias for playback.
     *
     * \return The supported medias for playback.
     *
     * \sa setPlayMedia()
     */
    QSet<HStorageMedium> playMedia() const;

    /*!
     * \brief Returns the supported medias for recording.
     *
     * \return The supported medias for recording.
     *
     * \sa setRecordMedia()
     */
    QSet<HStorageMedium> recordMedia() const;

    /*!
     * \brief Returns the possible recording quality modes.
     *
     * \return The possible recording quality modes.
     *
     * \sa setRecordQualityModes()
     */
    QSet<HRecordQualityMode> recordQualityModes() const;

    /*!
     * \brief Specifies the supported medias for playback.
     *
     * \param arg specifies the supported medias for playback.
     *
     * \sa playMedia()
     */
    void setPlayMedia(const QSet<HStorageMedium>& arg);

    /*!
     * \brief Specifies the supported medias for recording.
     *
     * \param arg specifies the supported medias for recording.
     *
     * \sa recordMedia()
     */
    void setRecordMedia(const QSet<HStorageMedium>& arg);

    /*!
     * \brief Specifies the possible recording quality modes.
     *
     * \param arg specifies the possible recording quality modes.
     *
     * \sa recordQualityModes()
     */
    void setRecordQualityModes(const QSet<HRecordQualityMode>& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HDeviceCapabilities
 */
H_UPNP_AV_EXPORT bool operator==(const HDeviceCapabilities&, const HDeviceCapabilities&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HDeviceCapabilities
 */
inline bool operator!=(const HDeviceCapabilities& obj1, const HDeviceCapabilities& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HDEVICECAPABILITIES_H_ */
