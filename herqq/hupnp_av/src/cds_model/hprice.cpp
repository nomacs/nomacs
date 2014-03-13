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

#include "hprice.h"

#include <QtCore/QMetaType>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HPrice>("Herqq::Upnp::Av::HPrice");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

namespace
{
inline bool isValidCurrency(const QString& arg)
{
    return arg.size() == 3; // TODO
}
}

HPrice::HPrice() :
    m_value(-1), m_currency()
{
}

HPrice::HPrice(float value, const QString& currency) :
    m_value(value), m_currency(currency)
{
    if (value < 0)
    {
        return;
    }
}

HPrice::~HPrice()
{
}

bool HPrice::isValid() const
{
    return m_value >= 0 && isValidCurrency(m_currency);
}

bool operator==(const HPrice& obj1, const HPrice& obj2)
{
    return obj1.value() == obj2.value() &&
           obj1.currency() == obj2.currency();
}

}
}
}

