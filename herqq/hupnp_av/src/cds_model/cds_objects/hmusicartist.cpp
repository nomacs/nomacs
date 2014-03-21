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

#include "hmusicartist.h"
#include "hmusicartist_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include "../hgenre.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMusicArtistPrivate
 ******************************************************************************/
HMusicArtistPrivate::HMusicArtistPrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HPersonPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::upnp_genre));
    insert(inst.get(HCdsProperties::upnp_artistDiscographyURI));
}

/*******************************************************************************
 * HMusicArtist
 ******************************************************************************/
HMusicArtist::HMusicArtist(const QString& clazz, CdsType cdsType) :
    HPerson(*new HMusicArtistPrivate(clazz, cdsType))
{
}

HMusicArtist::HMusicArtist(HMusicArtistPrivate& dd) :
    HPerson(dd)
{
}

HMusicArtist::HMusicArtist(
    const QString& title, const QString& parentId, const QString& id) :
        HPerson(*new HMusicArtistPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HMusicArtist::~HMusicArtist()
{
}

HMusicArtist* HMusicArtist::newInstance() const
{
    return new HMusicArtist();
}

void HMusicArtist::setGenres(const QList<HGenre>& arg)
{
    setCdsProperty(HCdsProperties::upnp_genre, toList(arg));
}

void HMusicArtist::setArtistDiscographyUri(const QUrl& arg)
{
    setCdsProperty(HCdsProperties::upnp_artistDiscographyURI, arg);
}

QList<HGenre> HMusicArtist::genres() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_genre, &value);
    return toList<HGenre>(value.toList());
}

QUrl HMusicArtist::artistDiscographyUri() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::upnp_artistDiscographyURI, &value);
    return value.toUrl();
}

}
}
}
