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
/*!
  @file    olympusmn_int.hpp
  @brief   Olympus makernote tags.<br>References:<br>
           [1] <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html#APP1">Exif file format, Appendix 1: MakerNote of Olympus Digicams</a> by TsuruZoh Tachibanaya<br>
           [2] <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey<br>
           [3] <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/olympus_mn.html">Olympus Makernote Format Specification</a> by Evan Hunter<br>
           [4] email communication with <a href="mailto:wstokes@gmail.com">Will Stokes</a>
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Will Stokes (wuz)
           <a href="mailto:wstokes@gmail.com">wstokes@gmail.com</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
  @author  Greg Mansfield
           <a href="mailto:G.Mansfield at computer dot org">G.Mansfield at computer dot org</a>
  @date    10-Mar-05, wuz: created
 */
#ifndef OLYMPUSMN_INT_HPP_
#define OLYMPUSMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <iosfwd>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Olympus cameras
    class OlympusMakerNote {
    public:
        //! Return read-only list of built-in Olympus tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in Olympus Camera Settings tags
        static const TagInfo* tagListCs();
        //! Return read-only list of built-in Olympus Equipment tags
        static const TagInfo* tagListEq();
        //! Return read-only list of built-in Olympus Raw Development tags
        static const TagInfo* tagListRd();
        //! Return read-only list of built-in Olympus Raw Development 2 tags
        static const TagInfo* tagListRd2();
        //! Return read-only list of built-in Olympus Image Processing tags
        static const TagInfo* tagListIp();
        //! Return read-only list of built-in Olympus Focus Info tags
        static const TagInfo* tagListFi();
        //! Return read-only list of built-in Olympus FE tags
        static const TagInfo* tagListFe();
        //! Return read-only list of built-in Olympus Raw Info tags
        static const TagInfo* tagListRi();

        //! @name Print functions for Olympus %MakerNote tags
        //@{
        //! Print 'Special Mode'
        static std::ostream& print0x0200(std::ostream& os, const Value& value, const ExifData*);
        //! Print Digital Zoom Factor
        static std::ostream& print0x0204(std::ostream& os, const Value& value, const ExifData*);
        //! Print White Balance Mode
        static std::ostream& print0x1015(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus equipment Lens type
        static std::ostream& print0x0201(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus CamerID
        static std::ostream& print0x0209(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus equipment Extender
        static std::ostream& printEq0x0301(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus camera settings Focus Mode
        static std::ostream& printCs0x0301(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus camera settings Gradation
        static std::ostream& print0x050f(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus camera settings Noise Filter
        static std::ostream& print0x0527(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus ArtFilter
        static  std::ostream& print0x0529(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus focus info ManualFlash
        static std::ostream& print0x1209(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus focus info AF Point
        static std::ostream& print0x0308(std::ostream& os, const Value& value, const ExifData*);
        //! Print Olympus generic
        static std::ostream& printGeneric(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Tag information
        static const TagInfo tagInfo_[];
        static const TagInfo tagInfoCs_[];
        static const TagInfo tagInfoEq_[];
        static const TagInfo tagInfoRd_[];
        static const TagInfo tagInfoRd2_[];
        static const TagInfo tagInfoIp_[];
        static const TagInfo tagInfoFi_[];
        static const TagInfo tagInfoFe_[];
        static const TagInfo tagInfoRi_[];

    }; // class OlympusMakerNote

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef OLYMPUSMN_INT_HPP_
