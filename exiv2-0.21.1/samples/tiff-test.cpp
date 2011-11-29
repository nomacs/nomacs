// ***************************************************************** -*- C++ -*-
// tiff-test.cpp, $Rev: 2286 $
// First and very simple TIFF write test.

#include <exiv2/exiv2.hpp>

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace Exiv2;

void print(const ExifData& exifData);

void mini1(const char* path);
void mini9(const char* path);

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    const char* path = argv[1];
    mini1(path);
    mini9(path);

    return 0;
}
catch (const AnyError& e) {
    std::cout << e << "\n";
}

void mini1(const char* path)
{
    ExifData exifData;
    Blob blob;
    WriteMethod wm;

    // Write nothing to a new structure, without a previous binary image
    wm = ExifParser::encode(blob, 0, 0, bigEndian, exifData);
    assert(wm == wmIntrusive);
    assert(blob.size() == 0);
    std::cout << "Test 1: Writing empty Exif data without original binary data: ok.\n";

    // Write nothing, this time with a previous binary image
    DataBuf buf = readFile(path);
    wm = ExifParser::encode(blob, buf.pData_, buf.size_, bigEndian, exifData);
    assert(wm == wmIntrusive);
    assert(blob.size() == 0);
    std::cout << "Test 2: Writing empty Exif data with original binary data: ok.\n";

    // Write something to a new structure, without a previous binary image
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    wm = ExifParser::encode(blob, 0, 0, bigEndian, exifData);
    assert(wm == wmIntrusive);
    std::cout << "Test 3: Wrote non-empty Exif data without original binary data:\n";
    exifData.clear();
    ByteOrder bo = ExifParser::decode(exifData, &blob[0], blob.size());
    assert(bo == bigEndian);
    print(exifData);
}

void mini9(const char* path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    tiffImage.readMetadata();

    std::cout << "MIME type:  " << tiffImage.mimeType() << "\n";
    std::cout << "Image size: " << tiffImage.pixelWidth() << " x " << tiffImage.pixelHeight() << "\n";

    ExifData& exifData = tiffImage.exifData();
    std::cout << "Before\n";
    print(exifData);
    std::cout << "======\n";

    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";

    std::cout << "After\n";
    print(exifData);
    tiffImage.writeMetadata();
}

void print(const ExifData& exifData)
{
    if (exifData.empty()) {
        std::string error("No Exif data found in the file");
        throw Exiv2::Error(1, error);
    }
    Exiv2::ExifData::const_iterator end = exifData.end();
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
        std::cout << std::setw(44) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << i->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << i->count() << "  "
                  << std::dec << i->value()
                  << "\n";
    }
}
