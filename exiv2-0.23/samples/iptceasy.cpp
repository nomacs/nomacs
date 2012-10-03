// ***************************************************************** -*- C++ -*-
// iptceasy.cpp, $Rev: 2286 $
// The quickest way to access, set or modify IPTC metadata.

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string file(argv[1]);

    Exiv2::IptcData iptcData;

    iptcData["Iptc.Application2.Headline"] = "The headline I am";
    iptcData["Iptc.Application2.Keywords"] = "Yet another keyword";
    iptcData["Iptc.Application2.DateCreated"] = "2004-8-3";
    iptcData["Iptc.Application2.Urgency"] = uint16_t(1);
    iptcData["Iptc.Envelope.ModelVersion"] = 42;
    iptcData["Iptc.Envelope.TimeSent"] = "14:41:0-05:00";
    iptcData["Iptc.Application2.RasterizedCaption"] = "230 42 34 2 90 84 23 146";
    iptcData["Iptc.0x0009.0x0001"] = "Who am I?";

    Exiv2::StringValue value;
    value.read("very!");
    iptcData["Iptc.Application2.Urgency"] = value;

    std::cout << "Time sent: " << iptcData["Iptc.Envelope.TimeSent"] << "\n";

    // Open image file
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert (image.get() != 0);

    // Set IPTC data and write it to the file
    image->setIptcData(iptcData);
    image->writeMetadata();

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
