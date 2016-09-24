// ***************************************************************** -*- C++ -*-
// mrwthumb.cpp, $Rev: 1532 $
// Sample program to extract a Minolta thumbnail from the makernote

#include "image.hpp"
#include "exif.hpp"
#include "error.hpp"
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

    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error(argv[1]);
        error += ": No Exif data found in the file";
        throw Exiv2::Error(1, error);
    }

    Exiv2::ExifKey key("Exif.Minolta.ThumbnailOffset");
    Exiv2::ExifData::const_iterator format = exifData.findKey(key);

    if (format != exifData.end()) {
        Exiv2::DataBuf buf = format->dataArea();

        // The first byte of the buffer needs to be patched
        buf.pData_[0] = 0xff;

        Exiv2::FileIo file("img_thumb.jpg");

        file.open("wb");
        file.write(buf.pData_, buf.size_);
        file.close();
    }

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
