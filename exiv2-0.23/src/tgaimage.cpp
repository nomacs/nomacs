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
  File:      tgaimage.cpp
  Version:   $Rev: 2681 $
  Author(s): Marco Piovanelli, Ovolab (marco)
  History:   05-Mar-2007, marco: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: tgaimage.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif
#include "tgaimage.hpp"
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

    TgaImage::TgaImage(BasicIo::AutoPtr io)
        : Image(ImageType::tga, mdNone, io)
    {
    } // TgaImage::TgaImage

    std::string TgaImage::mimeType() const
    {
        return "image/targa";
    }

    void TgaImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "TGA"));
    }

    void TgaImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "TGA"));
    }

    void TgaImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "TGA"));
    }

    void TgaImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::TgaImage::readMetadata: Reading TARGA file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isTgaType(*io_, false))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "TGA");
        }
        clearMetadata();

        /*
          The TARGA header goes as follows -- all numbers are in little-endian byte order:

          offset  length   name                     description
          ======  =======  =======================  ===========
           0      1 byte   ID length                length of image ID (0 to 255)
           1      1 byte   color map type           0 = no color map; 1 = color map included
           2      1 byte   image type                0 = no image;
                                                     1 = uncompressed color-mapped;
                                                     2 = uncompressed true-color;
                                                     3 = uncompressed black-and-white;
                                                     9 = RLE-encoded color mapped;
                                                    10 = RLE-encoded true-color;
                                                    11 = RLE-encoded black-and-white
           3      5 bytes  color map specification
           8      2 bytes  x-origin of image
          10      2 bytes  y-origin of image
          12      2 bytes  image width
          14      2 bytes  image height
          16      1 byte   pixel depth
          17      1 byte   image descriptor
        */
        byte buf[18];
        if (io_->read(buf, sizeof(buf)) == sizeof(buf))
        {
            pixelWidth_ = getShort(buf + 12, littleEndian);
            pixelHeight_ = getShort(buf + 14, littleEndian);
        }
    } // TgaImage::readMetadata

    void TgaImage::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "TGA"));
    } // TgaImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newTgaInstance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new TgaImage(io));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isTgaType(BasicIo& iIo, bool /*advance*/)
    {
        // not all TARGA files have a signature string, so first just try to match the file name extension
#ifdef EXV_UNICODE_PATH
        std::wstring wpath = iIo.wpath();
        if(   wpath.rfind(EXV_WIDEN(".tga")) != std::wstring::npos
           || wpath.rfind(EXV_WIDEN(".TGA")) != std::wstring::npos) {
            return true;
        }
#else
        std::string path = iIo.path();
        if(   path.rfind(".tga") != std::string::npos
           || path.rfind(".TGA") != std::string::npos) {
            return true;
        }
#endif
        byte buf[26];
        long curPos = iIo.tell();
        iIo.seek(-26, BasicIo::end);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        iIo.read(buf, sizeof(buf));
        if (iIo.error())
        {
            return false;
        }
        // some TARGA files, but not all, have a signature string at the end
        bool matched = (memcmp(buf + 8, "TRUEVISION-XFILE", 16) == 0);
        iIo.seek(curPos, BasicIo::beg);
        return matched;
    }
}                                       // namespace Exiv2
