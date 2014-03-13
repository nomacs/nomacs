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

#ifndef HCONNECTIONMANAGER_ID_H_
#define HCONNECTIONMANAGER_ID_H_

#include <HUpnpCore/HUdn>
#include <HUpnpCore/HServiceId>

class QString;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to provide information of a ConnectionManager.
 *
 * \headerfile hconnectionmanager_id.h HConnectionManagerId
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractConnectionManagerService::prepareForConnection(),
 * HAbstractConnectionManagerService::getCurrentConnectionInfo(),
 * HConnectionManagerAdapter::prepareForConnection(),
 * HConnectionManagerAdapter::getCurrentConnectionInfo()
 */
class H_UPNP_AV_EXPORT HConnectionManagerId
{
private:

    HUdn m_udn;
    HServiceId m_serviceId;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HConnectionManagerId();

    /*!
     * \brief Creates a new instance.
     *
     * \param arg specifies the ConnectionManager ID following the format:
     * <c>UDN/serviceId</c>, where the UDN is the Unique Device Name identifying the
     * UPnP device and serviceId is the ID identifying the UPnP service contained by
     * the UPnP device with the specified UDN.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the provided arguments. This parameter is optional and the default level is strict.
     *
     * \sa isValid()
     */
    HConnectionManagerId(
        const QString& arg, HValidityCheckLevel checkLevel = StrictChecks);

    /*!
     * \brief Creates a new instance.
     *
     * \param udn specifies the Unique Device Name identifying the
     * UPnP device.
     *
     * \param serviceId specifies the ID identifying the UPnP service contained by
     * the UPnP device with the specified UDN.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the provided arguments. This parameter is optional and the default level is strict.
     *
     * \sa isValid()
     */
    HConnectionManagerId(
        const HUdn& udn, const HServiceId& serviceId,
        HValidityCheckLevel checkLevel = StrictChecks);

    /*!
     * \brief Destroys the instance.
     */
    ~HConnectionManagerId();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \param level specifies the level of strictness used in validating
     * the object. This parameter is optional and the default level is strict.
     *
     * \return \e true in case the object is valid.
     */
    bool isValid(HValidityCheckLevel checkLevel = StrictChecks) const;

    /*!
     * \brief Returns the Unique Device Name component of this ID.
     *
     * \return The Unique Device Name component of this ID.
     */
    inline HUdn udn() const { return m_udn; }

    /*!
     * \brief Returns the serviceId component of this ID.
     *
     * \return The serviceId component of this ID.
     */
    inline HServiceId serviceId() const { return m_serviceId; }

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object. The format of the
     * returned string is: <c>UDN/serviceId</c>. The returned string is empty
     * if the object is not valid.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HConnectionManagerId
 */
H_UPNP_AV_EXPORT bool operator==(const HConnectionManagerId&, const HConnectionManagerId&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HConnectionManagerId
 */
inline bool operator!=(const HConnectionManagerId& obj1, const HConnectionManagerId& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HCONNECTIONMANAGER_ID_H_ */
