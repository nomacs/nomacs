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

#ifndef HVOLUMEDBRANGE_RESULT_H_
#define HVOLUMEDBRANGE_RESULT_H_

#include <HUpnpAv/HUpnpAv>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to contain the result of a create object action.
 *
 * \headerfile hvolumedbrange_result.h HVolumeDbRangeResult
 *
 * \ingroup hupnp_av_rcs
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HVolumeDbRangeResult
{
private:

    qint16 m_minValue, m_maxValue;

public:

     /*!
     * \brief Creates a new instance.
     *
     * Both minValue() and maxValue() are set to 0.
     */
    HVolumeDbRangeResult();

    /*!
     * \brief Creates a new instance.
     *
     * \param min specifies the minimum accepted value.
     *
     * \param max specifies the maximum accepted value.
     *
     * \remarks Both of the values should be specified in units of 1/256 decibels.
     */
    HVolumeDbRangeResult(qint16 min, qint16 max);

    /*!
     * \brief Returns the minimum acceptable value.
     *
     * \return The minimum acceptable value. The value is specified in units
     * of 1/256 decibels.
     */
    inline qint16 minValue() const { return m_minValue; }

    /*!
     * \brief Returns the maximum acceptable value.
     *
     * \return The maximum acceptable value. The value is specified in units
     * of 1/256 decibels.
     */
    inline qint16 maxValue() const { return m_maxValue; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HVolumeDbRangeResult
 */
H_UPNP_AV_EXPORT bool operator==(
    const HVolumeDbRangeResult& obj1, const HVolumeDbRangeResult& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HVolumeDbRangeResult
 */
inline bool operator!=(
    const HVolumeDbRangeResult& obj1, const HVolumeDbRangeResult& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HVOLUMEDBRANGE_RESULT_H_ */
