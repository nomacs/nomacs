// ***************************************************************** -*- C++ -*-
// exifvalue.cpp, $Rev: 3090 $
// Sample program to print value of an exif key in an image

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>

int main(int argc, char* const argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " file key\n";
        return 1;
    }

    const char* file = argv[1];
    const char* key  = argv[2];

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    if ( exifData.empty()) {
		std::cerr << "no metadata found in file " << file << std::endl;
		exit(2);
	}

	try {
     	std::cout << exifData[key] << std::endl;
	} catch (Exiv2::AnyError& e) {
    	std::cerr << "Caught Exiv2 exception '" << e << "'" << std::endl;
    	exit(3);
	} catch ( ... ) {
		std::cerr << "Caught a cold!" << std::endl;
    	exit(4);
	}

    return 0;
}
