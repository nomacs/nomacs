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

#include "hcds_datasource.h"
#include "hcds_datasource_p.h"
#include "hcds_datasource_configuration.h"
#include "../cds_objects/hitem.h"
#include "../cds_objects/hcontainer.h"

#include <QtCore/QSet>

/*!
 * \defgroup hupnp_av_cds_ds Data Sources
 * \ingroup hupnp_av_cds
 *
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HCdsDataSource
 *******************************************************************************/
HCdsDataSource::HCdsDataSource(QObject* parent) :
    HAbstractCdsDataSource(parent)
{
}

HCdsDataSource::HCdsDataSource(
    const HCdsDataSourceConfiguration& conf, QObject* parent) :
        HAbstractCdsDataSource(conf, parent)
{
}

HCdsDataSource::~HCdsDataSource()
{
}

}
}
}
