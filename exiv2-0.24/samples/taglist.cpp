// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Print a simple comma separated list of tags defined in Exiv2

  File:      taglist.cpp
  Version:   $Rev: 3189 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Jan-04, ahu: created
 */
// *****************************************************************************

#include <exiv2/exiv2.hpp>

#include <string>
#include <iostream>

using namespace Exiv2;

int main(int argc, char* argv[])
try {
    int rc = 0;

    switch (argc) {
    case 2:
    {
        std::string item(argv[1]);

        if (item == "Exif") {
            ExifTags::taglist(std::cout);
            break;
        }

        if (item == "Iptc") {
            IptcDataSets::dataSetList(std::cout);
            break;
        }

        if (ExifTags::isExifGroup(item) || ExifTags::isMakerGroup(item)) {
            ExifTags::taglist(std::cout, item);
            break;
        }

        try {
            XmpProperties::printProperties(std::cout, item);
        }
        catch(const AnyError&) {
            rc = 2;
        }

        break;
    }
    case 1:
        ExifTags::taglist(std::cout);
        break;
    default:
        rc = 1;
        break;
    }
    if (rc) {
        std::cout << "Usage: " << argv[0]
                  << " [Exif|Canon|CanonCs|CanonSi|CanonCf|Fujifilm|Minolta|Nikon1|Nikon2|Nikon3|Olympus|Panasonic|Pentax|Sigma|Sony|Iptc"
                  <<  "|dc|xmp|xmpRights|xmpMM|xmpBJ|xmpTPg|xmpDM|pdf|photoshop|crs|tiff|exif|aux|iptc]\n"
                  << "Print Exif tags, MakerNote tags, or Iptc datasets\n";
    }
    return rc;
}
catch (AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return 1;
}
