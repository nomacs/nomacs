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

#ifndef HSEARCHRESULT_H_
#define HSEARCHRESULT_H_

#include <HUpnpAv/HUpnpAv>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HSearchResultPrivate;

/*!
 * \brief This class is used to contain the results of a content directory \e browse
 * and \e search actions.
 *
 * \headerfile hsearchresult.h HSearchResult
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HContentDirectoryService::browse(), HContentDirectoryService::search(),
 * HContentDirectoryAdapter::browse(), HContentDirectoryAdapter::search()
 */
class H_UPNP_AV_EXPORT HSearchResult
{
private: // attributes

    QSharedDataPointer<HSearchResultPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HSearchResult();

    /*!
     * \brief Creates a new instance.
     *
     * \param result specifies the result of a \e browse or \e search action.
     * This is a DIDL-Lite XML Document.
     *
     * \param numberReturned specifies the number of objects returned in the
     * \a result argument.
     *
     * \param totalMatches specifies the total number of CDS objects \b in the
     * CDS object browsed.
     *
     * \param updateId specifies the value of the state variable
     * \c SystemUpdateID at the time the result was generated.
     *
     * \sa isEmpty()
     */
    HSearchResult(
        const QString& result, quint32 numberReturned, quint32 totalMatches,
        quint32 updateId);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HSearchResult(const HSearchResult&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     *
     * \return a reference to this.
     */
    HSearchResult& operator=(const HSearchResult&);

    /*!
     * \brief Destroys the instance.
     */
    ~HSearchResult();

    /*!
     * \brief Returns the result of the \e browse or \e search action.
     *
     * \return The result of the \e browse or \e search action. This is a
     * DIDL-Lite XML Document.
     */
    QString result() const;

    /*!
     * \brief Returns the number of objects contained in the result().
     *
     * \return The number of objects contained in the result().
     */
    quint32 numberReturned() const;

    /*!
     * \brief Returns the total number of CDS objects \b in the
     * CDS object browsed.
     *
     * \return The total number of CDS objects \b in the
     * CDS object browsed.
     */
    quint32 totalMatches() const;

    /*!
     * \brief Returns the value of the state variable
     * \c SystemUpdateID at the time the result was generated.
     *
     * \return The value of the state variable
     * \c SystemUpdateID at the time the result was generated..
     */
    quint32 updateId() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HSearchResult
 */
H_UPNP_AV_EXPORT bool operator==(
    const HSearchResult& obj1, const HSearchResult& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HSearchResult
 */
inline bool operator!=(
    const HSearchResult& obj1, const HSearchResult& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HSEARCHRESULT_H_ */
