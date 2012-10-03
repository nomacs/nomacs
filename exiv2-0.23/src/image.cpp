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
  File:      image.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <brad@robotbattle.com>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
             19-Jul-04, brad: revamped to be more flexible and support Iptc
             15-Jan-05, brad: inside-out design changes
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: image.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"

#include "cr2image.hpp"
#include "crwimage.hpp"
#include "epsimage.hpp"
#include "jpgimage.hpp"
#include "mrwimage.hpp"
#ifdef EXV_HAVE_LIBZ
# include "pngimage.hpp"
#endif // EXV_HAVE_LIBZ
#include "rafimage.hpp"
#include "tiffimage.hpp"
#include "orfimage.hpp"
#include "gifimage.hpp"
#include "psdimage.hpp"
#include "tgaimage.hpp"
#include "bmpimage.hpp"
#include "jp2image.hpp"
#include "rw2image.hpp"
#include "pgfimage.hpp"
#include "xmpsidecar.hpp"

// + standard includes
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
# define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                            // stat
#endif

// *****************************************************************************
namespace {

    using namespace Exiv2;

    //! Struct for storing image types and function pointers.
    struct Registry {
        //! Comparison operator to compare a Registry structure with an image type
        bool operator==(const int& imageType) const { return imageType == imageType_; }

        // DATA
        int            imageType_;
        NewInstanceFct newInstance_;
        IsThisTypeFct  isThisType_;
        AccessMode     exifSupport_;
        AccessMode     iptcSupport_;
        AccessMode     xmpSupport_;
        AccessMode     commentSupport_;
    };

    const Registry registry[] = {
        //image type       creation fct     type check  Exif mode    IPTC mode    XMP mode     Comment mode
        //---------------  ---------------  ----------  -----------  -----------  -----------  ------------
        { ImageType::jpeg, newJpegInstance, isJpegType, amReadWrite, amReadWrite, amReadWrite, amReadWrite },
        { ImageType::exv,  newExvInstance,  isExvType,  amReadWrite, amReadWrite, amReadWrite, amReadWrite },
        { ImageType::cr2,  newCr2Instance,  isCr2Type,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::crw,  newCrwInstance,  isCrwType,  amReadWrite, amNone,      amNone,      amReadWrite },
        { ImageType::mrw,  newMrwInstance,  isMrwType,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::tiff, newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::dng,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::nef,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::pef,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::arw,  newTiffInstance, isTiffType, amRead,      amRead,      amRead,      amNone      },
        { ImageType::rw2,  newRw2Instance,  isRw2Type,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::sr2,  newTiffInstance, isTiffType, amRead,      amRead,      amRead,      amNone      },
        { ImageType::srw,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::orf,  newOrfInstance,  isOrfType,  amReadWrite, amReadWrite, amReadWrite, amNone      },
#ifdef EXV_HAVE_LIBZ
        { ImageType::png,  newPngInstance,  isPngType,  amReadWrite, amReadWrite, amReadWrite, amReadWrite },
#endif // EXV_HAVE_LIBZ
        { ImageType::pgf,  newPgfInstance,  isPgfType,  amReadWrite, amReadWrite, amReadWrite, amReadWrite },
        { ImageType::raf,  newRafInstance,  isRafType,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::eps,  newEpsInstance,  isEpsType,  amNone,      amNone,      amReadWrite, amNone      },
        { ImageType::xmp,  newXmpInstance,  isXmpType,  amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::gif,  newGifInstance,  isGifType,  amNone,      amNone,      amNone,      amNone      },
        { ImageType::psd,  newPsdInstance,  isPsdType,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::tga,  newTgaInstance,  isTgaType,  amNone,      amNone,      amNone,      amNone      },
        { ImageType::bmp,  newBmpInstance,  isBmpType,  amNone,      amNone,      amNone,      amNone      },
        { ImageType::jp2,  newJp2Instance,  isJp2Type,  amReadWrite, amReadWrite, amReadWrite, amNone      },
        // End of list marker
        { ImageType::none, 0,               0,          amNone,      amNone,      amNone,      amNone      }
    };

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Image::Image(int              imageType,
                 uint16_t         supportedMetadata,
                 BasicIo::AutoPtr io)
        : io_(io),
          pixelWidth_(0),
          pixelHeight_(0),
          imageType_(imageType),
          supportedMetadata_(supportedMetadata),
#ifdef EXV_HAVE_XMP_TOOLKIT
          writeXmpFromPacket_(false),
#else
          writeXmpFromPacket_(true),
#endif
          byteOrder_(invalidByteOrder)
    {
    }

    Image::~Image()
    {
    }

    void Image::clearMetadata()
    {
        clearExifData();
        clearIptcData();
        clearXmpPacket();
        clearXmpData();
        clearComment();
    }

    ExifData& Image::exifData()
    {
        return exifData_;
    }

    IptcData& Image::iptcData()
    {
        return iptcData_;
    }

    XmpData& Image::xmpData()
    {
        return xmpData_;
    }

    std::string& Image::xmpPacket()
    {
        return xmpPacket_;
    }

    void Image::setMetadata(const Image& image)
    {
        if (checkMode(mdExif) & amWrite) {
            setExifData(image.exifData());
        }
        if (checkMode(mdIptc) & amWrite) {
            setIptcData(image.iptcData());
        }
        if (checkMode(mdXmp) & amWrite) {
            setXmpPacket(image.xmpPacket());
            setXmpData(image.xmpData());
        }
        if (checkMode(mdComment) & amWrite) {
            setComment(image.comment());
        }
    }

    void Image::clearExifData()
    {
        exifData_.clear();
    }

    void Image::setExifData(const ExifData& exifData)
    {
        exifData_ = exifData;
    }

    void Image::clearIptcData()
    {
        iptcData_.clear();
    }

    void Image::setIptcData(const IptcData& iptcData)
    {
        iptcData_ = iptcData;
    }

    void Image::clearXmpPacket()
    {
        xmpPacket_.clear();
        writeXmpFromPacket(true);
    }

    void Image::setXmpPacket(const std::string& xmpPacket)
    {
        xmpPacket_ = xmpPacket;
        writeXmpFromPacket(true);
    }

    void Image::clearXmpData()
    {
        xmpData_.clear();
        writeXmpFromPacket(false);
    }

    void Image::setXmpData(const XmpData& xmpData)
    {
        xmpData_ = xmpData;
        writeXmpFromPacket(false);
    }

    void Image::writeXmpFromPacket(bool flag)
    {
#ifdef EXV_HAVE_XMP_TOOLKIT
        writeXmpFromPacket_ = flag;
#endif
    }

    void Image::clearComment()
    {
        comment_.erase();
    }

    void Image::setComment(const std::string& comment)
    {
        comment_ = comment;
    }

    void Image::setByteOrder(ByteOrder byteOrder)
    {
        byteOrder_ = byteOrder;
    }

    ByteOrder Image::byteOrder() const
    {
        return byteOrder_;
    }

    int Image::pixelWidth() const
    {
        return pixelWidth_;
    }

    int Image::pixelHeight() const
    {
        return pixelHeight_;
    }

    const ExifData& Image::exifData() const
    {
        return exifData_;
    }

    const IptcData& Image::iptcData() const
    {
        return iptcData_;
    }

    const XmpData& Image::xmpData() const
    {
        return xmpData_;
    }

    std::string Image::comment() const
    {
        return comment_;
    }

    const std::string& Image::xmpPacket() const
    {
        return xmpPacket_;
    }

    BasicIo& Image::io() const
    {
        return *io_;
    }

    bool Image::writeXmpFromPacket() const
    {
        return writeXmpFromPacket_;
    }

    const NativePreviewList& Image::nativePreviews() const
    {
        return nativePreviews_;
    }

    bool Image::good() const
    {
        if (io_->open() != 0) return false;
        IoCloser closer(*io_);
        return ImageFactory::checkType(imageType_, *io_, false);
    }

    bool Image::supportsMetadata(MetadataId metadataId) const
    {
        return (supportedMetadata_ & metadataId) != 0;
    }

    AccessMode Image::checkMode(MetadataId metadataId) const
    {
        return ImageFactory::checkMode(imageType_, metadataId);
    }

    AccessMode ImageFactory::checkMode(int type, MetadataId metadataId)
    {
        const Registry* r = find(registry, type);
        if (!r) throw Error(13, type);
        AccessMode am = amNone;
        switch (metadataId) {
        case mdNone:
            break;
        case mdExif:
            am = r->exifSupport_;
            break;
        case mdIptc:
            am = r->iptcSupport_;
            break;
        case mdXmp:
            am = r->xmpSupport_;
            break;
        case mdComment:
            am = r->commentSupport_;
            break;
        // no default: let the compiler complain
        }
        return am;
    }

    bool ImageFactory::checkType(int type, BasicIo& io, bool advance)
    {
        const Registry* r = find(registry, type);
        if (0 != r) {
            return r->isThisType_(io, advance);
        }
        return false;
    } // ImageFactory::checkType

    int ImageFactory::getType(const std::string& path)
    {
        FileIo fileIo(path);
        return getType(fileIo);
    }

#ifdef EXV_UNICODE_PATH
    int ImageFactory::getType(const std::wstring& wpath)
    {
        FileIo fileIo(wpath);
        return getType(fileIo);
    }

#endif
    int ImageFactory::getType(const byte* data, long size)
    {
        MemIo memIo(data, size);
        return getType(memIo);
    }

    int ImageFactory::getType(BasicIo& io)
    {
        if (io.open() != 0) return ImageType::none;
        IoCloser closer(io);
        for (unsigned int i = 0; registry[i].imageType_ != ImageType::none; ++i) {
            if (registry[i].isThisType_(io, false)) {
                return registry[i].imageType_;
            }
        }
        return ImageType::none;
    } // ImageFactory::getType

    Image::AutoPtr ImageFactory::open(const std::string& path)
    {
        BasicIo::AutoPtr io(new FileIo(path));
        Image::AutoPtr image = open(io); // may throw
        if (image.get() == 0) throw Error(11, path);
        return image;
    }

#ifdef EXV_UNICODE_PATH
    Image::AutoPtr ImageFactory::open(const std::wstring& wpath)
    {
        BasicIo::AutoPtr io(new FileIo(wpath));
        Image::AutoPtr image = open(io); // may throw
        if (image.get() == 0) throw WError(11, wpath);
        return image;
    }

#endif
    Image::AutoPtr ImageFactory::open(const byte* data, long size)
    {
        BasicIo::AutoPtr io(new MemIo(data, size));
        Image::AutoPtr image = open(io); // may throw
        if (image.get() == 0) throw Error(12);
        return image;
    }

    Image::AutoPtr ImageFactory::open(BasicIo::AutoPtr io)
    {
        if (io->open() != 0) {
            throw Error(9, io->path(), strError());
        }
        for (unsigned int i = 0; registry[i].imageType_ != ImageType::none; ++i) {
            if (registry[i].isThisType_(*io, false)) {
                return registry[i].newInstance_(io, false);
            }
        }
        return Image::AutoPtr();
    } // ImageFactory::open

    Image::AutoPtr ImageFactory::create(int type,
                                        const std::string& path)
    {
        std::auto_ptr<FileIo> fileIo(new FileIo(path));
        // Create or overwrite the file, then close it
        if (fileIo->open("w+b") != 0) {
            throw Error(10, path, "w+b", strError());
        }
        fileIo->close();
        BasicIo::AutoPtr io(fileIo);
        Image::AutoPtr image = create(type, io);
        if (image.get() == 0) throw Error(13, type);
        return image;
    }

#ifdef EXV_UNICODE_PATH
    Image::AutoPtr ImageFactory::create(int type,
                                        const std::wstring& wpath)
    {
        std::auto_ptr<FileIo> fileIo(new FileIo(wpath));
        // Create or overwrite the file, then close it
        if (fileIo->open("w+b") != 0) {
            throw WError(10, wpath, "w+b", strError().c_str());
        }
        fileIo->close();
        BasicIo::AutoPtr io(fileIo);
        Image::AutoPtr image = create(type, io);
        if (image.get() == 0) throw Error(13, type);
        return image;
    }

#endif
    Image::AutoPtr ImageFactory::create(int type)
    {
        BasicIo::AutoPtr io(new MemIo);
        Image::AutoPtr image = create(type, io);
        if (image.get() == 0) throw Error(13, type);
        return image;
    }

    Image::AutoPtr ImageFactory::create(int type,
                                        BasicIo::AutoPtr io)
    {
        // BasicIo instance does not need to be open
        const Registry* r = find(registry, type);
        if (0 != r) {
            return r->newInstance_(io, true);
        }
        return Image::AutoPtr();
    } // ImageFactory::create

// *****************************************************************************
// template, inline and free functions

    void append(Blob& blob, const byte* buf, uint32_t len)
    {
        if (len != 0) {
            assert(buf != 0);
            Blob::size_type size = blob.size();
            if (blob.capacity() - size < len) {
                blob.reserve(size + 65536);
            }
            blob.resize(size + len);
            std::memcpy(&blob[size], buf, len);
        }
    } // append

}                                       // namespace Exiv2
