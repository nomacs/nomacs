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
  @file    tiffvisitor_int.hpp
  @brief   Internal operations on a TIFF composite tree, implemented as visitor
           classes.
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef TIFFVISITOR_INT_HPP_
#define TIFFVISITOR_INT_HPP_

// *****************************************************************************
// included header files
#include "exif.hpp"
#include "tifffwd_int.hpp"
#include "types.hpp"

// + standard includes
#include <memory>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <map>
#include <set>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

    class IptcData;
    class XmpData;

    namespace Internal {

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class defining the interface for TIFF composite
             vistors (Visitor pattern)

      A concrete visitor class is used as shown in the example below. Accept()
      will invoke the member function corresponding to the concrete type of each
      component in the composite.

      @code
      void visitorExample(Exiv2::TiffComponent* tiffComponent, Exiv2::TiffVisitor& visitor)
      {
          tiffComponent->accept(visitor);
      }
      @endcode
     */
    class TiffVisitor {
    public:
        //! Events for the stop/go flag. See setGo().
        enum GoEvent {
            //! Signal to control traversing of the composite tree.
            geTraverse       = 0,
            //! Signal used by TiffReader to signal an unknown makernote.
            geKnownMakernote = 1
            // Note: If you add more events here, adjust the events_ constant too!
        };

    private:
        static const int events_ = 2;  //!< The number of stop/go flags.
        bool go_[events_];             //!< Array of stop/go flags. See setGo().

    public:
        //! @name Creators
        //@{
        //! Default constructor. Initialises all stop/go flags to true.
        TiffVisitor();
        //! Virtual destructor
        virtual ~TiffVisitor();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the stop/go flag: true for go, false for stop.

          This mechanism is used by visitors and components to signal special
          events. Specifically, TiffFinder sets the geTraverse flag as soon as
          it finds the correct component to signal to components that the search
          should be aborted. TiffReader uses geKnownMakernote to signal problems
          reading a makernote to the TiffMnEntry component. There is an array
          of flags, one for each defined \em event, so different signals can be
          used independent of each other.
         */
        void setGo(GoEvent event, bool go);
        //! Operation to perform for a TIFF entry
        virtual void visitEntry(TiffEntry* object) =0;
        //! Operation to perform for a TIFF data entry
        virtual void visitDataEntry(TiffDataEntry* object) =0;
        //! Operation to perform for a TIFF image entry
        virtual void visitImageEntry(TiffImageEntry* object) =0;
        //! Operation to perform for a TIFF size entry
        virtual void visitSizeEntry(TiffSizeEntry* object) =0;
        //! Operation to perform for a TIFF directory
        virtual void visitDirectory(TiffDirectory* object) =0;
        /*!
          @brief Operation to perform for a TIFF directory, after all components
                 and before the next entry is processed.
         */
        virtual void visitDirectoryNext(TiffDirectory* object);
        /*!
          @brief Operation to perform for a TIFF directory, at the end of the
                 processing.
         */
        virtual void visitDirectoryEnd(TiffDirectory* object);
        //! Operation to perform for a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object) =0;
        //! Operation to perform for the makernote component
        virtual void visitMnEntry(TiffMnEntry* object) =0;
        //! Operation to perform for an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object) =0;
        //! Operation to perform after processing an IFD makernote
        virtual void visitIfdMakernoteEnd(TiffIfdMakernote* object);
        //! Operation to perform for a binary array
        virtual void visitBinaryArray(TiffBinaryArray* object) =0;
        /*!
          @brief Operation to perform for a TIFF binary array, at the end
                 of the processing.
         */
        virtual void visitBinaryArrayEnd(TiffBinaryArray* object);
        //! Operation to perform for an element of a binary array
        virtual void visitBinaryElement(TiffBinaryElement* object) =0;
        //@}

        //! @name Accessors
        //@{
        //! Check if stop flag for \em event is clear, return true if it's clear.
        bool go(GoEvent event) const;
        //@}

    }; // class TiffVisitor

    /*!
      @brief Search the composite for a component with \em tag and \em group.
             Return a pointer to the component or 0, if not found. The class
             is ready for a first search after construction and can be
             re-initialized with init().
    */
    class TiffFinder : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        //! Constructor, taking \em tag and \em group of the component to find.
        TiffFinder(uint16_t tag, IfdId group)
            : tag_(tag), group_(group), tiffComponent_(0) {}
        //! Virtual destructor
        virtual ~TiffFinder();
        //@}

        //! @name Manipulators
        //@{
        //! Find tag and group in a TIFF entry
        virtual void visitEntry(TiffEntry* object);
        //! Find tag and group in a TIFF data entry
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Find tag and group in a TIFF image entry
        virtual void visitImageEntry(TiffImageEntry* object);
        //! Find tag and group in a TIFF size entry
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Find tag and group in a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Find tag and group in a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Find tag and group in a TIFF makernote
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Find tag and group in an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Find tag and group in a binary array
        virtual void visitBinaryArray(TiffBinaryArray* object);
        //! Find tag and group in an element of a binary array
        virtual void visitBinaryElement(TiffBinaryElement* object);

        //! Check if \em object matches \em tag and \em group
        void findObject(TiffComponent* object);
        //! Initialize the Finder for a new search.
        void init(uint16_t tag, IfdId group);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the search result. 0 if no TIFF component was found
                 for the tag and group combination.
         */
        TiffComponent* result() const { return tiffComponent_; }
        //@}

    private:
        uint16_t tag_;
        IfdId group_;
        TiffComponent* tiffComponent_;
    }; // class TiffFinder

    /*!
      @brief Copy all image tags from the source tree (the tree that is traversed) to a 
             target tree, which is empty except for the root element provided in the
             constructor.
    */
    class TiffCopier : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor

          @param pRoot Pointer to the root element of the (empty) target tree.
          @param root 
          @param pHeader Pointer to the TIFF header of the source image.
          @param pPrimaryGroups Pointer to the list of primary groups.
         */
        TiffCopier(      TiffComponent*  pRoot,
                         uint32_t        root,
                   const TiffHeaderBase* pHeader,
                   const PrimaryGroups*  pPrimaryGroups);
        //! Virtual destructor
        virtual ~TiffCopier();
        //@}

        //! @name Manipulators
        //@{
        //! Copy a TIFF entry if it is an image tag
        virtual void visitEntry(TiffEntry* object);
        //! Copy a TIFF data entry if it is an image tag
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Copy a TIFF image entry if it is an image tag
        virtual void visitImageEntry(TiffImageEntry* object);
        //! Copy a TIFF size entry if it is an image tag
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Copy a TIFF directory if it is an image tag
        virtual void visitDirectory(TiffDirectory* object);
        //! Copy a TIFF sub-IFD if it is an image tag
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Copy a TIFF makernote if it is an image tag
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Copy an IFD makernote if it is an image tag
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Copy a binary array if it is an image tag
        virtual void visitBinaryArray(TiffBinaryArray* object);
        //! Copy an element of a binary array if it is an image tag
        virtual void visitBinaryElement(TiffBinaryElement* object);

        //! Check if \em object is an image tag and if so, copy it to the target tree.
        void copyObject(TiffComponent* object);
        //@}

    private:
              TiffComponent*  pRoot_;
              uint32_t        root_;
        const TiffHeaderBase* pHeader_;
        const PrimaryGroups*  pPrimaryGroups_;
    }; // class TiffCopier

    /*!
      @brief TIFF composite visitor to decode metadata from the TIFF tree and
             add it to an Image, which is supplied in the constructor (Visitor
             pattern). Used by TiffParser to decode the metadata from a
             TIFF composite.
     */
    class TiffDecoder : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor, taking metadata containers to add the metadata to,
                 the root element of the composite to decode and a FindDecoderFct
                 function to get the decoder function for each tag.
         */
        TiffDecoder(
            ExifData&            exifData,
            IptcData&            iptcData,
            XmpData&             xmpData,
            TiffComponent* const pRoot,
            FindDecoderFct       findDecoderFct
        );
        //! Virtual destructor
        virtual ~TiffDecoder();
        //@}

        //! @name Manipulators
        //@{
        //! Decode a TIFF entry
        virtual void visitEntry(TiffEntry* object);
        //! Decode a TIFF data entry
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Decode a TIFF image entry
        virtual void visitImageEntry(TiffImageEntry* object);
        //! Decode a TIFF size entry
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Decode a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Decode a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Decode a TIFF makernote
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Decode an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Decode a binary array
        virtual void visitBinaryArray(TiffBinaryArray* object);
        //! Decode an element of a binary array
        virtual void visitBinaryElement(TiffBinaryElement* object);

        //! Entry function, determines how to decode each tag
        void decodeTiffEntry(const TiffEntryBase* object);
        //! Decode a standard TIFF entry
        void decodeStdTiffEntry(const TiffEntryBase* object);
        //! Decode IPTC data from an IPTCNAA tag or Photoshop ImageResources
        void decodeIptc(const TiffEntryBase* object);
        //! Decode XMP packet from an XMLPacket tag
        void decodeXmp(const TiffEntryBase* object);
        //@}

    private:
        //! @name Manipulators
        //@{
        /*!
          @brief Get the data for a \em tag and \em group, either from the
                 \em object provided, if it matches or from the matching element
                 in the hierarchy.

          Populates \em pData and \em size with the result. If no matching
          element is found the function leaves both of these parameters unchanged.
        */
        void getObjData(byte const*&         pData,
                        long&                size,
                        uint16_t             tag,
                        IfdId                group,
                        const TiffEntryBase* object);
        //@}

    private:
        // DATA
        ExifData& exifData_;         //!< Exif metadata container
        IptcData& iptcData_;         //!< IPTC metadata container
        XmpData&  xmpData_;          //!< XMP metadata container
        TiffComponent* const pRoot_; //!< Root element of the composite
        const FindDecoderFct findDecoderFct_; //!< Ptr to the function to find special decoding functions
        std::string make_;           //!< Camera make, determined from the tags to decode
        bool decodedIptc_;           //!< Indicates if IPTC has been decoded yet

    }; // class TiffDecoder

    /*!
      @brief TIFF composite visitor to encode metadata from an image to the TIFF
             tree. The metadata containers and root element of the tree are
             supplied in the constructor. Used by TiffParserWorker to encode the
             metadata into a TIFF composite.

             For non-intrusive writing, the encoder is used as a visitor (by
             passing it to the accept() member of a TiffComponent). The
             composite tree is then traversed and metadata from the image is
             used to encode each existing component.

             For intrusive writing, add() is called, which loops through the
             metadata and creates and populates corresponding TiffComponents
             as needed.
     */
    class TiffEncoder : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor, taking the root element of the composite to encode
                 to, the image with the metadata to encode and a function to
                 find special encoders.
         */
        TiffEncoder(
            const ExifData&      exifData,
            const IptcData&      iptcData,
            const XmpData&       xmpData,
                  TiffComponent* pRoot,
            const bool           isNewImage,
            const PrimaryGroups* pPrimaryGroups,
            const TiffHeaderBase* pHeader,
                  FindEncoderFct findEncoderFct
        );
        //! Virtual destructor
        virtual ~TiffEncoder();
        //@}

        //! @name Manipulators
        //@{
        //! Encode a TIFF entry
        virtual void visitEntry(TiffEntry* object);
        //! Encode a TIFF data entry
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Encode a TIFF image entry
        virtual void visitImageEntry(TiffImageEntry* object);
        //! Encode a TIFF size entry
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Encode a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Update directory entries
        virtual void visitDirectoryNext(TiffDirectory* object);
        //! Encode a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Encode a TIFF makernote
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Encode an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Reset encoder to its original state, undo makernote specific settings
        virtual void visitIfdMakernoteEnd(TiffIfdMakernote* object);
        //! Encode a binary array
        virtual void visitBinaryArray(TiffBinaryArray* object);
        //! Re-encrypt binary array if necessary
        virtual void visitBinaryArrayEnd(TiffBinaryArray* object);
        //! Encode an element of a binary array
        virtual void visitBinaryElement(TiffBinaryElement* object);

        /*!
          @brief Top level encoder function. Determines how to encode each TIFF
                 component. This function is called by the visit methods of the
                 encoder as well as the add() method.

          If no \em datum is provided, search the metadata based on tag and
          group of the \em object. This is the case if the function is called
          from a visit method.

          Then check if a special encoder function is registered for the tag,
          and if so use it to encode the \em object. Else use the callback
          encoder function at the object (which results in a double-dispatch to
          the appropriate encoding function of the encoder.

          @param object Object in the TIFF component tree to encode.
          @param datum  The corresponding metadatum with the updated value.

          @note Encoder functions may use metadata other than \em datum.
         */
        void encodeTiffComponent(
                  TiffEntryBase* object,
            const Exifdatum*     datum =0
        );

        //! Callback encoder function for an element of a binary array.
        void encodeBinaryElement(TiffBinaryElement* object, const Exifdatum* datum);
        //! Callback encoder function for a binary array.
        void encodeBinaryArray(TiffBinaryArray* object, const Exifdatum* datum);
        //! Callback encoder function for a data entry.
        void encodeDataEntry(TiffDataEntry* object, const Exifdatum* datum);
        //! Callback encoder function for a standard TIFF entry
        void encodeTiffEntry(TiffEntry* object, const Exifdatum* datum);
        //! Callback encoder function for an image entry.
        void encodeImageEntry(TiffImageEntry* object, const Exifdatum* datum);
        //! Callback encoder function for a %Makernote entry.
        void encodeMnEntry(TiffMnEntry* object, const Exifdatum* datum);
        //! Callback encoder function for a size entry.
        void encodeSizeEntry(TiffSizeEntry* object, const Exifdatum* datum);
        //! Callback encoder function for a sub-IFD entry.
        void encodeSubIfd(TiffSubIfd* object, const Exifdatum* datum);

        //! Special encoder function for the base part of a TIFF entry.
        void encodeTiffEntryBase(TiffEntryBase* object, const Exifdatum* datum);
        //! Special encoder function for an offset entry.
        void encodeOffsetEntry(TiffEntryBase* object, const Exifdatum* datum);

        //! Special encoder function to encode SubIFD contents to Image group if it contains primary image data
        // Todo void encodeNikonSubIfd(TiffEntryBase* object, const Exifdatum* datum);

        //! Special encoder function to encode IPTC data to an IPTCNAA or Photoshop ImageResources tag.
        void encodeIptc(TiffEntryBase* object, const Exifdatum* datum);
        /*!
          @brief Add metadata from image to the TIFF composite.

          For each Exif metadatum, the corresponding TiffComponent is created
          if necessary and populated using encodeTiffComponent(). The add() function
          is used during intrusive writing, to create a new TIFF structure.

          @note For non-intrusive writing, the encoder is used as a visitor (by
          passing it to the accept() member of a TiffComponent). The composite
          tree is then traversed and metadata from the image is used to encode
          each existing component.
        */
        void add(
            TiffComponent* pRootDir,
            TiffComponent* pSourceDir,
            uint32_t       root
        );
        //! Set the dirty flag and end of traversing signal.
        void setDirty(bool flag =true);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the applicable byte order. May be different for
                 the Makernote and the rest of the TIFF entries.
         */
        ByteOrder byteOrder() const { return byteOrder_; }
        /*!
          @brief True if any tag was deleted or allocated in the process of
                 visiting a TIFF composite tree.
         */
        bool dirty() const;
        //! Return the write method used.
        WriteMethod writeMethod() const { return writeMethod_; }
        //@}

    private:
        //! @name Manipulators
        //@{
        /*!
          Encode IPTC data. Updates or adds tag Exif.Image.IPTCNAA, updates but
          never adds tag Exif.Image.ImageResources.
          This method is called from the constructor.
         */
        void encodeIptc();
        /*!
          Encode XMP data. Adds tag Exif.Image.XMLPacket with the XMP packet.
          This method is called from the constructor.
         */
        void encodeXmp();
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Update a directory entry. This is called after all directory
                 entries are encoded. It takes care of type and count changes
                 and size shrinkage for non-intrusive writing.
         */
        uint32_t updateDirEntry(byte* buf,
                                ByteOrder byteOrder,
                                TiffComponent* pTiffComponent) const;
        /*!
          @brief Check if the tag is an image tag of an existing image. Such 
                 tags are copied from the original image and can't be modifed.

                 The condition is true if there is an existing image (as
                 opposed to a newly created TIFF image) and \em tag, \em group
                 is considered an image tag of this image - whether or not
                 it's actually present in the existing image doesn't matter.
         */
        bool isImageTag(uint16_t tag, IfdId group) const;
        //@}

    private:
        // DATA
        ExifData exifData_;          //!< Copy of the Exif data to encode
        const IptcData& iptcData_;   //!< IPTC data to encode, just a reference
        const XmpData&  xmpData_;    //!< XMP data to encode, just a reference
        bool del_;                   //!< Indicates if Exif data entries should be deleted after encoding
        const TiffHeaderBase* pHeader_; //!< TIFF image header
        TiffComponent* pRoot_;       //!< Root element of the composite
        const bool isNewImage_;      //!< True if the TIFF image is created from scratch
        const PrimaryGroups* pPrimaryGroups_; //!< List of primary image groups
        TiffComponent* pSourceTree_; //!< Parsed source tree for reference
        ByteOrder byteOrder_;        //!< Byteorder for encoding
        ByteOrder origByteOrder_;    //!< Byteorder as set in the c'tor
        const FindEncoderFct findEncoderFct_; //!< Ptr to the function to find special encoding functions
        std::string make_;           //!< Camera make, determined from the tags to encode
        bool dirty_;                 //!< Signals if any tag is deleted or allocated
        WriteMethod writeMethod_;    //!< Write method used.

    }; // class TiffEncoder

    /*!
      @brief Simple state class containing relevant state information for
             the TIFF reader. This is in a separate class so that the
             reader can change state if needed (e.g., to read certain complex
             makernotes).
     */
    class TiffRwState {
        friend class TiffReader;
    public:
        //! TiffRWState auto_ptr type
        typedef std::auto_ptr<TiffRwState> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor.
        TiffRwState(ByteOrder byteOrder,
                    uint32_t  baseOffset)
            : byteOrder_(byteOrder),
              baseOffset_(baseOffset) {}
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the applicable byte order. May be different for
                 the Makernote and the rest of the TIFF entries.
         */
        ByteOrder          byteOrder()  const { return byteOrder_; }
        /*!
          @brief Return the base offset.

          TIFF standard format uses byte offsets which are always relative to
          the start of the TIFF file, i.e., relative to the start of the TIFF
          image header. In this case, the base offset is 0.  However, some
          camera vendors encode their makernotes in TIFF IFDs using offsets
          relative to (somewhere near) the start of the makernote data. In this
          case, base offset added to the start of the TIFF image header points
          to the basis for such makernote offsets.
         */
        uint32_t           baseOffset() const { return baseOffset_; }
        //@}

    private:
        ByteOrder byteOrder_;
        const uint32_t baseOffset_;
    }; // TiffRwState

    /*!
      @brief TIFF composite visitor to read the TIFF structure from a block of
             memory and build the composite from it (Visitor pattern). Used by
             TiffParser to read the TIFF data from a block of memory.
     */
    class TiffReader : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. The data buffer and table describing the TIFF
                           structure of the data are set in the constructor.
          @param pData     Pointer to the data buffer, starting with a TIFF header.
          @param size      Number of bytes in the data buffer.
          @param pRoot     Root element of the TIFF composite.
          @param state     State object for creation function, byte order and
                           base offset.
         */
        TiffReader(const byte*          pData,
                   uint32_t             size,
                   TiffComponent*       pRoot,
                   TiffRwState::AutoPtr state);

        //! Virtual destructor
        virtual ~TiffReader();
        //@}

        //! @name Manipulators
        //@{
        //! Read a TIFF entry from the data buffer
        virtual void visitEntry(TiffEntry* object);
        //! Read a TIFF data entry from the data buffer
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Read a TIFF image entry from the data buffer
        virtual void visitImageEntry(TiffImageEntry* object);
        //! Read a TIFF size entry from the data buffer
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Read a TIFF directory from the data buffer
        virtual void visitDirectory(TiffDirectory* object);
        //! Read a TIFF sub-IFD from the data buffer
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Read a TIFF makernote entry from the data buffer
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Read an IFD makernote from the data buffer
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Reset reader to its original state, undo makernote specific settings
        virtual void visitIfdMakernoteEnd(TiffIfdMakernote* object);
        //! Read a binary array from the data buffer
        virtual void visitBinaryArray(TiffBinaryArray* object);
        //! Read an element of a binary array from the data buffer
        virtual void visitBinaryElement(TiffBinaryElement* object);

        //! Read a standard TIFF entry from the data buffer
        void readTiffEntry(TiffEntryBase* object);
        //! Read a TiffDataEntryBase from the data buffer
        void readDataEntryBase(TiffDataEntryBase* object);
        //! Set the \em state class. Assumes ownership of the object passed in.
        void changeState(TiffRwState::AutoPtr state);
        //! Reset the state to the original state as set in the constructor.
        void resetState();
        //! Check IFD directory pointer \em start for circular reference
        bool circularReference(const byte* start, IfdId group);
        //! Return the next idx sequence number for \em group
        int nextIdx(IfdId group);

        /*!
          @brief Read deferred components.

          This function is called after the TIFF composite is read by passing a
          TiffReader to the accept() function of the root component. It reads
          all components for which reading was deferred during that pass.  This
          is usually done to make sure that all other components are accessible
          at the time the deferred components are processed.
         */
        void postProcess();
        //@}

        //! @name Accessors
        //@{
        //! Return the byte order.
        ByteOrder byteOrder() const;
        //! Return the base offset. See class TiffRwState for details
        uint32_t baseOffset() const;
        //@}

    private:
        typedef std::map<const byte*, IfdId> DirList;
        typedef std::map<uint16_t, int> IdxSeq;
        typedef std::vector<TiffComponent*> PostList;

        // DATA
        const byte*          pData_;      //!< Pointer to the memory buffer
        const uint32_t       size_;       //!< Size of the buffer
        const byte*          pLast_;      //!< Pointer to the last byte
        TiffComponent* const pRoot_;      //!< Root element of the composite
        TiffRwState*         pState_;     //!< State class
        TiffRwState*         pOrigState_; //!< State class as set in the c'tor
        DirList              dirList_;    //!< List of IFD pointers and their groups
        IdxSeq               idxSeq_;     //!< Sequences for group, used for the entry's idx
        PostList             postList_;   //!< List of components with deferred reading
        bool                 postProc_;   //!< True in postProcessList()
    }; // class TiffReader

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef TIFFVISITOR_INT_HPP_
