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

#include "hrootdir.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRootDir
 *******************************************************************************/
HRootDir::HRootDir() :
    m_dir(), m_scanMode(SingleDirectoryScan), m_watchMode(NoWatch)
{
}

HRootDir::HRootDir(const QDir& dir, ScanMode scanMode, WatchMode watchMode) :
    m_dir(), m_scanMode(), m_watchMode()
{
    if (dir.exists())
    {
        m_dir = dir;
        m_scanMode = scanMode;
        m_watchMode = watchMode;
    }
}

bool HRootDir::overlaps(const HRootDir& other) const
{
    bool retVal = false;

    QString thisPath = m_dir.absolutePath();
    QString otherPath = other.dir().absolutePath();

    if (thisPath == otherPath)
    {
        retVal = true;
    }
    else if (otherPath.startsWith(thisPath) && scanMode() == RecursiveScan)
    {
        retVal = true;
    }

    return retVal;
}

bool HRootDir::setDir(const QDir& dir)
{
    if (dir.exists())
    {
        m_dir = dir;
        return true;
    }
    return false;
}

void HRootDir::setScanMode(ScanMode smode)
{
    m_scanMode = smode;
}

bool operator==(const HRootDir& obj1, const HRootDir& obj2)
{
    return obj1.dir() == obj2.dir() &&
           obj1.watchMode() == obj2.watchMode() &&
           obj1.scanMode() == obj2.scanMode();
}

}
}
}
