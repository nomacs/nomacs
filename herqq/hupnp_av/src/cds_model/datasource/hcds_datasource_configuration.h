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

#ifndef HCDS_DATASOURCE_CONFIGURATION_H_
#define HCDS_DATASOURCE_CONFIGURATION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsDataSourceConfigurationPrivate;

/*!
 * \brief This class contains the configuration data passed to a HCdsDataSource.
 *
 * \headerfile hcds_datasource_configuration.h HCdsDataSourceConfiguration
 *
 * \ingroup hupnp_av_cds_ds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HCdsDataSource
 */
class H_UPNP_AV_EXPORT HCdsDataSourceConfiguration :
    public HClonable
{
H_DISABLE_COPY(HCdsDataSourceConfiguration)
H_DECLARE_PRIVATE(HCdsDataSourceConfiguration)

protected:

    HCdsDataSourceConfigurationPrivate* h_ptr;

    // Internal
    HCdsDataSourceConfiguration(HCdsDataSourceConfigurationPrivate& dd);

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;
    // Documented in HClonable
    virtual HCdsDataSourceConfiguration* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     */
    HCdsDataSourceConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HCdsDataSourceConfiguration();

    // Documented in HClonable
    virtual HCdsDataSourceConfiguration* clone() const;
};

}
}
}

#endif /* HCDS_DATASOURCE_CONFIGURATION_H_ */
