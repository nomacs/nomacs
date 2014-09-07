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

#ifndef HAVT_LASTCHANGE_INFO_H_
#define HAVT_LASTCHANGE_INFO_H_

#include <HUpnpAv/HUpnpAv>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAvtLastChangeInfoPrivate;

/*!
 * \brief This class is used to contain information of a LastChange event sent
 * by an AVTransport service.
 *
 * \headerfile havt_lastchange_info.h HAvtLastChangeInfo
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HConnection
 */
class H_UPNP_AV_EXPORT HAvtLastChangeInfo
{

private:

    HAvtLastChangeInfoPrivate* h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HAvtLastChangeInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param propertyName specifies the name of the property.
     *
     * \param value specifies the value of the property.
     *
     * \sa isValid()
     */
    HAvtLastChangeInfo(const QString& propertyName, const QVariant& value);

    /*!
     * \brief Destroys the instance.
     */
    ~HAvtLastChangeInfo();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HAvtLastChangeInfo(const HAvtLastChangeInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HAvtLastChangeInfo& operator=(const HAvtLastChangeInfo&);

    /*!
     * \brief Specifies the name of the property.
     *
     * \param name specifies the name of the property.
     *
     * \sa propertyName()
     */
    void setPropertyName(const QString& name);

    /*!
     * \brief Specifies the value of the property.
     *
     * \param value specifies the value of the property.
     *
     * \sa value()
     */
    void setValue(const QVariant& value);

    /*!
     * \brief Indicates the validity of the object.
     *
     * \return \e true in case the object is valid, i.e. the propertyName() is
     * defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of the property.
     *
     * \return The name of the property.
     *
     * \sa setPropertyName()
     */
    QString propertyName() const;

    /*!
     * \brief Returns the value of the property.
     *
     * \return The value of the property.
     *
     * \sa setValue()
     */
    QVariant value() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HAvtLastChangeInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HAvtLastChangeInfo&, const HAvtLastChangeInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HAvtLastChangeInfo
 */
inline bool operator!=(const HAvtLastChangeInfo& obj1, const HAvtLastChangeInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* s */
