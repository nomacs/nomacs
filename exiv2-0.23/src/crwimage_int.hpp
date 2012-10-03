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
  @file    crwimage_int.hpp
  @brief   Internal classes to support CRW/CIFF format.
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    28-Aug-05, ahu: created
 */
#ifndef CRWIMAGE_INT_HPP_
#define CRWIMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "tags_int.hpp"
#include "image.hpp"
#include "basicio.hpp"

// + standard includes
#include <iosfwd>
#include <string>
#include <vector>
#include <stack>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;

    namespace Internal {

// *****************************************************************************
// class declarations
    class CiffHeader;
    class CiffComponent;
    struct CrwMapping;
    struct CrwSubDir;

// *****************************************************************************
// type definitions

    //! Function pointer for functions to decode Exif tags from a CRW entry
    typedef void (*CrwDecodeFct)(const CiffComponent&,
                                 const CrwMapping*,
                                 Image&,
                                 ByteOrder);

    //! Function pointer for functions to encode CRW entries from Exif tags
    typedef void (*CrwEncodeFct)(const Image&,
                                 const CrwMapping*,
                                 CiffHeader*);

    //! Stack to hold a path of CRW directories
    typedef std::stack<CrwSubDir> CrwDirs;

    //! Type to identify where the data is stored in a directory
    enum DataLocId {
        invalidDataLocId,
        valueData,
        directoryData,
        lastDataLocId
    };

// *****************************************************************************
// class definitions

    /*!
      @brief Interface class for components of the CIFF directory hierarchy of a
             CRW (Canon Raw data) image. Both CIFF directories as well as
             entries implement this interface. This class is implemented as NVI
             (non-virtual interface).
     */
    class CiffComponent {
    public:
        //! CiffComponent auto_ptr type
        typedef std::auto_ptr<CiffComponent> AutoPtr;
        //! Container type to hold all metadata
        typedef std::vector<CiffComponent*> Components;

        //! @name Creators
        //@{
        //! Default constructor
        CiffComponent()
            : dir_(0), tag_(0), size_(0), offset_(0), pData_(0),
              isAllocated_(false) {}
        //! Constructor taking a tag and directory
        CiffComponent(uint16_t tag, uint16_t dir)
            : dir_(dir), tag_(tag), size_(0), offset_(0), pData_(0),
              isAllocated_(false) {}
        //! Virtual destructor.
        virtual ~CiffComponent();
        //@}

        //! @name Manipulators
        //@{
        // Default assignment operator is fine

        //! Add a component to the composition
        void add(AutoPtr component);
        /*!
          @brief Add \em crwTagId to the parse tree, if it doesn't exist
                 yet. \em crwDirs contains the path of subdirectories, starting
                 with the root directory, leading to \em crwTagId. Directories
                 that don't exist yet are added along the way. Returns a pointer
                 to the newly added component.

          @param crwDirs   Subdirectory path from root to the subdirectory
                           containing the tag to be added.
          @param crwTagId  Tag to be added.

          @return A pointer to the newly added component.
         */
        CiffComponent* add(CrwDirs& crwDirs, uint16_t crwTagId);
        /*!
          @brief Remove \em crwTagId from the parse tree, if it exists yet. \em
                 crwDirs contains the path of subdirectories, starting with the
                 root directory, leading to \em crwTagId.

          @param crwDirs   Subdirectory path from root to the subdirectory
                           containing the tag to be removed.
          @param crwTagId  Tag to be removed.
         */
        void remove(CrwDirs& crwDirs, uint16_t crwTagId);
        /*!
          @brief Read a component from a data buffer

          @param pData     Pointer to the data buffer.
          @param size      Number of bytes in the data buffer.
          @param start     Component starts at \em pData + \em start.
          @param byteOrder Applicable byte order (little or big endian).

          @throw Error If the component cannot be parsed.
         */
        void read(const byte* pData,
                  uint32_t    size,
                  uint32_t    start,
                  ByteOrder   byteOrder);
        /*!
          @brief Write the metadata from the raw metadata component to the
                 binary image \em blob. This method may append to the blob.

          @param blob      Binary image to add metadata to
          @param byteOrder Byte order
          @param offset    Current offset

          @return New offset
         */
        uint32_t write(Blob& blob, ByteOrder byteOrder, uint32_t offset);
        /*!
          @brief Writes the entry's value if size is larger than eight bytes. If
                 needed, the value is padded with one 0 byte to make the number
                 of bytes written to the blob even. The offset of the component
                 is set to the offset passed in.
          @param blob The binary image to write to.
          @param offset Offset from the start of the directory for this entry.

          @return New offset.
         */
        uint32_t writeValueData(Blob& blob, uint32_t offset);
        //! Set the directory tag for this component.
        void setDir(uint16_t dir)       { dir_ = dir; }
        //! Set the data value of the entry.
        void setValue(DataBuf buf);
        //@}

        //! Return the type id for a tag
        static TypeId typeId(uint16_t tag);
        //! Return the data location id for a tag
        static DataLocId dataLocation(uint16_t tag);

        //! @name Accessors
        //@{
        /*!
          @brief Decode metadata from the component and add it to
                 \em image.

          @param image Image to add metadata to
          @param byteOrder Byte order
         */
        void decode(Image& image, ByteOrder byteOrder) const;
        /*!
          @brief Print debug info about a component to \em os.

          @param os Output stream to write to
          @param byteOrder Byte order
          @param prefix Prefix to be written before each line of output
         */
        void print(std::ostream& os,
                   ByteOrder byteOrder,
                   const std::string& prefix ="") const;
        /*!
          @brief Write a directory entry for the component to the \em blob.
                 If the size of the data is not larger than 8 bytes, the
                 data is written to the directory entry.
         */
        void writeDirEntry(Blob& blob, ByteOrder byteOrder) const;
        //! Return the tag of the directory containing this component
        uint16_t dir()           const { return dir_; }

        //! Return the tag of this component
        uint16_t tag()           const { return tag_; }

        //! Return true if the component is empty, else false
        bool empty()             const;

        /*!
          @brief Return the data size of this component

          @note If the data is contained in the directory entry itself,
                this method returns 8, which is the maximum number
                of data bytes this component can have. The actual size,
                i.e., used data bytes, may be less than 8.
         */
        uint32_t size()          const { return size_; }

        //! Return the offset to the data from the start of the directory
        uint32_t offset()        const { return offset_; }

        //! Return a pointer to the data area of this component
        const byte* pData()      const { return pData_; }

        //! Return the tag id of this component
        uint16_t tagId()         const { return tag_ & 0x3fff; }

        //! Return the type id of thi component
        TypeId typeId()          const { return typeId(tag_); }

        //! Return the data location for this component
        DataLocId dataLocation() const { return dataLocation(tag_); }

        /*!
          @brief Finds \em crwTagId in directory \em crwDir, returning a pointer to
                 the component or 0 if not found.
         */
        CiffComponent* findComponent(uint16_t crwTagId, uint16_t crwDir) const;
        //@}

    protected:
        //! @name Manipulators
        //@{
        //! Implements add()
        virtual void doAdd(AutoPtr component) =0;
        //! Implements add(). The default implementation does nothing.
        virtual CiffComponent* doAdd(CrwDirs& crwDirs, uint16_t crwTagId);
        //! Implements remove(). The default implementation does nothing.
        virtual void doRemove(CrwDirs& crwDirs, uint16_t crwTagId);
        //! Implements read(). The default implementation reads a directory entry.
        virtual void doRead(const byte* pData,
                            uint32_t    size,
                            uint32_t    start,
                            ByteOrder   byteOrder);
        //! Implements write()
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 uint32_t  offset) =0;
        //! Set the size of the data area.
        void setSize(uint32_t size)        { size_ = size; }
        //! Set the offset for this component.
        void setOffset(uint32_t offset)    { offset_ = offset; }
        //@}

        //! @name Accessors
        //@{
        //! Implements decode()
        virtual void doDecode(Image& image,
                               ByteOrder byteOrder) const =0;
        //! Implements print(). The default implementation prints the entry.
        virtual void doPrint(std::ostream&      os,
                             ByteOrder          byteOrder,
                             const std::string& prefix) const;
        //! Implements empty(). Default implementation returns true if size is 0.
        virtual bool doEmpty() const;
        //! Implements findComponent(). The default implementation checks the entry.
        virtual CiffComponent* doFindComponent(uint16_t crwTagId,
                                               uint16_t crwDir) const;
        //@}

    private:
        // DATA
        uint16_t    dir_;         //!< Tag of the directory containing this component
        uint16_t    tag_;         //!< Tag of the entry
        uint32_t    size_;        //!< Size of the data area
        uint32_t    offset_;      //!< Offset to the data area from start of dir
        const byte* pData_;       //!< Pointer to the data area
        bool        isAllocated_; //!< True if this entry owns the value data

    }; // class CiffComponent

    /*!
      @brief This class models one directory entry of a CIFF directory of
             a CRW (Canon Raw data) image.
     */
    class CiffEntry : public CiffComponent {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        CiffEntry() {}
        //! Constructor taking a tag and directory
        CiffEntry(uint16_t tag, uint16_t dir) : CiffComponent(tag, dir) {}

        //! Virtual destructor.
        virtual ~CiffEntry();
        //@}

        // Default assignment operator is fine

    private:
        //! @name Manipulators
        //@{
        using CiffComponent::doAdd;
        // See base class comment
        virtual void doAdd(AutoPtr component);
        /*!
          @brief Implements write(). Writes only the value data of the entry,
                 using writeValueData().
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 uint32_t  offset);
        //@}

        //! @name Accessors
        //@{
        // See base class comment
        virtual void doDecode(Image& image, ByteOrder byteOrder) const;
        //@}

    }; // class CiffEntry

    //! This class models a CIFF directory of a CRW (Canon Raw data) image.
    class CiffDirectory : public CiffComponent {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        CiffDirectory() {}
        //! Constructor taking a tag and directory
        CiffDirectory(uint16_t tag, uint16_t dir) : CiffComponent(tag, dir) {}

        //! Virtual destructor
        virtual ~CiffDirectory();
        //@}

        //! @name Manipulators
        //@{
        // Default assignment operator is fine

        /*!
          @brief Parse a CIFF directory from a memory buffer

          @param pData     Pointer to the memory buffer containing the directory
          @param size      Size of the memory buffer
          @param byteOrder Applicable byte order (little or big endian)
         */
        void readDirectory(const byte* pData,
                           uint32_t    size,
                           ByteOrder   byteOrder);
        //@}

    private:
        //! @name Manipulators
        //@{
        // See base class comment
        virtual void doAdd(AutoPtr component);
        // See base class comment
        virtual CiffComponent* doAdd(CrwDirs& crwDirs, uint16_t crwTagId);
        // See base class comment
        virtual void doRemove(CrwDirs& crwDirs, uint16_t crwTagId);
        /*!
          @brief Implements write(). Writes the complete Ciff directory to
                 the blob.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 uint32_t  offset);
        // See base class comment
        virtual void doRead(const byte* pData,
                            uint32_t    size,
                            uint32_t    start,
                            ByteOrder   byteOrder);
        //@}

        //! @name Accessors
        //@{
        // See base class comment
        virtual void doDecode(Image&    image,
                              ByteOrder byteOrder) const;

        // See base class comment
        virtual void doPrint(std::ostream&      os,
                             ByteOrder          byteOrder,
                             const std::string& prefix) const;

        //! See base class comment. A directory is empty if it has no components.
        virtual bool doEmpty() const;

        // See base class comment
        virtual CiffComponent* doFindComponent(uint16_t crwTagId,
                                               uint16_t crwDir) const;
        //@}

    private:
        // DATA
        Components components_; //!< List of components in this dir

    }; // class CiffDirectory

    /*!
      @brief This class models the header of a CRW (Canon Raw data) image.  It
             is the head of a CIFF parse tree, consisting of CiffDirectory and
             CiffEntry objects. Most of its methods will walk the parse tree to
             perform the requested action.
     */
    class CiffHeader {
    public:
        //! CiffHeader auto_ptr type
        typedef std::auto_ptr<CiffHeader> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor
        CiffHeader()
            : pRootDir_  (0),
              byteOrder_ (littleEndian),
              offset_    (0x0000001a),
              pPadding_  (0),
              padded_    (0)
            {}
        //! Virtual destructor
        virtual ~CiffHeader();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the CRW image from a data buffer, starting with the Ciff
                 header.

          @param pData Pointer to the data buffer.
          @param size  Number of bytes in the data buffer.

          @throw Error If the image cannot be parsed.
         */
        void read(const byte* pData, uint32_t size);
        /*!
          @brief Set the value of entry \em crwTagId in directory \em crwDir to
                 \em buf. If this tag doesn't exist, it is added along with all
                 directories needed.

          @param crwTagId Tag to be added.
          @param crwDir   Parent directory of the tag.
          @param buf      Value to be set.
         */
        void add(uint16_t crwTagId, uint16_t crwDir, DataBuf buf);
        /*!
          @brief Remove entry \em crwTagId in directory \em crwDir from the parse
                 tree. If it's the last entry in the directory, the directory is
                 removed as well, etc.

          @param crwTagId Tag id to be removed.
          @param crwDir   Parent directory of the tag.
         */
        void remove(uint16_t crwTagId, uint16_t crwDir);
        //@}

        //! Return a pointer to the Canon CRW signature.
        static const char* signature() { return signature_; }

        //! @name Accessors
        //@{
        /*!
          @brief Write the CRW image to the binary image \em blob, starting with
                 the Ciff header. This method appends to the blob.

          @param blob Binary image to add to.

          @throw Error If the image cannot be written.
         */
        void write(Blob& blob) const;
        /*!
          @brief Decode the CRW image and add it to \em image.

          Walk the parse tree and convert CIFF entries to metadata
          entries which are added to \em image.

          @param image Image to add metadata to
         */
        void decode(Image& image) const;
        /*!
          @brief Print debug info for the CRW image to \em os.

          @param os Output stream to write to.
          @param prefix Prefix to be written before each line of output.
         */
        void print(std::ostream& os, const std::string& prefix ="") const;
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        /*!
          @brief Finds \em crwTagId in directory \em crwDir in the parse tree,
                 returning a pointer to the component or 0 if not found.
         */
        CiffComponent* findComponent(uint16_t crwTagId, uint16_t crwDir) const;
        //@}

    private:
        // DATA
        static const char signature_[];   //!< Canon CRW signature "HEAPCCDR"

        CiffDirectory*    pRootDir_;      //!< Pointer to the root directory
        ByteOrder         byteOrder_;     //!< Applicable byte order
        uint32_t          offset_;        //!< Offset to the start of the root dir
        byte*             pPadding_;      //!< Pointer to the (unknown) remainder
        uint32_t          padded_;        //!< Number of padding-bytes

    }; // class CiffHeader

    //! Structure for the CIFF directory hierarchy
    struct CrwSubDir {
        uint16_t crwDir_;                 //!< Directory tag
        uint16_t parent_;                 //!< Parent directory tag
    }; // struct CrwSubDir

    /*!
      @brief Structure for a mapping table for conversion of CIFF entries to
             image metadata and vice versa.
     */
    struct CrwMapping {
        //! @name Creators
        //@{
        //! Default constructor
        CrwMapping(
            uint16_t      crwTagId,
            uint16_t      crwDir,
            uint32_t      size,
            uint16_t      tag,
            Internal::IfdId ifdId,
            CrwDecodeFct  toExif,
            CrwEncodeFct  fromExif)
            : crwTagId_ (crwTagId),
              crwDir_   (crwDir),
              size_     (size),
              tag_      (tag),
              ifdId_    (ifdId),
              toExif_   (toExif),
              fromExif_ (fromExif)
            {}
        //@}

        // DATA
        uint16_t      crwTagId_;  //!< CRW tag id
        uint16_t      crwDir_;    //!< CRW directory tag
        uint32_t      size_;      //!< Data size (overwrites the size from the entry)
        uint16_t      tag_;       //!< Exif tag to map to
        IfdId         ifdId_;     //!< Exif Ifd id to map to
        CrwDecodeFct  toExif_;    //!< Conversion function
        CrwEncodeFct  fromExif_;  //!< Reverse conversion function

    }; // struct CrwMapping

    /*!
      @brief Static class providing mapping functionality from CRW entries
             to image metadata and vice versa
     */
    class CrwMap {
        //! @name Not implemented
        //@{
        //! Default constructor
        CrwMap();
        //@}

    public:
        /*!
          @brief Decode image metadata from a CRW entry, convert and add it
                 to the image metadata. This function converts only one CRW
                 component.

          @param ciffComponent Source CIFF entry
          @param image         Destination image for the metadata
          @param byteOrder     Byte order in which the data of the entry
                               is encoded
         */
        static void decode(const CiffComponent& ciffComponent,
                                 Image&         image,
                                 ByteOrder      byteOrder);
        /*!
          @brief Encode image metadata from \em image into the CRW parse tree.
                 This function converts all Exif metadata that %Exiv2 can
                 convert to CRW format, in a loop through the entries of the
                 mapping table.

          @param pHead         Destination parse tree.
          @param image         Source image containing the metadata.
         */
        static void encode(CiffHeader* pHead, const Image& image);

        /*!
          @brief Load the stack: loop through the CRW subdirs hierarchy and push
                 all directories on the path from \em crwDir to root onto the
                 stack \em crwDirs. Requires the subdirs array to be arranged in
                 bottom-up order to be able to finish in only one pass.
         */
        static void loadStack(CrwDirs& crwDirs, uint16_t crwDir);

    private:
        //! Return conversion information for one \em crwDir and \em crwTagId
        static const CrwMapping* crwMapping(uint16_t crwDir, uint16_t crwTagId);

        /*!
          @brief Standard decode function to convert CRW entries to
                 Exif metadata.

          Uses the mapping defined in the conversion structure \em pCrwMapping
          to convert the data. If the \em size field in the conversion structure
          is not 0, then it is used instead of the \em size provided by the
          entry itself.
         */
        static void decodeBasic(const CiffComponent& ciffComponent,
                                const CrwMapping*    pCrwMapping,
                                      Image&         image,
                                      ByteOrder      byteOrder);

        //! Decode the user comment
        static void decode0x0805(const CiffComponent& ciffComponent,
                                 const CrwMapping*    pCrwMapping,
                                       Image&         image,
                                       ByteOrder      byteOrder);

        //! Decode camera Make and Model information
        static void decode0x080a(const CiffComponent& ciffComponent,
                                 const CrwMapping*    pCrwMapping,
                                       Image&         image,
                                       ByteOrder      byteOrder);

        //! Decode Canon Camera Settings 1, 2 and Custom Function arrays
        static void decodeArray(const CiffComponent& ciffComponent,
                                const CrwMapping*    pCrwMapping,
                                      Image&         image,
                                      ByteOrder      byteOrder);

        //! Decode the date when the picture was taken
        static void decode0x180e(const CiffComponent& ciffComponent,
                                 const CrwMapping*    pCrwMapping,
                                       Image&         image,
                                       ByteOrder      byteOrder);

        //! Decode image width and height
        static void decode0x1810(const CiffComponent& ciffComponent,
                                 const CrwMapping*    pCrwMapping,
                                       Image&         image,
                                       ByteOrder      byteOrder);

        //! Decode the thumbnail image
        static void decode0x2008(const CiffComponent& ciffComponent,
                                 const CrwMapping*    pCrwMapping,
                                       Image&         image,
                                       ByteOrder      byteOrder);

        /*!
          @brief Standard encode function to convert Exif metadata to Crw
                 entries.

          This is the basic encode function taking one Exif key and converting
          it to one Ciff entry. Both are available in the \em pCrwMapping passed
          in.

          @param image Image with the metadata to encode
          @param pCrwMapping Pointer to an entry into the \em crwMapping_ table
                       with information on the source and target metadata entries.
          @param pHead Pointer to the head of the CIFF parse tree into which
                       the metadata from \em image is encoded.
         */
        static void encodeBasic(const Image&      image,
                                const CrwMapping* pCrwMapping,
                                      CiffHeader* pHead);

        //! Encode the user comment
        static void encode0x0805(const Image&      image,
                                 const CrwMapping* pCrwMapping,
                                       CiffHeader* pHead);

        //! Encode camera Make and Model information
        static void encode0x080a(const Image&      image,
                                 const CrwMapping* pCrwMapping,
                                       CiffHeader* pHead);

        //! Encode Canon Camera Settings 1, 2 and Custom Function arrays
        static void encodeArray(const Image&      image,
                                const CrwMapping* pCrwMapping,
                                      CiffHeader* pHead);

        //! Encode the date when the picture was taken
        static void encode0x180e(const Image&      image,
                                 const CrwMapping* pCrwMapping,
                                       CiffHeader* pHead);

        //! Encode image width and height
        static void encode0x1810(const Image&      image,
                                 const CrwMapping* pCrwMapping,
                                       CiffHeader* pHead);

        //! Encode the thumbnail image
        static void encode0x2008(const Image&      image,
                                 const CrwMapping* pCrwMapping,
                                       CiffHeader* pHead);
    private:
        // DATA
        static const CrwMapping crwMapping_[]; //!< Metadata conversion table
        static const CrwSubDir  crwSubDir_[];  //!< Ciff directory hierarchy

    }; // class CrwMap

// *****************************************************************************
// template, inline and free functions

    /*!
      @brief Pack the tag values of all \em ifdId tags in \em exifData into a
             data buffer. This function is used to pack Canon Camera Settings1,2
             and Custom Function tags.
     */
    DataBuf packIfdId(const ExifData& exifData,
                            IfdId     ifdId,
                            ByteOrder byteOrder);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef CRWIMAGE_INT_HPP_
