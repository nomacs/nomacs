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

#include "htransportsettings.h"

#include <HUpnpAv/HPlayMode>
#include <HUpnpAv/HRecordQualityMode>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HTransportSettingsPrivate
 ******************************************************************************/
class HTransportSettingsPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HTransportSettingsPrivate)

public:
    HPlayMode m_playMode;
    HRecordQualityMode m_rQualityMode;

    HTransportSettingsPrivate() :
        m_playMode(), m_rQualityMode()
    {
    }
};

/*******************************************************************************
 * HTransportSettings
 ******************************************************************************/
HTransportSettings::HTransportSettings() :
    h_ptr(new HTransportSettingsPrivate())
{
}

HTransportSettings::HTransportSettings(
    const HPlayMode& pm, const HRecordQualityMode& rqm) :
        h_ptr(new HTransportSettingsPrivate())
{
    h_ptr->m_playMode = pm;
    h_ptr->m_rQualityMode = rqm;
}

HTransportSettings::HTransportSettings(const HTransportSettings& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HTransportSettings& HTransportSettings::operator=(const HTransportSettings& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HTransportSettings::~HTransportSettings()
{
}

bool HTransportSettings::isEmpty() const
{
    return !h_ptr->m_playMode.isValid() && !h_ptr->m_rQualityMode.isValid();
}

const HPlayMode& HTransportSettings::playMode() const
{
    return h_ptr->m_playMode;
}

HRecordQualityMode HTransportSettings::recordQualityMode() const
{
    return h_ptr->m_rQualityMode;
}

void HTransportSettings::setPlayMode(const HPlayMode& arg)
{
    h_ptr->m_playMode = arg;
}

void HTransportSettings::setRecordQualityMode(const HRecordQualityMode& arg)
{
    h_ptr->m_rQualityMode = arg;
}

bool operator==(const HTransportSettings& obj1, const HTransportSettings& obj2)
{
    return obj1.playMode() == obj2.playMode() &&
           obj1.recordQualityMode() == obj2.recordQualityMode();
}

}
}
}
