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

#ifndef HSERVICES_SETUPDATA_H_
#define HSERVICES_SETUPDATA_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HServiceId>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

class HServiceSetupPrivate;

/*!
 * \brief This class is used to specify information that can be used to validate
 * a UPnP service.
 *
 * \headerfile hservices_setupdata.h HServiceSetup
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HServicesSetupData, HClientService, HServerService
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HServiceSetup
{
private:

    QSharedDataPointer<HServiceSetupPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HServiceSetup();

    /*!
     * \brief Creates a new instance.
     *
     * \param id specifies the service ID.
     *
     * \param serviceType specifies the service type.
     *
     * \param incReq specifies the \e inclusion \e requirement of the
     * service.
     *
     * \sa isValid()
     *
     * \remarks the version() is set to 1.
     */
    HServiceSetup(
        const HServiceId& id,
        const HResourceType& serviceType,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * \brief Creates a new instance.
     *
     * \param id specifies the service ID.
     *
     * \param serviceType specifies the service type.
     *
     * \param version specifies the version of the UPnP device, which first
     * specified the service.
     *
     * \param incReq specifies the \e inclusion \e requirement of the
     * service.
     *
     * \sa isValid()
     */
    HServiceSetup(
        const HServiceId& id,
        const HResourceType& serviceType,
        int version,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \a other to this.
     */
    HServiceSetup& operator=(const HServiceSetup&);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \a other.
     */
    HServiceSetup(const HServiceSetup&);

    /*!
     * \brief Destroys the instance.
     */
    ~HServiceSetup();

    /*!
     * \brief Returns the <em>inclusion requirement</em>.
     *
     * \return The <em>inclusion requirement</em>.
     *
     * \sa setInclusionRequirement()
     */
    HInclusionRequirement inclusionRequirement() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \param checkLevel specifies whether the validity of the object should be
     * checked strictly according to the UDA specification.
     *
     * \return \e true in case the object is valid, that is, the service ID,
     * service type, version and inclusion requirement are all properly defined
     * in respect to the specified \c checkLevel.
     */
    bool isValid(HValidityCheckLevel checkLevel) const;

    /*!
     * \brief Returns the service ID.
     *
     * \return The service ID.
     *
     * \sa setServiceId()
     */
    const HServiceId& serviceId() const;

    /*!
     * \brief Returns the service type.
     *
     * \return The service type.
     *
     * \sa setServiceType()
     */
    const HResourceType& serviceType() const;

    /*!
     * \brief Returns the version of the UPnP device, which first specified the service.
     *
     * \return The version of the UPnP device, which first specified the service.
     *
     * \sa setVersion()
     */
    int version() const;

    /*!
     * \brief Sets the the <em>inclusion requirement</em>.
     *
     * \param arg specifies the <em>inclusion requirement</em>.
     *
     * \sa inclusionRequirement()
     */
    void setInclusionRequirement(HInclusionRequirement arg);

    /*!
     * \brief Sets the service ID.
     *
     * \param arg specifies the service ID.
     *
     * \sa serviceId()
     */
    void setServiceId(const HServiceId& arg);

    /*!
     * \brief Sets the service type.
     *
     * \param arg specifies the service type.
     *
     * \sa serviceType()
     */
    void setServiceType(const HResourceType& arg);

    /*!
     * \brief Sets the version of the UPnP device, which first specified the service.
     *
     * \param version defines the version of the UPnP device,
     * which first specifies the service.
     *
     * \sa version()
     */
    void setVersion(int version);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the provided objects are equal, false otherwise.
 *
 * \relates HServiceSetup
 */
H_UPNP_CORE_EXPORT bool operator==(const HServiceSetup&, const HServiceSetup&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the provided objects are not equal, false otherwise.
 *
 * \relates HServiceSetup
 */
inline bool operator!=(const HServiceSetup& obj1, const HServiceSetup& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief This class is used to specify information that can be used to validate
 * UPnP services.
 *
 * \headerfile hservices_setupdata.h HServicesSetupData
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HServicesSetupData
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HServicesSetupData&, const HServicesSetupData&);

private:

    QHash<HServiceId, HServiceSetup> m_serviceSetupInfos;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HServicesSetupData();

    /*!
     * \brief Destroys the instance.
     */
    ~HServicesSetupData();

    /*!
     * \brief Retrieves a service setup object.
     *
     * \param id specifies the service ID of the item.
     *
     * \return The item with the specified service ID. Note that the returned item
     * is invalid, i.e. HServiceSetup::isValid() returns false in case no item
     * with the specified service ID was found.
     *
     * \sa contains()
     */
    HServiceSetup get(const HServiceId& id) const;

    /*!
     * \brief Indicates if the instance contains a service setup item that has the
     * specified service ID.
     *
     * \param id specifies the service ID of the item.
     *
     * \return \e true if the instance contains an item with the specified
     * service ID.
     *
     * \sa get()
     */
    bool contains(const HServiceId& id) const;

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true in case the instance has no items.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the number of contained items.
     *
     * \return The number of contained items.
     */
    int size() const;

    /*!
     * \brief Returns the service IDs of the contained items.
     *
     * \return The service IDs of the contained items.
     */
    QSet<HServiceId> serviceIds() const;

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \param overWrite specifies whether to replace an already existing item
     * with the same service ID. The default is \c false.
     *
     * \return \e true in case the item was added. The \a newItem will not be added
     * if the instance already contains an item that has the same
     * HServiceSetup::serviceId() as the \a newItem and the \a overWrite is
     * \c false, or the \a newItem is invalid.
     */
    bool insert(const HServiceSetup& newItem, bool overWrite = false);

    /*!
     * Removes an existing item.
     *
     * \param id specifies the service ID of the item to be removed.
     *
     * \return \e true in case the item was found and removed.
     */
    bool remove(const HServiceId& id);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the provided objects are equal, false otherwise.
 *
 * \relates HServicesSetupData
 */
H_UPNP_CORE_EXPORT bool operator==(const HServicesSetupData&, const HServicesSetupData&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the provided objects are not equal, false otherwise.
 *
 * \relates HServicesSetupData
 */
inline bool operator!=(const HServicesSetupData& obj1, const HServicesSetupData& obj2)
{
    return !(obj1 == obj2);
}

}
}

#endif /* HSERVICES_SETUPDATA_H_ */
