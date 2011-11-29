// ***************************************************************** -*- C++ -*-
// tiffmn-test.cpp, $Rev: 1512 $
// Makernote TIFF writer tests.

#include "tiffimage.hpp"
#include "exif.hpp"
#include "error.hpp"

#include <string>
#include <iostream>
#include <iomanip>

using namespace Exiv2;

void canonmn(const std::string& path);
void fujimn(const std::string& path);
void minoltamn(const std::string& path);
void nikon1mn(const std::string& path);
void nikon2mn(const std::string& path);
void nikon3mn(const std::string& path);
void olympusmn(const std::string& path);
void panasonicmn(const std::string& path);
void sigmamn(const std::string& path);
void sony1mn(const std::string& path);
void sony2mn(const std::string& path);

void print(const ExifData& exifData);

int main()
try {
    canonmn("exiv2-canonmn.tif");
    fujimn("exiv2-fujimn.tif");
    minoltamn("exiv2-minoltamn.tif");
    nikon1mn("exiv2-nikon1mn.tif");
    nikon2mn("exiv2-nikon2mn.tif");
    nikon3mn("exiv2-nikon3mn.tif");
    olympusmn("exiv2-olympusmn.tif");
    panasonicmn("exiv2-panasonicmn.tif");
    sigmamn("exiv2-sigmamn.tif");
    sony1mn("exiv2-sony1mn.tif");
    sony2mn("exiv2-sony2mn.tif");

    return 0;
}
catch (const Error& e) {
    std::cout << e << "\n";
}

void canonmn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "Canon";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Canon.OwnerName"] = "Camera Owner";

    print(exifData);
    tiffImage.writeMetadata();
}

void fujimn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "FUJIFILM";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Fujifilm.Quality"] = "Very good";

    print(exifData);
    tiffImage.writeMetadata();
}

void minoltamn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "Minolta";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Minolta.Quality"] = uint32_t(42);

    print(exifData);
    tiffImage.writeMetadata();
}

void nikon1mn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "NIKON";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Nikon1.Quality"] = "Excellent";

    print(exifData);
    tiffImage.writeMetadata();
}

void nikon2mn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "NIKON";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Nikon2.Quality"] = uint16_t(42);

    print(exifData);
    tiffImage.writeMetadata();
}

void nikon3mn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "NIKON";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Nikon3.Quality"] = "Nikon3 quality";

    print(exifData);
    tiffImage.writeMetadata();
}

void olympusmn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "OLYMPUS";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Olympus.Quality"] = uint16_t(42);

    print(exifData);
    tiffImage.writeMetadata();
}

void panasonicmn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "Panasonic";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Panasonic.Quality"] = uint16_t(42);

    print(exifData);
    tiffImage.writeMetadata();
}

void sigmamn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "SIGMA";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Sigma.Quality"] = "Sigma quality";

    print(exifData);
    tiffImage.writeMetadata();
}

void sony1mn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "SONY";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Sony.0x2000"] = uint16_t(42);

    print(exifData);
    tiffImage.writeMetadata();
}

void sony2mn(const std::string& path)
{
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(path)), false);
    ExifData& exifData = tiffImage.exifData();

    exifData["Exif.Image.Make"] = "SONY";
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    exifData["Exif.Sony.0x2001"] = uint16_t(43);

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
