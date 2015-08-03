// ***************************************************************** -*- C++ -*-
// crwparse.cpp, $Rev: 1560 $
// Print the CIFF structure of a CRW file

#include "crwimage.hpp"
#include "crwimage_int.hpp"
#include "futils.hpp"

#include <iostream>

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        std::cout << "Print the CIFF structure of a CRW file\n";
        return 1;
    }

    Exiv2::FileIo io(argv[1]);
    if(io.open() != 0) {
        throw Exiv2::Error(9, io.path(), Exiv2::strError());
    }
    Exiv2::IoCloser closer(io);

    // Ensure that this is a CRW image
    if (!Exiv2::isCrwType(io, false)) {
        if (io.error() || io.eof()) throw Exiv2::Error(14);
        throw Exiv2::Error(33);
    }

    // Read the image into a memory buffer
    long len = io.size();
    Exiv2::DataBuf buf(len);
    io.read(buf.pData_, len);
    if (io.error() || io.eof()) throw Exiv2::Error(14);

    // Parse the image, starting with a CIFF header component
    Exiv2::Internal::CiffHeader::AutoPtr parseTree(new Exiv2::Internal::CiffHeader);
    parseTree->read(buf.pData_, buf.size_);
    parseTree->print(std::cout);

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cerr << e << "\n";
    return -1;
}
