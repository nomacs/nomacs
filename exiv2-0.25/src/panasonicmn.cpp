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
  File:      panasonicmn.cpp
  Version:   $Rev: 3831 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier dot gilles at gmail dot com>
  History:   11-Jun-04, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: panasonicmn.cpp 3831 2015-05-20 01:27:32Z asp $")

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
        {  1, N_("TIFF")           },
        {  2, N_("High")           },
        {  3, N_("Normal")         },
        {  6, N_("Very High")      },
        {  7, N_("Raw")            },
        {  9, N_("Motion Picture") },
        { 11, N_("Full HD Movie")  },
        { 12, N_("4k Movie")       }
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
        { 12, N_("Shade")           },
        { 13, N_("Kelvin")          }
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails panasonicFocusMode[] = {
        {  1, N_("Auto")               },
        {  2, N_("Manual")             },
        {  4, N_("Auto, focus button") },
        {  5, N_("Auto, continuous")   },
        {  6, N_("AF-S")               },
        {  7, N_("AF-C")               },
        {  8, N_("AF-F")               }
    };

    //! ImageStabilizer, tag 0x001a
    extern const TagDetails panasonicImageStabilizer[] = {
        {  2, N_("On, Mode 1") },
        {  3, N_("Off")        },
        {  4, N_("On, Mode 2") },
        {  5, N_("Panning")    },
        {  6, N_("On, Mode 3") },
    };

    //! Macro, tag 0x001c
    extern const TagDetails panasonicMacro[] = {
        {   1, N_("On")         },
        {   2, N_("Off")        },
        { 257, N_("Tele-macro") },
        { 513, N_("Macro-zoom") }
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
        { 16, N_("Self Portrait")                  },
        { 17, N_("Economy")                        },
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
        { 35, N_("Clipboard")                      },
        { 36, N_("High speed continuous shooting") },
        { 37, N_("Intelligent auto")               },
        { 39, N_("Multi-aspect")                   },
        { 41, N_("Transform")                      },
        { 42, N_("Flash Burst")                    },
        { 43, N_("Pin Hole")                       },
        { 44, N_("Film Grain")                     },
        { 45, N_("My Color")                       },
        { 46, N_("Photo Frame")                    },
        { 51, N_("HDR")                            },
        { 55, N_("Handheld Night Shot")            },
        { 57, N_("3D")                             },
        { 59, N_("Creative Control")               },
        { 62, N_("Panorama")                       },
        { 63, N_("Glass Through")                  },
        { 64, N_("HDR")                            },
        { 66, N_("Digital Filter")                 },
        { 67, N_("Clear Portrait")                 },
        { 68, N_("Silky Skin")                     },
        { 69, N_("Backlit Softness")               },
        { 70, N_("Clear in Backlight")             },
        { 71, N_("Relaxing Tone")                  },
        { 72, N_("Sweet Child's Face")             },
        { 73, N_("Distinct Scenery")               },
        { 74, N_("Bright Blue Sky")                },
        { 75, N_("Romantic Sunset Glow")           },
        { 76, N_("Vivid Sunset Glow")              },
        { 77, N_("Glistening Water")               },
        { 78, N_("Clear Nightscape")               },
        { 79, N_("Cool Night Sky")                 },
        { 80, N_("Warm Glowing Nightscape")        },
        { 81, N_("Artistic Nightscape")            },
        { 82, N_("Glittering Illuminations")       },
        { 83, N_("Clear Night Portrait")           },
        { 84, N_("Soft Image of a Flower")         },
        { 85, N_("Appetizing Food")                },
        { 86, N_("Cute Desert")                    },
        { 87, N_("Freeze Animal Motion")           },
        { 88, N_("Clear Sports Shot")              },
        { 89, N_("Monochrome")                     },
        { 90, N_("Creative Control")               }
    };

    //! Audio, tag 0x0020
    extern const TagDetails panasonicAudio[] = {
        { 1, N_("Yes")    },
        { 2, N_("No")     },
        { 3, N_("Stereo") }
    };

    //! ColorEffect, tag 0x0028
    extern const TagDetails panasonicColorEffect[] = {
        { 1, N_("Off")             },
        { 2, N_("Warm")            },
        { 3, N_("Cool")            },
        { 4, N_("Black and white") },
        { 5, N_("Sepia")           },
        { 6, N_("Happy")           }
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
        { 1, N_("Off")                },
        { 2, "10 s"               },
        { 3, "2 s"                },
        { 4, "10 s / 3 pictures"  }
    };

    //! Rotation, tag 0x0030
    extern const TagDetails panasonicRotation[] = {
        { 1, N_("Horizontal (normal)") },
        { 3, N_("Rotate 180")          },
        { 6, N_("Rotate 90 CW")        },
        { 8, N_("Rotate 270 CW")       }
    };

    //! AFAssistLamp, tag 0x0031
    extern const TagDetails panasonicAFAssistLamp[] = {
        { 1, N_("Fired")                     },
        { 2, N_("Enabled but Not Used")      },
        { 3, N_("Disabled but Required")     },
        { 4, N_("Disabled and Not Required") }
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

    //! TextStamp, tag 0x003b, 0x003e, 000x8008 and 0x8009
    extern const TagDetails panasonicTextStamp[] = {
        { 1, N_("Off") },
        { 2, N_("On")  }
    };

    //! FilmMode, tag 0x0042
    extern const TagDetails panasonicFilmMode[] = {
        {  1, N_("Standard (color)") },
        {  2, N_("Dynamic (color)")  },
        {  3, N_("Nature (color)")   },
        {  4, N_("Smooth (color)")   },
        {  5, N_("Standard (B&W)")   },
        {  6, N_("Dynamic (B&W)")    },
        {  7, N_("Smooth (B&W)")     },
        { 10, N_("Nostalgic")        },
        { 11, N_("Vibrant")          }
    };

    //! Bracket Settings, tag 0x0045
    extern const TagDetails panasonicBracketSettings[] = {
        { 0, N_("No Bracket")               },
        { 1, N_("3 images, Sequence 0/-/+") },
        { 2, N_("3 images, Sequence -/0/+") },
        { 3, N_("5 images, Sequence 0/-/+") },
        { 4, N_("5 images, Sequence -/0/+") },
        { 5, N_("7 images, Sequence 0/-/+") },
        { 6, N_("7 images, Sequence -/0/+") }
    };

    //! Flash curtain, tag 0x0048
    extern const TagDetails panasonicFlashCurtain[] = {
        { 0, N_("n/a") },
        { 1, N_("1st") },
        { 2, N_("2nd") }
    };

    //! Long Shutter Noise Reduction, tag 0x0049
    extern const TagDetails panasonicLongShutterNoiseReduction[] = {
        { 1, N_("Off") },
        { 2, N_("On") }
    };

    //! Intelligent exposure, tag 0x005d
    extern const TagDetails panasonicIntelligentExposure[] = {
        { 0, N_("Off")      },
        { 1, N_("Low")      },
        { 2, N_("Standard") },
        { 3, N_("High")     }
    };

    //! Flash warning, tag 0x0062
    extern const TagDetails panasonicFlashWarning[] = {
        { 0, N_("No")                               },
        { 1, N_("Yes (flash required but disabled") }
    };

    //! Intelligent resolution, tag 0x0070
    extern const TagDetails panasonicIntelligentResolution[] = {
        { 0, N_("Off")      },
        { 1, N_("Low")      },
        { 2, N_("Standard") },
        { 3, N_("High")     },
        { 4, N_("Extended") }
    };

    //! Intelligent D-Range, tag 0x0079
    extern const TagDetails panasonicIntelligentDRange[] = {
        { 0, N_("Off")      },
        { 1, N_("Low")      },
        { 2, N_("Standard") },
        { 3, N_("High")     }
    };

    //! Clear Retouch, tag 0x007c
    extern const TagDetails panasonicClearRetouch[] = {
        { 0, N_("Off") },
        { 1, N_("On") }
    };

    //! Photo Style, tag 0x0089
    extern const TagDetails panasonicPhotoStyle[] = {
        { 0, N_("NoAuto")             },
        { 1, N_("Standard or Custom") },
        { 2, N_("Vivid")              },
        { 3, N_("Natural")            },
        { 4, N_("Monochrome")         },
        { 5, N_("Scenery")            },
        { 6, N_("Portrait")           }
    };

    //! Shading compensation, tag 0x008a
    extern const TagDetails panasonicShadingCompensation[] = {
        { 0, N_("Off") },
        { 1, N_("On") }
    };

    //! Camera orientation, tag 0x008f
    extern const TagDetails panasonicCameraOrientation[] = {
        { 0, N_("Normal")         },
        { 1, N_("Rotate CW")      },
        { 2, N_("Rotate 180")     },
        { 3, N_("Rotate CCW")     },
        { 4, N_("Tilt upwards")   },
        { 5, N_("Tilt downwards") }
    };

    //! Sweep panorama direction, tag 0x0093
    extern const TagDetails panasonicSweepPanoramaDirection[] = {
        { 0, N_("Off")           },
        { 1, N_("Left to Right") },
        { 2, N_("Right to Left") },
        { 3, N_("Top to Bottom") },
        { 4, N_("Bottom to Top") }
    };

    //! Timer recording, tag 0x0096
    extern const TagDetails panasonicTimerRecording[] = {
        { 0, N_("Off")                   },
        { 1, N_("Time Lapse")            },
        { 2, N_("Stop-Motion Animation") }
    };

    //! HDR, tag 0x009e
    extern const TagDetails panasonicHDR[] = {
        {     0, N_("Off")         },
        {   100, N_("1 EV")        },
        {   200, N_("2 EV")        },
        {   300, N_("3 EV")        },
        { 32868, N_("1 EV (Auto)") },
        { 32968, N_("2 EV (Auto)") },
        { 33068, N_("3 EV (Auto)") }
    };

    //! Shutter Type, tag 0x009f
    extern const TagDetails panasonicShutterType[] = {
        { 0, N_("Mechanical") },
        { 1, N_("Electronic") },
        { 2, N_("Hybrid")     }
    };

    //! Touch AE, tag 0x00ab
    extern const TagDetails panasonicTouchAE[] = {
        { 0, N_("Off") },
        { 1, N_("On")  }
    };

    //! Flash Fired, tag 0x8007
    extern const TagDetails panasonicFlashFired[] = {
        { 1, N_("No")  },
        { 2, N_("Yes") }
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
        TagInfo(0x0025, "InternalSerialNumber", N_("Internal Serial Number"), N_("This number is unique, and contains the date of manufacture, but is not the same as the number printed on the camera body."), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x0026, "ExifVersion", "Exif Version", N_("Exif version"), panasonicId, makerTags, undefined, -1, printExifVersion),
        TagInfo(0x0027, "0x0027", "0x0027", N_("Unknown"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0028, "ColorEffect", N_("Color Effect"), N_("Color effect"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicColorEffect)),
        TagInfo(0x0029, "TimeSincePowerOn", "Time since Power On", N_("Time in 1/100 s from when the camera was powered on to when the image is written to memory card"), panasonicId, makerTags, unsignedLong, -1, print0x0029),
        TagInfo(0x002a, "BurstMode", N_("Burst Mode"), N_("Burst mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicBurstMode)),
        TagInfo(0x002b, "SequenceNumber", N_("Sequence Number"), N_("Sequence number"), panasonicId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x002c, "Contrast", N_("Contrast"), N_("Contrast setting"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicContrast)),
        TagInfo(0x002d, "NoiseReduction", N_("NoiseReduction"), N_("Noise reduction"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicNoiseReduction)),
        TagInfo(0x002e, "SelfTimer", N_("Self Timer"), N_("Self timer"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicSelfTimer)),
        TagInfo(0x002f, "0x002f", "0x002f", N_("Unknown"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0030, "Rotation", N_("Rotation"), N_("Rotation"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicRotation)),
        TagInfo(0x0031, "AFAssistLamp", N_("AF Assist Lamp"), N_("AF Assist Lamp"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicAFAssistLamp)),
        TagInfo(0x0032, "ColorMode", N_("Color Mode"), N_("Color mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicColorMode)),
        TagInfo(0x0033, "BabyAge1", N_("Baby Age 1"), N_("Baby (or pet) age 1"), panasonicId, makerTags, asciiString, -1, print0x0033),
        TagInfo(0x0034, "OpticalZoomMode", N_("Optical Zoom Mode"), N_("Optical zoom mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicOpticalZoomMode)),
        TagInfo(0x0035, "ConversionLens", N_("Conversion Lens"), N_("Conversion lens"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicConversionLens)),
        TagInfo(0x0036, "TravelDay", N_("Travel Day"), N_("Travel day"), panasonicId, makerTags, unsignedShort, -1, print0x0036),
        TagInfo(0x0039, "Contrast", N_("Contrast"), N_("Contrast"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x003a, "WorldTimeLocation", N_("World Time Location"), N_("World time location"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicWorldTimeLocation)),
        TagInfo(0x003b, "TextStamp1", N_("Text Stamp 1"), N_("Text Stamp 1"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)),
        TagInfo(0x003c, "ProgramISO", N_("Program ISO"), N_("Program ISO"), panasonicId, makerTags, unsignedShort, -1, print0x003c),
        TagInfo(0x003d, "AdvancedSceneType", N_("Advanced Scene Type"), N_("Advanced Scene Type"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x003e, "TextStamp2", N_("Text Stamp 2"), N_("Text Stamp 2"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)),
        TagInfo(0x003f, "FacesDetected", N_("Faces detected"), N_("Faces detected"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0040, "Saturation", N_("Saturation"), N_("Saturation"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0041, "Sharpness", N_("Sharpness"), N_("Sharpness"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0042, "FilmMode", N_("Film Mode"), N_("Film mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicFilmMode)),
        TagInfo(0x0044, "ColorTempKelvin", N_("Color Temp Kelvin"), N_("Color Temperatur in Kelvin"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0045, "BracketSettings", N_("Bracket Settings"), N_("Bracket Settings"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicBracketSettings)),
        TagInfo(0x0046, "WBAdjustAB", N_("WB Adjust AB"), N_("WB adjust AB. Positive is a shift toward blue."), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0047, "WBAdjustGM", N_("WB Adjust GM"), N_("WBAdjustGM. Positive is a shift toward green."), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0048, "FlashCurtain", N_("Flash Curtain"), N_("Flash Curtain"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicFlashCurtain)),
        TagInfo(0x0049, "LongShutterNoiseReduction", N_("Long Shutter Noise Reduction"), N_("Long Shutter Noise Reduction"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicLongShutterNoiseReduction)),
        TagInfo(0x004b, "ImageWidth", N_("Image width"), N_("Image width"), panasonicId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x004c, "ImageHeight", N_("Image height"), N_("Image height"), panasonicId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x004d, "AFPointPosition", N_("AF Point Position"), N_("AF Point Position"), panasonicId, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x004e, "FaceDetInfo", N_("Face detection info"), N_("Face detection info"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x0051, "LensType", N_("Lens Type"), N_("Lens type"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0052, "LensSerialNumber", N_("Lens Serial Number"), N_("Lens serial number"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0053, "AccessoryType", N_("Accessory Type"), N_("Accessory type"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0054, "AccessorySerialNumber", N_("Accessory Serial Number"), N_("Accessory Serial Number"), panasonicId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0059, "Transform1", N_("Transform 1"), N_("Transform 1"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x005d, "IntelligentExposure", N_("Intelligent Exposure"), N_("Intelligent Exposure"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicIntelligentExposure)),
        TagInfo(0x0060, "LensFirmwareVersion", N_("Firmware Version of the Lens"), N_("Firmware Version of the Lens"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x0061, "FaceRecInfo", N_("Face recognition info"), N_("Face recognition info"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x0062, "FlashWarning", N_("Flash Warning"), N_("Flash warning"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicFlashWarning)),
        TagInfo(0x0065, "Title", N_("Title"), N_("Title"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x0066, "BabyName", N_("Baby Name"), N_("Baby name (or pet name)"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x0067, "Location", N_("Location"), N_("Location"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x0069, "Country", N_("Country"), N_("Country"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x006b, "State", N_("State"), N_("State"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x006d, "City", N_("City"), N_("City"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x006f, "Landmark", N_("Landmark"), N_("Landmark"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x0070, "IntelligentResolution", N_("Intelligent resolution"), N_("Intelligent resolution"), panasonicId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(panasonicIntelligentResolution)),
        TagInfo(0x0077, "BurstSheed", N_("Burst Speed"), N_("Burst Speed in pictures per second"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0079, "IntelligentDRange", N_("Intelligent Dynamic Range"), N_("Intelligent Dynamic Range"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicIntelligentDRange)),
        TagInfo(0x007c, "ClearRetouch", N_("Clear Retouch"), N_("Clear Retouch"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicClearRetouch)),
        TagInfo(0x0080, "City2", N_("City2"), N_("City2"), panasonicId, makerTags, undefined, -1, printPanasonicText),
        TagInfo(0x0086, "ManometerPressure", N_("Manometer Pressure"), N_("Manometer pressure"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0089, "PhotoStyle", N_("Photo style"), N_("Photo style"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicPhotoStyle)),
        TagInfo(0x008a, "ShadingCompensation", N_("Shading Compensation"), N_("Shading Compensation"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicShadingCompensation)),
        TagInfo(0x008c, "AccelerometerZ", N_("Accelerometer Z"), N_("positive is acceleration upwards"), panasonicId, makerTags, unsignedShort, -1, printAccelerometer),
        TagInfo(0x008d, "AccelerometerX", N_("Accelerometer X"), N_("positive is acceleration to the left"), panasonicId, makerTags, unsignedShort, -1, printAccelerometer),
        TagInfo(0x008e, "AccelerometerY", N_("Accelerometer Y"), N_("positive is acceleration backwards"), panasonicId, makerTags, unsignedShort, -1, printAccelerometer),
        TagInfo(0x008f, "CameraOrientation", N_("Camera Orientation"), N_("Camera Orientation"), panasonicId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(panasonicCameraOrientation)),
        TagInfo(0x0090, "RollAngle", N_("Roll Angle"), N_("degress of clockwise camera rotation"), panasonicId, makerTags, unsignedShort, -1, printRollAngle),
        TagInfo(0x0091, "PitchAngle", N_("Pitch Angle"), N_("degress of upwards camera tilt"), panasonicId, makerTags, unsignedShort, -1, printPitchAngle),
        TagInfo(0x0093, "SweepPanoramaDirection", N_("Sweep Panorama Direction"), N_("Sweep Panorama Direction"), panasonicId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(panasonicSweepPanoramaDirection)),
        TagInfo(0x0094, "PanoramaFieldOfView", N_("Field of View of Panorama"), N_("Field of View of Panorama"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0096, "TimerRecording", N_("Timer Recording"), N_("Timer Recording"), panasonicId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(panasonicTimerRecording)),
        TagInfo(0x009d, "InternalNDFilter", N_("Internal ND Filter"), N_("Internal ND Filter"), panasonicId, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x009e, "HDR", N_("HDR"), N_("HDR"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicHDR)),
        TagInfo(0x009f, "ShutterType", N_("Shutter Type"), N_("Shutter Type"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicShutterType)),
        TagInfo(0x00a3, "ClearRetouchValue", N_("Clear Retouch Value"), N_("Clear Retouch Value"), panasonicId, makerTags, unsignedRational, -1, printValue),
        TagInfo(0x00ab, "TouchAE", N_("TouchAE"), N_("TouchAE"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicTouchAE)),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x4449, "0x4449", "0x4449", N_("Unknown"), panasonicId, makerTags, undefined, -1, printValue),
        TagInfo(0x8000, "MakerNoteVersion", N_("MakerNote Version"), N_("MakerNote version"), panasonicId, makerTags, undefined, -1, printExifVersion),
        TagInfo(0x8001, "SceneMode", N_("Scene Mode"), N_("Scene mode"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicShootingMode)),
        TagInfo(0x8004, "WBRedLevel", N_("WB Red Level"), N_("WB red level"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x8005, "WBGreenLevel", N_("WB Green Level"), N_("WB green level"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x8006, "WBBlueLevel", N_("WB Blue Level"), N_("WB blue level"), panasonicId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x8007, "FlashFired", N_("Flash Fired"), N_("Flash Fired"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicFlashFired)),
        TagInfo(0x8008, "TextStamp3", N_("Text Stamp 3"), N_("Text Stamp 3"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)),
        TagInfo(0x8009, "TextStamp4", N_("Text Stamp 4"), N_("Text Stamp 4"), panasonicId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)),
        TagInfo(0x8010, "BabyAge2", N_("Baby Age 2"), N_("Baby (or pet) age 2"), panasonicId, makerTags, asciiString, -1, print0x0033),
        TagInfo(0x8012, "Transform2", N_("Transform 2"), N_("Transform 2"), panasonicId, makerTags, undefined, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownPanasonicMakerNoteTag)", "(UnknownPanasonicMakerNoteTag)", N_("Unknown PanasonicMakerNote tag"), panasonicId, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* PanasonicMakerNote::tagList()
    {
        return tagInfo_;
    }

    // tag Focus Mode
    std::ostream& PanasonicMakerNote::print0x000f(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        if (value.count() < 2 || value.typeId() != unsignedByte) {
            return os << value;
        }
        long l0 = value.toLong(0);
        long l1 = value.toLong(1);
        if      (l0 ==   0 && l1 ==  1) os << _("Spot mode on or 9 area");
        else if (l0 ==   0 && l1 == 16) os << _("Spot mode off or 3-area (high speed)");
        else if (l0 ==   0 && l1 == 23) os << _("23-area");
        else if (l0 ==   1 && l1 ==  0) os << _("Spot focussing");
        else if (l0 ==   1 && l1 ==  1) os << _("5-area");
        else if (l0 ==  16 && l1 ==  0) os << _("1-area");
        else if (l0 ==  16 && l1 == 16) os << _("1-area (high speed)");
        else if (l0 ==  32 && l1 ==  0) os << _("3-area (auto)");
        else if (l0 ==  32 && l1 ==  1) os << _("3-area (left)");
        else if (l0 ==  32 && l1 ==  2) os << _("3-area (center)");
        else if (l0 ==  32 && l1 ==  3) os << _("3-area (right)");
        else if (l0 ==  64 && l1 ==  0) os << _("Face Detect");
        else if (l0 == 128 && l1 ==  0) os << _("Spot Focusing 2");
        else os << value;
        return os;
    } // PanasonicMakerNote::print0x000f

    // tag White balance bias
    std::ostream& PanasonicMakerNote::print0x0023(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1)
           << value.toLong() / 3 << _(" EV");
        os.copyfmt(oss);

        os.flags(f);
        return os;

    } // PanasonicMakerNote::print0x0023

    // Time since power on
    std::ostream& PanasonicMakerNote::print0x0029(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        long time=value.toLong();
        os << std::setw(2) << std::setfill('0') << time / 360000 << ":"
           << std::setw(2) << std::setfill('0') << (time % 360000) / 6000 << ":"
           << std::setw(2) << std::setfill('0') << (time % 6000) / 100 << "."
           << std::setw(2) << std::setfill('0') << time % 100;
        os.copyfmt(oss);

        return os;

    } // PanasonicMakerNote::print0x0029

    // bage age
    std::ostream& PanasonicMakerNote::print0x0033(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        if(value.toString()=="9999:99:99 00:00:00")
        {
            os << N_("not set");
        }
        else
        {
            os << value;
        };
        return os;
    } // PanasonicMakerNote::print0x0033

    // Travel days
    std::ostream& PanasonicMakerNote::print0x0036(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        if(value.toLong()==65535)
        {
            os << N_("not set");
        }
        else
        {
            os << value;
        };
        return os;
    } // PanasonicMakerNote::print0x0036

    // Program ISO
    std::ostream& PanasonicMakerNote::print0x003c(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        switch(value.toLong())
        {
            case 65534:
                os << N_("Intelligent ISO");
                break;
            case 65535:
                os << N_("n/a");
                break;
            default:
                os << value;
                break;
        };
        return os;
    } // PanasonicMakerNote::print0x003c

    std::ostream& PanasonicMakerNote::printPanasonicText(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData*)
    {
        if(value.size()>0 && value.typeId() == undefined)
        {
            for(long i=0; i< value.size(); i++)
            {
                if(value.toLong(i)==0)
                {
                    break;
                };
                os << static_cast<char>(value.toLong(i));
            };
            return os;
        }
        else
        {
            return os << value;
        };
    } // PanasonicMakerNote::printPanasonicText

    std::ostream& PanasonicMakerNote::printAccelerometer(std::ostream& os, const Value& value, const ExifData*)
    {
        // value is stored as unsigned int, but should be readed as signed int, so manually convert it
        int i = value.toLong();
        i = i - ((i & 0x8000) >> 15) * 0xffff;
        return os << i;
    }  // PanasonicMakerNote::printAccelerometer

    std::ostream& PanasonicMakerNote::printRollAngle(std::ostream& os, const Value& value, const ExifData*)
    {
        // roll angle is stored as signed int, but tag states to be unsigned int
        int i = value.toLong();
        i = i - ((i & 0x8000) >> 15) * 0xffff;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1) << i / 10.0;
        os.copyfmt(oss);

        return os;
    }  // PanasonicMakerNote::printRollAngle

    std::ostream& PanasonicMakerNote::printPitchAngle(std::ostream& os, const Value& value, const ExifData*)
    {
        // pitch angle is stored as signed int, but tag states to be unsigned int
        // change sign to be compatible with ExifTool: positive is upwards
        int i = value.toLong();
        i = i - ((i & 0x8000) >> 15) * 0xffff;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1) << -i / 10.0;
        os.copyfmt(oss);

        return os;
    }  // PanasonicMakerNote::printPitchAngle

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
