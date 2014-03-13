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

#ifndef HDEVICEUDN_H_
#define HDEVICEUDN_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HUpnp>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HDeviceUdnPrivate;

/*!
 * \brief This class is used to identify a UPnP service within a UPnP device.
 *
 * \headerfile hdeviceudn.h HDeviceUdn
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HDeviceUdn
{
private:

    QSharedDataPointer<HDeviceUdnPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HDeviceUdn();

    /*!
     * \brief Creates a new instance.
     *
     * \param deviceUdn specifies the Unique Device Name of the UPnP device.
     *
     * \param serviceType specifies the service type of the device identified
     * by \a deviceUdn.
     *
     * \param serviceId specifies the service ID of the device identified
     * by \a deviceUdn.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified arguments. This parameter is optional.
     *
     * \sa isValid()
     */
    HDeviceUdn(
        const HUdn& deviceUdn, const HResourceType& serviceType,
        const HServiceId& serviceId,
        HValidityCheckLevel checkLevel = StrictChecks);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HDeviceUdn(const HDeviceUdn&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HDeviceUdn& operator=(const HDeviceUdn&);

    /*!
     * \brief Destroys the instance.
     */
    ~HDeviceUdn();

    /*!
     * \brief Indicates whether or not the object contains valid information identifying
     * a UPnP device and a service within it.
     *
     * \param level indicates whether the check should be strictly according
     * to the UDA specification. If set to false some checks are omitted that
     * are known to be poorly implemented in some UPnP software.
     *
     * \return \e true in case the object contains valid information identifying
     * a UPnP device and a service within it.
     */
    bool isValid(HValidityCheckLevel checkLevel) const;

    /*!
     * \brief Returns the Unique Device Name.
     *
     * \return The Unique Device Name.
     */
    const HUdn& udn() const;

    /*!
     * \brief Returns the service type.
     *
     * \return The service type.
     */
    const HResourceType& serviceType() const;

    /*!
     * \brief Returns the service ID.
     *
     * \return The service ID.
     */
    const HServiceId& serviceId() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HDeviceUdn
 */
H_UPNP_AV_EXPORT bool operator==(const HDeviceUdn& obj1, const HDeviceUdn& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HDeviceUdn
 */
inline bool operator!=(const HDeviceUdn& obj1, const HDeviceUdn& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HDeviceUdn)

#endif /* HDEVICEUDN_H_ */
