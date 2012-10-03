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
  File:      tiffvisitor.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: tiffvisitor.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffcomposite_int.hpp" // Do not change the order of these 2 includes,
#include "tiffvisitor_int.hpp"   // see bug #487
#include "tiffimage_int.hpp"
#include "makernote_int.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "value.hpp"
#include "image.hpp"
#include "jpgimage.hpp"
#include "i18n.h"             // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
namespace {
    //! Unary predicate that matches an Exifdatum with a given group and index.
    class FindExifdatum2 {
    public:
        //! Constructor, initializes the object with the group and index to look for.
        FindExifdatum2(Exiv2::Internal::IfdId group, int idx)
            : groupName_(Exiv2::Internal::groupName(group)), idx_(idx) {}
        //! Returns true if group and index match.
        bool operator()(const Exiv2::Exifdatum& md) const
        {
            return idx_ == md.idx() && 0 == strcmp(md.groupName().c_str(), groupName_);
        }

    private:
        const char* groupName_;
        int idx_;

    }; // class FindExifdatum2

    Exiv2::ByteOrder stringToByteOrder(const std::string& val)
    {
        Exiv2::ByteOrder bo = Exiv2::invalidByteOrder;
        if (0 == strcmp("II", val.c_str())) bo = Exiv2::littleEndian;
        else if (0 == strcmp("MM", val.c_str())) bo = Exiv2::bigEndian;

        return bo;
    }
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    TiffVisitor::TiffVisitor()
    {
        for (int i = 0; i < events_; ++i) {
            go_[i] = true;
        }
    }

    TiffVisitor::~TiffVisitor()
    {
    }

    void TiffVisitor::setGo(GoEvent event, bool go)
    {
        assert(event >= 0 && event < events_);
        go_[event] = go;
    }

    bool TiffVisitor::go(GoEvent event) const
    {
        assert(event >= 0 && event < events_);
        return go_[event];
    }

    void TiffVisitor::visitDirectoryNext(TiffDirectory* /*object*/)
    {
    }

    void TiffVisitor::visitDirectoryEnd(TiffDirectory* /*object*/)
    {
    }

    void TiffVisitor::visitIfdMakernoteEnd(TiffIfdMakernote* /*object*/)
    {
    }

    void TiffVisitor::visitBinaryArrayEnd(TiffBinaryArray* /*object*/)
    {
    }

    void TiffFinder::init(uint16_t tag, IfdId group)
    {
        tag_ = tag;
        group_ = group;
        tiffComponent_ = 0;
        setGo(geTraverse, true);
    }

    TiffFinder::~TiffFinder()
    {
    }

    void TiffFinder::findObject(TiffComponent* object)
    {
        if (object->tag() == tag_ && object->group() == group_) {
            tiffComponent_ = object;
            setGo(geTraverse, false);
        }
    }

    void TiffFinder::visitEntry(TiffEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDataEntry(TiffDataEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitImageEntry(TiffImageEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSizeEntry(TiffSizeEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDirectory(TiffDirectory* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSubIfd(TiffSubIfd* object)
    {
        findObject(object);
    }

    void TiffFinder::visitMnEntry(TiffMnEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        findObject(object);
    }

    void TiffFinder::visitBinaryArray(TiffBinaryArray* object)
    {
        findObject(object);
    }

    void TiffFinder::visitBinaryElement(TiffBinaryElement* object)
    {
        findObject(object);
    }

    TiffCopier::TiffCopier(      TiffComponent*  pRoot,
                                 uint32_t        root,
                           const TiffHeaderBase* pHeader,
                           const PrimaryGroups*  pPrimaryGroups)
        : pRoot_(pRoot),
          root_(root),
          pHeader_(pHeader),
          pPrimaryGroups_(pPrimaryGroups)
    {
        assert(pRoot_ != 0);
        assert(pHeader_ != 0);
        assert(pPrimaryGroups_ != 0);
    }

    TiffCopier::~TiffCopier()
    {
    }

    void TiffCopier::copyObject(TiffComponent* object)
    {
        assert(object != 0);

        if (pHeader_->isImageTag(object->tag(), object->group(), pPrimaryGroups_)) {
            TiffComponent::AutoPtr clone = object->clone();
            // Assumption is that the corresponding TIFF entry doesn't exist
            TiffPath tiffPath;
            TiffCreator::getPath(tiffPath, object->tag(), object->group(), root_);
            pRoot_->addPath(object->tag(), tiffPath, pRoot_, clone);
#ifdef DEBUG
            ExifKey key(object->tag(), groupName(object->group()));
            std::cerr << "Copied " << key << "\n";
#endif
        }
    }

    void TiffCopier::visitEntry(TiffEntry* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitDataEntry(TiffDataEntry* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitImageEntry(TiffImageEntry* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitSizeEntry(TiffSizeEntry* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitDirectory(TiffDirectory* /*object*/)
    {
        // Do not copy directories (avoids problems with SubIfds)
    }

    void TiffCopier::visitSubIfd(TiffSubIfd* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitMnEntry(TiffMnEntry* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitIfdMakernote(TiffIfdMakernote* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitBinaryArray(TiffBinaryArray* object)
    {
        copyObject(object);
    }

    void TiffCopier::visitBinaryElement(TiffBinaryElement* object)
    {
        copyObject(object);
    }

    TiffDecoder::TiffDecoder(
        ExifData&            exifData,
        IptcData&            iptcData,
        XmpData&             xmpData,
        TiffComponent* const pRoot,
        FindDecoderFct       findDecoderFct
    )
        : exifData_(exifData),
          iptcData_(iptcData),
          xmpData_(xmpData),
          pRoot_(pRoot),
          findDecoderFct_(findDecoderFct),
          decodedIptc_(false)
    {
        assert(pRoot != 0);

        exifData_.clear();
        iptcData_.clear();
        xmpData_.clear();

        // Find camera make
        TiffFinder finder(0x010f, ifd0Id);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te && te->pValue()) {
            make_ = te->pValue()->toString();
        }
    }

    TiffDecoder::~TiffDecoder()
    {
    }

    void TiffDecoder::visitEntry(TiffEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitDataEntry(TiffDataEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitImageEntry(TiffImageEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitSizeEntry(TiffSizeEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitDirectory(TiffDirectory* /*object*/)
    {
        // Nothing to do
    }

    void TiffDecoder::visitSubIfd(TiffSubIfd* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitMnEntry(TiffMnEntry* object)
    {
        // Always decode binary makernote tag
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        exifData_["Exif.MakerNote.Offset"] = object->mnOffset();
        switch (object->byteOrder()) {
        case littleEndian:
            exifData_["Exif.MakerNote.ByteOrder"] = "II";
            break;
        case bigEndian:
            exifData_["Exif.MakerNote.ByteOrder"] = "MM";
            break;
        case invalidByteOrder:
            assert(object->byteOrder() != invalidByteOrder);
            break;
        }
    }

    void TiffDecoder::getObjData(byte const*& pData,
                                 long& size,
                                 uint16_t tag,
                                 IfdId group,
                                 const TiffEntryBase* object)
    {
        if (object && object->tag() == tag && object->group() == group) {
            pData = object->pData();
            size = object->size();
            return;
        }
        TiffFinder finder(tag, group);
        pRoot_->accept(finder);
        TiffEntryBase const* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te) {
            pData = te->pData();
            size = te->size();
            return;
        }
    }

    void TiffDecoder::decodeXmp(const TiffEntryBase* object)
    {
        // add Exif tag anyway
        decodeStdTiffEntry(object);

        byte const* pData = 0;
        long size = 0;
        getObjData(pData, size, 0x02bc, ifd0Id, object);
        if (pData) {
            std::string xmpPacket;
            xmpPacket.assign(reinterpret_cast<const char*>(pData), size);
            std::string::size_type idx = xmpPacket.find_first_of('<');
            if (idx != std::string::npos && idx > 0) {
#ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Removing " << static_cast<unsigned long>(idx)
                            << " characters from the beginning of the XMP packet\n";
#endif
                xmpPacket = xmpPacket.substr(idx);
            }
            if (XmpParser::decode(xmpData_, xmpPacket)) {
#ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
            }
        }
    } // TiffDecoder::decodeXmp

    void TiffDecoder::decodeIptc(const TiffEntryBase* object)
    {
        // add Exif tag anyway
        decodeStdTiffEntry(object);

        // All tags are read at this point, so the first time we come here,
        // find the relevant IPTC tag and decode IPTC if found
        if (decodedIptc_) {
            return;
        }
        decodedIptc_ = true;
        // 1st choice: IPTCNAA
        byte const* pData = 0;
        long size = 0;
        getObjData(pData, size, 0x83bb, ifd0Id, object);
        if (pData) {
            if (0 == IptcParser::decode(iptcData_, pData, size)) {
                return;
            }
#ifndef SUPPRESS_WARNINGS
            else {
                EXV_WARNING << "Failed to decode IPTC block found in "
                            << "Directory Image, entry 0x83bb\n";
            }
#endif
        }

        // 2nd choice if no IPTCNAA record found or failed to decode it:
        // ImageResources
        pData = 0;
        size = 0;
        getObjData(pData, size, 0x8649, ifd0Id, object);
        if (pData) {
            byte const* record = 0;
            uint32_t sizeHdr = 0;
            uint32_t sizeData = 0;
            if (0 != Photoshop::locateIptcIrb(pData, size,
                                              &record, &sizeHdr, &sizeData)) {
                return;
            }
            if (0 == IptcParser::decode(iptcData_, record + sizeHdr, sizeData)) {
                return;
            }
#ifndef SUPPRESS_WARNINGS
            else {
                EXV_WARNING << "Failed to decode IPTC block found in "
                            << "Directory Image, entry 0x8649\n";
            }
#endif
        }
    } // TiffMetadataDecoder::decodeIptc

    void TiffDecoder::decodeTiffEntry(const TiffEntryBase* object)
    {
        assert(object != 0);

        // Don't decode the entry if value is not set
        if (!object->pValue()) return;

        const DecoderFct decoderFct = findDecoderFct_(make_,
                                                      object->tag(),
                                                      object->group());
        // skip decoding if decoderFct == 0
        if (decoderFct) {
            EXV_CALL_MEMBER_FN(*this, decoderFct)(object);
        }
    } // TiffDecoder::decodeTiffEntry

    void TiffDecoder::decodeStdTiffEntry(const TiffEntryBase* object)
    {
        assert(object != 0);
        ExifKey key(object->tag(), groupName(object->group()));
        key.setIdx(object->idx());
        exifData_.add(key, object->pValue());

    } // TiffDecoder::decodeTiffEntry

    void TiffDecoder::visitBinaryArray(TiffBinaryArray* object)
    {
        if (object->cfg() == 0 || !object->decoded()) {
            decodeTiffEntry(object);
        }
    }

    void TiffDecoder::visitBinaryElement(TiffBinaryElement* object)
    {
        decodeTiffEntry(object);
    }

    TiffEncoder::TiffEncoder(
            const ExifData&      exifData,
            const IptcData&      iptcData,
            const XmpData&       xmpData,
                  TiffComponent* pRoot,
            const bool           isNewImage,
            const PrimaryGroups* pPrimaryGroups,
            const TiffHeaderBase* pHeader,
                  FindEncoderFct findEncoderFct
    )
        : exifData_(exifData),
          iptcData_(iptcData),
          xmpData_(xmpData),
          del_(true),
          pHeader_(pHeader),
          pRoot_(pRoot),
          isNewImage_(isNewImage),
          pPrimaryGroups_(pPrimaryGroups),
          pSourceTree_(0),          
          findEncoderFct_(findEncoderFct),
          dirty_(false),
          writeMethod_(wmNonIntrusive)
    {
        assert(pRoot != 0);
        assert(pPrimaryGroups != 0);
        assert(pHeader != 0);

        byteOrder_ = pHeader->byteOrder();
        origByteOrder_ = byteOrder_;

        encodeIptc();
        encodeXmp();

        // Find camera make
        ExifKey key("Exif.Image.Make");
        ExifData::const_iterator pos = exifData_.findKey(key);
        if (pos != exifData_.end()) {
            make_ = pos->toString();
        }
        if (make_.empty() && pRoot_) {
            TiffFinder finder(0x010f, ifd0Id);
            pRoot_->accept(finder);
            TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
            if (te && te->pValue()) {
                make_ = te->pValue()->toString();
            }
        }
    }

    TiffEncoder::~TiffEncoder()
    {
    }

    void TiffEncoder::encodeIptc()
    {
        // Update IPTCNAA Exif tag, if it exists. Delete the tag if there
        // is no IPTC data anymore.
        // If there is new IPTC data and Exif.Image.ImageResources does
        // not exist, create a new IPTCNAA Exif tag.
        bool del = false;
        ExifKey iptcNaaKey("Exif.Image.IPTCNAA");
        ExifData::iterator pos = exifData_.findKey(iptcNaaKey);
        if (pos != exifData_.end()) {
            iptcNaaKey.setIdx(pos->idx());
            exifData_.erase(pos);
            del = true;
        }
        DataBuf rawIptc = IptcParser::encode(iptcData_);
        ExifKey irbKey("Exif.Image.ImageResources");
        pos = exifData_.findKey(irbKey);
        if (pos != exifData_.end()) {
            irbKey.setIdx(pos->idx());
        }
        if (rawIptc.size_ != 0 && (del || pos == exifData_.end())) {
            Value::AutoPtr value = Value::create(unsignedLong);
            DataBuf buf;
            if (rawIptc.size_ % 4 != 0) {
                // Pad the last unsignedLong value with 0s
                buf.alloc((rawIptc.size_ / 4) * 4 + 4);
                memset(buf.pData_, 0x0, buf.size_);
                memcpy(buf.pData_, rawIptc.pData_, rawIptc.size_);
            }
            else {
                buf = rawIptc; // Note: This resets rawIptc
            }
            value->read(buf.pData_, buf.size_, byteOrder_);
            Exifdatum iptcDatum(iptcNaaKey, value.get());
            exifData_.add(iptcDatum);
            pos = exifData_.findKey(irbKey); // needed after add()
        }
        // Also update IPTC IRB in Exif.Image.ImageResources if it exists,
        // but don't create it if not.
        if (pos != exifData_.end()) {
            DataBuf irbBuf(pos->value().size());
            pos->value().copy(irbBuf.pData_, invalidByteOrder);
            irbBuf = Photoshop::setIptcIrb(irbBuf.pData_, irbBuf.size_, iptcData_);
            exifData_.erase(pos);
            if (irbBuf.size_ != 0) {
                Value::AutoPtr value = Value::create(unsignedByte);
                value->read(irbBuf.pData_, irbBuf.size_, invalidByteOrder);
                Exifdatum iptcDatum(irbKey, value.get());
                exifData_.add(iptcDatum);
            }
        }
    } // TiffEncoder::encodeIptc

    void TiffEncoder::encodeXmp()
    {
        ExifKey xmpKey("Exif.Image.XMLPacket");
        // Remove any existing XMP Exif tag
        ExifData::iterator pos = exifData_.findKey(xmpKey);
        if (pos != exifData_.end()) {
            xmpKey.setIdx(pos->idx());
            exifData_.erase(pos);
        }
        std::string xmpPacket;
        if (XmpParser::encode(xmpPacket, xmpData_) > 1) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
        }
        if (!xmpPacket.empty()) {
            // Set the XMP Exif tag to the new value
            Value::AutoPtr value = Value::create(unsignedByte);
            value->read(reinterpret_cast<const byte*>(&xmpPacket[0]),
                        static_cast<long>(xmpPacket.size()),
                        invalidByteOrder);
            Exifdatum xmpDatum(xmpKey, value.get());
            exifData_.add(xmpDatum);
        }
    } // TiffEncoder::encodeXmp

    void TiffEncoder::setDirty(bool flag)
    {
        dirty_ = flag;
        setGo(geTraverse, !flag);
    }

    bool TiffEncoder::dirty() const
    {
        if (dirty_ || exifData_.count() > 0) return true;
        return false;
    }

    void TiffEncoder::visitEntry(TiffEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitDataEntry(TiffDataEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitImageEntry(TiffImageEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitSizeEntry(TiffSizeEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitDirectory(TiffDirectory* /*object*/)
    {
        // Nothing to do
    }

    void TiffEncoder::visitDirectoryNext(TiffDirectory* object)
    {
        // Update type and count in IFD entries, in case they changed
        assert(object != 0);

        byte* p = object->start() + 2;
        for (TiffDirectory::Components::iterator i = object->components_.begin();
             i != object->components_.end(); ++i) {
            p += updateDirEntry(p, byteOrder(), *i);
        }
    }

    uint32_t TiffEncoder::updateDirEntry(byte* buf,
                                         ByteOrder byteOrder,
                                         TiffComponent* pTiffComponent) const
    {
        assert(buf);
        assert(pTiffComponent);
        TiffEntryBase* pTiffEntry = dynamic_cast<TiffEntryBase*>(pTiffComponent);
        assert(pTiffEntry);
        us2Data(buf + 2, pTiffEntry->tiffType(), byteOrder);
        ul2Data(buf + 4, pTiffEntry->count(),    byteOrder);
        // Move data to offset field, if it fits and is not yet there.
        if (pTiffEntry->size() <= 4 && buf + 8 != pTiffEntry->pData()) {
#ifdef DEBUG
            std::cerr << "Copying data for tag " << pTiffEntry->tag()
                      << " to offset area.\n";
#endif
            memset(buf + 8, 0x0, 4);
            memcpy(buf + 8, pTiffEntry->pData(), pTiffEntry->size());
            memset(const_cast<byte*>(pTiffEntry->pData()), 0x0, pTiffEntry->size());
        }
        return 12;
    }

    void TiffEncoder::visitSubIfd(TiffSubIfd* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitMnEntry(TiffMnEntry* object)
    {
        // Test is required here as well as in the callback encoder function
        if (!object->mn_) {
            encodeTiffComponent(object);
        }
        else if (del_) {
            // The makernote is made up of decoded tags, delete binary tag
            ExifKey key(object->tag(), groupName(object->group()));
            ExifData::iterator pos = exifData_.findKey(key);
            if (pos != exifData_.end()) exifData_.erase(pos);
        }
    }

    void TiffEncoder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        ExifData::iterator pos = exifData_.findKey(ExifKey("Exif.MakerNote.ByteOrder"));
        if (pos != exifData_.end()) {
            // Set Makernote byte order
            ByteOrder bo = stringToByteOrder(pos->toString());
            if (bo != invalidByteOrder && bo != object->byteOrder()) {
                object->setByteOrder(bo);
                setDirty();
            }
            if (del_) exifData_.erase(pos);
        }
        if (del_) {
            // Remove remaining synthesized tags
            static const char* synthesizedTags[] = {
                "Exif.MakerNote.Offset",
            };
            for (unsigned int i = 0; i < EXV_COUNTOF(synthesizedTags); ++i) {
                ExifData::iterator pos = exifData_.findKey(ExifKey(synthesizedTags[i]));
                if (pos != exifData_.end()) exifData_.erase(pos);
            }
        }
        // Modify encoder for Makernote peculiarities, byte order
        byteOrder_ = object->byteOrder();

    } // TiffEncoder::visitIfdMakernote

    void TiffEncoder::visitIfdMakernoteEnd(TiffIfdMakernote* /*object*/)
    {
        // Reset byte order back to that from the c'tor
        byteOrder_ = origByteOrder_;

    } // TiffEncoder::visitIfdMakernoteEnd

    void TiffEncoder::visitBinaryArray(TiffBinaryArray* object)
    {
        if (object->cfg() == 0 || !object->decoded()) {
            encodeTiffComponent(object);
        }
    }

    void TiffEncoder::visitBinaryArrayEnd(TiffBinaryArray* object)
    {
        assert(object != 0);

        if (object->cfg() == 0 || !object->decoded()) return;
        int32_t size = object->TiffEntryBase::doSize();
        if (size == 0) return;
        if (!object->initialize(pRoot_)) return;

        // Re-encrypt buffer if necessary
        const CryptFct cryptFct = object->cfg()->cryptFct_;
        if (cryptFct != 0) {
            const byte* pData = object->pData();
            DataBuf buf = cryptFct(object->tag(), pData, size, pRoot_);
            if (buf.size_ > 0) {
                pData = buf.pData_;
                size = buf.size_;
            }
            if (!object->updOrigDataBuf(pData, size)) {
                setDirty();
            }
        }
    }

    void TiffEncoder::visitBinaryElement(TiffBinaryElement* object)
    {
        // Temporarily overwrite byteorder according to that of the binary element
        ByteOrder boOrig = byteOrder_;
        if (object->elByteOrder() != invalidByteOrder) byteOrder_ = object->elByteOrder();
        encodeTiffComponent(object);
        byteOrder_ = boOrig;
    }

    bool TiffEncoder::isImageTag(uint16_t tag, IfdId group) const
    {
        if (!isNewImage_ && pHeader_->isImageTag(tag, group, pPrimaryGroups_)) {
            return true;
        }
        return false;
    }

    void TiffEncoder::encodeTiffComponent(
              TiffEntryBase* object,
        const Exifdatum*     datum
    )
    {
        assert(object != 0);

        // Skip image tags of existing TIFF image - they were copied earlier -
        // but add and encode image tags of new images (creation)
        if (isImageTag(object->tag(), object->group())) return;

        ExifData::iterator pos = exifData_.end();
        const Exifdatum* ed = datum;
        if (ed == 0) {
            // Non-intrusive writing: find matching tag
            ExifKey key(object->tag(), groupName(object->group()));
            pos = exifData_.findKey(key);
            if (pos != exifData_.end()) {
                ed = &(*pos);
                if (object->idx() != pos->idx()) {
                    // Try to find exact match (in case of duplicate tags)
                    ExifData::iterator pos2 =
                        std::find_if(exifData_.begin(), exifData_.end(),
                                     FindExifdatum2(object->group(), object->idx()));
                    if (pos2 != exifData_.end() && pos2->key() == key.key()) {
                        ed = &(*pos2);
                        pos = pos2; // make sure we delete the correct tag below
                    }
                }
            }
            else {
                setDirty();
#ifdef DEBUG
                std::cerr << "DELETING          " << key << ", idx = " << object->idx() << "\n";
#endif
            }
        }
        else {
            // For intrusive writing, the index is used to preserve the order of
            // duplicate tags
            object->idx_ = ed->idx();
        }
        if (ed) {
            const EncoderFct fct = findEncoderFct_(make_, object->tag(), object->group());
            if (fct) {
                // If an encoding function is registered for the tag, use it
                EXV_CALL_MEMBER_FN(*this, fct)(object, ed);
            }
            else {
                // Else use the encode function at the object (results in a double-dispatch
                // to the appropriate encoding function of the encoder.
                object->encode(*this, ed);
            }
        }
        if (del_ && pos != exifData_.end()) {
            exifData_.erase(pos);
        }
#ifdef DEBUG
        std::cerr << "\n";
#endif
    } // TiffEncoder::encodeTiffComponent

    void TiffEncoder::encodeBinaryArray(TiffBinaryArray* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);
    } // TiffEncoder::encodeBinaryArray

    void TiffEncoder::encodeBinaryElement(TiffBinaryElement* object, const Exifdatum* datum)
    {
        encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeArrayElement

    void TiffEncoder::encodeDataEntry(TiffDataEntry* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);

        if (!dirty_ && writeMethod() == wmNonIntrusive) {
            assert(object);
            assert(object->pValue());
            if (  object->sizeDataArea_
                < static_cast<uint32_t>(object->pValue()->sizeDataArea())) {
#ifdef DEBUG
                ExifKey key(object->tag(), groupName(object->group()));
                std::cerr << "DATAAREA GREW     " << key << "\n";
#endif
                setDirty();
            }
            else {
                // Write the new dataarea, fill with 0x0
#ifdef DEBUG
                ExifKey key(object->tag(), groupName(object->group()));
                std::cerr << "Writing data area for " << key << "\n";
#endif
                DataBuf buf = object->pValue()->dataArea();
                memcpy(object->pDataArea_, buf.pData_, buf.size_);
                if (object->sizeDataArea_ - buf.size_ > 0) {
                    memset(object->pDataArea_ + buf.size_,
                           0x0, object->sizeDataArea_ - buf.size_);
                }
            }
        }

    } // TiffEncoder::encodeDataEntry

    void TiffEncoder::encodeTiffEntry(TiffEntry* object, const Exifdatum* datum)
    {
        encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeTiffEntry

    void TiffEncoder::encodeImageEntry(TiffImageEntry* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);

        uint32_t sizeDataArea = object->pValue()->sizeDataArea();

        if (sizeDataArea > 0 && writeMethod() == wmNonIntrusive) {
#ifdef DEBUG
            std::cerr << "\t DATAAREA IS SET (NON-INTRUSIVE WRITING)";
#endif
            setDirty();
        }

        if (sizeDataArea > 0 && writeMethod() == wmIntrusive) {
#ifdef DEBUG
            std::cerr << "\t DATAAREA IS SET (INTRUSIVE WRITING)";
#endif
            // Set pseudo strips (without a data pointer) from the size tag
            ExifKey key(object->szTag(), groupName(object->szGroup()));
            ExifData::const_iterator pos = exifData_.findKey(key);
            const byte* zero = 0;
            if (pos == exifData_.end()) {
#ifndef SUPPRESS_WARNINGS
                EXV_ERROR << "Size tag " << key
                          << " not found. Writing only one strip.\n";
#endif
                object->strips_.clear();
                object->strips_.push_back(std::make_pair(zero, sizeDataArea));
            }
            else {
                uint32_t sizeTotal = 0;
                object->strips_.clear();
                for (long i = 0; i < pos->count(); ++i) {
                    uint32_t len = pos->toLong(i);
                    object->strips_.push_back(std::make_pair(zero, len));
                    sizeTotal += len;
                }
                if (sizeTotal != sizeDataArea) {
#ifndef SUPPRESS_WARNINGS
                    ExifKey key2(object->tag(), groupName(object->group()));
                    EXV_ERROR << "Sum of all sizes of " << key
                              << " != data size of " << key2 << ". "
                              << "This results in an invalid image.\n";
#endif
                    // Todo: How to fix? Write only one strip?
                }
            }
        }

        if (sizeDataArea == 0 && writeMethod() == wmIntrusive) {
#ifdef DEBUG
            std::cerr << "\t USE STRIPS FROM SOURCE TREE IMAGE ENTRY";
#endif
            // Set strips from source tree
            if (pSourceTree_) {
                TiffFinder finder(object->tag(), object->group());
                pSourceTree_->accept(finder);
                TiffImageEntry* ti = dynamic_cast<TiffImageEntry*>(finder.result());
                if (ti) {
                    object->strips_ = ti->strips_;
                }
            }
#ifndef SUPPRESS_WARNINGS
            else {
                ExifKey key2(object->tag(), groupName(object->group()));
                EXV_WARNING << "No image data to encode " << key2 << ".\n";
            }
#endif
        }

    } // TiffEncoder::encodeImageEntry

    void TiffEncoder::encodeMnEntry(TiffMnEntry* object, const Exifdatum* datum)
    {
        // Test is required here as well as in the visit function
        if (!object->mn_) encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeMnEntry

    void TiffEncoder::encodeSizeEntry(TiffSizeEntry* object, const Exifdatum* datum)
    {
        encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeSizeEntry

    void TiffEncoder::encodeSubIfd(TiffSubIfd* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);
    } // TiffEncoder::encodeSubIfd

    void TiffEncoder::encodeTiffEntryBase(TiffEntryBase* object, const Exifdatum* datum)
    {
        assert(object != 0);
        assert(datum != 0);

#ifdef DEBUG
        bool tooLarge = false;
#endif
        uint32_t newSize = datum->size();
        if (newSize > object->size_) { // value doesn't fit, encode for intrusive writing
            setDirty();
#ifdef DEBUG
            tooLarge = true;
#endif
        }
        object->updateValue(datum->getValue(), byteOrder()); // clones the value
#ifdef DEBUG
        ExifKey key(object->tag(), groupName(object->group()));
        std::cerr << "UPDATING DATA     " << key;
        if (tooLarge) {
            std::cerr << "\t\t\t ALLOCATED " << std::dec << object->size_ << " BYTES";
        }
#endif
    } // TiffEncoder::encodeTiffEntryBase

    void TiffEncoder::encodeOffsetEntry(TiffEntryBase* object, const Exifdatum* datum)
    {
        assert(object != 0);
        assert(datum != 0);

        uint32_t newSize = datum->size();
        if (newSize > object->size_) { // value doesn't fit, encode for intrusive writing
            setDirty();
            object->updateValue(datum->getValue(), byteOrder()); // clones the value
#ifdef DEBUG
            ExifKey key(object->tag(), groupName(object->group()));
            std::cerr << "UPDATING DATA     " << key;
            std::cerr << "\t\t\t ALLOCATED " << object->size() << " BYTES";
#endif
        }
        else {
            object->setValue(datum->getValue()); // clones the value
#ifdef DEBUG
            ExifKey key(object->tag(), groupName(object->group()));
            std::cerr << "NOT UPDATING      " << key;
            std::cerr << "\t\t\t PRESERVE VALUE DATA";
#endif
        }

    } // TiffEncoder::encodeOffsetEntry

    void TiffEncoder::add(
        TiffComponent* pRootDir,
        TiffComponent* pSourceDir,
        uint32_t       root
    )
    {
        assert(pRootDir != 0);

        writeMethod_ = wmIntrusive;
        pSourceTree_ = pSourceDir;

        // Ensure that the exifData_ entries are not deleted, to be able to
        // iterate over all remaining entries.
        del_ = false;

        ExifData::const_iterator posBo = exifData_.end();
        for (ExifData::const_iterator i = exifData_.begin();
             i != exifData_.end(); ++i) {

            IfdId group = groupId(i->groupName());
            // Skip synthesized info tags
            if (group == mnId) {
                if (i->tag() == 0x0002) {
                    posBo = i;
                }
                continue;
            }

            // Skip image tags of existing TIFF image - they were copied earlier -
            // but add and encode image tags of new images (creation)
            if (isImageTag(i->tag(), group)) continue;

            // Assumption is that the corresponding TIFF entry doesn't exist
            TiffPath tiffPath;
            TiffCreator::getPath(tiffPath, i->tag(), group, root);
            TiffComponent* tc = pRootDir->addPath(i->tag(), tiffPath, pRootDir);
            TiffEntryBase* object = dynamic_cast<TiffEntryBase*>(tc);
#ifdef DEBUG
            if (object == 0) {
                std::cerr << "Warning: addPath() didn't add an entry for "
                          << i->groupName()
                          << " tag 0x" << std::setw(4) << std::setfill('0')
                          << std::hex << i->tag() << "\n";
            }
#endif
            if (object != 0) {
                encodeTiffComponent(object, &(*i));
            }
        }

        /*
          What follows is a hack. I can't think of a better way to set
          the makernote byte order (and other properties maybe) in the
          makernote header during intrusive writing. The thing is that
          visit/encodeIfdMakernote is not called in this case and there
          can't be an Exif tag which corresponds to this component.
         */
        if (posBo == exifData_.end()) return;

        TiffFinder finder(0x927c, exifId);
        pRootDir->accept(finder);
        TiffMnEntry* te = dynamic_cast<TiffMnEntry*>(finder.result());
        if (te) {
            TiffIfdMakernote* tim = dynamic_cast<TiffIfdMakernote*>(te->mn_);
            if (tim) {
                // Set Makernote byte order
                ByteOrder bo = stringToByteOrder(posBo->toString());
                if (bo != invalidByteOrder) tim->setByteOrder(bo);
            }
        }

    } // TiffEncoder::add

    TiffReader::TiffReader(const byte*    pData,
                           uint32_t       size,
                           TiffComponent* pRoot,
                           TiffRwState::AutoPtr state)
        : pData_(pData),
          size_(size),
          pLast_(pData + size),
          pRoot_(pRoot),
          pState_(state.release()),
          pOrigState_(pState_),
          postProc_(false)
    {
        assert(pData_);
        assert(size_ > 0);

    } // TiffReader::TiffReader

    TiffReader::~TiffReader()
    {
        if (pOrigState_ != pState_) delete pOrigState_;
        delete pState_;
    }

    void TiffReader::resetState() {
        if (pOrigState_ != pState_) delete pState_;
        pState_ = pOrigState_;
    }

    void TiffReader::changeState(TiffRwState::AutoPtr state)
    {
        if (state.get() != 0) {
            if (pOrigState_ != pState_) delete pState_;
            // invalidByteOrder indicates 'no change'
            if (state->byteOrder_ == invalidByteOrder) state->byteOrder_ = pState_->byteOrder_;
            pState_ = state.release();
        }
    }

    ByteOrder TiffReader::byteOrder() const
    {
        assert(pState_);
        return pState_->byteOrder_;
    }

    uint32_t TiffReader::baseOffset() const
    {
        assert(pState_);
        return pState_->baseOffset_;
    }

    void TiffReader::readDataEntryBase(TiffDataEntryBase* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        TiffFinder finder(object->szTag(), object->szGroup());
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te && te->pValue()) {
            object->setStrips(te->pValue(), pData_, size_, baseOffset());
        }
    }

    void TiffReader::visitEntry(TiffEntry* object)
    {
        readTiffEntry(object);
    }

    void TiffReader::visitDataEntry(TiffDataEntry* object)
    {
        readDataEntryBase(object);
    }

    void TiffReader::visitImageEntry(TiffImageEntry* object)
    {
        readDataEntryBase(object);
    }

    void TiffReader::visitSizeEntry(TiffSizeEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        TiffFinder finder(object->dtTag(), object->dtGroup());
        pRoot_->accept(finder);
        TiffDataEntryBase* te = dynamic_cast<TiffDataEntryBase*>(finder.result());
        if (te && te->pValue()) {
            te->setStrips(object->pValue(), pData_, size_, baseOffset());
        }
    }

    bool TiffReader::circularReference(const byte* start, IfdId group)
    {
        DirList::const_iterator pos = dirList_.find(start);
        if (pos != dirList_.end()) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << groupName(group) << " pointer references previously read "
                      << groupName(pos->second) << " directory. Ignored.\n";
#endif
            return true;
        }
        dirList_[start] = group;
        return false;
    }

    int TiffReader::nextIdx(IfdId group)
    {
        return ++idxSeq_[group];
    }

    void TiffReader::postProcess()
    {
        postProc_ = true;
        for (PostList::const_iterator pos = postList_.begin(); pos != postList_.end(); ++pos) {
            (*pos)->accept(*this);
        }
        postProc_ = false;
    }

    void TiffReader::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);

        const byte* p = object->start();
        assert(p >= pData_);

        if (circularReference(object->start(), object->group())) return;

        if (p + 2 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "Directory " << groupName(object->group())
                      << ": IFD exceeds data buffer, cannot read entry count.\n";
#endif
            return;
        }
        const uint16_t n = getUShort(p, byteOrder());
        p += 2;
        // Sanity check with an "unreasonably" large number
        if (n > 256) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "Directory " << groupName(object->group()) << " with "
                      << n << " entries considered invalid; not read.\n";
#endif
            return;
        }
        for (uint16_t i = 0; i < n; ++i) {
            if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                EXV_ERROR << "Directory " << groupName(object->group())
                          << ": IFD entry " << i
                          << " lies outside of the data buffer.\n";
#endif
                return;
            }
            uint16_t tag = getUShort(p, byteOrder());
            TiffComponent::AutoPtr tc = TiffCreator::create(tag, object->group());
            // The assertion typically fails if a component is not configured in
            // the TIFF structure table
            assert(tc.get());
            tc->setStart(p);
            object->addChild(tc);
            p += 12;
        }

        if (object->hasNext()) {
            if (p + 4 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                EXV_ERROR << "Directory " << groupName(object->group())
                          << ": IFD exceeds data buffer, cannot read next pointer.\n";
#endif
                return;
            }
            TiffComponent::AutoPtr tc(0);
            uint32_t next = getLong(p, byteOrder());
            if (next) {
                tc = TiffCreator::create(Tag::next, object->group());
#ifndef SUPPRESS_WARNINGS
                if (tc.get() == 0) {
                    EXV_WARNING << "Directory " << groupName(object->group())
                                << " has an unhandled next pointer.\n";
                }
#endif
            }
            if (tc.get()) {
                if (baseOffset() + next > size_) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Directory " << groupName(object->group())
                              << ": Next pointer is out of bounds; ignored.\n";
#endif
                    return;
                }
                tc->setStart(pData_ + baseOffset() + next);
                object->addNext(tc);
            }
        } // object->hasNext()

    } // TiffReader::visitDirectory

    void TiffReader::visitSubIfd(TiffSubIfd* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        if (   (object->tiffType() == ttUnsignedLong || object->tiffType() == ttSignedLong
                || object->tiffType() == ttTiffIfd)
            && object->count() >= 1) {
            // Todo: Fix hack
            uint32_t maxi = 9;
            if (object->group() == ifd1Id) maxi = 1;
            for (uint32_t i = 0; i < object->count(); ++i) {
                int32_t offset = getLong(object->pData() + 4*i, byteOrder());
                if (   baseOffset() + offset > size_
                    || static_cast<int32_t>(baseOffset()) + offset < 0) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Directory " << groupName(object->group())
                              << ", entry 0x" << std::setw(4)
                              << std::setfill('0') << std::hex << object->tag()
                              << " Sub-IFD pointer " << i
                              << " is out of bounds; ignoring it.\n";
#endif
                    return;
                }
                if (i >= maxi) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Directory " << groupName(object->group())
                                << ", entry 0x" << std::setw(4)
                                << std::setfill('0') << std::hex << object->tag()
                                << ": Skipping sub-IFDs beyond the first " << i << ".\n";
#endif
                    break;
                }
                // If there are multiple dirs, group is incremented for each
                TiffComponent::AutoPtr td(new TiffDirectory(object->tag(),
                                                            static_cast<IfdId>(object->newGroup_ + i)));
                td->setStart(pData_ + baseOffset() + offset);
                object->addChild(td);
            }
        }
#ifndef SUPPRESS_WARNINGS
        else {
            EXV_WARNING << "Directory " << groupName(object->group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << object->tag()
                        << " doesn't look like a sub-IFD.\n";
        }
#endif

    } // TiffReader::visitSubIfd

    void TiffReader::visitMnEntry(TiffMnEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        // Find camera make
        TiffFinder finder(0x010f, ifd0Id);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        std::string make;
        if (te && te->pValue()) {
            make = te->pValue()->toString();
            // create concrete makernote, based on make and makernote contents
            object->mn_ = TiffMnCreator::create(object->tag(),
                                                object->mnGroup_,
                                                make,
                                                object->pData_,
                                                object->size_,
                                                byteOrder());
        }
        if (object->mn_) object->mn_->setStart(object->pData());

    } // TiffReader::visitMnEntry

    void TiffReader::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        object->setImageByteOrder(byteOrder()); // set the byte order for the image

        if (!object->readHeader(object->start(),
                                static_cast<uint32_t>(pLast_ - object->start()),
                                byteOrder())) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "Failed to read "
                      << groupName(object->ifd_.group())
                      << " IFD Makernote header.\n";
#ifdef DEBUG
            if (static_cast<uint32_t>(pLast_ - object->start()) >= 16) {
                hexdump(std::cerr, object->start(), 16);
            }
#endif // DEBUG
#endif // SUPPRESS_WARNINGS
            setGo(geKnownMakernote, false);
            return;
        }

        object->ifd_.setStart(object->start() + object->ifdOffset());

        // Modify reader for Makernote peculiarities, byte order and offset
        object->mnOffset_ = static_cast<uint32_t>(object->start() - pData_);
        TiffRwState::AutoPtr state(
            new TiffRwState(object->byteOrder(), object->baseOffset())
        );
        changeState(state);

    } // TiffReader::visitIfdMakernote

    void TiffReader::visitIfdMakernoteEnd(TiffIfdMakernote* /*object*/)
    {
        // Reset state (byte order, create function, offset) back to that for the image
        resetState();
    } // TiffReader::visitIfdMakernoteEnd

    void TiffReader::readTiffEntry(TiffEntryBase* object)
    {
        assert(object != 0);

        byte* p = object->start();
        assert(p >= pData_);

        if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "Entry in directory " << groupName(object->group())
                      << "requests access to memory beyond the data buffer. "
                      << "Skipping entry.\n";
#endif
            return;
        }
        // Component already has tag
        p += 2;
        TiffType tiffType = getUShort(p, byteOrder());
        TypeId typeId = toTypeId(tiffType, object->tag(), object->group());
        long typeSize = TypeInfo::typeSize(typeId);
        if (0 == typeSize) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(object->group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << object->tag()
                        << " has unknown Exif (TIFF) type " << std::dec << tiffType
                        << "; setting type size 1.\n";
#endif
            typeSize = 1;
        }
        p += 2;
        uint32_t count = getULong(p, byteOrder());
        if (count >= 0x10000000) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "Directory " << groupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " has invalid size "
                      << std::dec << count << "*" << typeSize
                      << "; skipping entry.\n";
#endif
            return;
        }
        p += 4;
        uint32_t size = typeSize * count;
        int32_t offset = getLong(p, byteOrder());
        byte* pData = p;
        if (   size > 4
            && (   baseOffset() + offset >= size_
                || static_cast<int32_t>(baseOffset()) + offset <= 0)) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "Offset of directory " << groupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " is out of bounds: "
                      << "Offset = 0x" << std::setw(8)
                      << std::setfill('0') << std::hex << offset
                      << "; truncating the entry\n";
#endif
                size = 0;
        }
        if (size > 4) {
            pData = const_cast<byte*>(pData_) + baseOffset() + offset;
            if (size > static_cast<uint32_t>(pLast_ - pData)) {
#ifndef SUPPRESS_WARNINGS
                EXV_ERROR << "Upper boundary of data for "
                          << "directory " << groupName(object->group())
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds: "
                          << "Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << offset
                          << ", size = " << std::dec << size
                          << ", exceeds buffer size by "
                          // cast to make MSVC happy
                          << static_cast<uint32_t>(pData + size - pLast_)
                          << " Bytes; truncating the entry\n";
#endif
                size = 0;
            }
        }
        Value::AutoPtr v = Value::create(typeId);
        assert(v.get());
        v->read(pData, size, byteOrder());

        object->setValue(v);
        object->setData(pData, size);
        object->setOffset(offset);
        object->setIdx(nextIdx(object->group()));

    } // TiffReader::readTiffEntry

    void TiffReader::visitBinaryArray(TiffBinaryArray* object)
    {
        assert(object != 0);

        if (!postProc_) {
            // Defer reading children until after all other components are read, but
            // since state (offset) is not set during post-processing, read entry here
            readTiffEntry(object);
            object->iniOrigDataBuf();
            postList_.push_back(object);
            return;
        }

        // Check duplicates
        TiffFinder finder(object->tag(), object->group());
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te && te->idx() != object->idx()) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Not decoding duplicate binary array tag 0x"
                        << std::setw(4) << std::setfill('0') << std::hex
                        << object->tag() << std::dec << ", group "
                        << groupName(object->group()) << ", idx " << object->idx()
                        << "\n";
#endif
            object->setDecoded(false);
            return;
        }

        if (object->TiffEntryBase::doSize() == 0) return;
        if (!object->initialize(pRoot_)) return;
        const ArrayCfg* cfg = object->cfg();
        if (cfg == 0) return;

        const CryptFct cryptFct = cfg->cryptFct_;
        if (cryptFct != 0) {
            const byte* pData = object->pData();
            int32_t size = object->TiffEntryBase::doSize();
            DataBuf buf = cryptFct(object->tag(), pData, size, pRoot_);
            if (buf.size_ > 0) object->setData(buf);
        }

        const ArrayDef* defs = object->def();
        const ArrayDef* defsEnd = defs + object->defSize();
        const ArrayDef* def = &cfg->elDefaultDef_;
        ArrayDef gap = *def;

        for (uint32_t idx = 0; idx < object->TiffEntryBase::doSize(); ) {
            if (defs) {
                def = std::find(defs, defsEnd, idx);
                if (def == defsEnd) {
                    if (cfg->concat_) {
                        // Determine gap-size
                        const ArrayDef* xdef = defs;
                        for (; xdef != defsEnd && xdef->idx_ <= idx; ++xdef) {}
                        uint32_t gapSize = 0;
                        if (xdef != defsEnd && xdef->idx_ > idx) {
                            gapSize = xdef->idx_ - idx;
                        }
                        else {
                            gapSize = object->TiffEntryBase::doSize() - idx;
                        }
                        gap.idx_ = idx;
                        gap.tiffType_ = cfg->elDefaultDef_.tiffType_;
                        gap.count_ = gapSize / cfg->tagStep();
                        if (gap.count_ * cfg->tagStep() != gapSize) {
                            gap.tiffType_ = ttUndefined;
                            gap.count_ = gapSize;
                        }
                        def = &gap;
                    }
                    else {
                        def = &cfg->elDefaultDef_;
                    }
                }
            }
            idx += object->addElement(idx, *def); // idx may be different from def->idx_
        }

    } // TiffReader::visitBinaryArray

    void TiffReader::visitBinaryElement(TiffBinaryElement* object)
    {
        byte* pData   = object->start();
        uint32_t size = object->TiffEntryBase::doSize();
        ByteOrder bo = object->elByteOrder();
        if (bo == invalidByteOrder) bo = byteOrder();
        TypeId typeId = toTypeId(object->elDef()->tiffType_, object->tag(), object->group());
        Value::AutoPtr v = Value::create(typeId);
        assert(v.get());
        v->read(pData, size, bo);

        object->setValue(v);
        object->setOffset(0);
        object->setIdx(nextIdx(object->group()));

    } // TiffReader::visitBinaryElement

}}                                      // namespace Internal, Exiv2
