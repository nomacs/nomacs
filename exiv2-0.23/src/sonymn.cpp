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
  File:      sonymn.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Apr-05, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: sonymn.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "minoltamn_int.hpp"
#include "sonymn_int.hpp"
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

    // -- Standard Sony Makernotes tags ---------------------------------------------------------------

    //! Lookup table to translate Sony Auto HDR values to readable labels
    extern const TagDetails sonyHDRMode[] = {
        { 0x00000, N_("Off")   },
        { 0x10001, N_("Auto")  },
        { 0x10010, "1"         },
        { 0x10012, "2"         },
        { 0x10014, "3"         },
        { 0x10016, "4"         },
        { 0x10018, "5"         }
    };

    //! Lookup table to translate Sony model ID values to readable labels
    extern const TagDetails sonyModelId[] = {
        { 2,   "DSC-R1"    },
        { 256, "DSLR-A100" },
        { 257, "DSLR-A900" },
        { 258, "DSLR-A700" },
        { 259, "DSLR-A200" },
        { 260, "DSLR-A350" },
        { 261, "DSLR-A300" },
        { 263, "DSLR-A380" },
        { 264, "DSLR-A330" },
        { 265, "DSLR-A230" },
        { 269, "DSLR-A850" },
        { 273, "DSLR-A550" },
        { 274, "DSLR-A500" },
        { 275, "DSLR-A450" },
        { 278, "NEX-5"     },
        { 279, "NEX-3"     }
    };

    //! Lookup table to translate Sony dynamic range optimizer values to readable labels
    extern const TagDetails print0xb025[] = {
        { 0,  N_("Off")           },
        { 1,  N_("Standard ")     },
        { 2,  N_("Advanced Auto") },
        { 3,  N_("Auto")          },
        { 8,  N_("Advanced Lv1")  },
        { 9,  N_("Advanced Lv2")  },
        { 10, N_("Advanced Lv3")  },
        { 11, N_("Advanced Lv4")  },
        { 12, N_("Advanced Lv5")  },
        { 16, "1"                 },
        { 17, "2"                 },
        { 18, "3"                 },
        { 19, "4"                 },
        { 20, "5"                 }
    };

    //! Lookup table to translate Sony exposure mode values to readable labels
    extern const TagDetails sonyExposureMode[] = {
        { 0,     N_("Auto")                     },
        { 1,     N_("Portrait")                 },
        { 2,     N_("Beach")                    },
        { 4,     N_("Snow")                     },
        { 5,     N_("Landscape ")               },
        { 6,     N_("Program")                  },
        { 7,     N_("Aperture priority")        },
        { 8,     N_("Shutter priority")         },
        { 9,     N_("Night Scene / Twilight")   },
        { 10,    N_("Hi-Speed Shutter")         },
        { 11,    N_("Twilight Portrait")        },
        { 12,    N_("Soft Snap")                },
        { 13,    N_("Fireworks")                },
        { 14,    N_("Smile Shutter")            },
        { 15,    N_("Manual")                   },
        { 18,    N_("High Sensitivity")         },
        { 20,    N_("Advanced Sports Shooting") },
        { 29,    N_("Underwater")               },
        { 33,    N_("Gourmet")                  },
        { 34,    N_("Panorama")                 },
        { 35,    N_("Handheld Twilight")        },
        { 36,    N_("Anti Motion Blur")         },
        { 37,    N_("Pet")                      },
        { 38,    N_("Backlight Correction HDR") },
        { 65535, N_("n/a")                      }
    };

    //! Lookup table to translate Sony JPEG Quality values to readable labels
    extern const TagDetails sonyJPEGQuality[] = {
        { 0,     N_("Normal") },
        { 1,     N_("Fine")   },
        { 65535, N_("n/a")    }
    };

    //! Lookup table to translate Sony anti-blur values to readable labels
    extern const TagDetails sonyAntiBlur[] = {
        { 0,     N_("Off")             },
        { 1,     N_("On (Continuous)") },
        { 2,     N_("On (Shooting)")   },
        { 65535, N_("n/a")             }
    };

    //! Lookup table to translate Sony dynamic range optimizer values to readable labels
    extern const TagDetails print0xb04f[] = {
        { 0, N_("Off")      },
        { 1, N_("Standard") },
        { 2, N_("Plus")     }
    };

    //! Lookup table to translate Sony Intelligent Auto values to readable labels
    extern const TagDetails sonyIntelligentAuto[] = {
        { 0, N_("Off")      },
        { 1, N_("On")       },
        { 2, N_("Advanced") }
    };

    //! Lookup table to translate Sony WB values to readable labels
    extern const TagDetails sonyWhiteBalance[] = {
        { 0,  N_("Auto")                       },
        { 4,  N_("Manual")                     },
        { 5,  N_("Daylight")                   },
        { 6,  N_("Cloudy")                     },
        { 7,  N_("White Flourescent")          },
        { 8,  N_("Cool White Flourescent")     },
        { 9,  N_("Day White Flourescent")      },
        { 14, N_("Incandescent")               },
        { 15, N_("Flash")                      },
        { 17, N_("Underwater 1 (Blue Water)")  },
        { 18, N_("Underwater 2 (Green Water)") }
    };

    //! Lookup table to translate Sony AF mode values to readable labels
    extern const TagDetails sonyFocusMode[] = {
        { 1,     "AF-S"             },
        { 2,     "AF-C"             },
        { 4,     N_("Permanent-AF") },
        { 65535, N_("n/a")          }
    };

    //! Lookup table to translate Sony AF mode values to readable labels
    extern const TagDetails sonyAFMode[] = {
        { 0,     N_("Default")          },
        { 1,     N_("Multi AF")         },
        { 2,     N_("Center AF")        },
        { 3,     N_("Spot AF")          },
        { 4,     N_("Flexible Spot AF") },
        { 6,     N_("Touch AF")         },
        { 14,    N_("Manual Focus")     },
        { 15,    N_("Face Detected")    },
        { 65535, N_("n/a")              }
    };

    //! Lookup table to translate Sony AF illuminator values to readable labels
    extern const TagDetails sonyAFIlluminator[] = {
        { 0,     N_("Off") },
        { 1,     N_("Auto")  },
        { 65535, N_("n/a") }
    };

    //! Lookup table to translate Sony macro mode values to readable labels
    extern const TagDetails sonyMacroMode[] = {
        { 0,     N_("Off")         },
        { 1,     N_("On")          },
        { 2,     N_("Close Focus") },
        { 65535, N_("n/a")         }
    };

    //! Lookup table to translate Sony flash level values to readable labels
    extern const TagDetails sonyFlashLevel[] = {
        { -32768, N_("Low")    },
        { -1,     N_("n/a")    },
        { 0,      N_("Normal") },
        { 32767,  N_("High")   }
    };

    //! Lookup table to translate Sony release mode values to readable labels
    extern const TagDetails sonyReleaseMode[] = {
        { 0,     N_("Normal")                   },
        { 2,     N_("Burst")                    },
        { 5,     N_("Exposure Bracketing")      },
        { 6,     N_("White Balance Bracketing") },
        { 65535, N_("n/a")                      }
    };

    //! Lookup table to translate Sony sequence number values to readable labels
    extern const TagDetails sonySequenceNumber[] = {
        { 0,     N_("Single")                    },
        { 65535, N_("n/a")                       }
    };

    //! Lookup table to translate Sony long exposure noise reduction values to readable labels
    extern const TagDetails sonyLongExposureNoiseReduction[] = {
        { 0,     N_("Off") },
        { 1,     N_("On")  },
        { 65535, N_("n/a") }
    };

    std::ostream& SonyMakerNote::print0xb000(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 4)
        {
            os << "(" << value << ")";
        }
        else
        {
            std::string val = value.toString(0) + value.toString(1) + value.toString(2) + value.toString(3);
            if      (val == "0002") os << "JPEG";
            else if (val == "1000") os << "SR2";
            else if (val == "2000") os << "ARW 1.0";
            else if (val == "3000") os << "ARW 2.0";
            else if (val == "3100") os << "ARW 2.1";
            else if (val == "3200") os << "ARW 2.2";
            else                    os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& SonyMakerNote::printImageSize(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() == 2)
            os << value.toString(0) << " x " << value.toString(1);
        else
            os << "(" << value << ")";

        return os;
    }

    // Sony MakerNote Tag Info
    const TagInfo SonyMakerNote::tagInfo_[] = {
        TagInfo(0x0102, "Quality", N_("Image Quality"),
                N_("Image quality"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyImageQuality),
        TagInfo(0x0104, "FlashExposureComp", N_("Flash Exposure Compensation"),
                N_("Flash exposure compensation in EV"),
                sony1Id, makerTags, signedRational, -1, print0x9204),
        TagInfo(0x0105, "Teleconverter", N_("Teleconverter Model"),
                N_("Teleconverter Model"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyTeleconverterModel),
        TagInfo(0x0112, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune Value"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0114, "CameraSettings", N_("Camera Settings"),
                N_("Camera Settings"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x0115, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyWhiteBalanceStd),
        TagInfo(0x0116, "0x0116", "0x0116",
                N_("Unknown"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x0E00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x1000, "MultiBurstMode", N_("Multi Burst Mode"),
                N_("Multi Burst Mode"),
                sony1Id, makerTags, undefined, -1, printMinoltaSonyBoolValue),
        TagInfo(0x1001, "MultiBurstImageWidth", N_("Multi Burst Image Width"),
                N_("Multi Burst Image Width"),
                sony1Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x1002, "MultiBurstImageHeight", N_("Multi Burst Image Height"),
                N_("Multi Burst Image Height"),
                sony1Id, makerTags, unsignedShort, -1, printValue),
        // TODO : Implement Panorama tags decoding.
        TagInfo(0x1003, "Panorama", N_("Panorama"),
                N_("Panorama"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2000, "0x2000", "0x2000",
                N_("Unknown"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2001, "PreviewImage", N_("Preview Image"),
                N_("JPEG preview image"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2002, "0x2002", "0x2002",
                N_("Unknown"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x2003, "0x2003", "0x2003",
                N_("Unknown"),
                sony1Id, makerTags, asciiString, -1, printValue),
        TagInfo(0x2004, "Contrast", "Contrast",
                N_("Contrast"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2005, "Saturation", "Saturation",
                N_("Saturation"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2006, "0x2006", "0x2006",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2007, "0x2007", "0x2007",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2008, "0x2008", "0x2008",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2009, "0x2009", "0x2009",
                N_("Unknown"),
                sony1Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x200A, "AutoHDR", N_("Auto HDR"),
                N_("High Definition Range Mode"),
                sony1Id, makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyHDRMode)),
        // TODO : Implement Shot Info tags decoding.
        TagInfo(0x3000, "ShotInfo", N_("Shot Info"),
                N_("Shot Information"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0xB000, "FileFormat", N_("File Format"),
                N_("File Format"),
                sony1Id, makerTags, unsignedByte, -1, print0xb000),
        TagInfo(0xB001, "SonyModelID", N_("Sony Model ID"),
                N_("Sony Model ID"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyModelId)),
        TagInfo(0xB020, "ColorReproduction", N_("Color Reproduction"),
                N_("Color Reproduction"),
                sony1Id, makerTags, asciiString, -1, printValue),
        TagInfo(0xb021, "ColorTemperature", N_("Color Temperature"),
                N_("Color Temperature"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xB022, "ColorCompensationFilter", N_("Color Compensation Filter"),
                N_("Color Compensation Filter: negative is green, positive is magenta"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xB023, "SceneMode", N_("Scene Mode"),
                N_("Scene Mode"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonySceneMode),
        TagInfo(0xB024, "ZoneMatching", N_("Zone Matching"),
                N_("Zone Matching"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyZoneMatching),
        TagInfo(0xB025, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sony1Id, makerTags, unsignedLong, -1, EXV_PRINT_TAG(print0xb025)),
        TagInfo(0xB026, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyBoolValue),
        TagInfo(0xB027, "LensID", N_("Lens ID"),
                N_("Lens identifier"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyLensID),
        TagInfo(0xB028, "MinoltaMakerNote", N_("Minolta MakerNote"),
                N_("Minolta MakerNote"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0xB029, "ColorMode", N_("Color Mode"),
                N_("Color Mode"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyColorMode),
        TagInfo(0xB02B, "FullImageSize", N_("Full Image Size"),
                N_("Full Image Size"),
                sony1Id, makerTags, unsignedLong, -1, printImageSize),
        TagInfo(0xB02C, "PreviewImageSize", N_("Preview Image Size"),
                N_("Preview image size"),
                sony1Id, makerTags, unsignedLong, -1, printImageSize),
        TagInfo(0xB040, "Macro", N_("Macro"),
                N_("Macro"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyMacroMode)),
        TagInfo(0xB041, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyExposureMode)),
        TagInfo(0xB042, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyFocusMode)),
        TagInfo(0xB043, "AFMode", N_("AF Mode"),
                N_("AF Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAFMode)),
        TagInfo(0xB044, "AFIlluminator", N_("AF Illuminator"),
                N_("AF Illuminator"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAFIlluminator)),
        TagInfo(0xB047, "JPEGQuality", N_("JPEG Quality"),
                N_("JPEG Quality"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyJPEGQuality)),
        TagInfo(0xB048, "FlashLevel", N_("Flash Level"),
                N_("Flash Level"),
                sony1Id, makerTags, signedShort, -1, EXV_PRINT_TAG(sonyFlashLevel)),
        TagInfo(0xB049, "ReleaseMode", N_("Release Mode"),
                N_("Release Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyReleaseMode)),
        TagInfo(0xB04A, "SequenceNumber", N_("Sequence Number"),
                N_("Shot number in continous burst mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonySequenceNumber)),
        TagInfo(0xB04B, "AntiBlur", N_("Anti-Blur"),
                N_("Anti-Blur"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAntiBlur)),
        TagInfo(0xB04E, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"),
                N_("Long Exposure Noise Reduction"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyLongExposureNoiseReduction)),
        TagInfo(0xB04F, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(print0xb04f)),
        TagInfo(0xB052, "IntelligentAuto", N_("Intelligent Auto"),
                N_("Intelligent Auto"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyIntelligentAuto)),
        TagInfo(0xB054, "WhiteBalance2", N_("White Balance 2"),
                N_("White balance 2"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyWhiteBalance)),
        // End of list marker
        TagInfo(0xffff, "(UnknownSony1MakerNoteTag)", "(UnknownSony1MakerNoteTag)",
                N_("Unknown Sony1MakerNote tag"),
                sony1Id, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* SonyMakerNote::tagList()
    {
        return tagInfo_;
    }

    // -- Sony camera settings ---------------------------------------------------------------

    //! Lookup table to translate Sony camera settings drive mode values to readable labels
    extern const TagDetails sonyDriveModeStd[] = {
        { 1,  N_("Single Frame")                     },
        { 2,  N_("Continuous High")                  },
        { 4,  N_("Self-timer 10 sec")                },
        { 5,  N_("Self-timer 2 sec")                 },
        { 7,  N_("Continuous Bracketing")            },
        { 12, N_("Continuous Low")                   },
        { 18, N_("White Balance Bracketing Low")     },
        { 19, N_("D-Range Optimizer Bracketing Low") },
        { 19, N_("D-Range Optimizer Bracketing Low") } // To silence compiler warning
    };

    //! Lookup table to translate Sony camera settings focus mode values to readable labels
    extern const TagDetails sonyCSFocusMode[] = {
        { 0, N_("Manual") },
        { 1, "AF-S"       },
        { 2, "AF-C"       },
        { 3, "AF-A"       }
    };

    //! Lookup table to translate Sony camera settings metering mode values to readable labels
    extern const TagDetails sonyMeteringMode[] = {
        { 1, N_("Multi-segment")           },
        { 2, N_("Center weighted average") },
        { 4, N_("Spot")                    }
    };

    //! Lookup table to translate Sony camera settings creative style values to readable labels
    extern const TagDetails sonyCreativeStyle[] = {
        { 1,    N_("Standard")            },
        { 2,    N_("Vivid")               },
        { 3,    N_("Portrait")            },
        { 4,    N_("Landscape")           },
        { 5,    N_("Sunset")              },
        { 6,    N_("Night View/Portrait") },
        { 8,    N_("Black & White")       },
        { 9,    N_("Adobe RGB")           },
        { 11,   N_("Neutral")             },
        { 12,   N_("Clear")               },
        { 13,   N_("Deep")                },
        { 14,   N_("Light")               },
        { 15,   N_("Autumn")              },
        { 16,   N_("Sepia")               }
    };

    //! Lookup table to translate Sony camera settings flash mode values to readable labels
    extern const TagDetails sonyFlashMode[] = {
        { 0, N_("ADI") },
        { 1, N_("TTL") },
    };

    //! Lookup table to translate Sony AF illuminator values to readable labels
    extern const TagDetails sonyAFIlluminatorCS[] = {
        { 0, N_("Auto") },
        { 1, N_("Off")  }
    };

    //! Lookup table to translate Sony camera settings image style values to readable labels
    extern const TagDetails sonyImageStyle[] = {
        { 1,    N_("Standard")      },
        { 2,    N_("Vivid")         },
        { 9,    N_("Adobe RGB")     },
        { 11,   N_("Neutral")       },
        { 129,  N_("StyleBox1")     },
        { 130,  N_("StyleBox1")     },
        { 131,  N_("StyleBox1")     }
    };

    //! Lookup table to translate Sony camera settings exposure program values to readable labels
    extern const TagDetails sonyExposureProgram[] = {
        { 0,    N_("Auto")                      },
        { 1,    N_("Manual")                    },
        { 2,    N_("Program AE")                },
        { 3,    N_("Aperture-priority AE")      },
        { 4,    N_("Shutter speed priority AE") },
        { 8,    N_("Program Shift A")           },
        { 9,    N_("Program Shift S")           },
        { 16,   N_("Portrait")                  },
        { 17,   N_("Sports")                    },
        { 18,   N_("Sunset")                    },
        { 19,   N_("Night Portrait")            },
        { 20,   N_("Landscape")                 },
        { 21,   N_("Macro")                     },
        { 35,   N_("Auto No Flash")             }
    };

    //! Lookup table to translate Sony camera settings image size values to readable labels
    extern const TagDetails sonyImageSize[] = {
        { 1, N_("Large")  },
        { 2, N_("Medium") },
        { 3, N_("Small")  }
    };

    //! Lookup table to translate Sony aspect ratio values to readable labels
    extern const TagDetails sonyAspectRatio[] = {
        { 1, "3:2"   },
        { 2, "16:9"  }
    };

    //! Lookup table to translate Sony exposure level increments values to readable labels
    extern const TagDetails sonyExposureLevelIncrements[] = {
        { 33, "1/3 EV" },
        { 50, "1/2 EV" }
    };

    // Sony Camera Settings Tag Info
    // NOTE: all are for A200, A230, A300, A350, A700, A850 and A900 Sony model excepted
    // some entries which are only relevant with A700.

    // Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

    const TagInfo SonyMakerNote::tagInfoCs_[] = {
        // NOTE: A700 only
        TagInfo(0x0004, "DriveMode", N_("Drive Mode"),
                N_("Drive Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyDriveModeStd)),
        // NOTE: A700 only
        TagInfo(0x0006, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune"),
                sony1CsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0010, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCSFocusMode)),
        TagInfo(0x0011, "AFAreaMode", N_("AF Area Mode"),
                N_("AF Area Mode"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyAFAreaMode),
        TagInfo(0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"),
                N_("Local AF Area Point"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint),
        TagInfo(0x0015, "MeteringMode", N_("Metering Mode"),
                N_("Metering Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)),
        TagInfo(0x0016, "ISOSetting", N_("ISO Setting"),
                N_("ISO Setting"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0018, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"),
                N_("Dynamic Range Optimizer Mode"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode),
        TagInfo(0x0019, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"),
                N_("Dynamic Range Optimizer Level"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001A, "CreativeStyle", N_("Creative Style"),
                N_("Creative Style"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)),
        TagInfo(0x001C, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001D, "Contrast", N_("Contrast"),
                N_("Contrast"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001E, "Saturation", N_("Saturation"),
                N_("Saturation"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001F, "ZoneMatchingValue", N_("Zone Matching Value"),
                N_("Zone Matching Value"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0022, "Brightness", N_("Brightness"),
                N_("Brightness"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0023, "FlashMode", N_("FlashMode"),
                N_("FlashMode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashMode)),
        // NOTE: A700 only
        TagInfo(0x0028, "PrioritySetupShutterRelease", N_("Priority Setup Shutter Release"),
                N_("Priority Setup Shutter Release"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyPrioritySetupShutterRelease),
        // NOTE: A700 only
        TagInfo(0x0029, "AFIlluminator", N_("AF Illuminator"),
                N_("AF Illuminator"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAFIlluminatorCS)),
        // NOTE: A700 only
        TagInfo(0x002A, "AFWithShutter", N_("AF With Shutter"),
                N_("AF With Shutter"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue),
        // NOTE: A700 only
        TagInfo(0x002B, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"),
                N_("Long Exposure Noise Reduction"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        // NOTE: A700 only
        TagInfo(0x002C, "HighISONoiseReduction", N_("High ISO NoiseReduction"),
                N_("High ISO NoiseReduction"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        // NOTE: A700 only
        TagInfo(0x002D, "ImageStyle", N_("Image Style"),
                N_("Image Style"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageStyle)),
        TagInfo(0x003C, "ExposureProgram", N_("Exposure Program"),
                N_("Exposure Program"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)),
        TagInfo(0x003D, "ImageStabilization", N_("Image Stabilization"),
                N_("Image Stabilization"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x003F, "Rotation", N_("Rotation"),
                N_("Rotation"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyRotation),
        TagInfo(0x0054, "SonyImageSize", N_("Sony Image Size"),
                N_("Sony Image Size"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)),
        TagInfo(0x0055, "AspectRatio", N_("Aspect Ratio"),
                N_("Aspect Ratio"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAspectRatio)),
        TagInfo(0x0056, "Quality", N_("Quality"),
                N_("Quality"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyQualityCs),
        TagInfo(0x0058, "ExposureLevelIncrements", N_("Exposure Level Increments"),
                N_("Exposure Level Increments"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureLevelIncrements)),
        // End of list marker
        TagInfo(0xffff, "(UnknownSony1CsTag)", "(UnknownSony1CsTag)",
                N_("Unknown Sony1 Camera Settings tag"),
                sony1CsId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* SonyMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }

    // -- Sony camera settings 2 ---------------------------------------------------------------

    // Sony Camera Settings Tag Version 2 Info
    // NOTE: for A330, A380, A450, A500, A550 Sony model

    // Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

    const TagInfo SonyMakerNote::tagInfoCs2_[] = {
        TagInfo(0x0010, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCSFocusMode)),
        TagInfo(0x0011, "AFAreaMode", N_("AF Area Mode"),
                N_("AF Area Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyAFAreaMode),
        TagInfo(0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"),
                N_("Local AF Area Point"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint),
        TagInfo(0x0013, "MeteringMode", N_("Metering Mode"),
                N_("Metering Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)),
        TagInfo(0x0014, "ISOSetting", N_("ISO Setting"),
                N_("ISO Setting"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0016, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"),
                N_("Dynamic Range Optimizer Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode),
        TagInfo(0x0017, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"),
                N_("Dynamic Range Optimizer Level"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0018, "CreativeStyle", N_("Creative Style"),
                N_("Creative Style"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)),
        TagInfo(0x0019, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001A, "Contrast", N_("Contrast"),
                N_("Contrast"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001B, "Saturation", N_("Saturation"),
                N_("Saturation"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0023, "FlashMode", N_("FlashMode"),
                N_("FlashMode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashMode)),
        TagInfo(0x003C, "ExposureProgram", N_("Exposure Program"),
                N_("Exposure Program"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)),
        TagInfo(0x003F, "Rotation", N_("Rotation"),
                N_("Rotation"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyRotation),
        TagInfo(0x0054, "SonyImageSize", N_("Sony Image Size"),
                N_("Sony Image Size"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)),
        // End of list marker
        TagInfo(0xffff, "(UnknownSony1Cs2Tag)", "(UnknownSony1Cs2Tag)",
                N_("Unknown Sony1 Camera Settings 2 tag"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* SonyMakerNote::tagListCs2()
    {
        return tagInfoCs2_;
    }

}}                                      // namespace Internal, Exiv2
