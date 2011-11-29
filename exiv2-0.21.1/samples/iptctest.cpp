// ***************************************************************** -*- C++ -*-
/*
  Abstract : Sample program test the Iptc reading and writing.
             This is not designed to be a robust application.

  File     : iptctest.cpp
  Version  : $Rev: 2286 $
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History  : 01-Aug-04, brad: created
 */
// *****************************************************************************
// included header files
#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

using namespace Exiv2;

bool processLine(const std::string& line, int num, IptcData &iptcData);
void processAdd(const std::string& line, int num, IptcData &iptcData);
void processRemove(const std::string& line, int num, IptcData &iptcData);
void processModify(const std::string& line, int num, IptcData &iptcData);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
    try {

        if (argc != 2) {
            std::cout << "Usage: " << argv[0] << " image\n";
            std::cout << "Commands read from stdin.\n";
            return 1;
        }

        Image::AutoPtr image = ImageFactory::open(argv[1]);
        assert (image.get() != 0);
        image->readMetadata();

        // Process commands
        std::string line;
        int num = 0;
        std::getline(std::cin, line);
        while (line.length() && processLine(line, ++num, image->iptcData())) {
            std::getline(std::cin, line);
        }

        // Save any changes
        image->writeMetadata();

        return 0;
    }
    catch (AnyError& e) {
        std::cout << "Caught Exiv2 exception '" << e << "'\n";
        return -1;
    }
}

bool processLine(const std::string& line, int num, IptcData &iptcData)
{
    switch (line.at(0)) {
        case 'a':
        case 'A':
            processAdd(line, num, iptcData);
            break;
        case 'r':
        case 'R':
            processRemove(line, num, iptcData);
            break;
        case 'm':
        case 'M':
            processModify(line, num, iptcData);
            break;
        case 'q':
        case 'Q':
            return false;
        default:
            std::ostringstream os;
            os << "Unknown command (" << line.at(0) << ") at line " << num;
            throw Error(1, os.str());
    }
    return true;
}

void processAdd(const std::string& line, int num, IptcData &iptcData)
{
    std::string::size_type keyStart = line.find_first_not_of(" \t", 1);
    std::string::size_type keyEnd = line.find_first_of(" \t", keyStart+1);
    std::string::size_type dataStart = line.find_first_not_of(" \t", keyEnd+1);

    if (keyStart == std::string::npos ||
        keyEnd == std::string::npos ||
        dataStart == std::string::npos) {
        std::ostringstream os;
        os << "Invalid \'a\' command at line " << num;
        throw Error(1, os.str());
    }

    std::string key(line.substr(keyStart, keyEnd-keyStart));
    IptcKey iptcKey(key);

    std::string data(line.substr(dataStart));
    // if data starts and ends with quotes, remove them
    if (data.at(0) == '\"' && data.at(data.size()-1) == '\"') {
        data = data.substr(1, data.size()-2);
    }
    TypeId type = IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record());
    Value::AutoPtr value = Value::create(type);
    value->read(data);

    int rc = iptcData.add(iptcKey, value.get());
    if (rc) {
        throw Error(1, "Iptc dataset already exists and is not repeatable");
    }
}

void processRemove(const std::string& line, int num, IptcData &iptcData)
{
    std::string::size_type keyStart = line.find_first_not_of(" \t", 1);

    if (keyStart == std::string::npos) {
        std::ostringstream os;
        os << "Invalid \'r\' command at line " << num;
        throw Error(1, os.str());
    }

    const std::string key( line.substr(keyStart) );
    IptcKey iptcKey(key);

    IptcData::iterator iter = iptcData.findKey(iptcKey);
    if (iter != iptcData.end()) {
        iptcData.erase(iter);
    }
}

void processModify(const std::string& line, int num, IptcData &iptcData)
{
    std::string::size_type keyStart = line.find_first_not_of(" \t", 1);
    std::string::size_type keyEnd = line.find_first_of(" \t", keyStart+1);
    std::string::size_type dataStart = line.find_first_not_of(" \t", keyEnd+1);

    if (keyStart == std::string::npos ||
        keyEnd == std::string::npos ||
        dataStart == std::string::npos) {
        std::ostringstream os;
        os << "Invalid \'m\' command at line " << num;
        throw Error(1, os.str());
    }

    std::string key(line.substr(keyStart, keyEnd-keyStart));
    IptcKey iptcKey(key);

    std::string data(line.substr(dataStart));
    // if data starts and ends with quotes, remove them
    if (data.at(0) == '\"' && data.at(data.size()-1) == '\"') {
        data = data.substr(1, data.size()-2);
    }
    TypeId type = IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record());
    Value::AutoPtr value = Value::create(type);
    value->read(data);

    IptcData::iterator iter = iptcData.findKey(iptcKey);
    if (iter != iptcData.end()) {
        iter->setValue(value.get());
    }
    else {
        int rc = iptcData.add(iptcKey, value.get());
        if (rc) {
            throw Error(1, "Iptc dataset already exists and is not repeatable");
        }
    }
}
