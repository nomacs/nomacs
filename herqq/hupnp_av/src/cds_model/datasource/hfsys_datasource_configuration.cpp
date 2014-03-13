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

#include "hfsys_datasource_configuration.h"
#include "hfsys_datasource_configuration_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HFileSystemDataSourceConfigurationPrivate
 *******************************************************************************/
HFileSystemDataSourceConfigurationPrivate::HFileSystemDataSourceConfigurationPrivate() :
    m_rootDirs()
{
}

HFileSystemDataSourceConfigurationPrivate::~HFileSystemDataSourceConfigurationPrivate()
{
}

/*******************************************************************************
 * HFileSystemDataSourceConfiguration
 *******************************************************************************/
HFileSystemDataSourceConfiguration::HFileSystemDataSourceConfiguration() :
    HCdsDataSourceConfiguration(
        *new HFileSystemDataSourceConfigurationPrivate())
{
}

HFileSystemDataSourceConfiguration::HFileSystemDataSourceConfiguration(
    HFileSystemDataSourceConfigurationPrivate& dd) :
        HCdsDataSourceConfiguration(dd)
{
}

HFileSystemDataSourceConfiguration::~HFileSystemDataSourceConfiguration()
{
}

void HFileSystemDataSourceConfiguration::doClone(HClonable* target) const
{
    const H_D(HFileSystemDataSourceConfiguration);

    HFileSystemDataSourceConfiguration* conf =
        dynamic_cast<HFileSystemDataSourceConfiguration*>(target);

    if (!conf)
    {
        return;
    }

    HFileSystemDataSourceConfigurationPrivate* confPriv =
        static_cast<HFileSystemDataSourceConfigurationPrivate*>(
            conf->h_ptr);

    confPriv->m_rootDirs = h->m_rootDirs;
}

HFileSystemDataSourceConfiguration* HFileSystemDataSourceConfiguration::newInstance() const
{
    return new HFileSystemDataSourceConfiguration();
}

HFileSystemDataSourceConfiguration* HFileSystemDataSourceConfiguration::clone() const
{
    return static_cast<HFileSystemDataSourceConfiguration*>(HClonable::clone());
}

HRootDirs HFileSystemDataSourceConfiguration::rootDirs() const
{
    const H_D(HFileSystemDataSourceConfiguration);
    return h->m_rootDirs;
}

bool HFileSystemDataSourceConfiguration::addRootDir(const HRootDir& rootDir)
{
    H_D(HFileSystemDataSourceConfiguration);

    foreach(const HRootDir& rd, h->m_rootDirs)
    {
        if (rd.overlaps(rootDir))
        {
            return false;
        }
    }

    h->m_rootDirs.append(rootDir);
    return true;
}

bool HFileSystemDataSourceConfiguration::removeRootDir(const HRootDir& dir)
{
    H_D(HFileSystemDataSourceConfiguration);

    HRootDirs::iterator it = h->m_rootDirs.begin();
    for(; it != h->m_rootDirs.end(); ++it)
    {
        if (it->dir() == dir.dir())
        {
            h->m_rootDirs.erase(it);
            return true;
        }
    }
    return false;
}

bool HFileSystemDataSourceConfiguration::setRootDirs(const HRootDirs& dirs)
{
    H_D(HFileSystemDataSourceConfiguration);

    if (dirs.isEmpty())
    {
        h->m_rootDirs = dirs;
        return true;
    }

    HRootDirs tmp;
    tmp.append(dirs.at(0));

    for(int i = 1; i < dirs.size(); ++i)
    {
        foreach(const HRootDir& rd, tmp)
        {
            if (rd.overlaps(dirs.at(i)))
            {
                return false;
            }
        }
    }

    h->m_rootDirs = dirs;
    return true;
}

void HFileSystemDataSourceConfiguration::clear()
{
    H_D(HFileSystemDataSourceConfiguration);
    h->m_rootDirs.clear();
}

}
}
}
