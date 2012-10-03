// ***************************************************************** -*- C++ -*-
// mmap-test.cpp, $Rev: 2286 $
// Simple mmap tests

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <cstring>

using namespace Exiv2;

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    const char* path = argv[1];

    FileIo file(path);
    // Open the file in read mode
    if (file.open("rb") != 0) {
        throw Error(10, path, "rb", strError());
    }
    // Map it to memory
    const Exiv2::byte* pData = file.mmap();
    long size = file.size();
    DataBuf buf(size);
    // Read from the memory mapped region
    memcpy(buf.pData_, pData, buf.size_);
    // Reopen file in write mode and write to it
    file.write(buf.pData_, buf.size_);
    // Read from the mapped region again
    memcpy(buf.pData_, pData, buf.size_);
    file.close();

    return 0;
}
catch (const AnyError& e) {
    std::cout << e << "\n";
}
