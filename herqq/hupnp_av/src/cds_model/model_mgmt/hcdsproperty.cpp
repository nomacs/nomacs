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

#include "hcdsproperty.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{
class HCdsPropertyHandlerPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HCdsPropertyHandlerPrivate)

public:

    HOutSerializer m_outSerializer;
    HInSerializer m_inSerializer;
    HComparer m_comparer;
    HValidator m_validator;
};

/*******************************************************************************
 * HCdsPropertyHandler
 ******************************************************************************/
HCdsPropertyHandler::HCdsPropertyHandler() :
    h_ptr(new HCdsPropertyHandlerPrivate())
{
}

HCdsPropertyHandler::HCdsPropertyHandler(
    const HOutSerializer& oser, const HInSerializer& iser, const HComparer& cmp,
    const HValidator& validator) :
        h_ptr(new HCdsPropertyHandlerPrivate())
{
    h_ptr->m_outSerializer = oser;
    h_ptr->m_inSerializer = iser;
    h_ptr->m_comparer = cmp;
    h_ptr->m_validator = validator;
}

HCdsPropertyHandler::HCdsPropertyHandler(const HCdsPropertyHandler& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HCdsPropertyHandler& HCdsPropertyHandler::operator=(const HCdsPropertyHandler& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HCdsPropertyHandler::~HCdsPropertyHandler()
{
}

bool HCdsPropertyHandler::isValid() const
{
    return h_ptr->m_outSerializer && h_ptr->m_inSerializer;
}

HOutSerializer HCdsPropertyHandler::outSerializer() const
{
    return h_ptr->m_outSerializer;
}

HInSerializer HCdsPropertyHandler::inSerializer() const
{
    return h_ptr->m_inSerializer;
}

HComparer HCdsPropertyHandler::comparer() const
{
    return h_ptr->m_comparer;
}

HValidator HCdsPropertyHandler::validator() const
{
    return h_ptr->m_validator;
}

void HCdsPropertyHandler::setOutSerializer(const HOutSerializer& arg)
{
    h_ptr->m_outSerializer = arg;
}

void HCdsPropertyHandler::setInSerializer(const HInSerializer& arg)
{
    h_ptr->m_inSerializer = arg;
}

void HCdsPropertyHandler::setComparer(const HComparer& arg)
{
    h_ptr->m_comparer = arg;
}

void HCdsPropertyHandler::setValidator(const HValidator& arg)
{
    h_ptr->m_validator = arg;
}

/*******************************************************************************
 * HCdsProperty
 ******************************************************************************/
HCdsProperty::HCdsProperty() :
    m_info(), m_handler()
{
}

HCdsProperty::HCdsProperty(
    const HCdsPropertyInfo& info, const HCdsPropertyHandler& handler) :
        m_info(info), m_handler(handler)
{
}

bool HCdsProperty::isValid() const
{
    return m_info.isValid() && m_handler.isValid();
}

}
}
}
