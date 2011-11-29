// ***************************************************************** -*- C++ -*-
// tiffaddpath-test.cpp, $Rev: 1512 $
// Test driver to test adding new tags to a TIFF composite structure

#include "tiffcomposite_int.hpp"
#include "makernote2_int.hpp"
#include "tiffimage_int.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstdlib>

using namespace Exiv2;

void addPath(TiffComponent* pRootDir,
             uint16_t       tag,
             TiffPath&      tiffPath);

void printPath(TiffPath tiffPath,
               uint32_t tag,
               uint16_t grp);

struct TiffTagInfo {
    bool operator==(const uint32_t& tag) const;

    uint32_t    tag_;
    const char* name_;
};

extern const TiffTagInfo tiffTagInfo[] = {
    {   0x10000, "none" },
    {   0x20000, "root" },
    {   0x30000, "next" },
    {   0x40000, "all"  }
};

bool TiffTagInfo::operator==(const uint32_t& tag) const
{
    return tag_ == tag;
}

std::string tiffTagName(uint32_t tag)
{
    const TiffTagInfo* gi = find(tiffTagInfo, tag);
    std::string name;
    if (gi != 0) {
        name = gi->name_;
    }
    else {
        std::ostringstream os;
        os << "0x"  << std::hex << std::setw(4)
           << std::setfill('0') << std::right << tag;
        name = os.str();
    }
    return name;
}

// -----------------------------------------------------------------------------
// Main program
int main(int argc, char* const argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " tag group\n"
                  << "Print the TIFF path for a tag and group (decimal numbers)\n";
        return 1;
    }

    uint32_t tag = atol(argv[1]);
    uint16_t grp = atol(argv[2]);

    TiffComponent* pRootDir = new TiffDirectory(0, 1);

    TiffPath tiffPath1;
    TiffCreator::getPath(tiffPath1, tag, grp);
    printPath(tiffPath1, tag, grp);
    addPath(pRootDir, tag, tiffPath1);

    ++tag;
    TiffPath tiffPath2;
    TiffCreator::getPath(tiffPath2, tag, grp);
    printPath(tiffPath2, tag, grp);
    addPath(pRootDir, tag, tiffPath2);

    return 0;
}

// -----------------------------------------------------------------------------
void addPath(TiffComponent* pRootDir,
             uint16_t       tag,
             TiffPath&      tiffPath)
{
    TiffComponent* tc = pRootDir->addPath(tag, tiffPath);

    TiffPrinter tiffPrinter(std::cout);
    pRootDir->accept(tiffPrinter);

    std::cout << std::endl;
    if (tc) {
        std::cout << "Added tag " << tiffTagName(tc->tag())
                  << ", group " << tiffGroupName(tc->group()) << "\n";
    }
    else {
        std::cout << "No tag added\n";
    }
    std::cout << std::endl;
}

// -----------------------------------------------------------------------------
void printPath(TiffPath tiffPath,
               uint32_t tag,
               uint16_t grp)
{
    std::cout << "\nTiff path for tag "
              << std::setw(6) << std::setfill(' ') << std::left
              << tiffTagName(tag)
              << ", group " << tiffGroupName(grp)
              << " (id = " << std::dec << grp << "):\n\n"
              << "ext. tag group        new group   \n"
              << "-------- ------------ ------------\n";

    while (!tiffPath.empty())
    {
        const TiffStructure* ts = tiffPath.top();
        tiffPath.pop();
        std::cout << std::setw(8) << std::setfill(' ') << std::left
                  << tiffTagName(ts->extendedTag_)
                  << " " << std::setw(12) << std::setfill(' ') << std::left
                  << tiffGroupName(ts->group_)
                  << " " << std::setw(12) << std::setfill(' ') << std::left
                  << tiffGroupName(ts->newGroup_)
                  << "\n";
    }
    std::cout << std::endl;
}
