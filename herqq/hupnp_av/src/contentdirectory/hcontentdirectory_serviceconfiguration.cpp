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

#include "hcontentdirectory_serviceconfiguration.h"
#include "hcontentdirectory_serviceconfiguration_p.h"

#include "../cds_model/datasource/habstract_cds_datasource.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HContentDirectoryServiceConfigurationPrivate
 *******************************************************************************/
HContentDirectoryServiceConfigurationPrivate::HContentDirectoryServiceConfigurationPrivate() :
    m_dataSource(0), m_refCnt(0), m_hasOwnership(false)
{
}

HContentDirectoryServiceConfigurationPrivate::~HContentDirectoryServiceConfigurationPrivate()
{
    detach();
}

void HContentDirectoryServiceConfigurationPrivate::detach()
{
    if (m_refCnt && --*m_refCnt == 0)
    {
        if (m_hasOwnership)
        {
            delete m_dataSource;
        }
        delete m_refCnt;
    }
    m_dataSource = 0;
    m_refCnt = 0;
}

/*******************************************************************************
 * HContentDirectoryServiceConfiguration
 *******************************************************************************/
HContentDirectoryServiceConfiguration::HContentDirectoryServiceConfiguration() :
    h_ptr(new HContentDirectoryServiceConfigurationPrivate())
{
}

HContentDirectoryServiceConfiguration::HContentDirectoryServiceConfiguration(
    HContentDirectoryServiceConfigurationPrivate& dd) :
        h_ptr(&dd)
{
}

HContentDirectoryServiceConfiguration::~HContentDirectoryServiceConfiguration()
{
    delete h_ptr;
}

void HContentDirectoryServiceConfiguration::doClone(HClonable* target) const
{
    HContentDirectoryServiceConfiguration* conf =
        dynamic_cast<HContentDirectoryServiceConfiguration*>(target);

    if (!conf)
    {
        return;
    }

    if (h_ptr->m_dataSource)
    {
        conf->h_ptr->m_refCnt = h_ptr->m_refCnt;
        ++(*h_ptr->m_refCnt);
    }
    conf->h_ptr->m_hasOwnership = h_ptr->m_hasOwnership;
    conf->h_ptr->m_dataSource = h_ptr->m_dataSource;
}

HContentDirectoryServiceConfiguration* HContentDirectoryServiceConfiguration::newInstance() const
{
    return new HContentDirectoryServiceConfiguration();
}

HContentDirectoryServiceConfiguration* HContentDirectoryServiceConfiguration::clone() const
{
    return static_cast<HContentDirectoryServiceConfiguration*>(HClonable::clone());
}

bool HContentDirectoryServiceConfiguration::isValid() const
{
    return h_ptr->m_dataSource;
}

bool HContentDirectoryServiceConfiguration::hasOwnershipOfDataSource() const
{
    return h_ptr->m_dataSource && h_ptr->m_hasOwnership;
}

HAbstractCdsDataSource* HContentDirectoryServiceConfiguration::dataSource() const
{
    return h_ptr->m_dataSource.data();
}

void HContentDirectoryServiceConfiguration::setDataSource(
    HAbstractCdsDataSource* dataSource, bool takeOwnership)
{
    Q_ASSERT_X(dataSource, "", "A valid data source has to be provided");

    h_ptr->detach();

    if (!h_ptr->m_refCnt)
    {
        h_ptr->m_refCnt = new int(1);
    }

    h_ptr->m_hasOwnership = takeOwnership;
    h_ptr->m_dataSource = dataSource;
}

void HContentDirectoryServiceConfiguration::detachDataSource()
{
    h_ptr->detach();
}

}
}
}
