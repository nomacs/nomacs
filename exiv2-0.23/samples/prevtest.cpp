// ***************************************************************** -*- C++ -*-
// prevtest.cpp, $Rev: 2286 $
// Test access to preview images

#include <exiv2/exiv2.hpp>

#include <string>
#include <iostream>
#include <cassert>

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string filename(argv[1]);

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::PreviewManager loader(*image);
    Exiv2::PreviewPropertiesList list = loader.getPreviewProperties();
    for (Exiv2::PreviewPropertiesList::iterator pos = list.begin(); pos != list.end(); pos++) {
        std::cout << pos->mimeType_
                  << " preview, type " << pos->id_ << ", "
                  << pos->size_ << " bytes, "
                  << pos->width_ << 'x' << pos->height_ << " pixels"
                  << "\n";

        Exiv2::PreviewImage preview = loader.getPreviewImage(*pos);
        preview.writeFile(filename + "_"
                          + Exiv2::toString(pos->width_) + "x"
                          + Exiv2::toString(pos->height_));
    }

    // Cleanup
    Exiv2::XmpParser::terminate();
    
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
