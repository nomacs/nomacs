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

#include "hgenre.h"

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HGenre>("Herqq::Upnp::Av::HGenre");
    return true;
}

static bool regMetaT = registerMetaTypes();

/*!
 * \defgroup hupnp_av_cds_common Common
 * \ingroup hupnp_av_cds
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HGenre::HGenre() :
    m_name(), m_id(), m_extended()
{
}

HGenre::HGenre(const QString& name) :
    m_name(name.trimmed()), m_id(), m_extended()
{
}

HGenre::HGenre(
    const QString& name, const QString& id, const QStringList& extended) :
        m_name(), m_id(), m_extended()
{
    QString nameTrimmed = name.trimmed();
    if (!nameTrimmed.isEmpty())
    {
        m_name = nameTrimmed;

        QString idTrimmed = id.trimmed();
        if (!idTrimmed.isEmpty())
        {
            m_id = idTrimmed;
        }

        if (extended.size() > 0 && extended.at(0) == nameTrimmed)
        {
            QStringList extendedChecked; extendedChecked.append(nameTrimmed);
            for(int i = 1; i < extended.size(); ++i)
            {
                QString extTrimmed = extended.at(i).trimmed();
                if (extTrimmed.isEmpty())
                {
                    return;
                }
                extendedChecked.append(extTrimmed);
            }
            m_extended = extendedChecked;
        }
    }
}

HGenre::~HGenre()
{
}

bool HGenre::isValid() const
{
    return !m_name.isEmpty();
}

bool operator==(const HGenre& obj1, const HGenre& obj2)
{
    return obj1.id() == obj2.id() &&
           obj1.name() == obj2.name() &&
           obj1.extended() == obj2.extended();
}

}
}
}

