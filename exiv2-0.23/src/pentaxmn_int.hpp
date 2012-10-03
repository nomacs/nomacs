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
  @file    pentaxmn_int.hpp
  @brief   Pentax MakerNote implemented according to the specification
           http://www.gvsoft.homedns.org/exif/makernote-pentax-type3.html and
           based on ExifTool implementation and
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Pentax.html">Pentax Makernote list</a> by Phil Harvey<br>
  @version $Rev: 2681 $
  @author  Michal Cihar
           <a href="mailto:michal@cihar.com">michal@cihar.com</a>
  @date    27-Sep-07
 */
#ifndef PENTAXMN_INT_HPP_
#define PENTAXMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Pentaxfilm cameras
    class PentaxMakerNote {
    public:
        //! Return read-only list of built-in Pentaxfilm tags
        static const TagInfo* tagList();

        //! Print Pentax version
        static std::ostream& printPentaxVersion(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax resolution
        static std::ostream& printPentaxResolution(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax date
        static std::ostream& printPentaxDate(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax time
        static std::ostream& printPentaxTime(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax exposure
        static std::ostream& printPentaxExposure(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax F value
        static std::ostream& printPentaxFValue(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax focal length
        static std::ostream& printPentaxFocalLength(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax compensation
        static std::ostream& printPentaxCompensation(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax temperature
        static std::ostream& printPentaxTemperature(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax flash compensation
        static std::ostream& printPentaxFlashCompensation(std::ostream& os, const Value& value, const ExifData*);
        //! Print Pentax bracketing
        static std::ostream& printPentaxBracketing(std::ostream& os, const Value& value, const ExifData*);

    private:
        //! Tag information
        static const TagInfo tagInfo_[];
    }; // class PentaxMakerNote

    /*!
      @brief Print function to translate Pentax "combi-values" to a description
             by looking up a reference table.
     */
    template <int N, const TagDetails (&array)[N], int count, int ignoredcount, int ignoredcountmax>
    std::ostream& printCombiTag(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if ((value.count() != count && (value.count() < (count + ignoredcount) || value.count() > (count + ignoredcountmax))) || count > 4) {
            return printValue(os, value, metadata);
        }
        unsigned long l = 0;
        for (int c = 0; c < count; ++c) {
            if (value.toLong(c) < 0 || value.toLong(c) > 255) {
                return printValue(os, value, metadata);
            }
            l += (value.toLong(c) << ((count - c - 1) * 8));
        }
        const TagDetails* td = find(array, l);
        if (td) {
            os << exvGettext(td->label_);
        }
        else {
            os << exvGettext("Unknown") << " (0x"
               << std::setw(2 * count) << std::setfill('0')
               << std::hex << l << std::dec << ")";
        }

        return os;
    }

//! Shortcut for the printCombiTag template which requires typing the array name only once.
#define EXV_PRINT_COMBITAG(array, count, ignoredcount) printCombiTag<EXV_COUNTOF(array), array, count, ignoredcount, ignoredcount>
//! Shortcut for the printCombiTag template which requires typing the array name only once.
#define EXV_PRINT_COMBITAG_MULTI(array, count, ignoredcount, ignoredcountmax) printCombiTag<EXV_COUNTOF(array), array, count, ignoredcount, ignoredcountmax>

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef PENTAXMN_INT_HPP_
