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

#include "hcds_datasource_configuration.h"
#include "hcds_datasource_configuration_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HCdsDataSourceConfigurationPrivate
 *******************************************************************************/
HCdsDataSourceConfigurationPrivate::HCdsDataSourceConfigurationPrivate()
{
}

HCdsDataSourceConfigurationPrivate::~HCdsDataSourceConfigurationPrivate()
{
}

/*******************************************************************************
 * HCdsDataSourceConfiguration
 *******************************************************************************/
HCdsDataSourceConfiguration::HCdsDataSourceConfiguration() :
    h_ptr(new HCdsDataSourceConfigurationPrivate())
{
}

HCdsDataSourceConfiguration::HCdsDataSourceConfiguration(
    HCdsDataSourceConfigurationPrivate& dd) :
        h_ptr(&dd)
{
}

HCdsDataSourceConfiguration::~HCdsDataSourceConfiguration()
{
    delete h_ptr;
}

void HCdsDataSourceConfiguration::doClone(HClonable* target) const
{
    Q_UNUSED(target)
//    HCdsDataSourceConfiguration* conf =
//        dynamic_cast<HCdsDataSourceConfiguration*>(target);
//
//    if (!conf)
//    {
//        return;
//    }
}

HCdsDataSourceConfiguration* HCdsDataSourceConfiguration::newInstance() const
{
    return new HCdsDataSourceConfiguration();
}

HCdsDataSourceConfiguration* HCdsDataSourceConfiguration::clone() const
{
    return static_cast<HCdsDataSourceConfiguration*>(HClonable::clone());
}

}
}
}
