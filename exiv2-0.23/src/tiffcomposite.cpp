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
  File:      tiffcomposite.cpp
  Version:   $Rev: 2699 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: tiffcomposite.cpp 2699 2012-04-11 16:02:52Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "makernote_int.hpp"
#include "value.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <algorithm>

// *****************************************************************************
namespace {
    //! Add \em tobe - \em curr 0x00 filler bytes if necessary
    uint32_t fillGap(Exiv2::Internal::IoWrapper& ioWrapper, uint32_t curr, uint32_t tobe);
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    bool TiffMappingInfo::operator==(const TiffMappingInfo::Key& key) const
    {
        return    (   0 == strcmp("*", make_)
                   || 0 == strncmp(make_, key.m_.c_str(), strlen(make_)))
               && (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    IoWrapper::IoWrapper(BasicIo& io, const byte* pHeader, long size, OffsetWriter* pow)
        : io_(io), pHeader_(pHeader), size_(size), wroteHeader_(false), pow_(pow)
    {
        if (pHeader_ == 0 || size_ == 0) wroteHeader_ = true;
    }

    long IoWrapper::write(const byte* pData, long wcount)
    {
        if (!wroteHeader_ && wcount > 0) {
            io_.write(pHeader_, size_);
            wroteHeader_ = true;
        }
        return io_.write(pData, wcount);
    }

    int IoWrapper::putb(byte data)
    {
        if (!wroteHeader_) {
            io_.write(pHeader_, size_);
            wroteHeader_ = true;
        }
        return io_.putb(data);
    }

    void IoWrapper::setTarget(int id, uint32_t target)
    {
        if (pow_) pow_->setTarget(OffsetWriter::OffsetId(id), target);
    }

    TiffComponent::TiffComponent(uint16_t tag, IfdId group)
        : tag_(tag), group_(group), pStart_(0)
    {
    }

    TiffEntryBase::TiffEntryBase(uint16_t tag, IfdId group, TiffType tiffType)
        : TiffComponent(tag, group),
          tiffType_(tiffType), count_(0), offset_(0),
          size_(0), pData_(0), isMalloced_(false), idx_(0),
          pValue_(0)
    {
    }

    TiffSubIfd::TiffSubIfd(uint16_t tag, IfdId group, IfdId newGroup)
        : TiffEntryBase(tag, group, ttUnsignedLong), newGroup_(newGroup)
    {
    }

    TiffMnEntry::TiffMnEntry(uint16_t tag, IfdId group, IfdId mnGroup)
        : TiffEntryBase(tag, group, ttUndefined), mnGroup_(mnGroup), mn_(0)
    {
    }

    TiffIfdMakernote::TiffIfdMakernote(uint16_t  tag,
                                       IfdId     group,
                                       IfdId     mnGroup,
                                       MnHeader* pHeader,
                                       bool      hasNext)
        : TiffComponent(tag, group),
          pHeader_(pHeader),
          ifd_(tag, mnGroup, hasNext),
          mnOffset_(0),
          imageByteOrder_(invalidByteOrder)
    {
    }

    TiffBinaryArray::TiffBinaryArray(uint16_t tag,
                                     IfdId group,
                                     const ArrayCfg* arrayCfg,
                                     const ArrayDef* arrayDef,
                                     int defSize)
        : TiffEntryBase(tag, group, arrayCfg->elTiffType_),
          cfgSelFct_(0),
          arraySet_(0),
          arrayCfg_(arrayCfg),
          arrayDef_(arrayDef),
          defSize_(defSize),
          setSize_(0),
          origData_(0),
          origSize_(0),
          pRoot_(0),
          decoded_(false)
    {
        assert(arrayCfg != 0);
    }

    TiffBinaryArray::TiffBinaryArray(uint16_t tag,
                                     IfdId group,
                                     const ArraySet* arraySet,
                                     int setSize,
                                     CfgSelFct cfgSelFct)
        : TiffEntryBase(tag, group), // Todo: Does it make a difference that there is no type?
          cfgSelFct_(cfgSelFct),
          arraySet_(arraySet),
          arrayCfg_(0),
          arrayDef_(0),
          defSize_(0),
          setSize_(setSize),
          origData_(0),
          origSize_(0),
          pRoot_(0),
          decoded_(false)
    {
        // We'll figure out the correct cfg later
        assert(cfgSelFct != 0);
        assert(arraySet_ != 0);
    }

    TiffBinaryElement::TiffBinaryElement(uint16_t tag, IfdId group)
        : TiffEntryBase(tag, group)
    {
    }

    TiffComponent::~TiffComponent()
    {
    }

    TiffDirectory::~TiffDirectory()
    {
        for (Components::iterator i = components_.begin(); i != components_.end(); ++i) {
            delete *i;
        }
        delete pNext_;
    }

    TiffSubIfd::~TiffSubIfd()
    {
        for (Ifds::iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            delete *i;
        }
    }

    TiffEntryBase::~TiffEntryBase()
    {
        if (isMalloced_) {
            delete[] pData_;
        }
        delete pValue_;
    }

    TiffEntry::~TiffEntry()
    {
    }

    TiffDataEntryBase::~TiffDataEntryBase()
    {
    }

    TiffDataEntry::~TiffDataEntry()
    {
    }

    TiffImageEntry::~TiffImageEntry()
    {
    }

    TiffSizeEntry::~TiffSizeEntry()
    {
    }

    TiffMnEntry::~TiffMnEntry()
    {
        delete mn_;
    }

    TiffIfdMakernote::~TiffIfdMakernote()
    {
        delete pHeader_;
    }

    TiffBinaryArray::~TiffBinaryArray()
    {
        for (Components::iterator i = elements_.begin(); i != elements_.end(); ++i) {
            delete *i;
        }
    }

    TiffBinaryElement::~TiffBinaryElement()
    {
    }

    TiffEntryBase::TiffEntryBase(const TiffEntryBase& rhs)
        : TiffComponent(rhs),
          tiffType_(rhs.tiffType_),
          count_(rhs.count_),
          offset_(rhs.offset_),
          size_(rhs.size_),
          pData_(rhs.pData_),
          isMalloced_(rhs.isMalloced_),
          idx_(rhs.idx_),
          pValue_(rhs.pValue_ ? rhs.pValue_->clone().release() : 0)
    {
        if (rhs.isMalloced_) {
            pData_ = new byte[rhs.size_];
            memcpy(pData_, rhs.pData_, rhs.size_);
        }
    }

    TiffDirectory::TiffDirectory(const TiffDirectory& rhs)
        : TiffComponent(rhs),
          hasNext_(rhs.hasNext_),
          pNext_(0)
    {
    }

    TiffSubIfd::TiffSubIfd(const TiffSubIfd& rhs)
        : TiffEntryBase(rhs),
          newGroup_(rhs.newGroup_)
    {
    }

    TiffBinaryArray::TiffBinaryArray(const TiffBinaryArray& rhs)
        : TiffEntryBase(rhs),
          cfgSelFct_(rhs.cfgSelFct_),
          arraySet_(rhs.arraySet_),
          arrayCfg_(rhs.arrayCfg_),
          arrayDef_(rhs.arrayDef_),
          defSize_(rhs.defSize_),
          setSize_(rhs.setSize_),
          origData_(rhs.origData_),
          origSize_(rhs.origSize_),
          pRoot_(rhs.pRoot_),
          decoded_(false)
    {
    }

    TiffComponent::AutoPtr TiffComponent::clone() const
    {
        return AutoPtr(doClone());
    }

    TiffEntry* TiffEntry::doClone() const
    {
        return new TiffEntry(*this);
    }

    TiffDataEntry* TiffDataEntry::doClone() const
    {
        return new TiffDataEntry(*this);
    }

    TiffImageEntry* TiffImageEntry::doClone() const
    {
        return new TiffImageEntry(*this);
    }

    TiffSizeEntry* TiffSizeEntry::doClone() const
    {
        return new TiffSizeEntry(*this);
    }

    TiffDirectory* TiffDirectory::doClone() const
    {
        return new TiffDirectory(*this);
    }

    TiffSubIfd* TiffSubIfd::doClone() const
    {
        return new TiffSubIfd(*this);
    }

    TiffMnEntry* TiffMnEntry::doClone() const
    {
        assert(false); // Not implemented
        return 0;
    }

    TiffIfdMakernote* TiffIfdMakernote::doClone() const
    {
        assert(false); // Not implemented
        return 0;
    }

    TiffBinaryArray* TiffBinaryArray::doClone() const
    {
        return new TiffBinaryArray(*this);
    }

    TiffBinaryElement* TiffBinaryElement::doClone() const
    {
        return new TiffBinaryElement(*this);
    }

    int TiffComponent::idx() const
    {
        return 0;
    }

    int TiffEntryBase::idx() const
    {
        return idx_;
    }

    void TiffEntryBase::setData(DataBuf buf)
    {
        std::pair<byte*, long> p = buf.release();
        setData(p.first, p.second);
        isMalloced_ = true;
    }

    void TiffEntryBase::setData(byte* pData, int32_t size)
    {
        if (isMalloced_) {
            delete[] pData_;
        }
        pData_ = pData;
        size_  = size;
        if (pData_ == 0) size_ = 0;
    }

    void TiffEntryBase::updateValue(Value::AutoPtr value, ByteOrder byteOrder)
    {
        if (value.get() == 0) return;
        uint32_t newSize = value->size();
        if (newSize > size_) {
            setData(DataBuf(newSize));
        }
        memset(pData_, 0x0, size_);
        size_ = value->copy(pData_, byteOrder);
        assert(size_ == newSize);
        setValue(value);
    } // TiffEntryBase::updateValue

    void TiffEntryBase::setValue(Value::AutoPtr value)
    {
        if (value.get() == 0) return;
        tiffType_ = toTiffType(value->typeId());
        count_ = value->count();
        delete pValue_;
        pValue_ = value.release();
    } // TiffEntryBase::setValue

    void TiffDataEntry::setStrips(const Value* pSize,
                                  const byte*  pData,
                                  uint32_t     sizeData,
                                  uint32_t     baseOffset)
    {
        if (!pValue() || !pSize) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << ": Size or data offset value not set, ignoring them.\n";
#endif
            return;
        }
        if (pValue()->count() == 0) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << ": Data offset entry value is empty, ignoring it.\n";
#endif
            return;
        }
        if (pValue()->count() != pSize->count()) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << ": Size and data offset entries have different"
                        << " number of components, ignoring them.\n";
#endif
            return;
        }
        uint32_t size = 0;
        for (int i = 0; i < pSize->count(); ++i) {
            size += static_cast<uint32_t>(pSize->toLong(i));
        }
        uint32_t offset = static_cast<uint32_t>(pValue()->toLong(0));
        // Todo: Remove limitation of JPEG writer: strips must be contiguous
        // Until then we check: last offset + last size - first offset == size?
        if (  static_cast<uint32_t>(pValue()->toLong(pValue()->count()-1))
            + static_cast<uint32_t>(pSize->toLong(pSize->count()-1))
            - offset != size) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << ": Data area is not contiguous, ignoring it.\n";
#endif
            return;
        }
        if (   offset > sizeData
            || size > sizeData
            || baseOffset + offset > sizeData - size) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << ": Data area exceeds data buffer, ignoring it.\n";
#endif
            return;
        }
        pDataArea_ = const_cast<byte*>(pData) + baseOffset + offset;
        sizeDataArea_ = size;
        const_cast<Value*>(pValue())->setDataArea(pDataArea_, sizeDataArea_);
    } // TiffDataEntry::setStrips

    void TiffImageEntry::setStrips(const Value* pSize,
                                   const byte*  pData,
                                   uint32_t     sizeData,
                                   uint32_t     baseOffset)
    {
        if (!pValue() || !pSize) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << ": Size or data offset value not set, ignoring them.\n";
#endif
            return;
        }
        if (pValue()->count() != pSize->count()) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << ": Size and data offset entries have different"
                        << " number of components, ignoring them.\n";
#endif
            return;
        }
        for (int i = 0; i < pValue()->count(); ++i) {
            const uint32_t offset = static_cast<uint32_t>(pValue()->toLong(i));
            const byte* pStrip = pData + baseOffset + offset;
            const uint32_t size = static_cast<uint32_t>(pSize->toLong(i));

            if (   offset > sizeData
                || size > sizeData
                || baseOffset + offset > sizeData - size) {
#ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Directory " << groupName(group())
                            << ", entry 0x" << std::setw(4)
                            << std::setfill('0') << std::hex << tag()
                            << ": Strip " << std::dec << i
                            << " is outside of the data area; ignored.\n";
#endif
            }
            else if (size != 0) {
                strips_.push_back(std::make_pair(pStrip, size));
            }
        }
    } // TiffImageEntry::setStrips

    uint32_t TiffIfdMakernote::ifdOffset() const
    {
        if (!pHeader_) return 0;
        return pHeader_->ifdOffset();
    }

    ByteOrder TiffIfdMakernote::byteOrder() const
    {
        assert(imageByteOrder_ != invalidByteOrder);
        if (!pHeader_ || pHeader_->byteOrder() == invalidByteOrder) {
            return imageByteOrder_;
        }
        return pHeader_->byteOrder();
    }

    uint32_t TiffIfdMakernote::mnOffset() const
    {
        return mnOffset_;
    }

    uint32_t TiffIfdMakernote::baseOffset() const
    {
        if (!pHeader_) return 0;
        return pHeader_->baseOffset(mnOffset_);
    }

    bool TiffIfdMakernote::readHeader(const byte* pData,
                                      uint32_t    size,
                                      ByteOrder   byteOrder)
    {
        if (!pHeader_) return true;
        return pHeader_->read(pData, size, byteOrder);
    }

    void TiffIfdMakernote::setByteOrder(ByteOrder byteOrder)
    {
        if (pHeader_) pHeader_->setByteOrder(byteOrder);
    }

    uint32_t TiffIfdMakernote::sizeHeader() const
    {
        if (!pHeader_) return 0;
        return pHeader_->size();
    }

    uint32_t TiffIfdMakernote::writeHeader(IoWrapper& ioWrapper, ByteOrder byteOrder) const
    {
        if (!pHeader_) return 0;
        return pHeader_->write(ioWrapper, byteOrder);
    }

    uint32_t ArrayDef::size(uint16_t tag, IfdId group) const
    {
        TypeId typeId = toTypeId(tiffType_, tag, group);
        return count_ * TypeInfo::typeSize(typeId);
    }

    bool TiffBinaryArray::initialize(IfdId group)
    {
        if (arrayCfg_ != 0) return true; // Not a complex array or already initialized

        for (int idx = 0; idx < setSize_; ++idx) {
            if (arraySet_[idx].cfg_.group_ == group) {
                arrayCfg_ = &arraySet_[idx].cfg_;
                arrayDef_ = arraySet_[idx].def_;
                defSize_  = arraySet_[idx].defSize_;
                return true;
            }
        }
        return false;
    }

    bool TiffBinaryArray::initialize(TiffComponent* const pRoot)
    {
        if (cfgSelFct_ == 0) return true; // Not a complex array

        int idx = cfgSelFct_(tag(), pData(), TiffEntryBase::doSize(), pRoot);
        if (idx > -1) {
            arrayCfg_ = &arraySet_[idx].cfg_;
            arrayDef_ = arraySet_[idx].def_;
            defSize_  = arraySet_[idx].defSize_;
        }
        return idx > -1;
    }

    void TiffBinaryArray::iniOrigDataBuf()
    {
        origData_ = const_cast<byte*>(pData());
        origSize_ = TiffEntryBase::doSize();
    }

    bool TiffBinaryArray::updOrigDataBuf(const byte* pData, uint32_t size)
    {
        assert(pData != 0);

        if (origSize_ != size) return false;
        if (origData_ == pData) return true;
        memcpy(origData_, pData, origSize_);
        return true;
    }

    uint32_t TiffBinaryArray::addElement(uint32_t idx, const ArrayDef& def)
    {
        uint16_t tag = static_cast<uint16_t>(idx / cfg()->tagStep());
        int32_t sz = EXV_MIN(def.size(tag, cfg()->group_), TiffEntryBase::doSize() - idx);
        TiffComponent::AutoPtr tc = TiffCreator::create(tag, cfg()->group_);
        TiffBinaryElement* tp = dynamic_cast<TiffBinaryElement*>(tc.get());
        // The assertion typically fails if a component is not configured in
        // the TIFF structure table (TiffCreator::tiffTreeStruct_)
        assert(tp);
        tp->setStart(pData() + idx);
        tp->setData(const_cast<byte*>(pData() + idx), sz);
        tp->setElDef(def);
        tp->setElByteOrder(cfg()->byteOrder_);
        addChild(tc);
        return sz;
    } // TiffBinaryArray::addElement

    TiffComponent* TiffComponent::addPath(uint16_t tag,
                                          TiffPath& tiffPath,
                                          TiffComponent* const pRoot,
                                          TiffComponent::AutoPtr object)
    {
        return doAddPath(tag, tiffPath, pRoot, object);
    } // TiffComponent::addPath

    TiffComponent* TiffComponent::doAddPath(uint16_t  /*tag*/,
                                            TiffPath& /*tiffPath*/,
                                            TiffComponent* const /*pRoot*/,
                                            TiffComponent::AutoPtr /*object*/)
    {
        return this;
    } // TiffComponent::doAddPath

    TiffComponent* TiffDirectory::doAddPath(uint16_t tag,
                                            TiffPath& tiffPath,
                                            TiffComponent* const pRoot,
                                            TiffComponent::AutoPtr object)
    {
        assert(tiffPath.size() > 1);
        tiffPath.pop();
        const TiffPathItem tpi = tiffPath.top();

        TiffComponent* tc = 0;
        // Try to use an existing component if there is still at least one
        // composite tag on the stack or the tag to add is the MakerNote tag.
        // This is used to prevent duplicate entries. Sub-IFDs also, but the > 1
        // condition takes care of them, see below.
        if (   tiffPath.size() > 1
            || (tpi.extendedTag() == 0x927c && tpi.group() == exifId)) {
            if (tpi.extendedTag() == Tag::next) {
                tc = pNext_;
            }
            else {
                for (Components::iterator i = components_.begin(); i != components_.end(); ++i) {
                    if ((*i)->tag() == tpi.tag() && (*i)->group() == tpi.group()) {
                        tc = *i;
                        break;
                    }
                }
            }
        }
        if (tc == 0) {
            TiffComponent::AutoPtr atc;
            if (tiffPath.size() == 1 && object.get() != 0) {
                atc = object;
            }
            else {
                atc = TiffCreator::create(tpi.extendedTag(), tpi.group());
            }
            assert(atc.get() != 0);

            // Prevent dangling sub-IFD tags: Do not add a sub-IFD component without children.
            // Todo: How to check before creating the component?
            if (tiffPath.size() == 1 && dynamic_cast<TiffSubIfd*>(atc.get()) != 0) return 0;

            if (tpi.extendedTag() == Tag::next) {
                tc = this->addNext(atc);
            }
            else {
                tc = this->addChild(atc);
            }
        }
        return tc->addPath(tag, tiffPath, pRoot, object);
    } // TiffDirectory::doAddPath

    TiffComponent* TiffSubIfd::doAddPath(uint16_t tag,
                                         TiffPath& tiffPath,
                                         TiffComponent* const pRoot,
                                         TiffComponent::AutoPtr object)
    {
        assert(!tiffPath.empty());
        const TiffPathItem tpi1 = tiffPath.top();
        tiffPath.pop();
        if (tiffPath.empty()) {
            // If the last element in the path is the sub-IFD tag itself we're done.
            // But that shouldn't happen - see TiffDirectory::doAddPath
            return this;
        }
        const TiffPathItem tpi2 = tiffPath.top();
        tiffPath.push(tpi1);
        TiffComponent* tc = 0;
        for (Ifds::iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            if ((*i)->group() == tpi2.group()) {
                tc = *i;
                break;
            }
        }
        if (tc == 0) {
            if (tiffPath.size() == 1 && object.get() != 0) {
                tc = addChild(object);
            }
            else {
                TiffComponent::AutoPtr atc(new TiffDirectory(tpi1.tag(), tpi2.group()));
                tc = addChild(atc);
            }
            setCount(static_cast<uint32_t>(ifds_.size()));
        }
        return tc->addPath(tag, tiffPath, pRoot, object);
    } // TiffSubIfd::doAddPath

    TiffComponent* TiffMnEntry::doAddPath(uint16_t tag,
                                          TiffPath& tiffPath,
                                          TiffComponent* const pRoot,
                                          TiffComponent::AutoPtr object)
    {
        assert(!tiffPath.empty());
        const TiffPathItem tpi1 = tiffPath.top();
        tiffPath.pop();
        if (tiffPath.empty()) {
            // If the last element in the path is the makernote tag itself we're done
            return this;
        }
        const TiffPathItem tpi2 = tiffPath.top();
        tiffPath.push(tpi1);
        if (mn_ == 0) {
            mnGroup_ = tpi2.group();
            mn_ = TiffMnCreator::create(tpi1.tag(), tpi1.group(), mnGroup_);
            assert(mn_);
        }
        return mn_->addPath(tag, tiffPath, pRoot, object);
    } // TiffMnEntry::doAddPath

    TiffComponent* TiffIfdMakernote::doAddPath(uint16_t tag,
                                               TiffPath& tiffPath,
                                               TiffComponent* const pRoot,
                                               TiffComponent::AutoPtr object)
    {
        return ifd_.addPath(tag, tiffPath, pRoot, object);
    }

    TiffComponent* TiffBinaryArray::doAddPath(uint16_t tag,
                                              TiffPath& tiffPath,
                                              TiffComponent* const pRoot,
                                              TiffComponent::AutoPtr object)
    {
        pRoot_ = pRoot;
        if (tiffPath.size() == 1) {
            // An unknown complex binary array has no children and acts like a standard TIFF entry
            return this;
        }
        tiffPath.pop();
        const TiffPathItem tpi = tiffPath.top();
        // Initialize the binary array (if it is a complex array)
        initialize(tpi.group());
        TiffComponent* tc = 0;
        // Todo: Duplicates are not allowed!
        // To allow duplicate entries, we only check if the new component already
        // exists if there is still at least one composite tag on the stack
        if (tiffPath.size() > 1) {
            for (Components::iterator i = elements_.begin(); i != elements_.end(); ++i) {
                if ((*i)->tag() == tpi.tag() && (*i)->group() == tpi.group()) {
                    tc = *i;
                    break;
                }
            }
        }
        if (tc == 0) {
            TiffComponent::AutoPtr atc;
            if (tiffPath.size() == 1 && object.get() != 0) {
                atc = object;
            }
            else {
                atc = TiffCreator::create(tpi.extendedTag(), tpi.group());
            }
            assert(atc.get() != 0);
            assert(tpi.extendedTag() != Tag::next);
            tc = addChild(atc);
            setCount(static_cast<uint32_t>(elements_.size()));
        }
        return tc->addPath(tag, tiffPath, pRoot, object);
    } // TiffBinaryArray::doAddPath

    TiffComponent* TiffComponent::addChild(TiffComponent::AutoPtr tiffComponent)
    {
        return doAddChild(tiffComponent);
    } // TiffComponent::addChild

    TiffComponent* TiffComponent::doAddChild(AutoPtr /*tiffComponent*/)
    {
        return 0;
    } // TiffComponent::doAddChild

    TiffComponent* TiffDirectory::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = tiffComponent.release();
        components_.push_back(tc);
        return tc;
    } // TiffDirectory::doAddChild

    TiffComponent* TiffSubIfd::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffDirectory* d = dynamic_cast<TiffDirectory*>(tiffComponent.release());
        assert(d);
        ifds_.push_back(d);
        return d;
    } // TiffSubIfd::doAddChild

    TiffComponent* TiffMnEntry::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (mn_) {
            tc =  mn_->addChild(tiffComponent);
        }
        return tc;
    } // TiffMnEntry::doAddChild

    TiffComponent* TiffIfdMakernote::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        return ifd_.addChild(tiffComponent);
    }

    TiffComponent* TiffBinaryArray::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = tiffComponent.release();
        elements_.push_back(tc);
        setDecoded(true);
        return tc;
    } // TiffBinaryArray::doAddChild

    TiffComponent* TiffComponent::addNext(TiffComponent::AutoPtr tiffComponent)
    {
        return doAddNext(tiffComponent);
    } // TiffComponent::addNext

    TiffComponent* TiffComponent::doAddNext(AutoPtr /*tiffComponent*/)
    {
        return 0;
    } // TiffComponent::doAddNext

    TiffComponent* TiffDirectory::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (hasNext_) {
            tc = tiffComponent.release();
            pNext_ = tc;
        }
        return tc;
    } // TiffDirectory::doAddNext

    TiffComponent* TiffMnEntry::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (mn_) {
            tc = mn_->addNext(tiffComponent);
        }
        return tc;
    } // TiffMnEntry::doAddNext

    TiffComponent* TiffIfdMakernote::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        return ifd_.addNext(tiffComponent);
    }

    void TiffComponent::accept(TiffVisitor& visitor)
    {
        if (visitor.go(TiffVisitor::geTraverse)) doAccept(visitor); // one for NVI :)
    } // TiffComponent::accept

    void TiffEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitEntry(this);
    } // TiffEntry::doAccept

    void TiffDataEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDataEntry(this);
    } // TiffDataEntry::doAccept

    void TiffImageEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitImageEntry(this);
    } // TiffImageEntry::doAccept

    void TiffSizeEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSizeEntry(this);
    } // TiffSizeEntry::doAccept

    void TiffDirectory::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDirectory(this);
        for (Components::const_iterator i = components_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != components_.end(); ++i) {
            (*i)->accept(visitor);
        }
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitDirectoryNext(this);
        if (pNext_) pNext_->accept(visitor);
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitDirectoryEnd(this);
    } // TiffDirectory::doAccept

    void TiffSubIfd::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSubIfd(this);
        for (Ifds::iterator i = ifds_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != ifds_.end(); ++i) {
            (*i)->accept(visitor);
        }
    } // TiffSubIfd::doAccept

    void TiffMnEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitMnEntry(this);
        if (mn_) mn_->accept(visitor);
        if (!visitor.go(TiffVisitor::geKnownMakernote)) {
            delete mn_;
            mn_ = 0;
        }

    } // TiffMnEntry::doAccept

    void TiffIfdMakernote::doAccept(TiffVisitor& visitor)
    {
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitIfdMakernote(this);
        if (visitor.go(TiffVisitor::geKnownMakernote)) ifd_.accept(visitor);
        if (   visitor.go(TiffVisitor::geKnownMakernote)
            && visitor.go(TiffVisitor::geTraverse)) visitor.visitIfdMakernoteEnd(this);
    }

    void TiffBinaryArray::doAccept(TiffVisitor& visitor)
    {
        visitor.visitBinaryArray(this);
        for (Components::const_iterator i = elements_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != elements_.end(); ++i) {
            (*i)->accept(visitor);
        }
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitBinaryArrayEnd(this);
    } // TiffBinaryArray::doAccept

    void TiffBinaryElement::doAccept(TiffVisitor& visitor)
    {
        visitor.visitBinaryElement(this);
    } // TiffBinaryElement::doAccept

    void TiffEntryBase::encode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        doEncode(encoder, datum);
    } // TiffComponent::encode

    void TiffBinaryElement::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeBinaryElement(this, datum);
    } // TiffBinaryElement::doEncode

    void TiffBinaryArray::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeBinaryArray(this, datum);
    } // TiffBinaryArray::doEncode

    void TiffDataEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeDataEntry(this, datum);
    } // TiffDataEntry::doEncode

    void TiffEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeTiffEntry(this, datum);
    } // TiffEntry::doEncode

    void TiffImageEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeImageEntry(this, datum);
    } // TiffImageEntry::doEncode

    void TiffMnEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeMnEntry(this, datum);
    } // TiffMnEntry::doEncode

    void TiffSizeEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeSizeEntry(this, datum);
    } // TiffSizeEntry::doEncode

    void TiffSubIfd::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeSubIfd(this, datum);
    } // TiffSubIfd::doEncode

    uint32_t TiffComponent::count() const
    {
        return doCount();
    }

    uint32_t TiffDirectory::doCount() const
    {
        return static_cast<uint32_t>(components_.size());
    }

    uint32_t TiffEntryBase::doCount() const
    {
        return count_;
    }

    uint32_t TiffMnEntry::doCount() const
    {
        if (!mn_) {
            return TiffEntryBase::doCount();
        }
        // Count of IFD makernote in tag Exif.Photo.MakerNote is the size of the
        // Makernote in bytes
        assert(tiffType() == ttUndefined || tiffType() == ttUnsignedByte || tiffType() == ttSignedByte);
        return mn_->size();
    }

    uint32_t TiffIfdMakernote::doCount() const
    {
        return ifd_.count();
    } // TiffIfdMakernote::doCount

    uint32_t TiffBinaryArray::doCount() const
    {
        if (cfg() == 0 || !decoded()) return TiffEntryBase::doCount();

        if (elements_.empty()) return 0;

        TypeId typeId = toTypeId(tiffType(), tag(), group());
        long typeSize = TypeInfo::typeSize(typeId);
        if (0 == typeSize) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Directory " << groupName(group())
                        << ", entry 0x" << std::setw(4)
                        << std::setfill('0') << std::hex << tag()
                        << " has unknown Exif (TIFF) type " << std::dec << tiffType()
                        << "; setting type size 1.\n";
#endif
            typeSize = 1;
        }

        return static_cast<uint32_t>(static_cast<double>(size()) / typeSize + 0.5);
    }

    uint32_t TiffBinaryElement::doCount() const
    {
        return elDef_.count_;
    }

    uint32_t TiffComponent::write(IoWrapper& ioWrapper,
                                  ByteOrder byteOrder,
                                  int32_t   offset,
                                  uint32_t  valueIdx,
                                  uint32_t  dataIdx,
                                  uint32_t& imageIdx)
    {
        return doWrite(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
    } // TiffComponent::write

    uint32_t TiffDirectory::doWrite(IoWrapper& ioWrapper,
                                    ByteOrder byteOrder,
                                    int32_t   offset,
                                    uint32_t  valueIdx,
                                    uint32_t  dataIdx,
                                    uint32_t& imageIdx)
    {
        bool isRootDir = (imageIdx == uint32_t(-1));

        // Number of components to write
        const uint32_t compCount = count();
        if (compCount > 0xffff) throw Error(49, groupName(group()));

        // Size of next IFD, if any
        uint32_t sizeNext = 0;
        if (pNext_) sizeNext = pNext_->size();

        // Nothing to do if there are no entries and the size of the next IFD is 0
        if (compCount == 0 && sizeNext == 0) return 0;

        // Remember the offset of the CR2 RAW IFD
        if (group() == ifd3Id) {
#ifdef DEBUG
            std::cerr << "Directory " << groupName(group()) << " offset is 0x"
                      << std::setw(8) << std::setfill('0') << std::hex << offset << std::dec
                      << "\n";
#endif
            ioWrapper.setTarget(OffsetWriter::cr2RawIfdOffset, offset);
        }
        // Size of all directory entries, without values and additional data
        const uint32_t sizeDir = 2 + 12 * compCount + (hasNext_ ? 4 : 0);

        // TIFF standard requires IFD entries to be sorted in ascending order by tag.
        // Not sorting makernote directories sometimes preserves them better.
        if (group() < mnId) {
            std::sort(components_.begin(), components_.end(), cmpTagLt);
        }
        // Size of IFD values and additional data
        uint32_t sizeValue = 0;
        uint32_t sizeData = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                sv += sv & 1;               // Align value to word boundary
                sizeValue += sv;
            }
            // Also add the size of data, but only if needed
            if (isRootDir) {
                uint32_t sd = (*i)->sizeData();
                sd += sd & 1;               // Align data to word boundary
                sizeData += sd;
            }
        }

        uint32_t idx = 0;                   // Current IFD index / bytes written
        valueIdx = sizeDir;                 // Offset to the current IFD value
        dataIdx  = sizeDir + sizeValue;     // Offset to the entry's data area
        if (isRootDir) {                    // Absolute offset to the image data
            imageIdx = offset + dataIdx + sizeData + sizeNext;
            imageIdx += imageIdx & 1;       // Align image data to word boundary
        }

        // 1st: Write the IFD, a) Number of directory entries
        byte buf[4];
        us2Data(buf, static_cast<uint16_t>(compCount), byteOrder);
        ioWrapper.write(buf, 2);
        idx += 2;
        // b) Directory entries - may contain pointers to the value or data
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            idx += writeDirEntry(ioWrapper, byteOrder, offset, *i, valueIdx, dataIdx, imageIdx);
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                sv += sv & 1;               // Align value to word boundary
                valueIdx += sv;
            }
            uint32_t sd = (*i)->sizeData();
            sd += sd & 1;                   // Align data to word boundary
            dataIdx += sd;
        }
        // c) Pointer to the next IFD
        if (hasNext_) {
            memset(buf, 0x0, 4);
            if (pNext_ && sizeNext) {
                l2Data(buf, offset + dataIdx, byteOrder);
            }
            ioWrapper.write(buf, 4);
            idx += 4;
        }
        assert(idx == sizeDir);

        // 2nd: Write IFD values - may contain pointers to additional data
        valueIdx = sizeDir;
        dataIdx = sizeDir + sizeValue;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                uint32_t d = (*i)->write(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
                assert(sv == d);
                if ((sv & 1) == 1) {
                    ioWrapper.putb(0x0);    // Align value to word boundary
                    sv += 1;
                }
                idx += sv;
                valueIdx += sv;
            }
            uint32_t sd = (*i)->sizeData();
            sd += sd & 1;                   // Align data to word boundary
            dataIdx += sd;
        }
        assert(idx == sizeDir + sizeValue);

        // 3rd: Write data - may contain offsets too (eg sub-IFD)
        dataIdx = sizeDir + sizeValue;
        idx += writeData(ioWrapper, byteOrder, offset, dataIdx, imageIdx);

        // 4th: Write next-IFD
        if (pNext_ && sizeNext) {
            idx += pNext_->write(ioWrapper, byteOrder, offset + idx, uint32_t(-1), uint32_t(-1), imageIdx);
        }

        // 5th, at the root directory level only: write image data
        if (isRootDir) {
            idx += writeImage(ioWrapper, byteOrder);
        }

        return idx;
    } // TiffDirectory::doWrite

    uint32_t TiffDirectory::writeDirEntry(IoWrapper&     ioWrapper,
                                          ByteOrder      byteOrder,
                                          int32_t        offset,
                                          TiffComponent* pTiffComponent,
                                          uint32_t       valueIdx,
                                          uint32_t       dataIdx,
                                          uint32_t&      imageIdx) const
    {
        assert(pTiffComponent);
        TiffEntryBase* pDirEntry = dynamic_cast<TiffEntryBase*>(pTiffComponent);
        assert(pDirEntry);
        byte buf[8];
        us2Data(buf,     pDirEntry->tag(),      byteOrder);
        us2Data(buf + 2, pDirEntry->tiffType(), byteOrder);
        ul2Data(buf + 4, pDirEntry->count(),    byteOrder);
        ioWrapper.write(buf, 8);
        if (pDirEntry->size() > 4) {
            pDirEntry->setOffset(offset + static_cast<int32_t>(valueIdx));
            l2Data(buf, pDirEntry->offset(), byteOrder);
            ioWrapper.write(buf, 4);
        }
        else {
            const uint32_t len = pDirEntry->write(ioWrapper,
                                                  byteOrder,
                                                  offset,
                                                  valueIdx,
                                                  dataIdx,
                                                  imageIdx);
            assert(len <= 4);
            if (len < 4) {
                memset(buf, 0x0, 4);
                ioWrapper.write(buf, 4 - len);
            }
        }
        return 12;
    } // TiffDirectory::writeDirEntry

    uint32_t TiffEntryBase::doWrite(IoWrapper& ioWrapper,
                                    ByteOrder byteOrder,
                                    int32_t   /*offset*/,
                                    uint32_t  /*valueIdx*/,
                                    uint32_t  /*dataIdx*/,
                                    uint32_t& /*imageIdx*/)
    {
        if (!pValue_) return 0;

        DataBuf buf(pValue_->size());
        pValue_->copy(buf.pData_, byteOrder);
        ioWrapper.write(buf.pData_, buf.size_);
        return buf.size_;
    } // TiffEntryBase::doWrite

    uint32_t TiffEntryBase::writeOffset(byte*     buf,
                                        int32_t   offset,
                                        TiffType  tiffType,
                                        ByteOrder byteOrder)
    {
        uint32_t rc = 0;
        switch(tiffType) {
        case ttUnsignedShort:
        case ttSignedShort:
            if (static_cast<uint32_t>(offset) > 0xffff) throw Error(26);
            rc = s2Data(buf, static_cast<int16_t>(offset), byteOrder);
            break;
        case ttUnsignedLong:
        case ttSignedLong:
            rc = l2Data(buf, static_cast<int32_t>(offset), byteOrder);
            break;
        default:
            throw Error(27);
            break;
        }
        return rc;
    } // TiffEntryBase::writeOffset

    uint32_t TiffDataEntry::doWrite(IoWrapper& ioWrapper,
                                    ByteOrder byteOrder,
                                    int32_t   offset,
                                    uint32_t  /*valueIdx*/,
                                    uint32_t  dataIdx,
                                    uint32_t& /*imageIdx*/)
    {
        if (!pValue() || pValue()->count() == 0) return 0;

        DataBuf buf(pValue()->size());
        uint32_t idx = 0;
        const long prevOffset = pValue()->toLong(0);
        for (uint32_t i = 0; i < count(); ++i) {
            const long newDataIdx =   pValue()->toLong(i) - prevOffset
                                    + static_cast<long>(dataIdx);
            idx += writeOffset(buf.pData_ + idx,
                               offset + newDataIdx,
                               tiffType(),
                               byteOrder);
        }
        ioWrapper.write(buf.pData_, buf.size_);
        return buf.size_;
    } // TiffDataEntry::doWrite

    uint32_t TiffImageEntry::doWrite(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  /*valueIdx*/,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx)
    {
        uint32_t o2 = imageIdx;
        // For makernotes, write TIFF image data to the data area
        if (group() > mnId) o2 = offset + dataIdx;
#ifdef DEBUG
        std::cerr << "TiffImageEntry, Directory " << groupName(group())
                  << ", entry 0x" << std::setw(4)
                  << std::setfill('0') << std::hex << tag() << std::dec
                  << ": Writing offset " << o2 << "\n";
#endif
        DataBuf buf(static_cast<long>(strips_.size()) * 4);
        memset(buf.pData_, 0x0, buf.size_);
        uint32_t idx = 0;
        for (Strips::const_iterator i = strips_.begin(); i != strips_.end(); ++i) {
            idx += writeOffset(buf.pData_ + idx, o2, tiffType(), byteOrder);
            o2 += i->second;
            o2 += i->second & 1;                // Align strip data to word boundary
            if (!(group() > mnId)) {            // Todo: FIX THIS!! SHOULDN'T USE >
                imageIdx += i->second;
                imageIdx += i->second & 1;      // Align strip data to word boundary
            }
        }
        ioWrapper.write(buf.pData_, buf.size_);
        return buf.size_;
    } // TiffImageEntry::doWrite

    uint32_t TiffSubIfd::doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  /*valueIdx*/,
                                 uint32_t  dataIdx,
                                 uint32_t& /*imageIdx*/)
    {
        DataBuf buf(static_cast<long>(ifds_.size()) * 4);
        uint32_t idx = 0;
        // Sort IFDs by group, needed if image data tags were copied first
        std::sort(ifds_.begin(), ifds_.end(), cmpGroupLt);
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            idx += writeOffset(buf.pData_ + idx, offset + dataIdx, tiffType(), byteOrder);
            dataIdx += (*i)->size();
        }
        ioWrapper.write(buf.pData_, buf.size_);
        return buf.size_;
    } // TiffSubIfd::doWrite

    uint32_t TiffMnEntry::doWrite(IoWrapper& ioWrapper,
                                  ByteOrder byteOrder,
                                  int32_t   offset,
                                  uint32_t  valueIdx,
                                  uint32_t  dataIdx,
                                  uint32_t& imageIdx)
    {
        if (!mn_) {
            return TiffEntryBase::doWrite(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
        }
        return mn_->write(ioWrapper, byteOrder, offset + valueIdx, uint32_t(-1), uint32_t(-1), imageIdx);
    } // TiffMnEntry::doWrite

    uint32_t TiffIfdMakernote::doWrite(IoWrapper& ioWrapper,
                                       ByteOrder byteOrder,
                                       int32_t   offset,
                                       uint32_t  /*valueIdx*/,
                                       uint32_t  /*dataIdx*/,
                                       uint32_t& imageIdx)
    {
        mnOffset_ = offset;
        setImageByteOrder(byteOrder);
        uint32_t len = writeHeader(ioWrapper, this->byteOrder());
        len += ifd_.write(ioWrapper, this->byteOrder(),
                          offset - baseOffset() + len,
                          uint32_t(-1), uint32_t(-1),
                          imageIdx);
        return len;
    } // TiffIfdMakernote::doWrite

    uint32_t TiffBinaryArray::doWrite(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder,
                                      int32_t   offset,
                                      uint32_t  valueIdx,
                                      uint32_t  dataIdx,
                                      uint32_t& imageIdx)
    {
        if (cfg() == 0 || !decoded()) return TiffEntryBase::doWrite(ioWrapper,
                                                                    byteOrder,
                                                                    offset,
                                                                    valueIdx,
                                                                    dataIdx,
                                                                    imageIdx);
        if (cfg()->byteOrder_ != invalidByteOrder) byteOrder = cfg()->byteOrder_;
        // Tags must be sorted in ascending order
        std::sort(elements_.begin(), elements_.end(), cmpTagLt);
        uint32_t idx = 0;
        MemIo mio;
        IoWrapper mioWrapper(mio, 0, 0, 0);
        // Some array entries need to have the size in the first element
        if (cfg()->hasSize_) {
            byte buf[4];
            long elSize = TypeInfo::typeSize(toTypeId(cfg()->elTiffType_, 0, cfg()->group_));
            switch (elSize) {
            case 2:
                idx += us2Data(buf, size(), byteOrder);
                break;
            case 4:
                idx += ul2Data(buf, size(), byteOrder);
                break;
            default:
                assert(false);
            }
            mioWrapper.write(buf, elSize);
        }
        // write all tags of the array (Todo: assumes that there are no duplicates, need check)
        for (Components::const_iterator i = elements_.begin(); i != elements_.end(); ++i) {
            // Skip the manufactured tag, if it exists
            if (cfg()->hasSize_ && (*i)->tag() == 0) continue;
            uint32_t newIdx = (*i)->tag() * cfg()->tagStep();
            idx += fillGap(mioWrapper, idx, newIdx);
            idx += (*i)->write(mioWrapper, byteOrder, offset + newIdx, valueIdx, dataIdx, imageIdx);
        }
        if (cfg()->hasFillers_ && def()) {
            const ArrayDef* lastDef = def() + defSize() - 1;
            uint16_t lastTag = static_cast<uint16_t>(lastDef->idx_ / cfg()->tagStep());
            idx += fillGap(mioWrapper, idx, lastDef->idx_ + lastDef->size(lastTag, cfg()->group_));
        }
        DataBuf buf;
        if (cfg()->cryptFct_) {
            buf = cfg()->cryptFct_(tag(), mio.mmap(), static_cast<uint32_t>(mio.size()), pRoot_);
        }
        if (buf.size_ > 0) {
            ioWrapper.write(buf.pData_, buf.size_);
        }
        else {
            ioWrapper.write(mio.mmap(), static_cast<uint32_t>(mio.size()));
        }

        return idx;
    } // TiffBinaryArray::doWrite

    uint32_t TiffBinaryElement::doWrite(IoWrapper& ioWrapper,
                                        ByteOrder byteOrder,
                                        int32_t   /*offset*/,
                                        uint32_t  /*valueIdx*/,
                                        uint32_t  /*dataIdx*/,
                                        uint32_t& /*imageIdx*/)
    {
        Value const* pv = pValue();
        if (!pv || pv->count() == 0) return 0;
        DataBuf buf(pv->size());
        pv->copy(buf.pData_, byteOrder);
        ioWrapper.write(buf.pData_, buf.size_);
        return buf.size_;
    } // TiffBinaryElement::doWrite

    uint32_t TiffComponent::writeData(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder,
                                      int32_t   offset,
                                      uint32_t  dataIdx,
                                      uint32_t& imageIdx) const
    {
        return doWriteData(ioWrapper, byteOrder, offset, dataIdx, imageIdx);
    } // TiffComponent::writeData

    uint32_t TiffDirectory::doWriteData(IoWrapper& ioWrapper,
                                        ByteOrder byteOrder,
                                        int32_t   offset,
                                        uint32_t  dataIdx,
                                        uint32_t& imageIdx) const
    {
        uint32_t len = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            len += (*i)->writeData(ioWrapper, byteOrder, offset, dataIdx + len, imageIdx);
        }
        return len;
    } // TiffDirectory::doWriteData

    uint32_t TiffEntryBase::doWriteData(IoWrapper&/*ioWrapper*/,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/,
                                        uint32_t& /*imageIdx*/) const
    {
        return 0;
    } // TiffEntryBase::doWriteData

    uint32_t TiffImageEntry::doWriteData(IoWrapper& ioWrapper,
                                         ByteOrder byteOrder,
                                         int32_t   /*offset*/,
                                         uint32_t  /*dataIdx*/,
                                         uint32_t& /*imageIdx*/) const
    {
        uint32_t len = 0;
        // For makernotes, write TIFF image data to the data area
        if (group() > mnId) { // Todo: FIX THIS HACK!!!
            len = writeImage(ioWrapper, byteOrder);
        }
        return len;
    } // TiffImageEntry::doWriteData

    uint32_t TiffDataEntry::doWriteData(IoWrapper& ioWrapper,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/,
                                        uint32_t& /*imageIdx*/) const
    {
        if (!pValue()) return 0;

        DataBuf buf = pValue()->dataArea();
        ioWrapper.write(buf.pData_, buf.size_);
        // Align data to word boundary
        uint32_t align = (buf.size_ & 1);
        if (align) ioWrapper.putb(0x0);

        return buf.size_ + align;
    } // TiffDataEntry::doWriteData

    uint32_t TiffSubIfd::doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len  += (*i)->write(ioWrapper, byteOrder, offset + dataIdx + len, uint32_t(-1), uint32_t(-1), imageIdx);
        }
        // Align data to word boundary
        uint32_t align = (len & 1);
        if (align) ioWrapper.putb(0x0);

        return len + align;
    } // TiffSubIfd::doWriteData

    uint32_t TiffIfdMakernote::doWriteData(IoWrapper&/*ioWrapper*/,
                                           ByteOrder /*byteOrder*/,
                                           int32_t   /*offset*/,
                                           uint32_t  /*dataIdx*/,
                                           uint32_t& /*imageIdx*/) const
    {
        assert(false);
        return 0;
    } // TiffIfdMakernote::doWriteData

    uint32_t TiffComponent::writeImage(IoWrapper& ioWrapper,
                                       ByteOrder byteOrder) const
    {
        return doWriteImage(ioWrapper, byteOrder);
    } // TiffComponent::writeImage

    uint32_t TiffDirectory::doWriteImage(IoWrapper& ioWrapper,
                                         ByteOrder byteOrder) const
    {
        uint32_t len = 0;
        TiffComponent* pSubIfd = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            if ((*i)->tag() == 0x014a) {
                // Hack: delay writing of sub-IFD image data to get the order correct
                assert(pSubIfd == 0);
                pSubIfd = *i;
                continue;
            }
            len += (*i)->writeImage(ioWrapper, byteOrder);
        }
        if (pSubIfd) {
            len += pSubIfd->writeImage(ioWrapper, byteOrder);
        }
        if (pNext_) {
            len += pNext_->writeImage(ioWrapper, byteOrder);
        }
        return len;
    } // TiffDirectory::doWriteImage

    uint32_t TiffEntryBase::doWriteImage(IoWrapper&/*ioWrapper*/,
                                         ByteOrder /*byteOrder*/) const
    {
        return 0;
    } // TiffEntryBase::doWriteImage

    uint32_t TiffSubIfd::doWriteImage(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder) const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len  += (*i)->writeImage(ioWrapper, byteOrder);
        }
        return len;
    } // TiffSubIfd::doWriteImage

    uint32_t TiffIfdMakernote::doWriteImage(IoWrapper& ioWrapper,
                                            ByteOrder byteOrder) const
    {
        if (this->byteOrder() != invalidByteOrder) {
            byteOrder = this->byteOrder();
        }
        uint32_t len = ifd_.writeImage(ioWrapper, byteOrder);
        return len;
    } // TiffIfdMakernote::doWriteImage

    uint32_t TiffImageEntry::doWriteImage(IoWrapper& ioWrapper,
                                          ByteOrder  /*byteOrder*/) const
    {
        uint32_t len = pValue()->sizeDataArea();
        if (len > 0) {
#ifdef DEBUG
            std::cerr << "TiffImageEntry, Directory " << groupName(group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag() << std::dec
                      << ": Writing data area, size = " << len;
#endif
            DataBuf buf = pValue()->dataArea();
            ioWrapper.write(buf.pData_, buf.size_);
            uint32_t align = len & 1;       // Align image data to word boundary
            if (align) ioWrapper.putb(0x0);
            len += align;
        }
        else {
#ifdef DEBUG
            std::cerr << "TiffImageEntry, Directory " << groupName(group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag() << std::dec
                      << ": Writing " << strips_.size() << " strips";
#endif
            len = 0;
            for (Strips::const_iterator i = strips_.begin(); i != strips_.end(); ++i) {
                ioWrapper.write(i->first, i->second);
                len += i->second;
                uint32_t align = i->second & 1; // Align strip data to word boundary
                if (align) ioWrapper.putb(0x0);
                len += align;
            }
        }
#ifdef DEBUG
        std::cerr << ", len = " << len << " bytes\n";
#endif
        return len;
    } // TiffImageEntry::doWriteImage

    uint32_t TiffComponent::size() const
    {
        return doSize();
    } // TiffComponent::size

    uint32_t TiffDirectory::doSize() const
    {
        uint32_t compCount = count();
        // Size of the directory, without values and additional data
        uint32_t len = 2 + 12 * compCount + (hasNext_ ? 4 : 0);
        // Size of IFD values and data
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                sv += sv & 1;               // Align value to word boundary
                len += sv;
            }
            uint32_t sd = (*i)->sizeData();
            sd += sd & 1;                   // Align data to word boundary
            len += sd;
        }
        // Size of next-IFD, if any
        uint32_t sizeNext = 0;
        if (pNext_) {
            sizeNext = pNext_->size();
            len += sizeNext;
        }
        // Reset size of IFD if it has no entries and no or empty next IFD.
        if (compCount == 0 && sizeNext == 0) len = 0;
        return len;
    } // TiffDirectory::doSize

    uint32_t TiffEntryBase::doSize() const
    {
        return size_;
    } // TiffEntryBase::doSize

    uint32_t TiffImageEntry::doSize() const
    {
        return static_cast<uint32_t>(strips_.size()) * 4;
    } // TiffImageEntry::doSize

    uint32_t TiffSubIfd::doSize() const
    {
        return static_cast<uint32_t>(ifds_.size()) * 4;
    } // TiffSubIfd::doSize

    uint32_t TiffMnEntry::doSize() const
    {
        if (!mn_) {
            return TiffEntryBase::doSize();
        }
        return mn_->size();
    } // TiffMnEntry::doSize

    uint32_t TiffIfdMakernote::doSize() const
    {
        return sizeHeader() + ifd_.size();
    } // TiffIfdMakernote::doSize

    uint32_t TiffBinaryArray::doSize() const
    {
        if (cfg() == 0 || !decoded()) return TiffEntryBase::doSize();

        if (elements_.empty()) return 0;

        // Remaining assumptions:
        // - array elements don't "overlap"
        // - no duplicate tags in the array
        uint32_t idx = 0;
        uint32_t sz = cfg()->tagStep();
        for (Components::const_iterator i = elements_.begin(); i != elements_.end(); ++i) {
            if ((*i)->tag() > idx) {
                idx = (*i)->tag();
                sz = (*i)->size();
            }
        }
        idx = idx * cfg()->tagStep() + sz;

        if (cfg()->hasFillers_ && def()) {
            const ArrayDef* lastDef = def() + defSize() - 1;
            uint16_t lastTag = static_cast<uint16_t>(lastDef->idx_ / cfg()->tagStep());
            idx = EXV_MAX(idx, lastDef->idx_ + lastDef->size(lastTag, cfg()->group_));
        }
        return idx;

    } // TiffBinaryArray::doSize

    uint32_t TiffBinaryElement::doSize() const
    {
        if (!pValue()) return 0;
        return pValue()->size();
    } // TiffBinaryElement::doSize

    uint32_t TiffComponent::sizeData() const
    {
        return doSizeData();
    } // TiffComponent::sizeData

    uint32_t TiffDirectory::doSizeData() const
    {
        assert(false);
        return 0;
    } // TiffDirectory::doSizeData

    uint32_t TiffEntryBase::doSizeData() const
    {
        return 0;
    } // TiffEntryBase::doSizeData

    uint32_t TiffImageEntry::doSizeData() const
    {
        uint32_t len = 0;
        // For makernotes, TIFF image data is written to the data area
        if (group() > mnId) { // Todo: Fix this hack!!
            len = sizeImage();
        }
        return len;
    } // TiffImageEntry::doSizeData

    uint32_t TiffDataEntry::doSizeData() const
    {
        if (!pValue()) return 0;
        return pValue()->sizeDataArea();
    } // TiffDataEntry::doSizeData

    uint32_t TiffSubIfd::doSizeData() const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len += (*i)->size();
        }
        return len;
    } // TiffSubIfd::doSizeData

    uint32_t TiffIfdMakernote::doSizeData() const
    {
        assert(false);
        return 0;
    } // TiffIfdMakernote::doSizeData

    uint32_t TiffComponent::sizeImage() const
    {
        return doSizeImage();
    } // TiffComponent::sizeImage

    uint32_t TiffDirectory::doSizeImage() const
    {
        uint32_t len = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            len += (*i)->sizeImage();
        }
        if (pNext_) {
            len += pNext_->sizeImage();
        }
        return len;
    } // TiffDirectory::doSizeImage

    uint32_t TiffSubIfd::doSizeImage() const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len += (*i)->sizeImage();
        }
        return len;
    } // TiffSubIfd::doSizeImage

    uint32_t TiffIfdMakernote::doSizeImage() const
    {
        return ifd_.sizeImage();
    } // TiffIfdMakernote::doSizeImage

    uint32_t TiffEntryBase::doSizeImage() const
    {
        return 0;
    } // TiffEntryBase::doSizeImage

    uint32_t TiffImageEntry::doSizeImage() const
    {
        if (!pValue()) return 0;
        uint32_t len = pValue()->sizeDataArea();
        if (len == 0) {
            for (Strips::const_iterator i = strips_.begin(); i != strips_.end(); ++i) {
                len += i->second;
            }
        }
        return len;
    } // TiffImageEntry::doSizeImage

    // *************************************************************************
    // free functions
    TypeId toTypeId(TiffType tiffType, uint16_t tag, IfdId group)
    {
        TypeId ti = TypeId(tiffType);
        // On the fly type conversion for Exif.Photo.UserComment
        if (tag == 0x9286 && group == exifId && ti == undefined) {
            ti = comment;
        }
        return ti;
    }

    TiffType toTiffType(TypeId typeId)
    {
        if (static_cast<uint32_t>(typeId) > 0xffff) {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR << "'" << TypeInfo::typeName(typeId)
                      << "' is not a valid Exif (TIFF) type; using type '"
                      << TypeInfo::typeName(undefined) << "'.\n";
#endif
            return undefined;
        }
        return static_cast<uint16_t>(typeId);
    }

    bool cmpTagLt(TiffComponent const* lhs, TiffComponent const* rhs)
    {
        assert(lhs != 0);
        assert(rhs != 0);
        if (lhs->tag() != rhs->tag()) return lhs->tag() < rhs->tag();
        return lhs->idx() < rhs->idx();
    }

    bool cmpGroupLt(TiffComponent const* lhs, TiffComponent const* rhs)
    {
        assert(lhs != 0);
        assert(rhs != 0);
        return lhs->group() < rhs->group();
    }

    TiffComponent::AutoPtr newTiffEntry(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(new TiffEntry(tag, group));
    }

    TiffComponent::AutoPtr newTiffMnEntry(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(new TiffMnEntry(tag, group, mnId));
    }

    TiffComponent::AutoPtr newTiffBinaryElement(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(new TiffBinaryElement(tag, group));
    }

}}                                      // namespace Internal, Exiv2

// *****************************************************************************
// local definitions
namespace {
    uint32_t fillGap(Exiv2::Internal::IoWrapper& ioWrapper, uint32_t curr, uint32_t tobe)
    {
        if (curr < tobe) {
            Exiv2::DataBuf buf(tobe - curr);
            memset(buf.pData_, 0x0, buf.size_);
            ioWrapper.write(buf.pData_, buf.size_);
            return tobe - curr;
        }
        return 0;

    } // fillGap
}
