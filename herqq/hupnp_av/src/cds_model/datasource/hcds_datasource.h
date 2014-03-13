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

#ifndef HCDS_DATASOURCE_H_
#define HCDS_DATASOURCE_H_

#include <HUpnpAv/HAbstractCdsDataSource>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsDataSourcePrivate;

/*!
 * \brief This class is used to store instances of the HUPnPAv CDS object model.
 *
 * \headerfile hcds_datasource.h HCdsDataSource
 *
 * \ingroup hupnp_av_cds_ds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HCdsDataSourceConfiguration
 */
class H_UPNP_AV_EXPORT HCdsDataSource :
    public HAbstractCdsDataSource
{
Q_OBJECT
H_DISABLE_COPY(HCdsDataSource)

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject.
     *
     * \sa init(), isInitialized()
     *
     * \remarks a default configuration is created for the data source.
     */
    HCdsDataSource(QObject* parent = 0);

    /*!
     * \brief Creates a new instance.
     *
     * \param conf specifies the configuration of the data source.
     *
     * \param parent specifies the parent \c QObject.
     *
     * \sa init(), isInitialized()
     */
    HCdsDataSource(const HCdsDataSourceConfiguration& conf, QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HCdsDataSource();

    using HAbstractCdsDataSource::add;
    using HAbstractCdsDataSource::remove;
    using HAbstractCdsDataSource::clear;
    using HAbstractCdsDataSource::configuration;
};

}
}
}

#endif /* HCDS_DATASOURCE_H_ */
