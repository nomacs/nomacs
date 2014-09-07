/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HSERVICEINFO_H_
#define HSERVICEINFO_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QSharedDataPointer>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

class HServiceInfoPrivate;

/*!
 * \brief This class is used to contain information of a UPnP service
 * found in a UPnP device description document.
 *
 * A device description defines a UPnP device and among other things,
 * the definition includes the declarations of the services the device contains.
 *
 * \headerfile hserviceinfo.h HServiceInfo
 *
 * \ingroup hupnp_common
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HDeviceInfo, HActionInfo and HStateVariableInfo.
 */
class H_UPNP_CORE_EXPORT HServiceInfo
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HServiceInfo& obj1, const HServiceInfo& obj2);

private:

    QSharedDataPointer<HServiceInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HServiceInfo();

    /*!
     * Constructs a new instance from the specified parameters that the UDA
     * specification mandates for a UPnP service.
     *
     * The parameters the constructor expects are arguments defined in the
     * device description document and they are all mandatory for a valid
     * UPnP service.
     *
     * \param serviceId specifies the identifier of the service.
     *
     * \param serviceType specifies the type of the service.
     *
     * \param controlUrl specifies the URL for control.
     *
     * \param eventSubUrl specifies the URL for eventing.
     *
     * \param scpdUrl specifies the URL for service description.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param checkLevel specifies the level of strictness used in validating
     * the specified arguments. This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \remarks in case any of the provided arguments does not meet the
     * specified requirements, the created object is \e invalid.
     *
     * \sa isValid()
     */
    HServiceInfo(
        const HServiceId& serviceId,
        const HResourceType& serviceType,
        const QUrl& controlUrl,
        const QUrl& eventSubUrl,
        const QUrl& scpdUrl,
        HInclusionRequirement incReq = InclusionMandatory,
        HValidityCheckLevel checkLevel = StrictChecks,
        QString* err = 0);

    /*!
     * \brief Destroys the instance.
     */
    ~HServiceInfo();

    /*!
     * \brief Copy constructor.
     *
     * Copies the contents of the \c other to this.
     *
     * \param other specifies the object to be copied.
     */
    HServiceInfo(const HServiceInfo& other);

    /*!
     * \brief Assignment operator.
     *
     * Assigns the contents of the other to this.
     *
     * \param other specifies the object to be copied.
     */
    HServiceInfo& operator=(const HServiceInfo& other);

    /*!
     * \brief Indicates if the object is valid.
     *
     * A valid object contains the mandatory data of a UPnP service.
     *
     * \param level specifies the level of strictness used in validating
     * the object. This parameter is optional and the default level is strict.
     *
     * \return \e true in case the object is valid.
     */
    bool isValid(HValidityCheckLevel level) const;

    /*!
     * \brief Returns the service identifier found in the device description file.
     *
     * \return The service identifier found in the device description file.
     */
    const HServiceId& serviceId() const;

    /*!
     * \brief Returns the type of the service found in the device description file.
     *
     * \return The type of the service found in the device description file.
     */
    const HResourceType& serviceType() const;

    /*!
     * \brief Returns the URL for service description.
     *
     * This is the URL where the service description can be retrieved.
     * This is defined in the device description.
     *
     * \return The URL for service description.
     */
    QUrl scpdUrl() const;

    /*!
     * \brief Returns the URL for control.
     *
     * This is the URL to which the action invocations must be sent.
     * This is defined in the device description.
     *
     * \return The URL for control.
     */
    QUrl controlUrl() const;

    /*!
     * \brief Returns the URL for eventing.
     *
     * This is the URL to which subscriptions and un-subscriptions are sent.
     * This is defined in the device description.
     *
     * \return The URL for eventing.
     */
    QUrl eventSubUrl() const;

    /*!
     * \brief Indicates whether the service is required or optional.
     *
     * \return value indicating whether the service is required or optional.
     */
    HInclusionRequirement inclusionRequirement() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HServiceInfo
 */
H_UPNP_CORE_EXPORT bool operator==(const HServiceInfo&, const HServiceInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HServiceInfo
 */
inline bool operator!=(const HServiceInfo& obj1, const HServiceInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}

#endif /* HSERVICEINFO_H_ */
