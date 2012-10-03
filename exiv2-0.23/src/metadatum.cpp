// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      metadatum.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <brad@robotbattle.com>
  History:   26-Jan-04, ahu: created
             31-Jul-04, brad: isolated as a component
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: metadatum.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "metadatum.hpp"

// + standard includes
#include <iostream>
#include <iomanip>


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Key::~Key()
    {
    }

    Key::AutoPtr Key::clone() const
    {
        return AutoPtr(clone_());
    }

    Key& Key::operator=(const Key& /*rhs*/)
    {
        return *this;
    }

    Metadatum::Metadatum()
    {
    }

    Metadatum::Metadatum(const Metadatum& /*rhs*/)
    {
    }

    Metadatum::~Metadatum()
    {
    }

    Metadatum& Metadatum::operator=(const Metadatum& /*rhs*/)
    {
        return *this;
    }

    std::string Metadatum::print(const ExifData* pMetadata) const
    {
        std::ostringstream os;
        write(os, pMetadata);
        return os.str();
    }

    bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs)
    {
        return lhs.tag() < rhs.tag();
    }


    bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs)
    {
        return lhs.key() < rhs.key();
    }

}                                       // namespace Exiv2

