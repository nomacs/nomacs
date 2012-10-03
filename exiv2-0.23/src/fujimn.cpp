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
  File:      fujimn.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier dot gilles at gmail dot com>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
  Credits:   See header file.
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: fujimn.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "fujimn_int.hpp"
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

    //! OffOn, multiple tags
    extern const TagDetails fujiOffOn[] = {
        { 0, N_("Off") },
        { 1, N_("On")  }
    };

    //! Sharpness, tag 0x1001
    extern const TagDetails fujiSharpness[] = {
        { 1, N_("Soft mode 1") },
        { 2, N_("Soft mode 2") },
        { 3, N_("Normal")      },
        { 4, N_("Hard mode 1") },
        { 5, N_("Hard mode 2") }
    };

    //! WhiteBalance, tag 0x1002
    extern const TagDetails fujiWhiteBalance[] = {
        {    0, N_("Auto")                     },
        {  256, N_("Daylight")                 },
        {  512, N_("Cloudy")                   },
        {  768, N_("Fluorescent (daylight)")   },
        {  769, N_("Fluorescent (warm white)") },
        {  770, N_("Fluorescent (cool white)") },
        { 1024, N_("Incandescent")             },
        { 3480, N_("Custom")                   },
        { 3480, N_("Custom")                   } // To silence compiler warning
    };

    //! Color, tag 0x1003
    extern const TagDetails fujiColor[] = {
        {   0, N_("Normal")               },
        { 256, N_("High")                 },
        { 512, N_("Low")                  },
        { 768, N_("None (black & white)") },
        { 768, N_("None (black & white)") }     // To silence compiler warning
    };

    //! Tone, tag 0x1004
    extern const TagDetails fujiTone[] = {
        {   0, N_("Normal") },
        { 256, N_("High")   },
        { 512, N_("Low")    }
    };

    //! FlashMode, tag 0x1010
    extern const TagDetails fujiFlashMode[] = {
        { 0, N_("Auto")              },
        { 1, N_("On")                },
        { 2, N_("Off")               },
        { 3, N_("Red-eye reduction") },
        { 3, N_("Red-eye reduction") }          // To silence compiler warning
    };

    //! FocusMode, tag 0x1021
    extern const TagDetails fujiFocusMode[] = {
        { 0, N_("Auto")   },
        { 1, N_("Manual") }
    };

    //! PictureMode, tag 0x1031
    extern const TagDetails fujiPictureMode[] = {
        {   0, N_("Auto")                      },
        {   1, N_("Portrait")                  },
        {   2, N_("Landscape")                 },
        {   4, N_("Sports")                    },
        {   5, N_("Night scene")               },
        {   6, N_("Program AE")                },
        {   7, N_("Natural light")             },
        {   8, N_("Anti-blur")                 },
        {  10, N_("Sunset")                    },
        {  11, N_("Museum")                    },
        {  12, N_("Party")                     },
        {  13, N_("Flower")                    },
        {  14, N_("Text")                      },
        {  15, N_("Natural light & flash")     },
        {  16, N_("Beach")                     },
        {  17, N_("Snow")                      },
        {  18, N_("Fireworks")                 },
        {  19, N_("Underwater")                },
        { 256, N_("Aperture-priority AE")      },
        { 512, N_("Shutter speed priority AE") },
        { 768, N_("Manual")                    }
    };

    //! Continuous, tag 0x1100
    extern const TagDetails fujiContinuous[] = {
        { 0, N_("Off")              },
        { 1, N_("On")               },
        { 2, N_("No flash & flash") }
    };

    //! FinePixColor, tag 0x1210
    extern const TagDetails fujiFinePixColor[] = {
        { 0,  N_("Standard")      },
        { 16, N_("Chrome")        },
        { 48, N_("Black & white") }
    };

    //! DynamicRange, tag 0x1400
    extern const TagDetails fujiDynamicRange[] = {
        { 1, N_("Standard") },
        { 3, N_("Wide")     }
    };

    //! FilmMode, tag 0x1401
    extern const TagDetails fujiFilmMode[] = {
        {    0, N_("F0/Standard")           },
        {  256, N_("F1/Studio portrait")    },
        {  512, N_("F2/Fujichrome")         },
        {  768, N_("F3/Studio portrait Ex") },
        { 1024, N_("F4/Velvia")             }
    };

    //! DynamicRange, tag 0x1402
    extern const TagDetails fujiDynamicRangeSetting[] = {
        {     0, N_("Auto (100-400%)")      },
        {     1, N_("Raw")                  },
        {   256, N_("Standard (100%)")      },
        {   512, N_("Wide mode 1 (230%)")   },
        {   513, N_("Wide mode 2 (400%)")   },
        { 32768, N_("Film simulation mode") }
    };

    // Fujifilm MakerNote Tag Info
    const TagInfo FujiMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "Version", N_("Version"),
                N_("Fujifilm Makernote version"),
                fujiId, makerTags, undefined, -1, printValue),
        TagInfo(0x0010, "SerialNumber", N_("Serial Number"),
                N_("This number is unique, and contains the date of manufacture, "
                   "but is not the same as the number printed on the camera body."),
                fujiId, makerTags, asciiString, -1, printValue),
        TagInfo(0x1000, "Quality", N_("Quality"),
                N_("Image quality setting"),
                fujiId, makerTags, asciiString, -1, printValue),
        TagInfo(0x1001, N_("Sharpness"), N_("Sharpness"),
                N_("Sharpness setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiSharpness)),
        TagInfo(0x1002, "WhiteBalance", N_("White Balance"),
                N_("White balance setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiWhiteBalance)),
        TagInfo(0x1003, "Color", N_("Color"),
                N_("Chroma saturation setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiColor)),
        TagInfo(0x1004, "Tone", N_("Tone"),
                N_("Contrast setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiTone)),
        TagInfo(0x1010, "FlashMode", N_("Flash Mode"),
                N_("Flash firing mode setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiFlashMode)),
        TagInfo(0x1011, "FlashStrength", N_("Flash Strength"),
                N_("Flash firing strength compensation setting"),
                fujiId, makerTags, signedRational, -1, printValue),
        TagInfo(0x1020, "Macro", N_("Macro"),
                N_("Macro mode setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1021, "FocusMode", N_("Focus Mode"),
                N_("Focusing mode setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiFocusMode)),
        TagInfo(0x1022, "0x1022", "0x1022",
                N_("Unknown"),
                fujiId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x1030, "SlowSync", N_("Slow Sync"),
                N_("Slow synchro mode setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1031, "PictureMode", N_("Picture Mode"),
                N_("Picture mode setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiPictureMode)),
        TagInfo(0x1032, "0x1032", "0x1032",
                N_("Unknown"),
                fujiId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x1100, "Continuous", N_("Continuous"),
                N_("Continuous shooting or auto bracketing setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiContinuous)),
        TagInfo(0x1101, "SequenceNumber", N_("Sequence Number"),
                N_("Sequence number"),
                fujiId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x1200, "0x1200", "0x1200",
                N_("Unknown"),
                fujiId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x1210, "FinePixColor", N_("FinePix Color"),
                N_("Fuji FinePix color setting"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiFinePixColor)),
        TagInfo(0x1300, "BlurWarning", N_("Blur Warning"),
                N_("Blur warning status"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1301, "FocusWarning", N_("Focus Warning"),
                N_("Auto Focus warning status"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1302, "ExposureWarning", N_("Exposure Warning"),
                N_("Auto exposure warning status"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1400, "DynamicRange", N_("Dynamic Range"),
                N_("Dynamic range"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiDynamicRange)),
        TagInfo(0x1401, "FilmMode", N_("Film Mode"),
                N_("Film mode"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiFilmMode)),
        TagInfo(0x1402, "DynamicRangeSetting", N_("Dynamic Range Setting"),
                N_("Dynamic range settings"),
                fujiId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiDynamicRangeSetting)),
        TagInfo(0x1403, "DevelopmentDynamicRange", N_("Development Dynamic Range"),
                N_("Development dynamic range"),
                fujiId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x1404, "MinFocalLength", N_("Minimum Focal Length"),
                N_("Minimum focal length"),
                fujiId, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x1405, "MaxFocalLength", N_("Maximum Focal Length"),
                N_("Maximum focal length"),
                fujiId, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x1406, "MaxApertureAtMinFocal", N_("Maximum Aperture at Mininimum Focal"),
                N_("Maximum aperture at mininimum focal"),
                fujiId, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x1407, "MaxApertureAtMaxFocal", N_("Maximum Aperture at Maxinimum Focal"),
                N_("Maximum aperture at maxinimum focal"),
                fujiId, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x8000, "FileSource", N_("File Source"),
                N_("File source"),
                fujiId, makerTags, asciiString, -1, printValue),
        TagInfo(0x8002, "OrderNumber", N_("Order Number"),
                N_("Order number"),
                fujiId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x8003, "FrameNumber", N_("Frame Number"),
                N_("Frame number"),
                fujiId, makerTags, unsignedShort, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownFujiMakerNoteTag)", "(UnknownFujiMakerNoteTag)",
                N_("Unknown FujiMakerNote tag"),
                fujiId, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* FujiMakerNote::tagList()
    {
        return tagInfo_;
    }

}}                                      // namespace Internal, Exiv2
