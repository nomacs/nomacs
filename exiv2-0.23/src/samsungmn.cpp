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
  File:      samsungmn.cpp
  Version:   $Rev: 2701 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   27-Sep-10, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: samsungmn.cpp 2701 2012-04-13 14:08:56Z ahuggel $")

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
        { 0, N_("Built-in")                              },
        { 1, N_("Samsung 30mm F2 Pancake")               },
        { 2, N_("Samsung Zoom 18-55mm F3.5-5.6 OIS")     },
        { 3, N_("Samsung Zoom 50-200mm F4-5.6 ED OIS")   },
        { 4, N_("Samsung 20-50mm F3.5-5.6 Compact Zoom") },
        { 5, N_("Samsung 20mm F2.8 Pancake")             },
        { 7, N_("Samsung 60mm F2.8 Macro ED OIS SSA")    },
        { 8, N_("Samsung 16mm F2.4 Ultra Wide Pancake")  }
    };

    //! ColorSpace, tag 0xa011
    extern const TagDetails samsung2ColorSpace[] = {
        { 0, N_("sRGB")      },
        { 1, N_("Adobe RGB") }
    };

    //! SmartRange, tag 0xa012
    extern const TagDetails samsung2SmartRange[] = {
        { 0, N_("Off") },
        { 1, N_("On")  }
    };

    //! Print the camera temperature
    std::ostream& printCameraTemperature(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 1 || value.typeId() != signedRational) {
            return os << value;
        }
        return os << value.toFloat() << " C";
    }

    //! Print the 35mm focal length
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
        TagInfo(0x0001, "Version", N_("Version"), N_("Makernote version"), samsung2Id, makerTags, undefined, -1, printExifVersion),
        TagInfo(0x0021, "PictureWizard", N_("Picture Wizard"), N_("Picture wizard composite tag"), samsung2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0030, "LocalLocationName", N_("Local Location Name"), N_("Local location name"), samsung2Id, makerTags, asciiString, -1, printValue),
        TagInfo(0x0031, "LocationName", N_("Location Name"), N_("Location name"), samsung2Id, makerTags, asciiString, -1, printValue),
        TagInfo(0x0035, "Preview", N_("Pointer to a preview image"), N_("Offset to an IFD containing a preview image"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0043, "CameraTemperature", N_("Camera Temperature"), N_("Camera temperature"), samsung2Id, makerTags, signedRational, -1, printCameraTemperature),
        TagInfo(0xa001, "FirmwareName", N_("Firmware Name"), N_("Firmware name"), samsung2Id, makerTags, asciiString, -1, printValue),
        TagInfo(0xa003, "LensType", N_("Lens Type"), N_("Lens type"), samsung2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(samsung2LensType)),
        TagInfo(0xa004, "LensFirmware", N_("Lens Firmware"), N_("Lens firmware"), samsung2Id, makerTags, asciiString, -1, printValue),
        TagInfo(0xa010, "SensorAreas", N_("Sensor Areas"), N_("Sensor areas"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa011, "ColorSpace", N_("Color Space"), N_("Color space"), samsung2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(samsung2ColorSpace)),
        TagInfo(0xa012, "SmartRange", N_("Smart Range"), N_("Smart range"), samsung2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(samsung2SmartRange)),
        TagInfo(0xa013, "ExposureBiasValue", N_("Exposure Bias Value"), N_("Exposure bias value"), samsung2Id, makerTags, signedRational, -1, print0x9204),
        TagInfo(0xa014, "ISO", N_("ISO"), N_("ISO"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa018, "ExposureTime", N_("Exposure Time"), N_("Exposure time"), samsung2Id, makerTags, unsignedRational, -1, print0x829a),
        TagInfo(0xa019, "FNumber", N_("FNumber"), N_("The F number."), samsung2Id, makerTags, unsignedRational, -1, print0x829d),
        TagInfo(0xa01a, "FocalLengthIn35mmFormat", N_("Focal Length In 35mm Format"), N_("Focal length in 35mm format"), samsung2Id, makerTags, unsignedLong, -1, printFocalLength35),
        TagInfo(0xa020, "EncryptionKey", N_("Encryption Key"), N_("Encryption key"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa021, "WB_RGGBLevelsUncorrected", N_("WB RGGB Levels Uncorrected"), N_("WB RGGB levels not corrected for WB_RGGBLevelsBlack"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa022, "WB_RGGBLevelsAuto", N_("WB RGGB Levels Auto"), N_("WB RGGB levels auto"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa023, "WB_RGGBLevelsIlluminator1", N_("WB RGGB Levels Illuminator1"), N_("WB RGGB levels illuminator1"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa024, "WB_RGGBLevelsIlluminator2", N_("WB RGGB Levels Illuminator2"), N_("WB RGGB levels illuminator2"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa028, "WB_RGGBLevelsBlack", N_("WB RGGB Levels Black"), N_("WB RGGB levels black"), samsung2Id, makerTags, signedLong, -1, printValue),
        TagInfo(0xa030, "ColorMatrix", N_("Color Matrix"), N_("Color matrix"), samsung2Id, makerTags, signedLong, -1, printValue),
        TagInfo(0xa031, "ColorMatrixSRGB", N_("Color Matrix sRGB"), N_("Color matrix sRGB"), samsung2Id, makerTags, signedLong, -1, printValue),
        TagInfo(0xa032, "ColorMatrixAdobeRGB", N_("Color Matrix Adobe RGB"), N_("Color matrix Adobe RGB"), samsung2Id, makerTags, signedLong, -1, printValue),
        TagInfo(0xa040, "ToneCurve1", N_("Tone Curve 1"), N_("Tone curve 1"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa041, "ToneCurve2", N_("Tone Curve 2"), N_("Tone curve 2"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa042, "ToneCurve3", N_("Tone Curve 3"), N_("Tone curve 3"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xa043, "ToneCurve4", N_("Tone Curve 4"), N_("Tone curve 4"), samsung2Id, makerTags, unsignedLong, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownSamsung2MakerNoteTag)", "(UnknownSamsung2MakerNoteTag)", N_("Unknown Samsung2MakerNote tag"), samsung2Id, makerTags, undefined, -1, printValue)
    };

    const TagInfo* Samsung2MakerNote::tagList()
    {
        return tagInfo_;
    }

    //! PictureWizard Mode
    extern const TagDetails samsungPwMode[] = {
        {  0, N_("Standard")  },
        {  1, N_("Vivid")     },
        {  2, N_("Portrait")  },
        {  3, N_("Landscape") },
        {  4, N_("Forest")    },
        {  5, N_("Retro")     },
        {  6, N_("Cool")      },
        {  7, N_("Calm")      },
        {  8, N_("Classic")   },
        {  9, N_("Custom1")   },
        { 10, N_("Custom2")   },
        { 11, N_("Custom3")   }
    };

    //! Print the tag value minus 4
    std::ostream& printValueMinus4(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 1 || value.typeId() != unsignedShort) {
            return os << value;
        }
        return os << value.toLong(0) - 4;
    }

    // Samsung PictureWizard Tag Info
    const TagInfo Samsung2MakerNote::tagInfoPw_[] = {
        TagInfo(0x0000, "Mode", N_("Mode"), N_("Mode"), samsungPwId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(samsungPwMode)),
        TagInfo(0x0001, "Color", N_("Color"), N_("Color"), samsungPwId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0002, "Saturation", N_("Saturation"), N_("Saturation"), samsungPwId, makerTags, unsignedShort, 1, printValueMinus4),
        TagInfo(0x0003, "Sharpness", N_("Sharpness"), N_("Sharpness"), samsungPwId, makerTags, unsignedShort, 1, printValueMinus4),
        TagInfo(0x0004, "Contrast", N_("Contrast"), N_("Contrast"), samsungPwId, makerTags, unsignedShort, 1, printValueMinus4),
        // End of list marker
        TagInfo(0xffff, "(UnknownSamsungPictureWizardTag)", "(UnknownSamsungPictureWizardTag)", N_("Unknown SamsungPictureWizard tag"), samsungPwId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* Samsung2MakerNote::tagListPw()
    {
        return tagInfoPw_;
    }

}}                                      // namespace Internal, Exiv2
