// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      crwimage.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   28-Aug-05, ahu: created

 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: crwimage.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// Define DEBUG to output debug information to std::cerr, e.g, by calling make
// like this: make DEFS=-DDEBUG crwimage.o
//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "crwimage.hpp"
#include "crwimage_int.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "value.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "canonmn_int.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <iostream>
#include <iomanip>
#include <stack>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cassert>
#ifndef EXV_HAVE_TIMEGM
# include "timegm.h"
#endif

// *****************************************************************************
// local declarations
namespace {
    //! Helper class to map Exif orientation values to CRW rotation degrees
    class RotationMap {
    public:
        //! Get the orientation number for a degree value
        static uint16_t orientation(int32_t degrees);
        //! Get the degree value for an orientation number
        static int32_t  degrees(uint16_t orientation);
    private:
        //! Helper structure for the mapping list
        struct OmList {
            uint16_t orientation; //!< Exif orientation value
            int32_t  degrees;     //!< CRW Rotation degrees
        };
        // DATA
        static const OmList omList_[];
    }; // class RotationMap
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    CrwImage::CrwImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::crw, mdExif | mdComment, io)
    {
    } // CrwImage::CrwImage

    std::string CrwImage::mimeType() const
    {
        return "image/x-canon-crw";
    }

    int CrwImage::pixelWidth() const
    {
        Exiv2::ExifData::const_iterator widthIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        if (widthIter != exifData_.end() && widthIter->count() > 0) {
            return widthIter->toLong();
        }
        return 0;
    }

    int CrwImage::pixelHeight() const
    {
        Exiv2::ExifData::const_iterator heightIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        if (heightIter != exifData_.end() && heightIter->count() > 0) {
            return heightIter->toLong();
        }
        return 0;
    }

    void CrwImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // not supported
        throw(Error(32, "IPTC metadata", "CRW"));
    }

    void CrwImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading CRW file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isCrwType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(33);
        }
        clearMetadata();
        CrwParser::decode(this, io_->mmap(), io_->size());

    } // CrwImage::readMetadata

    void CrwImage::writeMetadata()
    {
#ifdef DEBUG
        std::cerr << "Writing CRW file " << io_->path() << "\n";
#endif
        // Read existing image
        DataBuf buf;
        if (io_->open() == 0) {
            IoCloser closer(*io_);
            // Ensure that this is the correct image type
            if (isCrwType(*io_, false)) {
                // Read the image into a memory buffer
                buf.alloc(io_->size());
                io_->read(buf.pData_, buf.size_);
                if (io_->error() || io_->eof()) {
                    buf.reset();
                }
            }
        }

        Blob blob;
        CrwParser::encode(blob, buf.pData_, buf.size_, this);

        // Write new buffer to file
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert(tempIo.get() != 0);
        tempIo->write((blob.size() > 0 ? &blob[0] : 0), static_cast<long>(blob.size()));
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // CrwImage::writeMetadata

    void CrwParser::decode(CrwImage* pCrwImage, const byte* pData, uint32_t size)
    {
        assert(pCrwImage != 0);
        assert(pData != 0);

        // Parse the image, starting with a CIFF header component
        CiffHeader::AutoPtr head(new CiffHeader);
        head->read(pData, size);
#ifdef DEBUG
        head->print(std::cerr);
#endif
        head->decode(*pCrwImage);

        // a hack to get absolute offset of preview image inside CRW structure
        CiffComponent* preview = head->findComponent(0x2007, 0x0000);
        if (preview) {
            (pCrwImage->exifData())["Exif.Image2.JPEGInterchangeFormat"] = uint32_t(preview->pData() - pData);
            (pCrwImage->exifData())["Exif.Image2.JPEGInterchangeFormatLength"] = preview->size();
        }
    } // CrwParser::decode

    void CrwParser::encode(
              Blob&     blob,
        const byte*     pData,
              uint32_t  size,
        const CrwImage* pCrwImage
    )
    {
        // Parse image, starting with a CIFF header component
        CiffHeader::AutoPtr head(new CiffHeader);
        if (size != 0) {
            head->read(pData, size);
        }

        // Encode Exif tags from image into the CRW parse tree and write the
        // structure to the binary image blob
        CrwMap::encode(head.get(), *pCrwImage);
        head->write(blob);

    } // CrwParser::encode

    // *************************************************************************
    // free functions
    Image::AutoPtr newCrwInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new CrwImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isCrwType(BasicIo& iIo, bool advance)
    {
        bool result = true;
        byte tmpBuf[14];
        iIo.read(tmpBuf, 14);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        if (!(   ('I' == tmpBuf[0] && 'I' == tmpBuf[1])
              || ('M' == tmpBuf[0] && 'M' == tmpBuf[1]))) {
            result = false;
        }
        if (   true == result
            && std::memcmp(tmpBuf + 6, CiffHeader::signature(), 8) != 0) {
            result = false;
        }
        if (!advance || !result) iIo.seek(-14, BasicIo::cur);
        return result;
    }

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    /*
      Mapping table used to decode and encode CIFF tags to/from Exif tags.  Only
      a subset of the Exif tags can be mapped to known tags found in CRW files
      and not all CIFF tags in the CRW files have a corresponding Exif tag. Tags
      which are not mapped in the table below are ignored.

      When decoding, each CIFF tag/directory pair in the CRW image is looked up
      in the table and if it has an entry, the corresponding decode function is
      called (CrwMap::decode). This function may or may not make use of the
      other parameters in the structure (such as the Exif tag and Ifd id).

      Encoding is done in a loop over the mapping table (CrwMap::encode). For
      each entry, the encode function is called, which looks up the (Exif)
      metadata to encode in the image. This function may or may not make use of
      the other parameters in the mapping structure.
    */
    const CrwMapping CrwMap::crwMapping_[] = {
        //         CrwTag  CrwDir  Size ExifTag IfdId    decodeFct     encodeFct
        //         ------  ------  ---- ------- -----    ---------     ---------
        CrwMapping(0x0805, 0x300a,   0, 0,      canonId, decode0x0805, encode0x0805),
        CrwMapping(0x080a, 0x2807,   0, 0,      canonId, decode0x080a, encode0x080a),
        CrwMapping(0x080b, 0x3004,   0, 0x0007, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x0810, 0x2807,   0, 0x0009, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x0815, 0x2804,   0, 0x0006, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x1029, 0x300b,   0, 0x0002, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x102a, 0x300b,   0, 0x0004, canonId, decodeArray,  encodeArray),
        CrwMapping(0x102d, 0x300b,   0, 0x0001, canonId, decodeArray,  encodeArray),
        CrwMapping(0x1033, 0x300b,   0, 0x000f, canonId, decodeArray,  encodeArray),
        CrwMapping(0x1038, 0x300b,   0, 0x0012, canonId, decodeArray,  encodeArray),
        CrwMapping(0x10a9, 0x300b,   0, 0x00a9, canonId, decodeBasic,  encodeBasic),
        // Mapped to Exif.Photo.ColorSpace instead (see below)
        //CrwMapping(0x10b4, 0x300b,   0, 0x00b4, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x10b4, 0x300b,   0, 0xa001, exifId,  decodeBasic,  encodeBasic),
        CrwMapping(0x10b5, 0x300b,   0, 0x00b5, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x10c0, 0x300b,   0, 0x00c0, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x10c1, 0x300b,   0, 0x00c1, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x1807, 0x3002,   0, 0x9206, exifId,  decodeBasic,  encodeBasic),
        CrwMapping(0x180b, 0x3004,   0, 0x000c, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x180e, 0x300a,   0, 0x9003, exifId,  decode0x180e, encode0x180e),
        CrwMapping(0x1810, 0x300a,   0, 0xa002, exifId,  decode0x1810, encode0x1810),
        CrwMapping(0x1817, 0x300a,   4, 0x0008, canonId, decodeBasic,  encodeBasic),
        //CrwMapping(0x1818, 0x3002,   0, 0x9204, exifId, decodeBasic,  encodeBasic),
        CrwMapping(0x183b, 0x300b,   0, 0x0015, canonId, decodeBasic,  encodeBasic),
        CrwMapping(0x2008, 0x0000,   0, 0,      ifd1Id,  decode0x2008, encode0x2008),
        // End of list marker
        CrwMapping(0x0000, 0x0000,   0, 0x0000, ifdIdNotSet, 0,            0)
    }; // CrwMap::crwMapping_[]

    /*
      CIFF directory hierarchy

                root
                 |
                300a
                 |
       +----+----+----+----+
       |    |    |    |    |
      2804 2807 3002 3003 300b
            |
           3004

      The array is arranged bottom-up so that starting with a directory at the
      bottom, the (unique) path to root can be determined in a single loop.
    */
    const CrwSubDir CrwMap::crwSubDir_[] = {
        // dir,   parent
        { 0x3004, 0x2807 },
        { 0x300b, 0x300a },
        { 0x3003, 0x300a },
        { 0x3002, 0x300a },
        { 0x2807, 0x300a },
        { 0x2804, 0x300a },
        { 0x300a, 0x0000 },
        { 0x0000, 0xffff },
        // End of list marker
        { 0xffff, 0xffff }
    };

    const char CiffHeader::signature_[] = "HEAPCCDR";

    CiffHeader::~CiffHeader()
    {
        delete   pRootDir_;
        delete[] pPadding_;
    }

    CiffComponent::~CiffComponent()
    {
        if (isAllocated_) delete[] pData_;
    }

    CiffEntry::~CiffEntry()
    {
    }

    CiffDirectory::~CiffDirectory()
    {
        Components::iterator b = components_.begin();
        Components::iterator e = components_.end();
        for (Components::iterator i = b; i != e; ++i) {
            delete *i;
        }
    }

    void CiffComponent::add(AutoPtr component)
    {
        doAdd(component);
    }

    void CiffEntry::doAdd(AutoPtr /*component*/)
    {
        throw Error(34, "CiffEntry::add");
    } // CiffEntry::doAdd

    void CiffDirectory::doAdd(AutoPtr component)
    {
        components_.push_back(component.release());
    } // CiffDirectory::doAdd

    void CiffHeader::read(const byte* pData, uint32_t size)
    {
        if (size < 14) throw Error(33);

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            throw Error(33);
        }
        offset_ = getULong(pData + 2, byteOrder_);
        if (offset_ < 14 || offset_ > size) throw Error(33);
        if (std::memcmp(pData + 6, signature(), 8) != 0) {
            throw Error(33);
        }

        delete pPadding_;
        pPadding_ = new byte[offset_ - 14];
        padded_ = offset_ - 14;
        std::memcpy(pPadding_, pData + 14, padded_);

        pRootDir_ = new CiffDirectory;
        pRootDir_->readDirectory(pData + offset_, size - offset_, byteOrder_);
    } // CiffHeader::read

    void CiffComponent::read(const byte* pData,
                             uint32_t    size,
                             uint32_t    start,
                             ByteOrder   byteOrder)
    {
        doRead(pData, size, start, byteOrder);
    }

    void CiffComponent::doRead(const byte* pData,
                               uint32_t    size,
                               uint32_t    start,
                               ByteOrder   byteOrder)
    {
        if (size < 10) throw Error(33);
        tag_ = getUShort(pData + start, byteOrder);

        DataLocId dl = dataLocation();
        assert(dl == directoryData || dl == valueData);

        if (dl == valueData) {
            size_   = getULong(pData + start + 2, byteOrder);
            offset_ = getULong(pData + start + 6, byteOrder);
        }
        if (dl == directoryData) {
            size_ = 8;
            offset_ = start + 2;
        }
        pData_ = pData + offset_;
#ifdef DEBUG
        std::cout << "  Entry for tag 0x"
                  << std::hex << tagId() << " (0x" << tag()
                  << "), " << std::dec << size_
                  << " Bytes, Offset is " << offset_ << "\n";
#endif

    } // CiffComponent::doRead

    void CiffDirectory::doRead(const byte* pData,
                               uint32_t    size,
                               uint32_t    start,
                               ByteOrder   byteOrder)
    {
        CiffComponent::doRead(pData, size, start, byteOrder);
#ifdef DEBUG
        std::cout << "Reading directory 0x" << std::hex << tag() << "\n";
#endif
        readDirectory(pData + offset(), this->size(), byteOrder);
#ifdef DEBUG
        std::cout << "<---- 0x" << std::hex << tag() << "\n";
#endif
    } // CiffDirectory::doRead

    void CiffDirectory::readDirectory(const byte* pData,
                                      uint32_t    size,
                                      ByteOrder   byteOrder)
    {
        uint32_t o = getULong(pData + size - 4, byteOrder);
        if (o + 2 > size) throw Error(33);
        uint16_t count = getUShort(pData + o, byteOrder);
#ifdef DEBUG
        std::cout << "Directory at offset " << std::dec << o
                  <<", " << count << " entries \n";
#endif
        o += 2;
        for (uint16_t i = 0; i < count; ++i) {
            if (o + 10 > size) throw Error(33);
            uint16_t tag = getUShort(pData + o, byteOrder);
            CiffComponent::AutoPtr m;
            switch (CiffComponent::typeId(tag)) {
            case directory: m = CiffComponent::AutoPtr(new CiffDirectory); break;
            default: m = CiffComponent::AutoPtr(new CiffEntry); break;
            }
            m->setDir(this->tag());
            m->read(pData, size, o, byteOrder);
            add(m);
            o += 10;
        }
    }  // CiffDirectory::readDirectory

    void CiffHeader::decode(Image& image) const
    {
        // Nothing to decode from the header itself, just add correct byte order
        if (pRootDir_) pRootDir_->decode(image, byteOrder_);
    } // CiffHeader::decode

    void CiffComponent::decode(Image& image, ByteOrder byteOrder) const
    {
        doDecode(image, byteOrder);
    }

    void CiffEntry::doDecode(Image& image, ByteOrder byteOrder) const
    {
        CrwMap::decode(*this, image, byteOrder);
    } // CiffEntry::doDecode

    void CiffDirectory::doDecode(Image& image, ByteOrder byteOrder) const
    {
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->decode(image, byteOrder);
        }
    } // CiffDirectory::doDecode

    void CiffHeader::write(Blob& blob) const
    {
        assert(   byteOrder_ == littleEndian
               || byteOrder_ == bigEndian);
        if (byteOrder_ == littleEndian) {
            blob.push_back(0x49);
            blob.push_back(0x49);
        }
        else {
            blob.push_back(0x4d);
            blob.push_back(0x4d);
        }
        uint32_t o = 2;
        byte buf[4];
        ul2Data(buf, offset_, byteOrder_);
        append(blob, buf, 4);
        o += 4;
        append(blob, reinterpret_cast<const byte*>(signature_), 8);
        o += 8;
        // Pad as needed
        if (pPadding_) {
            assert(padded_ == offset_ - o);
            append(blob, pPadding_, padded_);
        }
        else {
            for (uint32_t i = o; i < offset_; ++i) {
                blob.push_back(0);
                ++o;
            }
        }
        if (pRootDir_) {
            pRootDir_->write(blob, byteOrder_, offset_);
        }
    }

    uint32_t CiffComponent::write(Blob&     blob,
                                  ByteOrder byteOrder,
                                  uint32_t  offset)
    {
        return doWrite(blob, byteOrder, offset);
    }

    uint32_t CiffEntry::doWrite(Blob&     blob,
                                ByteOrder /*byteOrder*/,
                                uint32_t  offset)
    {
        return writeValueData(blob, offset);
    } // CiffEntry::doWrite

    uint32_t CiffComponent::writeValueData(Blob& blob, uint32_t offset)
    {
        if (dataLocation() == valueData) {
#ifdef DEBUG
            std::cout << "  Data for tag 0x" << std::hex << tagId()
                      << ", " << std::dec << size_ << " Bytes\n";
#endif
            offset_ = offset;
            append(blob, pData_, size_);
            offset += size_;
            // Pad the value to an even number of bytes
            if (size_ % 2 == 1) {
                blob.push_back(0);
                ++offset;
            }
        }
        return offset;
    } // CiffComponent::writeValueData

    uint32_t CiffDirectory::doWrite(Blob&     blob,
                                    ByteOrder byteOrder,
                                    uint32_t  offset)
    {
#ifdef DEBUG
        std::cout << "Writing directory 0x" << std::hex << tag() << "---->\n";
#endif
        // Ciff offsets are relative to the start of the directory
        uint32_t dirOffset = 0;

        // Value data
        const Components::iterator b = components_.begin();
        const Components::iterator e = components_.end();
        for (Components::iterator i = b; i != e; ++i) {
            dirOffset = (*i)->write(blob, byteOrder, dirOffset);
        }
        const uint32_t dirStart = dirOffset;

        // Number of directory entries
        byte buf[4];
        us2Data(buf, static_cast<uint16_t>(components_.size()), byteOrder);
        append(blob, buf, 2);
        dirOffset += 2;

        // Directory entries
        for (Components::iterator i = b; i != e; ++i) {
            (*i)->writeDirEntry(blob, byteOrder);
            dirOffset += 10;
        }

        // Offset of directory
        ul2Data(buf, dirStart, byteOrder);
        append(blob, buf, 4);
        dirOffset += 4;

        // Update directory entry
        setOffset(offset);
        setSize(dirOffset);

#ifdef DEBUG
        std::cout << "Directory is at offset " << std::dec << dirStart
                  << ", " << components_.size() << " entries\n"
                  << "<---- 0x" << std::hex << tag() << "\n";
#endif
        return offset + dirOffset;
    } // CiffDirectory::doWrite

    void CiffComponent::writeDirEntry(Blob& blob, ByteOrder byteOrder) const
    {
#ifdef DEBUG
        std::cout << "  Directory entry for tag 0x"
                  << std::hex << tagId() << " (0x" << tag()
                  << "), " << std::dec << size_
                  << " Bytes, Offset is " << offset_ << "\n";
#endif
        byte buf[4];

        DataLocId dl = dataLocation();
        assert(dl == directoryData || dl == valueData);

        if (dl == valueData) {
            us2Data(buf, tag_, byteOrder);
            append(blob, buf, 2);

            ul2Data(buf, size_, byteOrder);
            append(blob, buf, 4);

            ul2Data(buf, offset_, byteOrder);
            append(blob, buf, 4);
        }

        if (dl == directoryData) {
            // Only 8 bytes fit in the directory entry
            assert(size_ <= 8);

            us2Data(buf, tag_, byteOrder);
            append(blob, buf, 2);
            // Copy value instead of size and offset
            append(blob, pData_, size_);
            // Pad with 0s
            for (uint32_t i = size_; i < 8; ++i) {
                blob.push_back(0);
            }
        }
    } // CiffComponent::writeDirEntry

    void CiffHeader::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << _("Header, offset") << " = 0x" << std::setw(8) << std::setfill('0')
           << std::hex << std::right << offset_ << "\n";
        if (pRootDir_) pRootDir_->print(os, byteOrder_, prefix);
    } // CiffHeader::print

    void CiffComponent::print(std::ostream&      os,
                              ByteOrder          byteOrder,
                              const std::string& prefix) const
    {
        doPrint(os, byteOrder, prefix);
    }

    void CiffComponent::doPrint(std::ostream&      os,
                                ByteOrder          byteOrder,
                                const std::string& prefix) const
    {
        os << prefix
           << _("tag") << " = 0x" << std::setw(4) << std::setfill('0')
           << std::hex << std::right << tagId()
           << ", " << _("dir") << " = 0x" << std::setw(4) << std::setfill('0')
           << std::hex << std::right << dir()
           << ", " << _("type") << " = " << TypeInfo::typeName(typeId())
           << ", " << _("size") << " = " << std::dec << size_
           << ", " << _("offset") << " = " << offset_ << "\n";

        Value::AutoPtr value;
        if (typeId() != directory) {
            value = Value::create(typeId());
            value->read(pData_, size_, byteOrder);
            if (value->size() < 100) {
                os << prefix << *value << "\n";
            }
        }
    } // CiffComponent::doPrint

    void CiffDirectory::doPrint(std::ostream&      os,
                                ByteOrder          byteOrder,
                                const std::string& prefix) const
    {
        CiffComponent::doPrint(os, byteOrder, prefix);
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->print(os, byteOrder, prefix + "   ");
        }
    } // CiffDirectory::doPrint

    void CiffComponent::setValue(DataBuf buf)
    {
        if (isAllocated_) {
            delete pData_;
            pData_ = 0;
            size_ = 0;
        }
        isAllocated_ = true;
        std::pair<byte *, long> p = buf.release();
        pData_ = p.first;
        size_  = p.second;
        if (size_ > 8 && dataLocation() == directoryData) {
            tag_ &= 0x3fff;
        }
    } // CiffComponent::setValue

    TypeId CiffComponent::typeId(uint16_t tag)
    {
        TypeId ti = invalidTypeId;
        switch (tag & 0x3800) {
        case 0x0000: ti = unsignedByte; break;
        case 0x0800: ti = asciiString; break;
        case 0x1000: ti = unsignedShort; break;
        case 0x1800: ti = unsignedLong; break;
        case 0x2000: ti = undefined; break;
        case 0x2800: // fallthrough
        case 0x3000: ti = directory; break;
        }
        return ti;
    } // CiffComponent::typeId

    DataLocId CiffComponent::dataLocation(uint16_t tag)
    {
        DataLocId di = invalidDataLocId;
        switch (tag & 0xc000) {
        case 0x0000: di = valueData; break;
        case 0x4000: di = directoryData; break;
        }
        return di;
    } // CiffComponent::dataLocation

    /*!
      @brief Finds \em crwTagId in directory \em crwDir, returning a pointer to
             the component or 0 if not found.

     */
    CiffComponent* CiffHeader::findComponent(uint16_t crwTagId,
                                             uint16_t crwDir) const
    {
        if (pRootDir_ == 0) return 0;
        return pRootDir_->findComponent(crwTagId, crwDir);
    } // CiffHeader::findComponent

    CiffComponent* CiffComponent::findComponent(uint16_t crwTagId,
                                                uint16_t crwDir) const
    {
        return doFindComponent(crwTagId, crwDir);
    } // CiffComponent::findComponent

    CiffComponent* CiffComponent::doFindComponent(uint16_t crwTagId,
                                                  uint16_t crwDir) const
    {
        if (tagId() == crwTagId && dir() == crwDir) {
            return const_cast<CiffComponent*>(this);
        }
        return 0;
    } // CiffComponent::doFindComponent

    CiffComponent* CiffDirectory::doFindComponent(uint16_t crwTagId,
                                                  uint16_t crwDir) const
    {
        CiffComponent* cc = 0;
        const Components::const_iterator b = components_.begin();
        const Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            cc = (*i)->findComponent(crwTagId, crwDir);
            if (cc) return cc;
        }
        return 0;
    } // CiffDirectory::doFindComponent

    void CiffHeader::add(uint16_t crwTagId, uint16_t crwDir, DataBuf buf)
    {
        CrwDirs crwDirs;
        CrwMap::loadStack(crwDirs, crwDir);
        uint16_t rootDirectory = crwDirs.top().crwDir_;
        assert(rootDirectory == 0x0000);
        crwDirs.pop();
        if (!pRootDir_) pRootDir_ = new CiffDirectory;
        CiffComponent* cc = pRootDir_->add(crwDirs, crwTagId);
        cc->setValue(buf);
    } // CiffHeader::add

    CiffComponent* CiffComponent::add(CrwDirs& crwDirs, uint16_t crwTagId)
    {
        return doAdd(crwDirs, crwTagId);
    } // CiffComponent::add

    CiffComponent* CiffComponent::doAdd(CrwDirs& /*crwDirs*/, uint16_t /*crwTagId*/)
    {
        return 0;
    } // CiffComponent::doAdd

    CiffComponent* CiffDirectory::doAdd(CrwDirs& crwDirs, uint16_t crwTagId)
    {
        /*
          add()
            if stack not empty
              pop from stack
              find dir among components
              if not found, create it
              add()
            else
              find tag among components
              if not found, create it
              set value
        */

        CiffComponent* cc = 0;
        const Components::iterator b = components_.begin();
        const Components::iterator e = components_.end();

        if (!crwDirs.empty()) {
            CrwSubDir csd = crwDirs.top();
            crwDirs.pop();
            // Find the directory
            for (Components::iterator i = b; i != e; ++i) {
                if ((*i)->tag() == csd.crwDir_) {
                    cc = *i;
                    break;
                }
            }
            if (cc == 0) {
                // Directory doesn't exist yet, add it
                AutoPtr m(new CiffDirectory(csd.crwDir_, csd.parent_));
                cc = m.get();
                add(m);
            }
            // Recursive call to next lower level directory
            cc = cc->add(crwDirs, crwTagId);
        }
        else {
            // Find the tag
            for (Components::iterator i = b; i != e; ++i) {
                if ((*i)->tagId() == crwTagId) {
                    cc = *i;
                    break;
                }
            }
            if (cc == 0) {
                // Tag doesn't exist yet, add it
                AutoPtr m(new CiffEntry(crwTagId, tag()));
                cc = m.get();
                add(m);
            }
        }
        return cc;
    } // CiffDirectory::doAdd

    void CiffHeader::remove(uint16_t crwTagId, uint16_t crwDir)
    {
        if (pRootDir_) {
            CrwDirs crwDirs;
            CrwMap::loadStack(crwDirs, crwDir);
            uint16_t rootDirectory = crwDirs.top().crwDir_;
            assert(rootDirectory == 0x0000);
            crwDirs.pop();
            pRootDir_->remove(crwDirs, crwTagId);
        }
    } // CiffHeader::remove

    void CiffComponent::remove(CrwDirs& crwDirs, uint16_t crwTagId)
    {
        return doRemove(crwDirs, crwTagId);
    } // CiffComponent::remove

    void CiffComponent::doRemove(CrwDirs& /*crwDirs*/, uint16_t /*crwTagId*/)
    {
        // do nothing
    } // CiffComponent::doRemove

    void CiffDirectory::doRemove(CrwDirs& crwDirs, uint16_t crwTagId)
    {
        const Components::iterator b = components_.begin();
        const Components::iterator e = components_.end();
        Components::iterator i;

        if (!crwDirs.empty()) {
            CrwSubDir csd = crwDirs.top();
            crwDirs.pop();
            // Find the directory
            for (i = b; i != e; ++i) {
                if ((*i)->tag() == csd.crwDir_) {
                    // Recursive call to next lower level directory
                    (*i)->remove(crwDirs, crwTagId);
                    if ((*i)->empty()) components_.erase(i);
                    break;
                }
            }
        }
        else {
            // Find the tag
            for (i = b; i != e; ++i) {
                if ((*i)->tagId() == crwTagId) {
                    // Remove the entry and abort the loop
                    delete *i;
                    components_.erase(i);
                    break;
                }
            }
        }
    } // CiffDirectory::doRemove

    bool CiffComponent::empty() const
    {
        return doEmpty();
    }

    bool CiffComponent::doEmpty() const
    {
        return size_ == 0;
    }

    bool CiffDirectory::doEmpty() const
    {
        return components_.empty();
    }

    void CrwMap::decode(const CiffComponent& ciffComponent,
                        Image&               image,
                        ByteOrder            byteOrder)
    {
        const CrwMapping* cmi = crwMapping(ciffComponent.dir(),
                                           ciffComponent.tagId());
        if (cmi && cmi->toExif_) {
            cmi->toExif_(ciffComponent, cmi, image, byteOrder);
        }
    } // CrwMap::decode

    const CrwMapping* CrwMap::crwMapping(uint16_t crwDir, uint16_t crwTagId)
    {
        for (int i = 0; crwMapping_[i].ifdId_ != ifdIdNotSet; ++i) {
            if (   crwMapping_[i].crwDir_ == crwDir
                && crwMapping_[i].crwTagId_ == crwTagId) {
                return &(crwMapping_[i]);
            }
        }
        return 0;
    } // CrwMap::crwMapping

    void CrwMap::decode0x0805(const CiffComponent& ciffComponent,
                              const CrwMapping*    /*pCrwMapping*/,
                                    Image&         image,
                                    ByteOrder      /*byteOrder*/)
    {
        std::string s(reinterpret_cast<const char*>(ciffComponent.pData()));
        image.setComment(s);
    } // CrwMap::decode0x0805

    void CrwMap::decode0x080a(const CiffComponent& ciffComponent,
                              const CrwMapping*    /*pCrwMapping*/,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.typeId() != asciiString) return;

        // Make
        ExifKey key1("Exif.Image.Make");
        Value::AutoPtr value1 = Value::create(ciffComponent.typeId());
        uint32_t i = 0;
        for (;    i < ciffComponent.size()
               && ciffComponent.pData()[i] != '\0'; ++i) {
            // empty
        }
        value1->read(ciffComponent.pData(), ++i, byteOrder);
        image.exifData().add(key1, value1.get());

        // Model
        ExifKey key2("Exif.Image.Model");
        Value::AutoPtr value2 = Value::create(ciffComponent.typeId());
        uint32_t j = i;
        for (;    i < ciffComponent.size()
               && ciffComponent.pData()[i] != '\0'; ++i) {
            // empty
        }
        value2->read(ciffComponent.pData() + j, i - j + 1, byteOrder);
        image.exifData().add(key2, value2.get());
    } // CrwMap::decode0x080a

    void CrwMap::decodeArray(const CiffComponent& ciffComponent,
                             const CrwMapping*    pCrwMapping,
                                   Image&         image,
                                   ByteOrder      byteOrder)
    {
        if (ciffComponent.typeId() != unsignedShort) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
        }

        long aperture = 0;
        long shutterSpeed = 0;

        IfdId ifdId = ifdIdNotSet;
        switch (pCrwMapping->tag_) {
        case 0x0001: ifdId = canonCsId; break;
        case 0x0004: ifdId = canonSiId; break;
        case 0x000f: ifdId = canonCfId; break;
        case 0x0012: ifdId = canonPiId; break;
        }
        assert(ifdId != ifdIdNotSet);

        std::string groupName(Internal::groupName(ifdId));
        uint16_t c = 1;
        while (uint32_t(c)*2 < ciffComponent.size()) {
            uint16_t n = 1;
            ExifKey key(c, groupName);
            UShortValue value;
            if (ifdId == canonCsId && c == 23 && ciffComponent.size() > 50) n = 3;
            value.read(ciffComponent.pData() + c*2, n*2, byteOrder);
            image.exifData().add(key, &value);
            if (ifdId == canonSiId && c == 21) aperture = value.toLong();
            if (ifdId == canonSiId && c == 22) shutterSpeed = value.toLong();
            c += n;
        }

        if (ifdId == canonSiId) {
            // Exif.Photo.FNumber
            float f = fnumber(canonEv(aperture));
            Rational r = floatToRationalCast(f);
            URational ur(r.first, r.second);
            URationalValue fn;
            fn.value_.push_back(ur);
            image.exifData().add(ExifKey("Exif.Photo.FNumber"), &fn);

            // Exif.Photo.ExposureTime
            ur = exposureTime(canonEv(shutterSpeed));
            URationalValue et;
            et.value_.push_back(ur);
            image.exifData().add(ExifKey("Exif.Photo.ExposureTime"), &et);
        }
    } // CrwMap::decodeArray

    void CrwMap::decode0x180e(const CiffComponent& ciffComponent,
                              const CrwMapping*    pCrwMapping,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.size() < 8 || ciffComponent.typeId() != unsignedLong) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
        }
        assert(pCrwMapping != 0);
        ULongValue v;
        v.read(ciffComponent.pData(), 8, byteOrder);
        time_t t = v.value_[0];
#ifdef EXV_HAVE_GMTIME_R
        struct tm tms;
        struct tm* tm = &tms;
        tm = gmtime_r(&t, tm);
#else
        struct tm* tm = std::gmtime(&t);
#endif
        if (tm) {
            const size_t m = 20;
            char s[m];
            std::strftime(s, m, "%Y:%m:%d %H:%M:%S", tm);

            ExifKey key(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
            AsciiValue value;
            value.read(std::string(s));
            image.exifData().add(key, &value);
        }
    } // CrwMap::decode0x180e

    void CrwMap::decode0x1810(const CiffComponent& ciffComponent,
                              const CrwMapping*    pCrwMapping,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.typeId() != unsignedLong || ciffComponent.size() < 28) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
        }

        ExifKey key1("Exif.Photo.PixelXDimension");
        ULongValue value1;
        value1.read(ciffComponent.pData(), 4, byteOrder);
        image.exifData().add(key1, &value1);

        ExifKey key2("Exif.Photo.PixelYDimension");
        ULongValue value2;
        value2.read(ciffComponent.pData() + 4, 4, byteOrder);
        image.exifData().add(key2, &value2);

        int32_t r = getLong(ciffComponent.pData() + 12, byteOrder);
        uint16_t o = RotationMap::orientation(r);
        image.exifData()["Exif.Image.Orientation"] = o;

    } // CrwMap::decode0x1810

    void CrwMap::decode0x2008(const CiffComponent& ciffComponent,
                              const CrwMapping*    /*pCrwMapping*/,
                                    Image&         image,
                                    ByteOrder      /*byteOrder*/)
    {
        ExifThumb exifThumb(image.exifData());
        exifThumb.setJpegThumbnail(ciffComponent.pData(), ciffComponent.size());
    } // CrwMap::decode0x2008

    void CrwMap::decodeBasic(const CiffComponent& ciffComponent,
                             const CrwMapping*    pCrwMapping,
                                   Image&         image,
                                   ByteOrder      byteOrder)
    {
        assert(pCrwMapping != 0);
        // create a key and value pair
        ExifKey key(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
        Value::AutoPtr value;
        if (ciffComponent.typeId() != directory) {
            value = Value::create(ciffComponent.typeId());
            uint32_t size = 0;
            if (pCrwMapping->size_ != 0) {
                // size in the mapping table overrides all
                size = pCrwMapping->size_;
            }
            else if (ciffComponent.typeId() == asciiString) {
                // determine size from the data, by looking for the first 0
                uint32_t i = 0;
                for (;    i < ciffComponent.size()
                       && ciffComponent.pData()[i] != '\0'; ++i) {
                    // empty
                }
                size = ++i;
            }
            else {
                // by default, use the size from the directory entry
                size = ciffComponent.size();
            }
            value->read(ciffComponent.pData(), size, byteOrder);
        }
        // Add metadatum to exif data
        image.exifData().add(key, value.get());
    } // CrwMap::decodeBasic

    void CrwMap::loadStack(CrwDirs& crwDirs, uint16_t crwDir)
    {
        for (int i = 0; crwSubDir_[i].crwDir_ != 0xffff; ++i) {
            if (crwSubDir_[i].crwDir_ == crwDir) {
                crwDirs.push(crwSubDir_[i]);
                crwDir = crwSubDir_[i].parent_;
            }
        }
    } // CrwMap::loadStack

    void CrwMap::encode(CiffHeader* pHead, const Image& image)
    {
        for (const CrwMapping* cmi = crwMapping_; cmi->ifdId_ != ifdIdNotSet; ++cmi) {
            if (cmi->fromExif_ != 0) {
                cmi->fromExif_(image, cmi, pHead);
            }
        }
    } // CrwMap::encode

    void CrwMap::encodeBasic(const Image&      image,
                             const CrwMapping* pCrwMapping,
                                   CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);

        // Determine the source Exif metadatum
        ExifKey ek(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
        ExifData::const_iterator ed = image.exifData().findKey(ek);

        // Set the new value or remove the entry
        if (ed != image.exifData().end()) {
            DataBuf buf(ed->size());
            ed->copy(buf.pData_, pHead->byteOrder());
            pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, buf);
        }
        else {
            pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
        }
    } // CrwMap::encodeBasic

    void CrwMap::encode0x0805(const Image&      image,
                              const CrwMapping* pCrwMapping,
                                    CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);

        std::string comment = image.comment();

        CiffComponent* cc = pHead->findComponent(pCrwMapping->crwTagId_,
                                                 pCrwMapping->crwDir_);
        if (!comment.empty()) {
            uint32_t size = static_cast<uint32_t>(comment.size());
            if (cc && cc->size() > size) size = cc->size();
            DataBuf buf(size);
            std::memset(buf.pData_, 0x0, buf.size_);
            std::memcpy(buf.pData_, comment.data(), comment.size());
            pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, buf);
        }
        else {
            if (cc) {
                // Just delete the value, do not remove the tag
                DataBuf buf(cc->size());
                std::memset(buf.pData_, 0x0, buf.size_);
                cc->setValue(buf);
            }
        }
    } // CrwMap::encode0x0805

    void CrwMap::encode0x080a(const Image&      image,
                              const CrwMapping* pCrwMapping,
                                    CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);

        const ExifKey k1("Exif.Image.Make");
        const ExifKey k2("Exif.Image.Model");
        const ExifData::const_iterator ed1 = image.exifData().findKey(k1);
        const ExifData::const_iterator ed2 = image.exifData().findKey(k2);
        const ExifData::const_iterator edEnd = image.exifData().end();

        long size = 0;
        if (ed1 != edEnd) size += ed1->size();
        if (ed2 != edEnd) size += ed2->size();
        if (size != 0) {
            DataBuf buf(size);
            if (ed1 != edEnd) ed1->copy(buf.pData_, pHead->byteOrder());
            if (ed2 != edEnd) ed2->copy(buf.pData_ + ed1->size(), pHead->byteOrder());
            pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, buf);
        }
        else {
            pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
        }
    } // CrwMap::encode0x080a

    void CrwMap::encodeArray(const Image&      image,
                             const CrwMapping* pCrwMapping,
                                   CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);

        IfdId ifdId = ifdIdNotSet;
        switch (pCrwMapping->tag_) {
        case 0x0001: ifdId = canonCsId; break;
        case 0x0004: ifdId = canonSiId; break;
        case 0x000f: ifdId = canonCfId; break;
        case 0x0012: ifdId = canonPiId; break;
        }
        assert(ifdId != ifdIdNotSet);
        DataBuf buf = packIfdId(image.exifData(), ifdId, pHead->byteOrder());
        if (buf.size_ == 0) {
            // Try the undecoded tag
            encodeBasic(image, pCrwMapping, pHead);
        }
        if (buf.size_ > 0) {
            // Write the number of shorts to the beginning of buf
            us2Data(buf.pData_, static_cast<uint16_t>(buf.size_), pHead->byteOrder());
            pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, buf);
        }
        else {
            pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
        }
    } // CrwMap::encodeArray

    void CrwMap::encode0x180e(const Image&      image,
                              const CrwMapping* pCrwMapping,
                                    CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);

        time_t t = 0;
        const ExifKey key(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
        const ExifData::const_iterator ed = image.exifData().findKey(key);
        if (ed != image.exifData().end()) {
            struct tm tm;
            std::memset(&tm, 0x0, sizeof(tm));
            int rc = exifTime(ed->toString().c_str(), &tm);
            if (rc == 0) t = timegm(&tm);
        }
        if (t != 0) {
            DataBuf buf(12);
            std::memset(buf.pData_, 0x0, 12);
            ul2Data(buf.pData_, static_cast<uint32_t>(t), pHead->byteOrder());
            pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, buf);
        }
        else {
            pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
        }
    } // CrwMap::encode0x180e

    void CrwMap::encode0x1810(const Image&      image,
                              const CrwMapping* pCrwMapping,
                                    CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);

        const ExifKey kX("Exif.Photo.PixelXDimension");
        const ExifKey kY("Exif.Photo.PixelYDimension");
        const ExifKey kO("Exif.Image.Orientation");
        const ExifData::const_iterator edX = image.exifData().findKey(kX);
        const ExifData::const_iterator edY = image.exifData().findKey(kY);
        const ExifData::const_iterator edO = image.exifData().findKey(kO);
        const ExifData::const_iterator edEnd = image.exifData().end();

        CiffComponent* cc = pHead->findComponent(pCrwMapping->crwTagId_,
                                                 pCrwMapping->crwDir_);
        if (edX != edEnd || edY != edEnd || edO != edEnd) {
            uint32_t size = 28;
            if (cc && cc->size() > size) size = cc->size();
            DataBuf buf(size);
            std::memset(buf.pData_, 0x0, buf.size_);
            if (cc) std::memcpy(buf.pData_ + 8, cc->pData() + 8, cc->size() - 8);
            if (edX != edEnd && edX->size() == 4) {
                edX->copy(buf.pData_, pHead->byteOrder());
            }
            if (edY != edEnd && edY->size() == 4) {
                edY->copy(buf.pData_ + 4, pHead->byteOrder());
            }
            int32_t d = 0;
            if (edO != edEnd && edO->count() > 0 && edO->typeId() == unsignedShort) {
                d = RotationMap::degrees(static_cast<uint16_t>(edO->toLong()));
            }
            l2Data(buf.pData_ + 12, d, pHead->byteOrder());
            pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, buf);
        }
        else {
            pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
        }
    } // CrwMap::encode0x1810

    void CrwMap::encode0x2008(const Image&      image,
                              const CrwMapping* pCrwMapping,
                                    CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);

        ExifThumbC exifThumb(image.exifData());
        DataBuf buf = exifThumb.copy();
        if (buf.size_ != 0) {
            pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, buf);
        }
        else {
            pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
        }
    } // CrwMap::encode0x2008

    // *************************************************************************
    // free functions
    DataBuf packIfdId(const ExifData& exifData,
                            IfdId     ifdId,
                            ByteOrder byteOrder)
    {
        const uint16_t size = 1024;
        DataBuf buf(size);
        std::memset(buf.pData_, 0x0, buf.size_);

        uint16_t len = 0;
        const ExifData::const_iterator b = exifData.begin();
        const ExifData::const_iterator e = exifData.end();
        for (ExifData::const_iterator i = b; i != e; ++i) {
            if (i->ifdId() != ifdId) continue;
            const uint16_t s = i->tag()*2 + static_cast<uint16_t>(i->size());
            assert(s <= size);
            if (len < s) len = s;
            i->copy(buf.pData_ + i->tag()*2, byteOrder);
        }
        // Round the size to make it even.
        buf.size_ = len + len%2;
        return buf;
    }

}}                                       // namespace Internal, Exiv2

// *****************************************************************************
// local definitions
namespace {
    //! @cond IGNORE
    const RotationMap::OmList RotationMap::omList_[] = {
        { 1,    0 },
        { 3,  180 },
        { 3, -180 },
        { 6,   90 },
        { 6, -270 },
        { 8,  270 },
        { 8,  -90 },
        // last entry
        { 0,    0 }
    };

    uint16_t RotationMap::orientation(int32_t degrees)
    {
        uint16_t o = 1;
        for (int i = 0; omList_[i].orientation != 0; ++i) {
            if (omList_[i].degrees == degrees) {
                o = omList_[i].orientation;
                break;
            }
        }
        return o;
    }

    int32_t RotationMap::degrees(uint16_t orientation)
    {
        int32_t d = 0;
        for (int i = 0; omList_[i].orientation != 0; ++i) {
            if (omList_[i].orientation == orientation) {
                d = omList_[i].degrees;
                break;
            }
        }
        return d;
    }
    //! @endcond
}
