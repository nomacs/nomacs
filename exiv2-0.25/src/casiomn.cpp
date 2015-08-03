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
/*
  File:      Casiomn.cpp
  Version:   $Rev: 3091 $
  History:   30-Oct-13, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: Casiomn.cpp 3091 2013-07-24 05:15:04Z robinwmills $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "casiomn_int.hpp"
#include "tags_int.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <vector>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    //! RecordingMode, tag 0x0001
    extern const TagDetails casioRecordingMode[] = {
        {  1, N_("Single Shutter") },
        {  2, N_("Panorama")       },
        {  3, N_("Night Scene")    },
        {  4, N_("Portrait")       },
        {  5, N_("Landscape")      },
        {  7, N_("Panorama")       },
        { 10, N_("Night Scene")    },
        { 15, N_("Portrait")       },
        { 16, N_("Landscape")      }
    };

    //! Quality, tag 0x0002
    extern const TagDetails casioQuality[] = {
        { 1, N_("Economy") },
        { 2, N_("Normal")  },
        { 3, N_("Fine")    }
    };

    //! Focus Mode, tag 0x0003
    extern const TagDetails casioFocusMode[] = {
        { 2, N_("Macro")    },
        { 3, N_("Auto")     },
        { 4, N_("Manual")   },
        { 5, N_("Infinity") },
        { 7, N_("Sport AF") }
    };

    //! FlashMode, tag 0x0004
    extern const TagDetails casioFlashMode[] = {
        { 1, N_("Auto")              },
        { 2, N_("On")                },
        { 3, N_("Off")               },
        { 4, N_("Off")               },
        { 5, N_("Red-eye Reduction") }
    };

    //! Flash intensity, tag 0x0005
    extern const TagDetails casioFlashIntensity[] = {
        { 11, N_("Weak")   },
        { 12, N_("Low")    },
        { 13, N_("Normal") },
        { 14, N_("High")   },
        { 15, N_("Strong") }
    };

    //! white balance, tag 0x0007
    extern const TagDetails casioWhiteBalance[] = {
        {   1, N_("Auto")        },
        {   2, N_("Tungsten")    },
        {   3, N_("Daylight")    },
        {   4, N_("Fluorescent") },
        {   5, N_("Shade")       },
        { 129, N_("Manual")      }
    };

    //! Flash intensity, tag 0x0005
    extern const TagDetails casioDigitalZoom[] = {
        { 0x10000, N_("Off")   },
        { 0x10001, N_("2x")    },
        { 0x13333, N_("1.2x")  },
        { 0x13ae1, N_("1.23x") },
        { 0x19999, N_("1.6x")  },
        { 0x20000, N_("2x")    },
        { 0x33333, N_("3.2x")  },
        { 0x40000, N_("4x")    }
    };

    //! Sharpness, tag 0x000b
    extern const TagDetails casioSharpness[] = {
        {  0, N_("Normal") },
        {  1, N_("Soft")   },
        {  2, N_("Hard")   },
        { 16, N_("Normal") },
        { 17, N_("+1")     },
        { 18, N_("-1")     }
    };
        
    //! Contrast, tag 0x000c
    extern const TagDetails casioContrast[] = {
        {  0, N_("Normal") },
        {  1, N_("Low")   },
        {  2, N_("High")   },
        { 16, N_("Normal") },
        { 17, N_("+1")     },
        { 18, N_("-1")     }
    };

    //! Saturation, tag 0x000d
    extern const TagDetails casioSaturation[] = {
        {  0, N_("Normal") },
        {  1, N_("Low")   },
        {  2, N_("High")   },
        { 16, N_("Normal") },
        { 17, N_("+1")     },
        { 18, N_("-1")     }
    };

    //! Enhancement, tag 0x0016
    extern const TagDetails casioEnhancement[] = {
        { 1, N_("Off")         },
        { 2, N_("Red")         },
        { 3, N_("Green")       },
        { 4, N_("Blue")        },
        { 5, N_("Flesh Tones") }
    };

    //! Color filter, tag 0x0017
    extern const TagDetails casioColorFilter[] = {
        { 1, N_("Off")           },
        { 2, N_("Black & White") },
        { 3, N_("Sepia")         },
        { 4, N_("Red")           },
        { 5, N_("Green")         },
        { 6, N_("Blue")          },
        { 7, N_("Yellow")        },
        { 8, N_("Pink")          },
        { 9, N_("Purple")        }
    };

    //! flash intensity 2, tag 0x0019
    extern const TagDetails casioFlashIntensity2[] = {
        { 1, N_("Normal") },
        { 2, N_("Weak")   },
        { 3, N_("Strong") }
    };

    //! CCD Sensitivity intensity, tag 0x0020
    extern const TagDetails casioCCDSensitivity[] = {
        {  64, N_("Normal")                     },
        { 125, N_("+1.0")                       },
        { 250, N_("+2.0")                       },
        { 244, N_("+3.0")                       },
        {  80, N_("Normal (ISO 80 equivalent)") },
        { 100, N_("High")                       }
    };

    // Casio MakerNote Tag Info
    const TagInfo CasioMakerNote::tagInfo_[] = {
        TagInfo(0x0001, "RecodingMode", N_("RecodingMode"), N_("Recording Mode"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioRecordingMode)),
        TagInfo(0x0002, "Quality", N_("Quality"), N_("Quality"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioQuality)),
        TagInfo(0x0003, "FocusMode", N_("Focus Mode"), N_("Focus Mode"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioFocusMode)),
        TagInfo(0x0004, "FlashMode", N_("Flash Mode"), N_("Flash Mode"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioFlashMode)),
        TagInfo(0x0005, "FlashIntensity", N_("Flash Intensity"), N_("Flash Intensity"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioFlashIntensity)),
        TagInfo(0x0006, "ObjectDistance", N_("Object Distance"), N_("Distance to object"), casioId, makerTags, unsignedLong, -1, print0x0006),
        TagInfo(0x0007, "WhiteBalance", N_("White Balance"), N_("White balance settings"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioWhiteBalance)),
        TagInfo(0x000a, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), casioId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(casioDigitalZoom)),
        TagInfo(0x000b, "Sharpness", N_("Sharpness"), N_("Sharpness"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioSharpness)),
        TagInfo(0x000c, "Contrast", N_("Contrast"), N_("Contrast"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioContrast)),
        TagInfo(0x000d, "Saturation", N_("Saturation"), N_("Saturation"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioSaturation)),
        TagInfo(0x0014, "ISO", N_("ISO"), N_("ISO"), casioId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0015, "FirmwareDate", N_("Firmware date"), N_("Firmware date"), casioId, makerTags, asciiString, -1, print0x0015),
        TagInfo(0x0016, "Enhancement", N_("Enhancement"), N_("Enhancement"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioEnhancement)),
        TagInfo(0x0017, "ColorFilter", N_("Color Filter"), N_("Color Filter"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioColorFilter)),
        TagInfo(0x0018, "AFPoint", N_("AF Point"), N_("AF Point"), casioId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0019, "FlashIntensity2", N_("Flash Intensity"), N_("Flash Intensity"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioFlashIntensity2)),
        TagInfo(0x0020, "CCDSensitivity", N_("CCDSensitivity"), N_("CCDSensitivity"), casioId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casioCCDSensitivity)),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), casioId, makerTags, undefined, -1, printValue),
        TagInfo(0xffff, "(UnknownCasioMakerNoteTag)", "(UnknownCasioMakerNoteTag)", N_("Unknown CasioMakerNote tag"), casioId, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* CasioMakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& CasioMakerNote::print0x0006(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2) << value.toLong() / 1000.0 << _(" m");
        os.copyfmt(oss);
        os.flags(f);
        return os;
    } 

    std::ostream& CasioMakerNote::print0x0015(std::ostream& os, const Value& value, const ExifData*)
    {
        // format is:  "YYMM#00#00DDHH#00#00MM#00#00#00#00" or  "YYMM#00#00DDHH#00#00MMSS#00#00#00"
        std::string s;
        std::vector<char> numbers;
        for(long i=0; i<value.size(); i++)
        {
            long l=value.toLong(i);
            if(l!=0)
            {
                numbers.push_back((char)l);
            };
        };
        if(numbers.size()>=10)
        {
            //year
            long l=(numbers[0]-48)*10+(numbers[1]-48);
            if(l<70)
            {
                l+=2000;
            }
            else
            {
                l+=1900;
            };
            os << l << ":";
            // month, day, hour, minutes
            os << numbers[2] << numbers[3] << ":" << numbers[4] << numbers[5] << " " << numbers[6] << numbers[7] << ":" << numbers[8] << numbers[9];
            // optional seconds
            if(numbers.size()==12)
            {
                os << ":" << numbers[10] << numbers[11];
            };
        }
        else
        {
            os << value;
        };
        return os;
    } 

    //Casio Makernotes, Type 2
    //! Quality Mode, tag 0x0004
    extern const TagDetails casio2QualityMode[] = {
        { 0, N_("Economy") },
        { 1, N_("Normal")  },
        { 2, N_("Fine")    }
    };

    //! Image Size, tag 0x0009
    extern const TagDetails casio2ImageSize[] = {
        {  0, "640x480"   },
        {  4, "1600x1200" },
        {  5, "2048x1536" },
        { 20, "2288x1712" },
        { 21, "2592x1944" },
        { 22, "2304x1728" },
        { 36, "3008x2008" }
    };

    //! Focus Mode, tag 0x000d
    extern const TagDetails casio2FocusMode[] = {
        { 0, N_("Normal") },
        { 1, N_("Macro") }
    };

    //! ISO Speed, tag 0x0014
    extern const TagDetails casio2IsoSpeed[] = {
        { 3, "50"  },
        { 4, "64"  },
        { 6, "100" },
        { 9, "200" }
    };

    //! White Balance, tag 0x0019
    extern const TagDetails casio2WhiteBalance[] = {
        { 0, N_("Auto")        },
        { 1, N_("Daylight")    },
        { 2, N_("Shade")       },
        { 3, N_("Tungsten")    },
        { 4, N_("Fluorescent") },
        { 5, N_("Manual")      }
    };

    //! Saturation, tag 0x001f
    extern const TagDetails casio2Saturation[] = {
        { 0, N_("Low")    },
        { 1, N_("Normal") },
        { 2, N_("High")   }
    };

    //! Contrast, tag 0x0020
    extern const TagDetails casio2Contrast[] = {
        { 0, N_("Low")    },
        { 1, N_("Normal") },
        { 2, N_("High")   }
    };

    //! Sharpness, tag 0x0021
    extern const TagDetails casio2Sharpness[] = {
        { 0, N_("Soft")    },
        { 1, N_("Normal") },
        { 2, N_("Hard")   }
    };

    //! White Balance2, tag 0x2012
    extern const TagDetails casio2WhiteBalance2[] = {
        {  0, N_("Manual")      },
        {  1, N_("Daylight")    },
        {  2, N_("Cloudy")      },
        {  3, N_("Shade")       },
        {  4, N_("Flash")       },
        {  6, N_("Fluorescent") },
        {  9, N_("Tungsten")    },
        { 10, N_("Tungsten")    },
        { 12, N_("Flash")       }
    };

    //! Release Mode, tag 0x3001
    extern const TagDetails casio2ReleaseMode[] = {
        {  1, N_("Normal")              },
        {  3, N_("AE Bracketing")       },
        { 11, N_("WB Bracketing")       },
        { 13, N_("Contrast Bracketing") },
        { 19, N_("High Speed Burst")    }
    };

    //! Quality, tag 0x3002
    extern const TagDetails casio2Quality[] = {
        { 1, N_("Economy") },
        { 2, N_("Normal")  },
        { 3, N_("Fine")    }
    };

    //! Focus Mode 2, tag 0x3003
    extern const TagDetails casio2FocusMode2[] = {
        { 0, N_("Manual")                 },
        { 1, N_("Focus Lock")             },
        { 2, N_("Macro")                  },
        { 3, N_("Single-Area Auto Focus") },
        { 5, N_("Infinity")               },
        { 6, N_("Multi-Area Auto Focus")  },
        { 8, N_("Super Macro")            }
    };

    //! AutoISO, tag 0x3008
    extern const TagDetails casio2AutoISO[] = {
        {  1, N_("On")                   },
        {  2, N_("Off")                  },
        {  7, N_("On (high sensitiviy)") },
        {  8, N_("On (anti-shake)")      },
        { 10, N_("High Speed")           }
    };

    //! AFMode, tag 0x3009
    extern const TagDetails casio2AFMode[] = {
        { 0, N_("Off")            },
        { 1, N_("Spot")           },
        { 2, N_("Multi")          },
        { 3, N_("Face Detection") },
        { 4, N_("Tracking")       },
        { 5, N_("Intelligent")    }
    };

    //! ColorMode, tag 0x3015
    extern const TagDetails casio2ColorMode[] = {
        { 0, N_("Off")           },
        { 2, N_("Black & White") },
        { 3, N_("Sepia")         }
    };

    //! Enhancement, tag 0x3016
    extern const TagDetails casio2Enhancement[] = {
        { 0, N_("Off")         },
        { 1, N_("Scenery")     },
        { 3, N_("Green")       },
        { 5, N_("Unterwater")  },
        { 9, N_("Flesh Tones") }

    };

    //! Color Filter, tag 0x3017
    extern const TagDetails casio2ColorFilter[] = {
        { 0, N_("Off")    },
        { 1, N_("Blue")   },
        { 3, N_("Green")  },
        { 4, N_("Yellow") },
        { 5, N_("Red")    },
        { 6, N_("Purple") },
        { 7, N_("Pink")   }
    };

    //! Art Mode, tag 0x301b
    extern const TagDetails casio2ArtMode[] = {
        {  0, N_("Normal")                },
        {  8, N_("Silent Movie")          },
        { 39, N_("HDR")                   },
        { 45, N_("Premium Auto")          },
        { 47, N_("Painting")              },
        { 49, N_("Crayon Drawing")        },
        { 51, N_("Panorama")              },
        { 52, N_("Art HDR")               },
        { 62, N_("High Speed Night Shot") },
        { 64, N_("Monochrome")            },
        { 67, N_("Toy Camera")            },
        { 68, N_("Pop Art")               },
        { 69, N_("Light Tone")            }
    };

    //! Lighting Mode, tag 0x302a
    extern const TagDetails casio2LightingMode[] = {
        { 0, N_("Off")                 },
        { 1, N_("High Dynamic Range")  },
        { 5, N_("Shadow Enhance Low")  },
        { 6, N_("Shadow Enhance High") }
    };
        
    //! Portrait Refiner, tag 0x302b
    extern const TagDetails casio2PortraitRefiner[] = {
        { 0, N_("Off") },
        { 1, N_("+1")  },
        { 2, N_("+2")  }
    };

    //! Special Effect Setting, tag 0x3031
    extern const TagDetails casio2SpecialEffectSetting[] = {
        {  0, N_("Off")             },
        {  1, N_("Makeup")          },
        {  2, N_("Mist Removal")    },
        {  3, N_("Vivid Landscape") },
        { 16, N_("Art Shot")        }
    };

    //! Drive Mode, tag 0x3103
    extern const TagDetails casio2DriveMode[] = {
        {   0, N_("Single Shot")         },
        {   1, N_("Continuous Shooting") },
        {   2, N_("Continuous (2 fps)")  },
        {   3, N_("Continuous (3 fps)")  },
        {   4, N_("Continuous (4 fps)")  },
        {   5, N_("Continuous (5 fps)")  },
        {   6, N_("Continuous (6 fps)")  },
        {   7, N_("Continuous (7 fps)")  },
        {  10, N_("Continuous (10 fps)") },
        {  12, N_("Continuous (12 fps)") },
        {  15, N_("Continuous (15 fps)") },
        {  20, N_("Continuous (20 fps)") },
        {  30, N_("Continuous (30 fps)") },
        {  40, N_("Continuous (40 fps)") },
        {  60, N_("Continuous (60 fps)") },
        { 240, N_("Auto-N")              }
    };

    //! Video Quality, tag 0x4003
    extern const TagDetails casio2VideoQuality[] = {
        { 1, N_("Standard")        },
        { 3, N_("HD (720p)")       },
        { 4, N_("Full HD (1080p)") },
        { 5, N_("Low")             }
    };

    // Casio2 MakerNote Tag Info
    const TagInfo Casio2MakerNote::tagInfo_[] = {
        TagInfo(0x0002, "PreviewImageSize", N_("Preview Image Size"), N_("Preview Image Size"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0003, "PreviewImageLength", N_("Preview Image Length"), N_("Preview Image Length"), casio2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0004, "PreviewImageStart", N_("Preview Image Start"), N_("Preview Image Start"), casio2Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0008, "QualityMode", N_("Quality Mode"), N_("Quality Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2QualityMode)),
        TagInfo(0x0009, "ImageSize", N_("Image Size"), N_("Image Size"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2ImageSize)),
        TagInfo(0x000d, "FocusMode", N_("Focus Mode"), N_("Focus Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2FocusMode)),
        TagInfo(0x0014, "ISOSpeed", N_("ISO Speed"), N_("ISO Speed"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2IsoSpeed)),
        TagInfo(0x0019, "WhiteBalance", N_("White Balance"), N_("White Balance Setting"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2WhiteBalance)),
        TagInfo(0x001d, "FocalLength", N_("Focal Length"), N_("Focal Length"), casio2Id, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x001f, "Saturation", N_("Saturation"), N_("Saturation"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2Saturation)),
        TagInfo(0x0020, "Contrast", N_("Contrast"), N_("Contrast"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2Contrast)),
        TagInfo(0x0021, "Sharpness", N_("Sharpness"), N_("Sharpness"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2Sharpness)),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), casio2Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2000, "PreviewImage", N_("Preview Image"), N_("Preview Image"), casio2Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2001, "FirmwareDate", N_("Firmware Date"), N_("Firmware Date"), casio2Id, makerTags, asciiString, -1, print0x2001),
        TagInfo(0x2011, "WhiteBalanceBias", N_("White Balance Bias"), N_("White Balance Bias"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x2012, "WhiteBalance2", N_("White Balance"), N_("White Balance Setting"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2WhiteBalance2)),
        TagInfo(0x2021, "AFPointPosition", N_("AF Point Position"), N_("AF Point Position"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x2022, "ObjectDistance", N_("Object Distance"), N_("Object Distance"), casio2Id, makerTags, unsignedLong, -1, print0x2022),
        TagInfo(0x2034, "FlashDistance", N_("Flash Distance"), N_("Flash Distance"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x2076, "SpecialEffectMode", N_("Special Effect Mode"), N_("Special Effect Mode"), casio2Id, makerTags, unsignedByte, -1, printValue),
        TagInfo(0x2089, "FaceInfo", N_("Face Info"), N_("Face Info"), casio2Id, makerTags, undefined, -1, printValue),
        TagInfo(0x211c, "FacesDetected", N_("Faces detected"), N_("Faces detected"), casio2Id, makerTags, unsignedByte, -1, printValue),
        TagInfo(0x3000, "RecordMode", N_("Record Mode"), N_("Record Mode"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x3001, "ReleaseMode", N_("Release Mode"), N_("Release Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2ReleaseMode)),
        TagInfo(0x3002, "Quality", N_("Quality"), N_("Quality"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2Quality)),
        TagInfo(0x3003, "FocusMode2", N_("Focus Mode2"), N_("Focus Mode2"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2FocusMode2)),
        TagInfo(0x3006, "HometownCity", N_("Home town city"), N_("Home town city"), casio2Id, makerTags, asciiString, -1, printValue),
        TagInfo(0x3007, "BestShotMode", N_("Best Shot Mode"), N_("Best Shot Mode"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x3008, "AutoISO", N_("Auto ISO"), N_("Auto ISO"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2AutoISO)),
        TagInfo(0x3009, "AFMode", N_("AF Mode"), N_("AF Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2AFMode)),
        TagInfo(0x3011, "Sharpness2", N_("Sharpness"), N_("Sharpness"), casio2Id, makerTags, undefined, -1, printValue),
        TagInfo(0x3012, "Contrast2", N_("Contrast"), N_("Contrast"), casio2Id, makerTags, undefined, -1, printValue),
        TagInfo(0x3013, "Saturation2", N_("Saturation"), N_("Saturation"), casio2Id, makerTags, undefined, -1, printValue),
        TagInfo(0x3014, "ISO", N_("ISO"), N_("ISO"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x3015, "ColorMode", N_("Color Mode"), N_("Color Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2ColorMode)),
        TagInfo(0x3016, "Enhancement", N_("Enhancement"), N_("Enhancement"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2Enhancement)),
        TagInfo(0x3017, "ColorFilter", N_("Color Filter"), N_("Color Filter"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2ColorFilter)),
        TagInfo(0x301b, "ArtMode", N_("Art Mode"), N_("Art Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2ArtMode)),
        TagInfo(0x301c, "SequenceNumber", N_("Sequence Number"), N_("Sequence Number"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x3020, "ImageStabilization", N_("Image Stabilization"), N_("Image Stabilization"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x302a, "LightingMode", N_("Lighting Mode"), N_("Lighting Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2LightingMode)),
        TagInfo(0x302b, "PortraitRefiner", N_("Portrait Refiner"), N_("Portrait Refiner settings"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2PortraitRefiner)),
        TagInfo(0x3030, "SpecialEffectLevel", N_("Special Effect Level"), N_("Special Effect Level"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x3031, "SpecialEffectSetting", N_("Special Effect Setting"), N_("Special Effect Setting"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2SpecialEffectSetting)),
        TagInfo(0x3103, "DriveMode", N_("Drive Mode"), N_("Drive Mode"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2DriveMode)),
        TagInfo(0x310b, "ArtModeParameters", N_("Art Mode Parameters"), N_("Art Mode Parameters"), casio2Id, makerTags, undefined, -1, printValue),
        TagInfo(0x4001, "CaptureFrameRate", N_("Capture Frame Rate"), N_("Capture Frame Rate"), casio2Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x4003, "VideoQuality", N_("Video Quality"), N_("Video Quality"), casio2Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2VideoQuality)),
        TagInfo(0xffff, "(UnknownCasio2MakerNoteTag)", "(UnknownCasio2MakerNoteTag)", N_("Unknown Casio2MakerNote tag"), casio2Id, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* Casio2MakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& Casio2MakerNote::print0x2001(std::ostream& os, const Value& value, const ExifData*)
    {
        // format is:  "YYMM#00#00DDHH#00#00MM#00#00#00#00"
        std::string s;
        std::vector<char> numbers;
        for(long i=0; i<value.size(); i++)
        {
            long l=value.toLong(i);
            if(l!=0)
            {
                numbers.push_back((char)l);
            };
        };
        if(numbers.size()>=10)
        {
            //year
            long l=(numbers[0]-48)*10+(numbers[1]-48);
            if(l<70)
            {
                l+=2000;
            }
            else
            {
                l+=1900;
            };
            os << l << ":";
            // month, day, hour, minutes
            os << numbers[2] << numbers[3] << ":" << numbers[4] << numbers[5] << " " << numbers[6] << numbers[7] << ":" << numbers[8] << numbers[9];
        }
        else
        {
            os << value;
        };
        return os;
    } 

    std::ostream& Casio2MakerNote::print0x2022(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if(value.toLong()>=0x20000000)
        {
            os << N_("Inf");
            os.flags(f);
            return os;
        };
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2) << value.toLong() / 1000.0 << _(" m");
        os.copyfmt(oss);
        os.flags(f);
        return os;
    } 

}}                                      // namespace Internal, Exiv2
