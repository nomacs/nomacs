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
  @file    sonymn_int.hpp
  @brief   Sony MakerNote implemented using the following references:<br>
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Sony.html">Sony Makernote list</a> by Phil Harvey<br>
Email communication with <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a><br>

  @version $Rev: 3777 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
             @author  Gilles Caulier (cgilles)
           <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
  @date    18-Apr-05, ahu: created
 */
#ifndef SONYMN_INT_HPP_
#define SONYMN_INT_HPP_

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

    //! MakerNote for Sony cameras
    class SonyMakerNote {
    public:
        //! Return read-only list of built-in Sony tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in Sony Standard Camera Settings tags
        static const TagInfo* tagListCs();
        //! Return read-only list of built-in Sony Standard Camera Settings version 2 tags
        static const TagInfo* tagListCs2();

        //! @name Print functions for Sony %MakerNote tags
        //@{
        //! Print Sony Camera Model
        static std::ostream& print0xb000(std::ostream&, const Value&, const ExifData*);
        //! Print Full and Preview Image size
        static std::ostream& printImageSize(std::ostream&, const Value&, const ExifData*);

    private:
        //! Tag information
        static const TagInfo tagInfo_[];
        static const TagInfo tagInfoCs_[];
        static const TagInfo tagInfoCs2_[];

    }; // class SonyMakerNote

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef SONYMN_INT_HPP_
