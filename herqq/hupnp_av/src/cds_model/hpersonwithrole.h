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

#ifndef HPERSONWITHROLE_H_
#define HPERSONWITHROLE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class specifies information of a person assigned to a role.
 *
 * \headerfile hpersonwithrole.h HPersonWithRole
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HPersonWithRole
{
private:

    QString m_name;
    QString m_role;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HPersonWithRole();

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the name of the person. If this is empty, the
     * object is constructed as invalid.
     *
     * \param role specifies the role of the person.
     *
     * \sa isValid()
     */
    HPersonWithRole(const QString& name, const QString& role=QString());

    /*!
     * \brief Destroys the instance.
     */
    ~HPersonWithRole();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the name() is defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of the person.
     *
     * \return The name of the person. In a valid object this is never empty.
     */
    inline QString name() const { return m_name; }

    /*!
     * \brief Returns the role of the person.
     *
     * \return The role of the person. This may be empty.
     */
    inline QString role() const { return m_role; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HPersonWithRole
 */
H_UPNP_AV_EXPORT bool operator==(const HPersonWithRole& obj1, const HPersonWithRole& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HPersonWithRole
 */
inline bool operator!=(const HPersonWithRole& obj1, const HPersonWithRole& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HPersonWithRole)

#endif /* HPERSONWITHROLE_H_ */
