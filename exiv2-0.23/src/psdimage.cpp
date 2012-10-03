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
  File:      psdimage.cpp
  Version:   $Rev: 2681 $
  Author(s): Marco Piovanelli, Ovolab (marco)
             Michael Ulbrich (mul)
  History:   05-Mar-2007, marco: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: psdimage.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif
#include "psdimage.hpp"
#include "jpgimage.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <memory>

// Todo: Consolidate with existing code in struct Photoshop (jpgimage.hpp):
//       Extend this helper to a proper class with all required functionality,
//       then move it here or into a separate file?

//! @cond IGNORE
struct PhotoshopResourceBlock {
    uint32_t      resourceType;       // one of the markers in Photoshop::irbId_[]
    uint16_t      resourceId;
    unsigned char resourceName[2];    // Pascal string (length byte + characters), padded to an even size -- this assumes the empty string
    uint32_t      resourceDataSize;
};
//! @endcond

// Photoshop resource IDs (Cf. <http://search.cpan.org/~bettelli/Image-MetaData-JPEG-0.15/lib/Image/MetaData/JPEG/TagLists.pod>)
enum {
    kPhotoshopResourceID_Photoshop2Info            = 0x03e8, // [obsolete -- Photoshop 2.0 only] General information -- contains five 2-byte values: number of channels, rows, columns, depth and mode
    kPhotoshopResourceID_MacintoshClassicPrintInfo = 0x03e9, // [optional] Macintosh classic print record (120 bytes)
    kPhotoshopResourceID_MacintoshCarbonPrintInfo  = 0x03ea, // [optional] Macintosh carbon print info (variable-length XML format)
    kPhotoshopResourceID_Photoshop2ColorTable      = 0x03eb, // [obsolete -- Photoshop 2.0 only] Indexed color table
    kPhotoshopResourceID_ResolutionInfo            = 0x03ed, // PhotoshopResolutionInfo structure (see below)
    kPhotoshopResourceID_AlphaChannelsNames        = 0x03ee, // as a series of Pstrings
    kPhotoshopResourceID_DisplayInfo               = 0x03ef, // see appendix A in Photoshop SDK
    kPhotoshopResourceID_PStringCaption            = 0x03f0, // [optional] the caption, as a Pstring
    kPhotoshopResourceID_BorderInformation         = 0x03f1, // border width and units
    kPhotoshopResourceID_BackgroundColor           = 0x03f2, // see additional Adobe information
    kPhotoshopResourceID_PrintFlags                = 0x03f3, // labels, crop marks, colour bars, ecc...
    kPhotoshopResourceID_BWHalftoningInfo          = 0x03f4, // Gray-scale and multich. half-toning info
    kPhotoshopResourceID_ColorHalftoningInfo       = 0x03f5, // Colour half-toning information
    kPhotoshopResourceID_DuotoneHalftoningInfo     = 0x03f6, // Duo-tone half-toning information
    kPhotoshopResourceID_BWTransferFunc            = 0x03f7, // Gray-scale and multich. transfer function
    kPhotoshopResourceID_ColorTransferFuncs        = 0x03f8, // Colour transfer function
    kPhotoshopResourceID_DuotoneTransferFuncs      = 0x03f9, // Duo-tone transfer function
    kPhotoshopResourceID_DuotoneImageInfo          = 0x03fa, // Duo-tone image information
    kPhotoshopResourceID_EffectiveBW               = 0x03fb, // two bytes for the effective black and white values
    kPhotoshopResourceID_ObsoletePhotoshopTag1     = 0x03fc, // [obsolete]
    kPhotoshopResourceID_EPSOptions                = 0x03fd, // Encapsulated Postscript options
    kPhotoshopResourceID_QuickMaskInfo             = 0x03fe, // Quick Mask information. 2 bytes containing Quick Mask channel ID,  1 byte boolean indicating whether the mask was initially empty.
    kPhotoshopResourceID_ObsoletePhotoshopTag2     = 0x03ff, // [obsolete]
    kPhotoshopResourceID_LayerStateInfo            = 0x0400, // index of target layer (0 means bottom)
    kPhotoshopResourceID_WorkingPathInfo           = 0x0401, // should not be saved to the file
    kPhotoshopResourceID_LayersGroupInfo           = 0x0402, // for grouping layers together
    kPhotoshopResourceID_ObsoletePhotoshopTag3     = 0x0403, // [obsolete] ??
    kPhotoshopResourceID_IPTC_NAA                  = 0x0404, // IPTC/NAA data
    kPhotoshopResourceID_RawImageMode              = 0x0405, // image mode for raw format files
    kPhotoshopResourceID_JPEGQuality               = 0x0406, // [private]
    kPhotoshopResourceID_GridGuidesInfo            = 0x0408, // see additional Adobe information
    kPhotoshopResourceID_ThumbnailResource         = 0x0409, // see additional Adobe information
    kPhotoshopResourceID_CopyrightFlag             = 0x040a, // true if image is copyrighted
    kPhotoshopResourceID_URL                       = 0x040b, // text string with a resource locator
    kPhotoshopResourceID_ThumbnailResource2        = 0x040c, // see additional Adobe information
    kPhotoshopResourceID_GlobalAngle               = 0x040d, // global lighting angle for effects layer
    kPhotoshopResourceID_ColorSamplersResource     = 0x040e, // see additional Adobe information
    kPhotoshopResourceID_ICCProfile                = 0x040f, // see notes from Internat. Color Consortium
    kPhotoshopResourceID_Watermark                 = 0x0410, // one byte
    kPhotoshopResourceID_ICCUntagged               = 0x0411, // 1 means intentionally untagged
    kPhotoshopResourceID_EffectsVisible            = 0x0412, // 1 byte to show/hide all effects layers
    kPhotoshopResourceID_SpotHalftone              = 0x0413, // version, length and data
    kPhotoshopResourceID_IDsBaseValue              = 0x0414, // base value for new layers ID's
    kPhotoshopResourceID_UnicodeAlphaNames         = 0x0415, // length plus Unicode string
    kPhotoshopResourceID_IndexedColourTableCount   = 0x0416, // [Photoshop 6.0 and later] 2 bytes
    kPhotoshopResourceID_TransparentIndex          = 0x0417, // [Photoshop 6.0 and later] 2 bytes
    kPhotoshopResourceID_GlobalAltitude            = 0x0419, // [Photoshop 6.0 and later] 4 bytes
    kPhotoshopResourceID_Slices                    = 0x041a, // [Photoshop 6.0 and later] see additional Adobe info
    kPhotoshopResourceID_WorkflowURL               = 0x041b, // [Photoshop 6.0 and later] 4 bytes length + Unicode string
    kPhotoshopResourceID_JumpToXPEP                = 0x041c, // [Photoshop 6.0 and later] see additional Adobe info
    kPhotoshopResourceID_AlphaIdentifiers          = 0x041d, // [Photoshop 6.0 and later] 4*(n+1) bytes
    kPhotoshopResourceID_URLList                   = 0x041e, // [Photoshop 6.0 and later] structured Unicode URL's
    kPhotoshopResourceID_VersionInfo               = 0x0421, // [Photoshop 6.0 and later] see additional Adobe info
    kPhotoshopResourceID_ExifInfo                  = 0x0422, // [Photoshop 7.0?] Exif metadata
    kPhotoshopResourceID_XMPPacket                 = 0x0424, // [Photoshop 7.0?] XMP packet -- see  http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
    kPhotoshopResourceID_ClippingPathName          = 0x0bb7, // [Photoshop 6.0 and later] name of clipping path
    kPhotoshopResourceID_MorePrintFlags            = 0x2710  // [Photoshop 6.0 and later] Print flags information. 2 bytes version (=1), 1 byte center crop  marks, 1 byte (=0), 4 bytes bleed width value, 2 bytes bleed width  scale.
};

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    PsdImage::PsdImage(BasicIo::AutoPtr io)
        : Image(ImageType::psd, mdExif | mdIptc | mdXmp, io)
    {
    } // PsdImage::PsdImage

    std::string PsdImage::mimeType() const
    {
        return "image/x-photoshop";
    }

    void PsdImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "Photoshop"));
    }

    void PsdImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::PsdImage::readMetadata: Reading Photoshop file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isPsdType(*io_, false))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "Photoshop");
        }
        clearMetadata();

        /*
          The Photoshop header goes as follows -- all numbers are in big-endian byte order:

          offset  length   name       description
          ======  =======  =========  =========
           0      4 bytes  signature  always '8BPS'
           4      2 bytes  version    always equal to 1
           6      6 bytes  reserved   must be zero
          12      2 bytes  channels   number of channels in the image, including alpha channels (1 to 24)
          14      4 bytes  rows       the height of the image in pixels
          18      4 bytes  columns    the width of the image in pixels
          22      2 bytes  depth      the number of bits per channel
          24      2 bytes  mode       the color mode of the file; Supported values are: Bitmap=0; Grayscale=1; Indexed=2; RGB=3; CMYK=4; Multichannel=7; Duotone=8; Lab=9
        */
        byte buf[26];
        if (io_->read(buf, 26) != 26)
        {
            throw Error(3, "Photoshop");
        }
        pixelWidth_ = getLong(buf + 18, bigEndian);
        pixelHeight_ = getLong(buf + 14, bigEndian);

        // immediately following the image header is the color mode data section,
        // the first four bytes of which specify the byte size of the whole section
        if (io_->read(buf, 4) != 4)
        {
            throw Error(3, "Photoshop");
        }

        // skip it
        uint32_t colorDataLength = getULong(buf, bigEndian);
        if (io_->seek(colorDataLength, BasicIo::cur))
        {
            throw Error(3, "Photoshop");
        }

        // after the color data section, comes a list of resource blocks, preceeded by the total byte size
        if (io_->read(buf, 4) != 4)
        {
            throw Error(3, "Photoshop");
        }
        uint32_t resourcesLength = getULong(buf, bigEndian);
        while (resourcesLength > 0)
        {
            if (io_->read(buf, 8) != 8)
            {
                throw Error(3, "Photoshop");
            }

            if (!Photoshop::isIrb(buf, 4))
            {
                break; // bad resource type
            }
            uint16_t resourceId = getUShort(buf + 4, bigEndian);
            uint32_t resourceNameLength = buf[6] & ~1;

            // skip the resource name, plus any padding
            io_->seek(resourceNameLength, BasicIo::cur);

            // read resource size
            if (io_->read(buf, 4) != 4)
            {
                throw Error(3, "Photoshop");
            }
            uint32_t resourceSize = getULong(buf, bigEndian);
            uint32_t curOffset = io_->tell();

#ifdef DEBUG
        std::cerr << std::hex << "resourceId: " << resourceId << std::dec << " length: " << resourceSize << std::hex << "\n";
#endif

            readResourceBlock(resourceId, resourceSize);
            resourceSize = (resourceSize + 1) & ~1;        // pad to even
            io_->seek(curOffset + resourceSize, BasicIo::beg);
            resourcesLength -= (12 + resourceNameLength + resourceSize);
        }

    } // PsdImage::readMetadata

    void PsdImage::readResourceBlock(uint16_t resourceId, uint32_t resourceSize)
    {
        switch(resourceId)
        {
            case kPhotoshopResourceID_IPTC_NAA:
            {
                DataBuf rawIPTC(resourceSize);
                io_->read(rawIPTC.pData_, rawIPTC.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                if (IptcParser::decode(iptcData_, rawIPTC.pData_, rawIPTC.size_)) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
                    iptcData_.clear();
                }
                break;
            }

            case kPhotoshopResourceID_ExifInfo:
            {
                DataBuf rawExif(resourceSize);
                io_->read(rawExif.pData_, rawExif.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                ByteOrder bo = ExifParser::decode(exifData_, rawExif.pData_, rawExif.size_);
                setByteOrder(bo);
                if (rawExif.size_ > 0 && byteOrder() == invalidByteOrder) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
                    exifData_.clear();
                }
                break;
            }

            case kPhotoshopResourceID_XMPPacket:
            {
                DataBuf xmpPacket(resourceSize);
                io_->read(xmpPacket.pData_, xmpPacket.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                xmpPacket_.assign(reinterpret_cast<char *>(xmpPacket.pData_), xmpPacket.size_);
                if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
                }
                break;
            }

            // - PS 4.0 preview data is fetched from ThumbnailResource
            // - PS >= 5.0 preview data is fetched from ThumbnailResource2
            case kPhotoshopResourceID_ThumbnailResource:
            case kPhotoshopResourceID_ThumbnailResource2:
            {
                /*
                  Photoshop thumbnail resource header

                  offset  length    name            description
                  ======  ========  ====            ===========
                   0      4 bytes   format          = 1 (kJpegRGB). Also supports kRawRGB (0).
                   4      4 bytes   width           Width of thumbnail in pixels.
                   8      4 bytes   height          Height of thumbnail in pixels.
                  12      4 bytes   widthbytes      Padded row bytes as (width * bitspixel + 31) / 32 * 4.
                  16      4 bytes   size            Total size as widthbytes * height * planes
                  20      4 bytes   compressedsize  Size after compression. Used for consistentcy check.
                  24      2 bytes   bitspixel       = 24. Bits per pixel.
                  26      2 bytes   planes          = 1. Number of planes.
                  28      variable  data            JFIF data in RGB format.
                                                    Note: For resource ID 1033 the data is in BGR format.
                */
                byte buf[28];
                if (io_->read(buf, 28) != 28)
                {
                    throw Error(3, "Photoshop");
                }
                NativePreview nativePreview;
                nativePreview.position_ = io_->tell();
                nativePreview.size_ = getLong(buf + 20, bigEndian);    // compressedsize
                nativePreview.width_ = getLong(buf + 4, bigEndian);
                nativePreview.height_ = getLong(buf + 8, bigEndian);
                const uint32_t format = getLong(buf + 0, bigEndian);

                if (nativePreview.size_ > 0 && nativePreview.position_ >= 0) {
                    io_->seek(static_cast<long>(nativePreview.size_), BasicIo::cur);
                    if (io_->error() || io_->eof()) throw Error(14);

                    if (format == 1) {
                        nativePreview.filter_ = "";
                        nativePreview.mimeType_ = "image/jpeg";
                        nativePreviews_.push_back(nativePreview);
                    } else {
                        // unsupported format of native preview
                    }
                }
                break;
            }

            default:
            {
                break;
            }
        }
    } // PsdImage::readResourceBlock

    void PsdImage::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // PsdImage::writeMetadata

    void PsdImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(20);
        if (!outIo.isopen()) throw Error(21);

#ifdef DEBUG
        std::cout << "Exiv2::PsdImage::doWriteMetadata: Writing PSD file " << io_->path() << "\n";
        std::cout << "Exiv2::PsdImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

        // Ensure that this is the correct image type
        if (!isPsdType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(20);
            throw Error(22);
        }

        io_->seek(0, BasicIo::beg);    // rewind

        DataBuf lbuf(4096);
        byte buf[8];

        // Get Photoshop header from original file
        byte psd_head[26];
        if (io_->read(psd_head, 26) != 26) throw Error(3, "Photoshop");

        // Write Photoshop header data out to new PSD file
        if (outIo.write(psd_head, 26) != 26) throw Error(21);

        // Read colorDataLength from original PSD
        if (io_->read(buf, 4) != 4) throw Error(3, "Photoshop");

        uint32_t colorDataLength = getULong(buf, bigEndian);

        // Write colorDataLength
        ul2Data(buf, colorDataLength, bigEndian);
        if (outIo.write(buf, 4) != 4) throw Error(21);
#ifdef DEBUG
        std::cerr << std::dec << "colorDataLength: " << colorDataLength << "\n";
#endif
        // Copy colorData
        uint32_t readTotal = 0;
        long toRead = 0;
        while (readTotal < colorDataLength) {
            toRead =   static_cast<long>(colorDataLength - readTotal) < lbuf.size_
                     ? static_cast<long>(colorDataLength - readTotal) : lbuf.size_;
            if (io_->read(lbuf.pData_, toRead) != toRead) throw Error(3, "Photoshop");
            readTotal += toRead;
            if (outIo.write(lbuf.pData_, toRead) != toRead) throw Error(21);
        }
        if (outIo.error()) throw Error(21);

        uint32_t resLenOffset = io_->tell();  // remember for later update

        // Read length of all resource blocks from original PSD
        if (io_->read(buf, 4) != 4) throw Error(3, "Photoshop");

        uint32_t oldResLength = getULong(buf, bigEndian);
        uint32_t newResLength = 0;

        // Write oldResLength (will be updated later)
        ul2Data(buf, oldResLength, bigEndian);
        if (outIo.write(buf, 4) != 4) throw Error(21);

#ifdef DEBUG
        std::cerr << std::dec << "oldResLength: " << oldResLength << "\n";
#endif

        // Iterate over original resource blocks.
        // Replace or insert IPTC, EXIF and XMP
        // Original resource blocks assumed to be sorted ASC

        bool iptcDone = false;
        bool exifDone = false;
        bool xmpDone = false;
        while (oldResLength > 0) {
            if (io_->read(buf, 8) != 8) throw Error(3, "Photoshop");

            // read resource type and ID
            uint32_t resourceType = getULong(buf, bigEndian);

            if (!Photoshop::isIrb(buf, 4))
            {
                throw Error(3, "Photoshop"); // bad resource type
            }
            uint16_t resourceId = getUShort(buf + 4, bigEndian);
            uint32_t resourceNameLength = buf[6];
            uint32_t adjResourceNameLen = resourceNameLength & ~1;
            unsigned char resourceNameFirstChar = buf[7];

            // read rest of resource name, plus any padding
            DataBuf resName(256);
            if (   io_->read(resName.pData_, adjResourceNameLen)
                != static_cast<long>(adjResourceNameLen)) throw Error(3, "Photoshop");

            // read resource size (actual length w/o padding!)
            if (io_->read(buf, 4) != 4) throw Error(3, "Photoshop");

            uint32_t resourceSize = getULong(buf, bigEndian);
            uint32_t pResourceSize = (resourceSize + 1) & ~1;    // padded resource size
            uint32_t curOffset = io_->tell();

            // Write IPTC_NAA resource block
            if ((resourceId == kPhotoshopResourceID_IPTC_NAA  ||
                 resourceId >  kPhotoshopResourceID_IPTC_NAA) && iptcDone == false) {
                newResLength += writeIptcData(iptcData_, outIo);
                iptcDone = true;
            }

            // Write ExifInfo resource block
            else if ((resourceId == kPhotoshopResourceID_ExifInfo  ||
                      resourceId >  kPhotoshopResourceID_ExifInfo) && exifDone == false) {
                newResLength += writeExifData(exifData_, outIo);
                exifDone = true;
            }

            // Write XMPpacket resource block
            else if ((resourceId == kPhotoshopResourceID_XMPPacket  ||
                      resourceId >  kPhotoshopResourceID_XMPPacket) && xmpDone == false) {
                newResLength += writeXmpData(xmpData_, outIo);
                xmpDone = true;
            }

            // Copy all other resource blocks
            if (   resourceId != kPhotoshopResourceID_IPTC_NAA
                && resourceId != kPhotoshopResourceID_ExifInfo
                && resourceId != kPhotoshopResourceID_XMPPacket) {
#ifdef DEBUG
                std::cerr << std::hex << "copy : resourceType: " << resourceType << "\n";
                std::cerr << std::hex << "copy : resourceId: " << resourceId << "\n";
                std::cerr << std::dec;
#endif
                // Copy resource block to new PSD file
                ul2Data(buf, resourceType, bigEndian);
                if (outIo.write(buf, 4) != 4) throw Error(21);
                us2Data(buf, resourceId, bigEndian);
                if (outIo.write(buf, 2) != 2) throw Error(21);
                // Write resource name as Pascal string
                buf[0] = resourceNameLength & 0x00ff;
                if (outIo.write(buf, 1) != 1) throw Error(21);
                buf[0] = resourceNameFirstChar;
                if (outIo.write(buf, 1) != 1) throw Error(21);
                if (   outIo.write(resName.pData_, adjResourceNameLen)
                    != static_cast<long>(adjResourceNameLen)) throw Error(21);
                ul2Data(buf, resourceSize, bigEndian);
                if (outIo.write(buf, 4) != 4) throw Error(21);

                readTotal = 0;
                toRead = 0;
                while (readTotal < pResourceSize) {
                    toRead =   static_cast<long>(pResourceSize - readTotal) < lbuf.size_
                             ? static_cast<long>(pResourceSize - readTotal) : lbuf.size_;
                    if (io_->read(lbuf.pData_, toRead) != toRead) {
                        throw Error(3, "Photoshop");
                    }
                    readTotal += toRead;
                    if (outIo.write(lbuf.pData_, toRead) != toRead) throw Error(21);
                }
                if (outIo.error()) throw Error(21);
                newResLength += pResourceSize + adjResourceNameLen + 12;
            }

            io_->seek(curOffset + pResourceSize, BasicIo::beg);
            oldResLength -= (12 + adjResourceNameLen + pResourceSize);
        }

        // Append IPTC_NAA resource block, if not yet written
        if (iptcDone == false) {
            newResLength += writeIptcData(iptcData_, outIo);
            iptcDone = true;
        }

        // Append ExifInfo resource block, if not yet written
        if (exifDone == false) {
            newResLength += writeExifData(exifData_, outIo);
            exifDone = true;
        }

        // Append XmpPacket resource block, if not yet written
        if (xmpDone == false) {
            newResLength += writeXmpData(xmpData_, outIo);
            xmpDone = true;
        }

        // Copy remaining data
        long readSize = 0;
        while ((readSize=io_->read(lbuf.pData_, lbuf.size_))) {
            if (outIo.write(lbuf.pData_, readSize) != readSize) throw Error(21);
        }
        if (outIo.error()) throw Error(21);

        // Update length of resources
#ifdef DEBUG
        std::cerr << "newResLength: " << newResLength << "\n";
#endif
        outIo.seek(resLenOffset, BasicIo::beg);
        ul2Data(buf, newResLength, bigEndian);
        if (outIo.write(buf, 4) != 4) throw Error(21);

    } // PsdImage::doWriteMetadata

    uint32_t PsdImage::writeIptcData(const IptcData& iptcData, BasicIo& out) const
    {
        uint32_t resLength = 0;
        byte buf[8];

        if (iptcData.count() > 0) {
            DataBuf rawIptc = IptcParser::encode(iptcData);
            if (rawIptc.size_ > 0) {
#ifdef DEBUG
                std::cerr << std::hex << "write: resourceId: " << kPhotoshopResourceID_IPTC_NAA << "\n";
                std::cerr << std::dec << "Writing IPTC_NAA: size: " << rawIptc.size_ << "\n";
#endif
                if (out.write(reinterpret_cast<const byte*>(Photoshop::irbId_[0]), 4) != 4) throw Error(21);
                us2Data(buf, kPhotoshopResourceID_IPTC_NAA, bigEndian);
                if (out.write(buf, 2) != 2) throw Error(21);
                us2Data(buf, 0, bigEndian);                      // NULL resource name
                if (out.write(buf, 2) != 2) throw Error(21);
                ul2Data(buf, rawIptc.size_, bigEndian);
                if (out.write(buf, 4) != 4) throw Error(21);
                // Write encoded Iptc data
                if (out.write(rawIptc.pData_, rawIptc.size_) != rawIptc.size_) throw Error(21);
                resLength += rawIptc.size_ + 12;
                if (rawIptc.size_ & 1)    // even padding
                {
                    buf[0] = 0;
                    if (out.write(buf, 1) != 1) throw Error(21);
                    resLength++;
                }
            }
        }
        return resLength;
    } // PsdImage::writeIptcData

    uint32_t PsdImage::writeExifData(const ExifData& exifData, BasicIo& out)
    {
        uint32_t resLength = 0;
        byte buf[8];

        if (exifData.count() > 0) {
            Blob blob;
            ByteOrder bo = byteOrder();
            if (bo == invalidByteOrder) {
                bo = littleEndian;
                setByteOrder(bo);
            }
            ExifParser::encode(blob, bo, exifData);

            if (blob.size() > 0) {
#ifdef DEBUG
                std::cerr << std::hex << "write: resourceId: " << kPhotoshopResourceID_ExifInfo << "\n";
                std::cerr << std::dec << "Writing ExifInfo: size: " << blob.size() << "\n";
#endif
                if (out.write(reinterpret_cast<const byte*>(Photoshop::irbId_[0]), 4) != 4) throw Error(21);
                us2Data(buf, kPhotoshopResourceID_ExifInfo, bigEndian);
                if (out.write(buf, 2) != 2) throw Error(21);
                us2Data(buf, 0, bigEndian);                      // NULL resource name
                if (out.write(buf, 2) != 2) throw Error(21);
                ul2Data(buf, static_cast<uint32_t>(blob.size()), bigEndian);
                if (out.write(buf, 4) != 4) throw Error(21);
                // Write encoded Exif data
                if (out.write(&blob[0], static_cast<long>(blob.size())) != static_cast<long>(blob.size())) throw Error(21);
                resLength += static_cast<long>(blob.size()) + 12;
                if (blob.size() & 1)    // even padding
                {
                    buf[0] = 0;
                    if (out.write(buf, 1) != 1) throw Error(21);
                    resLength++;
                }
            }
        }
        return resLength;
    } // PsdImage::writeExifData

    uint32_t PsdImage::writeXmpData(const XmpData& xmpData, BasicIo& out) const
    {
        std::string xmpPacket;
        uint32_t resLength = 0;
        byte buf[8];

#ifdef DEBUG
        std::cerr << "writeXmpFromPacket(): " << writeXmpFromPacket() << "\n";
#endif
//        writeXmpFromPacket(true);
        if (writeXmpFromPacket() == false) {
            if (XmpParser::encode(xmpPacket, xmpData) > 1) {
#ifndef SUPPRESS_WARNINGS
                EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
            }
        }

        if (xmpPacket.size() > 0) {
#ifdef DEBUG
            std::cerr << std::hex << "write: resourceId: " << kPhotoshopResourceID_XMPPacket << "\n";
            std::cerr << std::dec << "Writing XMPPacket: size: " << xmpPacket.size() << "\n";
#endif
            if (out.write(reinterpret_cast<const byte*>(Photoshop::irbId_[0]), 4) != 4) throw Error(21);
            us2Data(buf, kPhotoshopResourceID_XMPPacket, bigEndian);
            if (out.write(buf, 2) != 2) throw Error(21);
            us2Data(buf, 0, bigEndian);                      // NULL resource name
            if (out.write(buf, 2) != 2) throw Error(21);
            ul2Data(buf, static_cast<uint32_t>(xmpPacket.size()), bigEndian);
            if (out.write(buf, 4) != 4) throw Error(21);
            // Write XMPPacket
            if (out.write(reinterpret_cast<const byte*>(xmpPacket.data()), static_cast<long>(xmpPacket.size()))
                != static_cast<long>(xmpPacket.size())) throw Error(21);
            if (out.error()) throw Error(21);
            resLength += static_cast<uint32_t>(xmpPacket.size()) + 12;
            if (xmpPacket.size() & 1)    // even padding
            {
                buf[0] = 0;
                if (out.write(buf, 1) != 1) throw Error(21);
                resLength++;
            }
        }
        return resLength;
    } // PsdImage::writeXmpData

    // *************************************************************************
    // free functions
    Image::AutoPtr newPsdInstance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new PsdImage(io));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isPsdType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 6;
        const unsigned char PsdHeader[6] = { '8', 'B', 'P', 'S', 0, 1 };
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        bool matched = (memcmp(buf, PsdHeader, len) == 0);
        if (!advance || !matched)
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return matched;
    }
}                                       // namespace Exiv2
