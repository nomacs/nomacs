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

#ifndef HPROTOCOLINFORESULT_H_
#define HPROTOCOLINFORESULT_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HProtocolInfo>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to contain the result of a GetProtocolInfo action.
 *
 * \headerfile hprepareforconnection_result.h HPrepareForConnectionResult.
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractConnectionManagerService::getProtocolInfo()
 * \sa HConnectionManagerAdapter::getProtocolInfo()
 */
class H_UPNP_AV_EXPORT HProtocolInfoResult
{
private:

    HProtocolInfos m_source;
    HProtocolInfos m_sink;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HProtocolInfoResult();

    /*!
     * \brief Creates a new instance.
     *
     * \param source specifies protocol-related information for \e sourcing data.
     *
     * \param sink specifies protocol-related information for \e sinking data.
     *
     * \sa isEmpty()
     */
    HProtocolInfoResult(const HProtocolInfos& source, const HProtocolInfos& sink);

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true if the object is empty, i.e. both source() and sink()
     * are undefined.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns protocol-related information for \e sourcing data.
     *
     * \return protocol-related information for \e sourcing data.
     *
     * \sa setSource()
     */
    inline const HProtocolInfos& source() const
    {
        return m_source;
    }

    /*!
     * \brief Returns protocol-related information for \e sinking data.
     *
     * \return protocol-related information for \e sinking data.
     *
     * \sa setSink()
     */
    inline const HProtocolInfos& sink() const
    {
        return m_sink;
    }

    /*!
     * \brief Specifies protocol-related information for \e sourcing data.
     *
     * \param arg specifies protocol-related information for \e sourcing data.
     *
     * \return \e true if the source was set.
     *
     * \remarks The \e source will not be set, if any of the specified HProtocolInfo
     * objects is invalid. However, empty list is accepted.
     *
     * \sa source()
     */
    bool setSource(const HProtocolInfos& arg);

    /*!
     * \brief Specifies protocol-related information for \e sinking data.
     *
     * \param arg specifies protocol-related information for \e sourcing data.
     *
     * \return \e true if the sink was set.
     *
     * \remarks The \e sink will not be set, if any of the specified HProtocolInfo
     * objects is invalid. However, empty list is accepted.
     *
     * \sa sink()
     */
    bool setSink(const HProtocolInfos& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HProtocolInfoResult
 */
H_UPNP_AV_EXPORT bool operator==(const HProtocolInfoResult&, const HProtocolInfoResult&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HProtocolInfoResult
 */
inline bool operator!=(const HProtocolInfoResult& obj1, const HProtocolInfoResult& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HPROTOCOLINFORESULT_H_ */
