// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2011 Andreas Huggel <ahuggel@gmx.net>
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
  File:      preview.cpp
  Version:   $Rev: 2455 $
  Author(s): Vladimir Nadvornik (vn) <nadvornik@suse.cz>
  History:   18-Sep-08, vn: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: preview.cpp 2455 2011-02-13 14:39:15Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include <string>

#include "preview.hpp"
#include "futils.hpp"

#include "image.hpp"
#include "cr2image.hpp"
#include "tiffimage.hpp"

// *****************************************************************************
namespace {

    using namespace Exiv2;

    /*!
      @brief Compare two preview images by number of pixels, if width and height
             of both lhs and rhs are available or else by size.
             Return true if lhs is smaller than rhs.
     */
    bool cmpPreviewProperties(
        const PreviewProperties& lhs,
        const PreviewProperties& rhs
    )
    {
        uint32_t l = lhs.width_ * lhs.height_;
        uint32_t r = rhs.width_ * rhs.height_;

        return l < r;
    }

    /*!
      Base class for image loaders. Provides virtual methods for reading properties
      and DataBuf.
     */
    class Loader {
    public:
        //! Virtual destructor.
        virtual ~Loader() {}

        //! Loader auto pointer
        typedef std::auto_ptr<Loader> AutoPtr;

        //! Create a Loader subclass for requested id
        static AutoPtr create(PreviewId id, const Image &image);

        //! Check if a preview image with given params exists in the image
        virtual bool valid() const { return valid_; }

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const = 0;

        //! Read preview image dimensions when they are not available directly
        virtual bool readDimensions() { return true; }

        //! A number of image loaders configured in the loaderList_ table
        static PreviewId getNumLoaders();

    protected:
        //! Constructor. Sets all image properies to unknown.
        Loader(PreviewId id, const Image &image);

        //! Functions that creates a loader from given parameters
        typedef AutoPtr (*CreateFunc)(PreviewId id, const Image &image, int parIdx);

        //! Structure to list possible loaders
        struct LoaderList {
            const char *imageMimeType_; //!< Image type for which the loader is valid, 0 matches all images
            CreateFunc create_;         //!< Function that creates particular loader instance
            int parIdx_;                //!< Parameter that is passed into CreateFunc
        };

        //! Table that lists possible loaders.  PreviewId is an index to this table.
        static const LoaderList loaderList_[];

        //! Identifies preview image type
        PreviewId id_;

        //! Source image reference
        const Image &image_;

        //! Preview image width
        uint32_t width_;

        //! Preview image length
        uint32_t height_;

        //! Preview image size in bytes
        uint32_t size_;

        //! True if the source image contains a preview image of given type
        bool valid_;
    };

    //! Loader for Jpeg previews that are not read into ExifData directly
    class LoaderExifJpeg : public Loader {
    public:

        //! Constructor
        LoaderExifJpeg(PreviewId id, const Image &image, int parIdx);

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const;

        //! Read preview image dimensions
        virtual bool readDimensions();

    protected:
        //! Structure that lists offset/size tag pairs
        struct Param {
            const char* offsetKey_;         //!< Offset tag
            const char* sizeKey_;           //!< Size tag
            const char* baseOffsetKey_;     //!< Tag that holds base offset or 0
        };

        //! Table that holds all possible offset/size pairs. parIdx is an index to this table
        static const Param param_[];

        //! Offset value
        uint32_t offset_;
    };

    //! Function to create new LoaderExifJpeg
    Loader::AutoPtr createLoaderExifJpeg(PreviewId id, const Image &image, int parIdx);

    //! Loader for Jpeg previews that are read into ExifData
    class LoaderExifDataJpeg : public Loader {
    public:
        //! Constructor
        LoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx);

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const;

        //! Read preview image dimensions
        virtual bool readDimensions();

    protected:

        //! Structure that lists data/size tag pairs
        struct Param {
            const char* dataKey_; //!< Data tag
            const char* sizeKey_; //!< Size tag
        };

        //! Table that holds all possible data/size pairs. parIdx is an index to this table
        static const Param param_[];

        //! Key that points to the Value that contains the JPEG preview in data area
        ExifKey dataKey_;
    };

    //! Function to create new LoaderExifDataJpeg
    Loader::AutoPtr createLoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx);

    //! Loader for Tiff previews - it can get image data from ExifData or image_.io() as needed
    class LoaderTiff : public Loader {
    public:
        //! Constructor
        LoaderTiff(PreviewId id, const Image &image, int parIdx);

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const;

    protected:
        //! Name of the group that contains the preview image
        const char *group_;

        //! Tag that contains image data. Possible values are "StripOffsets" or "TileOffsets"
        std::string offsetTag_;

        //! Tag that contains data sizes. Possible values are "StripByteCounts" or "TileByteCounts"
        std::string sizeTag_;

        //! Structure that lists preview groups
        struct Param {
            const char* group_; //!< Group name
            const char* checkTag_; //!< Tag to check or NULL
            const char* checkValue_; //!< The preview image is valid only if the checkTag_ has this value
        };

        //! Table that holds all possible groups. parIdx is an index to this table.
        static const Param param_[];

    };

    //! Function to create new LoaderTiff
    Loader::AutoPtr createLoaderTiff(PreviewId id, const Image &image, int parIdx);

// *****************************************************************************
// class member definitions

    const Loader::LoaderList Loader::loaderList_[] = {
        { 0,                   createLoaderExifDataJpeg, 0 },
        { 0,                   createLoaderExifDataJpeg, 1 },
        { 0,                   createLoaderExifDataJpeg, 2 },
        { 0,                   createLoaderExifDataJpeg, 3 },
        { 0,                   createLoaderExifDataJpeg, 4 },
        { 0,                   createLoaderExifDataJpeg, 5 },
        { 0,                   createLoaderExifDataJpeg, 6 },
        { 0,                   createLoaderExifDataJpeg, 7 },
        { "image/x-panasonic-rw2", createLoaderExifDataJpeg, 8 },
        { 0,                   createLoaderExifDataJpeg, 9 },
        { 0,                   createLoaderTiff,         0 },
        { 0,                   createLoaderTiff,         1 },
        { 0,                   createLoaderTiff,         2 },
        { 0,                   createLoaderTiff,         3 },
        { 0,                   createLoaderTiff,         4 },
        { 0,                   createLoaderTiff,         5 },
        { 0,                   createLoaderTiff,         6 },
        { 0,                   createLoaderExifJpeg,     0 },
        { 0,                   createLoaderExifJpeg,     1 },
        { 0,                   createLoaderExifJpeg,     2 },
        { 0,                   createLoaderExifJpeg,     3 },
        { 0,                   createLoaderExifJpeg,     4 },
        { 0,                   createLoaderExifJpeg,     5 },
        { 0,                   createLoaderExifJpeg,     6 },
        { "image/x-canon-cr2", createLoaderExifJpeg,     7 },
        { 0,                   createLoaderExifJpeg,     8 }
    };

    const LoaderExifJpeg::Param LoaderExifJpeg::param_[] = {
        { "Exif.Image.JPEGInterchangeFormat",     "Exif.Image.JPEGInterchangeFormatLength",     0 }, // 0
        { "Exif.SubImage1.JPEGInterchangeFormat", "Exif.SubImage1.JPEGInterchangeFormatLength", 0 }, // 1
        { "Exif.SubImage2.JPEGInterchangeFormat", "Exif.SubImage2.JPEGInterchangeFormatLength", 0 }, // 2
        { "Exif.SubImage3.JPEGInterchangeFormat", "Exif.SubImage3.JPEGInterchangeFormatLength", 0 }, // 3
        { "Exif.SubImage4.JPEGInterchangeFormat", "Exif.SubImage4.JPEGInterchangeFormatLength", 0 }, // 4
        { "Exif.SubThumb1.JPEGInterchangeFormat", "Exif.SubThumb1.JPEGInterchangeFormatLength", 0 }, // 5
        { "Exif.Image2.JPEGInterchangeFormat",    "Exif.Image2.JPEGInterchangeFormatLength",    0 }, // 6
        { "Exif.Image.StripOffsets",              "Exif.Image.StripByteCounts",                 0 }, // 7
        { "Exif.OlympusCs.PreviewImageStart",     "Exif.OlympusCs.PreviewImageLength",          "Exif.MakerNote.Offset"}  // 8
    };

    const LoaderExifDataJpeg::Param LoaderExifDataJpeg::param_[] = {
        { "Exif.Thumbnail.JPEGInterchangeFormat",    "Exif.Thumbnail.JPEGInterchangeFormatLength"    }, // 0
        { "Exif.NikonPreview.JPEGInterchangeFormat", "Exif.NikonPreview.JPEGInterchangeFormatLength" }, // 1
        { "Exif.Pentax.PreviewOffset",               "Exif.Pentax.PreviewLength"                     }, // 2
        { "Exif.Minolta.ThumbnailOffset",            "Exif.Minolta.ThumbnailLength"                  }, // 3
        { "Exif.SonyMinolta.ThumbnailOffset",        "Exif.SonyMinolta.ThumbnailLength"              }, // 4
        { "Exif.Olympus.ThumbnailImage",             0                                               }, // 5
        { "Exif.Olympus2.ThumbnailImage",            0                                               }, // 6
        { "Exif.Minolta.Thumbnail",                  0                                               }, // 7
        { "Exif.PanasonicRaw.PreviewImage",          0                                               }, // 8
        { "Exif.SamsungPreview.JPEGInterchangeFormat", "Exif.SamsungPreview.JPEGInterchangeFormatLength" } // 9
    };

    const LoaderTiff::Param LoaderTiff::param_[] = {
        { "Image",     "Exif.Image.NewSubfileType",     "1" },  // 0
        { "SubImage1", "Exif.SubImage1.NewSubfileType", "1" },  // 1
        { "SubImage2", "Exif.SubImage2.NewSubfileType", "1" },  // 2
        { "SubImage3", "Exif.SubImage3.NewSubfileType", "1" },  // 3
        { "SubImage4", "Exif.SubImage4.NewSubfileType", "1" },  // 4
        { "SubThumb1", "Exif.SubThumb1.NewSubfileType", "1" },  // 5
        { "Thumbnail", 0,                               0   }   // 6
    };

    Loader::AutoPtr Loader::create(PreviewId id, const Image &image)
    {
        if (id < 0 || id >= Loader::getNumLoaders())
            return AutoPtr();

        if (loaderList_[id].imageMimeType_ &&
            std::string(loaderList_[id].imageMimeType_) != std::string(image.mimeType()))
            return AutoPtr();

        AutoPtr loader = loaderList_[id].create_(id, image, loaderList_[id].parIdx_);

        if (loader.get() && !loader->valid()) loader.reset();
        return loader;
    }

    Loader::Loader(PreviewId id, const Image &image)
        : id_(id), image_(image),
          width_(0), height_(0),
          size_(0),
          valid_(false)
    {
    }

    PreviewProperties Loader::getProperties() const
    {
        PreviewProperties prop;
        prop.id_ = id_;
        prop.size_ = size_;
        prop.width_ = width_;
        prop.height_ = height_;
        return prop;
    }

    PreviewId Loader::getNumLoaders()
    {
        return (PreviewId)EXV_COUNTOF(loaderList_);
    }

    LoaderExifJpeg::LoaderExifJpeg(PreviewId id, const Image &image, int parIdx)
        : Loader(id, image)
    {
        offset_ = 0;
        ExifData::const_iterator pos = image_.exifData().findKey(ExifKey(param_[parIdx].offsetKey_));
        if (pos != image_.exifData().end() && pos->count() > 0) {
            offset_ = pos->toLong();
        }

        size_ = 0;
        pos = image_.exifData().findKey(ExifKey(param_[parIdx].sizeKey_));
        if (pos != image_.exifData().end() && pos->count() > 0) {
            size_ = pos->toLong();
        }

        if (offset_ == 0 || size_ == 0) return;

        if (param_[parIdx].baseOffsetKey_) {
            pos = image_.exifData().findKey(ExifKey(param_[parIdx].baseOffsetKey_));
            if (pos != image_.exifData().end() && pos->count() > 0) {
                offset_ += pos->toLong();
            }
        }

        if (offset_ + size_ > static_cast<uint32_t>(image_.io().size())) return;

        valid_ = true;
    }

    Loader::AutoPtr createLoaderExifJpeg(PreviewId id, const Image &image, int parIdx)
    {
        return Loader::AutoPtr(new LoaderExifJpeg(id, image, parIdx));
    }

    PreviewProperties LoaderExifJpeg::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.mimeType_ = "image/jpeg";
        prop.extension_ = ".jpg";
#ifdef EXV_UNICODE_PATH
        prop.wextension_ = EXV_WIDEN(".jpg");
#endif
        return prop;
    }

    DataBuf LoaderExifJpeg::getData() const
    {
        if (!valid()) return DataBuf();
        BasicIo &io = image_.io();

        if (io.open() != 0) {
            throw Error(9, io.path(), strError());
        }
        IoCloser closer(io);

        const Exiv2::byte* base = io.mmap();

        return DataBuf(base + offset_, size_);
    }

    bool LoaderExifJpeg::readDimensions()
    {
        if (!valid()) return false;
        if (width_ || height_) return true;

        BasicIo &io = image_.io();

        if (io.open() != 0) {
            throw Error(9, io.path(), strError());
        }
        IoCloser closer(io);
        const Exiv2::byte* base = io.mmap();

        try {
            Image::AutoPtr image = ImageFactory::open(base + offset_, size_);
            if (image.get() == 0) return false;
            image->readMetadata();

            width_ = image->pixelWidth();
            height_ = image->pixelHeight();
        }
        catch (const AnyError& /* error */ ) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Invalid JPEG preview image.\n";
#endif
            return false;
        }

        return true;
    }

    LoaderExifDataJpeg::LoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx)
        : Loader(id, image),
          dataKey_(param_[parIdx].dataKey_)
    {
        ExifData::const_iterator pos = image_.exifData().findKey(dataKey_);
        if (pos != image_.exifData().end()) {
            size_ = pos->sizeDataArea(); // indirect data
            if (size_ == 0 && pos->typeId() == undefined)
                size_ = pos->size(); // direct data
        }

        if (size_ == 0) return;

        valid_ = true;
    }

    Loader::AutoPtr createLoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx)
    {
        return Loader::AutoPtr(new LoaderExifDataJpeg(id, image, parIdx));
    }

    PreviewProperties LoaderExifDataJpeg::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.mimeType_ = "image/jpeg";
        prop.extension_ = ".jpg";
#ifdef EXV_UNICODE_PATH
        prop.wextension_ = EXV_WIDEN(".jpg");
#endif
        return prop;
    }

    DataBuf LoaderExifDataJpeg::getData() const
    {
        if (!valid()) return DataBuf();

        ExifData::const_iterator pos = image_.exifData().findKey(dataKey_);
        if (pos != image_.exifData().end()) {
            DataBuf buf = pos->dataArea(); // indirect data

            if (buf.size_ == 0) { // direct data
                buf = DataBuf(pos->size());
                pos->copy(buf.pData_, invalidByteOrder);
            }

            buf.pData_[0] = 0xff; // fix Minolta thumbnails with invalid jpeg header
            return buf;
        }

        return DataBuf();
    }

    bool LoaderExifDataJpeg::readDimensions()
    {
        if (!valid()) return false;

        DataBuf buf = getData();
        if (buf.size_ == 0) return false;

        try {
            Image::AutoPtr image = ImageFactory::open(buf.pData_, buf.size_);
            if (image.get() == 0) return false;
            image->readMetadata();

            width_ = image->pixelWidth();
            height_ = image->pixelHeight();
        }
        catch (const AnyError& /* error */ ) {
            return false;
        }

        return true;
    }

    LoaderTiff::LoaderTiff(PreviewId id, const Image &image, int parIdx)
        : Loader(id, image),
          group_(param_[parIdx].group_)
    {
        const ExifData &exifData = image_.exifData();

        int offsetCount = 0;
        ExifData::const_iterator pos;

        // check if the group_ contains a preview image
        if (param_[parIdx].checkTag_) {
            pos = exifData.findKey(ExifKey(param_[parIdx].checkTag_));
            if (pos == exifData.end()) return;
            if (param_[parIdx].checkValue_ && pos->toString() != param_[parIdx].checkValue_) return;
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".StripOffsets"));
        if (pos != exifData.end()) {
            offsetTag_ = "StripOffsets";
            sizeTag_ = "StripByteCounts";
            offsetCount = pos->value().count();
        }
        else {
            pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".TileOffsets"));
            if (pos == exifData.end()) return;
            offsetTag_ = "TileOffsets";
            sizeTag_ = "TileByteCounts";
            offsetCount = pos->value().count();
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + '.' + sizeTag_));
        if (pos == exifData.end()) return;
        if (offsetCount != pos->value().count()) return;
        for (int i = 0; i < offsetCount; i++) {
            size_ += pos->toLong(i);
        }

        if (size_ == 0) return;

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".ImageWidth"));
        if (pos != exifData.end() && pos->count() > 0) {
            width_ = pos->toLong();
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".ImageLength"));
        if (pos != exifData.end() && pos->count() > 0) {
            height_ = pos->toLong();
        }

        if (width_ == 0 || height_ == 0) return;

        valid_ = true;
    }

    Loader::AutoPtr createLoaderTiff(PreviewId id, const Image &image, int parIdx)
    {
        return Loader::AutoPtr(new LoaderTiff(id, image, parIdx));
    }

    PreviewProperties LoaderTiff::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.mimeType_ = "image/tiff";
        prop.extension_ = ".tif";
#ifdef EXV_UNICODE_PATH
        prop.wextension_ = EXV_WIDEN(".tif");
#endif
        return prop;
    }

    DataBuf LoaderTiff::getData() const
    {
        const ExifData &exifData = image_.exifData();

        ExifData preview;

        // copy tags
        for (ExifData::const_iterator pos = exifData.begin(); pos != exifData.end(); ++pos) {
            if (pos->groupName() == group_) {

                /*
                   write only the neccessary tags
                   tags that especially could cause problems are:
                   "NewSubfileType" - the result is no longer a thumbnail, it is a standalone image
                   "Orientation" - this tag typically appears only in the "Image" group. Deleting it ensures
                                   consistent result for all previews, including JPEG
                */
                std::string name = pos->tagName();
                if (name != "ImageWidth" &&
                    name != "ImageLength" &&
                    name != "BitsPerSample" &&
                    name != "Compression" &&
                    name != "PhotometricInterpretation" &&
                    name != "StripOffsets" &&
                    name != "SamplesPerPixel" &&
                    name != "RowsPerStrip" &&
                    name != "StripByteCounts" &&
                    name != "XResolution" &&
                    name != "YResolution" &&
                    name != "ResolutionUnit" &&
                    name != "ColorMap" &&
                    name != "TileWidth" &&
                    name != "TileLength" &&
                    name != "TileOffsets" &&
                    name != "TileByteCounts") continue;

                preview.add(ExifKey("Exif.Image." + pos->tagName()), &pos->value());
            }
        }

        Value &dataValue = const_cast<Value&>(preview["Exif.Image." + offsetTag_].value());

        if (dataValue.sizeDataArea() == 0) {
            // image data are not available via exifData, read them from image_.io()
            BasicIo &io = image_.io();

            if (io.open() != 0) {
                throw Error(9, io.path(), strError());
            }
            IoCloser closer(io);

            const Exiv2::byte* base = io.mmap();

            const Value &sizes = preview["Exif.Image." + sizeTag_].value();

            if (sizes.count() == dataValue.count()) {
                if (sizes.count() == 1) {
                    // this saves one copying of the buffer
                    uint32_t offset = dataValue.toLong(0);
                    uint32_t size = sizes.toLong(0);
                    if (offset + size <= static_cast<uint32_t>(io.size()))
                        dataValue.setDataArea(base + offset, size);
                }
                else {
                    // FIXME: the buffer is probably copied twice, it should be optimized
                    DataBuf buf(size_);
                    Exiv2::byte* pos = buf.pData_;
                    for (int i = 0; i < sizes.count(); i++) {
                        uint32_t offset = dataValue.toLong(i);
                        uint32_t size = sizes.toLong(i);
                        if (offset + size <= static_cast<uint32_t>(io.size()))
                            memcpy(pos, base + offset, size);
                        pos += size;
                    }
                    dataValue.setDataArea(buf.pData_, buf.size_);
                }
            }
        }

        // write new image
        MemIo mio;
        IptcData emptyIptc;
        XmpData  emptyXmp;
        TiffParser::encode(mio, 0, 0, Exiv2::littleEndian, preview, emptyIptc, emptyXmp);
        return DataBuf(mio.mmap(), mio.size());
    }

}                                       // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    PreviewImage::PreviewImage(const PreviewProperties& properties, DataBuf data)
        : properties_(properties)
    {
        pData_ = data.pData_;
        size_ = data.size_;
        data.release();
    }

    PreviewImage::~PreviewImage()
    {
        delete[] pData_;
    }

    PreviewImage::PreviewImage(const PreviewImage& rhs)
    {
        properties_ = rhs.properties_;
        pData_ = new byte[rhs.size_];
        memcpy(pData_, rhs.pData_, rhs.size_);
        size_ = rhs.size_;
    }

    PreviewImage& PreviewImage::operator=(const PreviewImage& rhs)
    {
        if (this == &rhs) return *this;
        if (rhs.size_ > size_) {
            delete[] pData_;
            pData_ = new byte[rhs.size_];
        }
        properties_ = rhs.properties_;
        memcpy(pData_, rhs.pData_, rhs.size_);
        size_ = rhs.size_;
        return *this;
    }

    long PreviewImage::writeFile(const std::string& path) const
    {
        std::string name = path + extension();
        // Todo: Creating a DataBuf here unnecessarily copies the memory
        DataBuf buf(pData_, size_);
        return Exiv2::writeFile(buf, name);
    }

#ifdef EXV_UNICODE_PATH
    long PreviewImage::writeFile(const std::wstring& wpath) const
    {
        std::wstring name = wpath + wextension();
        // Todo: Creating a DataBuf here unnecessarily copies the memory
        DataBuf buf(pData_, size_);
        return Exiv2::writeFile(buf, name);
    }

#endif
    DataBuf PreviewImage::copy() const
    {
        return DataBuf(pData_, size_);
    }

    const byte* PreviewImage::pData() const
    {
        return pData_;
    }

    uint32_t PreviewImage::size() const
    {
        return size_;
    }

    std::string PreviewImage::mimeType() const
    {
        return properties_.mimeType_;
    }

    std::string PreviewImage::extension() const
    {
        return properties_.extension_;
    }

#ifdef EXV_UNICODE_PATH
    std::wstring PreviewImage::wextension() const
    {
        return properties_.wextension_;
    }

#endif
    uint32_t PreviewImage::width() const
    {
        return properties_.width_;
    }

    uint32_t PreviewImage::height() const
    {
        return properties_.height_;
    }

    PreviewId PreviewImage::id() const
    {
        return properties_.id_;
    }

    PreviewManager::PreviewManager(const Image& image)
        : image_(image)
    {
    }

    PreviewPropertiesList PreviewManager::getPreviewProperties() const
    {
        PreviewPropertiesList list;
        // go through the loader table and store all successfully created loaders in the list
        for (PreviewId id = 0; id < Loader::getNumLoaders(); ++id) {
            Loader::AutoPtr loader = Loader::create(id, image_);
            if (loader.get() && loader->readDimensions()) {
                list.push_back(loader->getProperties());
            }
        }
        std::sort(list.begin(), list.end(), cmpPreviewProperties);
        return list;
    }

    PreviewImage PreviewManager::getPreviewImage(const PreviewProperties &properties) const
    {
        Loader::AutoPtr loader = Loader::create(properties.id_, image_);
        DataBuf buf;
        if (loader.get()) {
            buf = loader->getData();
        }

        return PreviewImage(properties, buf);
    }
}                                       // namespace Exiv2
