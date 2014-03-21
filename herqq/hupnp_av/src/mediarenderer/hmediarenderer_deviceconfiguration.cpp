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

#include "hmediarenderer_deviceconfiguration.h"
#include "hmediarenderer_deviceconfiguration_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMediaRendererDeviceConfigurationPrivate
 ******************************************************************************/
HMediaRendererDeviceConfigurationPrivate::HMediaRendererDeviceConfigurationPrivate() :
    m_mm(0), m_refCnt(0), m_hasOwnership(false)
{
}

HMediaRendererDeviceConfigurationPrivate::~HMediaRendererDeviceConfigurationPrivate()
{
    detach();
}

void HMediaRendererDeviceConfigurationPrivate::detach()
{
    if (m_refCnt && --*m_refCnt == 0)
    {
        if (m_hasOwnership)
        {
            delete m_mm;
        }
        delete m_refCnt;
    }
    m_mm = 0;
    m_refCnt = 0;
}

/*******************************************************************************
 * HMediaRendererDeviceConfiguration
 ******************************************************************************/
HMediaRendererDeviceConfiguration::HMediaRendererDeviceConfiguration() :
    h_ptr(new HMediaRendererDeviceConfigurationPrivate())
{
}

HMediaRendererDeviceConfiguration::~HMediaRendererDeviceConfiguration()
{
    delete h_ptr;
}

void HMediaRendererDeviceConfiguration::doClone(HClonable* target) const
{
    HMediaRendererDeviceConfiguration* conf =
        dynamic_cast<HMediaRendererDeviceConfiguration*>(target);

    if (!target)
    {
        return;
    }

    if (h_ptr->m_mm)
    {
        conf->h_ptr->m_refCnt = h_ptr->m_refCnt;
        ++(*h_ptr->m_refCnt);
    }
    conf->h_ptr->m_hasOwnership = h_ptr->m_hasOwnership;
    conf->h_ptr->m_mm = h_ptr->m_mm;
}

HMediaRendererDeviceConfiguration* HMediaRendererDeviceConfiguration::newInstance() const
{
    return new HMediaRendererDeviceConfiguration();
}

HMediaRendererDeviceConfiguration* HMediaRendererDeviceConfiguration::clone() const
{
    return static_cast<HMediaRendererDeviceConfiguration*>(HClonable::clone());
}

bool HMediaRendererDeviceConfiguration::isValid() const
{
    return h_ptr->m_mm;
}

HRendererConnectionManager* HMediaRendererDeviceConfiguration::rendererConnectionManager() const
{
    return h_ptr->m_mm;
}

bool HMediaRendererDeviceConfiguration::hasOwnershipOfRendererConnectionManager() const
{
    return h_ptr->m_mm && h_ptr->m_hasOwnership;
}

void HMediaRendererDeviceConfiguration::setRendererConnectionManager(
    HRendererConnectionManager* mm, bool takeOwnership)
{
    Q_ASSERT_X(mm, "", "A valid media manager has to be provided");

    h_ptr->detach();

    if (!h_ptr->m_refCnt)
    {
        h_ptr->m_refCnt = new int(1);
    }

    h_ptr->m_hasOwnership = takeOwnership;
    h_ptr->m_mm = mm;
}

void HMediaRendererDeviceConfiguration::detachRendererConnectionManager()
{
    h_ptr->detach();
}

}
}
}
