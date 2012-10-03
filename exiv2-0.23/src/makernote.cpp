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
  File:      makernote.cpp
  Version:   $Rev: 2698 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: makernote.cpp 2698 2012-04-11 16:02:44Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "makernote_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"

// + standard includes
#include <string>
#include <cstring>

// *****************************************************************************
namespace {
    // Todo: Can be generalized further - get any tag as a string/long/...
    //! Get the model name from tag Exif.Image.Model
    std::string getExifModel(Exiv2::Internal::TiffComponent* const pRoot);
    //! Nikon en/decryption function
    void ncrypt(Exiv2::byte* pData, uint32_t size, uint32_t count, uint32_t serial);
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    const TiffMnRegistry TiffMnCreator::registry_[] = {
        { "Canon",          canonId,     newIfdMn,       newIfdMn2       },
        { "FOVEON",         sigmaId,     newSigmaMn,     newSigmaMn2     },
        { "FUJI",           fujiId,      newFujiMn,      newFujiMn2      },
        { "KONICA MINOLTA", minoltaId,   newIfdMn,       newIfdMn2       },
        { "Minolta",        minoltaId,   newIfdMn,       newIfdMn2       },
        { "NIKON",          ifdIdNotSet, newNikonMn,     0               }, // mnGroup_ is not used
        { "OLYMPUS",        ifdIdNotSet, newOlympusMn,   0               }, // mnGroup_ is not used
        { "Panasonic",      panasonicId, newPanasonicMn, newPanasonicMn2 },
        { "PENTAX",         ifdIdNotSet, newPentaxMn,    0               }, // mnGroup_ is not used
        { "SAMSUNG",        samsung2Id,  newSamsungMn,   newSamsungMn2   },
        { "SIGMA",          sigmaId,     newSigmaMn,     newSigmaMn2     },
        { "SONY",           ifdIdNotSet, newSonyMn,      0               }, // mnGroup_ is not used
        // Entries below are only used for lookup by group
        { "-",              nikon1Id,    0,              newIfdMn2       },
        { "-",              nikon2Id,    0,              newNikon2Mn2    },
        { "-",              nikon3Id,    0,              newNikon3Mn2    },
        { "-",              sony1Id,     0,              newSony1Mn2     },
        { "-",              sony2Id,     0,              newSony2Mn2     },
        { "-",              olympusId,   0,              newOlympusMn2   },
        { "-",              olympus2Id,  0,              newOlympus2Mn2  },
        { "-",              pentaxId,    0,              newPentaxMn2    },
        { "-",              pentaxDngId, 0,              newPentaxDngMn2 }
    };

    bool TiffMnRegistry::operator==(const std::string& key) const
    {
        std::string make(make_);
        if (key.size() > 0 && key[0] == '-') return false;
        return make == key.substr(0, make.length());
    }

    bool TiffMnRegistry::operator==(IfdId key) const
    {
        return mnGroup_ == key;
    }

    TiffComponent* TiffMnCreator::create(uint16_t           tag,
                                         IfdId              group,
                                         const std::string& make,
                                         const byte*        pData,
                                         uint32_t           size,
                                         ByteOrder          byteOrder)
    {
        TiffComponent* tc = 0;
        const TiffMnRegistry* tmr = find(registry_, make);
        if (tmr) {
            assert(tmr->newMnFct_);
            tc = tmr->newMnFct_(tag,
                                group,
                                tmr->mnGroup_,
                                pData,
                                size,
                                byteOrder);
        }
        return tc;
    } // TiffMnCreator::create

    TiffComponent* TiffMnCreator::create(uint16_t           tag,
                                         IfdId              group,
                                         IfdId              mnGroup)
    {
        TiffComponent* tc = 0;
        const TiffMnRegistry* tmr = find(registry_, mnGroup);
        if (tmr) {

            if (tmr->newMnFct2_ == 0) {

                std::cout << "mnGroup = " << mnGroup << "\n";

            }

            assert(tmr->newMnFct2_);
            tc = tmr->newMnFct2_(tag, group, mnGroup);
        }
        return tc;
    } // TiffMnCreator::create

    MnHeader::~MnHeader()
    {
    }

    void MnHeader::setByteOrder(ByteOrder /*byteOrder*/)
    {
    }

    uint32_t MnHeader::ifdOffset() const
    {
        return 0;
    }

    ByteOrder MnHeader::byteOrder() const
    {
        return invalidByteOrder;
    }

    uint32_t MnHeader::baseOffset(uint32_t /*mnOffset*/) const
    {
        return 0;
    }

    const byte OlympusMnHeader::signature_[] = {
        'O', 'L', 'Y', 'M', 'P', 0x00, 0x01, 0x00
    };

    uint32_t OlympusMnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    OlympusMnHeader::OlympusMnHeader()
    {
        read(signature_, sizeOfSignature(), invalidByteOrder);
    }

    OlympusMnHeader::~OlympusMnHeader()
    {
    }

    uint32_t OlympusMnHeader::size() const
    {
        return header_.size_;
    }

    uint32_t OlympusMnHeader::ifdOffset() const
    {
        return sizeOfSignature();
    }

    bool OlympusMnHeader::read(const byte* pData,
                               uint32_t size,
                               ByteOrder /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        header_.alloc(sizeOfSignature());
        std::memcpy(header_.pData_, pData, header_.size_);
        if (   static_cast<uint32_t>(header_.size_) < sizeOfSignature()
            || 0 != memcmp(header_.pData_, signature_, 6)) {
            return false;
        }
        return true;
    } // OlympusMnHeader::read

    uint32_t OlympusMnHeader::write(IoWrapper& ioWrapper,
                                    ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // OlympusMnHeader::write

    const byte Olympus2MnHeader::signature_[] = {
        'O', 'L', 'Y', 'M', 'P', 'U', 'S', 0x00, 'I', 'I', 0x03, 0x00
    };

    uint32_t Olympus2MnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    Olympus2MnHeader::Olympus2MnHeader()
    {
        read(signature_, sizeOfSignature(), invalidByteOrder);
    }

    Olympus2MnHeader::~Olympus2MnHeader()
    {
    }

    uint32_t Olympus2MnHeader::size() const
    {
        return header_.size_;
    }

    uint32_t Olympus2MnHeader::ifdOffset() const
    {
        return sizeOfSignature();
    }

    uint32_t Olympus2MnHeader::baseOffset(uint32_t mnOffset) const
    {
        return mnOffset;
    }

    bool Olympus2MnHeader::read(const byte* pData,
                                uint32_t size,
                                ByteOrder /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        header_.alloc(sizeOfSignature());
        std::memcpy(header_.pData_, pData, header_.size_);
        if (   static_cast<uint32_t>(header_.size_) < sizeOfSignature()
            || 0 != memcmp(header_.pData_, signature_, 10)) {
            return false;
        }
        return true;
    } // Olympus2MnHeader::read

    uint32_t Olympus2MnHeader::write(IoWrapper& ioWrapper,
                                    ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // Olympus2MnHeader::write

    const byte FujiMnHeader::signature_[] = {
        'F', 'U', 'J', 'I', 'F', 'I', 'L', 'M', 0x0c, 0x00, 0x00, 0x00
    };
    const ByteOrder FujiMnHeader::byteOrder_ = littleEndian;

    uint32_t FujiMnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    FujiMnHeader::FujiMnHeader()
    {
        read(signature_, sizeOfSignature(), byteOrder_);
    }

    FujiMnHeader::~FujiMnHeader()
    {
    }

    uint32_t FujiMnHeader::size() const
    {
        return header_.size_;
    }

    uint32_t FujiMnHeader::ifdOffset() const
    {
        return start_;
    }

    ByteOrder FujiMnHeader::byteOrder() const
    {
        return byteOrder_;
    }

    uint32_t FujiMnHeader::baseOffset(uint32_t mnOffset) const
    {
        return mnOffset;
    }

    bool FujiMnHeader::read(const byte* pData,
                            uint32_t size,
                            ByteOrder /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        header_.alloc(sizeOfSignature());
        std::memcpy(header_.pData_, pData, header_.size_);
        // Read offset to the IFD relative to the start of the makernote
        // from the header. Note that we ignore the byteOrder argument
        start_ = getULong(header_.pData_ + 8, byteOrder_);
        if (   static_cast<uint32_t>(header_.size_) < sizeOfSignature()
            || 0 != memcmp(header_.pData_, signature_, 8)) {
            return false;
        }
        return true;
    } // FujiMnHeader::read

    uint32_t FujiMnHeader::write(IoWrapper& ioWrapper,
                                 ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // FujiMnHeader::write

    const byte Nikon2MnHeader::signature_[] = {
        'N', 'i', 'k', 'o', 'n', '\0', 0x01, 0x00
    };

    uint32_t Nikon2MnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    Nikon2MnHeader::Nikon2MnHeader()
    {
        read(signature_, sizeOfSignature(), invalidByteOrder);
    }

    Nikon2MnHeader::~Nikon2MnHeader()
    {
    }

    uint32_t Nikon2MnHeader::size() const
    {
        return sizeOfSignature();
    }

    uint32_t Nikon2MnHeader::ifdOffset() const
    {
        return start_;
    }

    bool Nikon2MnHeader::read(const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        if (0 != memcmp(pData, signature_, 6)) return false;
        buf_.alloc(sizeOfSignature());
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = sizeOfSignature();
        return true;
    } // Nikon2MnHeader::read

    uint32_t Nikon2MnHeader::write(IoWrapper& ioWrapper,
                                   ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // Nikon2MnHeader::write

    const byte Nikon3MnHeader::signature_[] = {
        'N', 'i', 'k', 'o', 'n', '\0', 0x02, 0x10, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    uint32_t Nikon3MnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    Nikon3MnHeader::Nikon3MnHeader()
    {
        buf_.alloc(sizeOfSignature());
        std::memcpy(buf_.pData_, signature_, buf_.size_);
        byteOrder_ = invalidByteOrder;
        start_ = sizeOfSignature();
    }

    Nikon3MnHeader::~Nikon3MnHeader()
    {
    }

    uint32_t Nikon3MnHeader::size() const
    {
        return sizeOfSignature();
    }

    uint32_t Nikon3MnHeader::ifdOffset() const
    {
        return start_;
    }

    ByteOrder Nikon3MnHeader::byteOrder() const
    {
        return byteOrder_;
    }

    uint32_t Nikon3MnHeader::baseOffset(uint32_t mnOffset) const
    {
        return mnOffset + 10;
    }

    bool Nikon3MnHeader::read(const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        if (0 != memcmp(pData, signature_, 6)) return false;
        buf_.alloc(sizeOfSignature());
        std::memcpy(buf_.pData_, pData, buf_.size_);
        TiffHeader th;
        if (!th.read(buf_.pData_ + 10, 8)) return false;
        byteOrder_ = th.byteOrder();
        start_ = 10 + th.offset();
        return true;
    } // Nikon3MnHeader::read

    uint32_t Nikon3MnHeader::write(IoWrapper& ioWrapper,
                                   ByteOrder byteOrder) const
    {
        assert(buf_.size_ >= 10);

        ioWrapper.write(buf_.pData_, 10);
        // Todo: This removes any gap between the header and
        // makernote IFD. The gap should be copied too.
        TiffHeader th(byteOrder);
        DataBuf buf = th.write();
        ioWrapper.write(buf.pData_, buf.size_);
        return 10 + buf.size_;
    } // Nikon3MnHeader::write

    void Nikon3MnHeader::setByteOrder(ByteOrder byteOrder)
    {
        byteOrder_ = byteOrder;
    }

    const byte PanasonicMnHeader::signature_[] = {
        'P', 'a', 'n', 'a', 's', 'o', 'n', 'i', 'c', 0x00, 0x00, 0x00
    };

    uint32_t PanasonicMnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    PanasonicMnHeader::PanasonicMnHeader()
    {
        read(signature_, sizeOfSignature(), invalidByteOrder);
    }

    PanasonicMnHeader::~PanasonicMnHeader()
    {
    }

    uint32_t PanasonicMnHeader::size() const
    {
        return sizeOfSignature();
    }

    uint32_t PanasonicMnHeader::ifdOffset() const
    {
        return start_;
    }

    bool PanasonicMnHeader::read(const byte* pData,
                                 uint32_t    size,
                                 ByteOrder   /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        if (0 != memcmp(pData, signature_, 9)) return false;
        buf_.alloc(sizeOfSignature());
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = sizeOfSignature();
        return true;
    } // PanasonicMnHeader::read

    uint32_t PanasonicMnHeader::write(IoWrapper& ioWrapper,
                                      ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // PanasonicMnHeader::write

    const byte PentaxDngMnHeader::signature_[] = {
        'P', 'E', 'N', 'T', 'A', 'X', ' ', 0x00, 'M', 'M'
    };

    uint32_t PentaxDngMnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    PentaxDngMnHeader::PentaxDngMnHeader()
    {
        read(signature_, sizeOfSignature(), invalidByteOrder);
    }

    PentaxDngMnHeader::~PentaxDngMnHeader()
    {
    }

    uint32_t PentaxDngMnHeader::size() const
    {
        return header_.size_;
    }

    uint32_t PentaxDngMnHeader::baseOffset(uint32_t mnOffset) const
    {
        return mnOffset;
    }

    uint32_t PentaxDngMnHeader::ifdOffset() const
    {
        return sizeOfSignature();
    }

    bool PentaxDngMnHeader::read(const byte* pData,
                              uint32_t size,
                              ByteOrder /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        header_.alloc(sizeOfSignature());
        std::memcpy(header_.pData_, pData, header_.size_);
        if (   static_cast<uint32_t>(header_.size_) < sizeOfSignature()
            || 0 != memcmp(header_.pData_, signature_, 7)) {
            return false;
        }
        return true;
    } // PentaxDngMnHeader::read

    uint32_t PentaxDngMnHeader::write(IoWrapper& ioWrapper,
                                   ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // PentaxDngMnHeader::write

    const byte PentaxMnHeader::signature_[] = {
        'A', 'O', 'C', 0x00, 'M', 'M'
    };

    uint32_t PentaxMnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    PentaxMnHeader::PentaxMnHeader()
    {
        read(signature_, sizeOfSignature(), invalidByteOrder);
    }

    PentaxMnHeader::~PentaxMnHeader()
    {
    }

    uint32_t PentaxMnHeader::size() const
    {
        return header_.size_;
    }

    uint32_t PentaxMnHeader::ifdOffset() const
    {
        return sizeOfSignature();
    }

    bool PentaxMnHeader::read(const byte* pData,
                              uint32_t size,
                              ByteOrder /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        header_.alloc(sizeOfSignature());
        std::memcpy(header_.pData_, pData, header_.size_);
        if (   static_cast<uint32_t>(header_.size_) < sizeOfSignature()
            || 0 != memcmp(header_.pData_, signature_, 3)) {
            return false;
        }
        return true;
    } // PentaxMnHeader::read

    uint32_t PentaxMnHeader::write(IoWrapper& ioWrapper,
                                   ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // PentaxMnHeader::write

    SamsungMnHeader::SamsungMnHeader()
    {
        read(0, 0, invalidByteOrder);
    }

    uint32_t SamsungMnHeader::size() const
    {
        return 0;
    }

    uint32_t SamsungMnHeader::baseOffset(uint32_t mnOffset) const
    {
        return mnOffset;
    }

    bool SamsungMnHeader::read(const byte* /*pData*/,
                               uint32_t    /*size*/,
                               ByteOrder   /*byteOrder*/)
    {
        return true;
    } // SamsungMnHeader::read

    uint32_t SamsungMnHeader::write(IoWrapper& /*ioWrapper*/,
                                    ByteOrder /*byteOrder*/) const
    {
        return 0;
    } // SamsungMnHeader::write

    const byte SigmaMnHeader::signature1_[] = {
        'S', 'I', 'G', 'M', 'A', '\0', '\0', '\0', 0x01, 0x00
    };
    const byte SigmaMnHeader::signature2_[] = {
        'F', 'O', 'V', 'E', 'O', 'N', '\0', '\0', 0x01, 0x00
    };

    uint32_t SigmaMnHeader::sizeOfSignature()
    {
        assert(sizeof(signature1_) == sizeof(signature2_));
        return sizeof(signature1_);
    }

    SigmaMnHeader::SigmaMnHeader()
    {
        read(signature1_, sizeOfSignature(), invalidByteOrder);
    }

    SigmaMnHeader::~SigmaMnHeader()
    {
    }

    uint32_t SigmaMnHeader::size() const
    {
        return sizeOfSignature();
    }

    uint32_t SigmaMnHeader::ifdOffset() const
    {
        return start_;
    }

    bool SigmaMnHeader::read(const byte* pData,
                             uint32_t    size,
                             ByteOrder   /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        if (   0 != memcmp(pData, signature1_, 8)
            && 0 != memcmp(pData, signature2_, 8)) return false;
        buf_.alloc(sizeOfSignature());
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = sizeOfSignature();
        return true;
    } // SigmaMnHeader::read

    uint32_t SigmaMnHeader::write(IoWrapper& ioWrapper,
                                  ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature1_, sizeOfSignature());
        return sizeOfSignature();
    } // SigmaMnHeader::write

    const byte SonyMnHeader::signature_[] = {
        'S', 'O', 'N', 'Y', ' ', 'D', 'S', 'C', ' ', '\0', '\0', '\0'
    };

    uint32_t SonyMnHeader::sizeOfSignature()
    {
        return sizeof(signature_);
    }

    SonyMnHeader::SonyMnHeader()
    {
        read(signature_, sizeOfSignature(), invalidByteOrder);
    }

    SonyMnHeader::~SonyMnHeader()
    {
    }

    uint32_t SonyMnHeader::size() const
    {
        return sizeOfSignature();
    }

    uint32_t SonyMnHeader::ifdOffset() const
    {
        return start_;
    }

    bool SonyMnHeader::read(const byte* pData,
                            uint32_t    size,
                            ByteOrder   /*byteOrder*/)
    {
        if (!pData || size < sizeOfSignature()) return false;
        if (0 != memcmp(pData, signature_, sizeOfSignature())) return false;
        buf_.alloc(sizeOfSignature());
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = sizeOfSignature();
        return true;
    } // SonyMnHeader::read

    uint32_t SonyMnHeader::write(IoWrapper& ioWrapper,
                                 ByteOrder /*byteOrder*/) const
    {
        ioWrapper.write(signature_, sizeOfSignature());
        return sizeOfSignature();
    } // SonyMnHeader::write

    // *************************************************************************
    // free functions

    TiffComponent* newIfdMn(uint16_t    tag,
                            IfdId       group,
                            IfdId       mnGroup,
                            const byte* /*pData*/,
                            uint32_t    size,
                            ByteOrder   /*byteOrder*/)
    {
        // Require at least an IFD with 1 entry, but not necessarily a next pointer
        if (size < 14) return 0;
        return newIfdMn2(tag, group, mnGroup);
    }

    TiffComponent* newIfdMn2(uint16_t tag,
                             IfdId    group,
                             IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, 0);
    }

    TiffComponent* newOlympusMn(uint16_t    tag,
                                IfdId       group,
                                IfdId       /*mnGroup*/,
                                const byte* pData,
                                uint32_t    size,
                                ByteOrder   /*byteOrder*/)
    {
        if (size < 10 ||   std::string(reinterpret_cast<const char*>(pData), 10)
                        != std::string("OLYMPUS\0II", 10)) {
            // Require at least the header and an IFD with 1 entry
            if (size < OlympusMnHeader::sizeOfSignature() + 18) return 0;
            return newOlympusMn2(tag, group, olympusId);
        }
        // Require at least the header and an IFD with 1 entry
        if (size < Olympus2MnHeader::sizeOfSignature() + 18) return 0;
        return newOlympus2Mn2(tag, group, olympus2Id);
    }

    TiffComponent* newOlympusMn2(uint16_t tag,
                                 IfdId    group,
                                 IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new OlympusMnHeader);
    }

    TiffComponent* newOlympus2Mn2(uint16_t tag,
                                  IfdId    group,
                                  IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new Olympus2MnHeader);
    }

    TiffComponent* newFujiMn(uint16_t    tag,
                             IfdId       group,
                             IfdId       mnGroup,
                             const byte* /*pData*/,
                             uint32_t    size,
                             ByteOrder   /*byteOrder*/)
    {
        // Require at least the header and an IFD with 1 entry
        if (size < FujiMnHeader::sizeOfSignature() + 18) return 0;
        return newFujiMn2(tag, group, mnGroup);
    }

    TiffComponent* newFujiMn2(uint16_t tag,
                              IfdId    group,
                              IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new FujiMnHeader);
    }

    TiffComponent* newNikonMn(uint16_t    tag,
                              IfdId       group,
                              IfdId       /*mnGroup*/,
                              const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        // If there is no "Nikon" string it must be Nikon1 format
        if (size < 6 ||    std::string(reinterpret_cast<const char*>(pData), 6)
                        != std::string("Nikon\0", 6)) {
            // Require at least an IFD with 1 entry
            if (size < 18) return 0;
            return newIfdMn2(tag, group, nikon1Id);
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeader tiffHeader;
        if (   size < 18
            || !tiffHeader.read(pData + 10, size - 10)
            || tiffHeader.tag() != 0x002a) {
            // Require at least the header and an IFD with 1 entry
            if (size < Nikon2MnHeader::sizeOfSignature() + 18) return 0;
            return newNikon2Mn2(tag, group, nikon2Id);
        }
        // Else we have a Nikon3 makernote
        // Require at least the header and an IFD with 1 entry
        if (size < Nikon3MnHeader::sizeOfSignature() + 18) return 0;
        return newNikon3Mn2(tag, group, nikon3Id);
    }

    TiffComponent* newNikon2Mn2(uint16_t tag,
                                IfdId    group,
                                IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new Nikon2MnHeader);
    }

    TiffComponent* newNikon3Mn2(uint16_t tag,
                                IfdId    group,
                                IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new Nikon3MnHeader);
    }

    TiffComponent* newPanasonicMn(uint16_t    tag,
                                  IfdId       group,
                                  IfdId       mnGroup,
                                  const byte* /*pData*/,
                                  uint32_t    size,
                                  ByteOrder   /*byteOrder*/)
    {
        // Require at least the header and an IFD with 1 entry, but without a next pointer
        if (size < PanasonicMnHeader::sizeOfSignature() + 14) return 0;
        return newPanasonicMn2(tag, group, mnGroup);
    }

    TiffComponent* newPanasonicMn2(uint16_t tag,
                                   IfdId    group,
                                   IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new PanasonicMnHeader, false);
    }

    TiffComponent* newPentaxMn(uint16_t    tag,
                               IfdId       group,
                               IfdId       /*mnGroup*/,
                               const byte* pData,
                               uint32_t    size,
                               ByteOrder   /*byteOrder*/)
    {
        if (   size > 8
            && std::string(reinterpret_cast<const char*>(pData), 8) == std::string("PENTAX \0", 8)) {
            // Require at least the header and an IFD with 1 entry
            if (size < PentaxDngMnHeader::sizeOfSignature() + 18) return 0;
            return newPentaxDngMn2(tag, group, pentaxDngId);
        }
        else if (   size > 4
                 && std::string(reinterpret_cast<const char*>(pData), 4) == std::string("AOC\0", 4)) {
            // Require at least the header and an IFD with 1 entry
            if (size < PentaxMnHeader::sizeOfSignature() + 18) return 0;
            return newPentaxMn2(tag, group, pentaxId);
        } else
            return 0;
    }

    TiffComponent* newPentaxMn2(uint16_t tag,
                                IfdId    group,
                                IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new PentaxMnHeader);
    }

    TiffComponent* newPentaxDngMn2(uint16_t tag,
                                IfdId    group,
                                IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new PentaxDngMnHeader);
    }

    TiffComponent* newSamsungMn(uint16_t    tag,
                                IfdId       group,
                                IfdId       mnGroup,
                                const byte* /*pData*/,
                                uint32_t    size,
                                ByteOrder   /*byteOrder*/)
    {
        // Require at least an IFD with 1 entry
        if (size < 18) return 0;
        return newSamsungMn2(tag, group, mnGroup);
    }

    TiffComponent* newSamsungMn2(uint16_t tag,
                                 IfdId    group,
                                 IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new SamsungMnHeader);
    }

    TiffComponent* newSigmaMn(uint16_t    tag,
                              IfdId       group,
                              IfdId       mnGroup,
                              const byte* /*pData*/,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        // Require at least the header and an IFD with 1 entry
        if (size < SigmaMnHeader::sizeOfSignature() + 18) return 0;
        return newSigmaMn2(tag, group, mnGroup);
    }

    TiffComponent* newSigmaMn2(uint16_t tag,
                               IfdId    group,
                               IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new SigmaMnHeader);
    }

    TiffComponent* newSonyMn(uint16_t    tag,
                             IfdId       group,
                             IfdId       /*mnGroup*/,
                             const byte* pData,
                             uint32_t    size,
                             ByteOrder   /*byteOrder*/)
    {
        // If there is no "SONY DSC " string we assume it's a simple IFD Makernote
        if (size < 12 ||   std::string(reinterpret_cast<const char*>(pData), 12)
                        != std::string("SONY DSC \0\0\0", 12)) {
            // Require at least an IFD with 1 entry
            if (size < 18) return 0;
            return newSony2Mn2(tag, group, sony2Id);
        }
        // Require at least the header and an IFD with 1 entry, but without a next pointer
        if (size < SonyMnHeader::sizeOfSignature() + 14) return 0;
        return newSony1Mn2(tag, group, sony1Id);
    }

    TiffComponent* newSony1Mn2(uint16_t tag,
                               IfdId    group,
                               IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new SonyMnHeader, false);
    }

    TiffComponent* newSony2Mn2(uint16_t tag,
                               IfdId    group,
                               IfdId    mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, 0, true);
    }

    //! Structure for an index into the array set of complex binary arrays.
    struct NikonArrayIdx {
        //! Key for comparisons
        struct Key {
            //! Constructor
            Key(uint16_t tag, const char* ver, uint32_t size) : tag_(tag), ver_(ver), size_(size) {}
            uint16_t    tag_;  //!< Tag number
            const char* ver_;  //!< Version string
            uint32_t    size_; //!< Size of the data (not the version string)
        };
        //! Comparison operator for a key
        bool operator==(const Key& key) const
        {
            return    key.tag_ == tag_
                   && 0 == strncmp(key.ver_, ver_, strlen(ver_))
                   && (size_ == 0 || key.size_ == size_);
        }

        uint16_t    tag_;   //!< Tag number of the binary array
        const char* ver_;   //!< Version string
        uint32_t    size_;  //!< Size of the data
        int         idx_;   //!< Index into the array set
        uint32_t    start_; //!< Start of the encrypted data
    };

#define NA ((uint32_t)-1)

    //! Nikon binary array version lookup table
    extern const NikonArrayIdx nikonArrayIdx[] = {
        // NikonSi
        { 0x0091, "0208",    0, 0,   4 }, // D80
        { 0x0091, "0209",    0, 1,   4 }, // D40
        { 0x0091, "0210", 5291, 2,   4 }, // D300
        { 0x0091, "0210", 5303, 3,   4 }, // D300, firmware version 1.10
        { 0x0091, "02",      0, 4,   4 }, // Other v2.* (encrypted)
        { 0x0091, "01",      0, 5,  NA }, // Other v1.* (not encrypted)
        // NikonCb
        { 0x0097, "0100",    0, 0,  NA },
        { 0x0097, "0102",    0, 1,  NA },
        { 0x0097, "0103",    0, 4,  NA },
        { 0x0097, "0204",    0, 3, 284 },
        { 0x0097, "0205",    0, 2,   4 },
        { 0x0097, "0206",    0, 3, 284 },
        { 0x0097, "0207",    0, 3, 284 },
        { 0x0097, "0208",    0, 3, 284 },
        { 0x0097, "0209",    0, 5, 284 },
        { 0x0097, "02",      0, 3, 284 },
        // NikonLd
        { 0x0098, "0100",    0, 0,  NA },
        { 0x0098, "0101",    0, 1,  NA },
        { 0x0098, "0201",    0, 1,   4 },
        { 0x0098, "0202",    0, 1,   4 },
        { 0x0098, "0203",    0, 1,   4 },
        { 0x0098, "0204",    0, 2,   4 },
        // NikonFl
        { 0x00a8, "0100",    0, 0,  NA },
        { 0x00a8, "0101",    0, 0,  NA },
        { 0x00a8, "0102",    0, 1,  NA },
        { 0x00a8, "0103",    0, 2,  NA },
    };

    int nikonSelector(uint16_t tag, const byte* pData, uint32_t size, TiffComponent* const /*pRoot*/)
    {
        if (size < 4) return -1;
        const NikonArrayIdx* aix = find(nikonArrayIdx, NikonArrayIdx::Key(tag, reinterpret_cast<const char*>(pData), size));
        return aix == 0 ? -1 : aix->idx_;
    }

    DataBuf nikonCrypt(uint16_t tag, const byte* pData, uint32_t size, TiffComponent* const pRoot)
    {
        DataBuf buf;

        if (size < 4) return buf;
        const NikonArrayIdx* nci = find(nikonArrayIdx, NikonArrayIdx::Key(tag, reinterpret_cast<const char*>(pData), size));
        if (nci == 0 || nci->start_ == NA || size <= nci->start_) return buf;

        // Find Exif.Nikon3.ShutterCount
        TiffFinder finder(0x00a7, nikon3Id);
        pRoot->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (!te || !te->pValue() || te->pValue()->count() == 0) return buf;
        uint32_t count = static_cast<uint32_t>(te->pValue()->toLong());

        // Find Exif.Nikon3.SerialNumber
        finder.init(0x001d, nikon3Id);
        pRoot->accept(finder);
        te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (!te || !te->pValue() || te->pValue()->count() == 0) return buf;
        bool ok(false);
        uint32_t serial = stringTo<uint32_t>(te->pValue()->toString(), ok);
        if (!ok) {
            std::string model = getExifModel(pRoot);
            if (model.empty()) return buf;
            if (model.find("D50") != std::string::npos) {
                serial = 0x22;
            }
            else {
                serial = 0x60;
            }
        }
        buf.alloc(size);
        memcpy(buf.pData_, pData, buf.size_);
        ncrypt(buf.pData_ + nci->start_, buf.size_ - nci->start_, count, serial);
        return buf;
    }

    int sonyCsSelector(uint16_t /*tag*/, const byte* /*pData*/, uint32_t /*size*/, TiffComponent* const pRoot)
    {
        std::string model = getExifModel(pRoot);
        if (model.empty()) return -1;
        int idx = 0;
        if (   model.find("DSLR-A330") != std::string::npos
            || model.find("DSLR-A380") != std::string::npos) {
            idx = 1;
        }
        return idx;
    }
}}                                      // namespace Internal, Exiv2

// *****************************************************************************
// local definitions
namespace {
    std::string getExifModel(Exiv2::Internal::TiffComponent* const pRoot)
    {
        Exiv2::Internal::TiffFinder finder(0x0110, Exiv2::Internal::ifd0Id); // Exif.Image.Model
        pRoot->accept(finder);
        Exiv2::Internal::TiffEntryBase* te = dynamic_cast<Exiv2::Internal::TiffEntryBase*>(finder.result());
        if (!te || !te->pValue() || te->pValue()->count() == 0) return std::string();
        return te->pValue()->toString();
    }

    void ncrypt(Exiv2::byte* pData, uint32_t size, uint32_t count, uint32_t serial)
    {
        static const Exiv2::byte xlat[2][256] = {
            { 0xc1,0xbf,0x6d,0x0d,0x59,0xc5,0x13,0x9d,0x83,0x61,0x6b,0x4f,0xc7,0x7f,0x3d,0x3d,
              0x53,0x59,0xe3,0xc7,0xe9,0x2f,0x95,0xa7,0x95,0x1f,0xdf,0x7f,0x2b,0x29,0xc7,0x0d,
              0xdf,0x07,0xef,0x71,0x89,0x3d,0x13,0x3d,0x3b,0x13,0xfb,0x0d,0x89,0xc1,0x65,0x1f,
              0xb3,0x0d,0x6b,0x29,0xe3,0xfb,0xef,0xa3,0x6b,0x47,0x7f,0x95,0x35,0xa7,0x47,0x4f,
              0xc7,0xf1,0x59,0x95,0x35,0x11,0x29,0x61,0xf1,0x3d,0xb3,0x2b,0x0d,0x43,0x89,0xc1,
              0x9d,0x9d,0x89,0x65,0xf1,0xe9,0xdf,0xbf,0x3d,0x7f,0x53,0x97,0xe5,0xe9,0x95,0x17,
              0x1d,0x3d,0x8b,0xfb,0xc7,0xe3,0x67,0xa7,0x07,0xf1,0x71,0xa7,0x53,0xb5,0x29,0x89,
              0xe5,0x2b,0xa7,0x17,0x29,0xe9,0x4f,0xc5,0x65,0x6d,0x6b,0xef,0x0d,0x89,0x49,0x2f,
              0xb3,0x43,0x53,0x65,0x1d,0x49,0xa3,0x13,0x89,0x59,0xef,0x6b,0xef,0x65,0x1d,0x0b,
              0x59,0x13,0xe3,0x4f,0x9d,0xb3,0x29,0x43,0x2b,0x07,0x1d,0x95,0x59,0x59,0x47,0xfb,
              0xe5,0xe9,0x61,0x47,0x2f,0x35,0x7f,0x17,0x7f,0xef,0x7f,0x95,0x95,0x71,0xd3,0xa3,
              0x0b,0x71,0xa3,0xad,0x0b,0x3b,0xb5,0xfb,0xa3,0xbf,0x4f,0x83,0x1d,0xad,0xe9,0x2f,
              0x71,0x65,0xa3,0xe5,0x07,0x35,0x3d,0x0d,0xb5,0xe9,0xe5,0x47,0x3b,0x9d,0xef,0x35,
              0xa3,0xbf,0xb3,0xdf,0x53,0xd3,0x97,0x53,0x49,0x71,0x07,0x35,0x61,0x71,0x2f,0x43,
              0x2f,0x11,0xdf,0x17,0x97,0xfb,0x95,0x3b,0x7f,0x6b,0xd3,0x25,0xbf,0xad,0xc7,0xc5,
              0xc5,0xb5,0x8b,0xef,0x2f,0xd3,0x07,0x6b,0x25,0x49,0x95,0x25,0x49,0x6d,0x71,0xc7 },
            { 0xa7,0xbc,0xc9,0xad,0x91,0xdf,0x85,0xe5,0xd4,0x78,0xd5,0x17,0x46,0x7c,0x29,0x4c,
              0x4d,0x03,0xe9,0x25,0x68,0x11,0x86,0xb3,0xbd,0xf7,0x6f,0x61,0x22,0xa2,0x26,0x34,
              0x2a,0xbe,0x1e,0x46,0x14,0x68,0x9d,0x44,0x18,0xc2,0x40,0xf4,0x7e,0x5f,0x1b,0xad,
              0x0b,0x94,0xb6,0x67,0xb4,0x0b,0xe1,0xea,0x95,0x9c,0x66,0xdc,0xe7,0x5d,0x6c,0x05,
              0xda,0xd5,0xdf,0x7a,0xef,0xf6,0xdb,0x1f,0x82,0x4c,0xc0,0x68,0x47,0xa1,0xbd,0xee,
              0x39,0x50,0x56,0x4a,0xdd,0xdf,0xa5,0xf8,0xc6,0xda,0xca,0x90,0xca,0x01,0x42,0x9d,
              0x8b,0x0c,0x73,0x43,0x75,0x05,0x94,0xde,0x24,0xb3,0x80,0x34,0xe5,0x2c,0xdc,0x9b,
              0x3f,0xca,0x33,0x45,0xd0,0xdb,0x5f,0xf5,0x52,0xc3,0x21,0xda,0xe2,0x22,0x72,0x6b,
              0x3e,0xd0,0x5b,0xa8,0x87,0x8c,0x06,0x5d,0x0f,0xdd,0x09,0x19,0x93,0xd0,0xb9,0xfc,
              0x8b,0x0f,0x84,0x60,0x33,0x1c,0x9b,0x45,0xf1,0xf0,0xa3,0x94,0x3a,0x12,0x77,0x33,
              0x4d,0x44,0x78,0x28,0x3c,0x9e,0xfd,0x65,0x57,0x16,0x94,0x6b,0xfb,0x59,0xd0,0xc8,
              0x22,0x36,0xdb,0xd2,0x63,0x98,0x43,0xa1,0x04,0x87,0x86,0xf7,0xa6,0x26,0xbb,0xd6,
              0x59,0x4d,0xbf,0x6a,0x2e,0xaa,0x2b,0xef,0xe6,0x78,0xb6,0x4e,0xe0,0x2f,0xdc,0x7c,
              0xbe,0x57,0x19,0x32,0x7e,0x2a,0xd0,0xb8,0xba,0x29,0x00,0x3c,0x52,0x7d,0xa8,0x49,
              0x3b,0x2d,0xeb,0x25,0x49,0xfa,0xa3,0xaa,0x39,0xa7,0xc5,0xa7,0x50,0x11,0x36,0xfb,
              0xc6,0x67,0x4a,0xf5,0xa5,0x12,0x65,0x7e,0xb0,0xdf,0xaf,0x4e,0xb3,0x61,0x7f,0x2f }
        };
        Exiv2::byte key = 0;
        for (int i = 0; i < 4; ++i) {
            key ^= (count >> (i*8)) & 0xff;
        }
        Exiv2::byte ci = xlat[0][serial & 0xff];
        Exiv2::byte cj = xlat[1][key];
        Exiv2::byte ck = 0x60;
        for (uint32_t i = 0; i < size; ++i) {
            cj += ci * ck++;
            pData[i] ^= cj;
        }
    }
}
