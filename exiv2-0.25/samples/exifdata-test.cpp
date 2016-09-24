// ***************************************************************** -*- C++ -*-
/*
  Abstract : ExifData assignment and copy construction unit tests

  File     : exifdata-test.cpp
  Version  : $Rev: 2286 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 20-Feb-05, ahu: created

 */
// *****************************************************************************
// included header files
#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>

void write(const std::string& file, Exiv2::ExifData& ed);
void print(const std::string& file);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string file(argv[1]);

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert (image.get() != 0);
    image->readMetadata();

    Exiv2::ExifData &ed = image->exifData();
    if (ed.empty()) {
        std::string error = file + ": No Exif data found in the file";
        throw Exiv2::Error(1, error);
    }

    std::cout << "Copy construction, non-intrusive changes\n";
    Exiv2::ExifData ed1(ed);
    ed1["Exif.Image.DateTime"] = "Sunday, 11am";
    ed1["Exif.Image.Orientation"] = uint16_t(2);
    ed1["Exif.Photo.DateTimeOriginal"] = "Sunday, 11am";
    ed1["Exif.Photo.MeteringMode"] = uint16_t(1);
    ed1["Exif.Iop.InteroperabilityIndex"] = "123";
//    ed1["Exif.Thumbnail.Orientation"] = uint16_t(2);
    write(file, ed1);
    print(file);
    std::cout << "----------------------------------------------\n";

    std::cout << "Copy construction, intrusive changes\n";
    Exiv2::ExifData ed2(ed);
    ed2["Exif.Image.DateTime"] = "Sunday, 11am and ten minutes";
    ed2["Exif.Image.Orientation"] = "2 3 4 5";
    ed2["Exif.Photo.DateTimeOriginal"] = "Sunday, 11am and ten minutes";
    ed2["Exif.Photo.MeteringMode"] = "1 2 3 4 5 6";
    ed2["Exif.Iop.InteroperabilityIndex"] = "1234";
    ed2["Exif.Thumbnail.Orientation"] = "2 3 4 5 6";
    write(file, ed2);
    print(file);
    std::cout << "----------------------------------------------\n";

    std::cout << "Assignment, non-intrusive changes\n";
    Exiv2::ExifData ed3;
    ed3["Exif.Iop.InteroperabilityVersion"] = "Test 6 Iop tag";
    ed3["Exif.Thumbnail.Artist"] = "Test 6 Ifd1 tag";
    ed3 = ed;
    ed3["Exif.Image.DateTime"] = "Sunday, 11am";
    ed3["Exif.Image.Orientation"] = uint16_t(2);
    ed3["Exif.Photo.DateTimeOriginal"] = "Sunday, 11am";
    ed3["Exif.Photo.MeteringMode"] = uint16_t(1);
    ed3["Exif.Iop.InteroperabilityIndex"] = "123";
    ed3["Exif.Thumbnail.Orientation"] = uint16_t(2);
    write(file, ed3);
    print(file);
    std::cout << "----------------------------------------------\n";

    std::cout << "Assignment, intrusive changes\n";
    Exiv2::ExifData ed4;
    ed4["Exif.Iop.InteroperabilityVersion"] = "Test 6 Iop tag";
    ed4["Exif.Thumbnail.Artist"] = "Test 6 Ifd1 tag";
    ed4 = ed;
    ed4["Exif.Image.DateTime"] = "Sunday, 11am and ten minutes";
    ed4["Exif.Image.Orientation"] = "2 3 4 5";
    ed4["Exif.Photo.DateTimeOriginal"] = "Sunday, 11am and ten minutes";
    ed4["Exif.Photo.MeteringMode"] = uint16_t(1);
    ed4["Exif.Iop.InteroperabilityIndex"] = "123";
    ed4["Exif.Thumbnail.Orientation"] = uint16_t(2);
    write(file, ed4);
    print(file);

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
}

void write(const std::string& file, Exiv2::ExifData& ed)
{
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert (image.get() != 0);
    image->setExifData(ed);
    image->writeMetadata();
}

void print(const std::string& file)
{
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert (image.get() != 0);
    image->readMetadata();

    Exiv2::ExifData &ed = image->exifData();
    Exiv2::ExifData::const_iterator end = ed.end();
    for (Exiv2::ExifData::const_iterator i = ed.begin(); i != end; ++i) {
        std::cout << std::setw(45) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << " "
                  << std::setw(12) << std::setfill(' ') << std::left
                  << i->ifdName() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << i->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << i->count() << " "
                  << std::dec << i->value()
                  << "\n";
    }
}
