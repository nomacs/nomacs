// ***************************************************************** -*- C++ -*-
// xmpsample.cpp, $Rev: 2639 $
// Sample/test for high level XMP classes. See also addmoddel.cpp

#include <exiv2/exiv2.hpp>

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>

bool isEqual(float a, float b)
{
    double d = std::fabs(a - b);
    return d < 0.00001;
}

int main()
try {
    // The XMP property container
    Exiv2::XmpData xmpData;

    // -------------------------------------------------------------------------
    // Teaser: Setting XMP properties doesn't get much easier than this:

    xmpData["Xmp.dc.source"]  = "xmpsample.cpp";    // a simple text value
    xmpData["Xmp.dc.subject"] = "Palmtree";         // an array item
    xmpData["Xmp.dc.subject"] = "Rubbertree";       // add a 2nd array item
    // a language alternative with two entries and without default
    xmpData["Xmp.dc.title"]   = "lang=de-DE Sonnenuntergang am Strand";
    xmpData["Xmp.dc.title"]   = "lang=en-US Sunset on the beach";

    // -------------------------------------------------------------------------
    // Any properties can be set provided the namespace is known. Values of any
    // type can be assigned to an Xmpdatum, if they have an output operator. The
    // default XMP value type for unknown properties is a simple text value.

    xmpData["Xmp.dc.one"]     = -1;
    xmpData["Xmp.dc.two"]     = 3.1415;
    xmpData["Xmp.dc.three"]   = Exiv2::Rational(5, 7);
    xmpData["Xmp.dc.four"]    = uint16_t(255);
    xmpData["Xmp.dc.five"]    = int32_t(256);
    xmpData["Xmp.dc.six"]     = false;

    // In addition, there is a dedicated assignment operator for Exiv2::Value
    Exiv2::XmpTextValue val("Seven");
    xmpData["Xmp.dc.seven"]   = val;
    xmpData["Xmp.dc.eight"]   = true;

    // Extracting values
    assert(xmpData["Xmp.dc.one"].toLong() == -1);
    assert(xmpData["Xmp.dc.one"].value().ok());

    const Exiv2::Value &getv1 = xmpData["Xmp.dc.one"].value();
    assert(isEqual(getv1.toFloat(), -1)); 
    assert(getv1.ok());
    assert(getv1.toRational() == Exiv2::Rational(-1, 1));
    assert(getv1.ok());

    const Exiv2::Value &getv2 = xmpData["Xmp.dc.two"].value();
    assert(isEqual(getv2.toFloat(), 3.1415f)); 
    assert(getv2.ok());
    assert(getv2.toLong() == 3);
    assert(getv2.ok());
    Exiv2::Rational R = getv2.toRational();
    assert(getv2.ok());
    assert(isEqual(static_cast<float>(R.first) / R.second, 3.1415f ));

    const Exiv2::Value &getv3 = xmpData["Xmp.dc.three"].value();
    assert(isEqual(getv3.toFloat(), 5.0f/7.0f)); 
    assert(getv3.ok());
    assert(getv3.toLong() == 0);  // long(5.0 / 7.0) 
    assert(getv3.ok());
    assert(getv3.toRational() == Exiv2::Rational(5, 7));
    assert(getv3.ok());
    
    const Exiv2::Value &getv6 = xmpData["Xmp.dc.six"].value();
    assert(getv6.toLong() == 0);
    assert(getv6.ok());
    assert(getv6.toFloat() == 0.0);
    assert(getv6.ok());
    assert(getv6.toRational() == Exiv2::Rational(0, 1));
    assert(getv6.ok());
    
    const Exiv2::Value &getv7 = xmpData["Xmp.dc.seven"].value();
    getv7.toLong(); // this should fail
    assert(!getv7.ok()); 

    const Exiv2::Value &getv8 = xmpData["Xmp.dc.eight"].value();
    assert(getv8.toLong() == 1);
    assert(getv8.ok());
    assert(getv8.toFloat() == 1.0);
    assert(getv8.ok());
    assert(getv8.toRational() == Exiv2::Rational(1, 1));
    assert(getv8.ok());

    // Deleting an XMP property
    Exiv2::XmpData::iterator pos = xmpData.findKey(Exiv2::XmpKey("Xmp.dc.eight"));
    if (pos == xmpData.end()) throw Exiv2::Error(1, "Key not found");
    xmpData.erase(pos);

    // -------------------------------------------------------------------------
    // Exiv2 has specialized values for simple XMP properties, arrays of simple
    // properties and language alternatives.

    // Add a simple XMP property in a known namespace    
    Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpText);
    v->read("image/jpeg");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.format"), v.get());

    // Add an ordered array of text values.
    v = Exiv2::Value::create(Exiv2::xmpSeq); // or xmpBag or xmpAlt.
    v->read("1) The first creator");         // The sequence in which the array
    v->read("2) The second creator");        // elements are added is their
    v->read("3) And another one");           // order in the array.
    xmpData.add(Exiv2::XmpKey("Xmp.dc.creator"), v.get());

    // Add a language alternative property
    v = Exiv2::Value::create(Exiv2::langAlt);
    v->read("lang=de-DE Hallo, Welt");       // The default doesn't need a 
    v->read("Hello, World");                 // qualifier
    xmpData.add(Exiv2::XmpKey("Xmp.dc.description"), v.get());

    // According to the XMP specification, Xmp.tiff.ImageDescription is an
    // alias for Xmp.dc.description. Exiv2 treats an alias just like any
    // other property and leaves it to the application to implement specific
    // behaviour if desired.
    xmpData["Xmp.tiff.ImageDescription"] = "TIFF image description";
    xmpData["Xmp.tiff.ImageDescription"] = "lang=de-DE TIFF Bildbeschreibung";

    // -------------------------------------------------------------------------
    // Register a namespace which Exiv2 doesn't know yet. This is only needed
    // when properties are added manually. If the XMP metadata is read from an
    // image, namespaces are decoded and registered at the same time.
    Exiv2::XmpProperties::registerNs("myNamespace/", "ns");

    // -------------------------------------------------------------------------
    // Add a property in the new custom namespace.
    xmpData["Xmp.ns.myProperty"] = "myValue";

    // -------------------------------------------------------------------------
    // There are no specialized values for structures, qualifiers and nested
    // types. However, these can be added by using an XmpTextValue and a path as
    // the key.

    // Add a structure
    Exiv2::XmpTextValue tv("16");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpDM.videoFrameSize/stDim:w"), &tv);
    tv.read("9");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpDM.videoFrameSize/stDim:h"), &tv);
    tv.read("inch");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpDM.videoFrameSize/stDim:unit"), &tv);

    // Add an element with a qualifier (using the namespace registered above)
    xmpData["Xmp.dc.publisher"] = "James Bond";  // creates an unordered array
    xmpData["Xmp.dc.publisher[1]/?ns:role"] = "secret agent";

    // Add a qualifer to an array element of Xmp.dc.creator (added above)
    tv.read("programmer");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.creator[2]/?ns:role"), &tv);

    // Add an array of structures
    tv.read("");                                         // Clear the value
    tv.setXmpArrayType(Exiv2::XmpValue::xaBag);
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef"), &tv); // Set the array type.

    tv.setXmpArrayType(Exiv2::XmpValue::xaNone);
    tv.read("Birthday party");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[1]/stJob:name"), &tv);
    tv.read("Photographer");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[1]/stJob:role"), &tv);

    tv.read("Wedding ceremony");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[2]/stJob:name"), &tv);
    tv.read("Best man");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[2]/stJob:role"), &tv);

    // Add a creator contact info structure
    xmpData["Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity"] = "Kuala Lumpur";
    xmpData["Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry"] = "Malaysia";
    xmpData["Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork"] = "http://www.exiv2.org";

    // -------------------------------------------------------------------------
    // Output XMP properties
    for (Exiv2::XmpData::const_iterator md = xmpData.begin(); 
         md != xmpData.end(); ++md) {
        std::cout << std::setfill(' ') << std::left
                  << std::setw(44)
                  << md->key() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << md->count() << "  "
                  << std::dec << md->value()
                  << std::endl;
    }

    // -------------------------------------------------------------------------
    // Serialize the XMP data and output the XMP packet
    std::string xmpPacket;
    if (0 != Exiv2::XmpParser::encode(xmpPacket, xmpData)) {
        throw Exiv2::Error(1, "Failed to serialize XMP data");
    }
    std::cout << xmpPacket << "\n";

    // Cleanup
    Exiv2::XmpParser::terminate();

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
