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
  File:      canonmn.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             David Cannings (dc) <david@edeca.net>
             Andi Clemens (ac) <andi.clemens@gmx.net>
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: canonmn.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "canonmn_int.hpp"
#include "tags_int.hpp"
#include "value.hpp"
#include "exif.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <ios>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    //! OffOn, multiple tags
    extern const TagDetails canonOffOn[] = {
        {  0, N_("Off") },
        {  1, N_("On")  }
    };

    //! Special treatment pretty-print function for non-unique lens ids.
    std::ostream& printCsLensByFocalLength(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata);

    //! ModelId, tag 0x0010
    extern const TagDetails canonModelId[] = {
        { 0x1010000, N_("PowerShot A30") },
        { 0x1040000, N_("PowerShot S300 / Digital IXUS 300 / IXY Digital 300") },
        { 0x1060000, N_("PowerShot A20") },
        { 0x1080000, N_("PowerShot A10") },
        { 0x1090000, N_("PowerShot S110 / Digital IXUS v / IXY Digital 200") },
        { 0x1100000, N_("PowerShot G2") },
        { 0x1110000, N_("PowerShot S40") },
        { 0x1120000, N_("PowerShot S30") },
        { 0x1130000, N_("PowerShot A40") },
        { 0x1140000, N_("EOS D30") },
        { 0x1150000, N_("PowerShot A100") },
        { 0x1160000, N_("PowerShot S200 / Digital IXUS v2 / IXY Digital 200a") },
        { 0x1170000, N_("PowerShot A200") },
        { 0x1180000, N_("PowerShot S330 / Digital IXUS 330 / IXY Digital 300a") },
        { 0x1190000, N_("PowerShot G3") },
        { 0x1210000, N_("PowerShot S45") },
        { 0x1230000, N_("PowerShot SD100 / Digital IXUS II / IXY Digital 30") },
        { 0x1240000, N_("PowerShot S230 / Digital IXUS v3 / IXY Digital 320") },
        { 0x1250000, N_("PowerShot A70") },
        { 0x1260000, N_("PowerShot A60") },
        { 0x1270000, N_("PowerShot S400 / Digital IXUS 400 / IXY Digital 400") },
        { 0x1290000, N_("PowerShot G5") },
        { 0x1300000, N_("PowerShot A300") },
        { 0x1310000, N_("PowerShot S50") },
        { 0x1340000, N_("PowerShot A80") },
        { 0x1350000, N_("PowerShot SD10 / Digital IXUS i / IXY Digital L") },
        { 0x1360000, N_("PowerShot S1 IS") },
        { 0x1370000, N_("PowerShot Pro1") },
        { 0x1380000, N_("PowerShot S70") },
        { 0x1390000, N_("PowerShot S60") },
        { 0x1400000, N_("PowerShot G6") },
        { 0x1410000, N_("PowerShot S500 / Digital IXUS 500 / IXY Digital 500") },
        { 0x1420000, N_("PowerShot A75") },
        { 0x1440000, N_("PowerShot SD110 / Digital IXUS IIs / IXY Digital 30a") },
        { 0x1450000, N_("PowerShot A400") },
        { 0x1470000, N_("PowerShot A310") },
        { 0x1490000, N_("PowerShot A85") },
        { 0x1520000, N_("PowerShot S410 / Digital IXUS 430 / IXY Digital 450") },
        { 0x1530000, N_("PowerShot A95") },
        { 0x1540000, N_("PowerShot SD300 / Digital IXUS 40 / IXY Digital 50") },
        { 0x1550000, N_("PowerShot SD200 / Digital IXUS 30 / IXY Digital 40") },
        { 0x1560000, N_("PowerShot A520") },
        { 0x1570000, N_("PowerShot A510") },
        { 0x1590000, N_("PowerShot SD20 / Digital IXUS i5 / IXY Digital L2") },
        { 0x1640000, N_("PowerShot S2 IS") },
        { 0x1650000, N_("PowerShot SD430 / IXUS Wireless / IXY Wireless") },
        { 0x1660000, N_("PowerShot SD500 / Digital IXUS 700 / IXY Digital 600") },
        { 0x1668000, N_("EOS D60") },
        { 0x1700000, N_("PowerShot SD30 / Digital IXUS i zoom / IXY Digital L3") },
        { 0x1740000, N_("PowerShot A430") },
        { 0x1750000, N_("PowerShot A410") },
        { 0x1760000, N_("PowerShot S80") },
        { 0x1780000, N_("PowerShot A620") },
        { 0x1790000, N_("PowerShot A610") },
        { 0x1800000, N_("PowerShot SD630 / Digital IXUS 65 / IXY Digital 80") },
        { 0x1810000, N_("PowerShot SD450 / Digital IXUS 55 / IXY Digital 60") },
        { 0x1820000, N_("PowerShot TX1") },
        { 0x1870000, N_("PowerShot SD400 / Digital IXUS 50 / IXY Digital 55") },
        { 0x1880000, N_("PowerShot A420") },
        { 0x1890000, N_("PowerShot SD900 / Digital IXUS 900 Ti / IXY Digital 1000") },
        { 0x1900000, N_("PowerShot SD550 / Digital IXUS 750 / IXY Digital 700") },
        { 0x1920000, N_("PowerShot A700") },
        { 0x1940000, N_("PowerShot SD700 IS / Digital IXUS 800 IS / IXY Digital 800 IS") },
        { 0x1950000, N_("PowerShot S3 IS") },
        { 0x1960000, N_("PowerShot A540") },
        { 0x1970000, N_("PowerShot SD600 / Digital IXUS 60 / IXY Digital 70") },
        { 0x1980000, N_("PowerShot G7") },
        { 0x1990000, N_("PowerShot A530") },
        { 0x2000000, N_("PowerShot SD800 IS / Digital IXUS 850 IS / IXY Digital 900 IS") },
        { 0x2010000, N_("PowerShot SD40 / Digital IXUS i7 / IXY Digital L4") },
        { 0x2020000, N_("PowerShot A710 IS") },
        { 0x2030000, N_("PowerShot A640") },
        { 0x2040000, N_("PowerShot A630") },
        { 0x2090000, N_("PowerShot S5 IS") },
        { 0x2100000, N_("PowerShot A460") },
        { 0x2120000, N_("PowerShot SD850 IS / Digital IXUS 950 IS") },
        { 0x2130000, N_("PowerShot A570 IS") },
        { 0x2140000, N_("PowerShot A560") },
        { 0x2150000, N_("PowerShot SD750 / Digital IXUS 75 / IXY Digital 90") },
        { 0x2160000, N_("PowerShot SD1000 / Digital IXUS 70 / IXY Digital 10") },
        { 0x2180000, N_("PowerShot A550") },
        { 0x2190000, N_("PowerShot A450") },
        { 0x3010000, N_("PowerShot Pro90 IS") },
        { 0x4040000, N_("PowerShot G1") },
        { 0x6040000, N_("PowerShot S100 / Digital IXUS / IXY Digital") },
        { 0x4007d675, N_("HV10") },
        { 0x4007d777, N_("iVIS DC50") },
        { 0x4007d778, N_("iVIS HV20") },
        { 0x80000001, N_("EOS-1D") },
        { 0x80000167, N_("EOS-1DS") },
        { 0x80000168, N_("EOS 10D") },
        { 0x80000169, N_("EOS-1D Mark III") },
        { 0x80000170, N_("EOS Digital Rebel / 300D / Kiss Digital") },
        { 0x80000174, N_("EOS-1D Mark II") },
        { 0x80000175, N_("EOS 20D") },
        { 0x80000188, N_("EOS-1Ds Mark II") },
        { 0x80000189, N_("EOS Digital Rebel XT / 350D / Kiss Digital N") },
        { 0x80000213, N_("EOS 5D") },
        { 0x80000232, N_("EOS-1D Mark II N") },
        { 0x80000234, N_("EOS 30D") },
        { 0x80000236, N_("EOS Digital Rebel XTi / 400D / Kiss Digital X") },
        { 0x80000250, N_("EOS 7D") },
        { 0x80000252, N_("EOS Rebel T1i / 500D / Kiss X3") },
        { 0x80000254, N_("EOS Rebel XS / 1000D / Kiss F") },
        { 0x80000261, N_("EOS 50D") },
        { 0x80000270, N_("EOS Rebel T2i / 550D / Kiss X4") },
        { 0x80000281, N_("EOS-1D Mark IV") },
    };

    //! SerialNumberFormat, tag 0x0015
    extern const TagDetails canonSerialNumberFormat[] = {
        {  0x90000000, N_("Format 1") },
        {  0xa0000000, N_("Format 2") },
    };

    //! SuperMacro, tag 0x001a
    extern const TagDetails canonSuperMacro[] = {
        {  0, N_("Off")     },
        {  1, N_("On (1)")  },
        {  2, N_("On (2)")  }
    };

    //! ColorSpace, tag 0x00b4
    extern const TagDetails canonColorSpace[] = {
        {  1, N_("sRGB")      },
        {  2, N_("Adobe RGB") }
    };

    // Canon MakerNote Tag Info
    const TagInfo CanonMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "0x0000", "0x0000", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0001, "CameraSettings", N_("Camera Settings"), N_("Various camera settings"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0002, "FocalLength", N_("Focal Length"), N_("Focal length"), canonId, makerTags, unsignedShort, -1, printFocalLength),
        TagInfo(0x0003, "0x0003", "0x0003", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0004, "ShotInfo", N_("Shot Info"), N_("Shot information"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0005, "Panorama", N_("Panorama"), N_("Panorama"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0006, "ImageType", N_("Image Type"), N_("Image type"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0007, "FirmwareVersion", N_("Firmware Version"), N_("Firmware version"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0008, "FileNumber", N_("File Number"), N_("File number"), canonId, makerTags, unsignedLong, -1, print0x0008),
        TagInfo(0x0009, "OwnerName", N_("Owner Name"), N_("Owner Name"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x000c, "SerialNumber", N_("Serial Number"), N_("Camera serial number"), canonId, makerTags, unsignedLong, -1, print0x000c),
        TagInfo(0x000d, "CameraInfo", N_("Camera Info"), N_("Camera info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x000f, "CustomFunctions", N_("Custom Functions"), N_("Custom Functions"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0010, "ModelID", N_("ModelID"), N_("Model ID"), canonId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonModelId)),
        TagInfo(0x0012, "PictureInfo", N_("Picture Info"), N_("Picture info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0013, "ThumbnailImageValidArea", N_("Thumbnail Image Valid Area"), N_("Thumbnail image valid area"), canonId, makerTags, signedShort, -1, printValue),
        TagInfo(0x0015, "SerialNumberFormat", N_("Serial Number Format"), N_("Serial number format"), canonId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonSerialNumberFormat)),
        TagInfo(0x001a, "SuperMacro", N_("Super Macro"), N_("Super macro"), canonId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonSuperMacro)),
        TagInfo(0x0026, "AFInfo", N_("AF Info"), N_("AF info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0083, "OriginalDecisionDataOffset", N_("Original Decision Data Offset"), N_("Original decision data offset"), canonId, makerTags, signedLong, -1, printValue),
        TagInfo(0x00a4, "WhiteBalanceTable", N_("White Balance Table"), N_("White balance table"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0095, "LensModel", N_("LensModel"), N_("LensModel"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0096, "InternalSerialNumber", N_("Internal Serial Number"), N_("Internal serial number"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0097, "DustRemovalData", N_("Dust Removal Data"), N_("Dust removal data"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0099, "CustomFunctions", N_("Custom Functions"), N_("Custom functions"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00a0, "ProcessingInfo", N_("Processing Info"), N_("Processing info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00aa, "MeasuredColor", N_("Measured Color"), N_("Measured color"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00b4, "ColorSpace", N_("ColorSpace"), N_("ColorSpace"), canonId, makerTags, signedShort,  -1, EXV_PRINT_TAG(canonColorSpace)),
        TagInfo(0x00b5, "0x00b5", "0x00b5", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00c0, "0x00c0", "0x00c0", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00c1, "0x00c1", "0x00c1", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00d0, "VRDOffset", N_("VRD Offset"), N_("VRD offset"), canonId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x00e0, "SensorInfo", N_("Sensor Info"), N_("Sensor info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x4001, "ColorData", N_("Color Data"), N_("Color data"), canonId, makerTags, unsignedShort, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", N_("Unknown CanonMakerNote tag"), canonId, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* CanonMakerNote::tagList()
    {
        return tagInfo_;
    }

    //! Macro, tag 0x0001
    extern const TagDetails canonCsMacro[] = {
        { 1, N_("On")  },
        { 2, N_("Off") }
    };

    //! Quality, tag 0x0003
    extern const TagDetails canonCsQuality[] = {
        { 1,   N_("Economy")      },
        { 2,   N_("Normal")       },
        { 3,   N_("Fine")         },
        { 4,   N_("RAW")          },
        { 5,   N_("Superfine")    },
        { 130, N_("Normal Movie") }
    };

    //! FlashMode, tag 0x0004
    extern const TagDetails canonCsFlashMode[] = {
        {  0, N_("Off")            },
        {  1, N_("Auto")           },
        {  2, N_("On")             },
        {  3, N_("Red-eye")        },
        {  4, N_("Slow sync")      },
        {  5, N_("Auto + red-eye") },
        {  6, N_("On + red-eye")   },
        { 16, N_("External")       },
        { 16, N_("External")       }    // To silence compiler warning
    };

    //! DriveMode, tag 0x0005
    extern const TagDetails canonCsDriveMode[] = {
        { 0, N_("Single / timer")             },
        { 1, N_("Continuous")                 },
        { 2, N_("Movie")                      },
        { 3, N_("Continuous, speed priority") },
        { 4, N_("Continuous, low")            },
        { 5, N_("Continuous, high")           }
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails canonCsFocusMode[] = {
        {  0, N_("One shot AF")  },
        {  1, N_("AI servo AF")  },
        {  2, N_("AI focus AF")  },
        {  3, N_("Manual focus") },
        {  4, N_("Single")       },
        {  5, N_("Continuous")   },
        {  6, N_("Manual focus") },
        { 16, N_("Pan focus")    },
        { 16, N_("Pan focus")    }    // To silence compiler warning
    };

    //! ImageSize, tag 0x000a
    extern const TagDetails canonCsImageSize[] = {
        { 0, N_("Large")    },
        { 1, N_("Medium")   },
        { 2, N_("Small")    },
        { 5, N_("Medium 1") },
        { 6, N_("Medium 2") },
        { 7, N_("Medium 3") }
    };

    //! EasyMode, tag 0x000b
    extern const TagDetails canonCsEasyMode[] = {
        {  0, N_("Full auto")        },
        {  1, N_("Manual")           },
        {  2, N_("Landscape")        },
        {  3, N_("Fast shutter")     },
        {  4, N_("Slow shutter")     },
        {  5, N_("Night Scene")      },
        {  6, N_("Gray scale")       },
        {  7, N_("Sepia")            },
        {  8, N_("Portrait")         },
        {  9, N_("Sports")           },
        { 10, N_("Macro / close-up") },
        { 11, N_("Black & white")    },
        { 12, N_("Pan focus")        },
        { 13, N_("Vivid")            },
        { 14, N_("Neutral")          },
        { 15, N_("Flash off")        },
        { 16, N_("Long shutter")     },
        { 17, N_("Super macro")      },
        { 18, N_("Foliage")          },
        { 19, N_("Indoor")           },
        { 20, N_("Fireworks")        },
        { 21, N_("Beach")            },
        { 22, N_("Underwater")       },
        { 23, N_("Snow")             },
        { 24, N_("Kids & pets")      },
        { 25, N_("Night SnapShot")   },
        { 26, N_("Digital macro")    },
        { 27, N_("My Colors")        },
        { 28, N_("Still image")      }
    };

    //! DigitalZoom, tag 0x000c
    extern const TagDetails canonCsDigitalZoom[] = {
        { 0, N_("None")  },
        { 1, "2x"        },
        { 2, "4x"        },
        { 3, N_("Other") },
        { 3, N_("Other") }                      // To silence compiler warning
    };

    //! Contrast, Saturation Sharpness, tags 0x000d, 0x000e, 0x000f
    extern const TagDetails canonCsLnh[] = {
        { 0xffff, N_("Low")    },
        { 0x0000, N_("Normal") },
        { 0x0001, N_("High")   }
    };

    //! ISOSpeeds, tag 0x0010
    extern const TagDetails canonCsISOSpeed[] = {
        {     0, N_("n/a")       },
        {    14, N_("Auto High") },
        {    15, N_("Auto")      },
        {    16,   "50"      },
        {    17,  "100"      },
        {    18,  "200"      },
        {    19,  "400"      },
        {    20,  "800"      },
        { 16464,   "80"      },
        { 16484,  "100"      },
        { 16584,  "200"      },
        { 16784,  "400"      },
        { 17184,  "800"      },
        { 17984, "1600"      },
        { 19584, "3200"      }
    };

    //! MeteringMode, tag 0x0011
    extern const TagDetails canonCsMeteringMode[] = {
        { 0, N_("Default")         },
        { 1, N_("Spot")            },
        { 2, N_("Average")         },
        { 3, N_("Evaluative")      },
        { 4, N_("Partial")         },
        { 5, N_("Center weighted") }
    };

    //! FocusType, tag 0x0012
    extern const TagDetails canonCsFocusType[] = {
        {  0, N_("Manual")       },
        {  1, N_("Auto")         },
        {  2, N_("Not known")    },
        {  3, N_("Macro")        },
        {  4, N_("Very close")   },
        {  5, N_("Close")        },
        {  6, N_("Middle range") },
        {  7, N_("Far range")    },
        {  8, N_("Pan focus")    },
        {  9, N_("Super macro")  },
        { 10, N_("Infinity")     }
    };

    //! AFPoint, tag 0x0013
    extern const TagDetails canonCsAfPoint[] = {
        { 0x2005, N_("Manual AF point selection") },
        { 0x3000, N_("None (MF)")                 },
        { 0x3001, N_("Auto-selected")             },
        { 0x3002, N_("Right")                     },
        { 0x3003, N_("Center")                    },
        { 0x3004, N_("Left")                      },
        { 0x4001, N_("Auto AF point selection")   }
    };

    //! ExposureProgram, tag 0x0014
    extern const TagDetails canonCsExposureProgram[] = {
        { 0, N_("Easy shooting (Auto)")   },
        { 1, N_("Program (P)")            },
        { 2, N_("Shutter priority (Tv)")  },
        { 3, N_("Aperture priority (Av)") },
        { 4, N_("Manual (M)")             },
        { 5, N_("A-DEP")                  },
        { 6, N_("M-DEP")                  }
    };

    //! LensType, tag 0x0016
    extern const TagDetails canonCsLensType[] = {
        {   1, "Canon EF 50mm f/1.8"                                        },
        {   2, "Canon EF 28mm f/2.8"                                        },
        {   3, "Canon EF 135mm f/2.8 Soft"                                  },
        {   4, "Canon EF 35-105mm f/3.5-4.5 or Sigma Lens"                  }, // 0
        {   4, "Sigma UC Zoom 35-135mm f/4-5.6"                             }, // 1
        {   5, "Canon EF 35-70mm f/3.5-4.5"                                 },
        {   6, "Canon EF 28-70mm f/3.5-4.5 or Sigma or Tokina Lens"         }, // 0
        {   6, "Sigma 18-50mm f/3.5-5.6 DC"                                 }, // 1
        {   6, "Sigma 18-125mm f/3.5-5.6 DC IF ASP"                         }, // 2
        {   6, "Tokina AF193-2 19-35mm f/3.5-4.5"                           }, // 3
        {   6, "Sigma 28-80mm f/3.5-5.6 II Macro"                           }, // 4
        {   7, "Canon EF 100-300mm f/5.6L"                                  },
        {   8, "Canon EF 100-300mm f/5.6"                                   }, // 0
        {   8, "Sigma 70-300mm f/4-5.6 DG Macro"                            }, // 1
        {   8, "Tokina AT-X242AF 24-200mm f/3.5-5.6"                        }, // 2
        {   9, "Canon EF 70-210mm f/4"                                      }, // 0
        {   9, "Sigma 55-200mm f/4-5.6 DC"                                  }, // 1
        {  10, "Canon EF 50mm f/2.5 Macro"                                  }, // 0
        {  10, "Sigma 50mm f/2.8 EX"                                        }, // 1
        {  10, "Sigma 28mm f/1.8"                                           }, // 2
        {  10, "Sigma 105mm f/2.8 Macro EX"                                 }, // 3
        {  10, "Sigma 70mm f/2.8 EX DG Macro EF"                            }, // 4
        {  11, "Canon EF 35mm f/2"                                          },
        {  13, "Canon EF 15mm f/2.8 Fisheye"                                },
        {  14, "Canon EF 50-200mm f/3.5-4.5L"                               },
        {  15, "Canon EF 50-200mm f/3.5-4.5"                                },
        {  16, "Canon EF 35-135mm f/3.5-4.5"                                },
        {  17, "Canon EF 35-70mm f/3.5-4.5A"                                },
        {  18, "Canon EF 28-70mm f/3.5-4.5"                                 },
        {  20, "Canon EF 100-200mm f/4.5A"                                  },
        {  21, "Canon EF 80-200mm f/2.8L"                                   },
        {  22, "Canon EF 20-35mm f/2.8L"                                    }, // 0
        {  22, "Tokina AT-X280AF PRO 28-80mm f/2.8 Aspherical"              }, // 1
        {  23, "Canon EF 35-105mm f/3.5-4.5"                                },
        {  24, "Canon EF 35-80mm f/4-5.6 Power Zoom"                        },
        {  25, "Canon EF 35-80mm f/4-5.6 Power Zoom"                        },
        {  26, "Canon EF 100mm f/2.8 Macro or Cosina or Tamron Lens"        }, // 0
        {  26, "Cosina 100mm f/3.5 Macro AF"                                }, // 1
        {  26, "Tamron SP AF 90mm f/2.8 Di Macro"                           }, // 2
        {  26, "Tamron SP AF 180mm f/3.5 Di Macro"                          }, // 3
        {  26, "Carl Zeiss Planar T* 50mm f/1.4"                            }, // 4
        {  27, "Canon EF 35-80mm f/4-5.6"                                   },
        {  28, "Canon EF 80-200mm f/4.5-5.6"                                }, // 0
        {  28, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"               }, // 1
        {  28, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro"  }, // 2
        {  28, "Tamron AF 70-300mm f/4.5-5.6 Di LD 1:2 Macro Zoom"          }, // 3
        {  28, "Tamron AF Aspherical 28-200mm f/3.8-5.6"                    }, // 4
        {  29, "Canon EF 50mm f/1.8 MkII"                                   },
        {  30, "Canon EF 35-105mm f/4.5-5.6"                                },
        {  31, "Canon EF 75-300mm f/4-5.6"                                  }, // 0
        {  31, "Tamron SP AF 300mm f/2.8 LD IF"                             }, // 1
        {  32, "Canon EF 24mm f/2.8"                                        }, // 0
        {  32, "Sigma 15mm f/2.8 EX Fisheye"                                }, // 1
        {  33, "Voigtlander or Zeiss Lens"                                  }, // 0
        {  33, "Voigtlander Ultron 40mm f/2 SLII Aspherical"                }, // 1
        {  33, "Zeiss Distagon 35mm T* f/2 ZE"                              }, // 2
        {  35, "Canon EF 35-80mm f/4-5.6"                                   },
        {  36, "Canon EF 38-76mm f/4.5-5.6"                                 },
        {  37, "Canon EF 35-80mm f/4-5.6"                                   }, // 0
        {  37, "Tamron 70-200mm f/2.8 Di LD IF Macro"                       }, // 1
        {  37, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro Model A20" }, // 2
        {  37, "Tamron SP AF 17-50mm f/2.8 XR Di II VC LD Aspherical [IF] " }, // 3
        {  37, "Tamron AF 18-270mm f/3.5-6.3 Di II VC LD Aspherical [IF] Macro" }, // 4
        {  38, "Canon EF 80-200mm f/4.5-5.6"                                },
        {  39, "Canon EF 75-300mm f/4-5.6"                                  },
        {  40, "Canon EF 28-80mm f/3.5-5.6"                                 },
        {  41, "Canon EF 28-90mm f/4-5.6"                                   },
        {  42, "Canon EF 28-200mm f/3.5-5.6"                                }, // 0
        {  42, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro Model A20" }, // 1
        {  43, "Canon EF 28-105mm f/4-5.6"                                  },
        {  44, "Canon EF 90-300mm f/4.5-5.6"                                },
        {  45, "Canon EF-S 18-55mm f/3.5-5.6"                               },
        {  46, "Canon EF 28-90mm f/4-5.6"                                   },
        {  48, "Canon EF-S 18-55mm f/3.5-5.6 IS"                            },
        {  49, "Canon EF-S 55-250mm f/4-5.6 IS"                             },
        {  50, "Canon EF-S 18-200mm f/3.5-5.6 IS"                           },
        {  51, "Canon EF-S 18-135mm f/3.5-5.6 IS"                           },
        {  52, "Canon EF-S 18-55mm f/3.5-5.6 IS II"                         },
        {  53, "Canon EF-S 18-55mm f/3.5-5.6 III"                           },
        {  94, "Canon TS-E 17mm f/4L"                                       },
        {  95, "Canon TS-E 24.0mm f/3.5 L II"                               },
        { 124, "Canon MP-E 65mm f/2.8 1-5x Macro Photo"                     },
        { 125, "Canon TS-E 24mm f/3.5L"                                     },
        { 126, "Canon TS-E 45mm f/2.8"                                      },
        { 127, "Canon TS-E 90mm f/2.8"                                      },
        { 129, "Canon EF 300mm f/2.8L"                                      },
        { 130, "Canon EF 50mm f/1.0L"                                       },
        { 131, "Canon EF 28-80mm f/2.8-4L"                                  }, // 0
        { 131, "Sigma 8mm f/3.5 EX DG Circular Fisheye"                     }, // 1
        { 131, "Sigma 17-35mm f/2.8-4 EX DG Aspherical HSM"                 }, // 2
        { 131, "Sigma 17-70mm f/2.8-4.5 DC Macro"                           }, // 3
        { 131, "Sigma APO 50-150mm f/2.8 EX DC HSM"                         }, // 4
        { 131, "Sigma APO 120-300mm f/2.8 EX DG HSM"                        }, // 5
        { 131, "Sigma 4.5mm F2.8 EX DC HSM Circular Fisheye"                }, // 6
        { 132, "Canon EF 1200mm f/5.6L"                                     },
        { 134, "Canon EF 600mm f/4L IS"                                     },
        { 135, "Canon EF 200mm f/1.8L"                                      },
        { 136, "Canon EF 300mm f/2.8L"                                      },
        { 137, "Canon EF 85mm f/1.2L"                                       }, // 0
        { 137, "Sigma 18-50mm f/2.8-4.5 DC OS HSM"                          }, // 1
        { 137, "Sigma 50-200mm f/4-5.6 DC OS HSM"                           }, // 2
        { 137, "Sigma 18-250mm f/3.5-6.3 DC OS HSM"                         }, // 3
        { 137, "Sigma 24-70mm f/2.8 IF EX DG HSM"                           }, // 4
        { 137, "Sigma 18-125mm f/3.8-5.6 DC OS HSM"                         }, // 5
        { 137, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM"                      }, // 6
        { 137, "Sigma 17-50mm f/2.8 OS HSM"                                 }, // 7
        { 137, "Sigma 18-200mm f/3.5-6.3 II DC OS HSM"                      }, // 8
        { 137, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                  }, // 9
        { 138, "Canon EF 28-80mm f/2.8-4L"                                  },
        { 139, "Canon EF 400mm f/2.8L"                                      },
        { 140, "Canon EF 500mm f/4.5L"                                      },
        { 141, "Canon EF 500mm f/4.5L"                                      },
        { 142, "Canon EF 300mm f/2.8L IS"                                   },
        { 143, "Canon EF 500mm f/4L IS"                                     },
        { 144, "Canon EF 35-135mm f/4-5.6 USM"                              },
        { 145, "Canon EF 100-300mm f/4.5-5.6 USM"                           },
        { 146, "Canon EF 70-210mm f/3.5-4.5 USM"                            },
        { 147, "Canon EF 35-135mm f/4-5.6 USM"                              },
        { 148, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 149, "Canon EF 100mm f/2 USM"                                     },
        { 150, "Canon EF 14mm f/2.8L"                                       }, // 0
        { 150, "Sigma 20mm EX f/1.8"                                        }, // 1
        { 150, "Sigma 30mm f/1.4 DC HSM"                                    }, // 2
        { 150, "Sigma 24mm f/1.8 DG Macro EX"                               }, // 3
        { 151, "Canon EF 200mm f/2.8L"                                      },
        { 152, "Canon EF 300mm f/4L IS"                                     }, // 0
        { 152, "Sigma 12-24mm f/4.5-5.6 EX DG ASPHERICAL HSM"               }, // 1
        { 152, "Sigma 14mm f/2.8 EX Aspherical HSM"                         }, // 2
        { 152, "Sigma 10-20mm f/4-5.6"                                      }, // 3
        { 152, "Sigma 100-300mm f/4"                                        }, // 4
        { 153, "Canon EF 35-350mm f/3.5-5.6L"                               }, // 0
        { 153, "Sigma 50-500mm f/4-6.3 APO HSM EX"                          }, // 1
        { 153, "Tamron AF 28-300mm f/3.5-6.3 XR LD Aspherical [IF] Macro"   }, // 2
        { 153, "Tamron AF 18-200mm f/3.5-6.3 XR Di II LD Aspherical [IF] Macro Model A14" }, // 3
        { 153, "Tamron 18-250mm f/3.5-6.3 Di II LD Aspherical [IF] Macro"   }, // 4
        { 154, "Canon EF 20mm f/2.8 USM"                                    },
        { 155, "Canon EF 85mm f/1.8 USM"                                    },
        { 156, "Canon EF 28-105mm f/3.5-4.5 USM"                            }, // 0
        { 156, "Tamron SP AF 70-300mm F4-5.6 Di VC USD"                     }, // 1
        { 160, "Canon EF 20-35mm f/3.5-4.5 USM"                             }, // 0
        { 160, "Tamron AF 19-35mm f/3.5-4.5"                                }, // 1
        { 160, "Tokina AT-X 124 AF 12-24mm f/4 DX"                          }, // 2
        { 160, "Tokina AT-X 107 AF DX Fish-eye 10-17mm f/3.5-4.5"           }, // 3
        { 160, "Tokina AT-X 116 PRO DX AF 11-16mm f/2.8"                    }, // 4
        { 161, "Canon EF 28-70mm f/2.8L"                                    }, // 0
        { 161, "Sigma 24-70mm EX f/2.8"                                     }, // 1
        { 161, "Sigma 28-70mm f/2.8 EX"                                     }, // 2
        { 161, "Tamron AF 17-50mm f/2.8 Di-II LD Aspherical"                }, // 3
        { 161, "Tamron 90mm f/2.8"                                          }, // 4
        { 162, "Canon EF 200mm f/2.8L"                                      },
        { 163, "Canon EF 300mm f/4L"                                        },
        { 164, "Canon EF 400mm f/5.6L"                                      },
        { 165, "Canon EF 70-200mm f/2.8 L"                                  },
        { 166, "Canon EF 70-200mm f/2.8 L + 1.4x"                           },
        { 167, "Canon EF 70-200mm f/2.8 L + 2x"                             },
        { 168, "Canon EF 28mm f/1.8 USM"                                    },
        { 169, "Canon EF 17-35mm f/2.8L"                                    }, // 0
        { 169, "Sigma 18-200mm f/3.5-6.3 DC OS"                             }, // 1
        { 169, "Sigma 15-30mm f/3.5-4.5 EX DG Aspherical"                   }, // 2
        { 169, "Sigma 18-50mm f/2.8 Macro"                                  }, // 3
        { 169, "Sigma 50mm f/1.4 EX DG HSM"                                 }, // 4
        { 169, "Sigma 85mm f/1.4 EX DG HSM"                                 }, // 5
        { 169, "Sigma 30mm f/1.4 EX DC HSM"                                 }, // 6
        { 170, "Canon EF 200mm f/2.8L II"                                   },
        { 171, "Canon EF 300mm f/4L"                                        },
        { 172, "Canon EF 400mm f/5.6L"                                      },
        { 173, "Canon EF 180mm Macro f/3.5L or Sigma Lens"                  }, // 0
        { 173, "Sigma 180mm EX HSM Macro f/3.5"                             }, // 1
        { 173, "Sigma APO Macro 150mm f/3.5 EX DG IF HSM"                   }, // 2
        { 174, "Canon EF 135mm f/2L"                                        }, // 0
        { 174, "Sigma 70-200mm f/2.8 EX DG APO OS HSM"                      }, // 1
        { 175, "Canon EF 400mm f/2.8L"                                      },
        { 176, "Canon EF 24-85mm f/3.5-4.5 USM"                             },
        { 177, "Canon EF 300mm f/4L IS"                                     },
        { 178, "Canon EF 28-135mm f/3.5-5.6 IS"                             },
        { 179, "Canon EF 24mm f/1.4L"                                       },
        { 180, "Canon EF 35mm f/1.4L"                                       },
        { 181, "Canon EF 100-400mm f/4.5-5.6L IS + 1.4x"                    },
        { 182, "Canon EF 100-400mm f/4.5-5.6L IS + 2x"                      },
        { 183, "Canon EF 100-400mm f/4.5-5.6L IS"                           },
        { 184, "Canon EF 400mm f/2.8L + 2x"                                 },
        { 185, "Canon EF 600mm f/4L IS"                                     },
        { 186, "Canon EF 70-200mm f/4L"                                     },
        { 187, "Canon EF 70-200mm f/4L + 1.4x"                              },
        { 188, "Canon EF 70-200mm f/4L + 2x"                                },
        { 189, "Canon EF 70-200mm f/4L + 2.8x"                              },
        { 190, "Canon EF 100mm f/2.8 Macro"                                 },
        { 191, "Canon EF 400mm f/4 DO IS"                                   },
        { 193, "Canon EF 35-80mm f/4-5.6 USM"                               },
        { 194, "Canon EF 80-200mm f/4.5-5.6 USM"                            },
        { 195, "Canon EF 35-105mm f/4.5-5.6 USM"                            },
        { 196, "Canon EF 75-300mm f/4-5.6 USM"                              },
        { 197, "Canon EF 75-300mm f/4-5.6 IS USM"                           },
        { 198, "Canon EF 50mm f/1.4 USM"                                    },
        { 199, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 200, "Canon EF 75-300mm f/4-5.6 USM"                              },
        { 201, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 202, "Canon EF 28-80mm f/3.5-5.6 USM IV"                          },
        { 208, "Canon EF 22-55mm f/4-5.6 USM"                               },
        { 209, "Canon EF 55-200mm f/4.5-5.6"                                },
        { 210, "Canon EF 28-90mm f/4-5.6 USM"                               },
        { 211, "Canon EF 28-200mm f/3.5-5.6 USM"                            },
        { 212, "Canon EF 28-105mm f/4-5.6 USM"                              },
        { 213, "Canon EF 90-300mm f/4.5-5.6 USM"                            },
        { 214, "Canon EF-S 18-55mm f/3.5-5.6 USM"                           },
        { 215, "Canon EF 55-200mm f/4.5-5.6 II USM"                         },
        { 224, "Canon EF 70-200mm f/2.8L IS"                                },
        { 225, "Canon EF 70-200mm f/2.8L IS + 1.4x"                         },
        { 226, "Canon EF 70-200mm f/2.8L IS + 2x"                           },
        { 227, "Canon EF 70-200mm f/2.8L IS + 2.8x"                         },
        { 228, "Canon EF 28-105mm f/3.5-4.5 USM"                            },
        { 229, "Canon EF 16-35mm f/2.8L"                                    },
        { 230, "Canon EF 24-70mm f/2.8L"                                    },
        { 231, "Canon EF 17-40mm f/4L"                                      },
        { 232, "Canon EF 70-300mm f/4.5-5.6 DO IS USM"                      },
        { 233, "Canon EF 28-300mm f/3.5-5.6L IS"                            },
        { 234, "Canon EF-S 17-85mm f4-5.6 IS USM"                           },
        { 235, "Canon EF-S 10-22mm f/3.5-4.5 USM"                           },
        { 236, "Canon EF-S 60mm f/2.8 Macro USM"                            },
        { 237, "Canon EF 24-105mm f/4L IS"                                  },
        { 238, "Canon EF 70-300mm f/4-5.6 IS USM"                           },
        { 239, "Canon EF 85mm f/1.2L II"                                    },
        { 240, "Canon EF-S 17-55mm f/2.8 IS USM"                            },
        { 241, "Canon EF 50mm f/1.2L"                                       },
        { 242, "Canon EF 70-200mm f/4L IS"                                  },
        { 243, "Canon EF 70-200mm f/4L IS + 1.4x"                           },
        { 244, "Canon EF 70-200mm f/4L IS + 2x"                             },
        { 245, "Canon EF 70-200mm f/4L IS + 2.8x"                           },
        { 246, "Canon EF 16-35mm f/2.8L II"                                 },
        { 247, "Canon EF 14mm f/2.8L II USM"                                },
        { 248, "Canon EF 200mm f/2L IS"                                     },
        { 249, "Canon EF 800mm f/5.6L IS"                                   },
        { 250, "Canon EF 24 f/1.4L II"                                      },
        { 251, "Canon EF 70-200mm f/2.8L IS II USM"                         },
        { 254, "Canon EF 100mm f/2.8L Macro IS USM"                         },
        { 488, "Canon EF-S 15-85mm f/3.5-5.6 IS USM"                        },
        { 489, "Canon EF 70-300mm f/4-5.6L IS USM"                          },
        { 490, "Canon EF 8-15mm f/4L USM"                                   },
        { 491, "Canon EF 300mm f/2.8L IS II USM"                            }
    };

    //! A lens id and a pretty-print function for special treatment of the id.
    struct LensIdFct {
        long     id_;                           //!< Lens id
        PrintFct fct_;                          //!< Pretty-print function
        //! Comparison operator for find template
        bool operator==(long id) const { return id_ == id; }
    };

    //! List of lens ids which require special treatment with the medicine
    const LensIdFct lensIdFct[] = {
        {   4, 0                        }, // no known medicine
        {   6, printCsLensByFocalLength },
        {   8, printCsLensByFocalLength },
        {   9, printCsLensByFocalLength },
        {  10, printCsLensByFocalLength }, // works partly
        {  22, printCsLensByFocalLength },
        {  26, printCsLensByFocalLength }, // works partly
        {  28, printCsLensByFocalLength },
        {  31, printCsLensByFocalLength },
        {  32, printCsLensByFocalLength },
        {  33, printCsLensByFocalLength }, // not tested
        {  37, printCsLensByFocalLength },
        {  42, printCsLensByFocalLength },
        { 131, printCsLensByFocalLength },
        { 137, printCsLensByFocalLength }, // not tested
        { 150, printCsLensByFocalLength },
        { 152, printCsLensByFocalLength },
        { 153, printCsLensByFocalLength },
        { 156, printCsLensByFocalLength },
        { 160, printCsLensByFocalLength },
        { 161, printCsLensByFocalLength },
        { 169, printCsLensByFocalLength },
        { 173, printCsLensByFocalLength }, // works partly
        { 174, printCsLensByFocalLength }  // not tested
    };

    //! FlashActivity, tag 0x001c
    extern const TagDetails canonCsFlashActivity[] = {
        { 0, N_("Did not fire") },
        { 1, N_("Fired")        }
    };

    //! FlashDetails, tag 0x001d
    extern const TagDetailsBitmask canonCsFlashDetails[] = {
        { 0x4000, N_("External flash")        },
        { 0x2000, N_("Internal flash")        },
        { 0x0001, N_("Manual")                },
        { 0x0002, N_("TTL")                   },
        { 0x0004, N_("A-TTL")                 },
        { 0x0008, N_("E-TTL")                 },
        { 0x0010, N_("FP sync enabled")       },
        { 0x0080, N_("2nd-curtain sync used") },
        { 0x0800, N_("FP sync used")          }
    };

    //! FocusContinuous, tag 0x0020
    extern const TagDetails canonCsFocusContinuous[] = {
        { 0, N_("Single")     },
        { 1, N_("Continuous") }
    };

    //! AESetting, tag 0x0021
    extern const TagDetails canonCsAESetting[] = {
        { 0, N_("Normal AE")                       },
        { 1, N_("Exposure compensation")           },
        { 2, N_("AE lock")                         },
        { 3, N_("AE lock + exposure compensation") },
        { 4, N_("No AE")                           }
    };

    //! ImageStabilization, tag 0x0022
    extern const TagDetails canonCsImageStabilization[] = {
        { 0, N_("Off")           },
        { 1, N_("On")            },
        { 2, N_("On, shot only") }
    };

    //! SpotMeteringMode, tag 0x0027
    extern const TagDetails canonCsSpotMeteringMode[] = {
        { 0,   N_("Center")   },
        { 1,   N_("AF Point") }
    };

    //! PhotoEffect, tag 0x0028
    extern const TagDetails canonCsPhotoEffect[] = {
        { 0,   N_("Off")           },
        { 1,   N_("Vivid")         },
        { 2,   N_("Neutral")       },
        { 3,   N_("Smooth")        },
        { 4,   N_("Sepia")         },
        { 5,   N_("B&W")           },
        { 6,   N_("Custom")        },
        { 100, N_("My color data") },
        { 100, N_("My color data") }    // To silence compiler warning
    };

    //! ManualFlashOutput, tag 0x0029
    extern const TagDetails canonCsManualFlashOutput[] = {
        { 0x0000, N_("n/a")    },
        { 0x0500, N_("Full")   },
        { 0x0502, N_("Medium") },
        { 0x0504, N_("Low")    },
        { 0x7fff, N_("n/a")    }
    };

    //! SRAWQuality, tag 0x002e
    extern const TagDetails canonCsSRAWQuality[] = {
        { 0, N_("n/a")          },
        { 1, N_("sRAW1 (mRAW)") },
        { 2, N_("sRAW2 (sRAW)") }
    };

    // Canon Camera Settings Tag Info
    const TagInfo CanonMakerNote::tagInfoCs_[] = {
        TagInfo(0x0001, "Macro", N_("Macro"), N_("Macro mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsMacro)),
        TagInfo(0x0002, "Selftimer", N_("Selftimer"), N_("Self timer"), canonCsId, makerTags, signedShort, 1, printCs0x0002),
        TagInfo(0x0003, "Quality", N_("Quality"), N_("Quality"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsQuality)),
        TagInfo(0x0004, "FlashMode", N_("Flash Mode"), N_("Flash mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFlashMode)),
        TagInfo(0x0005, "DriveMode", N_("Drive Mode"), N_("Drive mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsDriveMode)),
        TagInfo(0x0006, "0x0006", "0x0006", N_("Unknown"), canonCsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0007, "FocusMode", N_("Focus Mode"), N_("Focus mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusMode)),
        TagInfo(0x0008, "0x0008", "0x0008", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000a, "ImageSize", N_("Image Size"), N_("Image size"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsImageSize)),
        TagInfo(0x000b, "EasyMode", N_("Easy Mode"), N_("Easy shooting mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsEasyMode)),
        TagInfo(0x000c, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsDigitalZoom)),
        TagInfo(0x000d, "Contrast", N_("Contrast"), N_("Contrast setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x000e, "Saturation", N_("Saturation"), N_("Saturation setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x000f, "Sharpness", N_("Sharpness"), N_("Sharpness setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x0010, "ISOSpeed", N_("ISO Speed Mode"), N_("ISO speed setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsISOSpeed)),
        TagInfo(0x0011, "MeteringMode", N_("Metering Mode"), N_("Metering mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsMeteringMode)),
        TagInfo(0x0012, "FocusType", N_("Focus Type"), N_("Focus type setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusType)),
        TagInfo(0x0013, "AFPoint", N_("AF Point"), N_("AF point selected"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsAfPoint)),
        TagInfo(0x0014, "ExposureProgram", N_("Exposure Program"), N_("Exposure mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsExposureProgram)),
        TagInfo(0x0015, "0x0015", "0x0015", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0016, "LensType", N_("Lens Type"), N_("Lens type"), canonCsId, makerTags, signedShort, 1, printCsLensType),
        TagInfo(0x0017, "Lens", N_("Lens"), N_("'long' and 'short' focal length of lens (in 'focal units') and 'focal units' per mm"), canonCsId, makerTags, unsignedShort, 3, printCsLens),
        TagInfo(0x0018, "ShortFocal", N_("Short Focal"), N_("Short focal"), canonCsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0019, "FocalUnits", N_("Focal Units"), N_("Focal units"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x001a, "MaxAperture", N_("Max Aperture"), N_("Max aperture"), canonCsId, makerTags, signedShort, 1, printSi0x0015),
        TagInfo(0x001b, "MinAperture", N_("Min Aperture"), N_("Min aperture"), canonCsId, makerTags, signedShort, 1, printSi0x0015),
        TagInfo(0x001c, "FlashActivity", N_("Flash Activity"), N_("Flash activity"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFlashActivity)),
        TagInfo(0x001d, "FlashDetails", N_("Flash Details"), N_("Flash details"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG_BITMASK(canonCsFlashDetails)),
        TagInfo(0x001e, "0x001e", "0x001e", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x001f, "0x001f", "0x001f", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0020, "FocusContinuous", N_("Focus Continuous"), N_("Focus continuous setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusContinuous)),
        TagInfo(0x0021, "AESetting", N_("AESetting"), N_("AE setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsAESetting)),
        TagInfo(0x0022, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsImageStabilization)),
        TagInfo(0x0023, "DisplayAperture", N_("Display Aperture"), N_("Display aperture"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0024, "ZoomSourceWidth", N_("Zoom Source Width"), N_("Zoom source width"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0025, "ZoomTargetWidth", N_("Zoom Target Width"), N_("Zoom target width"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0026, "0x0026", "0x0026", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0027, "SpotMeteringMode", N_("Spot Metering Mode"), N_("Spot metering mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSpotMeteringMode)),
        TagInfo(0x0028, "PhotoEffect", N_("Photo Effect"), N_("Photo effect"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsPhotoEffect)),
        TagInfo(0x0029, "ManualFlashOutput", N_("Manual Flash Output"), N_("Manual flash output"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)),
        TagInfo(0x002a, "ColorTone", N_("Color Tone"), N_("Color tone"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x002e, "SRAWQuality", N_("SRAW Quality Tone"), N_("SRAW quality"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", N_("Unknown Canon Camera Settings 1 tag"), canonCsId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }

    //! WhiteBalance, multiple tags
    extern const TagDetails canonSiWhiteBalance[] = {
        {  0, N_("Auto")                        },
        {  1, N_("Sunny")                       },
        {  2, N_("Cloudy")                      },
        {  3, N_("Tungsten")                    },
        {  4, N_("Fluorescent")                 },
        {  5, N_("Flash")                       },
        {  6, N_("Custom")                      },
        {  7, N_("Black & White")               },
        {  8, N_("Shade")                       },
        {  9, N_("Manual Temperature (Kelvin)") },
        { 10, N_("PC Set 1")                    },
        { 11, N_("PC Set 2")                    },
        { 12, N_("PC Set 3")                    },
        { 14, N_("Daylight Fluorescent")        },
        { 15, N_("Custom 1")                    },
        { 16, N_("Custom 2")                    },
        { 17, N_("Underwater")                  },
        { 18, N_("Custom 3")                    },
        { 19, N_("Custom 3")                    },
        { 20, N_("PC Set 4")                    },
        { 21, N_("PC Set 5")                    }
    };

    //! AFPointUsed, tag 0x000e
    extern const TagDetailsBitmask canonSiAFPointUsed[] = {
        { 0x0004, N_("left")   },
        { 0x0002, N_("center") },
        { 0x0001, N_("right")  }
    };

    //! FlashBias, tag 0x000f
    extern const TagDetails canonSiFlashBias[] = {
        { 0xffc0, "-2 EV"    },
        { 0xffcc, "-1.67 EV" },
        { 0xffd0, "-1.50 EV" },
        { 0xffd4, "-1.33 EV" },
        { 0xffe0, "-1 EV"    },
        { 0xffec, "-0.67 EV" },
        { 0xfff0, "-0.50 EV" },
        { 0xfff4, "-0.33 EV" },
        { 0x0000, "0 EV"     },
        { 0x000c, "0.33 EV"  },
        { 0x0010, "0.50 EV"  },
        { 0x0014, "0.67 EV"  },
        { 0x0020, "1 EV"     },
        { 0x002c, "1.33 EV"  },
        { 0x0030, "1.50 EV"  },
        { 0x0034, "1.67 EV"  },
        { 0x0040, "2 EV"     }
    };

    // Canon Shot Info Tag
    const TagInfo CanonMakerNote::tagInfoSi_[] = {
        TagInfo(0x0001, "0x0001", "0x0001", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed Used"), N_("ISO speed used"), canonSiId, makerTags, unsignedShort, 1, printSi0x0002),
        TagInfo(0x0003, "MeasuredEV", N_("Measured EV"), N_("Measured EV"), canonSiId, makerTags, unsignedShort, 1, printSi0x0003),
        TagInfo(0x0004, "TargetAperture", N_("Target Aperture"), N_("Target Aperture"), canonSiId, makerTags, unsignedShort, 1, printSi0x0015),
        TagInfo(0x0005, "TargetShutterSpeed", N_("Target Shutter Speed"), N_("Target shutter speed"), canonSiId, makerTags, unsignedShort, 1, printSi0x0016),
        TagInfo(0x0006, "0x0006", "0x0006", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0007, "WhiteBalance", N_("White Balance"), N_("White balance setting"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)),
        TagInfo(0x0008, "0x0008", "0x0008", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0009, "Sequence", N_("Sequence"), N_("Sequence number (if in a continuous burst)"), canonSiId, makerTags, unsignedShort, 1, printSi0x0009),
        TagInfo(0x000a, "0x000a", "0x000a", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000b, "0x000b", "0x000b", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000c, "0x000c", "0x000c", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000d, "0x000d", "0x000d", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000e, "AFPointUsed", N_("AF Point Used"), N_("AF point used"), canonSiId, makerTags, unsignedShort, 1, printSi0x000e),
        TagInfo(0x000f, "FlashBias", N_("Flash Bias"), N_("Flash bias"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonSiFlashBias)),
        TagInfo(0x0010, "0x0010", "0x0010", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0011, "0x0011", "0x0011", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0012, "0x0012", "0x0012", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0013, "SubjectDistance", N_("Subject Distance"), N_("Subject distance (units are not clear)"), canonSiId, makerTags, unsignedShort, 1, printSi0x0013),
        TagInfo(0x0014, "0x0014", "0x0014", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0015, "ApertureValue", N_("Aperture Value"), N_("Aperture"), canonSiId, makerTags, unsignedShort, 1, printSi0x0015),
        TagInfo(0x0016, "ShutterSpeedValue", N_("Shutter Speed Value"), N_("Shutter speed"), canonSiId, makerTags, unsignedShort, 1, printSi0x0016),
        TagInfo(0x0017, "MeasuredEV2", N_("Measured EV 2"), N_("Measured EV 2"), canonSiId, makerTags, unsignedShort, 1, printSi0x0017),
        TagInfo(0x0018, "0x0018", "0x0018", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonSiTag)", "(UnknownCanonSiTag)", N_("Unknown Canon Camera Settings 2 tag"), canonSiId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListSi()
    {
        return tagInfoSi_;
    }

    //! PanoramaDirection, tag 0x0005
    extern const TagDetails canonPaDirection[] = {
        { 0, N_("Left to right")          },
        { 1, N_("Right to left")          },
        { 2, N_("Bottom to top")          },
        { 3, N_("Top to bottom")          },
        { 4, N_("2x2 matrix (Clockwise)") }
    };

    // Canon Panorama Info
    const TagInfo CanonMakerNote::tagInfoPa_[] = {
        TagInfo(0x0002, "PanoramaFrame", N_("Panorama Frame"), N_("Panorama frame number"), canonPaId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0005, "PanoramaDirection", N_("Panorama Direction"), N_("Panorama direction"), canonPaId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonPaDirection)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCs2Tag)", "(UnknownCanonCs2Tag)", N_("Unknown Canon Panorama tag"), canonPaId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPa()
    {
        return tagInfoPa_;
    }

    // Canon Custom Function Tag Info
    const TagInfo CanonMakerNote::tagInfoCf_[] = {
        TagInfo(0x0001, "NoiseReduction", N_("Noise Reduction"), N_("Long exposure noise reduction"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0002, "ShutterAeLock", N_("Shutter Ae Lock"), N_("Shutter/AE lock buttons"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0003, "MirrorLockup", N_("Mirror Lockup"), N_("Mirror lockup"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0004, "ExposureLevelIncrements", N_("Exposure Level Increments"), N_("Tv/Av and exposure level"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0005, "AFAssist", N_("AF Assist"), N_("AF assist light"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0006, "FlashSyncSpeedAv", N_("Flash Sync Speed Av"), N_("Shutter speed in Av mode"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0007, "AEBSequence", N_("AEB Sequence"), N_("AEB sequence/auto cancellation"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0008, "ShutterCurtainSync", N_("Shutter Curtain Sync"), N_("Shutter curtain sync"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0009, "LensAFStopButton", N_("Lens AF Stop Button"), N_("Lens AF stop button Fn. Switch"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000a, "FillFlashAutoReduction", N_("Fill Flash Auto Reduction"), N_("Auto reduction of fill flash"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000b, "MenuButtonReturn", N_("Menu Button Return"), N_("Menu button return position"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000c, "SetButtonFunction", N_("Set Button Function"), N_("SET button func. when shooting"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000d, "SensorCleaning", N_("Sensor Cleaning"), N_("Sensor cleaning"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000e, "SuperimposedDisplay", N_("Superimposed Display"), N_("Superimposed display"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000f, "ShutterReleaseNoCFCard", N_("Shutter Release No CF Card"), N_("Shutter Release W/O CF Card"), canonCfId, makerTags, unsignedShort, 1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", N_("Unknown Canon Custom Function tag"), canonCfId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListCf()
    {
        return tagInfoCf_;
    }

    //! AFPointsUsed, tag 0x0016
    extern const TagDetailsBitmask canonPiAFPointsUsed[] = {
        { 0x01, N_("right")     },
        { 0x02, N_("mid-right") },
        { 0x04, N_("bottom")    },
        { 0x08, N_("center")    },
        { 0x10, N_("top")       },
        { 0x20, N_("mid-left")  },
        { 0x40, N_("left")      }
    };

    //! AFPointsUsed20D, tag 0x001a
    extern const TagDetailsBitmask canonPiAFPointsUsed20D[] = {
        { 0x001, N_("top")         },
        { 0x002, N_("upper-left")  },
        { 0x004, N_("upper-right") },
        { 0x008, N_("left")        },
        { 0x010, N_("center")      },
        { 0x020, N_("right")       },
        { 0x040, N_("lower-left")  },
        { 0x080, N_("lower-right") },
        { 0x100, N_("bottom")      }
    };

    // Canon Picture Info Tag
    const TagInfo CanonMakerNote::tagInfoPi_[] = {
        TagInfo(0x0002, "ImageWidth", N_("Image Width"), N_("Image width"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0003, "ImageHeight", N_("Image Height"), N_("Image height"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0004, "ImageWidthAsShot", N_("Image Width As Shot"), N_("Image width (as shot)"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0005, "ImageHeightAsShot", N_("Image Height As Shot"), N_("Image height (as shot)"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0016, "AFPointsUsed", N_("AF Points Used"), N_("AF points used"), canonPiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed)),
        TagInfo(0x001a, "AFPointsUsed20D", N_("AF Points Used 20D"), N_("AF points used (20D)"), canonPiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed20D)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPiTag)", "(UnknownCanonPiTag)", N_("Unknown Canon Picture Info tag"), canonPiId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPi()
    {
        return tagInfoPi_;
    }

    //! BracketMode, tag 0x0003
    extern const TagDetails canonBracketMode[] = {
        { 0, N_("Off") },
        { 1, N_("AEB") },
        { 2, N_("FEB") },
        { 3, N_("ISO") },
        { 4, N_("WB")  }
    };

    //! RawJpgSize, tag 0x0007
    extern const TagDetails canonRawJpgSize[] = {
        { 0,   N_("Large")        },
        { 1,   N_("Medium")       },
        { 2,   N_("Small")        },
        { 5,   N_("Medium 1")     },
        { 6,   N_("Medium 2")     },
        { 7,   N_("Medium 3")     },
        { 8,   N_("Postcard")     },
        { 9,   N_("Widescreen")   },
        { 129, N_("Medium Movie") },
        { 130, N_("Small Movie")  }
    };

    //! NoiseReduction, tag 0x0008
    extern const TagDetails canonNoiseReduction[] = {
        { 0, N_("Off")  },
        { 1, N_("On 1") },
        { 2, N_("On 2") },
        { 3, N_("On")   },
        { 4, N_("Auto") }
    };

    //! WBBracketMode, tag 0x0009
    extern const TagDetails canonWBBracketMode[] = {
        { 0, N_("Off")           },
        { 1, N_("On (shift AB)") },
        { 2, N_("On (shift GM)") }
    };

    //! FilterEffect, tag 0x000e
    extern const TagDetails canonFilterEffect[] = {
        { 0, N_("None")   },
        { 1, N_("Yellow") },
        { 2, N_("Orange") },
        { 3, N_("Red")    },
        { 4, N_("Green")  }
    };

    //! ToningEffect, tag 0x000e
    extern const TagDetails canonToningEffect[] = {
        { 0, N_("None")   },
        { 1, N_("Sepia")  },
        { 2, N_("Blue")   },
        { 3, N_("Purple") },
        { 4, N_("Green")  }
    };

    // Canon File Info Tag
    const TagInfo CanonMakerNote::tagInfoFi_[] = {
        TagInfo(0x0001, "FileNumber", N_("File Number"), N_("File Number"), canonFiId, makerTags, unsignedLong, 1, printFiFileNumber),
        TagInfo(0x0003, "BracketMode", N_("Bracket Mode"), N_("Bracket Mode"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonBracketMode)),
        TagInfo(0x0004, "BracketValue", N_("Bracket Value"), N_("Bracket Value"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0005, "BracketShotNumber", N_("Bracket Shot Number"), N_("Bracket Shot Number"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0006, "RawJpgQuality", N_("Raw Jpg Quality"), N_("Raw Jpg Quality"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsQuality)),
        TagInfo(0x0007, "RawJpgSize", N_("Raw Jpg Size"), N_("Raw Jpg Size"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonRawJpgSize)),
        TagInfo(0x0008, "NoiseReduction", N_("Noise Reduction"), N_("Noise Reduction"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonNoiseReduction)),
        TagInfo(0x0009, "WBBracketMode", N_("WB Bracket Mode"), N_("WB Bracket Mode"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonWBBracketMode)),
        TagInfo(0x000c, "WBBracketValueAB", N_("WB Bracket Value AB"), N_("WB Bracket Value AB"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000d, "WBBracketValueGM", N_("WB Bracket Value GM"), N_("WB Bracket Value GM"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000e, "FilterEffect", N_("Filter Effect"), N_("Filter Effect"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonFilterEffect)),
        TagInfo(0x000f, "ToningEffect", N_("Toning Effect"), N_("Toning Effect"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonToningEffect)),
        TagInfo(0x0010, "MacroMagnification", N_("Macro Magnification"), N_("Macro magnification"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0013, "LiveViewShooting", N_("Live View Shooting"), N_("Live view shooting"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)),
        TagInfo(0x0019, "FlashExposureLock", N_("Flash Exposure Lock"), N_("Flash exposure lock"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonFiTag)", "(UnknownCanonFiTag)", N_("Unknown Canon File Info tag"), canonFiId, makerTags, signedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListFi()
    {
        return tagInfoFi_;
    }

    //! Tone Curve Values
    extern const TagDetails canonToneCurve[] = {
        { 0, N_("Standard") },
	{ 1, N_("Manual")   },
	{ 2, N_("Custom")   }
    };

    //! Sharpness Frequency Values
    extern const TagDetails canonSharpnessFrequency[] = {
        { 0, N_("n/a")      },
        { 1, N_("Lowest")   },
        { 2, N_("Low")      },
        { 3, N_("Standard") },
        { 4, N_("High")     },
        { 5, N_("Highest")  }
    };

    //! PictureStyle Values
    extern const TagDetails canonPictureStyle[] = {
        { 0x00, N_("None")            },
        { 0x01, N_("Standard")        },
        { 0x02, N_("Portrait")        },
        { 0x03, N_("High Saturation") },
        { 0x04, N_("Adobe RGB")       },
        { 0x05, N_("Low Saturation")  },
        { 0x06, N_("CM Set 1")        },
        { 0x07, N_("CM Set 2")        },
        { 0x21, N_("User Def. 1")     },
        { 0x22, N_("User Def. 2")     },
        { 0x23, N_("User Def. 3")     },
        { 0x41, N_("PC 1")            },
        { 0x42, N_("PC 2")            },
        { 0x43, N_("PC 3")            },
        { 0x81, N_("Standard")        },
        { 0x82, N_("Portrait")        },
        { 0x83, N_("Landscape")       },
        { 0x84, N_("Neutral")         },
        { 0x85, N_("Faithful")        },
        { 0x86, N_("Monochrome")      },
    };

    // Canon Processing Info Tag
    const TagInfo CanonMakerNote::tagInfoPr_[] = {
        TagInfo(0x0001, "ToneCurve", N_("ToneCurve"), N_("Tone curve"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonToneCurve)),
        TagInfo(0x0002, "Sharpness", N_("Sharpness"), N_("Sharpness"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0003, "SharpnessFrequency", N_("SharpnessFrequency"), N_("Sharpness frequency"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonSharpnessFrequency)),
        TagInfo(0x0004, "SensorRedLevel", N_("SensorRedLevel"), N_("Sensor red level"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0005, "SensorBlueLevel", N_("SensorBlueLevel"), N_("Sensor blue level"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0006, "WhiteBalanceRed", N_("WhiteBalanceRed"), N_("White balance red"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0007, "WhiteBalanceBlue", N_("WhiteBalanceBlue"), N_("White balance blue"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0008, "WhiteBalance", N_("WhiteBalance"), N_("White balance"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)),
        TagInfo(0x0009, "ColorTemperature", N_("ColorTemperature"), N_("Color Temperature"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000a, "PictureStyle", N_("PictureStyle"), N_("Picture style"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonPictureStyle)),
        TagInfo(0x000b, "DigitalGain", N_("DigitalGain"), N_("Digital gain"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000c, "WBShiftAB", N_("WBShiftAB"), N_("WBShift AB"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000d, "WBShiftGM", N_("WBShiftGM"), N_("WB Shift GM"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0xffff, "(UnknownCanonPrTag)", "(UnknownCanonPrTag)", N_("Unknown Canon Processing Info tag"), canonPrId, makerTags, signedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPr()
    {
        return tagInfoPr_;
    }

    std::ostream& CanonMakerNote::printFiFileNumber(std::ostream& os,
                                                    const Value& value,
                                                    const ExifData* metadata)
    {
        if (   !metadata || value.typeId() != unsignedLong
            || value.count() == 0) return os << "(" << value << ")";

        ExifData::const_iterator pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end()) return os << "(" << value << ")";

        // Ported from Exiftool
        std::string model = pos->toString();
        if (   model.find("20D") != std::string::npos
            || model.find("350D") != std::string::npos
            || model.substr(model.size() - 8, 8) == "REBEL XT"
            || model.find("Kiss Digital N") != std::string::npos) {
            uint32_t val = value.toLong();
            uint32_t dn = (val & 0xffc0) >> 6;
            uint32_t fn = ((val >> 16) & 0xff) + ((val & 0x3f) << 8);
            return os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
        }
        if (   model.find("30D") != std::string::npos
            || model.find("400D") != std::string::npos
            || model.find("REBEL XTi") != std::string::npos
            || model.find("Kiss Digital X") != std::string::npos
            || model.find("K236") != std::string::npos) {
            uint32_t val = value.toLong();
            uint32_t dn = (val & 0xffc00) >> 10;
            while (dn < 100) dn += 0x40;
            uint32_t fn = ((val & 0x3ff) << 4) + ((val >> 20) & 0x0f);
            return os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
        }

        return os << "(" << value << ")";
    }

    std::ostream& CanonMakerNote::printFocalLength(std::ostream& os,
                                                   const Value& value,
                                                   const ExifData* metadata)
    {
        if (   !metadata
            || value.count() < 4
            || value.typeId() != unsignedShort) {
            return os << value;
        }

        ExifKey key("Exif.CanonCs.Lens");
        ExifData::const_iterator pos = metadata->findKey(key);
        if (   pos != metadata->end()
            && pos->value().count() >= 3
            && pos->value().typeId() == unsignedShort) {
            float fu = pos->value().toFloat(2);
            if (fu != 0.0) {
                float fl = value.toFloat(1) / fu;
                std::ostringstream oss;
                oss.copyfmt(os);
                os << std::fixed << std::setprecision(1);
                os << fl << " mm";
                os.copyfmt(oss);
                return os;
            }
        }

        return os << value;
    }

    std::ostream& CanonMakerNote::print0x0008(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::string n = value.toString();
        if (n.length() < 4) return os << "(" << n << ")";
        return os << n.substr(0, n.length() - 4) << "-"
                  << n.substr(n.length() - 4);
    }

    std::ostream& CanonMakerNote::print0x000c(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::istringstream is(value.toString());
        uint32_t l;
        is >> l;
        return os << std::setw(4) << std::setfill('0') << std::hex
                  << ((l & 0xffff0000) >> 16)
                  << std::setw(5) << std::setfill('0') << std::dec
                  << (l & 0x0000ffff);
    }

    std::ostream& CanonMakerNote::printCs0x0002(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        if (l == 0) {
            os << "Off";
        }
        else {
            os << l / 10.0 << " s";
        }
        return os;
    }

    //! Helper structure
    struct LensTypeAndFocalLength {
        long        lensType_;                  //!< Lens type
        std::string focalLength_;               //!< Focal length
    };

    //! Compare tag details with a lens entry
    bool operator==(const TagDetails& td, const LensTypeAndFocalLength& ltfl) {
        return (   td.val_ == ltfl.lensType_
                && std::string(td.label_).find(ltfl.focalLength_) != std::string::npos);
    }

    std::ostream& printCsLensByFocalLength(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata)
    {
        if (   !metadata || value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        LensTypeAndFocalLength ltfl;
        ltfl.lensType_ = value.toLong();

        ExifKey key("Exif.CanonCs.Lens");
        ExifData::const_iterator pos = metadata->findKey(key);
        if (   pos != metadata->end()
            && pos->value().count() >= 3
            && pos->value().typeId() == unsignedShort) {
            float fu = pos->value().toFloat(2);
            if (fu != 0.0) {
                float len1 = pos->value().toLong(0) / fu;
                float len2 = pos->value().toLong(1) / fu;
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(0);
                if (len1 == len2) {
                    oss << len1 << "mm";
                } else {
                    oss << len2 << "-" << len1 << "mm";
                }
                ltfl.focalLength_ = oss.str();
            }
        }
        if (ltfl.focalLength_.empty()) return os << value;

        const TagDetails* td = find(canonCsLensType, ltfl);
        if (!td) return os << value;
        return os << td->label_;
    }

    std::ostream& CanonMakerNote::printCsLensType(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData* metadata)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << "(" << value << ")";

        const LensIdFct* lif = find(lensIdFct, value.toLong());
        if (!lif) {
            return EXV_PRINT_TAG(canonCsLensType)(os, value, metadata);
        }
        if (metadata && lif->fct_) {
            return lif->fct_(os, value, metadata);
        }
        return os << value;
    }

    std::ostream& CanonMakerNote::printCsLens(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        if (   value.count() < 3
            || value.typeId() != unsignedShort) {
            return os << "(" << value << ")";
        }

        float fu = value.toFloat(2);
        if (fu == 0.0) return os << value;
        float len1 = value.toLong(0) / fu;
        float len2 = value.toLong(1) / fu;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1);
        if (len1 == len2) {
            os << len1 << " mm";
        } else {
            os << len2 << " - " << len1 << " mm";
        }
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0002(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() == unsignedShort
            && value.count() > 0) {
            // Ported from Exiftool by Will Stokes
            os << exp(canonEv(value.toLong()) * log(2.0)) * 100.0 / 32.0;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0003(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() == unsignedShort
            && value.count() > 0) {
            // The offset of '5' seems to be ok for most Canons (see Exiftool)
            // It might be explained by the fakt, that most Canons have a longest
            // exposure of 30s which is 5 EV below 1s
            // see also printSi0x0017
            std::ostringstream oss;
            oss.copyfmt(os);
            int res = static_cast<int>(100.0 * (value.toLong() / 32.0 + 5.0) + 0.5);
            os << std::fixed << std::setprecision(2) << res / 100.0;
            os.copyfmt(oss);
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0009(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x000e(std::ostream& os,
                                                const Value& value,
                                                const ExifData* pExifData)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        long num = (l & 0xf000) >> 12;
        os << num << " focus points; ";
        long used = l & 0x0fff;
        if (used == 0) {
            os << "none";
        }
        else {
            EXV_PRINT_TAG_BITMASK(canonSiAFPointUsed)(os, value, pExifData);
        }
        os << " used";
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0013(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        if (l == 0xffff) {
            os << "Infinite";
        }
        else {
            os << l << "";
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0015(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        long val = static_cast<int16_t>(value.toLong());
        if (val < 0) return os << value;
        os << std::setprecision(2)
           << "F" << fnumber(canonEv(val));
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0016(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        URational ur = exposureTime(canonEv(value.toLong()));
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        return os << " s";
    }

    std::ostream& CanonMakerNote::printSi0x0017(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2)
           << value.toLong() / 8.0 - 6.0;
        os.copyfmt(oss);
        return os;
    }

// *****************************************************************************
// free functions

    float canonEv(long val)
    {
        // temporarily remove sign
        int sign = 1;
        if (val < 0) {
            sign = -1;
            val = -val;
        }
        // remove fraction
        float frac = static_cast<float>(val & 0x1f);
        val -= long(frac);
        // convert 1/3 (0x0c) and 2/3 (0x14) codes
        if (frac == 0x0c) {
            frac = 32.0f / 3;
        }
        else if (frac == 0x14) {
            frac = 64.0f / 3;
        }
        return sign * (val + frac) / 32.0f;
    }

}}                                      // namespace Internal, Exiv2
