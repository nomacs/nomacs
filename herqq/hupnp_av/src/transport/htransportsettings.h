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

#ifndef HTRANSPORTSETTINGS_H_
#define HTRANSPORTSETTINGS_H_

#include <HUpnpAv/HUpnpAv>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HTransportSettingsPrivate;

/*!
 * \brief This is a class used to contain various transport settings of a virtual
 * AVTransport instance.
 *
 * \headerfile htransportsettings.h HTransportSettings
 *
 * \ingroup hupnp_av_avt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HTransportSettings
{

private:

    QSharedDataPointer<HTransportSettingsPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HTransportSettings();

    /*!
     * \brief Creates a new, empty instance.
     *
     * \param playMode specifies the play mode setting.
     *
     * \param recQualityMode specifies the record quality mode setting.
     *
     * \sa isEmpty()
     */
    HTransportSettings(
        const HPlayMode& playMode,
        const HRecordQualityMode& reqQualityMode);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HTransportSettings(const HTransportSettings&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HTransportSettings& operator=(const HTransportSettings&);

    /*!
     * \brief Destroys the instance.
     */
    ~HTransportSettings();

    /*!
     * \brief Indicates whether the object contains information.
     *
     * \return \e true if the object is empty.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the play mode setting.
     *
     * \return The play mode setting.
     */
    const HPlayMode& playMode() const;

    /*!
     * \brief Returns the record quality mode setting.
     *
     * \return The record quality mode setting.
     */
    HRecordQualityMode recordQualityMode() const;

    /*!
     * \brief Sets the play mode setting.
     *
     * \param arg specifies the play mode setting.
     */
    void setPlayMode(const HPlayMode& arg);

    /*!
     * \brief Sets the record quality mode setting.
     *
     * \param arg specifies the record quality mode setting.
     */
    void setRecordQualityMode(const HRecordQualityMode& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HTransportSettings
 */
H_UPNP_AV_EXPORT bool operator==(const HTransportSettings& obj1, const HTransportSettings& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HTransportSettings
 */
inline bool operator!=(const HTransportSettings& obj1, const HTransportSettings& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HTRANSPORTSETTINGS_H_ */
