// ***************************************************************** -*- C++ -*-
// xmpdump.cpp, $Rev: 1512 $
// Sample program to dump the XMP packet of an image

#include "image.hpp"
#include <string>
#include <iostream>
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

    const std::string& xmpPacket = image->xmpPacket();
    if (xmpPacket.empty()) {
        std::string error(argv[1]);
        error += ": No XMP packet found in the file";
        throw Exiv2::Error(1, error);
    }
    std::cout << xmpPacket << "\n";

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
