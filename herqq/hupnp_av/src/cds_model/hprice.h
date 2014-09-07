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

#ifndef HPRICE_H_
#define HPRICE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class represents the price of a content item as declared in the
 * ContentDirectory:3 specification.
 *
 * \headerfile hprice.h HPrice
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HPrice
{
private:

    float m_value;
    QString m_currency;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HPrice();

    /*!
     * \brief Creates a new instance.
     *
     * \param value specifies the actual price value.
     *
     * \param currency specifies the desired currency as defined in the
     * ISO 4217 Type Currency Code List.
     *
     * \sa isValid()
     */
    HPrice(float value, const QString& currency);

    /*!
     * \brief Destroys the instance.
     */
    ~HPrice();

    /*!
     * \brief Indicates if the instance is valid.
     *
     * \return \e true if the instance is valid, i.e. value() is not negative
     * and the currency is one of the values defined by the ISO 4217.
     */
    bool isValid() const;

    /*!
     * \brief Returns the unit of currency used for the value().
     *
     * \return The unit of currency used for the value().
     */
    inline QString currency() const { return m_currency; }

    /*!
     * \brief Returns the actual price value.
     *
     * \return The actual price value.
     */
    inline float value() const { return m_value; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HPrice
 */
H_UPNP_AV_EXPORT bool operator==(const HPrice& obj1, const HPrice& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HPrice
 */
inline bool operator!=(const HPrice& obj1, const HPrice& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HPrice)

#endif /* HPRICE_H_ */
