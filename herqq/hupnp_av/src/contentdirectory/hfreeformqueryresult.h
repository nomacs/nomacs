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

#ifndef HFREEFORMQUERYRESULT_H_
#define HFREEFORMQUERYRESULT_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to contain the result of a free form query action.
 *
 * \headerfile hfreeformqueryresult.h HFreeFormQueryResult
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractContentDirectoryService::freeFormQuery()
 * \sa HContentDirectoryAdapter::freeFormQuery()
 */
class H_UPNP_AV_EXPORT HFreeFormQueryResult
{
private:

    QString m_queryResult;
    quint32 m_updateId;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HFreeFormQueryResult();

    /*!
     * \brief Creates a new instance.
     *
     * \param queryResult specifies the result of the free form query.
     *
     * \param updateId specifies the value of the state variable
     * \c SystemUpdateID at the time the result was generated.
     *
     * \sa isValid()
     */
    HFreeFormQueryResult(const QString& queryResult, quint32 updateId);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. the queryResult() is not empty.
     */
    bool isValid() const;

    /*!
     * \brief Returns the result of the free form query.
     *
     * \return The result of the free form query.
     */
    inline QString queryResult() const { return m_queryResult; }

    /*!
     * \brief Returns the value of the state variable
     * \c SystemUpdateID at the time the result was generated.
     *
     * \return The value of the state variable
     * \c SystemUpdateID at the time the result was generated.
     */
    inline quint32 updateId() const { return m_updateId; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HFreeFormQueryResult
 */
H_UPNP_AV_EXPORT bool operator==(
    const HFreeFormQueryResult& obj1, const HFreeFormQueryResult& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HFreeFormQueryResult
 */
inline bool operator!=(
    const HFreeFormQueryResult& obj1, const HFreeFormQueryResult& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HFREEFORMQUERYRESULT_H_ */
