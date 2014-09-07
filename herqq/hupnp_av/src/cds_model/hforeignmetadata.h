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

#ifndef HFOREIGNMETADATA_H_
#define HFOREIGNMETADATA_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \headerfile hforeignmetadata.h HForeignMetadata
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HForeignMetadata
{
public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HForeignMetadata();

    /*!
     * \brief Destroys the instance.
     */
    ~HForeignMetadata();

    /*!
     * Indicates if the object is valid.
     *
     * \return \e true if the object is valid.
     */
    bool isValid() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HForeignMetadata
 */
H_UPNP_AV_EXPORT bool operator==(const HForeignMetadata&, const HForeignMetadata&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HForeignMetadata
 */
inline bool operator!=(const HForeignMetadata& obj1, const HForeignMetadata& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HForeignMetadata)

#endif /* HFOREIGNMETADATA_H_ */
