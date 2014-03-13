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

#ifndef HCONTENTDIRECTORY_SERVICECONFIGURATION_H_
#define HCONTENTDIRECTORY_SERVICECONFIGURATION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HContentDirectoryServiceConfigurationPrivate;

/*!
 * \brief This class contains the configuration data passed to a HContentDirectoryService.
 *
 * \headerfile hcontentdirectory_serviceconfiguration.h HContentDirectoryServiceConfiguration
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HContentDirectoryService
 */
class H_UPNP_AV_EXPORT HContentDirectoryServiceConfiguration :
    public HClonable
{
H_DISABLE_COPY(HContentDirectoryServiceConfiguration)
H_DECLARE_PRIVATE(HContentDirectoryServiceConfiguration)

protected:

    HContentDirectoryServiceConfigurationPrivate* h_ptr;
    HContentDirectoryServiceConfiguration(HContentDirectoryServiceConfigurationPrivate& dd);

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;
    // Documented in HClonable
    virtual HContentDirectoryServiceConfiguration* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     */
    HContentDirectoryServiceConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HContentDirectoryServiceConfiguration();

    // Documented in HClonable
    virtual HContentDirectoryServiceConfiguration* clone() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. the dataSource() is defined.
     */
    virtual bool isValid() const;

    /*!
     * Indicates if the configuration object has the ownership of the data source.
     *
     * \return \e true if the configuration object has the ownership of the data source.
     *
     * \remarks This function returns \e false when no data source is set.
     *
     * \sa setDataSource(), dataSource(), detachDataSource()
     */
    bool hasOwnershipOfDataSource() const;

    /*!
     * \brief Returns the data source to be used by the ContentDirectoryService.
     *
     * \return The data source to be used by the ContentDirectoryService.
     *
     * \sa isValid()
     *
     * \remarks The ownership of the data source is \b never transferred.
     *
     * \sa setDatasource()
     */
    HAbstractCdsDataSource* dataSource() const;

    /*!
     * \brief Sets the data source.
     *
     * \param dataSource specifies the data source to be used by the
     * HMediaServerDevice.
     *
     * \param takeOwnership specifies whether the ownership of the \c dataSource
     * is passed to the ContentDirectoryService instance. If \e true, the ContentDirectoryService
     * will delete the data source once it is done using it. Otherwise the caller
     * is responsible for ensuring that the data source is not deleted before the
     * ContentDirectoryService.
     *
     * \sa datasource(), detachDatasource()
     */
    void setDataSource(HAbstractCdsDataSource* dataSource, bool takeOwnership);

    /*!
     * \brief Detaches the data source associated with this instance, if any.
     *
     * If the object has a data source set and the object has the ownership of it,
     * this call decreases the reference count of the associated data source.
     * In this case if the reference count goes to zero the associated data source
     * is deleted.
     *
     * \sa setDataSource()
     */
    void detachDataSource();
};

}
}
}

#endif /* HCONTENTDIRECTORY_SERVICECONFIGURATION_H_ */
