// ***************************************************************** -*- C++ -*-
/*
  Abstract : ExifData write unit tests

  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  Version  : $Rev: 2286 $

  Test procedure:
   $ rm -f test.jpg thumb.jpg iii ttt;
   $ ./exifprint ../test/img_1771.jpg > iii;
   $ cp ../test/img_1771.jpg ./test.jpg;
   $ ./makernote-test2 ../test/img_1771.jpg > ttt;
   $ diff iii ttt

 */
// *****************************************************************************
// included header files
#include <exiv2/exiv2.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <utility>
#include <cassert>

// *****************************************************************************
// local declarations

using namespace Exiv2;

void testCase(const std::string& file1,
              const std::string& file2,
              const std::string& thumb,
              const std::string& key,
              const std::string& value);
void exifPrint(const ExifData& exifData);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
try {

    if (argc != 3) {
        std::cout << "Usage: write-test file case\n\n"
                  << "where case is an integer between 1 and 11\n";
        return 1;
    }

    std::string testFile = argv[1];
    std::istringstream iss(argv[2]);
    int testNo;
    iss >> testNo;

    int rc = 0;
    switch (testNo) {
    case 1:
        std::cerr << "Case 1: ";
        std::cerr << "Non-intrusive change to the standard Exif metadata\n";
        testCase(testFile, "test1.jpg", "thumb1",
                 "Exif.Photo.DateTimeOriginal",
                 "1999:11:22 00:11:22");
        break;
    case 2:
        std::cerr << "Case 2: ";
        std::cerr << "Non-intrusive change to the makernote metadata\n";
        testCase(testFile, "test2.jpg", "thumb2",
                 "Exif.Canon.OwnerName",
                 "Chan YeeSend");
        break;
    case 3:
        std::cerr << "Case 3: ";
        std::cerr << "Non-intrusive change to the Exif metadata (w/o makernote)\n";
        testCase(testFile, "test3.jpg", "thumb3",
                 "Exif.Photo.DateTimeOriginal",
                 "1999:11:22 00:11:22");
        break;
    case 4:
        std::cerr << "Case 4: ";
        std::cerr << "Intrusive change to the standard Exif metadata\n";
        testCase(testFile, "test4.jpg", "thumb4",
                 "Exif.Photo.DateTimeOriginal",
                 "1999:11:22 00:11:22 and twenty seconds");
        break;
    case 5:
        std::cerr << "Case 5: ";
        std::cerr << "Intrusive change to the Canon makernote metadata\n";
        testCase(testFile, "test5.jpg", "thumb5",
                 "Exif.Canon.OwnerName",
                 "Frau Chan YeeSend und Herr Andreas Huggel");
        break;
    case 6:
        std::cerr << "Case 6: ";
        std::cerr << "Intrusive change to the Exif metadata (w/o makernote)\n";
        testCase(testFile, "test6.jpg", "thumb6",
                 "Exif.Photo.DateTimeOriginal",
                 "1999:11:22 00:11:22 and twenty seconds");
        break;
    case 7:
        std::cerr << "Case 7: ";
        std::cerr << "Intrusive change to the Fujifilm makernote metadata\n";
        testCase(testFile, "test7.jpg", "thumb7",
                 "Exif.Fujifilm.Quality",
                 "Typical Fujifilm Quality");
        break;
    case 8:
        std::cerr << "Case 8: ";
        std::cerr << "Intrusive change to the Sigma makernote metadata\n";
        testCase(testFile, "test8.jpg", "thumb8",
                 "Exif.Sigma.ResolutionMode",
                 "Sigma HI resolution");
        break;
    case 9:
        std::cerr << "Case 9: ";
        std::cerr << "Intrusive change to the Nikon1 makernote metadata\n";
        testCase(testFile, "test9.jpg", "thumb9",
                 "Exif.Nikon1.Quality",
                 "Typical Nikon1 Quality");
        break;
    case 10:
        std::cerr << "Case 10: ";
        std::cerr << "Intrusive change to the Nikon2 makernote metadata\n";
        testCase(testFile, "test10.jpg", "thumb10",
                 "Exif.Nikon2.0x0002",
                 "Nikon2 Version 2");
        break;
    case 11:
        std::cerr << "Case 11: ";
        std::cerr << "Intrusive change to the Nikon3 makernote metadata\n";
        testCase(testFile, "test11.jpg", "thumb11",
                 "Exif.Nikon3.Quality",
                 "Typical Nikon3 Quality");
        break;

        // ToDo: Erase Sigma thumbnail

        // ToDo: Write to a broken (truncated) IFD entry

    default:
        std::cout << "Usage: exiftest file case\n\n"
                  << "where case is an integer between 1 and 11\n";
        rc = 1;
        break;
    }

    return rc;
}
catch (AnyError& e) {
    std::cerr << "Caught Exiv2 exception '" << e << "'\n";
    return 1;
}
}

// *****************************************************************************
void testCase(const std::string& file1,
              const std::string& file2,
              const std::string& thumb,
              const std::string& key,
              const std::string& value)
{
    ExifKey ek(key);

    //Open first image
    Image::AutoPtr image1 = ImageFactory::open(file1);
    assert(image1.get() != 0);

    // Load existing metadata
    std::cerr << "---> Reading file " << file1 << "\n";
    image1->readMetadata();

    Exiv2::ExifData &ed1 = image1->exifData();
    std::cerr << "---> Modifying Exif data\n";
    Exiv2::ExifData::iterator pos = ed1.findKey(ek);
    if (pos == ed1.end()) {
        throw Error(1, "Metadatum with key = " + ek.key() + " not found");
    }
    pos->setValue(value);

    // Open second image
    Image::AutoPtr image2 = ImageFactory::open(file2);
    assert(image2.get() != 0);

    image2->setExifData(image1->exifData());

    std::cerr << "---> Writing Exif data to file " << file2 << "\n";
    image2->writeMetadata();

    std::cerr << "---> Reading file " << file2 << "\n";
    image2->readMetadata();

    Exiv2::ExifData &ed2 = image2->exifData();
    exifPrint(ed2);

    std::cerr << "---> Writing Exif thumbnail to file " << thumb << ".*\n";
    ExifThumbC et2(ed2);
    et2.writeFile(thumb);
}

// *****************************************************************************

void exifPrint(const ExifData& exifData)
{
    ExifData::const_iterator i = exifData.begin();
    for (; i != exifData.end(); ++i) {
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
