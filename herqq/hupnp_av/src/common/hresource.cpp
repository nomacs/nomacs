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

#include "hresource.h"
#include "hprotocolinfo.h"

#include <QtCore/QUrl>
#include <QtCore/QHash>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HResourcePrivate
 ******************************************************************************/
class HResourcePrivate :
    public QSharedData
{
public:

    QHash<QString, QString> m_mediaInfo;
    QUrl m_location;
    HProtocolInfo m_protocolInfo;
    quint32 m_updateCount;
    bool m_trackChangesOptionEnabled;

    HResourcePrivate();
};

HResourcePrivate::HResourcePrivate() :
    m_mediaInfo(), m_location(), m_protocolInfo(), m_updateCount(0),
    m_trackChangesOptionEnabled(false)
{
}

/*******************************************************************************
 * HResource
 ******************************************************************************/
HResource::HResource() :
    h_ptr(new HResourcePrivate())
{
}

HResource::HResource(const HProtocolInfo& protocolInfo) :
    h_ptr(new HResourcePrivate())
{
    h_ptr->m_protocolInfo = protocolInfo;
}

HResource::HResource(const QUrl& location, const HProtocolInfo& protocolInfo) :
    h_ptr(new HResourcePrivate())
{
    h_ptr->m_location = location;
    h_ptr->m_protocolInfo = protocolInfo;
}

HResource::~HResource()
{
}

HResource::HResource(const HResource& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HResource& HResource::operator=(const HResource& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

QUrl HResource::location() const
{
    return h_ptr->m_location;
}

const HProtocolInfo& HResource::protocolInfo() const
{
    return h_ptr->m_protocolInfo;
}

const QHash<QString, QString>& HResource::mediaInfo() const
{
    return h_ptr->m_mediaInfo;
}

quint32 HResource::updateCount() const
{
    return h_ptr->m_updateCount;
}

bool HResource::trackChangesOptionEnabled() const
{
    return h_ptr->m_trackChangesOptionEnabled;
}

void HResource::setLocation(const QUrl& arg)
{
    h_ptr->m_location = arg;
}

void HResource::setProtocolInfo(const HProtocolInfo& arg)
{
    h_ptr->m_protocolInfo = arg;
}

void HResource::setMediaInfo(const QHash<QString, QString>& arg)
{
    h_ptr->m_mediaInfo = arg;
}

void HResource::setUpdateCount(quint32 arg)
{
    h_ptr->m_updateCount = arg;
}

void HResource::enableTrackChangesOption(bool arg)
{
    h_ptr->m_trackChangesOptionEnabled = arg;
}

bool operator==(const HResource& obj1, const HResource& obj2)
{
    return obj1.h_ptr->m_location == obj2.h_ptr->m_location &&
           obj1.h_ptr->m_protocolInfo == obj2.h_ptr->m_protocolInfo &&
           obj1.h_ptr->m_mediaInfo    == obj2.h_ptr->m_mediaInfo &&
           obj1.h_ptr->m_trackChangesOptionEnabled ==
           obj2.h_ptr->m_trackChangesOptionEnabled &&
           obj1.h_ptr->m_mediaInfo    == obj2.h_ptr->m_mediaInfo;
}

}
}
}

