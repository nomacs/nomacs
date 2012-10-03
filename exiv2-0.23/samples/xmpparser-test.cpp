// ***************************************************************** -*- C++ -*-
// xmpparser-test.cpp, $Rev: 2286 $
// Read an XMP packet from a file, parse and re-serialize it.

#include <exiv2/exiv2.hpp>

#include <string>
#include <iostream>
#include <iomanip>

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string filename(argv[1]);
    Exiv2::DataBuf buf = Exiv2::readFile(filename);
    std::string xmpPacket;
    xmpPacket.assign(reinterpret_cast<char*>(buf.pData_), buf.size_);
    std::cerr << "-----> Decoding XMP data read from " << filename << " <-----\n";
    Exiv2::XmpData xmpData;
    if (0 != Exiv2::XmpParser::decode(xmpData, xmpPacket)) {
        std::string error(argv[1]);
        error += ": Failed to parse file contents (XMP packet)";
        throw Exiv2::Error(1, error);
    }
    if (xmpData.empty()) {
        std::string error(argv[1]);
        error += ": No XMP properties found in the XMP packet";
        throw Exiv2::Error(1, error);
    }
    for (Exiv2::XmpData::const_iterator md = xmpData.begin(); 
         md != xmpData.end(); ++md) {
        std::cout << std::setfill(' ') << std::left
                  << std::setw(44)
                  << md->key() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << md->count() << "  "
                  << std::dec << md->value()
                  << std::endl;
    }
    filename += "-new";
    std::cerr << "-----> Encoding XMP data to write to " << filename << " <-----\n";
    if (0 != Exiv2::XmpParser::encode(xmpPacket, xmpData)) {
        std::string error(argv[1]);
        error += ": Failed to encode the XMP data";
        throw Exiv2::Error(1, error);
    }
    Exiv2::FileIo file(filename);
    if (file.open("wb") != 0) {
        throw Exiv2::Error(10, filename, "wb", Exiv2::strError());
    }
    if (file.write(reinterpret_cast<const Exiv2::byte*>(xmpPacket.data()), static_cast<long>(xmpPacket.size())) == 0) {
        throw Exiv2::Error(2, filename, Exiv2::strError(), "FileIo::write");
    }
    Exiv2::XmpParser::terminate();
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
