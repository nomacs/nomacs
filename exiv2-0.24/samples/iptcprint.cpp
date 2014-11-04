// ***************************************************************** -*- C++ -*-
// iptcprint.cpp, $Rev: 3090 $
// Sample program to print the IPTC metadata of an image

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
    assert (image.get() != 0);
    image->readMetadata();

    Exiv2::IptcData &iptcData = image->iptcData();
    if (iptcData.empty()) {
        std::string error(argv[1]);
        error += ": No IPTC data found in the file";
        throw Exiv2::Error(1, error);
    }

    Exiv2::IptcData::iterator end = iptcData.end();
    for (Exiv2::IptcData::iterator md = iptcData.begin(); md != end; ++md) {
        std::cout << std::setw(44) << std::setfill(' ') << std::left
                  << md->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << md->tag() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << md->count() << "  "
                  << std::dec << md->value()
                  << std::endl;
    }

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
