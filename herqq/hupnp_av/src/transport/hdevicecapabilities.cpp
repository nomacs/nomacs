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

#include "hdevicecapabilities.h"

#include <HUpnpAv/HStorageMedium>
#include <HUpnpAv/HRecordQualityMode>

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HDeviceCapabilitiesPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HDeviceCapabilitiesPrivate)

public:

    QSet<HStorageMedium> m_playMedia;
    QSet<HStorageMedium> m_recMedia;
    QSet<HRecordQualityMode> m_reqQualityModes;

    HDeviceCapabilitiesPrivate() :
        m_playMedia(), m_recMedia(), m_reqQualityModes()
    {
    }
};

/*******************************************************************************
 * HDeviceCapabilities
 ******************************************************************************/
HDeviceCapabilities::HDeviceCapabilities() :
    h_ptr(new HDeviceCapabilitiesPrivate())
{
}

HDeviceCapabilities::HDeviceCapabilities(
    const QSet<QString>& playMedia,
    const QSet<QString>& recMedia,
    const QSet<QString>& reqQualityModes) :
        h_ptr(new HDeviceCapabilitiesPrivate())
{
    foreach(const QString& pm, playMedia)
    {
        HStorageMedium sm(pm);
        if (sm.isValid())
        {
            h_ptr->m_playMedia.insert(sm);
        }
    }

    foreach(const QString& rm, recMedia)
    {
        HStorageMedium sm(rm);
        if (sm.isValid())
        {
            h_ptr->m_recMedia.insert(sm);
        }
    }

    foreach(const QString& tmp, reqQualityModes)
    {
        HRecordQualityMode rqm(tmp);
        if (rqm.isValid())
        {
            h_ptr->m_reqQualityModes.insert(rqm);
        }
    }
}

HDeviceCapabilities::HDeviceCapabilities(
    const QSet<HStorageMedium>& playMedia,
    const QSet<HStorageMedium>& recMedia,
    const QSet<HRecordQualityMode>& reqQualityModes) :
        h_ptr(new HDeviceCapabilitiesPrivate())
{
    h_ptr->m_playMedia = playMedia;
    h_ptr->m_recMedia = recMedia;
    h_ptr->m_reqQualityModes = reqQualityModes;

    h_ptr->m_playMedia.remove(HStorageMedium::Unknown);
    h_ptr->m_recMedia.remove(HStorageMedium::Unknown);
    h_ptr->m_reqQualityModes.remove(HRecordQualityMode::Undefined);
}

HDeviceCapabilities::~HDeviceCapabilities()
{
}

HDeviceCapabilities::HDeviceCapabilities(const HDeviceCapabilities& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

HDeviceCapabilities& HDeviceCapabilities::operator=(const HDeviceCapabilities& other)
{
    Q_ASSERT(this != &other);
    h_ptr = other.h_ptr;
    return *this;
}

bool HDeviceCapabilities::isValid() const
{
    return h_ptr->m_playMedia.isEmpty() ?
          !h_ptr->m_recMedia.isEmpty() && !h_ptr->m_reqQualityModes.isEmpty() : true;
}

QSet<HStorageMedium> HDeviceCapabilities::playMedia() const
{
    return h_ptr->m_playMedia;
}

QSet<HStorageMedium> HDeviceCapabilities::recordMedia() const
{
    return h_ptr->m_recMedia;
}

QSet<HRecordQualityMode> HDeviceCapabilities::recordQualityModes() const
{
    return h_ptr->m_reqQualityModes;
}

void HDeviceCapabilities::setPlayMedia(const QSet<HStorageMedium>& arg)
{
    h_ptr->m_playMedia = arg;
}

void HDeviceCapabilities::setRecordMedia(const QSet<HStorageMedium>& arg)
{
    h_ptr->m_recMedia = arg;
}

void HDeviceCapabilities::setRecordQualityModes(const QSet<HRecordQualityMode>& arg)
{
    h_ptr->m_reqQualityModes = arg;
}

bool operator==(const HDeviceCapabilities& obj1, const HDeviceCapabilities& obj2)
{
    return obj1.playMedia() == obj2.playMedia() &&
           obj1.recordMedia() == obj2.recordMedia() &&
           obj1.recordQualityModes() == obj2.recordQualityModes();
}

}
}
}
