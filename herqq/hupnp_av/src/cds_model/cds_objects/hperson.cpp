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

#include "hperson.h"
#include "hperson_p.h"
#include "../model_mgmt/hcdsproperties.h"

#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPersonPrivate
 ******************************************************************************/
HPersonPrivate::HPersonPrivate(const QString& clazz, HObject::CdsType cdsType) :
    HContainerPrivate(clazz, cdsType)
{
    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::dc_language));
}

/*******************************************************************************
 * HPerson
 ******************************************************************************/
HPerson::HPerson(const QString& clazz, CdsType cdsType) :
    HContainer(*new HPersonPrivate(clazz, cdsType))
{
}

HPerson::HPerson(HPersonPrivate& dd) :
    HContainer(dd)
{
}

HPerson::HPerson(
    const QString& title, const QString& parentId, const QString& id) :
        HContainer(*new HPersonPrivate(sClass(), sType()))
{
    init(title, parentId, id);
}

HPerson::~HPerson()
{
}

QStringList HPerson::languages() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dc_language, &value);
    return value.toStringList();
}

void HPerson::setLanguages(const QStringList& arg)
{
    setCdsProperty(HCdsProperties::dc_language, arg);
}

HPerson* HPerson::newInstance() const
{
    return new HPerson();
}

}
}
}
