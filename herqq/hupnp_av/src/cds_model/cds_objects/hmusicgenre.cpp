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

#include "hmusicgenre.h"
#include "hmusicgenre_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HMusicGenrePrivate
 ******************************************************************************/
HMusicGenrePrivate::HMusicGenrePrivate(
    const QString& clazz, HObject::CdsType cdsType) :
        HGenreContainerPrivate(clazz, cdsType)
{
}

/*******************************************************************************
 * HMusicGenre
 ******************************************************************************/
HMusicGenre::HMusicGenre(const QString& clazz, CdsType cdsType) :
    HGenreContainer(*new HMusicGenrePrivate(clazz, cdsType))
{
}

HMusicGenre::HMusicGenre(HMusicGenrePrivate& dd) :
    HGenreContainer(dd)
{
}

HMusicGenre::HMusicGenre(
    const QString& title, const QString& parentId, const QString& id) :
        HGenreContainer(*new HMusicGenrePrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HMusicGenre::~HMusicGenre()
{
}

HMusicGenre* HMusicGenre::newInstance() const
{
    return new HMusicGenre();
}

}
}
}
