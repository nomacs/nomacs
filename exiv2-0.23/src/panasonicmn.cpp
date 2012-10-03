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
  File:      panasonicmn.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier dot gilles at gmail dot com>
  History:   11-Jun-04, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: panasonicmn.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "panasonicmn_int.hpp"
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

    //! Quality, tag 0x0001
    extern const TagDetails panasonicQuality[] = {
        { 2, N_("High")           },
        { 3, N_("Normal")         },
        { 6, N_("Very High")      },
        { 7, N_("Raw")            },
        { 9, N_("Motion Picture") }
    };

    //! WhiteBalance, tag 0x0003
    extern const TagDetails panasonicWhiteBalance[] = {
        {  1, N_("Auto")            },
        {  2, N_("Daylight")        },
        {  3, N_("Cloudy")          },
        {  4, N_("Halogen")         },
        {  5, N_("Manual")          },
        {  8, N_("Flash")           },
        { 10, N_("Black and white") },
        { 11, N_("Manual")          },
        { 11, N_("Manual")          }           // To silence compiler warning
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails panasonicFocusMode[] = {
        {  1, N_("Auto")               },
        {  2, N_("Manual")             },
        {  4, N_("Auto, focus button") },
        {  5, N_("Auto, continuous")   },
        {  5, N_("Auto, continuous")   }        // To silence compiler warning
    };

    //! ImageStabilizer, tag 0x001a
    extern const TagDetails panasonicImageStabilizer[] = {
        {  2, N_("On, Mode 1") },
        {  3, N_("Off")        },
        {  4, N_("On, Mode 2") }
    };

    //! Macro, tag 0x001c
    extern const TagDetails panasonicMacro[] = {
        {   1, N_("On")         },
        {   2, N_("Off")        },
        { 257, N_("Tele-macro") }
    };

    //! ShootingMode, tag 0x001f and SceneMode, tag 0x8001
    extern const TagDetails panasonicShootingMode[] = {
        {  0, N_("Off")                            }, // only SceneMode
        {  1, N_("Normal")                         },
        {  2, N_("Portrait")                       },
        {  3, N_("Scenery")                        },
        {  4, N_("Sports")                         },
        {  5, N_("Night portrait")                 },
        {  6, N_("Program")                        },
        {  7, N_("Aperture priority")              },
        {  8, N_("Shutter-speed priority")         },
        {  9, N_("Macro")                          },
        { 10, N_("Spot")                           },
        { 11, N_("Manual")                         },
        { 12, N_("Movie preview")                  },
        { 13, N_("Panning")                        },
        { 14, N_("Simple")                         },
        { 15, N_("Color effects")                  },
        { 18, N_("Fireworks")                      },
        { 19, N_("Party")                          },
        { 20, N_("Snow")                           },
        { 21, N_("Night scenery")                  },
        { 22, N_("Food")                           },
        { 23, N_("Baby")                           },
        { 24, N_("Soft skin")                      },
        { 25, N_("Candlelight")                    },
        { 26, N_("Starry night")                   },
        { 27, N_("High sensitivity")               },
        { 28, N_("Panorama assist")                },
        { 29, N_("Underwater")                     },
        { 30, N_("Beach")                          },
        { 31, N_("Aerial photo")                   },
        { 32, N_("Sunset")                         },
        { 33, N_("Pet")                            },
        { 34, N_("Intelligent ISO")                },
        { 36, N_("High speed continuous shooting") },
        { 37, N_("Intelligent auto")               },
    };

    //! Audio, tag 0x0020
    extern const TagDetails panasonicAudio[] = {
        { 1, N_("Yes") },
        { 2, N_("No")  }
    };

    //! ColorEffect, tag 0x0028
    extern const TagDetails panasonicColorEffect[] = {
        { 1, N_("Off")             },
        { 2, N_("Warm")            },
        { 3, N_("Cool")            },
        { 4, N_("Black and white") },
        { 5, N_("Sepia")           }
    };

    //! BustMode, tag 0x002a
    extern const TagDetails panasonicBurstMode[] = {
        { 0, N_("Off")              },
        { 1, N_("Low/High quality") },
        { 2, N_("Infinite")         }
    };

    //! Contrast, tag 0x002c
    extern const TagDetails panasonicContrast[] = {
        {   0, N_("Normal")      },
        {   1, N_("Low")         },
        {   2, N_("High")        },
        {   6, N_("Medium low")  },
        {   7, N_("Medium high") },
        { 256, N_("Low")         },
        { 272, N_("Standard")    },
        { 288, N_("High")        },
        { 288, N_("High")        }              // To silence compiler warning
    };

    //! NoiseReduction, tag 0x002d
    extern const TagDetails panasonicNoiseReduction[] = {
        { 0, N_("Standard") },
        { 1, N_("Low (-1)")     },
        { 2, N_("High (+1)")    },
        { 3, N_("Lowest (-2)")  },
        { 4, N_("Highest (+2)") }
    };

    //! SelfTimer, tag 0x002e
    extern const TagDetails panasonicSelfTimer[] = {
        { 1, N_("Off") },
        { 2, N_("10s") },
        { 3, N_("2s")  }
    };

    //! Rotation, tag 0x0030
    extern const TagDetails panasonicRotation[] = {
        { 1, N_("Horizontal (normal)") },
        { 6, N_("Rotate 90 CW")        },
        { 8, N_("Rotate 270 CW")       }
    };

    //! ColorMode, tag 0x0032
    extern const TagDetails panasonicColorMode[] = {
        { 0, N_("Normal")  },
        { 1, N_("Natural") },
        { 2, N_("Vivid")   }
    };

    //! OpticalZoomMode, tag 0x0034
    extern const TagDetails panasonicOpticalZoomMode[] = {
        { 1, N_("Standard")  },
        { 2, N_("EX optics") }
    };

    //! ConversionLens, tag 0x0035
    extern const TagDetails panasonicConversionLens[] = {
        { 1, N_("Off")       },
        { 2, N_("Wide")      },
        { 3, N_("Telephoto") },
        { 4, N_("Macro")     },
        { 4, N_("Macro")     }                  // To silence compiler warning
    };

    //! WorldTimeLocation, tag 0x003a
    extern const TagDetails panasonicWorldTimeLocation[] = {
        { 1, N_("Home")        },
        { 2, N_("Destination") }
    };

    //! FilmMode, tag 0x0042
    extern const TagDetails panasonicFilmMode[] = {
        { 1, N_("Standard (color)") },
        { 2, N_("Dynamic (color)")  },
        { 3, N_("Nature (color)")   },
        { 4, N_("Smooth (color)")   },
        { 5, N_("Standard (B&W)")   },
        { 6, N_("Dynamic (B&W)")    },
        { 7, N_("Smooth (B&W)")     }
    };

    // Panasonic MakerNote Tag Info
    const TagInfo PanasonicMakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Quality", N_("Quality"), N_("Image Quality"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicQuality)),
        TagInfo(0x0002, "FirmwareVersion", N_("Firmware Version"), N_("Firmware version"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x0003, "WhiteBalance", N_("White Balance"), N_("White balance setting"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicWhiteBalance)),
        TagInfo(0x0004, "0x0004", "0x0004", N_("Unknown"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0007, "FocusMode", N_("Focus Mode"), N_("Focus mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicFocusMode)),
        TagInfo(0x000f, "AFMode", N_("AF Mode"), N_("AF mode"), panasonicId, makerTags, unsignedByte, -1, print0x000f),
        TagInfo(0x001a, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicImageStabilizer)),
        TagInfo(0x001c, "Macro", N_("Macro"), N_("Macro mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicMacro)),
        TagInfo(0x001f, "ShootingMode", N_("Shooting Mode"), N_("Shooting mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicShootingMode)),
        TagInfo(0x0020, "Audio", N_("Audio"), N_("Audio"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicAudio)),
        TagInfo(0x0021, "DataDump", N_("Data Dump"), N_("Data dump"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x0022, "0x0022", "0x0022", N_("Unknown"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0023, "WhiteBalanceBias", N_("White Balance Bias"), N_("White balance adjustment"), panasonicId, makerTags, signedShort, -1, print0x0023),
        TagInfo(0x0024, "FlashBias", N_("FlashBias"), N_("Flash bias"), panasonicId, makerTags, signedShort, -1, printValue),
        TagInfo(0x0025, "InternalSerialNumber", N_("Internal Serial Number"), N_("This number is unique, and contains the date of manufacture, but is not the same as the number printed on the camera body."), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x0026, "ExifVersion", "Exif Version", N_("Exif version"), panasonicId, makerTags, undefined, -1, printExifVersion),
        TagInfo(0x0027, "0x0027", "0x0027", N_("Unknown"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0028, "ColorEffect", N_("Color Effect"), N_("Color effect"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicColorEffect)),
        TagInfo(0x0029, "TimeSincePowerOn", "Time since Power On", N_("Time in 1/100 s from when the camera was powered on to when the image is written to memory card"), panasonicId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x002a, "BurstMode", N_("Burst Mode"), N_("Burst mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicBurstMode)),
        TagInfo(0x002b, "SequenceNumber", N_("Sequence Number"), N_("Sequence number"), panasonicId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x002c, "Contrast", N_("Contrast"), N_("Contrast setting"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicContrast)),
        TagInfo(0x002d, "NoiseReduction", N_("NoiseReduction"), N_("Noise reduction"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicNoiseReduction)),
        TagInfo(0x002e, "SelfTimer", N_("Self Timer"), N_("Self timer"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicSelfTimer)),
        TagInfo(0x002f, "0x002f", "0x002f", N_("Unknown"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0030, "Rotation", N_("Rotation"), N_("Rotation"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicRotation)),
        TagInfo(0x0031, "0x0031", "0x0031", N_("Unknown"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0032, "ColorMode", N_("Color Mode"), N_("Color mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicColorMode)),
        TagInfo(0x0033, "BabyAge", N_("Baby Age"), N_("Baby (or pet) age"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0034, "OpticalZoomMode", N_("Optical Zoom Mode"), N_("Optical zoom mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicOpticalZoomMode)),
        TagInfo(0x0035, "ConversionLens", N_("Conversion Lens"), N_("Conversion lens"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicConversionLens)),
        TagInfo(0x0036, "TravelDay", N_("Travel Day"), N_("Travel day"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0039, "Contrast", N_("Contrast"), N_("Contrast"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x003a, "WorldTimeLocation", N_("World Time Location"), N_("World time location"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicWorldTimeLocation)),
        TagInfo(0x003c, "ProgramISO", N_("Program ISO"), N_("Program ISO"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0040, "Saturation", N_("Saturation"), N_("Saturation"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0041, "Sharpness", N_("Sharpness"), N_("Sharpness"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0042, "FilmMode", N_("Film Mode"), N_("Film mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicFilmMode)),
        TagInfo(0x0046, "WBAdjustAB", N_("WB Adjust AB"), N_("WB adjust AB. Positive is a shift toward blue."), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0047, "WBAdjustGM", N_("WB Adjust GM"), N_("WBAdjustGM. Positive is a shift toward green."), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0051, "LensType", N_("Lens Type"), N_("Lens type"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0052, "LensSerialNumber", N_("Lens Serial Number"), N_("Lens serial number"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0053, "AccessoryType", N_("Accessory Type"), N_("Accessory type"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x4449, "0x4449", "0x4449", N_("Unknown"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x8000, "MakerNoteVersion", N_("MakerNote Version"), N_("MakerNote version"), panasonicId, makerTags, undefined, -1, printExifVersion),
        TagInfo(0x8001, "SceneMode", N_("Scene Mode"), N_("Scene mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicShootingMode)),
        TagInfo(0x8004, "WBRedLevel", N_("WB Red Level"), N_("WB red level"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x8005, "WBGreenLevel", N_("WB Green Level"), N_("WB green level"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x8006, "WBBlueLevel", N_("WB Blue Level"), N_("WB blue level"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x8010, "BabyAge", N_("Baby Age"), N_("Baby (or pet) age"), panasonicId, makerTags, asciiString, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownPanasonicMakerNoteTag)", "(UnknownPanasonicMakerNoteTag)", N_("Unknown PanasonicMakerNote tag"), panasonicId, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* PanasonicMakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& PanasonicMakerNote::print0x000f(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        if (value.count() < 2 || value.typeId() != unsignedByte) {
            return os << value;
        }
        long l0 = value.toLong(0);
        long l1 = value.toLong(1);
        if      (l0 ==  0 && l1 ==  1) os << _("Spot mode on");
        else if (l0 ==  0 && l1 == 16) os << _("Spot mode off or 3-area (high speed)");
        else if (l0 ==  1 && l1 ==  0) os << _("Spot focussing");
        else if (l0 ==  1 && l1 ==  1) os << _("5-area");
        else if (l0 == 16 && l1 ==  0) os << _("1-area");
        else if (l0 == 16 && l1 == 16) os << _("1-area (high speed)");
        else if (l0 == 32 && l1 ==  0) os << _("3-area (auto)");
        else if (l0 == 32 && l1 ==  1) os << _("3-area (left)");
        else if (l0 == 32 && l1 ==  2) os << _("3-area (center)");
        else if (l0 == 32 && l1 ==  3) os << _("3-area (right)");
        else os << value;
        return os;
    } // PanasonicMakerNote::print0x000f

    std::ostream& PanasonicMakerNote::print0x0023(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1)
           << value.toLong() / 3 << _(" EV");
        os.copyfmt(oss);

        return os;

    } // PanasonicMakerNote::print0x0023

    // Panasonic MakerNote Tag Info
    const TagInfo PanasonicMakerNote::tagInfoRaw_[] = {
        TagInfo(0x0001, "Version", N_("Version"), N_("Panasonic raw version"), panaRawId, panaRaw, undefined, -1, printExifVersion),
        TagInfo(0x0002, "SensorWidth", N_("Sensor Width"), N_("Sensor width"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0003, "SensorHeight", N_("Sensor Height"), N_("Sensor height"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0004, "SensorTopBorder", N_("Sensor Top Border"), N_("Sensor top border"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0005, "SensorLeftBorder", N_("Sensor Left Border"), N_("Sensor left border"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0006, "ImageHeight", N_("Image Height"), N_("Image height"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0007, "ImageWidth", N_("Image Width"), N_("Image width"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0011, "RedBalance", N_("Red Balance"), N_("Red balance (found in Digilux 2 RAW images)"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0012, "BlueBalance", N_("Blue Balance"), N_("Blue balance"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0017, "ISOSpeed", N_("ISO Speed"), N_("ISO speed setting"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0024, "WBRedLevel", N_("WB Red Level"), N_("WB red level"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0025, "WBGreenLevel", N_("WB Green Level"), N_("WB green level"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0026, "WBBlueLevel", N_("WB Blue Level"), N_("WB blue level"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x002e, "PreviewImage", N_("Preview Image"), N_("Preview image"), panaRawId, panaRaw, undefined, -1, printValue),
        TagInfo(0x010f, "Make", N_("Manufacturer"), N_("The manufacturer of the recording equipment"), panaRawId, panaRaw, asciiString, -1, printValue),
        TagInfo(0x0110, "Model", N_("Model"), N_("The model name or model number of the equipment"), panaRawId, panaRaw, asciiString, -1, printValue),
        TagInfo(0x0111, "StripOffsets", N_("Strip Offsets"), N_("Strip offsets"), panaRawId, panaRaw, unsignedLong, -1, printValue),
        TagInfo(0x0112, "Orientation", N_("Orientation"), N_("Orientation"), panaRawId, panaRaw, unsignedShort, -1, print0x0112),
        TagInfo(0x0116, "RowsPerStrip", N_("Rows Per Strip"), N_("The number of rows per strip"), panaRawId, panaRaw, unsignedShort, -1, printValue),
        TagInfo(0x0117, "StripByteCounts", N_("Strip Byte Counts"), N_("Strip byte counts"), panaRawId, panaRaw, unsignedLong, -1, printValue),
        TagInfo(0x0118, "RawDataOffset", N_("Raw Data Offset"), N_("Raw data offset"), panaRawId, panaRaw, unsignedLong, -1, printValue),
        TagInfo(0x8769, "ExifTag", N_("Exif IFD Pointer"), N_("A pointer to the Exif IFD"), panaRawId, panaRaw, unsignedLong, -1, printValue),
        TagInfo(0x8825, "GPSTag", N_("GPS Info IFD Pointer"), N_("A pointer to the GPS Info IFD"), panaRawId, panaRaw, unsignedLong, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownPanasonicRawTag)", "(UnknownPanasonicRawTag)", N_("Unknown PanasonicRaw tag"), panaRawId, panaRaw, asciiString, -1, printValue)
    };

    const TagInfo* PanasonicMakerNote::tagListRaw()
    {
        return tagInfoRaw_;
    }

}}                                      // namespace Internal, Exiv2
