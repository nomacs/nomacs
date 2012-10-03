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
  File:      orfimage.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-May-06, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: orfimage.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "orfimage.hpp"
#include "orfimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    OrfImage::OrfImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::orf, mdExif | mdIptc | mdXmp, io)
    {
    } // OrfImage::OrfImage

    std::string OrfImage::mimeType() const
    {
        return "image/x-olympus-orf";
    }

    int OrfImage::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int OrfImage::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void OrfImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "ORF"));
    }

    void OrfImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading ORF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isOrfType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "ORF");
        }
        clearMetadata();
        ByteOrder bo = OrfParser::decode(exifData_,
                                         iptcData_,
                                         xmpData_,
                                         io_->mmap(),
                                         io_->size());
        setByteOrder(bo);
    } // OrfImage::readMetadata

    void OrfImage::writeMetadata()
    {
#ifdef DEBUG
        std::cerr << "Writing ORF file " << io_->path() << "\n";
#endif
        ByteOrder bo = byteOrder();
        byte* pData = 0;
        long size = 0;
        IoCloser closer(*io_);
        if (io_->open() == 0) {
            // Ensure that this is the correct image type
            if (isOrfType(*io_, false)) {
                pData = io_->mmap(true);
                size = io_->size();
                OrfHeader orfHeader;
                if (0 == orfHeader.read(pData, 8)) {
                    bo = orfHeader.byteOrder();
                }
            }
        }
        if (bo == invalidByteOrder) {
            bo = littleEndian;
        }
        setByteOrder(bo);
        OrfParser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_); // may throw
    } // OrfImage::writeMetadata

    ByteOrder OrfParser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        OrfHeader orfHeader;
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        Tag::root,
                                        TiffMapping::findDecoder,
                                        &orfHeader);
    }

    WriteMethod OrfParser::encode(
              BasicIo&  io,
        const byte*     pData,
              uint32_t  size,
              ByteOrder byteOrder,
        const ExifData& exifData,
        const IptcData& iptcData,
        const XmpData&  xmpData
    )
    {
        // Copy to be able to modify the Exif data
        ExifData ed = exifData;

        // Delete IFDs which do not occur in TIFF images
        static const IfdId filteredIfds[] = {
            panaRawId
        };
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredIfds); ++i) {
#ifdef DEBUG
            std::cerr << "Warning: Exif IFD " << filteredIfds[i] << " not encoded\n";
#endif
            ed.erase(std::remove_if(ed.begin(),
                                    ed.end(),
                                    FindExifdatum(filteredIfds[i])),
                     ed.end());
        }

        std::auto_ptr<TiffHeaderBase> header(new OrfHeader(byteOrder));
        return TiffParserWorker::encode(io,
                                        pData,
                                        size,
                                        ed,
                                        iptcData,
                                        xmpData,
                                        Tag::root,
                                        TiffMapping::findEncoder,
                                        header.get(),
                                        0);
    }

    // *************************************************************************
    // free functions
    Image::AutoPtr newOrfInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new OrfImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isOrfType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        OrfHeader orfHeader;
        bool rc = orfHeader.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    OrfHeader::OrfHeader(ByteOrder byteOrder)
        : TiffHeaderBase(0x4f52, 8, byteOrder, 0x00000008),
          sig_(0x4f52)
    {
    }

    OrfHeader::~OrfHeader()
    {
    }

    bool OrfHeader::read(const byte* pData, uint32_t size)
    {
        if (size < 8) return false;

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            setByteOrder(littleEndian);
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            setByteOrder(bigEndian);
        }
        else {
            return false;
        }
        uint16_t sig = getUShort(pData + 2, byteOrder());
        if (tag() != sig && 0x5352 != sig) return false; // #658: Added 0x5352 for SP-560UZ
        sig_ = sig;
        setOffset(getULong(pData + 4, byteOrder()));
        if (offset() != 0x00000008) return false;

        return true;
    } // OrfHeader::read

    DataBuf OrfHeader::write() const
    {
        DataBuf buf(8);
        switch (byteOrder()) {
        case littleEndian:
            buf.pData_[0] = 0x49;
            buf.pData_[1] = 0x49;
            break;
        case bigEndian:
            buf.pData_[0] = 0x4d;
            buf.pData_[1] = 0x4d;
            break;
        case invalidByteOrder:
            assert(false);
            break;
        }
        us2Data(buf.pData_ + 2, sig_, byteOrder());
        ul2Data(buf.pData_ + 4, 0x00000008, byteOrder());
        return buf;
    }

}}                                      // namespace Internal, Exiv2
