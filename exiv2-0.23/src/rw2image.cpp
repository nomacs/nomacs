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
  File:      rw2image.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-Jan-09, ahu: created

 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: rw2image.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "rw2image.hpp"
#include "rw2image_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "image.hpp"
#include "preview.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#ifdef DEBUG
# include <iostream>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    Rw2Image::Rw2Image(BasicIo::AutoPtr io)
        : Image(ImageType::rw2, mdExif | mdIptc | mdXmp, io)
    {
    } // Rw2Image::Rw2Image

    std::string Rw2Image::mimeType() const
    {
        return "image/x-panasonic-rw2";
    }

    int Rw2Image::pixelWidth() const
    {
        ExifData::const_iterator imageWidth =
            exifData_.findKey(Exiv2::ExifKey("Exif.PanasonicRaw.SensorWidth"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int Rw2Image::pixelHeight() const
    {
        ExifData::const_iterator imageHeight =
            exifData_.findKey(Exiv2::ExifKey("Exif.PanasonicRaw.SensorHeight"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void Rw2Image::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "RW2"));
    }

    void Rw2Image::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "RW2"));
    }

    void Rw2Image::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "RW2"));
    }

    void Rw2Image::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading RW2 file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isRw2Type(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "RW2");
        }
        clearMetadata();
        ByteOrder bo = Rw2Parser::decode(exifData_,
                                         iptcData_,
                                         xmpData_,
                                         io_->mmap(),
                                         io_->size());
        setByteOrder(bo);

        // A lot more metadata is hidden in the embedded preview image
        // Todo: This should go into the Rw2Parser, but for that it needs the Image
        PreviewManager loader(*this);
        PreviewPropertiesList list = loader.getPreviewProperties();
        // Todo: What if there are more preview images?
        if (list.size() > 1) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "RW2 image contains more than one preview. None used.\n";
#endif
        }
        if (list.size() != 1) return;
        ExifData exifData;
        PreviewImage preview = loader.getPreviewImage(*list.begin());
        Image::AutoPtr image = ImageFactory::open(preview.pData(), preview.size());
        if (image.get() == 0) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Failed to open RW2 preview image.\n";
#endif
            return;
        }
        image->readMetadata();
        ExifData& prevData = image->exifData();
        if (!prevData.empty()) {
            // Filter duplicate tags
            for (ExifData::const_iterator pos = exifData_.begin(); pos != exifData_.end(); ++pos) {
                if (pos->ifdId() == panaRawId) continue;
                ExifData::iterator dup = prevData.findKey(ExifKey(pos->key()));
                if (dup != prevData.end()) {
#ifdef DEBUG
                    std::cerr << "Filtering duplicate tag " << pos->key()
                              << " (values '" << pos->value()
                              << "' and '" << dup->value() << "')\n";
#endif
                    prevData.erase(dup);
                }
            }
        }
        // Remove tags not applicable for raw images
        static const char* filteredTags[] = {
            "Exif.Photo.ComponentsConfiguration",
            "Exif.Photo.CompressedBitsPerPixel",
            "Exif.Panasonic.ColorEffect",
            "Exif.Panasonic.Contrast",
            "Exif.Panasonic.NoiseReduction",
            "Exif.Panasonic.ColorMode",
            "Exif.Panasonic.OpticalZoomMode",
            "Exif.Panasonic.Contrast",
            "Exif.Panasonic.Saturation",
            "Exif.Panasonic.Sharpness",
            "Exif.Panasonic.FilmMode",
            "Exif.Panasonic.SceneMode",
            "Exif.Panasonic.WBRedLevel",
            "Exif.Panasonic.WBGreenLevel",
            "Exif.Panasonic.WBBlueLevel",
            "Exif.Photo.ColorSpace",
            "Exif.Photo.PixelXDimension",
            "Exif.Photo.PixelYDimension",
            "Exif.Photo.SceneType",
            "Exif.Photo.CustomRendered",
            "Exif.Photo.DigitalZoomRatio",
            "Exif.Photo.SceneCaptureType",
            "Exif.Photo.GainControl",
            "Exif.Photo.Contrast",
            "Exif.Photo.Saturation",
            "Exif.Photo.Sharpness",
            "Exif.Image.PrintImageMatching",
            "Exif.Image.YCbCrPositioning"
        };
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredTags); ++i) {
            ExifData::iterator pos = prevData.findKey(ExifKey(filteredTags[i]));
            if (pos != prevData.end()) {
#ifdef DEBUG
                std::cerr << "Exif tag " << pos->key() << " removed\n";
#endif
                prevData.erase(pos);
            }
        }

        // Add the remaining tags
        for (ExifData::const_iterator pos = prevData.begin(); pos != prevData.end(); ++pos) {
            exifData_.add(*pos);
        }

    } // Rw2Image::readMetadata

    void Rw2Image::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "RW2"));
    } // Rw2Image::writeMetadata

    ByteOrder Rw2Parser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        Rw2Header rw2Header;
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        Tag::pana,
                                        TiffMapping::findDecoder,
                                        &rw2Header);
    }

    // *************************************************************************
    // free functions
    Image::AutoPtr newRw2Instance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new Rw2Image(io));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isRw2Type(BasicIo& iIo, bool advance)
    {
        const int32_t len = 24;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        Rw2Header header;
        bool rc = header.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    Rw2Header::Rw2Header()
        : TiffHeaderBase(0x0055, 24, littleEndian, 0x00000018)
    {
    }

    Rw2Header::~Rw2Header()
    {
    }

    DataBuf Rw2Header::write() const
    {
        // Todo: Implement me!
        return DataBuf();
    }

}}                                      // namespace Internal, Exiv2
