// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2011 Andreas Huggel <ahuggel@gmx.net>
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
  File:      samsungmn.cpp
  Version:   $Rev: 2455 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   27-Sep-10, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: samsungmn.cpp 2455 2011-02-13 14:39:15Z ahuggel $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "samsungmn_int.hpp"
#include "tags_int.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    //! LensType, tag 0xa003
    extern const TagDetails samsung2LensType[] = {
        { 1, N_("Samsung 30mm F2")                     },
        { 2, N_("Samsung Zoom 18-55mm F3.5-5.6 OIS")   },
        { 3, N_("Samsung Zoom 50-200mm F4-5.6 ED OIS") }
    };

    std::ostream& printPictureWizard(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 5 || value.typeId() != unsignedShort) {
            return os << value;
        }
        return os <<  "Mode: " << value.toLong(0)
                  << ", Col: " << value.toLong(1)
                  << ", Sat: " << value.toLong(2) - 4
                  << ", Sha: " << value.toLong(3) - 4
                  << ", Con: " << value.toLong(4) - 4;
    }

    std::ostream& printCameraTemperature(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 1 || value.typeId() != signedRational) {
            return os << value;
        }
        return os << value.toFloat() << " C";
    }

    std::ostream& printFocalLength35(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 1 || value.typeId() != unsignedLong) {
            return os << value;
        }
        long length = value.toLong();
        if (length == 0) {
            os << _("Unknown");
        }
        else {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1) << length / 10.0 << " mm";
            os.copyfmt(oss);
        }
        return os;
    }

    // Samsung MakerNote Tag Info
    const TagInfo Samsung2MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", N_("Version"), N_("Makernote version"),
                samsung2Id, makerTags, undefined, -1, printExifVersion),
        TagInfo(0x0021, "PictureWizard", N_("PictureWizard"), N_("Picture wizard"),
                samsung2Id, makerTags, unsignedShort, -1, printPictureWizard),
        TagInfo(0x0035, "Preview", N_("Pointer to a preview image"), N_("Offset to an IFD containing a preview image"),
                samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0043, "CameraTemperature", N_("Camera Temperature"), N_("Camera temperature"),
                samsung2Id, makerTags, signedRational, -1, printCameraTemperature),
        TagInfo(0xa001, "FirmwareName", N_("Firmware Name"), N_("Firmware name"),
                samsung2Id, makerTags, asciiString, -1, printValue),
        TagInfo(0xa003, "LensType", N_("Lens Type"), N_("Lens type"),
                samsung2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(samsung2LensType)),
        TagInfo(0xa010, "SensorAreas", N_("Sensor Areas"), N_("Sensor areas"),
                samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa013, "ExposureBiasValue", N_("Exposure Bias Value"), N_("Exposure bias value"),
                samsung2Id, makerTags, signedRational, -1, print0x9204),
        TagInfo(0xa014, "ISO", N_("ISO"), N_("ISO"),
                samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa018, "ExposureTime", N_("Exposure Time"), N_("Exposure time"),
                samsung2Id, makerTags, unsignedRational, -1, print0x829a),
        TagInfo(0xa019, "FNumber", N_("FNumber"), N_("The F number."),
                samsung2Id, makerTags, unsignedRational, -1, print0x829d),
        TagInfo(0xa01a, "FocalLengthIn35mmFormat", N_("Focal Length In 35mm Format"), N_("Focal length in 35mm format"),
                samsung2Id, makerTags, unsignedLong, -1, printFocalLength35),
        TagInfo(0xa021, "WB_RGGBLevels", N_("WB RGGB Levels"), N_("WB RGGB levels"),
                samsung2Id, makerTags, unsignedLong, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownSamsung2MakerNoteTag)", "(UnknownSamsung2MakerNoteTag)",
                N_("Unknown Samsung2MakerNote tag"),
                samsung2Id, makerTags, undefined, -1, printValue)
    };

    const TagInfo* Samsung2MakerNote::tagList()
    {
        return tagInfo_;
    }

}}                                      // namespace Internal, Exiv2
