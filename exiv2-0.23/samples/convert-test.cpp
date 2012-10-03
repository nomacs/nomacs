// ***************************************************************** -*- C++ -*-
// convert-test.cpp, $Rev: 2286 $
// Conversion test driver - make sure you have a copy of the input file around!

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::XmpData xmpData;
    Exiv2::copyExifToXmp(image->exifData(), xmpData);

    Exiv2::ExifData exifData;
    Exiv2::copyXmpToExif(xmpData, exifData);

    image->setXmpData(xmpData);
    image->setExifData(exifData);
    image->writeMetadata();
    
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
