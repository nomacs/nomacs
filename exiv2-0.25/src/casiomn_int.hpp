// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
/*!
  @file    casiomn_int.hpp
  @brief   Casio MakerNote implemented using the following references:
           <a href="http://gvsoft.homedns.org/exif/makernote-casio-type1.html">Casio MakerNote Information</a> by GVsoft,
           Casio.pm of <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey,
           <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/casio_mn.html#Casio_Type_1_Tags">Casio Makernote Format Specification</a> by Evan Hunter.
  @version $Rev: 3091 $
  @date    30-Oct-13, ahu: created
 */
#ifndef CASIOMN_INT_HPP_
#define CASIOMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Casio cameras
    class CasioMakerNote {
    public:
        //! Return read-only list of built-in Casio tags
        static const TagInfo* tagList();
        //! Print ObjectDistance
        static std::ostream& print0x0006(std::ostream& os, const Value& value, const ExifData*);
        //! Print FirmwareDate
        static std::ostream& print0x0015(std::ostream& os, const Value& value, const ExifData*);

    private:
        //! Makernote tag list
        static const TagInfo tagInfo_[];

    }; // class CasioMakerNote

    //! MakerNote for Casio2 cameras
    class Casio2MakerNote {
    public:
        //! Return read-only list of built-in Casio2 tags
        static const TagInfo* tagList();
        //! Print FirmwareDate
        static std::ostream& print0x2001(std::ostream& os, const Value& value, const ExifData*);
        //! Print ObjectDistance
        static std::ostream& print0x2022(std::ostream& os, const Value& value, const ExifData*);

    private:
        //! Makernote tag list
        static const TagInfo tagInfo_[];

    }; // class Casio2MakerNote

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef CasioMN_INT_HPP_
