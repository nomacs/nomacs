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
/*!
  @file    tiffcomposite_int.hpp
  @brief   Internal classes used in a TIFF composite structure
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef TIFFCOMPOSITE_INT_HPP_
#define TIFFCOMPOSITE_INT_HPP_

// *****************************************************************************
// included header files
#include "value.hpp"
#include "tifffwd_int.hpp"
#include "types.hpp"

// + standard includes
#include <iosfwd>
#include <vector>
#include <string>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

    class BasicIo;

    namespace Internal {

// *****************************************************************************
// class definitions

    //! TIFF value type.
    typedef uint16_t TiffType;

    const TiffType ttUnsignedByte     = 1; //!< Exif BYTE type
    const TiffType ttAsciiString      = 2; //!< Exif ASCII type
    const TiffType ttUnsignedShort    = 3; //!< Exif SHORT type
    const TiffType ttUnsignedLong     = 4; //!< Exif LONG type
    const TiffType ttUnsignedRational = 5; //!< Exif RATIONAL type
    const TiffType ttSignedByte       = 6; //!< Exif SBYTE type
    const TiffType ttUndefined        = 7; //!< Exif UNDEFINED type
    const TiffType ttSignedShort      = 8; //!< Exif SSHORT type
    const TiffType ttSignedLong       = 9; //!< Exif SLONG type
    const TiffType ttSignedRational   =10; //!< Exif SRATIONAL type
    const TiffType ttTiffFloat        =11; //!< TIFF FLOAT type
    const TiffType ttTiffDouble       =12; //!< TIFF DOUBLE type
    const TiffType ttTiffIfd          =13; //!< TIFF IFD type

    //! Convert the \em tiffType of a \em tag and \em group to an Exiv2 \em typeId.
    TypeId toTypeId(TiffType tiffType, uint16_t tag, IfdId group);
    //! Convert the %Exiv2 \em typeId to a TIFF value type.
    TiffType toTiffType(TypeId typeId);

    /*!
      Special TIFF tags for the use in TIFF structures only
    */
    namespace Tag {
        const uint32_t none = 0x10000; //!< Dummy tag
        const uint32_t root = 0x20000; //!< Special tag: root IFD
        const uint32_t next = 0x30000; //!< Special tag: next IFD
        const uint32_t all  = 0x40000; //!< Special tag: all tags in a group
        const uint32_t pana = 0x80000; //!< Special tag: root IFD of Panasonic RAW images
    }

    /*!
      @brief A tupel consisting of extended Tag and group used as an item in
             TIFF paths.
    */
    class TiffPathItem {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffPathItem(uint32_t extendedTag, IfdId group)
            : extendedTag_(extendedTag), group_(group) {}
        //@}

        //! @name Accessors
        //@{
        //! Return the tag corresponding to the extended tag
        uint16_t tag()         const { return static_cast<uint16_t>(extendedTag_ & 0xffff); }
        //! Return the extended tag (32 bit so that it can contain special tags)
        uint32_t extendedTag() const { return extendedTag_; }
        //! Return the group
        IfdId    group()       const { return group_; }
        //@}

    private:
        // DATA
        uint32_t extendedTag_;
        IfdId    group_;
    }; // class TiffPathItem

    /*!
      @brief Simple IO wrapper to ensure that the header is only written if there is
             any other data at all.

      The wrapper is initialized with an IO reference and a pointer to a TIFF header.
      Subsequently the wrapper is used by all TIFF write methods. It takes care that
      the TIFF header is written to the IO first before any other output and only if
      there is any other data.
     */
    class IoWrapper {
    public:
        //! @name Creators
        //@{
        /*!
          brief Constructor.

          The IO wrapper owns none of the objects passed in so the caller is
          responsible to keep them alive.
         */
        IoWrapper(BasicIo& io, const byte* pHeader, long size, OffsetWriter* pow);
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Wraps the corresponding BasicIo::write() method.

          Writes the TIFF header to the IO, if it hasn't been written yet, followed
          by the data passed in the arguments.
         */
        long write(const byte* pData, long wcount);
        /*!
          @brief Wraps the corresponding BasicIo::putb() method.

          Writes the TIFF header to the IO, if it hasn't been written yet, followed
          by the data passed in the argument.
         */
        int putb(byte data);
        //! Wrapper for OffsetWriter::setTarget(), using an int instead of the enum to reduce include deps
        void setTarget(int id, uint32_t target);
        //@}

    private:
        // DATA
        BasicIo& io_;              //! Reference for the IO instance.
        const byte* pHeader_;      //! Pointer to the header data.
        long size_;                //! Size of the header data.
        bool wroteHeader_;         //! Indicates if the header has been written.
        OffsetWriter* pow_;        //! Pointer to an offset-writer, if any, or 0
    }; // class IoWrapper

    /*!
      @brief Interface class for components of a TIFF directory hierarchy
             (Composite pattern).  Both TIFF directories as well as entries
             implement this interface.  A component can be uniquely identified
             by a tag, group tupel.  This class is implemented as a NVI
             (Non-Virtual Interface) and it has an interface for visitors
             (Visitor pattern) to perform operations on all components.
     */
    class TiffComponent {
    public:
        //! TiffComponent auto_ptr type
        typedef std::auto_ptr<TiffComponent> AutoPtr;
        //! Container type to hold all metadata
        typedef std::vector<TiffComponent*> Components;

        //! @name Creators
        //@{
        //! Constructor
        TiffComponent(uint16_t tag, IfdId group);
        //! Virtual destructor.
        virtual ~TiffComponent();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Add a TIFF entry \em tag to the component. Components on
                 the path to the entry are added if they don't exist yet.

          @param tag      The tag of the new entry
          @param tiffPath A path from the TIFF root element to a TIFF entry.
          @param pRoot    Pointer to the root component of the TIFF composite.
          @param object   TIFF component to add. If 0, the correct entry will be
                          created.

          @return A pointer to the newly added TIFF entry.
         */
        TiffComponent* addPath(uint16_t tag,
                               TiffPath& tiffPath,
                               TiffComponent* const pRoot,
                               AutoPtr object =AutoPtr(0));
        /*!
          @brief Add a child to the component. Default is to do nothing.
          @param tiffComponent Auto pointer to the component to add.
          @return Return a pointer to the newly added child element or 0.
         */
        TiffComponent* addChild(AutoPtr tiffComponent);
        /*!
            @brief Add a "next" component to the component. Default is to do
                   nothing.
            @param tiffComponent Auto pointer to the component to add.
            @return Return a pointer to the newly added "next" element or 0.
         */
        TiffComponent* addNext(AutoPtr tiffComponent);
        /*!
          @brief Interface to accept visitors (Visitor pattern). Visitors
                 can perform operations on all components of the composite.

          @param visitor The visitor.
         */
        void accept(TiffVisitor& visitor);
        /*!
          @brief Set a pointer to the start of the binary representation of the
                 component in a memory buffer. The buffer must be allocated and
                 freed outside of this class.
         */
        void setStart(const byte* pStart) { pStart_ = const_cast<byte*>(pStart); }
        /*!
          @brief Write a TiffComponent to a binary image.

          @param ioWrapper  IO wrapper to which the TiffComponent is written.
          @param byteOrder  Applicable byte order (little or big endian).
          @param offset     Offset from the start of the image (TIFF header) to
                            the component.
          @param valueIdx   Index of the component to be written relative to offset.
          @param dataIdx    Index of the data area of the component relative to offset.
          @param imageIdx   Index of the image data area relative to offset.
          @return           Number of bytes written to the IO wrapper including all
                            nested components.
          @throw            Error If the component cannot be written.
         */
        uint32_t write(IoWrapper& ioWrapper,
                       ByteOrder byteOrder,
                       int32_t   offset,
                       uint32_t  valueIdx,
                       uint32_t  dataIdx,
                       uint32_t& imageIdx);
        //@}

        //! @name Accessors
        //@{
        //! Return the tag of this entry.
        uint16_t tag()                        const { return tag_; }
        //! Return the group id of this component
        IfdId    group()                      const { return group_; }
        //! Return a pointer to the start of the binary representation of the component
        byte*    start()                      const { return pStart_; }
        /*!
          @brief Return an auto-pointer to a copy of itself (deep copy, but
                 without any children). The caller owns this copy and the
                 auto-pointer ensures that it will be deleted.
         */
        AutoPtr clone() const;
        /*!
          @brief Write the IFD data of this component to a binary image.
                 Return the number of bytes written. Components derived from
                 TiffEntryBase implement this method if needed.
         */
        uint32_t writeData(IoWrapper& ioWrapper,
                           ByteOrder byteOrder,
                           int32_t   offset,
                           uint32_t  dataIdx,
                           uint32_t& imageIdx) const;
        /*!
          @brief Write the image data of this component to a binary image.
                 Return the number of bytes written. TIFF components implement
                 this method if needed.
         */
        uint32_t writeImage(IoWrapper& ioWrapper,
                            ByteOrder byteOrder) const;
        /*!
          @brief Return the size in bytes of the IFD value of this component
                 when written to a binary image.
         */
        uint32_t size() const;
        /*!
          @brief Return the number of components in this component.
         */
        uint32_t count() const;
        /*!
          @brief Return the size in bytes of the IFD data of this component when
                 written to a binary image.  This is a support function for
                 write(). Components derived from TiffEntryBase implement this
                 method corresponding to their implementation of writeData().
         */
        uint32_t sizeData() const;
        /*!
          @brief Return the size in bytes of the image data of this component
                 when written to a binary image.  This is a support function for
                 write(). TIFF components implement this method corresponding to
                 their implementation of writeImage().
         */
        uint32_t sizeImage() const;
        /*!
          @brief Return the unique id of the entry in the image.
         */
        // Todo: This is only implemented in TiffEntryBase. It is needed here so that
        //       we can sort components by tag and idx. Something is not quite right.
        virtual int idx() const;
        //@}

    protected:
        //! @name Protected Manipulators
        //@{
        //! Implements addPath(). The default implementation does nothing.
        virtual TiffComponent* doAddPath(uint16_t  tag,
                                         TiffPath& tiffPath,
                                         TiffComponent* const pRoot,
                                         TiffComponent::AutoPtr object);
        //! Implements addChild(). The default implementation does nothing.
        virtual TiffComponent* doAddChild(AutoPtr tiffComponent);
        //! Implements addNext(). The default implementation does nothing.
        virtual TiffComponent* doAddNext(AutoPtr tiffComponent);
        //! Implements accept().
        virtual void doAccept(TiffVisitor& visitor) =0;
        //! Implements write().
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx) =0;
        //@}

        //! @name Protected Accessors
        //@{
        //! Internal virtual copy constructor, implements clone().
        virtual TiffComponent* doClone() const =0;
        //! Implements writeData().
        virtual uint32_t doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const =0;
        //! Implements writeImage().
        virtual uint32_t doWriteImage(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder) const =0;
        //! Implements size().
        virtual uint32_t doSize() const =0;
        //! Implements count().
        virtual uint32_t doCount() const =0;
        //! Implements sizeData().
        virtual uint32_t doSizeData() const =0;
        //! Implements sizeImage().
        virtual uint32_t doSizeImage() const =0;
        //@}

    private:
        // DATA
        uint16_t tag_;      //!< Tag that identifies the component
        IfdId    group_;    //!< Group id for this component
        /*!
          Pointer to the start of the binary representation of the component in
          a memory buffer. The buffer is allocated and freed outside of this class.
         */
        byte*    pStart_;

    }; // class TiffComponent

    //! TIFF mapping table for functions to decode special cases
    struct TiffMappingInfo {
        struct Key;
        /*!
          @brief Compare a TiffMappingInfo with a TiffMappingInfo::Key.
                 The two are equal if TiffMappingInfo::make_ equals a substring
                 of the key of the same size. E.g., mapping info = "OLYMPUS",
                 key = "OLYMPUS OPTICAL CO.,LTD" (found in the image) match,
                 the extendedTag is Tag::all or equal to the extended tag of the
                 key, and the group is equal to that of the key.
         */
        bool operator==(const Key& key) const;
        //! Return the tag corresponding to the extended tag
        uint16_t tag() const { return static_cast<uint16_t>(extendedTag_ & 0xffff); }

        // DATA
        const char* make_;        //!< Camera make for which these mapping functions apply
        uint32_t    extendedTag_; //!< Tag (32 bit so that it can contain special tags)
        IfdId       group_;       //!< Group that contains the tag
        DecoderFct  decoderFct_;  //!< Decoder function for matching tags
        EncoderFct  encoderFct_;  //!< Encoder function for matching tags

    }; // struct TiffMappingInfo

    //! Search key for TIFF mapping structures.
    struct TiffMappingInfo::Key {
        //! Constructor
        Key(const std::string& m, uint32_t e, IfdId g) : m_(m), e_(e), g_(g) {}
        std::string m_;                    //!< Camera make
        uint32_t    e_;                    //!< Extended tag
        IfdId       g_;                    //!< %Group
    };

    /*!
      @brief This abstract base class provides the common functionality of an
             IFD directory entry and defines an extended interface for derived
             concrete entries, which allows access to the attributes of the
             entry.
     */
    class TiffEntryBase : public TiffComponent {
        friend class TiffReader;
        friend class TiffEncoder;
        friend int selectNikonLd(TiffBinaryArray* const, TiffComponent* const);
    public:
        //! @name Creators
        //@{
        //! Default constructor.
        TiffEntryBase(uint16_t tag, IfdId group, TiffType tiffType =ttUndefined);
        //! Virtual destructor.
        virtual ~TiffEntryBase();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Encode a TIFF component from the metadatum provided and
                 information from the \em encoder as needed.

          Implemented as double-dispatch calls back to one of the specific
          encoding functions at the \em encoder.
         */
        void encode(TiffEncoder& encoder, const Exifdatum* datum);
        //! Set the offset
        void setOffset(int32_t offset) { offset_ = offset; }
        //! Set pointer and size of the entry's data (not taking ownership of the data).
        void setData(byte* pData, int32_t size);
        //! Set the entry's data buffer, taking ownership of the data buffer passed in.
        void setData(DataBuf buf);
         /*!
          @brief Update the value. Takes ownership of the pointer passed in.

          Update binary value data and call setValue().
        */
        void updateValue(Value::AutoPtr value, ByteOrder byteOrder);
        /*!
          @brief Set tag value. Takes ownership of the pointer passed in.

          Update type, count and the pointer to the value.
        */
        void setValue(Value::AutoPtr value);
        //@}

        //! @name Accessors
        //@{
        //! Return the TIFF type
        TiffType tiffType()      const { return tiffType_; }
        /*!
          @brief Return the offset to the data area relative to the base
                 for the component (usually the start of the TIFF header)
         */
        int32_t offset()         const { return offset_; }
        /*!
          @brief Return the unique id of the entry in the image
         */
        virtual int idx()        const;
        /*!
          @brief Return a pointer to the binary representation of the
                 value of this component.
         */
        const byte* pData()      const { return pData_; }
        //! Return a const pointer to the converted value of this component
        const Value* pValue()    const { return pValue_; }
        //@}

    protected:
        //! @name Protected Creators
        //@{
        //! Copy constructor (used to implement clone()).
        TiffEntryBase(const TiffEntryBase& rhs);
        //@}

        //! @name Protected Manipulators
        //@{
        //! Implements encode().
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum) =0;
        //! Set the number of components in this entry
        void setCount(uint32_t count) { count_ = count; }
        //! Set the unique id of the entry in the image
        void setIdx(int idx) { idx_ = idx; }
        /*!
          @brief Implements write(). Write the value of a standard TIFF entry to
                 the \em ioWrapper, return the number of bytes written. Only the
                 \em ioWrapper and \em byteOrder arguments are used.
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        //! Implements count().
        virtual uint32_t doCount() const;
        /*!
          @brief Implements writeData(). Standard TIFF entries have no data:
                 write nothing and return 0.
         */
        virtual uint32_t doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Standard TIFF entries have no image data:
                 write nothing and return 0.
         */
        virtual uint32_t doWriteImage(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder) const;
        //! Implements size(). Return the size of a standard TIFF entry
        virtual uint32_t doSize() const;
        //! Implements sizeData(). Return 0.
        virtual uint32_t doSizeData() const;
        //! Implements sizeImage(). Return 0.
        virtual uint32_t doSizeImage() const;
        //@}

        //! Helper function to write an \em offset to a preallocated binary buffer
        static uint32_t writeOffset(byte*     buf,
                                    int32_t   offset,
                                    TiffType  tiffType,
                                    ByteOrder byteOrder);

    private:
        //! @name NOT implemented
        //@{
        //! Assignment operator.
        TiffEntryBase& operator=(const TiffEntryBase& rhs);
        //@}

        // DATA
        TiffType tiffType_;   //!< Field TIFF type
        uint32_t count_;      //!< The number of values of the indicated type
        int32_t  offset_;     //!< Offset to the data area
        /*!
          Size of the data buffer holding the value in bytes, there is no
          minimum size.
         */
        uint32_t size_;
        byte*    pData_;      //!< Pointer to the data area
        bool     isMalloced_; //!< True if this entry owns the value data
        int      idx_;        //!< Unique id of the entry in the image
        Value*   pValue_;     //!< Converted data value

    }; // class TiffEntryBase

    /*!
      @brief A standard TIFF IFD entry.
     */
    class TiffEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffEntry(uint16_t tag, IfdId group) : TiffEntryBase(tag, group) {}
        //! Virtual destructor.
        virtual ~TiffEntry();
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffEntry* doClone() const;
        //@}

    }; // class TiffEntry

    /*!
      @brief Interface for a standard TIFF IFD entry consisting of a value
             which is a set of offsets to a data area. The sizes of these "strips"
             are provided in a related TiffSizeEntry, tag and group of which are
             set in the constructor. The implementations of this interface differ
             in whether the data areas are extracted to the higher level metadata
             (TiffDataEntry) or not (TiffImageEntry).
     */
    class TiffDataEntryBase : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffDataEntryBase(uint16_t tag, IfdId group, uint16_t szTag, IfdId szGroup)
            : TiffEntryBase(tag, group),
              szTag_(szTag), szGroup_(szGroup) {}
        //! Virtual destructor.
        virtual ~TiffDataEntryBase();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the data areas ("strips").

          @param pSize Pointer to the Value holding the sizes corresponding
                       to this data entry.
          @param pData Pointer to the data area.
          @param sizeData Size of the data area.
          @param baseOffset Base offset into the data area.
         */
        virtual void setStrips(const Value* pSize,
                               const byte*  pData,
                               uint32_t     sizeData,
                               uint32_t     baseOffset) =0;
        //@}

        //! @name Accessors
        //@{
        //! Return the group of the entry which has the size
        uint16_t szTag()   const { return szTag_; }
        //! Return the group of the entry which has the size
        IfdId    szGroup() const { return szGroup_; }
        //@}

    private:
        // DATA
        const uint16_t szTag_;               //!< Tag of the entry with the size
        const IfdId    szGroup_;             //!< Group of the entry with the size

    }; // class TiffDataEntryBase

    /*!
      @brief A standard TIFF IFD entry consisting of a value which is an offset
             to a data area and the data area. The size of the data area is
             provided in a related TiffSizeEntry, tag and group of which are set
             in the constructor.

             This component extracts the data areas ("strips") and makes them
             available in the higher level metadata. It is used, e.g., for
             \em Exif.Thumbnail.JPEGInterchangeFormat for which the size
             is provided in \em Exif.Thumbnail.JPEGInterchangeFormatLength.
     */
    class TiffDataEntry : public TiffDataEntryBase {
        friend class TiffEncoder;
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffDataEntry(uint16_t tag, IfdId group, uint16_t szTag, IfdId szGroup)
            : TiffDataEntryBase(tag, group, szTag, szGroup),
              pDataArea_(0), sizeDataArea_(0) {}
        //! Virtual destructor.
        virtual ~TiffDataEntry();
        //@}

        //! @name Manipulators
        //@{
        virtual void setStrips(const Value* pSize,
                               const byte*  pData,
                               uint32_t     sizeData,
                               uint32_t     baseOffset);
        //@}

    protected:
        //! @name Protected Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        /*!
          @brief Implements write(). Write pointers into the data area to the
                 \em ioWrapper, relative to the offsets in the value. Return the
                 number of bytes written. The \em valueIdx argument is not used.

          The number of components in the value determines how many offsets are
          written. Set the first value to 0, the second to the size of the first
          data area, etc. when creating a new data entry. Offsets will be adjusted
          on write. The type of the value can only be signed or unsigned short or
          long.
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffDataEntry* doClone() const;
        /*!
          @brief Implements writeData(). Write the data area to the \em ioWrapper.
                 Return the number of bytes written.
         */
        virtual uint32_t doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        // Using doWriteImage from base class
        // Using doSize() from base class
        //! Implements sizeData(). Return the size of the data area.
        virtual uint32_t doSizeData() const;
        // Using doSizeImage from base class
        //@}

    private:
        // DATA
        byte*          pDataArea_;           //!< Pointer to the data area (never alloc'd)
        uint32_t       sizeDataArea_;        //!< Size of the data area

    }; // class TiffDataEntry

    /*!
      @brief A standard TIFF IFD entry consisting of a value which is an array
             of offsets to image data areas. The sizes of the image data areas are
             provided in a related TiffSizeEntry, tag and group of which are set
             in the constructor.

             The data is not extracted into the higher level metadata tags, it is
             only copied to the target image when the image is written.
             This component is used, e.g., for
             \em Exif.Image.StripOffsets for which the sizes are provided in
             \em Exif.Image.StripByteCounts.
     */
    class TiffImageEntry : public TiffDataEntryBase {
        friend class TiffEncoder;
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffImageEntry(uint16_t tag, IfdId group, uint16_t szTag, IfdId szGroup)
            : TiffDataEntryBase(tag, group, szTag, szGroup) {}
        //! Virtual destructor.
        virtual ~TiffImageEntry();
        //@}

        //! @name Manipulators
        //@{
        virtual void setStrips(const Value* pSize,
                               const byte*  pData,
                               uint32_t     sizeData,
                               uint32_t     baseOffset);
        //@}

    protected:
        //! @name Protected Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        /*!
          @brief Implements write(). Write pointers into the image data area to the
                 \em ioWrapper. Return the number of bytes written. The \em valueIdx
                 and \em dataIdx  arguments are not used.
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffImageEntry* doClone() const;
        /*!
          @brief Implements writeData(). Write the image data area to the \em ioWrapper.
                 Return the number of bytes written.

          This function writes the image data to the data area of the current
          directory. It is used for TIFF image entries in the makernote (large
          preview images) so that the image data remains in the makernote IFD.
         */
        virtual uint32_t doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Write the image data area to the \em ioWrapper.
                 Return the number of bytes written.
         */
        virtual uint32_t doWriteImage(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder) const;
        //! Implements size(). Return the size of the strip pointers.
        virtual uint32_t doSize() const;
        //! Implements sizeData(). Return the size of the image data area.
        virtual uint32_t doSizeData() const;
        //! Implements sizeImage(). Return the size of the image data area.
        virtual uint32_t doSizeImage() const;
        //@}

    private:
        //! Pointers to the image data (strips) and their sizes.
        typedef std::vector<std::pair<const byte*, uint32_t> > Strips;

        // DATA
        Strips   strips_;       //!< Image strips data (never alloc'd) and sizes

    }; // class TiffImageEntry

    /*!
      @brief A TIFF IFD entry containing the size of a data area of a related
             TiffDataEntry. This component is used, e.g. for
             \em Exif.Thumbnail.JPEGInterchangeFormatLength, which contains the
             size of \em Exif.Thumbnail.JPEGInterchangeFormat.
     */
    class TiffSizeEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffSizeEntry(uint16_t tag, IfdId group, uint16_t dtTag, IfdId dtGroup)
            : TiffEntryBase(tag, group), dtTag_(dtTag), dtGroup_(dtGroup) {}
        //! Virtual destructor.
        virtual ~TiffSizeEntry();
        //@}

        //! @name Accessors
        //@{
        //! Return the group of the related entry which has the data area
        uint16_t dtTag()   const { return dtTag_; }
        //! Return the group of the related entry which has the data area
        IfdId    dtGroup() const { return dtGroup_; }
        //@}

    protected:
        //! @name Protected Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffSizeEntry* doClone() const;
        //@}

    private:
        // DATA
        const uint16_t dtTag_;        //!< Tag of the entry with the data area
        const IfdId    dtGroup_;      //!< Group of the entry with the data area

    }; // class TiffSizeEntry

    /*!
      @brief This class models a TIFF directory (%Ifd). It is a composite
             component of the TIFF tree.
     */
    class TiffDirectory : public TiffComponent {
        friend class TiffEncoder;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffDirectory(uint16_t tag, IfdId group, bool hasNext =true)
            : TiffComponent(tag, group), hasNext_(hasNext), pNext_(0) {}
        //! Virtual destructor
        virtual ~TiffDirectory();
        //@}

        //! @name Accessors
        //@{
        //! Return true if the directory has a next pointer
        bool hasNext() const { return hasNext_; }
        //@}

    protected:
        //! @name Protected Creators
        //@{
        //! Copy constructor (used to implement clone()).
        TiffDirectory(const TiffDirectory& rhs);
        //@}

        //! @name Protected Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag,
                                         TiffPath& tiffPath,
                                         TiffComponent* const pRoot,
                                         TiffComponent::AutoPtr object);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual TiffComponent* doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        /*!
          @brief Implements write(). Write the TIFF directory, values and
                 additional data, including the next-IFD, if any, to the
                 \em ioWrapper, return the number of bytes written.
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffDirectory* doClone() const;
        /*!
          @brief This class does not really implement writeData(), it only has
                 write(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Write the image data of the TIFF
                 directory to the \em ioWrapper by forwarding the call to each
                 component as well as the next-IFD, if there is any. Return the
                 number of bytes written.
         */
        virtual uint32_t doWriteImage(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder) const;
        /*!
          @brief Implements size(). Return the size of the TIFF directory,
                 values and additional data, including the next-IFD, if any.
         */
        virtual uint32_t doSize() const;
        /*!
          @brief Implements count(). Return the number of entries in the TIFF
                 directory. Does not count entries which are marked as deleted.
         */
        virtual uint32_t doCount() const;
        /*!
          @brief This class does not really implement sizeData(), it only has
                 size(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doSizeData() const;
        /*!
          @brief Implements sizeImage(). Return the sum of the image sizes of
                 all components plus that of the next-IFD, if there is any.
         */
        virtual uint32_t doSizeImage() const;
        //@}

    private:
        //! @name NOT implemented
        //@{
        //! Assignment operator.
        TiffDirectory& operator=(const TiffDirectory& rhs);
        //@}

        //! @name Private Accessors
        //@{
        //! Write a binary directory entry for a TIFF component.
        uint32_t writeDirEntry(IoWrapper&     ioWrapper,
                               ByteOrder      byteOrder,
                               int32_t        offset,
                               TiffComponent* pTiffComponent,
                               uint32_t       valueIdx,
                               uint32_t       dataIdx,
                               uint32_t&      imageIdx) const;
        //@}

    private:
        // DATA
        Components components_; //!< List of components in this directory
        const bool hasNext_;    //!< True if the directory has a next pointer
        TiffComponent* pNext_;  //!< Pointer to the next IFD

    }; // class TiffDirectory

    /*!
      @brief This class models a TIFF sub-directory (sub-IFD). A sub-IFD
             is an entry with one or more values that are pointers to IFD
             structures containing an IFD. The TIFF standard defines
             some important tags to be sub-IFDs, including the %Exif and
             GPS tags.
     */
    class TiffSubIfd : public TiffEntryBase {
        friend class TiffReader;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffSubIfd(uint16_t tag, IfdId group, IfdId newGroup);
        //! Virtual destructor
        virtual ~TiffSubIfd();
        //@}

    protected:
        //! @name Protected Creators
        //@{
        //! Copy constructor (used to implement clone()).
        TiffSubIfd(const TiffSubIfd& rhs);
        //@}

        //! @name Protected Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag,
                                         TiffPath& tiffPath,
                                         TiffComponent* const pRoot,
                                         TiffComponent::AutoPtr object);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        /*!
          @brief Implements write(). Write the sub-IFD pointers to the \em ioWrapper,
                 return the number of bytes written. The \em valueIdx and
                 \em imageIdx arguments are not used.
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffSubIfd* doClone() const;
        /*!
          @brief Implements writeData(). Write the sub-IFDs to the \em ioWrapper.
                 Return the number of bytes written.
         */
        virtual uint32_t doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Write the image data of each sub-IFD to
                 the \em ioWrapper. Return the number of bytes written.
         */
        virtual uint32_t doWriteImage(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder) const;
        //! Implements size(). Return the size of the sub-Ifd pointers.
        uint32_t doSize() const;
        //! Implements sizeData(). Return the sum of the sizes of all sub-IFDs.
        virtual uint32_t doSizeData() const;
        //! Implements sizeImage(). Return the sum of the image sizes of all sub-IFDs.
        virtual uint32_t doSizeImage() const;
        //@}

    private:
        //! @name NOT implemented
        //@{
        //! Assignment operator.
        TiffSubIfd& operator=(const TiffSubIfd& rhs);
        //@}

        //! A collection of TIFF directories (IFDs)
        typedef std::vector<TiffDirectory*> Ifds;

        // DATA
        IfdId    newGroup_; //!< Start of the range of group numbers for the sub-IFDs
        Ifds     ifds_;     //!< The subdirectories

    }; // class TiffSubIfd

    /*!
      @brief This class is the basis for Makernote support in TIFF. It contains
             a pointer to a concrete Makernote. The TiffReader visitor has the
             responsibility to create the correct Make/Model specific Makernote
             for a particular TIFF file. Calls to child management methods are
             forwarded to the concrete Makernote, if there is one.
     */
    class TiffMnEntry : public TiffEntryBase {
        friend class TiffReader;
        friend class TiffDecoder;
        friend class TiffEncoder;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffMnEntry(uint16_t tag, IfdId group, IfdId mnGroup);
        //! Virtual destructor
        virtual ~TiffMnEntry();
        //@}

    protected:
        //! @name Protected Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag,
                                         TiffPath& tiffPath,
                                         TiffComponent* const pRoot,
                                         TiffComponent::AutoPtr object);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual TiffComponent* doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        /*!
          @brief Implements write() by forwarding the call to the actual
                 concrete Makernote, if there is one.
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffMnEntry* doClone() const;
        //! Implements count(). Return number of components in the entry.
        virtual uint32_t doCount() const;
        // Using doWriteData from base class
        // Using doWriteImage from base class
        /*!
          @brief Implements size() by forwarding the call to the actual
                 concrete Makernote, if there is one.
         */
        virtual uint32_t doSize() const;
        // Using doSizeData from base class
        // Using doSizeImage from base class
        //@}

    private:
        //! @name NOT implemented
        //@{
        //! Copy constructor.
        TiffMnEntry(const TiffMnEntry& rhs);
        //! Assignment operator.
        TiffMnEntry& operator=(const TiffMnEntry& rhs);
        //@}

        // DATA
        IfdId          mnGroup_;             //!< New group for concrete mn
        TiffComponent* mn_;                  //!< The Makernote

    }; // class TiffMnEntry

    /*!
      @brief Tiff IFD Makernote. This is a concrete class suitable for all
             IFD makernotes.

             Contains a makernote header (which can be 0) and an IFD and
             implements child mgmt functions to deal with the IFD entries. The
             various makernote weirdnesses are taken care of in the makernote
             header (and possibly in special purpose IFD entries).
     */
    class TiffIfdMakernote : public TiffComponent {
        friend class TiffReader;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffIfdMakernote(uint16_t  tag,
                         IfdId     group,
                         IfdId     mnGroup,
                         MnHeader* pHeader,
                         bool      hasNext =true);
        //! Virtual destructor
        virtual ~TiffIfdMakernote();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the header from a data buffer, return true if successful.

          The default implementation simply returns true.
         */
        bool readHeader(const byte* pData, uint32_t size, ByteOrder byteOrder);
        /*!
          @brief Set the byte order for the makernote.
         */
        void setByteOrder(ByteOrder byteOrder);
        /*!
          @brief Set the byte order used for the image.
         */
        void setImageByteOrder(ByteOrder byteOrder) { imageByteOrder_ = byteOrder; }
        //@}

        //! @name Accessors
        //@{
        //! Return the size of the header in bytes.
        uint32_t sizeHeader() const;
        //! Write the header to a data buffer, return the number of bytes written.
        uint32_t writeHeader(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        /*!
          @brief Return the offset to the makernote from the start of the
                 TIFF header.
        */
        uint32_t mnOffset() const;
        /*!
          @brief Return the offset to the start of the Makernote IFD from
                 the start of the Makernote.
                 Returns 0 if there is no header.
         */
        uint32_t ifdOffset() const;
        /*!
          @brief Return the byte order for the makernote. Requires the image
                 byte order to be set (setImageByteOrder()).  Returns the byte
                 order for the image if there is no header or the byte order for
                 the header is \c invalidByteOrder.
         */
        ByteOrder byteOrder() const;
        /*!
          @brief Return the byte order used for the image.
         */
        ByteOrder imageByteOrder() const { return imageByteOrder_; }
        /*!
          @brief Return the base offset for use with the makernote IFD entries
                 relative to the start of the TIFF header.
                 Returns 0 if there is no header.
         */
        uint32_t baseOffset() const;
        //@}

    protected:
        //! @name Protected Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag,
                                         TiffPath& tiffPath,
                                         TiffComponent* const pRoot,
                                         TiffComponent::AutoPtr object);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual TiffComponent* doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        /*!
          @brief Implements write(). Write the Makernote header, TIFF directory,
                 values and additional data to the \em ioWrapper, return the
                 number of bytes written.
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffIfdMakernote* doClone() const;
        /*!
          @brief This class does not really implement writeData(), it only has
                 write(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doWriteData(IoWrapper& ioWrapper,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Write the image data of the IFD of
                 the Makernote. Return the number of bytes written.
         */
        virtual uint32_t doWriteImage(IoWrapper& ioWrapper,
                                      ByteOrder byteOrder) const;
        /*!
          @brief Implements size(). Return the size of the Makernote header,
                 TIFF directory, values and additional data.
         */
        virtual uint32_t doSize() const;
        /*!
          @brief Implements count(). Return the number of entries in the IFD
                 of the Makernote. Does not count entries which are marked as
                 deleted.
         */
        virtual uint32_t doCount() const;
        /*!
          @brief This class does not really implement sizeData(), it only has
                 size(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doSizeData() const;
        /*!
          @brief Implements sizeImage(). Return the total image data size of the
                 makernote IFD.
         */
        virtual uint32_t doSizeImage() const;
        //@}

    private:
        /*!
          @name NOT implemented

          Implementing the copy constructor and assignment operator will require
          cloning the header, i.e., clone() functionality on the MnHeader
          hierarchy.
         */
        //@{
        //! Copy constructor.
        TiffIfdMakernote(const TiffIfdMakernote& rhs);
        //! Assignment operator.
        TiffIfdMakernote& operator=(const TiffIfdMakernote& rhs);
        //@}

        // DATA
        MnHeader*     pHeader_;                 //!< Makernote header
        TiffDirectory ifd_;                     //!< Makernote IFD
        uint32_t      mnOffset_;                //!< Makernote offset
        ByteOrder     imageByteOrder_;          //!< Byte order for the image

    }; // class TiffIfdMakernote

    /*!
      @brief Function pointer type for a function to determine which cfg + def
             of a corresponding array set to use.
     */
    typedef int (*CfgSelFct)(uint16_t, const byte*, uint32_t, TiffComponent* const);

    //! Function pointer type for a crypt function used for binary arrays.
    typedef DataBuf (*CryptFct)(uint16_t, const byte*, uint32_t, TiffComponent* const);

    //! Defines one tag in a binary array
    struct ArrayDef {
        //! Comparison with idx
        bool operator==(uint32_t idx) const { return idx_ == idx; }
        //! Get the size in bytes of a tag.
        uint32_t size(uint16_t tag, IfdId group) const;
        // DATA
        uint32_t idx_;             //!< Index in bytes from the start
        TiffType tiffType_;        //!< TIFF type of the element
        uint32_t count_;           //!< Number of components
    };

    //! Additional configuration for a binary array.
    struct ArrayCfg {
        /*!
          @brief Return the size of the default tag, which is used
                 to calculate tag numbers as idx/tagStep
         */
        uint32_t tagStep() const { return elDefaultDef_.size(0, group_); }
        //DATA
        IfdId       group_;        //!< Group for the elements
        ByteOrder   byteOrder_;    //!< Byte order, invalidByteOrder to inherit
        TiffType    elTiffType_;   //!< Type for the array entry and the size element, if any
        CryptFct    cryptFct_;     //!< Crypt function, 0 if not used
        bool        hasSize_;      //!< If true, first tag is the size element
        bool        hasFillers_;   //!< If true, write all defined tags
        bool        concat_;       //!< If true, concatenate gaps between defined tags to single tags
        ArrayDef    elDefaultDef_; //!< Default element
    };

    //! Combination of array configuration and definition for arrays
    struct ArraySet {
        const ArrayCfg  cfg_;      //!< Binary array configuration
        const ArrayDef* def_;      //!< Binary array definition array
        const int       defSize_;  //!< Size of the array definition array
    };

    /*!
      @brief Composite to model an array of different tags. The tag types as well
             as other aspects of the array are configurable. The elements of this
             component are of type TiffBinaryElement.
     */
    class TiffBinaryArray : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffBinaryArray(uint16_t tag,
                        IfdId group,
                        const ArrayCfg* arrayCfg,
                        const ArrayDef* arrayDef,
                        int defSize);
        //! Constructor for a complex binary array
        TiffBinaryArray(uint16_t tag,
                        IfdId group,
                        const ArraySet* arraySet,
                        int setSize,
                        CfgSelFct cfgSelFct);
        //! Virtual destructor
        virtual ~TiffBinaryArray();
        //@}

        //! @name Manipulators
        //@{
        //! Add an element to the binary array, return the size of the element
        uint32_t addElement(uint32_t idx, const ArrayDef& def);
        /*!
          @brief Setup cfg and def for the component, in case of a complex binary array.
                 Else do nothing. Return true if the initialization succeeded, else false.

          This version of initialize() is used during intrusive writing. It determines the
          correct settings based on the \em group passed in (which is the group of the first
          tag that is added to the array). It doesn't require cfgSelFct_.

          @param group Group to setup the binary array for.
          @return true if the initialization succeeded, else false.
         */
        bool initialize(IfdId group);
        /*!
          @brief Setup cfg and def for the component, in case of a complex binary array.
                 Else do nothing. Return true if the initialization succeeded, else false.

          This version of initialize() is used for reading and non-intrusive writing. It
          calls cfgSelFct_ to determine the correct settings.

          @param pRoot Pointer to the root component of the TIFF tree.
          @return true if the initialization succeeded, else false.
         */
        bool initialize(TiffComponent* const pRoot);
        //! Initialize the original data buffer and its size from the base entry.
        void iniOrigDataBuf();
        //! Update the original data buffer and its size, return true if successful.
        bool updOrigDataBuf(const byte* pData, uint32_t size);
        //! Set a flag to indicate if the array was decoded
        void setDecoded(bool decoded) { decoded_ = decoded; }
        //@}

        //! @name Accessors
        //@{
        //! Return a pointer to the configuration
        const ArrayCfg* cfg() const { return arrayCfg_; }
        //! Return a pointer to the definition
        const ArrayDef* def() const { return arrayDef_; }
        //! Return the number of elements in the definition
        int defSize() const { return defSize_; }
        //! Return the flag which indicates if the array was decoded
        bool decoded() const { return decoded_; }
        //@}

    protected:
        //! @name Protected Creators
        //@{
        //! Copy constructor (used to implement clone()).
        TiffBinaryArray(const TiffBinaryArray& rhs);
        //@}

        //! @name Protected Manipulators
        //@{
        /*!
          @brief Implements addPath(). Todo: Document it!
         */
        virtual TiffComponent* doAddPath(uint16_t tag,
                                         TiffPath& tiffPath,
                                         TiffComponent* const pRoot,
                                         TiffComponent::AutoPtr object);
        /*!
          @brief Implements addChild(). Todo: Document it!
         */
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        /*!
          @brief Implements write(). Todo: Document it!
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffBinaryArray* doClone() const;
        //! Implements count(). Todo: Document it!
        virtual uint32_t doCount() const;
        // Using doWriteData from base class
        // Using doWriteImage from base class
        /*!
          @brief Implements size(). Todo: Document it!
         */
        virtual uint32_t doSize() const;
        // Using doSizeData from base class
        // Using doSizeImage from base class
        //@}

    private:
        //! @name NOT implemented
        //@{
        //! Assignment operator.
        TiffBinaryArray& operator=(const TiffBinaryArray& rhs);
        //@}

        // DATA
        const CfgSelFct cfgSelFct_; //!< Pointer to a function to determine which cfg to use (may be 0)
        const ArraySet* arraySet_;  //!< Pointer to the array set, if any (may be 0)
        const ArrayCfg* arrayCfg_;  //!< Pointer to the array configuration (must not be 0, except for unrecognized complex binary arrays)
        const ArrayDef* arrayDef_;  //!< Pointer to the array definition (may be 0)
        int defSize_;               //!< Size of the array definition array (may be 0)
        int setSize_;               //!< Size of the array set (may be 0)
        Components elements_;       //!< List of elements in this composite
        byte* origData_;            //!< Pointer to the original data buffer (unencrypted)
        uint32_t origSize_;         //!< Size of the original data buffer
        TiffComponent* pRoot_;      //!< Pointer to the root component of the TIFF tree. (Only used for intrusive writing.)
        bool decoded_;              //!< Flag to indicate if the array was decoded
    }; // class TiffBinaryArray

    /*!
      @brief Element of a TiffBinaryArray.
     */
    class TiffBinaryElement : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffBinaryElement(uint16_t tag, IfdId group);
        //! Virtual destructor.
        virtual ~TiffBinaryElement();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the array definition for this element.
         */
        void setElDef(const ArrayDef& def) { elDef_ = def; }
        /*!
          @brief Set the byte order of this element.
         */
        void setElByteOrder(ByteOrder byteOrder) { elByteOrder_ = byteOrder; }
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the array definition of this element.
         */
        const ArrayDef* elDef()       const { return &elDef_; }
        /*!
          @brief Return the byte order of this element.
         */
        ByteOrder       elByteOrder() const { return elByteOrder_; }
        //@}

    protected:
        //! @name Protected Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        /*!
          @brief Implements write(). Todo: Document it!
         */
        virtual uint32_t doWrite(IoWrapper& ioWrapper,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}

        //! @name Protected Accessors
        //@{
        virtual TiffBinaryElement* doClone() const;
        /*!
          @brief Implements count(). Returns the count from the element definition.
         */
        virtual uint32_t doCount() const;
        // Using doWriteData from base class
        // Using doWriteImage from base class
        /*!
          @brief Implements size(). Returns count * type-size, both taken from
                 the element definition.
         */
        virtual uint32_t doSize() const;
        // Using doSizeData from base class
        // Using doSizeImage from base class
        //@}

    private:
        // DATA
        ArrayDef  elDef_;        //!< The array element definition
        ByteOrder elByteOrder_;  //!< Byte order to read/write the element

    }; // class TiffBinaryElement

// *****************************************************************************
// template, inline and free functions

    /*!
      @brief Compare two TIFF component pointers by tag. Return true if the tag
             of component lhs is less than that of rhs.
     */
    bool cmpTagLt(TiffComponent const* lhs, TiffComponent const* rhs);

    /*!
      @brief Compare two TIFF component pointers by group. Return true if the
             group of component lhs is less than that of rhs.
     */
    bool cmpGroupLt(TiffComponent const* lhs, TiffComponent const* rhs);

    //! Function to create and initialize a new TIFF entry
    TiffComponent::AutoPtr newTiffEntry(uint16_t tag, IfdId group);

    //! Function to create and initialize a new TIFF makernote entry
    TiffComponent::AutoPtr newTiffMnEntry(uint16_t tag, IfdId group);

    //! Function to create and initialize a new binary array element
    TiffComponent::AutoPtr newTiffBinaryElement(uint16_t tag, IfdId group);

    //! Function to create and initialize a new TIFF directory
    template<IfdId newGroup>
    TiffComponent::AutoPtr newTiffDirectory(uint16_t tag, IfdId /*group*/)
    {
        return TiffComponent::AutoPtr(new TiffDirectory(tag, newGroup));
    }

    //! Function to create and initialize a new TIFF sub-directory
    template<IfdId newGroup>
    TiffComponent::AutoPtr newTiffSubIfd(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(new TiffSubIfd(tag, group, newGroup));
    }

    //! Function to create and initialize a new binary array entry
    template<const ArrayCfg* arrayCfg, int N, const ArrayDef (&arrayDef)[N]>
    TiffComponent::AutoPtr newTiffBinaryArray0(uint16_t tag, IfdId group)
    {
        // *& acrobatics is a workaround for a MSVC 7.1 bug
        return TiffComponent::AutoPtr(
            new TiffBinaryArray(tag, group, arrayCfg, *(&arrayDef), N));
    }

    //! Function to create and initialize a new simple binary array entry
    template<const ArrayCfg* arrayCfg>
    TiffComponent::AutoPtr newTiffBinaryArray1(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(
            new TiffBinaryArray(tag, group, arrayCfg, 0, 0));
    }

    //! Function to create and initialize a new complex binary array entry
    template<const ArraySet* arraySet, int N, CfgSelFct cfgSelFct>
    TiffComponent::AutoPtr newTiffBinaryArray2(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(
            new TiffBinaryArray(tag, group, arraySet, N, cfgSelFct));
    }

    //! Function to create and initialize a new TIFF entry for a thumbnail (data)
    template<uint16_t szTag, IfdId szGroup>
    TiffComponent::AutoPtr newTiffThumbData(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(
            new TiffDataEntry(tag, group, szTag, szGroup));
    }

    //! Function to create and initialize a new TIFF entry for a thumbnail (size)
    template<uint16_t dtTag, IfdId dtGroup>
    TiffComponent::AutoPtr newTiffThumbSize(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(
            new TiffSizeEntry(tag, group, dtTag, dtGroup));
    }

    //! Function to create and initialize a new TIFF entry for image data
    template<uint16_t szTag, IfdId szGroup>
    TiffComponent::AutoPtr newTiffImageData(uint16_t tag, IfdId group)
    {
        return TiffComponent::AutoPtr(
            new TiffImageEntry(tag, group, szTag, szGroup));
    }

    //! Function to create and initialize a new TIFF entry for image data (size)
    template<uint16_t dtTag, IfdId dtGroup>
    TiffComponent::AutoPtr newTiffImageSize(uint16_t tag, IfdId group)
    {
        // Todo: Same as newTiffThumbSize - consolidate (rename)?
        return TiffComponent::AutoPtr(
            new TiffSizeEntry(tag, group, dtTag, dtGroup));
    }

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef TIFFCOMPOSITE_INT_HPP_
