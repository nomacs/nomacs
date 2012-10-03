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
  File:      minoltamn.cpp
  Version:   $Rev: 2681 $
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
             Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-May-06, gc: submitted
  Credits:   See header file.
 */

// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: minoltamn.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "minoltamn_int.hpp"
#include "tags_int.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    // -- Standard Minolta Makernotes tags ---------------------------------------------------------------

    //! Lookup table to translate Minolta color mode values to readable labels
    extern const TagDetails minoltaColorMode[] = {
        { 0,  N_("Natural Color")  },
        { 1,  N_("Black & White")  },
        { 2,  N_("Vivid Color")    },
        { 3,  N_("Solarization")   },
        { 4,  N_("AdobeRGB")       },
        { 5,  N_("Sepia")          },
        { 9,  N_("Natural")        },
        { 12, N_("Portrait")       },
        { 13, N_("Natural sRGB")   },
        { 14, N_("Natural+ sRGB")  },
        { 15, N_("Landscape")      },
        { 16, N_("Evening")        },
        { 17, N_("Night Scene")    },
        { 18, N_("Night Portrait") }
    };

    //! Lookup table to translate Minolta image quality values to readable labels
    extern const TagDetails minoltaImageQuality[] = {
        { 0, N_("Raw")        },
        { 1, N_("Super Fine") },
        { 2, N_("Fine")       },
        { 3, N_("Standard")   },
        { 4, N_("Economy")    },
        { 5, N_("Extra Fine") }
    };

    //! Lookup table to translate Minolta image stabilization values
    extern const TagDetails minoltaImageStabilization[] = {
        { 1, N_("Off") },
        { 5, N_("On")  }
    };

    // Minolta Tag Info
    const TagInfo MinoltaMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "Version", N_("Makernote Version"),
                N_("String 'MLT0' (not null terminated)"),
                minoltaId, makerTags, undefined, -1, printValue),
        TagInfo(0x0001, "CameraSettingsStdOld", N_("Camera Settings (Std Old)"),
                N_("Standard Camera settings (Old Camera models like D5, D7, S304, and S404)"),
                minoltaId, makerTags, undefined, -1, printValue),
        TagInfo(0x0003, "CameraSettingsStdNew", N_("Camera Settings (Std New)"),
                N_("Standard Camera settings (New Camera Models like D7u, D7i, and D7hi)"),
                minoltaId, makerTags, undefined, -1, printValue),
        TagInfo(0x0004, "CameraSettings7D", N_("Camera Settings (7D)"),
                N_("Camera Settings (for Dynax 7D model)"),
                minoltaId, makerTags, undefined, -1, printValue),
        TagInfo(0x0018, "ImageStabilizationData", N_("Image Stabilization Data"),
                N_("Image stabilization data"),
                minoltaId, makerTags, undefined, -1, printValue),

        // TODO: Implement WB Info A100 tags decoding.
        TagInfo(0x0020, "WBInfoA100", N_("WB Info A100"),
                N_("White balance information for the Sony DSLR-A100"),
                minoltaId, makerTags, undefined, -1, printValue),

        TagInfo(0x0040, "CompressedImageSize", N_("Compressed Image Size"),
                N_("Compressed image size"),
                minoltaId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0081, "Thumbnail", N_("Thumbnail"),
                N_("Jpeg thumbnail 640x480 pixels"),
                minoltaId, makerTags, undefined, -1, printValue),
        TagInfo(0x0088, "ThumbnailOffset", N_("Thumbnail Offset"),
                N_("Offset of the thumbnail"),
                minoltaId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0089, "ThumbnailLength", N_("Thumbnail Length"),
                N_("Size of the thumbnail"),
                minoltaId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0100, "SceneMode", N_("Scene Mode"),
                N_("Scene Mode"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonySceneMode),

        // TODO: for A100, use Sony table from printMinoltaSonyColorMode().
        TagInfo(0x0101, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                minoltaId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(minoltaColorMode)),

        TagInfo(0x0102, "Quality", N_("Image Quality"),
                N_("Image quality"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonyImageQuality),

        // TODO: Tag 0x0103 : quality or image size (see ExifTool doc).
        TagInfo(0x0103, "0x0103", N_("0x0103"),
                N_("0x0103"),
                minoltaId, makerTags, unsignedLong, -1, printValue),

        TagInfo(0x0104, "FlashExposureComp", N_("Flash Exposure Compensation"),
                N_("Flash exposure compensation in EV"),
                minoltaId, makerTags, signedRational, -1, print0x9204),
        TagInfo(0x0105, "Teleconverter", N_("Teleconverter Model"),
                N_("Teleconverter Model"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonyTeleconverterModel),
        TagInfo(0x0107, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                minoltaId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(minoltaImageStabilization)),
        TagInfo(0x0109, "RawAndJpgRecording", N_("RAW+JPG Recording"),
                N_("RAW and JPG files recording"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonyBoolValue),
        TagInfo(0x010a, "ZoneMatching", N_("Zone Matching"),
                N_("Zone matching"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonyZoneMatching),
        TagInfo(0x010b, "ColorTemperature", N_("Color Temperature"),
                N_("Color temperature"),
                minoltaId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x010c, "LensID", N_("Lens ID"),
                N_("Lens identifier"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonyLensID),
        TagInfo(0x0111, "ColorCompensationFilter", N_("Color Compensation Filter"),
                N_("Color Compensation Filter: negative is green, positive is magenta"),
                minoltaId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0112, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune Value"),
                minoltaId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0113, "ImageStabilizationA100", N_("Image Stabilization A100"),
                N_("Image Stabilization for the Sony DSLR-A100"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonyBoolValue),

        // TODO: implement CameraSettingsA100 tags decoding.
        TagInfo(0x0114, "CameraSettings5D", N_("Camera Settings (5D)"),
                N_("Camera Settings (for Dynax 5D model)"),
                minoltaId, makerTags, undefined, -1, printValue),

        TagInfo(0x0115, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                minoltaId, makerTags, unsignedLong, -1, printMinoltaSonyWhiteBalanceStd),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                minoltaId, makerTags, undefined, -1, printValue),
        TagInfo(0x0f00, "CameraSettingsZ1", N_("Camera Settings (Z1)"),
                N_("Camera Settings (for Z1, DImage X, and F100 models)"),
                minoltaId, makerTags, undefined, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownMinoltaMakerNoteTag)", "(UnknownMinoltaMakerNoteTag)",
                N_("Unknown Minolta MakerNote tag"),
                minoltaId, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* MinoltaMakerNote::tagList()
    {
        return tagInfo_;
    }

    // -- Standard Minolta camera settings ---------------------------------------------------------------

    //! Lookup table to translate Minolta Std camera settings exposure mode values to readable labels
    extern const TagDetails minoltaExposureModeStd[] = {
        { 0, N_("Program")           },
        { 1, N_("Aperture priority") },
        { 2, N_("Shutter priority")  },
        { 3, N_("Manual")            }
    };

    //! Lookup table to translate Minolta Std camera settings flash mode values to readable labels
    extern const TagDetails minoltaFlashModeStd[] = {
        { 0, N_("Fill flash")        },
        { 1, N_("Red-eye reduction") },
        { 2, N_("Rear flash sync")   },
        { 3, N_("Wireless")          },
        { 4, N_("Off")               }
    };

    //! Lookup table to translate Minolta Std camera settings white balance values to readable labels
    extern const TagDetails minoltaWhiteBalanceStd[] = {
        { 0,  N_("Auto")          },
        { 1,  N_("Daylight")      },
        { 2,  N_("Cloudy")        },
        { 3,  N_("Tungsten")      },
        { 5,  N_("Custom")        },
        { 7,  N_("Fluorescent")   },
        { 8,  N_("Fluorescent 2") },
        { 11, N_("Custom 2")      },
        { 12, N_("Custom 3")      }
    };

    //! Lookup table to translate Minolta Std camera settings image size values to readable labels
    extern const TagDetails minoltaImageSizeStd[] = {
        { 0, N_("Full size") },
        { 1, "1600x1200"     },
        { 2, "1280x960"      },
        { 3, "640x480"       },
        { 6, "2080x1560"     },
        { 7, "2560x1920"     },
        { 8, "3264x2176"     }
    };

    //! Lookup table to translate Minolta Std camera settings image quality values to readable labels
    extern const TagDetails minoltaImageQualityStd[] = {
        { 0, N_("Raw")        },
        { 1, N_("Super fine") },
        { 2, N_("Fine")       },
        { 3, N_("Standard")   },
        { 4, N_("Economy")    },
        { 5, N_("Extra fine") }
    };

    //! Lookup table to translate Minolta Std camera settings drive mode values to readable labels
    extern const TagDetails minoltaDriveModeStd[] = {
        { 0, N_("Single Frame")   },
        { 1, N_("Continuous")     },
        { 2, N_("Self-timer")     },
        { 4, N_("Bracketing")     },
        { 5, N_("Interval")       },
        { 6, N_("UHS continuous") },
        { 7, N_("HS continuous")  }
    };

    //! Lookup table to translate Minolta Std camera settings metering mode values to readable labels
    extern const TagDetails minoltaMeteringModeStd[] = {
        { 0, N_("Multi-segment")           },
        { 1, N_("Center weighted average") },
        { 2, N_("Spot")                    }
    };

    //! Lookup table to translate Minolta Std camera settings digital zoom values to readable labels
    extern const TagDetails minoltaDigitalZoomStd[] = {
        { 0, N_("Off")                      },
        { 1, N_("Electronic magnification") },
        { 2, "2x"                           }
    };

    //! Lookup table to translate Minolta Std camera bracket step mode values to readable labels
    extern const TagDetails minoltaBracketStepStd[] = {
        { 0, "1/3 EV" },
        { 1, "2/3 EV" },
        { 2, "1 EV"   }
    };

    //! Lookup table to translate Minolta Std camera settings AF points values to readable labels
    extern const TagDetails minoltaAFPointsStd[] = {
        { 0, N_("Center")       },
        { 1, N_("Top")          },
        { 2, N_("Top-right")    },
        { 3, N_("Right")        },
        { 4, N_("Bottom-right") },
        { 5, N_("Bottom")       },
        { 6, N_("Bottom-left")  },
        { 7, N_("Left")         },
        { 8, N_("Top-left")     }
    };

    //! Lookup table to translate Minolta Std camera settings flash fired values to readable labels
    extern const TagDetails minoltaFlashFired[] = {
        { 0, N_("Did not fire") },
        { 1, N_("Fired")        }
    };

    //! Lookup table to translate Minolta Std camera settings sharpness values to readable labels
    extern const TagDetails minoltaSharpnessStd[] = {
        { 0, N_("Hard")   },
        { 1, N_("Normal") },
        { 2, N_("Soft")   }
    };

    //! Lookup table to translate Minolta Std camera settings subject program values to readable labels
    extern const TagDetails minoltaSubjectProgramStd[] = {
        { 0, N_("None")           },
        { 1, N_("Portrait")       },
        { 2, N_("Text")           },
        { 3, N_("Night portrait") },
        { 4, N_("Sunset")         },
        { 5, N_("Sports action")  }
    };

    //! Lookup table to translate Minolta Std camera settings ISO settings values to readable labels
    extern const TagDetails minoltaISOSettingStd[] = {
        { 0, "100"      },
        { 1, "200"      },
        { 2, "400"      },
        { 3, "800"      },
        { 4, N_("Auto") },
        { 5, "64"       }
    };

    //! Lookup table to translate Minolta Std camera settings model values to readable labels
    extern const TagDetails minoltaModelStd[] = {
        { 0, "DiMAGE 7 | X1 | X21 | X31" },
        { 1, "DiMAGE 5"                  },
        { 2, "DiMAGE S304"               },
        { 3, "DiMAGE S404"               },
        { 4, "DiMAGE 7i"                 },
        { 5, "DiMAGE 7Hi"                },
        { 6, "DiMAGE A1"                 },
        { 7, "DiMAGE A2 | S414"          },
        { 7, "DiMAGE A2 | S414"          }      // To silence compiler warning
    };

    //! Lookup table to translate Minolta Std camera settings interval mode values to readable labels
    extern const TagDetails minoltaIntervalModeStd[] = {
        { 0, N_("Still image")      },
        { 1, N_("Time-lapse movie") }
    };

    //! Lookup table to translate Minolta Std camera settings folder name values to readable labels
    extern const TagDetails minoltaFolderNameStd[] = {
        { 0, N_("Standard form") },
        { 1, N_("Data form")     }
    };

    //! Lookup table to translate Minolta Std camera settings color mode values to readable labels
    extern const TagDetails minoltaColorModeStd[] = {
        { 0, N_("Natural color")   },
        { 1, N_("Black and white") },
        { 2, N_("Vivid color")     },
        { 3, N_("Solarization")    },
        { 4, N_("Adobe RGB")       }
    };

    //! Lookup table to translate Minolta Std camera settings wide focus zone values to readable labels
    extern const TagDetails minoltaWideFocusZoneStd[] = {
        { 0, N_("No zone")                              },
        { 1, N_("Center zone (horizontal orientation)") },
        { 1, N_("Center zone (vertical orientation)")   },
        { 1, N_("Left zone")                            },
        { 4, N_("Right zone")                           }
    };

    //! Lookup table to translate Minolta Std camera settings focus mode values to readable labels
    extern const TagDetails minoltaFocusModeStd[] = {
        { 0, N_("Auto focus")   },
        { 1, N_("Manual focus") }
    };

    //! Lookup table to translate Minolta Std camera settings focus area values to readable labels
    extern const TagDetails minoltaFocusAreaStd[] = {
        { 0, N_("Wide focus (normal)") },
        { 1, N_("Spot focus")          }
    };

    //! Lookup table to translate Minolta Std camera settings DEC switch position values to readable labels
    extern const TagDetails minoltaDECPositionStd[] = {
        { 0, N_("Exposure")   },
        { 1, N_("Contrast")   },
        { 2, N_("Saturation") },
        { 3, N_("Filter")     }
    };

    //! Lookup table to translate Minolta Std camera settings color profile values to readable labels
    extern const TagDetails minoltaColorProfileStd[] = {
        { 0, N_("Not embedded") },
        { 1, N_("Embedded")     }
    };

    //! Lookup table to translate Minolta Std camera settings data Imprint values to readable labels
    extern const TagDetails minoltaDataImprintStd[] = {
        { 0, N_("None")       },
        { 1, "YYYY/MM/DD"     },
        { 2, "MM/DD/HH:MM"    },
        { 3, N_("Text")       },
        { 4, N_("Text + ID#") }
    };

    //! Lookup table to translate Minolta Std camera settings flash metering values to readable labels
    extern const TagDetails minoltaFlashMeteringStd[] = {
        { 0, N_("ADI (Advanced Distance Integration)") },
        { 1, N_("Pre-flash TTl")                       },
        { 2, N_("Manual flash control")                }
    };

    std::ostream& MinoltaMakerNote::printMinoltaExposureSpeedStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-1;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaExposureTimeStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-6;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaFNumberStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-1;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaExposureCompensationStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << value.toLong()/256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaFocalLengthStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/3)-2;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaDateStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << value.toLong() / 65536 << ":" << std::right << std::setw(2) << std::setfill('0')
           << (value.toLong() - value.toLong() / 65536 * 65536) / 256 << ":"
           << std::right << std::setw(2) << std::setfill('0') << value.toLong() % 256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaTimeStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << std::right << std::setw(2) << std::setfill('0') << value.toLong() / 65536
           << ":" << std::right << std::setw(2) << std::setfill('0')
           << (value.toLong() - value.toLong() / 65536 * 65536) / 256 << ":"
           << std::right << std::setw(2) << std::setfill('0') << value.toLong() % 256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaFlashExposureCompStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()-6)/3;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaWhiteBalanceStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << value.toLong()/256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaBrightnessStd(std::ostream& os, const Value& value, const ExifData*)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-6;
        return os;
    }

    // Minolta Standard Camera Settings Tag Info (Old and New)
    const TagInfo MinoltaMakerNote::tagInfoCsStd_[] = {
        TagInfo(0x0001, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure mode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaExposureModeStd)),
        TagInfo(0x0002, "FlashMode", N_("Flash Mode"),
                N_("Flash mode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashModeStd)),
        TagInfo(0x0003, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaWhiteBalanceStd)),
        TagInfo(0x0004, "ImageSize", N_("Image Size"),
                N_("Image size"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaImageSizeStd)),
        TagInfo(0x0005, "Quality", N_("Image Quality"),
                N_("Image quality"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaImageQualityStd)),
        TagInfo(0x0006, "DriveMode", N_("Drive Mode"),
                N_("Drive mode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaDriveModeStd)),
        TagInfo(0x0007, "MeteringMode", N_("Metering Mode"),
                N_("Metering mode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaMeteringModeStd)),
        TagInfo(0x0008, "ISO", N_("ISO"),
                N_("ISO Value"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaExposureSpeedStd),
        TagInfo(0x0009, "ExposureTime", N_("Exposure Time"),
                N_("Exposure time"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaExposureTimeStd),
        TagInfo(0x000A, "FNumber", N_("FNumber"),
                N_("The F-Number"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaFNumberStd),
        TagInfo(0x000B, "MacroMode", N_("Macro Mode"),
                N_("Macro mode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaSonyBoolValue),
        TagInfo(0x000C, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaDigitalZoomStd)),
        TagInfo(0x000D, "ExposureCompensation", N_("Exposure Compensation"),
                N_("Exposure compensation"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaExposureCompensationStd),
        TagInfo(0x000E, "BracketStep", N_("Bracket Step"),
                N_("Bracket step"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaBracketStepStd)),
        TagInfo(0x0010, "IntervalLength", N_("Interval Length"),
                N_("Interval length"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x0011, "IntervalNumber", N_("Interval Number"),
                N_("Interval number"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x0012, "FocalLength", N_("Focal Length"),
                N_("Focal length"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaFocalLengthStd),
        TagInfo(0x0013, "FocusDistance", N_("Focus Distance"),
                N_("Focus distance"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x0014, "FlashFired", N_("Flash Fired"),
                N_("Flash fired"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashFired)),
        TagInfo(0x0015, "MinoltaDate", N_("Minolta Date"),
                N_("Minolta date"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaDateStd),
        TagInfo(0x0016, "MinoltaTime", N_("Minolta Time"),
                N_("Minolta time"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaTimeStd),
        TagInfo(0x0017, "MaxAperture", N_("Max Aperture"),
                N_("Max aperture"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x001A, "FileNumberMemory", N_("File Number Memory"),
                N_("File number memory"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaSonyBoolValue),
        TagInfo(0x001B, "LastFileNumber", N_("Last Image Number"),
                N_("Last image number"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x001C, "ColorBalanceRed", N_("Color Balance Red"),
                N_("Color balance red"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaWhiteBalanceStd),
        TagInfo(0x001D, "ColorBalanceGreen", N_("Color Balance Green"),
                N_("Color balance green"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaWhiteBalanceStd),
        TagInfo(0x001E, "ColorBalanceBlue", N_("Color Balance Blue"),
                N_("Color balance blue"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaWhiteBalanceStd),
        TagInfo(0x001F, "Saturation", N_("Saturation"),
                N_("Saturation"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x0020, "Contrast", N_("Contrast"),
                N_("Contrast"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x0021, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaSharpnessStd)),
        TagInfo(0x0022, "SubjectProgram", N_("Subject Program"),
                N_("Subject program"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaSubjectProgramStd)),
        TagInfo(0x0023, "FlashExposureComp", N_("Flash Exposure Compensation"),
                N_("Flash exposure compensation in EV"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaFlashExposureCompStd),
        TagInfo(0x0024, "ISOSetting", N_("ISO Settings"),
                N_("ISO setting"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaISOSettingStd)),
        TagInfo(0x0025, "MinoltaModel", N_("Minolta Model"),
                N_("Minolta model"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaModelStd)),
        TagInfo(0x0026, "IntervalMode", N_("Interval Mode"),
                N_("Interval mode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaIntervalModeStd)),
        TagInfo(0x0027, "FolderName", N_("Folder Name"),
                N_("Folder name"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFolderNameStd)),
        TagInfo(0x0028, "ColorMode", N_("ColorMode"),
                N_("ColorMode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaColorModeStd)),
        TagInfo(0x0029, "ColorFilter", N_("Color Filter"),
                N_("Color filter"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x002A, "BWFilter", N_("Black and White Filter"),
                N_("Black and white filter"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x002B, "Internal Flash", N_("Internal Flash"),
                N_("Internal Flash"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashFired)),
        TagInfo(0x002C, "Brightness", N_("Brightness"),
                N_("Brightness"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printMinoltaBrightnessStd),
        TagInfo(0x002D, "SpotFocusPointX", N_("Spot Focus Point X"),
                N_("Spot focus point X"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x002E, "SpotFocusPointY", N_("Spot Focus Point Y"),
                N_("Spot focus point Y"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x002F, "WideFocusZone", N_("Wide Focus Zone"),
                N_("Wide focus zone"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaWideFocusZoneStd)),
        TagInfo(0x0030, "FocusMode", N_("Focus Mode"),
                N_("Focus mode"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFocusModeStd)),
        TagInfo(0x0031, "FocusArea", N_("Focus area"),
                N_("Focus area"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFocusAreaStd)),
        TagInfo(0x0032, "DECPosition", N_("DEC Switch Position"),
                N_("DEC switch position"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaDECPositionStd)),
        TagInfo(0x0033, "ColorProfile", N_("Color Profile"),
                N_("Color profile"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaColorProfileStd)),
        TagInfo(0x0034, "DataImprint", N_("Data Imprint"),
                N_("Data Imprint"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaDataImprintStd)),
        TagInfo(0x003F, "FlashMetering", N_("Flash Metering"),
                N_("Flash metering"),
                minoltaCsNewId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashMeteringStd)),
        // End of list marker
        TagInfo(0xffff, "(UnknownMinoltaCsStdTag)", "(UnknownMinoltaCsStdTag)",
                N_("Unknown Minolta Camera Settings tag"),
                minoltaCsNewId, makerTags, unsignedLong, 1, printValue)
    };

    const TagInfo* MinoltaMakerNote::tagListCsStd()
    {
        return tagInfoCsStd_;
    }

    // -- Minolta Dynax 7D camera settings ---------------------------------------------------------------

    //! Lookup table to translate Minolta Dynax 7D camera settings exposure mode values to readable labels
    extern const TagDetails minoltaExposureMode7D[] = {
        { 0, N_("Program")           },
        { 1, N_("Aperture priority") },
        { 2, N_("Shutter priority")  },
        { 3, N_("Manual")            },
        { 4, N_("Auto")              },
        { 5, N_("Program-shift A")   },
        { 6, N_("Program-shift S")   }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings image size values to readable labels
    extern const TagDetails minoltaImageSize7D[] = {
        { 0, N_("Large")  },
        { 1, N_("Medium") },
        { 2, N_("Small")  }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings image quality values to readable labels
    extern const TagDetails minoltaImageQuality7D[] = {
        { 0,  N_("Raw")      },
        { 16, N_("Fine")     },
        { 32, N_("Normal")   },
        { 34, N_("Raw+Jpeg") },
        { 48, N_("Economy")  }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings white balance values to readable labels
    extern const TagDetails minoltaWhiteBalance7D[] = {
        { 0,   N_("Auto")        },
        { 1,   N_("Daylight")    },
        { 2,   N_("Shade")       },
        { 3,   N_("Cloudy")      },
        { 4,   N_("Tungsten")    },
        { 5,   N_("Fluorescent") },
        { 256, N_("Kelvin")      },
        { 512, N_("Manual")      },
        { 512, N_("Manual")      }              // To silence compiler warning
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings focus mode values to readable labels
    extern const TagDetails minoltaFocusMode7D[] = {
        { 0, N_("Single-shot AF") },
        { 1, N_("Continuous AF")  },
        { 3, N_("Manual")         },
        { 4, N_("Automatic AF")   }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings AF points values to readable labels
    extern const TagDetails minoltaAFPoints7D[] = {
        { 1,   N_("Center")       },
        { 2,   N_("Top")          },
        { 4,   N_("Top-right")    },
        { 8,   N_("Right")        },
        { 16,  N_("Bottom-right") },
        { 32,  N_("Bottom")       },
        { 64,  N_("Bottom-left")  },
        { 128, N_("Left")         },
        { 256, N_("Top-left")     }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings ISO settings values to readable labels
    extern const TagDetails minoltaISOSetting7D[] = {
        { 0, N_("Auto") },
        { 1, "100"      },
        { 3, "200"      },
        { 4, "400"      },
        { 5, "800"      },
        { 6, "1600"     },
        { 7, "3200"     }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings color space values to readable labels
    extern const TagDetails minoltaColorSpace7D[] = {
        { 0, N_("sRGB (Natural)")  },
        { 1, N_("sRGB (Natural+)") },
        { 4, N_("Adobe RGB")       }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings rotation values to readable labels
    extern const TagDetails minoltaRotation7D[] = {
        { 72, N_("Horizontal (normal)") },
        { 76, N_("Rotate 90 CW")        },
        { 82, N_("Rotate 270 CW")       }
    };

    // Minolta Dynax 7D Camera Settings Tag Info
    const TagInfo MinoltaMakerNote::tagInfoCs7D_[] = {
        TagInfo(0x0000, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure mode"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaExposureMode7D)),
        TagInfo(0x0002, "ImageSize", N_("Image Size"),
                N_("Image size"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaImageSize7D)),
        TagInfo(0x0003, "Quality", N_("Image Quality"),
                N_("Image quality"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaImageQuality7D)),
        TagInfo(0x0004, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaWhiteBalance7D)),
        TagInfo(0x000E, "FocusMode", N_("Focus Mode"),
                N_("Focus mode"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaFocusMode7D)),
        TagInfo(0x0010, "AFPoints", N_("AF Points"),
                N_("AF points"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaAFPoints7D)),
        TagInfo(0x0015, "FlashFired", N_("Flash Fired"),
                N_("Flash fired"),
                minoltaCs7DId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashFired)),
        TagInfo(0x0016, "FlashMode", N_("Flash Mode"),
                N_("Flash mode"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001C, "ISOSpeed", N_("ISO Speed Mode"),
                N_("ISO speed setting"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaISOSetting7D)),
        TagInfo(0x001E, "ExposureCompensation", N_("Exposure Compensation"),
                N_("Exposure compensation"),
                minoltaCs7DId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0025, "ColorSpace", N_("Color Space"),
                N_("Color space"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaColorSpace7D)),
        TagInfo(0x0026, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0027, "Contrast", N_("Contrast"),
                N_("Contrast"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0028, "Saturation", N_("Saturation"),
                N_("Saturation"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x002D, "FreeMemoryCardImages", N_("Free Memory Card Images"),
                N_("Free memory card images"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x003F, "ColorTemperature", N_("Color Temperature"),
                N_("Color temperature"),
                minoltaCs7DId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0040, "Hue", N_("Hue"), N_("Hue"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0046, "Rotation", N_("Rotation"),
                N_("Rotation"),
                minoltaCs7DId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaRotation7D)),
        TagInfo(0x0047, "FNumber", N_("FNumber"),
                N_("The F-Number"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0048, "ExposureTime", N_("Exposure Time"),
                N_("Exposure time"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        // 0x004A is a dupplicate than 0x002D.
        TagInfo(0x004A, "FreeMemoryCardImages", N_("Free Memory Card Images"),
                N_("Free memory card images"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x005E, "ImageNumber", N_("Image Number"),
                N_("Image number"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0060, "NoiseReduction", N_("Noise Reduction"),
                N_("Noise reduction"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        // 0x0062 is a dupplicate than 0x005E.
        TagInfo(0x0062, "ImageNumber", N_("Image Number"),
                N_("Image number"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0071, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x0075, "ZoneMatchingOn", N_("Zone Matching On"),
                N_("Zone matching on"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownMinoltaCs7DTag)", "(UnknownMinoltaCs7DTag)",
                N_("Unknown Minolta Camera Settings 7D tag"),
                minoltaCs7DId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* MinoltaMakerNote::tagListCs7D()
    {
        return tagInfoCs7D_;
    }

    // -- Minolta Dynax 5D camera settings ---------------------------------------------------------------

    //! Lookup table to translate Minolta Dynax 5D camera settings exposure mode values to readable labels
    extern const TagDetails minoltaExposureMode5D[] = {
        { 0,      N_("Program")             },
        { 1,      N_("Aperture priority")   },
        { 2,      N_("Shutter priority")    },
        { 3,      N_("Manual")              },
        { 4,      N_("Auto")                },
        { 5,      N_("Program Shift A")     },
        { 6,      N_("Program Shift S")     },
        { 0x1013, N_("Portrait")            },
        { 0x1023, N_("Sports")              },
        { 0x1033, N_("Sunset")              },
        { 0x1043, N_("Night View/Portrait") },
        { 0x1053, N_("Landscape")           },
        { 0x1083, N_("Macro")               }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings image size values to readable labels
    extern const TagDetails minoltaImageSize5D[] = {
        { 0, N_("Large")  },
        { 1, N_("Medium") },
        { 2, N_("Small")  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings image quality values to readable labels
    extern const TagDetails minoltaImageQuality5D[] = {
        { 0,  N_("Raw")      },
        { 16, N_("Fine")     },
        { 32, N_("Normal")   },
        { 34, N_("Raw+Jpeg") },
        { 48, N_("Economy")  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings white balance values to readable labels
    extern const TagDetails minoltaWhiteBalance5D[] = {
        { 0,   N_("Auto")        },
        { 1,   N_("Daylight")    },
        { 2,   N_("Cloudy")      },
        { 3,   N_("Shade")       },
        { 4,   N_("Tungsten")    },
        { 5,   N_("Fluorescent") },
        { 6,   N_("Flash")       },
        { 256, N_("Kelvin")      },
        { 512, N_("Manual")      }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings metering mode values to readable labels
    extern const TagDetails minoltaMeteringMode5D[] = {
        { 0, N_("Multi-segment")   },
        { 1, N_("Center weighted") },
        { 2, N_("Spot")            }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings ISO settings values to readable labels
    extern const TagDetails minoltaISOSetting5D[] = {
        { 0,  N_("Auto")                     },
        { 1,  "100"                          },
        { 3,  "200"                          },
        { 4,  "400"                          },
        { 5,  "800"                          },
        { 6,  "1600"                         },
        { 7,  "3200"                         },
        { 8,  N_("200 (Zone Matching High)") },
        { 10, N_("80 (Zone Matching Low)")   }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings color space values to readable labels
    extern const TagDetails minoltaColorSpace5D[] = {
        { 0, N_("sRGB (Natural)")  },
        { 1, N_("sRGB (Natural+)") },
        { 2, N_("Monochrome")      },
        { 3, N_("Adobe RGB (ICC)") },
        { 4, N_("Adobe RGB")       }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings rotation values to readable labels
    extern const TagDetails minoltaRotation5D[] = {
        { 72, N_("Horizontal (normal)") },
        { 76, N_("Rotate 90 CW")        },
        { 82, N_("Rotate 270 CW")       }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings focus position values to readable labels
    extern const TagDetails minoltaFocusPosition5D[] = {
        { 0, N_("Wide")       },
        { 1, N_("Central")    },
        { 2, N_("Up")         },
        { 3, N_("Up right")   },
        { 4, N_("Right")      },
        { 5, N_("Down right") },
        { 6, N_("Down")       },
        { 7, N_("Down left")  },
        { 8, N_("Left")       },
        { 9, N_("Up left")    }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings focus area values to readable labels
    extern const TagDetails minoltaFocusArea5D[] = {
        { 0, N_("Wide")      },
        { 1, N_("Selection") },
        { 2, N_("Spot")      }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings focus mode values to readable labels
    extern const TagDetails minoltaAFMode5D[] = {
        { 0, "AF-A" },
        { 1, "AF-S" },
        { 2, "AF-D" },
        { 3, "DMF"  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings picture finish values to readable labels
    extern const TagDetails minoltaPictureFinish5D[] = {
        { 0, N_("Natural")         },
        { 1, N_("Natural+")        },
        { 2, N_("Portrait")        },
        { 3, N_("Wind Scene")      },
        { 4, N_("Evening Scene")   },
        { 5, N_("Night Scene")     },
        { 6, N_("Night Portrait")  },
        { 7, N_("Monochrome")      },
        { 8, N_("Adobe RGB")       },
        { 9, N_("Adobe RGB (ICC)") }
    };

    //! Method to convert Minolta Dynax 5D exposure manual bias values.
    std::ostream& MinoltaMakerNote::printMinoltaExposureManualBias5D(std::ostream& os, const Value& value, const ExifData*)
    {
        // From Xavier Raynaud: the value is converted from 0:256 to -5.33:5.33

        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2)
           << (float (value.toLong()-128)/24);
        os.copyfmt(oss);
        return os;
    }

    //! Method to convert Minolta Dynax 5D exposure compensation values.
    std::ostream& MinoltaMakerNote::printMinoltaExposureCompensation5D(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2)
           << (float (value.toLong()-300)/100);
        os.copyfmt(oss);
        return os;
    }

    // Minolta Dynax 5D Camera Settings Tag Info
    const TagInfo MinoltaMakerNote::tagInfoCs5D_[] = {
        TagInfo(0x000A, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure mode"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaExposureMode5D)),
        TagInfo(0x000C, "ImageSize", N_("Image Size"),
                N_("Image size"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaImageSize5D)),
        TagInfo(0x000D, "Quality", N_("Image Quality"),
                N_("Image quality"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaImageQuality5D)),
        TagInfo(0x000E, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaWhiteBalance5D)),
        TagInfo(0x001A, "FocusPosition", N_("Focus Position"),
                N_("Focus position"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaFocusPosition5D)),
        TagInfo(0x001B, "FocusArea", N_("Focus Area"),
                N_("Focus area"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaFocusArea5D)),
        TagInfo(0x001F, "FlashFired", N_("Flash Fired"),
                N_("Flash fired"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaFlashFired)),
        TagInfo(0x0025, "MeteringMode", N_("Metering Mode"),
                N_("Metering mode"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaMeteringMode5D)),
        TagInfo(0x0026, "ISOSpeed", N_("ISO Speed Mode"),
                N_("ISO speed setting"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaISOSetting5D)),
        TagInfo(0x002F, "ColorSpace", N_("Color Space"),
                N_("Color space"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaColorSpace5D)),
        TagInfo(0x0030, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0031, "Contrast", N_("Contrast"),
                N_("Contrast"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0032, "Saturation", N_("Saturation"),
                N_("Saturation"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0035, "ExposureTime", N_("Exposure Time"),
                N_("Exposure time"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0036, "FNumber", N_("FNumber"),
                N_("The F-Number"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0037, "FreeMemoryCardImages", N_("Free Memory Card Images"),
                N_("Free memory card images"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0038, "ExposureRevision", N_("Exposure Revision"),
                N_("Exposure revision"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0048, "FocusMode", N_("Focus Mode"),
                N_("Focus mode"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaFocusModeStd)),
        TagInfo(0x0049, "ColorTemperature", N_("Color Temperature"),
                N_("Color temperature"),
                minoltaCs5DId, makerTags, signedShort, -1, printValue),
        TagInfo(0x0050, "Rotation", N_("Rotation"),
                N_("Rotation"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaRotation5D)),
        TagInfo(0x0053, "ExposureCompensation", N_("Exposure Compensation"),
                N_("Exposure compensation"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printMinoltaExposureCompensation5D),
        TagInfo(0x0054, "FreeMemoryCardImages", N_("Free Memory Card Images"),
                N_("Free memory card images"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0065, "Rotation2", N_("Rotation2"),
                N_("Rotation2"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printMinoltaSonyRotation),
        TagInfo(0x006E, "Color Temperature", N_("Color Temperature"),
                N_("Color Temperature"),
                minoltaCs5DId, makerTags, signedShort, -1, printValue),
        TagInfo(0x0071, "PictureFinish", N_("Picture Finish"),
                N_("Picture Finish"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaPictureFinish5D)),
        TagInfo(0x0091, "ExposureManualBias", N_("Exposure Manual Bias"),
                N_("Exposure manual bias"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printMinoltaExposureManualBias5D),
        TagInfo(0x009E, "AFMode", N_("AF Mode"),
                N_("AF mode"),
                minoltaCs5DId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(minoltaAFMode5D)),
        TagInfo(0x00AE, "ImageNumber", N_("Image Number"),
                N_("Image number"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00B0, "NoiseReduction", N_("Noise Reduction"),
                N_("Noise reduction"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printMinoltaSonyBoolValue),
        TagInfo(0x00BD, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                minoltaCs5DId, makerTags, unsignedShort, -1, printMinoltaSonyBoolValue),

        // From Xavier Raynaud: some notes on missing tags.
        // 0x0051 seems to be identical to FNumber (0x0036). An approx. relation between Tag value
        // and Fstop is exp(-0.335+value*0.043)
        // 0x0052 seems to be identical to ExposureTime (0x0035). An approx. relation between Tag
        // value and Exposure time is exp(-4+value*0.085)

        // End of list marker
        TagInfo(0xFFFF, "(UnknownMinoltaCs5DTag)", "(UnknownMinoltaCs5DTag)",
                N_("Unknown Minolta Camera Settings 5D tag"),
                minoltaCs5DId, makerTags, invalidTypeId, -1, printValue)
    };

    const TagInfo* MinoltaMakerNote::tagListCs5D()
    {
        return tagInfoCs5D_;
    }

    // -- Sony A100 camera settings ---------------------------------------------------------------

    //! Lookup table to translate Sony A100 camera settings drive mode 2 values to readable labels
    extern const TagDetails sonyDriveMode2A100[] = {
        { 0,    N_("Self-timer 10 sec")             },
        { 1,    N_("Continuous")                    },
        { 4,    N_("Self-timer 2 sec")              },
        { 5,    N_("Single Frame")                  },
        { 8,    N_("White Balance Bracketing Low")  },
        { 9,    N_("White Balance Bracketing High") },
        { 770,  N_("Single-frame Bracketing Low")   },
        { 771,  N_("Continuous Bracketing Low")     },
        { 1794, N_("Single-frame Bracketing High")  },
        { 1795, N_("Continuous Bracketing High")    }
    };

    //! Lookup table to translate Sony A100 camera settings focus mode values to readable labels
    extern const TagDetails sonyFocusModeA100[] = {
        { 0, "AF-S"   },
        { 1, "AF-C"   },
        { 4, "AF-A"   },
        { 5, "Manual" },
        { 6, "DMF"    }
    };

    //! Lookup table to translate Sony A100 camera settings flash mode values to readable labels
    extern const TagDetails sonyFlashModeA100[] = {
        { 0, N_("Auto")            },
        { 2, N_("Rear flash sync") },
        { 3, N_("Wireless")        },
        { 4, N_("Fill flash")      }
    };

    //! Lookup table to translate Sony A100 camera settings metering mode values to readable labels
    extern const TagDetails sonyMeteringModeA100[] = {
        { 0, N_("Multi-segment")           },
        { 1, N_("Center weighted average") },
        { 2, N_("Spot")                    }
    };

    //! Lookup table to translate Sony A100 camera settings zone matching mode values to readable labels
    extern const TagDetails sonyZoneMatchingModeA100[] = {
        { 0,    N_("Off")      },
        { 1,    N_("Standard") },
        { 2,    N_("Advanced") }
    };

    //! Lookup table to translate Sony A100 camera settings color space values to readable labels

    extern const TagDetails sonyColorSpaceA100[] = {
        { 0, N_("sRGB")      },
        { 5, N_("Adobe RGB") }
    };

    //! Lookup table to translate Sony A100 camera settings drive mode values to readable labels
    extern const TagDetails sonyDriveModeA100[] = {
        { 0, N_("Single Frame")             },
        { 1, N_("Continuous")               },
        { 2, N_("Self-timer")               },
        { 3, N_("Continuous Bracketing")    },
        { 4, N_("Single-Frame Bracketing")  },
        { 5, N_("White Balance Bracketing") }
    };

    //! Lookup table to translate Sony A100 camera settings self timer time values to readable labels
    extern const TagDetails sonySelfTimerTimeA100[] = {
        { 0, "10s" },
        { 4, "2s"  }
    };

    //! Lookup table to translate Sony A100 camera settings continuous bracketing values to readable labels
    extern const TagDetails sonyContinuousBracketingA100[] = {
        { 0x303, N_("Low")  },
        { 0x703, N_("High") }
    };

    //! Lookup table to translate Sony A100 camera settings single frame bracketing values to readable labels
    extern const TagDetails sonySingleFrameBracketingA100[] = {
        { 0x302, N_("Low")  },
        { 0x702, N_("High") }
    };

    //! Lookup table to translate Sony A100 camera settings white balance bracketing values to readable labels
    extern const TagDetails sonyWhiteBalanceBracketingA100[] = {
        { 0x8, N_("Low")  },
        { 0x9, N_("High") }
    };

    //! Lookup table to translate Sony A100 camera settings white balance setting values to readable labels
    extern const TagDetails sonyWhiteBalanceSettingA100[] = {
        { 0x0000, N_("Auto")                           },
        { 0x0001, N_("Preset")                         },
        { 0x0002, N_("Custom")                         },
        { 0x0003, N_("Color Temperature/Color Filter") },
        { 0x8001, N_("Preset")                         },
        { 0x8002, N_("Custom")                         },
        { 0x8003, N_("Color Temperature/Color Filter") }
    };

    //! Lookup table to translate Sony A100 camera settings preset white balance values to readable labels
    extern const TagDetails sonyPresetWhiteBalanceA100[] = {
        { 1, N_("Daylight")    },
        { 2, N_("Cloudy")      },
        { 3, N_("Shade")       },
        { 4, N_("Tungsten")    },
        { 5, N_("Fluorescent") },
        { 6, N_("Flash")       }
    };

    //! Lookup table to translate Sony A100 camera settings color temperature setting values to readable labels
    extern const TagDetails sonyColorTemperatureSettingA100[] = {
        { 0, N_("Temperature")  },
        { 2, N_("Color Filter") }
    };

    //! Lookup table to translate Sony A100 camera settings custom WB setting values to readable labels
    extern const TagDetails sonyCustomWBSettingA100[] = {
        { 0, N_("Setup")  },
        { 2, N_("Recall") }
    };

    //! Lookup table to translate Sony A100 camera settings custom WB error values to readable labels
    extern const TagDetails sonyCustomWBErrorA100[] = {
        { 0, N_("Ok")    },
        { 2, N_("Error") }
    };

    //! Lookup table to translate Sony A100 camera settings image size values to readable labels
    extern const TagDetails sonyImageSizeA100[] = {
        { 0, N_("Standard") },
        { 1, N_("Medium")   },
        { 2, N_("Small")    }
    };

    //! Lookup table to translate Sony A100 camera settings instant playback setup values to readable labels
    extern const TagDetails sonyInstantPlaybackSetupA100[] = {
        { 0, N_("Image and Information") },
        { 1, N_("Image Only")            },
        { 3, N_("Image and Histogram")   }
    };

    //! Lookup table to translate Sony A100 camera settings flash default setup values to readable labels
    extern const TagDetails sonyFlashDefaultA100[] = {
        { 0, N_("Auto")       },
        { 1, N_("Fill Flash") }
    };

    //! Lookup table to translate Sony A100 camera settings auto bracket order values to readable labels
    extern const TagDetails sonyAutoBracketOrderA100[] = {
        { 0, "0-+" },
        { 1, "-0+" }
    };

    //! Lookup table to translate Sony A100 camera settings focus hold button values to readable labels
    extern const TagDetails sonyFocusHoldButtonA100[] = {
        { 0, N_("Focus Hold")  },
        { 1, N_("DOF Preview") }
    };

    //! Lookup table to translate Sony A100 camera settings AEL button values to readable labels
    extern const TagDetails sonyAELButtonA100[] = {
        { 0, N_("Hold")        },
        { 1, N_("Toggle")      },
        { 2, N_("Spot Hold")   },
        { 3, N_("Spot Toggle") }
    };

    //! Lookup table to translate Sony A100 camera settings control dial set values to readable labels
    extern const TagDetails sonyControlDialSetA100[] = {
        { 0, N_("Shutter Speed") },
        { 1, N_("Aperture")      }
    };

    //! Lookup table to translate Sony A100 camera settings exposure compensation mode values to readable labels
    extern const TagDetails sonyExposureCompensationModeA100[] = {
        { 0, N_("Ambient and Flash") },
        { 1, N_("Ambient Only")      }
    };

    //! Lookup table to translate Sony A100 camera settings sony AF area illumination values to readable labels
    extern const TagDetails sonyAFAreaIlluminationA100[] = {
        { 0, N_("0.3 seconds") },
        { 1, N_("0.6 seconds") },
        { 2, N_("Off")         }
    };

    //! Lookup table to translate Sony A100 camera settings monitor display off values to readable labels
    extern const TagDetails sonyMonitorDisplayOffA100[] = {
        { 0, N_("Automatic") },
        { 1, N_("Manual")    }
    };

    //! Lookup table to translate Sony A100 camera settings record display values to readable labels
    extern const TagDetails sonyRecordDisplayA100[] = {
        { 0, N_("Auto-rotate") },
        { 1, N_("Horizontal")  }
    };

    //! Lookup table to translate Sony A100 camera settings play display values to readable labels
    extern const TagDetails sonyPlayDisplayA100[] = {
        { 0, N_("Auto-rotate")   },
        { 1, N_("Manual Rotate") }
    };

    //! Lookup table to translate Sony A100 camera settings metering off scale indicator values to readable labels
    extern const TagDetails sonyMeteringOffScaleIndicatorA100[] = {
        { 0,   N_("Within Range")     },
        { 1,   N_("Under/Over Range") },
        { 255, N_("Out of Range")     }
    };

    //! Lookup table to translate Sony A100 camera settings exposure indicator values to readable labels
    extern const TagDetails sonyExposureIndicatorA100[] = {
        { 0,   N_("Not Indicated")   },
        { 1,   N_("Under Scale")     },
        { 119, N_("Bottom of Scale") },
        { 120, "-2.0"                },
        { 121, "-1.7"                },
        { 122, "-1.5"                },
        { 123, "-1.3"                },
        { 124, "-1.0"                },
        { 125, "-0.7"                },
        { 126, "-0.5"                },
        { 127, "-0.3"                },
        { 128, "-0.0"                },
        { 129, "+0.3"                },
        { 130, "+0.5"                },
        { 131, "+0.7"                },
        { 132, "+1.0"                },
        { 133, "+1.3"                },
        { 134, "+1.5"                },
        { 135, "+1.7"                },
        { 136, "+2.0"                },
        { 253, N_("Top of Scale")    },
        { 254, N_("Over Scale")      }
    };

    //! Lookup table to translate Sony A100 camera settings focus mode switch values to readable labels
    extern const TagDetails sonyFocusModeSwitchA100[] = {
        { 0, N_("AM") },
        { 1, N_("MF") }
    };

    //! Lookup table to translate Sony A100 camera settings flash type switch values to readable labels
    extern const TagDetails sonyFlashTypeA100[] = {
        { 0, N_("Off")      },
        { 1, N_("Built-in") },
        { 2, N_("External") }
    };

    //! Lookup table to translate Sony A100 camera settings battery level switch values to readable labels
    extern const TagDetails sonyBatteryLevelA100[] = {
        { 3, N_("Very Low")                   },
        { 4, N_("Low")                        },
        { 5, N_("Half Full")                  },
        { 6, N_("Sufficient Power Remaining") }
    };

    // Sony A100 Camera Settings Tag Info
    const TagInfo MinoltaMakerNote::tagInfoCsA100_[] = {
        TagInfo(0x0000, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaExposureMode5D)),
        TagInfo(0x0001, "ExposureCompensationSetting", N_("Exposure Compensation Setting"),
                N_("Exposure compensation setting"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0005, "HighSpeedSync", N_("High Speed Sync"),
                N_("High speed sync"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x0006, "ManualExposureTime", N_("Manual Exposure Time"),
                N_("Manual exposure time"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0007, "ManualFNumber", N_("Manual FNumber"),
                N_("Manual FNumber"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0008, "ExposureTime", N_("Exposure Time"),
                N_("Exposure time"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0009, "FNumber", N_("FNumber"),
                N_("FNumber"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000A, "DriveMode2", N_("Drive Mode 2"),
                N_("Drive mode 2"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyDriveMode2A100)),
        TagInfo(0x000B, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaWhiteBalance5D)),
        TagInfo(0x000C, "FocusMode", N_("Focus Mode"),
                N_("Focus mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFocusModeA100)),
        TagInfo(0x000D, "LocalAFAreaPoint", N_("Local AF Area Point"),
                N_("Local AF Area Point"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint),
        TagInfo(0x000E, "AFAreaMode", N_("AF Area Mode"),
                N_("AF Area Mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyAFAreaMode),
        TagInfo(0x000F, "FlashMode", N_("FlashMode"),
                N_("FlashMode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashModeA100)),
        TagInfo(0x0010, "FlashExposureCompSetting", N_("Flash Exposure Comp Setting"),
                N_("Flash exposure compensation setting"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0012, "MeteringMode", N_("Metering Mode"),
                N_("Metering mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringModeA100)),
        TagInfo(0x0013, "ISOSetting", N_("ISO Setting"),
                N_("ISO setting"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0014, "ZoneMatchingMode", N_("Zone Matching Mode"),
                N_("Zone Matching Mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyZoneMatchingModeA100)),
        TagInfo(0x0015, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"),
                N_("Dynamic range optimizer mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode),
        TagInfo(0x0016, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyColorMode),
        TagInfo(0x0017, "ColorSpace", N_("Color Space"),
                N_("Color space"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyColorSpaceA100)),
        TagInfo(0x0018, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0019, "Contrast", N_("Contrast"),
                N_("Contrast"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001A, "Saturation", N_("Saturation"),
                N_("Saturation"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001C, "FlashMetering", N_("Flash Metering"),
                N_("Flash metering"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(minoltaFlashMeteringStd)),
        TagInfo(0x001D, "PrioritySetupShutterRelease", N_("Priority Setup Shutter Release"),
                N_("Priority Setup Shutter Release"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyPrioritySetupShutterRelease),
        TagInfo(0x001E, "DriveMode", N_("Drive Mode"),
                N_("Drive mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyDriveModeA100)),
        TagInfo(0x001F, "SelfTimerTime", N_("Self Timer Time"),
                N_("Self timer time"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonySelfTimerTimeA100)),
        TagInfo(0x0020, "ContinuousBracketing", N_("Continuous Bracketing"),
                N_("Continuous bracketing"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyContinuousBracketingA100)),
        TagInfo(0x0021, "SingleFrameBracketing", N_("Single Frame Bracketing"),
                N_("Single frame bracketing"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonySingleFrameBracketingA100)),
        TagInfo(0x0022, "WhiteBalanceBracketing", N_("White Balance Bracketing"),
                N_("White balance bracketing"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyWhiteBalanceBracketingA100)),
        TagInfo(0x0023, "WhiteBalanceSetting", N_("White Balance Setting"),
                N_("White balance setting"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyWhiteBalanceSettingA100)),
        TagInfo(0x0024, "PresetWhiteBalance", N_("Preset White Balance"),
                N_("Preset white balance"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyPresetWhiteBalanceA100)),
        TagInfo(0x0025, "ColorTemperatureSetting", N_("Color Temperature Setting"),
                N_("Color temperature setting"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyColorTemperatureSettingA100)),
        TagInfo(0x0026, "CustomWBSetting", N_("Custom WB Setting"),
                N_("Custom WB setting"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCustomWBSettingA100)),
        TagInfo(0x0027, "DynamicRangeOptimizerSettings", N_("Dynamic Range Optimizer Settings"),
                N_("Dynamic Range Optimizer Settings"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode),
        TagInfo(0x0032, "FreeMemoryCardImages", N_("Free Memory Card Images"),
                N_("Free memory card images"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0034, "CustomWBRedLevel", N_("Custom WB Red Level"),
                N_("Custom WB red level"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0035, "CustomWBGreenLevel", N_("Custom WB Green Level"),
                N_("Custom WB green level"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0036, "CustomWBBlueLevel", N_("Custom WB Blue Level"),
                N_("CustomWB blue level"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0037, "CustomWBError", N_("Custom WB Error"),
                N_("Custom WB Error"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCustomWBErrorA100)),
        TagInfo(0x0038, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White balance fine tune"),
                sony1MltCsA100Id, makerTags, signedShort, 1, printValue),
        TagInfo(0x0039, "ColorTemperature", N_("Color Temperature"),
                N_("Color temperature"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x003A, "ColorCompensationFilter", N_("Color Compensation Filter"),
                N_("Color compensation filter"),
                sony1MltCsA100Id, makerTags, signedShort, 1, printValue),
        TagInfo(0x003B, "SonyImageSize", N_("Sony Image Size"),
                N_("Sony Image Size"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSizeA100)),
        TagInfo(0x003C, "Quality", N_("Quality"),
                N_("Quality"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyQualityCs),
        TagInfo(0x003D, "InstantPlaybackTime", N_("Instant Playback Time"),
                N_("Instant playback time"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x003E, "InstantPlaybackSetup", N_("Instant Playback Setup"),
                N_("Instant playback setup"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyInstantPlaybackSetupA100)),
        TagInfo(0x003F, "NoiseReduction", N_("Noise Reduction"),
                N_("Noise reduction"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x0040, "EyeStartAF", N_("Eye Start AF"),
                N_("Eye start AF"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue),
        TagInfo(0x0041, "RedEyeReduction", N_("Red Eye Reduction"),
                N_("Red eye reduction"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x0042, "FlashDefault", N_("Flash Default"),
                N_("Flash default"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashDefaultA100)),
        TagInfo(0x0043, "AutoBracketOrder", N_("Auto Bracket Order"),
                N_("Auto bracket order"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAutoBracketOrderA100)),
        TagInfo(0x0044, "FocusHoldButton", N_("Focus Hold Button"),
                N_("Focus hold button"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFocusHoldButtonA100)),
        TagInfo(0x0045, "AELButton", N_("AEL Button"),
                N_("AEL button"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAELButtonA100)),
        TagInfo(0x0046, "ControlDialSet", N_("Control Dial Set"),
                N_("Control dial set"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyControlDialSetA100)),
        TagInfo(0x0047, "ExposureCompensationMode", N_("Exposure Compensation Mode"),
                N_("Exposure compensation mode"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureCompensationModeA100)),
        TagInfo(0x0048, "AFAssist", N_("AF Assist"),
                N_("AF assist"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue),
        TagInfo(0x0049, "CardShutterLock", N_("Card Shutter Lock"),
                N_("Card shutter lock"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue),
        TagInfo(0x004A, "LensShutterLock", N_("Lens Shutter Lock"),
                N_("Lens shutter lock"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue),
        TagInfo(0x004B, "AFAreaIllumination", N_("AF Area Illumination"),
                N_("AF area illumination"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAFAreaIlluminationA100)),
        TagInfo(0x004C, "MonitorDisplayOff", N_("Monitor Display Off"),
                N_("Monitor display off"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMonitorDisplayOffA100)),
        TagInfo(0x004D, "RecordDisplay", N_("Record Display"),
                N_("Record display"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyRecordDisplayA100)),
        TagInfo(0x004E, "PlayDisplay", N_("Play Display"),
                N_("Play display"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyPlayDisplayA100)),
        TagInfo(0x0050, "ExposureIndicator", N_("Exposure Indicator"),
                N_("Exposure indicator"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)),
        TagInfo(0x0051, "AELExposureIndicator", N_("AEL Exposure Indicator"),
                N_("AEL exposure indicator (also indicates exposure for next shot when bracketing)"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)),
        TagInfo(0x0052, "ExposureBracketingIndicatorLast", N_("Exposure Bracketing Indicator Last"),
                N_("Exposure bracketing indicator last (indicator for last shot when bracketing)"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)),
        TagInfo(0x0053, "MeteringOffScaleIndicator", N_("Metering Off Scale Indicator"),
                N_("Metering off scale indicator (two flashing triangles when under or over metering scale)"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringOffScaleIndicatorA100)),
        TagInfo(0x0054, "FlashExposureIndicator", N_("Flash Exposure Indicator"),
                N_("Flash exposure indicator"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)),
        TagInfo(0x0055, "FlashExposureIndicatorNext", N_("Flash Exposure Indicator Next"),
                N_("Flash exposure indicator next (indicator for next shot when bracketing)"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)),
        TagInfo(0x0056, "FlashExposureIndicatorLast", N_("Flash Exposure Indicator Last"),
                N_("Flash exposure indicator last (indicator for last shot when bracketing)"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)),
        TagInfo(0x0057, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x0058, "FocusModeSwitch", N_("Focus Mode Switch"),
                N_("Focus mode switch"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFocusModeSwitchA100)),
        TagInfo(0x0059, "FlashType", N_("Flash Type"),
                N_("Flash type"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashTypeA100)),
        TagInfo(0x005A, "Rotation", N_("Rotation"),
                N_("Rotation"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyRotation),
        TagInfo(0x004B, "AELock", N_("AE Lock"),
                N_("AE lock"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x005E, "ColorTemperature", N_("Color Temperature"),
                N_("Color temperature"),
                sony1MltCsA100Id, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x005F, "ColorCompensationFilter", N_("Color Compensation Filter"),
                N_("Color compensation filter: negative is green, positive is magenta"),
                sony1MltCsA100Id, makerTags, unsignedLong, 1, printValue),
        TagInfo(0x0060, "BatteryLevel", N_("Battery Level"),
                N_("Battery level"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyBatteryLevelA100)),
        // End of list marker
        TagInfo(0xffff, "(UnknownSonyCsA100Tag)", "(UnknownSonyCsA100Tag)",
                N_("Unknown Sony Camera Settings A100 tag"),
                sony1MltCsA100Id, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* MinoltaMakerNote::tagListCsA100()
    {
        return tagInfoCsA100_;
    }

    // TODO : Add camera settings tags info "New2"...

    // -- Minolta and Sony MakerNote Common Values ---------------------------------------

    //! Lookup table to translate Minolta/Sony Lens id values to readable labels
    /* NOTE:
       - duplicate tags value are:
       0/25520, 4/25920, 13/25610, 19/25910, 22/26050/26070,
       25500/25501/26130, 25540/25541/25850, 25580/25581, 2564025641,
       25720/25721, 25790/25791, 25960/25961, 25980/25981, 26150/26151
       - No need to i18n these string.
    */
    extern TagDetails const minoltaSonyLensID[] = {
        { 0,     "Minolta AF 28-85mm F3.5-4.5 New" },
        { 1,     "Minolta AF 80-200mm F2.8 HS-APO G" },
        { 2,     "Minolta AF 28-70mm F2.8 G" },
        { 3,     "Minolta AF 28-80mm F4-5.6" },
        { 4,     "Minolta AF 85mm F1.4G" },
        { 5,     "Minolta AF 35-70mm F3.5-4.5 [II]" },
        { 6,     "Minolta AF 24-85mm F3.5-4.5 [New]" },
        { 7,     "Minolta AF 100-300mm F4.5-5.6(D) APO [New] | "
                 "Minolta AF 100-400mm F4.5-6.7(D) | "
                 "Sigma AF 100-300mm F4 EX DG IF" },
        { 8,     "Minolta AF 70-210mm F4.5-5.6 [II]" },
        { 9,     "Minolta AF 50mm F3.5 Macro" },
        { 10,    "Minolta AF 28-105mm F3.5-4.5 [New]" },
        { 11,    "Minolta AF 300mm F4 HS-APO G" },
        { 12,    "Minolta AF 100mm F2.8 Soft Focus" },
        { 13,    "Minolta AF 75-300mm F4.5-5.6 (New or II)" },
        { 14,    "Minolta AF 100-400mm F4.5-6.7 APO" },
        { 15,    "Minolta AF 400mm F4.5 HS-APO G" },
        { 16,    "Minolta AF 17-35mm F3.5 G" },
        { 17,    "Minolta AF 20-35mm F3.5-4.5" },
        { 18,    "Minolta AF 28-80mm F3.5-5.6 II" },
        { 19,    "Minolta AF 35mm F1.4 G" },
        { 20,    "Minolta/Sony 135mm F2.8 [T4.5] STF" },
        { 22,    "Minolta AF 35-80mm F4-5.6 II" },
        { 23,    "Minolta AF 200mm F4 Macro APO G" },
        { 24,    "Minolta/Sony AF 24-105mm F3.5-4.5 (D) | "
                 "Sigma 18-50mm F2.8 | "
                 "Sigma 17-70mm F2.8-4.5 (D) | "
                 "Sigma 20-40mm F2.8 EX DG Aspherical IF | "
                 "Sigma 18-200mm F3.5-6.3 DC | "
                 "Sigma 20-40mm F2.8 EX DG Aspherical IF | "
                 "Sigma DC 18-125mm F4-5,6 D | "
                 "Tamron SP AF 28-75mm F2.8 XR Di (IF) Macro" },
        { 25,    "Minolta AF 100-300mm F4.5-5.6 APO (D) | "
                 "Sigma 100-300mm F4 EX (APO (D) or D IF) | "
                 "Sigma 70mm F2.8 EX DG Macro | "
                 "Sigma 20mm F1.8 EX DG Aspherical RF | "
                 "Sigma 30mm F1.4 DG EX" },
        { 27,    "Minolta AF 85mm F1.4 G (D)" },
        { 28,    "Minolta/Sony AF 100mm F2.8 Macro (D) | "
                 "Tamron SP AF 90mm F2.8 Di Macro" },
        { 29,    "Minolta/Sony AF 75-300mm F4.5-5.6 (D) " },
        { 30,    "Minolta AF 28-80mm F3.5-5.6 (D) | "
                 "Sigma AF 10-20mm F4-5.6 EX DC | "
                 "Sigma AF 12-24mm F4.5-5.6 EX DG | "
                 "Sigma 28-70mm EX DG F2.8 | "
                 "Sigma 55-200mm F4-5.6 DC" },
        { 31,    "Minolta AF 50mm F2.8 Macro(D) | "
                 "Minolta AF 50mm F3.5 Macro" },
        { 32,    "Minolta AF 100-400mm F4.5-6.7(D) | "
                 "Minolta AF 300mm F2.8G APO(D) SSM" },
        { 33,    "Minolta/Sony AF 70-200mm F2.8 G" },
        { 35,    "Minolta AF 85mm F1.4 G (D) Limited" },
        { 36,    "Minolta AF 28-100mm F3.5-5.6 (D)" },
        { 38,    "Minolta AF 17-35mm F2.8-4 (D)" },
        { 39,    "Minolta AF 28-75mm F2.8 (D)" },
        { 40,    "Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | "
                 "Sony AF DT 18-200mm F3.5-6.3" },
        { 41,    "Minolta/Sony AF DT 11-18mm F4.5-5.6 (D) | "
                 "Tamron SP AF 11-18mm F4.5-5.6 Di II LD Aspherical IF" },
        { 42,    "Minolta/Sony AF DT 18-200mm F3.5-6.3 (D)" },
        { 43,    "Sony 35mm F1.4 G (SAL-35F14G)" },
        { 44,    "Sony 50mm F1.4 (SAL-50F14)" },
        { 45,    "Carl Zeiss Planar T* 85mm F1.4 ZA" },
        { 46,    "Carl Zeiss Vario-Sonnar T* DT 16-80mm F3.5-4.5 ZA" },
        { 47,    "Carl Zeiss Sonnar T* 135mm F1.8 ZA" },
        { 48,    "Carl Zeiss Vario-Sonnar T* 24-70mm F2.8 ZA SSM (SAL-2470Z)" },
        { 49,    "Sony AF DT 55-200mm F4-5.6" },
        { 50,    "Sony AF DT 18-250mm F3.5-6.3" },
        { 51,    "Sony AF DT 16-105mm F3.5-5.6 | "
                 "Sony AF DT 55-200mm F4-5.5" },
        { 52,    "Sony 70-300mm F4.5-5.6 G SSM" },
        { 53,    "Sony AF 70-400mm F4.5-5.6 G SSM (SAL-70400G)" },
        { 54,    "Carl Zeiss Vario-Sonnar T* 16-35mm F2.8 ZA SSM (SAL-1635Z)" },
        { 55,    "Sony DT 18-55mm F3.5-5.6 SAM (SAL-1855)" },
        { 56,    "Sony AF DT 55-200mm F4-5.6 SAM" },
        { 57,    "Sony AF DT 50mm F1.8 SAM" },
        { 58,    "Sony AF DT 30mm F2.8 SAM Macro" },
        { 128,   "Sigma 70-200mm F2.8 APO EX DG MACRO | "
                 "Tamron 18-200mm F3.5-6.3 | "
                 "Tamron 28-300mm F3.5-6.3 | "
                 "Tamron 80-300mm F3.5-6.3 | "
                 "Tamron AF 28-200mm F3.8-5.6 XR Di Aspherical [IF] MACRO | "
                 "Tamron SP AF 17-35mm F2.8-4 Di LD Aspherical IF | "
                 "Sigma AF 50-150mm F2.8 EX DC APO HSM II | "
                 "Sigma 10-20mm F3.5 EX DC HSM | "
                 "Sigma 70-200mm F2.8 II EX DG APO MACRO HSM" },
        { 129,   "Tamron 200-400mm F5.6 LD | "
                 "Tamron 70-300mm F4-5.6 LD" },
        { 131,   "Tamron 20-40mm F2.7-3.5 SP Aspherical IF" },
        { 135,   "Vivitar 28-210mm F3.5-5.6" },
        { 136,   "Tokina EMZ M100 AF 100mm F3.5" },
        { 137,   "Cosina 70-210mm F2.8-4 AF" },
        { 138,   "Soligor 19-35mm F3.5-4.5" },
        { 142,   "Voigtlander 70-300mm F4.5-5.6" },
        { 146,   "Voigtlander Macro APO-Lanthar 125mm F2.5 SL" },
        { 193,   "Minolta AF 1.4x APO II" },
        { 255,   "Tamron SP AF 17-50mm F2.8 XR Di II LD Aspherical | "
                 "Tamron AF 18-250mm F3.5-6.3 XR Di II LD | "
                 "Tamron AF 55-200mm F4-5.6 Di II | "
                 "Tamron AF 70-300mm F4-5.6 Di LD MACRO 1:2 | "
                 "Tamron SP AF 200-500mm F5.0-6.3 Di LD IF | "
                 "Tamron SP AF 10-24mm F3.5-4.5 Di II LD Aspherical IF | "
                 "Tamron SP AF 70-200mm F2.8 Di LD IF Macro | "
                 "Tamron SP AF 28-75mm F2.8 XR Di LD Aspherical IF" },
        { 25500, "Minolta AF 50mm F1.7" },
        { 25501, "Minolta AF 50mm F1.7" },
        { 25510, "Minolta AF 35-70mm F1.4" },
        { 25511, "Minolta AF 35-70mm F4 | "
                 "Sigma UC AF 28-70mm F3.5-4.5 | "
                 "Sigma AF 28-70mm F2.8 | "
                 "Sigma M-AF 70-200mm F2.8 EX Aspherical | "
                 "Quantaray M-AF 35-80mm F4-5.6 " },
        { 25520, "Minolta AF 28-85mm F3.5-4.5" },
        { 25521, "Minolta AF 28-85mm F3.5-4.5 | "
                 "Tokina 19-35mm F3.5-4.5 | "
                 "Tokina 28-70mm F2.8 AT-X | "
                 "Tokina 80-400mm F4.5-5.6 AT-X AF II 840 | "
                 "Tokina AF PRO 28-80mm F2.8 AT-X 280 | "
                 "Tokina AT-X PRO II AF 28-70mm F2.6-2.8 270 | "
                 "Tamron AF 19-35mm F3.5-4.5 | "
                 "Angenieux AF 28-70mm F2.6" },
        { 25530, "Minolta AF 28-135mm F4-4.5" },
        { 25531, "Minolta AF 28-135mm F4-4.5 | "
                 "Sigma ZOOM-alpha 35-135mm F3.5-4.5 | "
                 "Sigma 28-105mm F2.8-4 Aspherical" },
        { 25540, "Minolta AF 35-105mm F3.5-4.5" },
        { 25541, "Minolta AF 35-105mm F3.5-4.5" },
        { 25550, "Minolta AF 70-210mm F4" },
        { 25551, "Minolta AF 70-210mm F4 Macro | "
                 "Sigma 70-210mm F4-5.6 APO | "
                 "Sigma M-AF 70-200mm F2.8 EX APO | "
                 "Sigma 75-200mm F2.8-3.5" },
        { 25560, "Minolta AF 135mm F2.8" },
        { 25561, "Minolta AF 135mm F2.8" },
        { 25570, "Minolta AF 28mm F2.8" },
        { 25571, "Minolta/Sony AF 28mm F2.8" },
        { 25580, "Minolta AF 24-50mm F4" },
        { 25581, "Minolta AF 24-50mm F4" },
        { 25600, "Minolta AF 100-200mm F4.5" },
        { 25601, "Minolta AF 100-200mm F4.5" },
        { 25610, "Minolta AF 75-300mm F4.5-5.6" },
        { 25611, "Minolta AF 75-300mm F4.5-5.6 | "
                 "Sigma 70-300mm F4-5.6 DL Macro | "
                 "Sigma 300mm F4 APO Macro | "
                 "Sigma AF 500mm F4.5 APO | "
                 "Sigma AF 170-500mm F5-6.3 APO Aspherical | "
                 "Tokina AT-X AF 300mm F4 | "
                 "Tokina AT-X AF 400mm F5.6 SD | "
                 "Tokina AF 730 II 75-300mm F4.5-5.6" },
        { 25620, "Minolta AF 50mm F1.4" },
        { 25621, "Minolta AF 50mm F1.4 [New]" },
        { 25630, "Minolta AF 300mm F2.8G APO" },
        { 25631, "Minolta AF 300mm F2.8 APO | "
                 "Sigma AF 50-500mm F4-6.3 EX DG APO | "
                 "Sigma AF 170-500mm F5-6.3 APO Aspherical | "
                 "Sigma AF 500mm F4.5 EX DG APO | "
                 "Sigma 400mm F5.6 APO" },
        { 25640, "Minolta AF 50mm F2.8 Macro" },
        { 25641, "Minolta AF 50mm F2.8 Macro | "
                 "Sigma 50mm F2.8 EX Macro" },
        { 25650, "Minolta AF 600mm F4 APO" },
        { 25651, "Minolta AF 600mm F4 APO" },
        { 25660, "Minolta AF 24mm F2.8" },
        { 25661, "Minolta AF 24mm F2.8 | "
                 "Sigma 17-35mm F2.8-4.0 EX-D" },
        { 25720, "Minolta AF 500mm F8 Reflex" },
        { 25721, "Minolta/Sony AF 500mm F8 Reflex" },
        { 25780, "Minolta/Sony AF 16mm F2.8 Fisheye" },
        { 25781, "Minolta/Sony AF 16mm F2.8 Fisheye | "
                 "Sigma 8mm F4 EX [DG] Fisheye | "
                 "Sigma 14mm F3.5 | "
                 "Sigma 15mm F2.8 Fisheye" },
        { 25790, "Minolta AF 20mm F2.8" },
        { 25791, "Minolta/Sony AF 20mm F2.8" },
        { 25810, "Minolta AF 100mm F2.8 Macro" },
        { 25811, "Minolta AF 100mm F2.8 Macro [New] | "
                 "Sigma AF 90mm F2.8 Macro | "
                 "Sigma AF 105mm F2.8 EX [DG] Macro | "
                 "Sigma 180mm F5.6 Macro | "
                 "Tamron 90mm F2.8 Macro" },
        { 25850, "Minolta AF 35-105mm F3.5-4.5" },
        { 25851, "Beroflex 35-135mm F3.5-4.5" },
        { 25858, "Minolta AF 35-105mm F3.5-4.5 New | "
                 "Tamron 24-135mm F3.5-5.6" },
        { 25880, "Minolta AF 70-210mm F3.5-4.5" },
        { 25881, "Minolta AF 70-210mm F3.5-4.5" },
        { 25890, "Minolta AF 80-200mm F2.8 APO" },
        { 25891, "Minolta AF 80-200mm F2.8 APO | "
                 "Tokina 80-200mm F2.8" },
        { 25910, "Minolta AF 35mm F1.4G" },
        { 25911, "Minolta AF 35mm F1.4" },
        { 25920, "Minolta AF 85mm F1.4G" },
        { 25921, "Minolta AF 85mm F1.4G(D)" },
        { 25930, "Minolta AF 200mm F2.8 APO" },
        { 25931, "Minolta AF 200mm F2.8 G APO" },
        { 25940, "Minolta AF 3X-1X F1.7-2.8 Macro" },
        { 25941, "Minolta AF 3x-1x F1.7-2.8 Macro" },
        { 25960, "Minolta AF 28mm F2" },
        { 25961, "Minolta AF 28mm F2" },
        { 25970, "Minolta AF 35mm F2" },
        { 25971, "Minolta AF 35mm F2 [New]" },
        { 25980, "Minolta AF 100mm F2" },
        { 25981, "Minolta AF 100mm F2" },
        { 26040, "Minolta AF 80-200mm F4.5-5.6" },
        { 26041, "Minolta AF 80-200mm F4.5-5.6" },
        { 26050, "Minolta AF 35-80mm F4-5.6" },
        { 26051, "Minolta AF 35-80mm F4-5.6" },
        { 26060, "Minolta AF 100-300mm F4.5-5.6" },
        { 26061, "Minolta AF 100-300mm F4.5-5.6(D) | "
                 "Sigma 105mm F2.8 Macro EX-DG" },
        { 26070, "Minolta AF 35-80mm F4-5.6" },
        { 26071, "Minolta AF 35-80mm F4-5.6" },
        { 26080, "Minolta AF 300mm F2.8G APO High Speed" },
        { 26081, "Minolta AF 300mm F2.8G" },
        { 26090, "Minolta AF 600mm F4G APO High Speed" },
        { 26091, "Minolta AF 600mm F4 HS-APO G" },
        { 26120, "Minolta AF 200mm F2.8G APO High Speed" },
        { 26121, "Minolta AF 200mm F2.8G(D)" },
        { 26130, "Minolta AF 50mm F1.7" },
        { 26131, "Minolta AF 50mm F1.7 New" },
        { 26150, "Minolta AF 28-105mm F3.5-4.5 Xi" },
        { 26151, "Minolta AF 28-105mm F3.5-4.5 xi" },
        { 26160, "Minolta AF 35-200mm F4.5-5.6 Xi" },
        { 26161, "Minolta AF 35-200mm F4.5-5.6 Xi" },
        { 26180, "Minolta AF 28-80mm F4-5.6 Xi" },
        { 26181, "Minolta AF 28-80mm F4-5.6 xi" },
        { 26190, "Minolta AF 80-200mm F4.5-5.6 Xi" },
        { 26191, "Minolta AF 80-200mm F4.5-5.6 Xi" },
        { 26201, "Minolta AF 28-70mm F2.8 G" },
        { 26210, "Minolta AF 100-300mm F4.5-5.6 Xi" },
        { 26211, "Minolta AF 100-300mm F4.5-5.6 xi" },
        { 26240, "Minolta AF 35-80mm F4-5.6 Power" },
        { 26241, "Minolta AF 35-80mm F4-5.6 Power Zoom" },
        { 26281, "Minolta AF 80-200mm F2.8 G" },
        { 26291, "Minolta AF 85mm F1.4 New" },
        { 26311, "Minolta/Sony AF 100-300mm F4.5-5.6 APO" },
        { 26321, "Minolta AF 24-50mm F4 New" },
        { 26381, "Minolta AF 50mm F2.8 Macro New" },
        { 26391, "Minolta AF 100mm F2.8 Macro" },
        { 26411, "Minolta/Sony AF 20mm F2.8 New" },
        { 26421, "Minolta AF 24mm F2.8 New" },
        { 26441, "Minolta AF 100-400mm F4.5-6.7 APO" },
        { 26621, "Minolta AF 50mm F1.4 New" },
        { 26671, "Minolta AF 35mm F2 New" },
        { 26681, "Minolta AF 28mm F2 New" },
        { 26721, "Minolta AF 24-105mm F3.5-4.5 (D)" },
        { 45671, "Tokina 70-210mm F4-5.6" },
        { 45741, "Minolta AF200mm F2.8G x2 | "
                 "Tokina 300mm F2.8 x2 | "
                 "Tokina RF 500mm F8.0 x2 | "
                 "Tamron SP AF 90mm F2.5"},
        { 45751, "1.4x Teleconverter " },
        { 45851, "Tamron SP AF 300mm F2.8 LD IF" },
        { 65535, "T-Mount | "
                 "Arax MC 35mm F2.8 Tilt+Shift | "
                 "Arax MC 80mm F2.8 Tilt+Shift | "
                 "Zenitar MF 16mm F2.8 Fisheye M42 | "
                 "Samyang 500mm Mirror F8.0 | "
                 "Pentacon Auto 135mm F2.8 | "
                 "Pentacon Auto 29mm F2.8 | "
                 "Helios 44-2 58mm F2.0 | "
                 "No Lens" }
    };

    std::ostream& printMinoltaSonyLensID(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Minolta A100 and all other Sony Alpha camera color mode values to readable labels
    extern const TagDetails minoltaSonyColorMode[] = {
        { 0,   N_("Standard")            },
        { 1,   N_("Vivid Color")         },
        { 2,   N_("Portrait")            },
        { 3,   N_("Landscape")           },
        { 4,   N_("Sunset")              },
        { 5,   N_("Night View/Portrait") },
        { 6,   N_("Black & White")       },
        { 7,   N_("AdobeRGB")            },
        { 12,  N_("Neutral")             },
        { 100, N_("Neutral")             },
        { 101, N_("Clear")               },
        { 102, N_("Deep")                },
        { 103, N_("Light")               },
        { 104, N_("Night View")          },
        { 105, N_("Autumn Leaves")       }
    };

    std::ostream& printMinoltaSonyColorMode(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyColorMode)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Minolta/Sony bool function values to readable labels
    extern const TagDetails minoltaSonyBoolFunction[] = {
        { 0, N_("Off") },
        { 1, N_("On")  }
    };

    std::ostream& printMinoltaSonyBoolValue(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyBoolFunction)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Minolta/Sony bool inverse function values to readable labels
    extern const TagDetails minoltaSonyBoolInverseFunction[] = {
        { 0, N_("On")  },
        { 1, N_("Off") }
    };

    std::ostream& printMinoltaSonyBoolInverseValue(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyBoolInverseFunction)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony camera settings focus mode values to readable labels
    extern const TagDetails minoltaSonyAFAreaMode[] = {
        { 0, N_("Wide")  },
        { 1, N_("Local") },
        { 2, N_("Spot")  }
    };

    std::ostream& printMinoltaSonyAFAreaMode(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyAFAreaMode)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony camera settings Local AF Area Point values to readable labels
    extern const TagDetails minoltaSonyLocalAFAreaPoint[] = {
        { 1,  N_("Center")       },
        { 2,  N_("Top")          },
        { 3,  N_("Top-Right")    },
        { 4,  N_("Right")        },
        { 5,  N_("Bottom-Right") },
        { 6,  N_("Bottom")       },
        { 7,  N_("Bottom-Left")  },
        { 8,  N_("Left")         },
        { 9,  N_("Top-Left")     },
        { 10, N_("Far-Right")    },
        { 11, N_("Far-Left")     }
    };

    std::ostream& printMinoltaSonyLocalAFAreaPoint(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyLocalAFAreaPoint)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony camera settings dynamic range optimizer mode values to readable labels
    extern const TagDetails minoltaSonyDynamicRangeOptimizerMode[] = {
        { 0,    N_("Off")            },
        { 1,    N_("Standard")       },
        { 2,    N_("Advanced Auto")  },
        { 3,    N_("Advanced Level") },
        { 4097, N_("Auto")           }
    };

    std::ostream& printMinoltaSonyDynamicRangeOptimizerMode(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyDynamicRangeOptimizerMode)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony camera settings priority setup shutter release values to readable labels
    extern const TagDetails minoltaSonyPrioritySetupShutterRelease[] = {
        { 0, N_("AF")      },
        { 1, N_("Release") }
    };

    std::ostream& printMinoltaSonyPrioritySetupShutterRelease(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyPrioritySetupShutterRelease)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony camera settings quality values to readable labels
    extern const TagDetails minoltaSonyQualityCs[] = {
        { 0,   N_("RAW ")       },
        { 2,   N_("CRAW ")      },
        { 16,  N_("Extra Fine") },
        { 32,  N_("Fine")       },
        { 34,  N_("RAW+JPEG")   },
        { 35,  N_("CRAW+JPEG")  },
        { 48,  N_("Standard")   }
    };

    std::ostream& printMinoltaSonyQualityCs(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyQualityCs)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony camera settings rotation values to readable labels
    extern const TagDetails minoltaSonyRotation[] = {
        { 0, N_("Horizontal (normal)") },
        { 1, N_("Rotate 90 CW")        },
        { 2, N_("Rotate 270 CW")       }
    };

    std::ostream& printMinoltaSonyRotation(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyRotation)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Minolta/Sony scene mode values to readable labels
    extern const TagDetails minoltaSonySceneMode[] = {
        { 0,  N_("Standard")            },
        { 1,  N_("Portrait")            },
        { 2,  N_("Text")                },
        { 3,  N_("Night Scene")         },
        { 4,  N_("Sunset")              },
        { 5,  N_("Sports")              },
        { 6,  N_("Landscape")           },
        { 7,  N_("Night Portrait")      },
        { 8,  N_("Macro")               },
        { 9,  N_("Super Macro")         },
        { 16, N_("Auto")                },
        { 17, N_("Night View/Portrait") }
    };

    std::ostream& printMinoltaSonySceneMode(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonySceneMode)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony/Minolta image quality values to readable labels
    extern const TagDetails minoltaSonyImageQuality[] = {
        { 0, N_("Raw")                   },
        { 1, N_("Super Fine")            },
        { 2, N_("Fine")                  },
        { 3, N_("Standard")              },
        { 4, N_("Economy")               },
        { 5, N_("Extra Fine")            },
        { 6, N_("Raw + JPEG")            },
        { 7, N_("Compressed Raw")        },
        { 8, N_("Compressed Raw + JPEG") }
    };

    std::ostream& printMinoltaSonyImageQuality(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyImageQuality)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony/Minolta teleconverter model values to readable labels
    extern const TagDetails minoltaSonyTeleconverterModel[] = {
        { 0x00, N_("None")                     },
        { 0x48, N_("Minolta AF 2x APO (D)")    },
        { 0x50, N_("Minolta AF 2x APO II")     },
        { 0x88, N_("Minolta AF 1.4x APO (D)")  },
        { 0x90, N_("Minolta AF 1.4x APO II")   }
    };

    std::ostream& printMinoltaSonyTeleconverterModel(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyTeleconverterModel)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Sony/Minolta Std camera settings white balance values to readable labels
    extern const TagDetails minoltaSonyWhiteBalanceStd[] = {
        { 0x00,  N_("Auto")                           },
        { 0x01,  N_("Color Temperature/Color Filter") },
        { 0x10,  N_("Daylight")                       },
        { 0x20,  N_("Cloudy")                         },
        { 0x30,  N_("Shade")                          },
        { 0x40,  N_("Tungsten")                       },
        { 0x50,  N_("Flash")                          },
        { 0x60,  N_("Fluorescent")                    },
        { 0x70,  N_("Custom")                         }
    };

    std::ostream& printMinoltaSonyWhiteBalanceStd(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyWhiteBalanceStd)(os, value, metadata);
    }

    //! Lookup table to translate Sony/Minolta zone matching values to readable labels
    extern const TagDetails minoltaSonyZoneMatching[] = {
        { 0, N_("ISO Setting Used") },
        { 1, N_("High Key") },
        { 2, N_("Low Key")  }
    };

    std::ostream& printMinoltaSonyZoneMatching(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyZoneMatching)(os, value, metadata);
    }

    std::ostream& printMinoltaSonyFlashExposureComp(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 1 || value.typeId() != signedRational) {
            return os << "(" << value << ")";
        }
        return os << std::fixed << std::setprecision(2) << value.toFloat(0) << " EV";
    }

}}                                      // namespace Internal, Exiv2
