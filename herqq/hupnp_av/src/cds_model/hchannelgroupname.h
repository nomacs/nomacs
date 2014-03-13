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

#ifndef HCHANNELGROUPNAME_H_
#define HCHANNELGROUPNAME_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>
#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to represent a <em>channel group name</em>.
 *
 * \headerfile hchannelgroupname.h HChannelGroupName
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HChannelGroupName
{
private:

    QString m_name;
    QString m_id;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HChannelGroupName();

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the user friendly name of the channel group.
     *
     * \param id specifies the ID of a channel group that is used to differentiate
     * it from other channel groups implemented in a ContentDirectory service.
     * The ID has to follow a format of
     * <c><ICANN registered domain>"_"<channel group id defined in the domain></c>.
     * For instance, "mydomain.com_myChannelGroupId".
     *
     * \sa isValid()
     */
    HChannelGroupName(const QString& name, const QString& id);

    /*!
     * \brief Destroys the instance.
     */
    ~HChannelGroupName();

    /*!
     * \brief Indicates whether the object is valid.
     *
     * \return \e true if the object is valid, i.e. name() and id() are both
     * defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the user friendly name of the channel group.
     *
     * \return The user friendly name of the channel group.
     */
    inline QString name() const { return m_name; }

    /*!
     * \brief Returns the ID of a channel group.
     *
     * \return The ID of a channel group that is used to differentiate
     * it from other channel groups implemented in a ContentDirectory service.
     */
    inline QString id() const { return m_id; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HChannelGroupName
 */
H_UPNP_AV_EXPORT bool operator==(const HChannelGroupName& obj1, const HChannelGroupName& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HChannelGroupName
 */
inline bool operator!=(const HChannelGroupName& obj1, const HChannelGroupName& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HChannelGroupName)

#endif /* HCHANNELGROUPNAME_H_ */
