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

#ifndef HCREATEOBJECTRESULT_H_
#define HCREATEOBJECTRESULT_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to contain the result of a create object action.
 *
 * \headerfile hcreateobjectresult.h HCreateObjectResult
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractContentDirectoryService::createObject()
 * \sa HContentDirectoryAdapter::createObject()
 */
class H_UPNP_AV_EXPORT HCreateObjectResult
{
private:

    QString m_objectId;
    QString m_result;

public:

     /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HCreateObjectResult();

    /*!
     * \brief Creates a new instance.
     *
     * \param objectId specifies the ID of the created CDS object.
     *
     * \param result specifies the CDS metadata of the created object. The string
     * contains a DIDL-Lite XML document.
     *
     * \sa isValid()
     */
    HCreateObjectResult(const QString& objectId, const QString& result);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. the objectId() and result()
     * are both defined.
     *
     * \note This does not check that result() represents a valid XML document.
     */
    bool isValid() const;

    /*!
     * \brief Returns the ID of the created CDS object.
     *
     * \return The ID of the created CDS object.
     */
    inline QString objectId() const { return m_objectId; }

    /*!
     * \brief Returns the CDS metadata of the created object.
     *
     * \return The CDS metadata of the created object. The string
     * contains a DIDL-Lite XML document.
     */
    inline QString result() const { return m_result; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HCreateObjectResult
 */
H_UPNP_AV_EXPORT bool operator==(
    const HCreateObjectResult& obj1, const HCreateObjectResult& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HCreateObjectResult
 */
inline bool operator!=(
    const HCreateObjectResult& obj1, const HCreateObjectResult& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HCREATEOBJECTRESULT_H_ */
