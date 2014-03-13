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

#ifndef HPREPAREFORCONNECTION_RESULT_H_
#define HPREPAREFORCONNECTION_RESULT_H_

#include <HUpnpAv/HUpnpAv>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to contain the result of a PrepareForConnection action.
 *
 * \headerfile hprepareforconnection_result.h HPrepareForConnectionResult.
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractConnectionManagerService::prepareForConnection()
 * \sa HConnectionManagerAdapter::prepareForConnection()
 */
class H_UPNP_AV_EXPORT HPrepareForConnectionResult
{
private:

    qint32 m_connectionId;
    qint32 m_avTransportId;
    qint32 m_rcsId;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HPrepareForConnectionResult();

    /*!
     * \brief Creates a new instance.
     *
     * \param connectionId specifies the connection ID.
     *
     * \param avTransportId specifies the virtual AVTransport instance ID.
     * This may be \c -1 if there is no virtual AV Transport associated with the
     * \a connectionId.
     *
     * \param rcsId specifies the virtual RenderingControlService instance ID.
     * This may be \c -1 if there is no virtual Rendering Control associated with the
     * \a connectionId.
     *
     * \sa isValid()
     */
    explicit HPrepareForConnectionResult(
        qint32 connectionId, qint32 avTransportId = -1, qint32 rcsId = -1);

    /*!
     * \brief Returns the connection ID.
     *
     * \return The connection ID.
     *
     * \sa setConnectionId()
     */
    inline qint32 connectionId() const
    {
        return m_connectionId;
    }

    /*!
     * \brief Returns the virtual AVTransport instance ID.
     *
     * \return The virtual AVTransport instance ID.
     *
     * \sa setAvTransportId()
     */
    inline qint32 avTransportId() const
    {
        return m_avTransportId;
    }

    /*!
     * \brief Returns the virtual RenderingControlService instance ID.
     *
     * \return The virtual RenderingControlService instance ID.
     *
     * \sa setRcsId()
     */
    inline qint32 rcsId() const
    {
        return m_rcsId;
    }

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e, the connectionId() is
     * appropriately defined.
     */
    bool isValid() const;

    /*!
     * \brief Sets the connection ID.
     *
     * \param arg specifies the connection ID.
     *
     * \sa connectionId()
     */
    void setConnectionId(qint32 arg);

    /*!
     * \brief Sets the virtual AVTransport instance ID.
     *
     * \param arg specifies the virtual AVTransport instance ID.
     *
     * \sa avTransportId()
     */
    void setAvTransportId(qint32 arg);

    /*!
     * \brief Sets the virtual RenderingControlService instance ID.
     *
     * \param arg specifies the virtual RenderingControlService instance ID.
     *
     * \sa rcsId()
     */
    void setRcsId(qint32 arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HPrepareForConnectionResult
 */
H_UPNP_AV_EXPORT bool operator==(const HPrepareForConnectionResult&, const HPrepareForConnectionResult&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HPrepareForConnectionResult
 */
inline bool operator!=(const HPrepareForConnectionResult& obj1, const HPrepareForConnectionResult& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HPREPAREFORCONNECTION_RESULT_H_ */
