// ***************************************************************** -*- C++ -*-
/*
  Abstract : ExifData write unit tests for Exif data created from scratch

  File     : write2-test.cpp
  Version  : $Rev: 2286 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 26-Jun-04, ahu: created

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

    std::cout <<"----- Some IFD0 tags\n";
    Exiv2::ExifData ed1;
    ed1["Exif.Image.Model"] = "Test 1";

    Exiv2::Value::AutoPtr v1 = Exiv2::Value::create(Exiv2::unsignedShort);
    v1->read("160 161 162 163");
    ed1.add(Exiv2::ExifKey("Exif.Image.SamplesPerPixel"), v1.get());

    Exiv2::Value::AutoPtr v2 = Exiv2::Value::create(Exiv2::signedLong);
    v2->read("-2 -1 0 1");
    ed1.add(Exiv2::ExifKey("Exif.Image.XResolution"), v2.get());

    Exiv2::Value::AutoPtr v3 = Exiv2::Value::create(Exiv2::signedRational);
    v3->read("-2/3 -1/3 0/3 1/3");
    ed1.add(Exiv2::ExifKey("Exif.Image.YResolution"), v3.get());

    Exiv2::Value::AutoPtr v4 = Exiv2::Value::create(Exiv2::undefined);
    v4->read("255 254 253 252");
    ed1.add(Exiv2::ExifKey("Exif.Image.WhitePoint"), v4.get());

    write(file, ed1);
    print(file);

    std::cout <<"\n----- One Exif tag\n";
    Exiv2::ExifData ed2;
    ed2["Exif.Photo.DateTimeOriginal"] = "Test 2";
    write(file, ed2);
    print(file);

    std::cout <<"\n----- Canon MakerNote tags\n";
    Exiv2::ExifData edMn1;
    edMn1["Exif.Image.Make"]   = "Canon";
    edMn1["Exif.Image.Model"]  = "Canon PowerShot S40";
    edMn1["Exif.Canon.0xabcd"] = "A Canon makernote tag";
    edMn1["Exif.CanonCs.0x0002"] = uint16_t(41);
    edMn1["Exif.CanonSi.0x0005"] = uint16_t(42);
    edMn1["Exif.CanonCf.0x0001"] = uint16_t(43);
    edMn1["Exif.CanonPi.0x0001"] = uint16_t(44);
    edMn1["Exif.CanonPa.0x0001"] = uint16_t(45);
    write(file, edMn1);
    print(file);

    std::cout <<"\n----- Non-intrusive writing of special Canon MakerNote tags\n";
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::ExifData& rEd = image->exifData();
    rEd["Exif.CanonCs.0x0001"] = uint16_t(88);
    rEd["Exif.CanonSi.0x0004"] = uint16_t(99);
    image->writeMetadata();
    print(file);

    std::cout <<"\n----- One Fujifilm MakerNote tag\n";
    Exiv2::ExifData edMn2;
    edMn2["Exif.Image.Make"]      = "FUJIFILM";
    edMn2["Exif.Image.Model"]     = "FinePixS2Pro";
    edMn2["Exif.Fujifilm.0x1000"] = "A Fujifilm QUALITY tag";
    write(file, edMn2);
    print(file);

    std::cout <<"\n----- One Sigma/Foveon MakerNote tag\n";
    Exiv2::ExifData edMn3;
    edMn3["Exif.Image.Make"]   = "SIGMA";
    edMn3["Exif.Image.Model"]  = "SIGMA SD10";
    edMn3["Exif.Sigma.0x0018"] = "Software? Exiv2!";
    write(file, edMn3);
    print(file);

    std::cout <<"\n----- One Nikon1 MakerNote tag\n";
    Exiv2::ExifData edMn4;
    edMn4["Exif.Image.Make"]    = "NIKON";
    edMn4["Exif.Image.Model"]   = "E990";
    edMn4["Exif.Nikon1.0x0080"] = "ImageAdjustment by Exiv2";
    write(file, edMn4);
    print(file);

    std::cout <<"\n----- One Nikon2 MakerNote tag\n";
    Exiv2::ExifData edMn5;
    edMn5["Exif.Image.Make"]    = "NIKON";
    edMn5["Exif.Image.Model"]   = "E950";
    edMn5["Exif.Nikon2.0xffff"] = "An obscure Nikon2 tag";
    write(file, edMn5);
    print(file);

    std::cout <<"\n----- One Nikon3 MakerNote tag\n";
    Exiv2::ExifData edMn6;
    edMn6["Exif.Image.Make"]    = "NIKON CORPORATION";
    edMn6["Exif.Image.Model"]   = "NIKON D70";
    edMn6["Exif.Nikon3.0x0004"] = "A boring Nikon3 Quality tag";
    write(file, edMn6);
    print(file);

    std::cout <<"\n----- One Olympus MakerNote tag\n";
    Exiv2::ExifData edMn7;
    edMn7["Exif.Image.Make"]      = "OLYMPUS CORPORATION";
    edMn7["Exif.Image.Model"]     = "C8080WZ";
    edMn7["Exif.Olympus.0x0201"] = uint16_t(1);
    write(file, edMn7);
    print(file);

    std::cout <<"\n----- One Panasonic MakerNote tag\n";
    Exiv2::ExifData edMn8;
    edMn8["Exif.Image.Make"]      = "Panasonic";
    edMn8["Exif.Image.Model"]     = "DMC-FZ5";
    edMn8["Exif.Panasonic.0x0001"] = uint16_t(1);
    write(file, edMn8);
    print(file);

    std::cout <<"\n----- One Sony1 MakerNote tag\n";
    Exiv2::ExifData edMn9;
    edMn9["Exif.Image.Make"]      = "SONY";
    edMn9["Exif.Image.Model"]     = "DSC-W7";
    edMn9["Exif.Sony1.0x2000"] = "0 1 2 3 4 5";
    write(file, edMn9);
    print(file);

    std::cout <<"\n----- Minolta MakerNote tags\n";
    Exiv2::ExifData edMn10;
    edMn10["Exif.Image.Make"]   = "Minolta";
    edMn10["Exif.Image.Model"]  = "A fancy Minolta camera";
    edMn10["Exif.Minolta.ColorMode"] = uint32_t(1);
    edMn10["Exif.MinoltaCsNew.WhiteBalance"] = uint32_t(2);
    edMn10["Exif.MinoltaCs5D.WhiteBalance"] = uint16_t(3);
    edMn10["Exif.MinoltaCs5D.ColorTemperature"] = int16_t(-1);
    edMn10["Exif.MinoltaCs7D.WhiteBalance"] = uint16_t(4);
    edMn10["Exif.MinoltaCs7D.ExposureCompensation"] = int16_t(-2);
    edMn10["Exif.MinoltaCs7D.ColorTemperature"] = int16_t(-3);
    write(file, edMn10);
    print(file);

    std::cout <<"\n----- One IOP tag\n";
    Exiv2::ExifData ed3;
    ed3["Exif.Iop.InteroperabilityIndex"] = "Test 3";
    write(file, ed3);
    print(file);

    std::cout <<"\n----- One GPS tag\n";
    Exiv2::ExifData ed4;
    ed4["Exif.GPSInfo.GPSVersionID"] = "19 20";
    write(file, ed4);
    print(file);

    std::cout <<"\n----- One IFD1 tag\n";
    Exiv2::ExifData ed5;
    ed5["Exif.Thumbnail.Artist"] = "Test 5";
    write(file, ed5);
    print(file);

    std::cout <<"\n----- One IOP and one IFD1 tag\n";
    Exiv2::ExifData ed6;
    ed6["Exif.Iop.InteroperabilityIndex"] = "Test 6 Iop tag";
    ed6["Exif.Thumbnail.Artist"] = "Test 6 Ifd1 tag";
    write(file, ed6);
    print(file);

    std::cout <<"\n----- One IFD0 and one IFD1 tag\n";
    Exiv2::ExifData ed7;
    ed7["Exif.Thumbnail.Artist"] = "Test 7";
    Exiv2::Value::AutoPtr v5 = Exiv2::Value::create(Exiv2::unsignedShort);
    v5->read("160 161 162 163");
    ed7.add(Exiv2::ExifKey("Exif.Image.SamplesPerPixel"), v5.get());
    write(file, ed7);
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
    assert(image.get() != 0);

    image->setExifData(ed);
    image->writeMetadata();
}

void print(const std::string& file)
{
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert(image.get() != 0);
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
