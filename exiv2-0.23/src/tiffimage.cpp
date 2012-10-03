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
  File:      tiffimage.cpp
  Version:   $Rev: 2701 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Mar-06, ahu: created

 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: tiffimage.cpp 2701 2012-04-13 14:08:56Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "makernote_int.hpp"
#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <memory>

/* --------------------------------------------------------------------------

   Todo:

   + CR2 Makernotes don't seem to have a next pointer but Canon Jpeg Makernotes
     do. What a mess. (That'll become an issue when it comes to writing to CR2)
   + Sony makernotes in RAW files do not seem to have header like those in Jpegs.
     And maybe no next pointer either.

   in crwimage.* :

   + Fix CiffHeader according to TiffHeader
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename loadStack to getPath for consistency

   -------------------------------------------------------------------------- */

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    TiffImage::TiffImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::tiff, mdExif | mdIptc | mdXmp, io),
          pixelWidth_(0), pixelHeight_(0)
    {
    } // TiffImage::TiffImage

    //! Structure for TIFF compression to MIME type mappings
    struct MimeTypeList {
        //! Comparison operator for compression
        bool operator==(int compression) const { return compression_ == compression; }
        int compression_;                       //!< TIFF compression
        const char* mimeType_;                  //!< MIME type
    };

    //! List of TIFF compression to MIME type mappings
    MimeTypeList mimeTypeList[] = {
        { 32770, "image/x-samsung-srw" },
        { 34713, "image/x-nikon-nef"   },
        { 65535, "image/x-pentax-pef"  }
    };

    std::string TiffImage::mimeType() const
    {
        if (!mimeType_.empty()) return mimeType_;

        mimeType_ = std::string("image/tiff");
        std::string key = "Exif." + primaryGroup() + ".Compression";
        ExifData::const_iterator md = exifData_.findKey(ExifKey(key));
        if (md != exifData_.end() && md->count() > 0) {
            const MimeTypeList* i = find(mimeTypeList, static_cast<int>(md->toLong()));
            if (i) mimeType_ = std::string(i->mimeType_);
        }
        return mimeType_;
    }

    std::string TiffImage::primaryGroup() const
    {
        if (!primaryGroup_.empty()) return primaryGroup_;

        static const char* keys[] = {
            "Exif.Image.NewSubfileType",
            "Exif.SubImage1.NewSubfileType",
            "Exif.SubImage2.NewSubfileType",
            "Exif.SubImage3.NewSubfileType",
            "Exif.SubImage4.NewSubfileType",
            "Exif.SubImage5.NewSubfileType",
            "Exif.SubImage6.NewSubfileType",
            "Exif.SubImage7.NewSubfileType",
            "Exif.SubImage8.NewSubfileType",
            "Exif.SubImage9.NewSubfileType"
        };
        // Find the group of the primary image, default to "Image"
        primaryGroup_ = std::string("Image");
        for (unsigned int i = 0; i < EXV_COUNTOF(keys); ++i) {
            ExifData::const_iterator md = exifData_.findKey(ExifKey(keys[i]));
            // Is it the primary image?
            if (md != exifData_.end() && md->count() > 0 && md->toLong() == 0) {
                // Sometimes there is a JPEG primary image; that's not our first choice
                primaryGroup_ = md->groupName();
                std::string key = "Exif." + primaryGroup_ + ".JPEGInterchangeFormat";
                if (exifData_.findKey(ExifKey(key)) == exifData_.end()) break;
            }
        }
        return primaryGroup_;
    }

    int TiffImage::pixelWidth() const
    {
        if (pixelWidth_ != 0) return pixelWidth_;

        ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageWidth"));
        ExifData::const_iterator imageWidth = exifData_.findKey(key);
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            pixelWidth_ = static_cast<int>(imageWidth->toLong());
        }
        return pixelWidth_;
    }

    int TiffImage::pixelHeight() const
    {
        if (pixelHeight_ != 0) return pixelHeight_;

        ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageLength"));
        ExifData::const_iterator imageHeight = exifData_.findKey(key);
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            pixelHeight_ = imageHeight->toLong();
        }
        return pixelHeight_;
    }

    void TiffImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "TIFF"));
    }

    void TiffImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading TIFF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) throw Error(9, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isTiffType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "TIFF");
        }
        clearMetadata();
        ByteOrder bo = TiffParser::decode(exifData_,
                                          iptcData_,
                                          xmpData_,
                                          io_->mmap(),
                                          io_->size());
        setByteOrder(bo);
    } // TiffImage::readMetadata

    void TiffImage::writeMetadata()
    {
#ifdef DEBUG
        std::cerr << "Writing TIFF file " << io_->path() << "\n";
#endif
        ByteOrder bo = byteOrder();
        byte* pData = 0;
        long size = 0;
        IoCloser closer(*io_);
        if (io_->open() == 0) {
            // Ensure that this is the correct image type
            if (isTiffType(*io_, false)) {
                pData = io_->mmap(true);
                size = io_->size();
                TiffHeader tiffHeader;
                if (0 == tiffHeader.read(pData, 8)) {
                    bo = tiffHeader.byteOrder();
                }
            }
        }
        if (bo == invalidByteOrder) {
            bo = littleEndian;
        }
        setByteOrder(bo);
        TiffParser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_); // may throw
    } // TiffImage::writeMetadata

    ByteOrder TiffParser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        Tag::root,
                                        TiffMapping::findDecoder);
    } // TiffParser::decode

    WriteMethod TiffParser::encode(
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

        std::auto_ptr<TiffHeaderBase> header(new TiffHeader(byteOrder));
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
    } // TiffParser::encode

    // *************************************************************************
    // free functions
    Image::AutoPtr newTiffInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new TiffImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isTiffType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        TiffHeader tiffHeader;
        bool rc = tiffHeader.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2

// Shortcuts for the newTiffBinaryArray templates.
#define EXV_BINARY_ARRAY(arrayCfg, arrayDef) (newTiffBinaryArray0<&arrayCfg, EXV_COUNTOF(arrayDef), arrayDef>)
#define EXV_SIMPLE_BINARY_ARRAY(arrayCfg) (newTiffBinaryArray1<&arrayCfg>)
#define EXV_COMPLEX_BINARY_ARRAY(arraySet, cfgSelFct) (newTiffBinaryArray2<arraySet, EXV_COUNTOF(arraySet), cfgSelFct>)

namespace Exiv2 {
    namespace Internal {

    //! Constant for non-encrypted binary arrays
    const CryptFct notEncrypted = 0;

    //! Canon Camera Settings binary array - configuration
    extern const ArrayCfg canonCsCfg = {
        canonCsId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // With size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Canon Camera Settings binary array - definition
    extern const ArrayDef canonCsDef[] = {
        { 46, ttUnsignedShort, 3 } // Exif.CanonCs.Lens
    };

    //! Canon Shot Info binary array - configuration
    extern const ArrayCfg canonSiCfg = {
        canonSiId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // With size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon Panorama binary array - configuration
    extern const ArrayCfg canonPaCfg = {
        canonPaId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon Custom Function binary array - configuration
    extern const ArrayCfg canonCfCfg = {
        canonCfId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // With size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon Picture Info binary array - configuration
    extern const ArrayCfg canonPiCfg = {
        canonPiId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon File Info binary array - configuration
    extern const ArrayCfg canonFiCfg = {
        canonFiId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // Has a size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttSignedShort, 1 }
    };
    //! Canon File Info binary array - definition
    extern const ArrayDef canonFiDef[] = {
        { 2, ttUnsignedLong, 1 }
    };

    //! Canon Processing Info binary array - configuration
    extern const ArrayCfg canonPrCfg = {
        canonPrId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // Has a size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttSignedShort, 1 }
    };

    //! Nikon Vibration Reduction binary array - configuration
    extern const ArrayCfg nikonVrCfg = {
        nikonVrId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Vibration Reduction binary array - definition
    extern const ArrayDef nikonVrDef[] = {
        { 0, ttUndefined,     4 }, // Version
        { 7, ttUnsignedByte,  1 }  // The array contains 8 bytes
    };

    //! Nikon Picture Control binary array - configuration
    extern const ArrayCfg nikonPcCfg = {
        nikonPcId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Picture Control binary array - definition
    extern const ArrayDef nikonPcDef[] = {
        {  0, ttUndefined,     4 }, // Version
        {  4, ttAsciiString,  20 },
        { 24, ttAsciiString,  20 },
        { 48, ttUnsignedByte,  1 },
        { 49, ttUnsignedByte,  1 },
        { 50, ttUnsignedByte,  1 },
        { 51, ttUnsignedByte,  1 },
        { 52, ttUnsignedByte,  1 },
        { 53, ttUnsignedByte,  1 },
        { 54, ttUnsignedByte,  1 },
        { 55, ttUnsignedByte,  1 },
        { 56, ttUnsignedByte,  1 },
        { 57, ttUnsignedByte,  1 }  // The array contains 58 bytes
    };

    //! Nikon World Time binary array - configuration
    extern const ArrayCfg nikonWtCfg = {
        nikonWtId,        // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon World Time binary array - definition
    extern const ArrayDef nikonWtDef[] = {
        { 0, ttSignedShort,   1 },
        { 2, ttUnsignedByte,  1 },
        { 3, ttUnsignedByte,  1 }
    };

    //! Nikon ISO info binary array - configuration
    extern const ArrayCfg nikonIiCfg = {
        nikonIiId,        // Group for the elements
        bigEndian,        // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon ISO info binary array - definition
    extern const ArrayDef nikonIiDef[] = {
        {  0, ttUnsignedByte,  1 },
        {  4, ttUnsignedShort, 1 },
        {  6, ttUnsignedByte,  1 },
        { 10, ttUnsignedShort, 1 },
        { 13, ttUnsignedByte,  1 }  // The array contains 14 bytes
    };

    //! Nikon Auto Focus binary array - configuration
    extern const ArrayCfg nikonAfCfg = {
        nikonAfId,        // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Auto Focus binary array - definition
    extern const ArrayDef nikonAfDef[] = {
        { 0, ttUnsignedByte,  1 },
        { 1, ttUnsignedByte,  1 },
        { 2, ttUnsignedShort, 1 } // The array contains 4 bytes
    };

    //! Nikon Auto Focus 2 binary array - configuration
    extern const ArrayCfg nikonAf2Cfg = {
        nikonAf2Id,       // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Auto Focus 2 binary array - definition
    extern const ArrayDef nikonAf2Def[] = {
        {  0, ttUndefined,     4 }, // Version
        {  4, ttUnsignedByte,  1 }, // ContrastDetectAF
        {  5, ttUnsignedByte,  1 }, // AFAreaMode
        {  6, ttUnsignedByte,  1 }, // PhaseDetectAF
        {  7, ttUnsignedByte,  1 }, // PrimaryAFPoint
        {  8, ttUnsignedByte,  7 }, // AFPointsUsed
        { 16, ttUnsignedShort, 1 }, // AFImageWidth
        { 18, ttUnsignedShort, 1 }, // AFImageHeight
        { 20, ttUnsignedShort, 1 }, // AFAreaXPosition
        { 22, ttUnsignedShort, 1 }, // AFAreaYPosition
        { 24, ttUnsignedShort, 1 }, // AFAreaWidth
        { 26, ttUnsignedShort, 1 }, // AFAreaHeight
        { 28, ttUnsignedShort, 1 }, // ContrastDetectAFInFocus
    };
    
    //! Nikon AF Fine Tune binary array - configuration
    extern const ArrayCfg nikonAFTCfg = {
        nikonAFTId,       // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon AF Fine Tune binary array - definition
    extern const ArrayDef nikonAFTDef[] = {
        {  0, ttUnsignedByte,  1 }, // AF Fine Tune on/off
        {  1, ttUnsignedByte,  1 }, // AF Fine Tune index
        {  2, ttUnsignedByte,  1 }  // AF Fine Tune value
    };

    //! Nikon File Info binary array - configuration
    extern const ArrayCfg nikonFiCfg = {
        nikonFiId,        // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon File Info binary array - definition
    extern const ArrayDef nikonFiDef[] = {
        { 0, ttUndefined,     4 }, // Version
        { 6, ttUnsignedShort, 1 }, // Directory Number
        { 8, ttUnsignedShort, 1 }  // File Number
    };

    //! Nikon Multi Exposure binary array - configuration
    extern const ArrayCfg nikonMeCfg = {
        nikonMeId,        // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Multi Exposure binary array - definition
    extern const ArrayDef nikonMeDef[] = {
        {  0, ttUndefined,     4 }, // Version
        {  4, ttUnsignedLong,  1 }, // MultiExposureMode
        {  8, ttUnsignedLong,  1 }, // MultiExposureShots
        { 12, ttUnsignedLong,  1 }  // MultiExposureAutoGain
    };

    //! Nikon Flash Info binary array - configuration 1
    extern const ArrayCfg nikonFl1Cfg = {
        nikonFl1Id,       // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Flash Info binary array - definition 1
    extern const ArrayDef nikonFl1Def[] = {
        {  0, ttUndefined,     4 }, // Version
        {  4, ttUnsignedByte,  1 }, // FlashSource
        {  6, ttUnsignedShort, 1 }, // ExternalFlashFirmware
        {  8, ttUnsignedByte,  1 }, // ExternalFlashFlags
        { 11, ttUnsignedByte,  1 }, // FlashFocalLength
        { 12, ttUnsignedByte,  1 }, // RepeatingFlashRate
        { 13, ttUnsignedByte,  1 }, // RepeatingFlashCount
        { 14, ttUnsignedByte,  1 }, // FlashGNDistance
        { 15, ttUnsignedByte,  1 }, // FlashGroupAControlMode
        { 16, ttUnsignedByte,  1 }  // FlashGroupBControlMode
    };
    //! Nikon Flash Info binary array - configuration 2
    extern const ArrayCfg nikonFl2Cfg = {
        nikonFl2Id,       // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Flash Info binary array - definition 2
    extern const ArrayDef nikonFl2Def[] = {
        {  0, ttUndefined,     4 }, // Version
        {  4, ttUnsignedByte,  1 }, // FlashSource
        {  6, ttUnsignedShort, 1 }, // ExternalFlashFirmware
        {  8, ttUnsignedByte,  1 }, // ExternalFlashFlags
        { 12, ttUnsignedByte,  1 }, // FlashFocalLength
        { 13, ttUnsignedByte,  1 }, // RepeatingFlashRate
        { 14, ttUnsignedByte,  1 }, // RepeatingFlashCount
        { 15, ttUnsignedByte,  1 }, // FlashGNDistance
    };
    //! Nikon Flash Info binary array - configuration 3
    extern const ArrayCfg nikonFl3Cfg = {
        nikonFl3Id,       // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Flash Info binary array - definition
    extern const ArrayDef nikonFl3Def[] = {
        {  0, ttUndefined,     4 }, // Version
        {  4, ttUnsignedByte,  1 }, // FlashSource
        {  6, ttUnsignedShort, 1 }, // ExternalFlashFirmware
        {  8, ttUnsignedByte,  1 }, // ExternalFlashFlags
        { 12, ttUnsignedByte,  1 }, // FlashFocalLength
        { 13, ttUnsignedByte,  1 }, // RepeatingFlashRate
        { 14, ttUnsignedByte,  1 }, // RepeatingFlashCount
        { 15, ttUnsignedByte,  1 }, // FlashGNDistance
        { 16, ttUnsignedByte,  1 }, // FlashColorFilter
    };
    //! Nikon Lens Data configurations and definitions
    extern const ArraySet nikonFlSet[] = {
        { nikonFl1Cfg, nikonFl1Def, EXV_COUNTOF(nikonFl1Def) },
        { nikonFl2Cfg, nikonFl2Def, EXV_COUNTOF(nikonFl2Def) },
        { nikonFl3Cfg, nikonFl3Def, EXV_COUNTOF(nikonFl3Def) }
    };

    //! Nikon Shot Info binary array - configuration 1 (D80)
    extern const ArrayCfg nikonSi1Cfg = {
        nikonSi1Id,       // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 1 (D80)
    extern const ArrayDef nikonSi1Def[] = {
        {    0, ttUndefined,    4 }, // Version
        {  586, ttUnsignedLong, 1 }, // ShutterCount
        { 1155, ttUnsignedByte, 1 }  // The array contains 1156 bytes
    };
    //! Nikon Shot Info binary array - configuration 2 (D40)
    extern const ArrayCfg nikonSi2Cfg = {
        nikonSi2Id,       // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 2 (D40)
    extern const ArrayDef nikonSi2Def[] = {
        {    0, ttUndefined,    4 }, // Version
        {  582, ttUnsignedLong, 1 }, // ShutterCount
        {  738, ttUnsignedByte, 1 },
        { 1112, ttUnsignedByte, 1 }  // The array contains 1113 bytes
    };
    //! Nikon Shot Info binary array - configuration 3 (D300a)
    extern const ArrayCfg nikonSi3Cfg = {
        nikonSi3Id,       // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 3 (D300a)
    extern const ArrayDef nikonSi3Def[] = {
        {    0, ttUndefined,     4 }, // Version
        {  604, ttUnsignedByte,  1 }, // ISO
        {  633, ttUnsignedLong,  1 }, // ShutterCount
        {  721, ttUnsignedShort, 1 }, // AFFineTuneAdj
        {  814, ttUndefined,  4478 }  // The array contains 5291 bytes
    };
    //! Nikon Shot Info binary array - configuration 4 (D300b)
    extern const ArrayCfg nikonSi4Cfg = {
        nikonSi4Id,       // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 4 (D300b)
    extern const ArrayDef nikonSi4Def[] = {
        {    0, ttUndefined,     4 }, // Version
        {  644, ttUnsignedLong,  1 }, // ShutterCount
        {  732, ttUnsignedShort, 1 }, // AFFineTuneAdj
        {  826, ttUndefined,  4478 }  // The array contains 5303 bytes
    };
    //! Nikon Shot Info binary array - configuration 5 (ver 02.xx)
    extern const ArrayCfg nikonSi5Cfg = {
        nikonSi5Id,       // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags (don't know how many)
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 5 (ver 01.xx and ver 02.xx)
    extern const ArrayDef nikonSi5Def[] = {
        {    0, ttUndefined,     4 }, // Version
        {  106, ttUnsignedLong,  1 }, // ShutterCount1
        {  110, ttUnsignedLong,  1 }, // DeletedImageCount
        {  117, ttUnsignedByte,  1 }, // VibrationReduction
        {  130, ttUnsignedByte,  1 }, // VibrationReduction1
        {  343, ttUndefined,     2 }, // ShutterCount
        {  430, ttUnsignedByte,  1 }, // VibrationReduction2
        {  598, ttUnsignedByte,  1 }, // ISO
        {  630, ttUnsignedLong,  1 }  // ShutterCount
    };
    //! Nikon Shot Info binary array - configuration 6 (ver 01.xx)
    extern const ArrayCfg nikonSi6Cfg = {
        nikonSi6Id,       // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        false,            // Write all tags (don't know how many)
        true,             // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data configurations and definitions
    extern const ArraySet nikonSiSet[] = {
        { nikonSi1Cfg, nikonSi1Def, EXV_COUNTOF(nikonSi1Def) },
        { nikonSi2Cfg, nikonSi2Def, EXV_COUNTOF(nikonSi2Def) },
        { nikonSi3Cfg, nikonSi3Def, EXV_COUNTOF(nikonSi3Def) },
        { nikonSi4Cfg, nikonSi4Def, EXV_COUNTOF(nikonSi4Def) },
        { nikonSi5Cfg, nikonSi5Def, EXV_COUNTOF(nikonSi5Def) },
        { nikonSi6Cfg, nikonSi5Def, EXV_COUNTOF(nikonSi5Def) }  // uses nikonSi5Def
    };

    //! Nikon Lens Data binary array - configuration 1
    extern const ArrayCfg nikonLd1Cfg = {
        nikonLd1Id,       // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        true,             // Write all tags
        false,            // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data binary array - configuration 2
    extern const ArrayCfg nikonLd2Cfg = {
        nikonLd2Id,       // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        false,            // Concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data binary array - configuration 3
    extern const ArrayCfg nikonLd3Cfg = {
        nikonLd3Id,       // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        false,            // Don't concatenate gaps
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data binary array - definition
    extern const ArrayDef nikonLdDef[] = {
        { 0, ttUndefined, 4 } // Version
    };
    //! Nikon Lens Data configurations and definitions
    extern const ArraySet nikonLdSet[] = {
        { nikonLd1Cfg, nikonLdDef, EXV_COUNTOF(nikonLdDef) },
        { nikonLd2Cfg, nikonLdDef, EXV_COUNTOF(nikonLdDef) },
        { nikonLd3Cfg, nikonLdDef, EXV_COUNTOF(nikonLdDef) }
    };

    //! Nikon Color Balance binary array - configuration 1
    extern const ArrayCfg nikonCb1Cfg = {
        nikonCb1Id,       // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        false,            // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 2
    extern const ArrayCfg nikonCb2Cfg = {
        nikonCb2Id,       // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 2a
    extern const ArrayCfg nikonCb2aCfg = {
        nikonCb2aId,      // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 2b
    extern const ArrayCfg nikonCb2bCfg = {
        nikonCb2bId,      // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 3
    extern const ArrayCfg nikonCb3Cfg = {
        nikonCb3Id,       // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        false,            // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 4
    extern const ArrayCfg nikonCb4Cfg = {
        nikonCb4Id,       // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - definition 1 (D100)
    extern const ArrayDef nikonCb1Def[] = {
        {  0, ttUndefined,        4 }, // Version
        { 72, ttUnsignedShort,    4 }  // Color balance levels
    };
    //! Nikon Color Balance binary array - definition 2 (D2H)
    extern const ArrayDef nikonCb2Def[] = {
        {  0, ttUndefined,        4 }, // Version
        { 10, ttUnsignedShort,    4 }  // Color balance levels
    };
    //! Nikon Color Balance binary array - definition 2a (D50)
    extern const ArrayDef nikonCb2aDef[] = {
        {  0, ttUndefined,        4 }, // Version
        { 18, ttUnsignedShort,    4 }  // Color balance levels
    };
    //! Nikon Color Balance binary array - definition 2b (D2X=0204,D2Hs=0206,D200=0207,D40=0208)
    extern const ArrayDef nikonCb2bDef[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,  140 }, // Unknown
        {284, ttUnsignedShort,    3 }, // Unknown (encrypted)
        {290, ttUnsignedShort,    4 }  // Color balance levels
    };
    //! Nikon Color Balance binary array - definition 3 (D70)
    extern const ArrayDef nikonCb3Def[] = {
        {  0, ttUndefined,        4 }, // Version
        { 20, ttUnsignedShort,    4 }  // Color balance levels
    };
    //! Nikon Color Balance binary array - definition 4 (D3)
    extern const ArrayDef nikonCb4Def[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,  140 }, // Unknown
        {284, ttUnsignedShort,    5 }, // Unknown (encrypted)
        {294, ttUnsignedShort,    4 }  // Color balance levels
    };
    //! Nikon Color Balance configurations and definitions
    extern const ArraySet nikonCbSet[] = {
        { nikonCb1Cfg,  nikonCb1Def,  EXV_COUNTOF(nikonCb1Def)  },
        { nikonCb2Cfg,  nikonCb2Def,  EXV_COUNTOF(nikonCb2Def)  },
        { nikonCb2aCfg, nikonCb2aDef, EXV_COUNTOF(nikonCb2aDef) },
        { nikonCb2bCfg, nikonCb2bDef, EXV_COUNTOF(nikonCb2bDef) },
        { nikonCb3Cfg,  nikonCb3Def,  EXV_COUNTOF(nikonCb3Def)  },
        { nikonCb4Cfg,  nikonCb4Def,  EXV_COUNTOF(nikonCb4Def)  }
    };

    //! Minolta Camera Settings (old) binary array - configuration
    extern const ArrayCfg minoCsoCfg = {
        minoltaCsOldId,   // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedLong, 1 }
    };

    //! Minolta Camera Settings (new) binary array - configuration
    extern const ArrayCfg minoCsnCfg = {
        minoltaCsNewId,   // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedLong, 1 }
    };

    //! Minolta 7D Camera Settings binary array - configuration
    extern const ArrayCfg minoCs7Cfg = {
        minoltaCs7DId,    // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Minolta 7D Camera Settings binary array - definition
    extern const ArrayDef minoCs7Def[] = {
        {  60, ttSignedShort, 1 }, // Exif.MinoltaCs7D.ExposureCompensation
        { 126, ttSignedShort, 1 }  // Exif.MinoltaCs7D.ColorTemperature
    };

    //! Minolta 5D Camera Settings binary array - configuration
    extern const ArrayCfg minoCs5Cfg = {
        minoltaCs5DId,    // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Minolta 5D Camera Settings binary array - definition
    extern const ArrayDef minoCs5Def[] = {
        { 146, ttSignedShort, 1 } // Exif.MinoltaCs5D.ColorTemperature
    };

    // Todo: Performance of the handling of Sony Camera Settings can be
    //       improved by defining all known array elements in the definitions
    //       sonyCsDef and sonyCs2Def below and enabling the 'concatenate gaps'
    //       setting in all four configurations.

    //! Sony1 Camera Settings binary array - configuration
    extern const ArrayCfg sony1CsCfg = {
        sony1CsId,        // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Sony1 Camera Settings 2 binary array - configuration
    extern const ArrayCfg sony1Cs2Cfg = {
        sony1Cs2Id,       // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Sony[12] Camera Settings binary array - definition
    extern const ArrayDef sonyCsDef[] = {
        {  12, ttSignedShort,   1 }  // Exif.Sony[12]Cs.WhiteBalanceFineTune
    };
    //! Sony2 Camera Settings binary array - configuration
    extern const ArrayCfg sony2CsCfg = {
        sony2CsId,        // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Sony2 Camera Settings 2 binary array - configuration
    extern const ArrayCfg sony2Cs2Cfg = {
        sony2Cs2Id,       // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Sony[12] Camera Settings 2 binary array - definition
    extern const ArrayDef sonyCs2Def[] = {
        {  44, ttUnsignedShort, 1 } // Exif.Sony[12]Cs2.FocusMode
    };
    //! Sony1 Camera Settings configurations and definitions
    extern const ArraySet sony1CsSet[] = {
        { sony1CsCfg,  sonyCsDef,  EXV_COUNTOF(sonyCsDef)  },
        { sony1Cs2Cfg, sonyCs2Def, EXV_COUNTOF(sonyCs2Def) }
    };
    //! Sony2 Camera Settings configurations and definitions
    extern const ArraySet sony2CsSet[] = {
        { sony2CsCfg,  sonyCsDef,  EXV_COUNTOF(sonyCsDef)  },
        { sony2Cs2Cfg, sonyCs2Def, EXV_COUNTOF(sonyCs2Def) }
    };

    //! Sony Minolta Camera Settings (old) binary array - configuration
    extern const ArrayCfg sony1MCsoCfg = {
        sony1MltCsOldId,  // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedLong, 1 }
    };

    //! Sony Minolta Camera Settings (new) binary array - configuration
    extern const ArrayCfg sony1MCsnCfg = {
        sony1MltCsNewId,  // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedLong, 1 }
    };

    //! Sony Minolta 7D Camera Settings binary array - configuration
    extern const ArrayCfg sony1MCs7Cfg = {
        sony1MltCs7DId,   // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };

    //! Sony Minolta A100 Camera Settings binary array - configuration
    extern const ArrayCfg sony1MCsA100Cfg = {
        sony1MltCsA100Id, // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        false,            // Don't concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Sony Minolta A100 Camera Settings binary array - definition
    extern const ArrayDef sony1MCsA100Def[] = {
        { 112, ttSignedShort, 1 }, // Exif.Sony1MltCsA100.WhiteBalanceFineTune
        { 116, ttSignedShort, 1 }, // Exif.Sony1MltCsA100.ColorCompensationFilter
        { 190, ttSignedShort, 1 }  // Exif.Sony1MltCsA100.ColorCompensationFilter2
    };

    //! Samsung PictureWizard binary array - configuration
    extern const ArrayCfg samsungPwCfg = {
        samsungPwId,      // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        true,             // Concatenate gaps
        { 0, ttUnsignedShort, 1 }
    };
    //! Samsung PictureWizard binary array - definition
    extern const ArrayDef samsungPwDef[] = {
        {  0, ttUnsignedShort, 1 }, // Mode
        {  2, ttUnsignedShort, 1 }, // Color
        {  4, ttUnsignedShort, 1 }, // Saturation
        {  6, ttUnsignedShort, 1 }, // Sharpness
        {  8, ttUnsignedShort, 1 }  // Contrast
    };

    /*
      This table lists for each group in a tree, its parent group and tag.
      Root identifies the root of a TIFF tree, as there is a need for multiple
      trees. Groups are the nodes of a TIFF tree. A group is an IFD or any
      other composite component.

      With this table, it is possible, for a given group (and tag) to find a
      path, i.e., a list of groups and tags, from the root to that group (tag).
    */
    const TiffTreeStruct TiffCreator::tiffTreeStruct_[] = {
        // root      group             parent group      parent tag
        //---------  ----------------- ----------------- ----------
        { Tag::root, ifdIdNotSet,      ifdIdNotSet,      Tag::root },
        { Tag::root, ifd0Id,           ifdIdNotSet,      Tag::root },
        { Tag::root, subImage1Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage2Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage3Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage4Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage5Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage6Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage7Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage8Id,      ifd0Id,           0x014a    },
        { Tag::root, subImage9Id,      ifd0Id,           0x014a    },
        { Tag::root, exifId,           ifd0Id,           0x8769    },
        { Tag::root, gpsId,            ifd0Id,           0x8825    },
        { Tag::root, iopId,            exifId,           0xa005    },
        { Tag::root, ifd1Id,           ifd0Id,           Tag::next },
        { Tag::root, ifd2Id,           ifd1Id,           Tag::next },
        { Tag::root, ifd3Id,           ifd2Id,           Tag::next },
        { Tag::root, olympusId,        exifId,           0x927c    },
        { Tag::root, olympus2Id,       exifId,           0x927c    },
        { Tag::root, subThumb1Id,      ifd1Id,           0x014a    },
        { Tag::root, olympusEqId,      olympus2Id,       0x2010    },
        { Tag::root, olympusCsId,      olympus2Id,       0x2020    },
        { Tag::root, olympusRdId,      olympus2Id,       0x2030    },
        { Tag::root, olympusRd2Id,     olympus2Id,       0x2031    },
        { Tag::root, olympusIpId,      olympus2Id,       0x2040    },
        { Tag::root, olympusFiId,      olympus2Id,       0x2050    },
        { Tag::root, olympusFe1Id,     olympus2Id,       0x2100    },
        { Tag::root, olympusFe2Id,     olympus2Id,       0x2200    },
        { Tag::root, olympusFe3Id,     olympus2Id,       0x2300    },
        { Tag::root, olympusFe4Id,     olympus2Id,       0x2400    },
        { Tag::root, olympusFe5Id,     olympus2Id,       0x2500    },
        { Tag::root, olympusFe6Id,     olympus2Id,       0x2600    },
        { Tag::root, olympusFe7Id,     olympus2Id,       0x2700    },
        { Tag::root, olympusFe8Id,     olympus2Id,       0x2800    },
        { Tag::root, olympusFe9Id,     olympus2Id,       0x2900    },
        { Tag::root, olympusRiId,      olympus2Id,       0x3000    },
        { Tag::root, fujiId,           exifId,           0x927c    },
        { Tag::root, canonId,          exifId,           0x927c    },
        { Tag::root, canonCsId,        canonId,          0x0001    },
        { Tag::root, canonSiId,        canonId,          0x0004    },
        { Tag::root, canonPaId,        canonId,          0x0005    },
        { Tag::root, canonCfId,        canonId,          0x000f    },
        { Tag::root, canonPiId,        canonId,          0x0012    },
        { Tag::root, canonFiId,        canonId,          0x0093    },
        { Tag::root, canonPrId,        canonId,          0x00a0    },
        { Tag::root, nikon1Id,         exifId,           0x927c    },
        { Tag::root, nikon2Id,         exifId,           0x927c    },
        { Tag::root, nikon3Id,         exifId,           0x927c    },
        { Tag::root, nikonPvId,        nikon3Id,         0x0011    },
        { Tag::root, nikonVrId,        nikon3Id,         0x001f    },
        { Tag::root, nikonPcId,        nikon3Id,         0x0023    },
        { Tag::root, nikonWtId,        nikon3Id,         0x0024    },
        { Tag::root, nikonIiId,        nikon3Id,         0x0025    },
        { Tag::root, nikonAfId,        nikon3Id,         0x0088    },
        { Tag::root, nikonSi1Id,       nikon3Id,         0x0091    },
        { Tag::root, nikonSi2Id,       nikon3Id,         0x0091    },
        { Tag::root, nikonSi3Id,       nikon3Id,         0x0091    },
        { Tag::root, nikonSi4Id,       nikon3Id,         0x0091    },
        { Tag::root, nikonSi5Id,       nikon3Id,         0x0091    },
        { Tag::root, nikonSi6Id,       nikon3Id,         0x0091    },
        { Tag::root, nikonCb1Id,       nikon3Id,         0x0097    },
        { Tag::root, nikonCb2Id,       nikon3Id,         0x0097    },
        { Tag::root, nikonCb2aId,      nikon3Id,         0x0097    },
        { Tag::root, nikonCb2bId,      nikon3Id,         0x0097    },
        { Tag::root, nikonCb3Id,       nikon3Id,         0x0097    },
        { Tag::root, nikonCb4Id,       nikon3Id,         0x0097    },
        { Tag::root, nikonLd1Id,       nikon3Id,         0x0098    },
        { Tag::root, nikonLd2Id,       nikon3Id,         0x0098    },
        { Tag::root, nikonLd3Id,       nikon3Id,         0x0098    },
        { Tag::root, nikonMeId,        nikon3Id,         0x00b0    },
        { Tag::root, nikonAf2Id,       nikon3Id,         0x00b7    },
        { Tag::root, nikonFiId,        nikon3Id,         0x00b8    },
        { Tag::root, nikonAFTId,       nikon3Id,         0x00b9    },
        { Tag::root, nikonFl1Id,       nikon3Id,         0x00a8    },
        { Tag::root, nikonFl2Id,       nikon3Id,         0x00a8    },
        { Tag::root, nikonFl3Id,       nikon3Id,         0x00a8    },
        { Tag::root, panasonicId,      exifId,           0x927c    },
        { Tag::root, pentaxId,         exifId,           0x927c    },
        { Tag::root, pentaxDngId,      ifd0Id,           0xc634    },
        { Tag::root, samsung2Id,       exifId,           0x927c    },
        { Tag::root, samsungPwId,      samsung2Id,       0x0021    },
        { Tag::root, samsungPvId,      samsung2Id,       0x0035    },
        { Tag::root, sigmaId,          exifId,           0x927c    },
        { Tag::root, sony1Id,          exifId,           0x927c    },
        { Tag::root, sony1CsId,        sony1Id,          0x0114    },
        { Tag::root, sony1Cs2Id,       sony1Id,          0x0114    },
        { Tag::root, sonyMltId,        sony1Id,          0xb028    },
        { Tag::root, sony1MltCsOldId,  sonyMltId,        0x0001    },
        { Tag::root, sony1MltCsNewId,  sonyMltId,        0x0003    },
        { Tag::root, sony1MltCs7DId,   sonyMltId,        0x0004    },
        { Tag::root, sony1MltCsA100Id, sonyMltId,        0x0114    },
        { Tag::root, sony2Id,          exifId,           0x927c    },
        { Tag::root, sony2CsId,        sony2Id,          0x0114    },
        { Tag::root, sony2Cs2Id,       sony2Id,          0x0114    },
        { Tag::root, minoltaId,        exifId,           0x927c    },
        { Tag::root, minoltaCsOldId,   minoltaId,        0x0001    },
        { Tag::root, minoltaCsNewId,   minoltaId,        0x0003    },
        { Tag::root, minoltaCs7DId,    minoltaId,        0x0004    },
        { Tag::root, minoltaCs5DId,    minoltaId,        0x0114    },
        // ---------------------------------------------------------
        // Panasonic RW2 raw images
        { Tag::pana, ifdIdNotSet,      ifdIdNotSet,      Tag::pana },
        { Tag::pana, panaRawId,        ifdIdNotSet,      Tag::pana },
        { Tag::pana, exifId,           panaRawId,        0x8769    },
        { Tag::pana, gpsId,            panaRawId,        0x8825    }
    };

    /*
      This table describes the layout of each known TIFF group (including
      non-standard structures and IFDs only seen in RAW images).

      The key of the table consists of the first two attributes, (extended) tag
      and group. Tag is the TIFF tag or one of a few extended tags, group
      identifies the IFD or any other composite component.

      Each entry of the table defines for a particular tag and group combination
      the corresponding TIFF component create function.
     */
    const TiffGroupStruct TiffCreator::tiffGroupStruct_[] = {
        // ext. tag  group             create function
        //---------  ----------------- -----------------------------------------
        // Root directory
        { Tag::root, ifdIdNotSet,      newTiffDirectory<ifd0Id>                  },

        // IFD0
        {    0x8769, ifd0Id,           newTiffSubIfd<exifId>                     },
        {    0x8825, ifd0Id,           newTiffSubIfd<gpsId>                      },
        {    0x0111, ifd0Id,           newTiffImageData<0x0117, ifd0Id>          },
        {    0x0117, ifd0Id,           newTiffImageSize<0x0111, ifd0Id>          },
        {    0x0144, ifd0Id,           newTiffImageData<0x0145, ifd0Id>          },
        {    0x0145, ifd0Id,           newTiffImageSize<0x0144, ifd0Id>          },
        {    0x0201, ifd0Id,           newTiffImageData<0x0202, ifd0Id>          },
        {    0x0202, ifd0Id,           newTiffImageSize<0x0201, ifd0Id>          },
        {    0x014a, ifd0Id,           newTiffSubIfd<subImage1Id>                },
        {    0xc634, ifd0Id,           newTiffMnEntry                            },
        { Tag::next, ifd0Id,           newTiffDirectory<ifd1Id>                  },
        {  Tag::all, ifd0Id,           newTiffEntry                              },

        // Subdir subImage1
        {    0x0111, subImage1Id,      newTiffImageData<0x0117, subImage1Id>     },
        {    0x0117, subImage1Id,      newTiffImageSize<0x0111, subImage1Id>     },
        {    0x0144, subImage1Id,      newTiffImageData<0x0145, subImage1Id>     },
        {    0x0145, subImage1Id,      newTiffImageSize<0x0144, subImage1Id>     },
        {    0x0201, subImage1Id,      newTiffImageData<0x0202, subImage1Id>     },
        {    0x0202, subImage1Id,      newTiffImageSize<0x0201, subImage1Id>     },
        { Tag::next, subImage1Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage1Id,      newTiffEntry                              },

        // Subdir subImage2
        {    0x0111, subImage2Id,      newTiffImageData<0x0117, subImage2Id>     },
        {    0x0117, subImage2Id,      newTiffImageSize<0x0111, subImage2Id>     },
        {    0x0144, subImage2Id,      newTiffImageData<0x0145, subImage2Id>     },
        {    0x0145, subImage2Id,      newTiffImageSize<0x0144, subImage2Id>     },
        {    0x0201, subImage2Id,      newTiffImageData<0x0202, subImage2Id>     },
        {    0x0202, subImage2Id,      newTiffImageSize<0x0201, subImage2Id>     },
        { Tag::next, subImage2Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage2Id,      newTiffEntry                              },

        // Subdir subImage3
        {    0x0111, subImage3Id,      newTiffImageData<0x0117, subImage3Id>     },
        {    0x0117, subImage3Id,      newTiffImageSize<0x0111, subImage3Id>     },
        {    0x0144, subImage3Id,      newTiffImageData<0x0145, subImage3Id>     },
        {    0x0145, subImage3Id,      newTiffImageSize<0x0144, subImage3Id>     },
        {    0x0201, subImage3Id,      newTiffImageData<0x0202, subImage3Id>     },
        {    0x0202, subImage3Id,      newTiffImageSize<0x0201, subImage3Id>     },
        { Tag::next, subImage3Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage3Id,      newTiffEntry                              },

        // Subdir subImage4
        {    0x0111, subImage4Id,      newTiffImageData<0x0117, subImage4Id>     },
        {    0x0117, subImage4Id,      newTiffImageSize<0x0111, subImage4Id>     },
        {    0x0144, subImage4Id,      newTiffImageData<0x0145, subImage4Id>     },
        {    0x0145, subImage4Id,      newTiffImageSize<0x0144, subImage4Id>     },
        {    0x0201, subImage4Id,      newTiffImageData<0x0202, subImage4Id>     },
        {    0x0202, subImage4Id,      newTiffImageSize<0x0201, subImage4Id>     },
        { Tag::next, subImage4Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage4Id,      newTiffEntry                              },

        // Subdir subImage5
        {    0x0111, subImage5Id,      newTiffImageData<0x0117, subImage5Id>     },
        {    0x0117, subImage5Id,      newTiffImageSize<0x0111, subImage5Id>     },
        {    0x0144, subImage5Id,      newTiffImageData<0x0145, subImage5Id>     },
        {    0x0145, subImage5Id,      newTiffImageSize<0x0144, subImage5Id>     },
        {    0x0201, subImage5Id,      newTiffImageData<0x0202, subImage5Id>     },
        {    0x0202, subImage5Id,      newTiffImageSize<0x0201, subImage5Id>     },
        { Tag::next, subImage5Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage5Id,      newTiffEntry                              },

        // Subdir subImage6
        {    0x0111, subImage6Id,      newTiffImageData<0x0117, subImage6Id>     },
        {    0x0117, subImage6Id,      newTiffImageSize<0x0111, subImage6Id>     },
        {    0x0144, subImage6Id,      newTiffImageData<0x0145, subImage6Id>     },
        {    0x0145, subImage6Id,      newTiffImageSize<0x0144, subImage6Id>     },
        {    0x0201, subImage6Id,      newTiffImageData<0x0202, subImage6Id>     },
        {    0x0202, subImage6Id,      newTiffImageSize<0x0201, subImage6Id>     },
        { Tag::next, subImage6Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage6Id,      newTiffEntry                              },

        // Subdir subImage7
        {    0x0111, subImage7Id,      newTiffImageData<0x0117, subImage7Id>     },
        {    0x0117, subImage7Id,      newTiffImageSize<0x0111, subImage7Id>     },
        {    0x0144, subImage7Id,      newTiffImageData<0x0145, subImage7Id>     },
        {    0x0145, subImage7Id,      newTiffImageSize<0x0144, subImage7Id>     },
        {    0x0201, subImage7Id,      newTiffImageData<0x0202, subImage7Id>     },
        {    0x0202, subImage7Id,      newTiffImageSize<0x0201, subImage7Id>     },
        { Tag::next, subImage7Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage7Id,      newTiffEntry                              },

        // Subdir subImage8
        {    0x0111, subImage8Id,      newTiffImageData<0x0117, subImage8Id>     },
        {    0x0117, subImage8Id,      newTiffImageSize<0x0111, subImage8Id>     },
        {    0x0144, subImage8Id,      newTiffImageData<0x0145, subImage8Id>     },
        {    0x0145, subImage8Id,      newTiffImageSize<0x0144, subImage8Id>     },
        {    0x0201, subImage8Id,      newTiffImageData<0x0202, subImage8Id>     },
        {    0x0202, subImage8Id,      newTiffImageSize<0x0201, subImage8Id>     },
        { Tag::next, subImage8Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage8Id,      newTiffEntry                              },

        // Subdir subImage9
        {    0x0111, subImage9Id,      newTiffImageData<0x0117, subImage9Id>     },
        {    0x0117, subImage9Id,      newTiffImageSize<0x0111, subImage9Id>     },
        {    0x0144, subImage9Id,      newTiffImageData<0x0145, subImage9Id>     },
        {    0x0145, subImage9Id,      newTiffImageSize<0x0144, subImage9Id>     },
        {    0x0201, subImage9Id,      newTiffImageData<0x0202, subImage9Id>     },
        {    0x0202, subImage9Id,      newTiffImageSize<0x0201, subImage9Id>     },
        { Tag::next, subImage9Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subImage9Id,      newTiffEntry                              },

        // Exif subdir
        {    0xa005, exifId,           newTiffSubIfd<iopId>                      },
        {    0x927c, exifId,           newTiffMnEntry                            },
        { Tag::next, exifId,           newTiffDirectory<ignoreId>                },
        {  Tag::all, exifId,           newTiffEntry                              },

        // GPS subdir
        { Tag::next, gpsId,            newTiffDirectory<ignoreId>                },
        {  Tag::all, gpsId,            newTiffEntry                              },

        // IOP subdir
        { Tag::next, iopId,            newTiffDirectory<ignoreId>                },
        {  Tag::all, iopId,            newTiffEntry                              },

        // IFD1
        {    0x0111, ifd1Id,           newTiffThumbData<0x0117, ifd1Id>          },
        {    0x0117, ifd1Id,           newTiffThumbSize<0x0111, ifd1Id>          },
        {    0x0144, ifd1Id,           newTiffImageData<0x0145, ifd1Id>          },
        {    0x0145, ifd1Id,           newTiffImageSize<0x0144, ifd1Id>          },
        {    0x014a, ifd1Id,           newTiffSubIfd<subThumb1Id>                },
        {    0x0201, ifd1Id,           newTiffThumbData<0x0202, ifd1Id>          },
        {    0x0202, ifd1Id,           newTiffThumbSize<0x0201, ifd1Id>          },
        { Tag::next, ifd1Id,           newTiffDirectory<ifd2Id>                  },
        {  Tag::all, ifd1Id,           newTiffEntry                              },

        // Subdir subThumb1
        {    0x0111, subThumb1Id,      newTiffImageData<0x0117, subThumb1Id>     },
        {    0x0117, subThumb1Id,      newTiffImageSize<0x0111, subThumb1Id>     },
        {    0x0144, subThumb1Id,      newTiffImageData<0x0145, subThumb1Id>     },
        {    0x0145, subThumb1Id,      newTiffImageSize<0x0144, subThumb1Id>     },
        {    0x0201, subThumb1Id,      newTiffImageData<0x0202, subThumb1Id>     },
        {    0x0202, subThumb1Id,      newTiffImageSize<0x0201, subThumb1Id>     },
        { Tag::next, subThumb1Id,      newTiffDirectory<ignoreId>                },
        {  Tag::all, subThumb1Id,      newTiffEntry                              },

        // IFD2 (eg, in Pentax PEF and Canon CR2 files)
        {    0x0111, ifd2Id,           newTiffImageData<0x0117, ifd2Id>          },
        {    0x0117, ifd2Id,           newTiffImageSize<0x0111, ifd2Id>          },
        {    0x0144, ifd1Id,           newTiffImageData<0x0145, ifd2Id>          },
        {    0x0145, ifd1Id,           newTiffImageSize<0x0144, ifd2Id>          },
        {    0x0201, ifd2Id,           newTiffImageData<0x0202, ifd2Id>          },
        {    0x0202, ifd2Id,           newTiffImageSize<0x0201, ifd2Id>          },
        { Tag::next, ifd2Id,           newTiffDirectory<ifd3Id>                  },
        {  Tag::all, ifd2Id,           newTiffEntry                              },

        // IFD3 (eg, in Canon CR2 files)
        {    0x0111, ifd3Id,           newTiffImageData<0x0117, ifd3Id>          },
        {    0x0117, ifd3Id,           newTiffImageSize<0x0111, ifd3Id>          },
        {    0x0144, ifd1Id,           newTiffImageData<0x0145, ifd3Id>          },
        {    0x0145, ifd1Id,           newTiffImageSize<0x0144, ifd3Id>          },
        {    0x0201, ifd3Id,           newTiffImageData<0x0202, ifd3Id>          },
        {    0x0202, ifd3Id,           newTiffImageSize<0x0201, ifd3Id>          },
        { Tag::next, ifd3Id,           newTiffDirectory<ignoreId>                },
        {  Tag::all, ifd3Id,           newTiffEntry                              },

        // Olympus makernote - some Olympus cameras use Minolta structures
        // Todo: Adding such tags will not work (maybe result in a Minolta makernote), need separate groups
        {    0x0001, olympusId,        EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)       },
        {    0x0003, olympusId,        EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)       },
        { Tag::next, olympusId,        newTiffDirectory<ignoreId>                },
        {  Tag::all, olympusId,        newTiffEntry                              },

        // Olympus2 makernote
        {    0x0001, olympus2Id,       EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)       },
        {    0x0003, olympus2Id,       EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)       },
        {    0x2010, olympus2Id,       newTiffSubIfd<olympusEqId>                },
        {    0x2020, olympus2Id,       newTiffSubIfd<olympusCsId>                },
        {    0x2030, olympus2Id,       newTiffSubIfd<olympusRdId>                },
        {    0x2031, olympus2Id,       newTiffSubIfd<olympusRd2Id>               },
        {    0x2040, olympus2Id,       newTiffSubIfd<olympusIpId>                },
        {    0x2050, olympus2Id,       newTiffSubIfd<olympusFiId>                },
        {    0x2100, olympus2Id,       newTiffSubIfd<olympusFe1Id>               },
        {    0x2200, olympus2Id,       newTiffSubIfd<olympusFe2Id>               },
        {    0x2300, olympus2Id,       newTiffSubIfd<olympusFe3Id>               },
        {    0x2400, olympus2Id,       newTiffSubIfd<olympusFe4Id>               },
        {    0x2500, olympus2Id,       newTiffSubIfd<olympusFe5Id>               },
        {    0x2600, olympus2Id,       newTiffSubIfd<olympusFe6Id>               },
        {    0x2700, olympus2Id,       newTiffSubIfd<olympusFe7Id>               },
        {    0x2800, olympus2Id,       newTiffSubIfd<olympusFe8Id>               },
        {    0x2900, olympus2Id,       newTiffSubIfd<olympusFe9Id>               },
        {    0x3000, olympus2Id,       newTiffSubIfd<olympusRiId>                },
        { Tag::next, olympus2Id,       newTiffDirectory<ignoreId>                },
        {  Tag::all, olympus2Id,       newTiffEntry                              },

        // Olympus2 equipment subdir
        {  Tag::all, olympusEqId,        newTiffEntry                            },

        // Olympus2 camera settings subdir
        {    0x0101, olympusCsId,        newTiffImageData<0x0102, olympusCsId>   },
        {    0x0102, olympusCsId,        newTiffImageSize<0x0101, olympusCsId>   },
        {  Tag::all, olympusCsId,        newTiffEntry                            },

        // Olympus2 raw development subdir
        {  Tag::all, olympusRdId,        newTiffEntry                            },

        // Olympus2 raw development 2 subdir
        {  Tag::all, olympusRd2Id,       newTiffEntry                            },

        // Olympus2 image processing subdir
        {  Tag::all, olympusIpId,        newTiffEntry                            },

        // Olympus2 focus info subdir
        {  Tag::all, olympusFiId,        newTiffEntry                            },

        // Olympus2 FE 1 subdir
        {  Tag::all, olympusFe1Id,       newTiffEntry                            },

        // Olympus2 FE 2 subdir
        {  Tag::all, olympusFe2Id,       newTiffEntry                            },

        // Olympus2 FE 3 subdir
        {  Tag::all, olympusFe3Id,       newTiffEntry                            },

        // Olympus2 FE 4 subdir
        {  Tag::all, olympusFe4Id,       newTiffEntry                            },

        // Olympus2 FE 5 subdir
        {  Tag::all, olympusFe5Id,       newTiffEntry                            },

        // Olympus2 FE 6 subdir
        {  Tag::all, olympusFe6Id,       newTiffEntry                            },

        // Olympus2 FE 7 subdir
        {  Tag::all, olympusFe7Id,       newTiffEntry                            },

        // Olympus2 FE 8 subdir
        {  Tag::all, olympusFe8Id,       newTiffEntry                            },

        // Olympus2 FE 9 subdir
        {  Tag::all, olympusFe9Id,       newTiffEntry                            },

        // Olympus2 Raw Info subdir
        {  Tag::all, olympusRiId,        newTiffEntry                            },

        // Fujifilm makernote
        { Tag::next, fujiId,           newTiffDirectory<ignoreId>                },
        {  Tag::all, fujiId,           newTiffEntry                              },

        // Canon makernote
        {    0x0001, canonId,          EXV_BINARY_ARRAY(canonCsCfg, canonCsDef)  },
        {    0x0004, canonId,          EXV_SIMPLE_BINARY_ARRAY(canonSiCfg)       },
        {    0x0005, canonId,          EXV_SIMPLE_BINARY_ARRAY(canonPaCfg)       },
        {    0x000f, canonId,          EXV_SIMPLE_BINARY_ARRAY(canonCfCfg)       },
        {    0x0012, canonId,          EXV_SIMPLE_BINARY_ARRAY(canonPiCfg)       },
        {    0x0093, canonId,          EXV_BINARY_ARRAY(canonFiCfg, canonFiDef)  },
        {    0x00a0, canonId,          EXV_SIMPLE_BINARY_ARRAY(canonPrCfg)  },
        { Tag::next, canonId,          newTiffDirectory<ignoreId>                },
        {  Tag::all, canonId,          newTiffEntry                              },

        // Canon makernote composite tags
        {  Tag::all, canonCsId,        newTiffBinaryElement                      },
        {  Tag::all, canonSiId,        newTiffBinaryElement                      },
        {  Tag::all, canonPaId,        newTiffBinaryElement                      },
        {  Tag::all, canonCfId,        newTiffBinaryElement                      },
        {  Tag::all, canonPiId,        newTiffBinaryElement                      },
        {  Tag::all, canonFiId,        newTiffBinaryElement                      },
        {  Tag::all, canonPrId,        newTiffBinaryElement                      },

        // Nikon1 makernote
        { Tag::next, nikon1Id,         newTiffDirectory<ignoreId>                },
        {  Tag::all, nikon1Id,         newTiffEntry                              },

        // Nikon2 makernote
        { Tag::next, nikon2Id,         newTiffDirectory<ignoreId>                },
        {  Tag::all, nikon2Id,         newTiffEntry                              },

        // Nikon3 makernote
        { Tag::next, nikon3Id,         newTiffDirectory<ignoreId>                },
        {    0x0011, nikon3Id,         newTiffSubIfd<nikonPvId>                  },
        {    0x001f, nikon3Id,         EXV_BINARY_ARRAY(nikonVrCfg, nikonVrDef)  },
        {    0x0023, nikon3Id,         EXV_BINARY_ARRAY(nikonPcCfg, nikonPcDef)  },
        {    0x0024, nikon3Id,         EXV_BINARY_ARRAY(nikonWtCfg, nikonWtDef)  },
        {    0x0025, nikon3Id,         EXV_BINARY_ARRAY(nikonIiCfg, nikonIiDef)  },
        {    0x0088, nikon3Id,         EXV_BINARY_ARRAY(nikonAfCfg, nikonAfDef)  },
        {    0x0091, nikon3Id,         EXV_COMPLEX_BINARY_ARRAY(nikonSiSet, nikonSelector) },
        {    0x0097, nikon3Id,         EXV_COMPLEX_BINARY_ARRAY(nikonCbSet, nikonSelector) },
        {    0x0098, nikon3Id,         EXV_COMPLEX_BINARY_ARRAY(nikonLdSet, nikonSelector) },
        {    0x00a8, nikon3Id,         EXV_COMPLEX_BINARY_ARRAY(nikonFlSet, nikonSelector) },
        {    0x00b0, nikon3Id,         EXV_BINARY_ARRAY(nikonMeCfg, nikonMeDef)  },
        {    0x00b7, nikon3Id,         EXV_BINARY_ARRAY(nikonAf2Cfg, nikonAf2Def)},
        {    0x00b8, nikon3Id,         EXV_BINARY_ARRAY(nikonFiCfg, nikonFiDef)  },
        {    0x00b9, nikon3Id,         EXV_BINARY_ARRAY(nikonAFTCfg, nikonAFTDef)  },
        {  Tag::all, nikon3Id,         newTiffEntry                              },

        // Nikon3 makernote preview subdir
        {    0x0201, nikonPvId,        newTiffThumbData<0x0202, nikonPvId>       },
        {    0x0202, nikonPvId,        newTiffThumbSize<0x0201, nikonPvId>       },
        { Tag::next, nikonPvId,        newTiffDirectory<ignoreId>                },
        {  Tag::all, nikonPvId,        newTiffEntry                              },

        // Nikon3 vibration reduction
        {  Tag::all, nikonVrId,        newTiffBinaryElement                      },

        // Nikon3 picture control
        {  Tag::all, nikonPcId,        newTiffBinaryElement                      },

        // Nikon3 world time
        {  Tag::all, nikonWtId,        newTiffBinaryElement                      },

        // Nikon3 ISO info
        {  Tag::all, nikonIiId,        newTiffBinaryElement                      },

        // Nikon3 auto focus
        {  Tag::all, nikonAfId,        newTiffBinaryElement                      },
        
        // Nikon3 auto focus 2
        {  Tag::all, nikonAf2Id,       newTiffBinaryElement                      },
        
        // Nikon3 AF Fine Tune
        {  Tag::all, nikonAFTId,       newTiffBinaryElement                      },
        
        // Nikon3 file info
        {  Tag::all, nikonFiId,        newTiffBinaryElement                      },

        // Nikon3 multi exposure
        {  Tag::all, nikonMeId,        newTiffBinaryElement                      },

        // Nikon3 flash info
        {  Tag::all, nikonFl1Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonFl2Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonFl3Id,       newTiffBinaryElement                      },

        // Nikon3 shot info
        {  Tag::all, nikonSi1Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonSi2Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonSi3Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonSi4Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonSi5Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonSi6Id,       newTiffBinaryElement                      },

        // Nikon3 color balance
        {  Tag::all, nikonCb1Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonCb2Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonCb2aId,      newTiffBinaryElement                      },
        {  Tag::all, nikonCb2bId,      newTiffBinaryElement                      },
        {  Tag::all, nikonCb3Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonCb4Id,       newTiffBinaryElement                      },

        // Nikon3 lens data
        {  Tag::all, nikonLd1Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonLd2Id,       newTiffBinaryElement                      },
        {  Tag::all, nikonLd3Id,       newTiffBinaryElement                      },

        // Panasonic makernote
        { Tag::next, panasonicId,      newTiffDirectory<ignoreId>                },
        {  Tag::all, panasonicId,      newTiffEntry                              },

        // Pentax DNG makernote
        {    0x0003, pentaxDngId,      newTiffThumbSize<0x0004, pentaxDngId>     },
        {    0x0004, pentaxDngId,      newTiffThumbData<0x0003, pentaxDngId>     },
        { Tag::next, pentaxDngId,      newTiffDirectory<ignoreId>                },
        {  Tag::all, pentaxDngId,      newTiffEntry                              },

        // Pentax makernote
        {    0x0003, pentaxId,         newTiffThumbSize<0x0004, pentaxId>        },
        {    0x0004, pentaxId,         newTiffThumbData<0x0003, pentaxId>        },
        { Tag::next, pentaxId,         newTiffDirectory<ignoreId>                },
        {  Tag::all, pentaxId,         newTiffEntry                              },

        // Samsung2 makernote
        {    0x0021, samsung2Id,       EXV_BINARY_ARRAY(samsungPwCfg, samsungPwDef) },
        {    0x0035, samsung2Id,       newTiffSubIfd<samsungPvId>                },
        { Tag::next, samsung2Id,       newTiffDirectory<ignoreId>                },
        {  Tag::all, samsung2Id,       newTiffEntry                              },

        // Samsung PictureWizard binary array
        {  Tag::all, samsungPwId,      newTiffBinaryElement                      },

        // Samsung2 makernote preview subdir
        {    0x0201, samsungPvId,      newTiffThumbData<0x0202, samsungPvId>     },
        {    0x0202, samsungPvId,      newTiffThumbSize<0x0201, samsungPvId>     },
        { Tag::next, samsungPvId,      newTiffDirectory<ignoreId>                },
        {  Tag::all, samsungPvId,      newTiffEntry                              },

        // Sigma/Foveon makernote
        { Tag::next, sigmaId,          newTiffDirectory<ignoreId>                },
        {  Tag::all, sigmaId,          newTiffEntry                              },

        // Sony1 makernote
        {    0x0114, sony1Id,          EXV_COMPLEX_BINARY_ARRAY(sony1CsSet, sonyCsSelector) },
        {    0xb028, sony1Id,          newTiffSubIfd<sonyMltId>                  },
        { Tag::next, sony1Id,          newTiffDirectory<ignoreId>                },
        {  Tag::all, sony1Id,          newTiffEntry                              },

        // Sony1 camera settings
        {  Tag::all, sony1CsId,        newTiffBinaryElement                      },
        {  Tag::all, sony1Cs2Id,       newTiffBinaryElement                      },

        // Sony2 makernote
        {    0x0114, sony2Id,          EXV_COMPLEX_BINARY_ARRAY(sony2CsSet, sonyCsSelector) },
        { Tag::next, sony2Id,          newTiffDirectory<ignoreId>                },
        {  Tag::all, sony2Id,          newTiffEntry                              },

        // Sony2 camera settings
        {  Tag::all, sony2CsId,        newTiffBinaryElement                      },
        {  Tag::all, sony2Cs2Id,       newTiffBinaryElement                      },

        // Sony1 Minolta makernote
        {    0x0001, sonyMltId,        EXV_SIMPLE_BINARY_ARRAY(sony1MCsoCfg)     },
        {    0x0003, sonyMltId,        EXV_SIMPLE_BINARY_ARRAY(sony1MCsnCfg)     },
        {    0x0004, sonyMltId,        EXV_BINARY_ARRAY(sony1MCs7Cfg, minoCs7Def)}, // minoCs7Def [sic]
        {    0x0088, sonyMltId,        newTiffThumbData<0x0089, sonyMltId>       },
        {    0x0089, sonyMltId,        newTiffThumbSize<0x0088, sonyMltId>       },
        {    0x0114, sonyMltId,        EXV_BINARY_ARRAY(sony1MCsA100Cfg, sony1MCsA100Def)},
        { Tag::next, sonyMltId,        newTiffDirectory<ignoreId>                },
        {  Tag::all, sonyMltId,        newTiffEntry                              },

        // Sony1 Minolta makernote composite tags
        {  Tag::all, sony1MltCsOldId,  newTiffBinaryElement                      },
        {  Tag::all, sony1MltCsNewId,  newTiffBinaryElement                      },
        {  Tag::all, sony1MltCs7DId,   newTiffBinaryElement                      },
        {  Tag::all, sony1MltCsA100Id, newTiffBinaryElement                      },

        // Minolta makernote
        {    0x0001, minoltaId,        EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)       },
        {    0x0003, minoltaId,        EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)       },
        {    0x0004, minoltaId,        EXV_BINARY_ARRAY(minoCs7Cfg, minoCs7Def)  },
        {    0x0088, minoltaId,        newTiffThumbData<0x0089, minoltaId>       },
        {    0x0089, minoltaId,        newTiffThumbSize<0x0088, minoltaId>       },
        {    0x0114, minoltaId,        EXV_BINARY_ARRAY(minoCs5Cfg, minoCs5Def)  },
        { Tag::next, minoltaId,        newTiffDirectory<ignoreId>                },
        {  Tag::all, minoltaId,        newTiffEntry                              },

        // Minolta makernote composite tags
        {  Tag::all, minoltaCsOldId,   newTiffBinaryElement                      },
        {  Tag::all, minoltaCsNewId,   newTiffBinaryElement                      },
        {  Tag::all, minoltaCs7DId,    newTiffBinaryElement                      },
        {  Tag::all, minoltaCs5DId,    newTiffBinaryElement                      },

        // -----------------------------------------------------------------------
        // Root directory of Panasonic RAW images
        { Tag::pana, ifdIdNotSet,      newTiffDirectory<panaRawId>               },

        // IFD0 of Panasonic RAW images
        {    0x8769, panaRawId,        newTiffSubIfd<exifId>                     },
        {    0x8825, panaRawId,        newTiffSubIfd<gpsId>                      },
//        {    0x0111, panaRawId,        newTiffImageData<0x0117, panaRawId>       },
//        {    0x0117, panaRawId,        newTiffImageSize<0x0111, panaRawId>       },
        { Tag::next, panaRawId,        newTiffDirectory<ignoreId>                },
        {  Tag::all, panaRawId,        newTiffEntry                              },

        // -----------------------------------------------------------------------
        // Tags which are not de/encoded
        { Tag::next, ignoreId,           newTiffDirectory<ignoreId>              },
        {  Tag::all, ignoreId,           newTiffEntry                            }
    };

    // TIFF mapping table for special decoding and encoding requirements
    const TiffMappingInfo TiffMapping::tiffMappingInfo_[] = {
        { "*",       Tag::all, ignoreId,  0, 0 }, // Do not decode tags with group == ignoreId
        { "*",         0x02bc, ifd0Id,    &TiffDecoder::decodeXmp,          0 /*done before the tree is traversed*/ },
        { "*",         0x83bb, ifd0Id,    &TiffDecoder::decodeIptc,         0 /*done before the tree is traversed*/ },
        { "*",         0x8649, ifd0Id,    &TiffDecoder::decodeIptc,         0 /*done before the tree is traversed*/ }
    };

    DecoderFct TiffMapping::findDecoder(const std::string& make,
                                              uint32_t     extendedTag,
                                              IfdId        group)
    {
        DecoderFct decoderFct = &TiffDecoder::decodeStdTiffEntry;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // This may set decoderFct to 0, meaning that the tag should not be decoded
            decoderFct = td->decoderFct_;
        }
        return decoderFct;
    }

    EncoderFct TiffMapping::findEncoder(
        const std::string& make,
              uint32_t     extendedTag,
              IfdId        group
    )
    {
        EncoderFct encoderFct = 0;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // Returns 0 if no special encoder function is found
            encoderFct = td->encoderFct_;
        }
        return encoderFct;
    }

    bool TiffTreeStruct::operator==(const TiffTreeStruct::Key& key) const
    {
        return key.r_ == root_ && key.g_ == group_;
    }

    TiffComponent::AutoPtr TiffCreator::create(uint32_t extendedTag,
                                               IfdId    group)
    {
        TiffComponent::AutoPtr tc(0);
        uint16_t tag = static_cast<uint16_t>(extendedTag & 0xffff);
        const TiffGroupStruct* ts = find(tiffGroupStruct_,
                                         TiffGroupStruct::Key(extendedTag, group));
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(tag, group);
        }
#ifdef DEBUG
        else {
            if (!ts) {
                std::cerr << "Warning: No TIFF structure entry found for ";
            }
            else {
                std::cerr << "Warning: No TIFF component creator found for ";
            }
            std::cerr << "extended tag 0x" << std::setw(4) << std::setfill('0')
                      << std::hex << std::right << extendedTag
                      << ", group " << groupName(group) << "\n";
        }
#endif
        return tc;
    } // TiffCreator::create

    void TiffCreator::getPath(TiffPath& tiffPath,
                              uint32_t  extendedTag,
                              IfdId     group,
                              uint32_t  root)
    {
        const TiffTreeStruct* ts = 0;
        do {
            tiffPath.push(TiffPathItem(extendedTag, group));
            ts = find(tiffTreeStruct_, TiffTreeStruct::Key(root, group));
            assert(ts != 0);
            extendedTag = ts->parentExtTag_;
            group = ts->parentGroup_;
        } while (!(ts->root_ == root && ts->group_ == ifdIdNotSet));

    } // TiffCreator::getPath

    ByteOrder TiffParserWorker::decode(
              ExifData&          exifData,
              IptcData&          iptcData,
              XmpData&           xmpData,
        const byte*              pData,
              uint32_t           size,
              uint32_t           root,
              FindDecoderFct     findDecoderFct,
              TiffHeaderBase*    pHeader
    )
    {
        // Create standard TIFF header if necessary
        std::auto_ptr<TiffHeaderBase> ph;
        if (!pHeader) {
            ph = std::auto_ptr<TiffHeaderBase>(new TiffHeader);
            pHeader = ph.get();
        }
        TiffComponent::AutoPtr rootDir = parse(pData, size, root, pHeader);
        if (0 != rootDir.get()) {
            TiffDecoder decoder(exifData,
                                iptcData,
                                xmpData,
                                rootDir.get(),
                                findDecoderFct);
            rootDir->accept(decoder);
        }
        return pHeader->byteOrder();

    } // TiffParserWorker::decode

    WriteMethod TiffParserWorker::encode(
              BasicIo&           io,
        const byte*              pData,
              uint32_t           size,
        const ExifData&          exifData,
        const IptcData&          iptcData,
        const XmpData&           xmpData,
              uint32_t           root,
              FindEncoderFct     findEncoderFct,
              TiffHeaderBase*    pHeader,
              OffsetWriter*      pOffsetWriter
    )
    {
        /*
           1) parse the binary image, if one is provided, and
           2) attempt updating the parsed tree in-place ("non-intrusive writing")
           3) else, create a new tree and write a new TIFF structure ("intrusive
              writing"). If there is a parsed tree, it is only used to access the
              image data in this case.
         */
        assert(pHeader);
        assert(pHeader->byteOrder() != invalidByteOrder);
        WriteMethod writeMethod = wmIntrusive;
        TiffComponent::AutoPtr parsedTree = parse(pData, size, root, pHeader);
        PrimaryGroups primaryGroups;
        findPrimaryGroups(primaryGroups, parsedTree.get());
        if (0 != parsedTree.get()) {
            // Attempt to update existing TIFF components based on metadata entries
            TiffEncoder encoder(exifData,
                                iptcData,
                                xmpData,
                                parsedTree.get(),
                                false,
                                &primaryGroups,
                                pHeader,
                                findEncoderFct);
            parsedTree->accept(encoder);
            if (!encoder.dirty()) writeMethod = wmNonIntrusive;
        }
        if (writeMethod == wmIntrusive) {
            TiffComponent::AutoPtr createdTree = TiffCreator::create(root, ifdIdNotSet);
            if (0 != parsedTree.get()) {
                // Copy image tags from the original image to the composite
                TiffCopier copier(createdTree.get(), root, pHeader, &primaryGroups);
                parsedTree->accept(copier);
            }
            // Add entries from metadata to composite
            TiffEncoder encoder(exifData,
                                iptcData,
                                xmpData,
                                createdTree.get(),
                                parsedTree.get() == 0,
                                &primaryGroups,
                                pHeader,
                                findEncoderFct);
            encoder.add(createdTree.get(), parsedTree.get(), root);
            // Write binary representation from the composite tree
            DataBuf header = pHeader->write();
            BasicIo::AutoPtr tempIo(io.temporary()); // may throw
            assert(tempIo.get() != 0);
            IoWrapper ioWrapper(*tempIo, header.pData_, header.size_, pOffsetWriter);
            uint32_t imageIdx(uint32_t(-1));
            createdTree->write(ioWrapper,
                               pHeader->byteOrder(),
                               header.size_,
                               uint32_t(-1),
                               uint32_t(-1),
                               imageIdx);
            if (pOffsetWriter) pOffsetWriter->writeOffsets(*tempIo);
            io.transfer(*tempIo); // may throw
#ifndef SUPPRESS_WARNINGS
            EXV_INFO << "Write strategy: Intrusive\n";
#endif
        }
#ifndef SUPPRESS_WARNINGS
        else {
            EXV_INFO << "Write strategy: Non-intrusive\n";
        }
#endif
        return writeMethod;
    } // TiffParserWorker::encode

    TiffComponent::AutoPtr TiffParserWorker::parse(
        const byte*              pData,
              uint32_t           size,
              uint32_t           root,
              TiffHeaderBase*    pHeader
    )
    {
        if (pData == 0 || size == 0) return TiffComponent::AutoPtr(0);
        if (!pHeader->read(pData, size) || pHeader->offset() >= size) {
            throw Error(3, "TIFF");
        }
        TiffComponent::AutoPtr rootDir = TiffCreator::create(root, ifdIdNotSet);
        if (0 != rootDir.get()) {
            rootDir->setStart(pData + pHeader->offset());
            TiffRwState::AutoPtr state(
                new TiffRwState(pHeader->byteOrder(), 0));
            TiffReader reader(pData, size, rootDir.get(), state);
            rootDir->accept(reader);
            reader.postProcess();
        }
        return rootDir;

    } // TiffParserWorker::parse

    void TiffParserWorker::findPrimaryGroups(PrimaryGroups& primaryGroups,
                                             TiffComponent* pSourceDir)
    {
        if (0 == pSourceDir) return;

        const IfdId imageGroups[] = {
            ifd0Id,
            ifd1Id,
            ifd2Id,
            ifd3Id,
            subImage1Id,
            subImage2Id,
            subImage3Id,
            subImage4Id,
            subImage5Id,
            subImage6Id,
            subImage7Id,
            subImage8Id,
            subImage9Id
        };

        for (unsigned int i = 0; i < EXV_COUNTOF(imageGroups); ++i) {
            TiffFinder finder(0x00fe, imageGroups[i]);
            pSourceDir->accept(finder);
            TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
            if (   te
                && te->pValue()->typeId() == unsignedLong
                && te->pValue()->count() == 1
                && (te->pValue()->toLong() & 1) == 0) {
                primaryGroups.push_back(te->group());
            }
        }

    } // TiffParserWorker::findPrimaryGroups

    TiffHeaderBase::TiffHeaderBase(uint16_t  tag,
                                   uint32_t  size,
                                   ByteOrder byteOrder,
                                   uint32_t  offset)
        : tag_(tag),
          size_(size),
          byteOrder_(byteOrder),
          offset_(offset)
    {
    }

    TiffHeaderBase::~TiffHeaderBase()
    {
    }

    bool TiffHeaderBase::read(const byte* pData, uint32_t size)
    {
        if (!pData || size < 8) return false;

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return false;
        }
        if (tag_ != getUShort(pData + 2, byteOrder_)) return false;
        offset_ = getULong(pData + 4, byteOrder_);

        return true;
    } // TiffHeaderBase::read

    DataBuf TiffHeaderBase::write() const
    {
        DataBuf buf(8);
        switch (byteOrder_) {
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
        us2Data(buf.pData_ + 2, tag_, byteOrder_);
        ul2Data(buf.pData_ + 4, 0x00000008, byteOrder_);
        return buf;
    }

    void TiffHeaderBase::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << _("TIFF header, offset") << " = 0x"
           << std::setw(8) << std::setfill('0') << std::hex << std::right
           << offset_;

        switch (byteOrder_) {
        case littleEndian:     os << ", " << _("little endian encoded"); break;
        case bigEndian:        os << ", " << _("big endian encoded");    break;
        case invalidByteOrder: break;
        }
        os << "\n";
    } // TiffHeaderBase::print

    ByteOrder TiffHeaderBase::byteOrder() const
    {
        return byteOrder_;
    }

    void TiffHeaderBase::setByteOrder(ByteOrder byteOrder)
    {
        byteOrder_ = byteOrder;
    }

    uint32_t TiffHeaderBase::offset() const
    {
        return offset_;
    }

    void TiffHeaderBase::setOffset(uint32_t offset)
    {
        offset_ = offset;
    }

    uint32_t TiffHeaderBase::size() const
    {
        return size_;
    }

    uint16_t TiffHeaderBase::tag() const
    {
        return tag_;
    }

    bool TiffHeaderBase::isImageTag(      uint16_t       /*tag*/,
                                          IfdId          /*group*/,
                                    const PrimaryGroups* /*primaryGroups*/) const
    {
        return false;
    }

    bool isTiffImageTag(uint16_t tag, IfdId group)
    {
        //! List of TIFF image tags
        static const TiffImgTagStruct tiffImageTags[] = {
            { 0x00fe, ifd0Id }, // Exif.Image.NewSubfileType
            { 0x00ff, ifd0Id }, // Exif.Image.SubfileType
            { 0x0100, ifd0Id }, // Exif.Image.ImageWidth
            { 0x0101, ifd0Id }, // Exif.Image.ImageLength
            { 0x0102, ifd0Id }, // Exif.Image.BitsPerSample
            { 0x0103, ifd0Id }, // Exif.Image.Compression
            { 0x0106, ifd0Id }, // Exif.Image.PhotometricInterpretation
            { 0x010a, ifd0Id }, // Exif.Image.FillOrder
            { 0x0111, ifd0Id }, // Exif.Image.StripOffsets
            { 0x0115, ifd0Id }, // Exif.Image.SamplesPerPixel
            { 0x0116, ifd0Id }, // Exif.Image.RowsPerStrip
            { 0x0117, ifd0Id }, // Exif.Image.StripByteCounts
            { 0x011a, ifd0Id }, // Exif.Image.XResolution
            { 0x011b, ifd0Id }, // Exif.Image.YResolution
            { 0x011c, ifd0Id }, // Exif.Image.PlanarConfiguration
            { 0x0122, ifd0Id }, // Exif.Image.GrayResponseUnit
            { 0x0123, ifd0Id }, // Exif.Image.GrayResponseCurve
            { 0x0124, ifd0Id }, // Exif.Image.T4Options
            { 0x0125, ifd0Id }, // Exif.Image.T6Options
            { 0x0128, ifd0Id }, // Exif.Image.ResolutionUnit
            { 0x012d, ifd0Id }, // Exif.Image.TransferFunction
            { 0x013d, ifd0Id }, // Exif.Image.Predictor
            { 0x013e, ifd0Id }, // Exif.Image.WhitePoint
            { 0x013f, ifd0Id }, // Exif.Image.PrimaryChromaticities
            { 0x0140, ifd0Id }, // Exif.Image.ColorMap
            { 0x0141, ifd0Id }, // Exif.Image.HalftoneHints
            { 0x0142, ifd0Id }, // Exif.Image.TileWidth
            { 0x0143, ifd0Id }, // Exif.Image.TileLength
            { 0x0144, ifd0Id }, // Exif.Image.TileOffsets
            { 0x0145, ifd0Id }, // Exif.Image.TileByteCounts
            { 0x014c, ifd0Id }, // Exif.Image.InkSet
            { 0x014d, ifd0Id }, // Exif.Image.InkNames
            { 0x014e, ifd0Id }, // Exif.Image.NumberOfInks
            { 0x0150, ifd0Id }, // Exif.Image.DotRange
            { 0x0151, ifd0Id }, // Exif.Image.TargetPrinter
            { 0x0152, ifd0Id }, // Exif.Image.ExtraSamples
            { 0x0153, ifd0Id }, // Exif.Image.SampleFormat
            { 0x0154, ifd0Id }, // Exif.Image.SMinSampleValue
            { 0x0155, ifd0Id }, // Exif.Image.SMaxSampleValue
            { 0x0156, ifd0Id }, // Exif.Image.TransferRange
            { 0x0157, ifd0Id }, // Exif.Image.ClipPath
            { 0x0158, ifd0Id }, // Exif.Image.XClipPathUnits
            { 0x0159, ifd0Id }, // Exif.Image.YClipPathUnits
            { 0x015a, ifd0Id }, // Exif.Image.Indexed
            { 0x015b, ifd0Id }, // Exif.Image.JPEGTables
            { 0x0200, ifd0Id }, // Exif.Image.JPEGProc
            { 0x0201, ifd0Id }, // Exif.Image.JPEGInterchangeFormat
            { 0x0202, ifd0Id }, // Exif.Image.JPEGInterchangeFormatLength
            { 0x0203, ifd0Id }, // Exif.Image.JPEGRestartInterval
            { 0x0205, ifd0Id }, // Exif.Image.JPEGLosslessPredictors
            { 0x0206, ifd0Id }, // Exif.Image.JPEGPointTransforms
            { 0x0207, ifd0Id }, // Exif.Image.JPEGQTables
            { 0x0208, ifd0Id }, // Exif.Image.JPEGDCTables
            { 0x0209, ifd0Id }, // Exif.Image.JPEGACTables
            { 0x0211, ifd0Id }, // Exif.Image.YCbCrCoefficients
            { 0x0212, ifd0Id }, // Exif.Image.YCbCrSubSampling
            { 0x0213, ifd0Id }, // Exif.Image.YCbCrPositioning
            { 0x0214, ifd0Id }, // Exif.Image.ReferenceBlackWhite
            { 0x828d, ifd0Id }, // Exif.Image.CFARepeatPatternDim
            { 0x828e, ifd0Id }, // Exif.Image.CFAPattern
            { 0x8773, ifd0Id }, // Exif.Image.InterColorProfile
            { 0x8824, ifd0Id }, // Exif.Image.SpectralSensitivity
            { 0x8828, ifd0Id }, // Exif.Image.OECF
            { 0x9102, ifd0Id }, // Exif.Image.CompressedBitsPerPixel
            { 0x9217, ifd0Id }, // Exif.Image.SensingMethod
        };

        // If tag, group is one of the image tags listed above -> bingo!
        if (find(tiffImageTags, TiffImgTagStruct::Key(tag, group))) {
#ifdef DEBUG
            ExifKey key(tag, groupName(group));
            std::cerr << "Image tag: " << key << " (3)\n";
#endif
            return true;
        }
#ifdef DEBUG
        std::cerr << "Not an image tag: " << key << " (4)\n";
#endif
        return false;
    }

    TiffHeader::TiffHeader(ByteOrder byteOrder, uint32_t offset, bool hasImageTags)
        : TiffHeaderBase(42, 8, byteOrder, offset),
          hasImageTags_(hasImageTags)
    {
    }

    TiffHeader::~TiffHeader()
    {
    }

    bool TiffHeader::isImageTag(      uint16_t       tag,
                                      IfdId          group,
                                const PrimaryGroups* pPrimaryGroups) const
    {
        if (!hasImageTags_) {
#ifdef DEBUG
            std::cerr << "No image tags in this image\n";
#endif
            return false;
        }
#ifdef DEBUG
        ExifKey key(tag, groupName(group));
#endif
        // If there are primary groups and none matches group, we're done
        if (   pPrimaryGroups != 0
            && !pPrimaryGroups->empty()
            && std::find(pPrimaryGroups->begin(), pPrimaryGroups->end(), group)
               == pPrimaryGroups->end()) {
#ifdef DEBUG
            std::cerr << "Not an image tag: " << key << " (1)\n";
#endif
            return false;
        }
        // All tags of marked primary groups other than IFD0 are considered
        // image tags. That should take care of NEFs until we know better.
        if (   pPrimaryGroups != 0
            && !pPrimaryGroups->empty()
            && group != ifd0Id) {
#ifdef DEBUG
            ExifKey key(tag, groupName(group));
            std::cerr << "Image tag: " << key << " (2)\n";
#endif
            return true;
        }
        // Finally, if tag, group is one of the TIFF image tags -> bingo!
        return isTiffImageTag(tag, group);
    } // TiffHeader::isImageTag

    void OffsetWriter::setOrigin(OffsetId id, uint32_t origin, ByteOrder byteOrder)
    {
        offsetList_[id] = OffsetData(origin, byteOrder);
    }

    void OffsetWriter::setTarget(OffsetId id, uint32_t target)
    {
        OffsetList::iterator it = offsetList_.find(id);
        if (it != offsetList_.end()) it->second.target_ = target;
    }

    void OffsetWriter::writeOffsets(BasicIo& io) const
    {
        for (OffsetList::const_iterator it = offsetList_.begin(); it != offsetList_.end(); ++it) {
            io.seek(it->second.origin_, BasicIo::beg);
            byte buf[4] = { 0, 0, 0, 0 };
            l2Data(buf, it->second.target_, it->second.byteOrder_);
            io.write(buf, 4);
        }
    }

}}                                       // namespace Internal, Exiv2
