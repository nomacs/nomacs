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
  File:      mrwimage.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-May-06, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: mrwimage.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "mrwimage.hpp"
#include "tiffimage.hpp"
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

    MrwImage::MrwImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::mrw, mdExif | mdIptc | mdXmp, io)
    {
    } // MrwImage::MrwImage

    std::string MrwImage::mimeType() const
    {
        return "image/x-minolta-mrw";
    }

    int MrwImage::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int MrwImage::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void MrwImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "MRW"));
    }

    void MrwImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "MRW"));
    }

    void MrwImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "MRW"));
    }

    void MrwImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading MRW file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isMrwType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "MRW");
        }
        clearMetadata();

        // Find the TTW block and read it into a buffer
        uint32_t const len = 8;
        byte tmp[len];
        io_->read(tmp, len);
        uint32_t pos = len;
        uint32_t const end = getULong(tmp + 4, bigEndian);

        pos += len;
        if (pos > end) throw Error(14);
        io_->read(tmp, len);
        if (io_->error() || io_->eof()) throw Error(14);

        while (memcmp(tmp + 1, "TTW", 3) != 0) {
            uint32_t const siz = getULong(tmp + 4, bigEndian);
            pos += siz;
            if (pos > end) throw Error(14);
            io_->seek(siz, BasicIo::cur);
            if (io_->error() || io_->eof()) throw Error(14);

            pos += len;
            if (pos > end) throw Error(14);
            io_->read(tmp, len);
            if (io_->error() || io_->eof()) throw Error(14);
        }

        DataBuf buf(getULong(tmp + 4, bigEndian));
        io_->read(buf.pData_, buf.size_);
        if (io_->error() || io_->eof()) throw Error(14);

        ByteOrder bo = TiffParser::decode(exifData_,
                                          iptcData_,
                                          xmpData_,
                                          buf.pData_,
                                          buf.size_);
        setByteOrder(bo);
    } // MrwImage::readMetadata

    void MrwImage::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "MRW"));
    } // MrwImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newMrwInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new MrwImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isMrwType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 4;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        int rc = memcmp(buf, "\0MRM", 4);
        if (!advance || rc != 0) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc == 0;
    }

}                                       // namespace Exiv2
