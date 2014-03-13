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

#include "hmusicalbum.h"
#include "hmusicalbum_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../hgenre.h"
#include "../hpersonwithrole.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMusicAlbumPrivate
 ******************************************************************************/
HMusicAlbumPrivate::HMusicAlbumPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HAlbumPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_artist));
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_producer));
    insert(inst.get(HCdsProperties::upnp_albumArtURI));
    insert(inst.get(HCdsProperties::upnp_toc));
}

/*******************************************************************************
 * HMusicAlbum
 ******************************************************************************/
HMusicAlbum::HMusicAlbum(const QString& clazz, CdsType cdsType) :
    HAlbum(*new HMusicAlbumPrivate(clazz, cdsType))
{
}

HMusicAlbum::HMusicAlbum(HMusicAlbumPrivate& dd) :
    HAlbum(dd)
{
}

HMusicAlbum::HMusicAlbum(
    const QString& title, const QString& parentId, const QString& id) :
        HAlbum(*new HMusicAlbumPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HMusicAlbum::~HMusicAlbum()
{
}

HMusicAlbum* HMusicAlbum::newInstance() const
{
    return new HMusicAlbum();
}

void HMusicAlbum::setArtists(const QList<HPersonWithRole>& arg)
{
    setCdsProperty(HCdsProperties::upnp_artist, toList(arg));
}

void HMusicAlbum::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HMusicAlbum::setProducers(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::upnp_producer, arg);
}

void HMusicAlbum::setAlbumArtUrls(const QList<QUrl>& arg)
{
    setCdsProperty(HCdsProperties::upnp_albumArtURI, toList(arg));
}

void HMusicAlbum::setToc(const QString& arg)
{
    setCdsProperty(HCdsProperties::upnp_toc, arg);
}

QList<HPersonWithRole> HMusicAlbum::artists() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_artist, &value);
    return toList<HPersonWithRole>(value.toList());
}

QList<HGenre> HMusicAlbum::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QStringList HMusicAlbum::producers() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_producer, &value);
    return value.toStringList();
}

QList<QUrl> HMusicAlbum::albumArtUrls() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_albumArtURI, &value);
    return toList<QUrl>(value.toList());
}

QString HMusicAlbum::toc() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_toc, &value);
    return value.toString();
}

}
}
}
