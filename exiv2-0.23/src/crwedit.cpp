// ***************************************************************** -*- C++ -*-
// crwedit.cpp, $Rev: 1560 $
// Print the CIFF structure of a CRW file

#include "crwimage.hpp"
#include "crwimage_int.hpp"
#include "futils.hpp"

#include <iostream>
#include <string>
#include <cstring>

void remove(Exiv2::Internal::CiffHeader* pHead);
void add(Exiv2::Internal::CiffHeader* pHead);
void help();
void write(const std::string& filename, const Exiv2::Internal::CiffHeader* pHead);

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        std::cout << "Edit the CIFF structure of a CRW file\n";
        return 1;
    }

    std::string filename(argv[1]);
    Exiv2::FileIo io(filename);
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

    // Allow user to make changes
    bool go = true;
    while (go) {
        char cmd;
        std::cout << "command> ";
        std::cin >> cmd;
        switch (cmd) {
        case 'q': go = false; break;
        case 'p': parseTree->print(std::cout); break;
        case 'a': add(parseTree.get()); break;
        case 'd': remove(parseTree.get()); break;
        case 'w': write(filename, parseTree.get()); break;
        case 'h': help(); break;
        }
    }

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cerr << e << "\n";
    return -1;
}

void write(const std::string& filename, const Exiv2::Internal::CiffHeader* pHead)
{
    Exiv2::Blob blob;
    pHead->write(blob);

    Exiv2::FileIo io(filename);
    if(io.open("wb") != 0) {
        throw Exiv2::Error(9, io.path(), Exiv2::strError());
    }
    Exiv2::IoCloser closer(io);
    long ret = io.write(&blob[0], blob.size());
    if (static_cast<size_t>(ret) != blob.size()) throw Exiv2::Error(21);
    io.close();
}

void remove(Exiv2::Internal::CiffHeader* pHead)
{
    uint16_t crwTag, crwDir;
    std::cout << "crwTag> 0x";
    std::cin >> std::hex >> crwTag;
    std::cout << "crwDir> 0x";
    std::cin >> std::hex >> crwDir;
    std::cout << "Deleting tag 0x" << std::hex << crwTag
              << " in dir 0x" << crwDir << ", ok? ";
    char cmd;
    std::cin >> cmd;
    if (cmd != 'n' && cmd != 'N') {
        pHead->remove(crwTag, crwDir);
    }
    else {
        std::cout << "Canceled.\n";
    }
}

void add(Exiv2::Internal::CiffHeader* pHead)
{
    uint16_t crwTag, crwDir;
    uint32_t size;
    std::cout << "crwTag> 0x";
    std::cin >> std::hex >> crwTag;
    std::cout << "crwDir> 0x";
    std::cin >> std::hex >> crwDir;
    std::cout << "size> ";
    std::cin >> std::dec >> size;
    std::cout << "Adding tag 0x" << std::hex << crwTag
              << " in dir 0x" << crwDir << ", " << size << " bytes, ok? ";
    char cmd;
    std::cin >> cmd;
    if (cmd != 'n' && cmd != 'N') {
        Exiv2::DataBuf buf(size);
        std::memset(buf.pData_, 0xaa, size);
        pHead->add(crwTag, crwDir, buf);
    }
    else {
        std::cout << "Canceled.\n";
    }
}

void help()
{
    std::cout << "a: add tag, d: delete tag, p: print tags, w: write file, q: quit\n";
}
