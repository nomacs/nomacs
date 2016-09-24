// ***************************************************************** -*- C++ -*-
// easyaccess-test.cpp, $Rev: 3090 $
// Sample program using high-level metadata access functions

// included header files
#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

typedef Exiv2::ExifData::const_iterator (*EasyAccessFct)(const Exiv2::ExifData& ed);

struct EasyAccess {
    const char*   label_;
    EasyAccessFct findFct_;
};

static const EasyAccess easyAccess[] = {
    { "Orientation",          Exiv2::orientation      },
    { "ISO speed",            Exiv2::isoSpeed         },
    { "Flash bias",           Exiv2::flashBias        },
    { "Exposure mode",        Exiv2::exposureMode     },
    { "Scene mode",           Exiv2::sceneMode        },
    { "Macro mode",           Exiv2::macroMode        },
    { "Image quality",        Exiv2::imageQuality     },
    { "White balance",        Exiv2::whiteBalance     },
    { "Lens name",            Exiv2::lensName         },
    { "Saturation",           Exiv2::saturation       },
    { "Sharpness",            Exiv2::sharpness        },
    { "Contrast",             Exiv2::contrast         },
    { "Scene capture type",   Exiv2::sceneCaptureType },
    { "Metering mode",        Exiv2::meteringMode     },
    { "Camera make",          Exiv2::make             },
    { "Camera model",         Exiv2::model            },
    { "Exposure time",        Exiv2::exposureTime     },
    { "FNumber",              Exiv2::fNumber          },
    { "Subject distance",     Exiv2::subjectDistance  },
    { "Camera serial number", Exiv2::serialNumber     },
    { "Focal length",         Exiv2::focalLength      },
    { "AF point",             Exiv2::afPoint          }
};

int main(int argc, char **argv)
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
    assert (image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData& ed = image->exifData();

    for (unsigned int i = 0; i < EXV_COUNTOF(easyAccess); ++i) {
        Exiv2::ExifData::const_iterator pos = easyAccess[i].findFct_(ed);
        std::cout << std::setw(20) << std::left << easyAccess[i].label_;
        if (pos != ed.end()) {
            std::cout << " (" << std::setw(35) << pos->key() << ") : "
                      << pos->print(&ed) << "\n";
        }
        else {
            std::cout << " (" << std::setw(35) << " " << ") : \n";
        }
    }

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
