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
  File:      exif.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: exif.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "exif.hpp"
#include "metadatum.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "value.hpp"
#include "types.hpp"
#include "error.hpp"
#include "basicio.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "tiffcomposite_int.hpp" // for Tag::root

// + standard includes
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cstdio>

// *****************************************************************************
namespace {

    //! Unary predicate that matches a Exifdatum with a given key
    class FindExifdatumByKey {
    public:
        //! Constructor, initializes the object with the key to look for
        FindExifdatumByKey(const std::string& key) : key_(key) {}
        /*!
          @brief Returns true if the key of \em exifdatum is equal
                 to that of the object.
        */
        bool operator()(const Exiv2::Exifdatum& exifdatum) const
        {
            return key_ == exifdatum.key();
        }

    private:
        const std::string& key_;

    }; // class FindExifdatumByKey

    /*!
      @brief Exif %Thumbnail image. This abstract base class provides the
             interface for the thumbnail image that is optionally embedded in
             the Exif data. This class is used internally by ExifData, it is
             probably not useful for a client as a standalone class.  Instead,
             use an instance of ExifData to access the Exif thumbnail image.
     */
    class Thumbnail {
    public:
        //! Shortcut for a %Thumbnail auto pointer.
        typedef std::auto_ptr<Thumbnail> AutoPtr;

        //! @name Creators
        //@{
        //! Virtual destructor
        virtual ~Thumbnail() {}
        //@}

        //! Factory function to create a thumbnail for the Exif metadata provided.
        static AutoPtr create(const Exiv2::ExifData& exifData);

        //! @name Accessors
        //@{
        /*!
          @brief Return the thumbnail image in a %DataBuf. The caller owns the
                 data buffer and %DataBuf ensures that it will be deleted.
         */
        virtual Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const =0;
        /*!
          @brief Return the MIME type of the thumbnail ("image/tiff" or
                 "image/jpeg").
         */
        virtual const char* mimeType() const =0;
        /*!
          @brief Return the file extension for the format of the thumbnail
                 (".tif", ".jpg").
         */
        virtual const char* extension() const =0;
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like extension() but returns the extension in a wchar_t.
          @note This function is only available on Windows.
         */
        virtual const wchar_t* wextension() const =0;
#endif
        //@}

    }; // class Thumbnail

    //! Exif thumbnail image in TIFF format
    class TiffThumbnail : public Thumbnail {
    public:
        //! Shortcut for a %TiffThumbnail auto pointer.
        typedef std::auto_ptr<TiffThumbnail> AutoPtr;

        //! @name Manipulators
        //@{
        //! Assignment operator.
        TiffThumbnail& operator=(const TiffThumbnail& rhs);
        //@}

        //! @name Accessors
        //@{
        Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const;
        const char* mimeType() const;
        const char* extension() const;
#ifdef EXV_UNICODE_PATH
        const wchar_t* wextension() const;
#endif
        //@}

    }; // class TiffThumbnail

    //! Exif thumbnail image in JPEG format
    class JpegThumbnail : public Thumbnail {
    public:
        //! Shortcut for a %JpegThumbnail auto pointer.
        typedef std::auto_ptr<JpegThumbnail> AutoPtr;

        //! @name Manipulators
        //@{
        //! Assignment operator.
        JpegThumbnail& operator=(const JpegThumbnail& rhs);
        //@}

        //! @name Accessors
        //@{
        Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const;
        const char* mimeType() const;
        const char* extension() const;
#ifdef EXV_UNICODE_PATH
        const wchar_t* wextension() const;
#endif
        //@}

    }; // class JpegThumbnail

    //! Helper function to sum all components of the value of a metadatum
    long sumToLong(const Exiv2::Exifdatum& md);

    //! Helper function to delete all tags of a specific IFD from the metadata.
    void eraseIfd(Exiv2::ExifData& ed, Exiv2::Internal::IfdId ifdId);

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    /*!
      @brief Set the value of \em exifDatum to \em value. If the object already
             has a value, it is replaced. Otherwise a new ValueType\<T\> value
             is created and set to \em value.

      This is a helper function, called from Exifdatum members. It is meant to
      be used with T = (u)int16_t, (u)int32_t or (U)Rational. Do not use directly.
    */
    template<typename T>
    Exiv2::Exifdatum& setValue(Exiv2::Exifdatum& exifDatum, const T& value)
    {
        std::auto_ptr<Exiv2::ValueType<T> > v
            = std::auto_ptr<Exiv2::ValueType<T> >(new Exiv2::ValueType<T>);
        v->value_.push_back(value);
        exifDatum.value_ = v;
        return exifDatum;
    }

    Exifdatum::Exifdatum(const ExifKey& key, const Value* pValue)
        : key_(key.clone())
    {
        if (pValue) value_ = pValue->clone();
    }

    Exifdatum::~Exifdatum()
    {
    }

    Exifdatum::Exifdatum(const Exifdatum& rhs)
        : Metadatum(rhs)
    {
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
    }

    std::ostream& Exifdatum::write(std::ostream& os, const ExifData* pMetadata) const
    {
        if (value().count() == 0) return os;
        PrintFct fct = printValue;
        const TagInfo* ti = Internal::tagInfo(tag(), static_cast<IfdId>(ifdId()));
        if (ti != 0) fct = ti->printFct_;
        return fct(os, value(), pMetadata);
    }

    const Value& Exifdatum::value() const
    {
        if (value_.get() == 0) throw Error(8);
        return *value_;
    }

    Exifdatum& Exifdatum::operator=(const Exifdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);

        key_.reset();
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy

        value_.reset();
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy

        return *this;
    } // Exifdatum::operator=

    Exifdatum& Exifdatum::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Exifdatum& Exifdatum::operator=(const uint16_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const uint32_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const URational& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const int16_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const int32_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const Rational& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Exifdatum::setValue(const Value* pValue)
    {
        value_.reset();
        if (pValue) value_ = pValue->clone();
    }

    int Exifdatum::setValue(const std::string& value)
    {
        if (value_.get() == 0) {
            TypeId type = key_->defaultTypeId();
            value_ = Value::create(type);
        }
        return value_->read(value);
    }

    int Exifdatum::setDataArea(const byte* buf, long len)
    {
        return value_.get() == 0 ? -1 : value_->setDataArea(buf, len);
    }

    std::string Exifdatum::key() const
    {
        return key_.get() == 0 ? "" : key_->key();
    }

    const char* Exifdatum::familyName() const
    {
        return key_.get() == 0 ? "" : key_->familyName();
    }

    std::string Exifdatum::groupName() const
    {
        return key_.get() == 0 ? "" : key_->groupName();
    }

    std::string Exifdatum::tagName() const
    {
        return key_.get() == 0 ? "" : key_->tagName();
    }

    std::string Exifdatum::tagLabel() const
    {
        return key_.get() == 0 ? "" : key_->tagLabel();
    }

    uint16_t Exifdatum::tag() const
    {
        return key_.get() == 0 ? 0xffff : key_->tag();
    }

    int Exifdatum::ifdId() const
    {
        return key_.get() == 0 ? ifdIdNotSet : key_->ifdId();
    }

    const char* Exifdatum::ifdName() const
    {
        return key_.get() == 0 ? "" : Internal::ifdName(static_cast<Internal::IfdId>(key_->ifdId()));
    }

    int Exifdatum::idx() const
    {
        return key_.get() == 0 ? 0 : key_->idx();
    }

    long Exifdatum::copy(byte* buf, ByteOrder byteOrder) const
    {
        return value_.get() == 0 ? 0 : value_->copy(buf, byteOrder);
    }

    TypeId Exifdatum::typeId() const
    {
        return value_.get() == 0 ? invalidTypeId : value_->typeId();
    }

    const char* Exifdatum::typeName() const
    {
        return TypeInfo::typeName(typeId());
    }

    long Exifdatum::typeSize() const
    {
        return TypeInfo::typeSize(typeId());
    }

    long Exifdatum::count() const
    {
        return value_.get() == 0 ? 0 : value_->count();
    }

    long Exifdatum::size() const
    {
        return value_.get() == 0 ? 0 : value_->size();
    }

    std::string Exifdatum::toString() const
    {
        return value_.get() == 0 ? "" : value_->toString();
    }

    std::string Exifdatum::toString(long n) const
    {
        return value_.get() == 0 ? "" : value_->toString(n);
    }

    long Exifdatum::toLong(long n) const
    {
        return value_.get() == 0 ? -1 : value_->toLong(n);
    }

    float Exifdatum::toFloat(long n) const
    {
        return value_.get() == 0 ? -1 : value_->toFloat(n);
    }

    Rational Exifdatum::toRational(long n) const
    {
        return value_.get() == 0 ? Rational(-1, 1) : value_->toRational(n);
    }

    Value::AutoPtr Exifdatum::getValue() const
    {
        return value_.get() == 0 ? Value::AutoPtr(0) : value_->clone();
    }

    long Exifdatum::sizeDataArea() const
    {
        return value_.get() == 0 ? 0 : value_->sizeDataArea();
    }

    DataBuf Exifdatum::dataArea() const
    {
        return value_.get() == 0 ? DataBuf(0, 0) : value_->dataArea();
    }

    ExifThumbC::ExifThumbC(const ExifData& exifData)
        : exifData_(exifData)
    {
    }

    DataBuf ExifThumbC::copy() const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return DataBuf();
        return thumbnail->copy(exifData_);
    }

    long ExifThumbC::writeFile(const std::string& path) const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return 0;
        std::string name = path + thumbnail->extension();
        DataBuf buf(thumbnail->copy(exifData_));
        if (buf.size_ == 0) return 0;
        return Exiv2::writeFile(buf, name);
    }

#ifdef EXV_UNICODE_PATH
    long ExifThumbC::writeFile(const std::wstring& wpath) const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return 0;
        std::wstring name = wpath + thumbnail->wextension();
        DataBuf buf(thumbnail->copy(exifData_));
        if (buf.size_ == 0) return 0;
        return Exiv2::writeFile(buf, name);
    }

#endif
    const char* ExifThumbC::mimeType() const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return "";
        return thumbnail->mimeType();
    }

    const char* ExifThumbC::extension() const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return "";
        return thumbnail->extension();
    }

#ifdef EXV_UNICODE_PATH
    const wchar_t* ExifThumbC::wextension() const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return EXV_WIDEN("");
        return thumbnail->wextension();
    }

#endif
    ExifThumb::ExifThumb(ExifData& exifData)
        : ExifThumbC(exifData), exifData_(exifData)
    {
    }

    void ExifThumb::setJpegThumbnail(
        const std::string& path,
              URational    xres,
              URational    yres,
              uint16_t     unit
    )
    {
        DataBuf thumb = readFile(path); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_, xres, yres, unit);
    }

#ifdef EXV_UNICODE_PATH
    void ExifThumb::setJpegThumbnail(
        const std::wstring& wpath,
              URational     xres,
              URational     yres,
              uint16_t      unit
    )
    {
        DataBuf thumb = readFile(wpath); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_, xres, yres, unit);
    }

#endif
    void ExifThumb::setJpegThumbnail(
        const byte*     buf,
              long      size,
              URational xres,
              URational yres,
              uint16_t  unit
    )
    {
        setJpegThumbnail(buf, size);
        exifData_["Exif.Thumbnail.XResolution"] = xres;
        exifData_["Exif.Thumbnail.YResolution"] = yres;
        exifData_["Exif.Thumbnail.ResolutionUnit"] = unit;
    }

    void ExifThumb::setJpegThumbnail(const std::string& path)
    {
        DataBuf thumb = readFile(path); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_);
    }

#ifdef EXV_UNICODE_PATH
    void ExifThumb::setJpegThumbnail(const std::wstring& wpath)
    {
        DataBuf thumb = readFile(wpath); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_);
    }

#endif
    void ExifThumb::setJpegThumbnail(const byte* buf, long size)
    {
        exifData_["Exif.Thumbnail.Compression"] = uint16_t(6);
        Exifdatum& format = exifData_["Exif.Thumbnail.JPEGInterchangeFormat"];
        format = uint32_t(0);
        format.setDataArea(buf, size);
        exifData_["Exif.Thumbnail.JPEGInterchangeFormatLength"] = uint32_t(size);
    }

    void ExifThumb::erase()
    {
        eraseIfd(exifData_, ifd1Id);
    }

    Exifdatum& ExifData::operator[](const std::string& key)
    {
        ExifKey exifKey(key);
        iterator pos = findKey(exifKey);
        if (pos == end()) {
            add(Exifdatum(exifKey));
            pos = findKey(exifKey);
        }
        return *pos;
    }

    void ExifData::add(const ExifKey& key, const Value* pValue)
    {
        add(Exifdatum(key, pValue));
    }

    void ExifData::add(const Exifdatum& exifdatum)
    {
        // allow duplicates
        exifMetadata_.push_back(exifdatum);
    }

    ExifData::const_iterator ExifData::findKey(const ExifKey& key) const
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindExifdatumByKey(key.key()));
    }

    ExifData::iterator ExifData::findKey(const ExifKey& key)
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindExifdatumByKey(key.key()));
    }

    void ExifData::clear()
    {
        exifMetadata_.clear();
    }

    void ExifData::sortByKey()
    {
        exifMetadata_.sort(cmpMetadataByKey);
    }

    void ExifData::sortByTag()
    {
        exifMetadata_.sort(cmpMetadataByTag);
    }

    ExifData::iterator ExifData::erase(ExifData::iterator beg, ExifData::iterator end)
    {
        return exifMetadata_.erase(beg, end);
    }

    ExifData::iterator ExifData::erase(ExifData::iterator pos)
    {
        return exifMetadata_.erase(pos);
    }

    ByteOrder ExifParser::decode(
              ExifData& exifData,
        const byte*     pData,
              uint32_t  size
    )
    {
        IptcData iptcData;
        XmpData  xmpData;
        ByteOrder bo = TiffParser::decode(exifData,
                                          iptcData,
                                          xmpData,
                                          pData,
                                          size);
#ifndef SUPPRESS_WARNINGS
        if (!iptcData.empty()) {
            EXV_WARNING << "Ignoring IPTC information encoded in the Exif data.\n";
        }
        if (!xmpData.empty()) {
            EXV_WARNING << "Ignoring XMP information encoded in the Exif data.\n";
        }
#endif
        return bo;
    } // ExifParser::decode

    //! @cond IGNORE
    enum Ptt { pttLen, pttTag, pttIfd };
    struct PreviewTags {
        Ptt ptt_;
        const char* key_;
    };
    //! @endcond

    WriteMethod ExifParser::encode(
              Blob&     blob,
        const byte*     pData,
              uint32_t  size,
              ByteOrder byteOrder,
        const ExifData& exifData
    )
    {
        ExifData ed = exifData;

        // Delete IFD0 tags that are "not recorded" in compressed images
        // Reference: Exif 2.2 specs, 4.6.8 Tag Support Levels, section A
        static const char* filteredIfd0Tags[] = {
            "Exif.Image.PhotometricInterpretation",
            "Exif.Image.StripOffsets",
            "Exif.Image.RowsPerStrip",
            "Exif.Image.StripByteCounts",
            "Exif.Image.JPEGInterchangeFormat",
            "Exif.Image.JPEGInterchangeFormatLength",
            "Exif.Image.SubIFDs"
        };
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredIfd0Tags); ++i) {
            ExifData::iterator pos = ed.findKey(ExifKey(filteredIfd0Tags[i]));
            if (pos != ed.end()) {
#ifdef DEBUG
                std::cerr << "Warning: Exif tag " << pos->key() << " not encoded\n";
#endif
                ed.erase(pos);
            }
        }

        // Delete IFDs which do not occur in JPEGs
        static const IfdId filteredIfds[] = {
            subImage1Id,
            subImage2Id,
            subImage3Id,
            subImage4Id,
            subImage5Id,
            subImage6Id,
            subImage7Id,
            subImage8Id,
            subImage9Id,
            subThumb1Id,
            panaRawId,
            ifd2Id,
            ifd3Id
        };
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredIfds); ++i) {
#ifdef DEBUG
            std::cerr << "Warning: Exif IFD " << filteredIfds[i] << " not encoded\n";
#endif
            eraseIfd(ed, filteredIfds[i]);
        }

        // IPTC and XMP are stored elsewhere, not in the Exif APP1 segment.
        IptcData emptyIptc;
        XmpData  emptyXmp;

        // Encode and check if the result fits into a JPEG Exif APP1 segment
        MemIo mio1;
        std::auto_ptr<TiffHeaderBase> header(new TiffHeader(byteOrder, 0x00000008, false));
        WriteMethod wm = TiffParserWorker::encode(mio1,
                                                  pData,
                                                  size,
                                                  ed,
                                                  emptyIptc,
                                                  emptyXmp,
                                                  Tag::root,
                                                  TiffMapping::findEncoder,
                                                  header.get(),
                                                  0);
        if (mio1.size() <= 65527) {
            append(blob, mio1.mmap(), mio1.size());
            return wm;
        }

        // If it doesn't fit, remove additional tags

        // Delete preview tags if the preview is larger than 32kB.
        // Todo: Enhance preview classes to be able to write and delete previews and use that instead.
        // Table must be sorted by preview, the first tag in each group is the size
        static const PreviewTags filteredPvTags[] = {
            { pttLen, "Exif.Minolta.ThumbnailLength"                  },
            { pttTag, "Exif.Minolta.ThumbnailOffset"                  },
            { pttLen, "Exif.Minolta.Thumbnail"                        },
            { pttLen, "Exif.NikonPreview.JPEGInterchangeFormatLength" },
            { pttIfd, "NikonPreview"                                  },
            { pttLen, "Exif.Olympus.ThumbnailLength"                  },
            { pttTag, "Exif.Olympus.ThumbnailOffset"                  },
            { pttLen, "Exif.Olympus.ThumbnailImage"                   },
            { pttLen, "Exif.Olympus.Thumbnail"                        },
            { pttLen, "Exif.Olympus2.ThumbnailLength"                 },
            { pttTag, "Exif.Olympus2.ThumbnailOffset"                 },
            { pttLen, "Exif.Olympus2.ThumbnailImage"                  },
            { pttLen, "Exif.Olympus2.Thumbnail"                       },
            { pttLen, "Exif.OlympusCs.PreviewImageLength"             },
            { pttTag, "Exif.OlympusCs.PreviewImageStart"              },
            { pttTag, "Exif.OlympusCs.PreviewImageValid"              },
            { pttLen, "Exif.Pentax.PreviewLength"                     },
            { pttTag, "Exif.Pentax.PreviewOffset"                     },
            { pttTag, "Exif.Pentax.PreviewResolution"                 },
            { pttLen, "Exif.Thumbnail.StripByteCounts"                },
            { pttIfd, "Thumbnail"                                     },
            { pttLen, "Exif.Thumbnail.JPEGInterchangeFormatLength"    },
            { pttIfd, "Thumbnail"                                     }
        };
        bool delTags = false;
        ExifData::iterator pos;
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredPvTags); ++i) {
            switch (filteredPvTags[i].ptt_) {
            case pttLen:
                delTags = false;
                pos = ed.findKey(ExifKey(filteredPvTags[i].key_));
                if (pos != ed.end() && sumToLong(*pos) > 32768) {
                    delTags = true;
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Exif tag " << pos->key() << " not encoded\n";
#endif
                    ed.erase(pos);
                }
                break;
            case pttTag:
                if (delTags) {
                    pos = ed.findKey(ExifKey(filteredPvTags[i].key_));
                    if (pos != ed.end()) {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << "Exif tag " << pos->key() << " not encoded\n";
#endif
                        ed.erase(pos);
                    }
                }
                break;
            case pttIfd:
                if (delTags) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Exif IFD " << filteredPvTags[i].key_ << " not encoded\n";
#endif
                    eraseIfd(ed, Internal::groupId(filteredPvTags[i].key_));
                }
                break;
            }
        }

        // Delete unknown tags larger than 4kB and known tags larger than 40kB.
        for (ExifData::iterator pos = ed.begin(); pos != ed.end(); ) {
            if (   (pos->size() > 4096 && pos->tagName().substr(0, 2) == "0x")
                || pos->size() > 40960) {
#ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Exif tag " << pos->key() << " not encoded\n";
#endif
                pos = ed.erase(pos);
            }
            else {
                ++pos;
            }
        }

        // Encode the remaining Exif tags again, don't care if it fits this time
        MemIo mio2;
        wm = TiffParserWorker::encode(mio2,
                                      pData,
                                      size,
                                      ed,
                                      emptyIptc,
                                      emptyXmp,
                                      Tag::root,
                                      TiffMapping::findEncoder,
                                      header.get(),
                                      0);
        append(blob, mio2.mmap(), mio2.size());
#ifdef DEBUG
        if (wm == wmIntrusive) {
            std::cerr << "SIZE OF EXIF DATA IS " << std::dec << io.size() << " BYTES\n";
        }
        else {
            std::cerr << "SIZE DOESN'T MATTER, NON-INTRUSIVE WRITING USED\n";
        }
#endif
        return wm;

    } // ExifParser::encode

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {

    //! @cond IGNORE
    Thumbnail::AutoPtr Thumbnail::create(const Exiv2::ExifData& exifData)
    {
        Thumbnail::AutoPtr thumbnail;
        const Exiv2::ExifKey k1("Exif.Thumbnail.Compression");
        Exiv2::ExifData::const_iterator pos = exifData.findKey(k1);
        if (pos != exifData.end()) {
            if (pos->count() == 0) return thumbnail;
            long compression = pos->toLong();
            if (compression == 6) {
                thumbnail = Thumbnail::AutoPtr(new JpegThumbnail);
            }
            else {
                thumbnail = Thumbnail::AutoPtr(new TiffThumbnail);
            }
        }
        else {
            const Exiv2::ExifKey k2("Exif.Thumbnail.JPEGInterchangeFormat");
            pos = exifData.findKey(k2);
            if (pos != exifData.end()) {
                thumbnail = Thumbnail::AutoPtr(new JpegThumbnail);
            }
        }
        return thumbnail;
    }

    const char* TiffThumbnail::mimeType() const
    {
        return "image/tiff";
    }

    const char* TiffThumbnail::extension() const
    {
        return ".tif";
    }

#ifdef EXV_UNICODE_PATH
    const wchar_t* TiffThumbnail::wextension() const
    {
        return EXV_WIDEN(".tif");
    }

#endif
    Exiv2::DataBuf TiffThumbnail::copy(const Exiv2::ExifData& exifData) const
    {
        Exiv2::ExifData thumb;
        // Copy all Thumbnail (IFD1) tags from exifData to Image (IFD0) tags in thumb
        for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end(); ++i) {
            if (i->groupName() == "Thumbnail") {
                std::string key = "Exif.Image." + i->tagName();
                thumb.add(Exiv2::ExifKey(key), &i->value());
            }
        }

        Exiv2::MemIo io;
        Exiv2::IptcData emptyIptc;
        Exiv2::XmpData  emptyXmp;
        Exiv2::TiffParser::encode(io, 0, 0, Exiv2::littleEndian, thumb, emptyIptc, emptyXmp);
        return io.read(io.size());
    }

    const char* JpegThumbnail::mimeType() const
    {
        return "image/jpeg";
    }

    const char* JpegThumbnail::extension() const
    {
        return ".jpg";
    }

#ifdef EXV_UNICODE_PATH
    const wchar_t* JpegThumbnail::wextension() const
    {
        return EXV_WIDEN(".jpg");
    }

#endif
    Exiv2::DataBuf JpegThumbnail::copy(const Exiv2::ExifData& exifData) const
    {
        Exiv2::ExifKey key("Exif.Thumbnail.JPEGInterchangeFormat");
        Exiv2::ExifData::const_iterator format = exifData.findKey(key);
        if (format == exifData.end()) return Exiv2::DataBuf();
        return format->dataArea();
    }

    long sumToLong(const Exiv2::Exifdatum& md)
    {
        long sum = 0;
        for (int i = 0; i < md.count(); ++i) {
            sum += md.toLong(i);
        }
        return sum;
    }

    void eraseIfd(Exiv2::ExifData& ed, Exiv2::IfdId ifdId)
    {
        ed.erase(std::remove_if(ed.begin(),
                                ed.end(),
                                Exiv2::FindExifdatum(ifdId)),
                 ed.end());
    }
    //! @endcond
}
