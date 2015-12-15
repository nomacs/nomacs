// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
  File:      gifimage.cpp
  Version:   $Rev: 3777 $
  Author(s): Marco Piovanelli, Ovolab (marco)
  History:   26-Feb-2007, marco: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: gifimage.cpp 3777 2015-05-02 11:55:40Z ahuggel $")

// *****************************************************************************
// included header files
#include "config.h"

#include "gifimage.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    GifImage::GifImage(BasicIo::AutoPtr io)
        : Image(ImageType::gif, mdNone, io)
    {
    } // GifImage::GifImage

    std::string GifImage::mimeType() const
    {
        return "image/gif";
    }

    void GifImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "GIF"));
    }

    void GifImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "GIF"));
    }

    void GifImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "GIF"));
    }

    void GifImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::GifImage::readMetadata: Reading GIF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isGifType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "GIF");
        }
        clearMetadata();

        byte buf[4];
        if (io_->read(buf, sizeof(buf)) == sizeof(buf))
        {
            pixelWidth_ = getShort(buf, littleEndian);
            pixelHeight_ = getShort(buf + 2, littleEndian);
        }
    } // GifImage::readMetadata

    void GifImage::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "GIF"));
    } // GifImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newGifInstance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new GifImage(io));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isGifType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 6;
        const unsigned char Gif87aId[8] = { 'G', 'I', 'F', '8', '7', 'a' };
        const unsigned char Gif89aId[8] = { 'G', 'I', 'F', '8', '9', 'a' };
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        bool matched =    (memcmp(buf, Gif87aId, len) == 0)
                       || (memcmp(buf, Gif89aId, len) == 0);
        if (!advance || !matched)
        {
            iIo.seek(-len, BasicIo::cur);
        }
        return matched;
    }
}                                       // namespace Exiv2
