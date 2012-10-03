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
  @file    jpgimage.hpp
  @brief   Class JpegImage to access JPEG images
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad)
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @author  Volker Grabsch (vog)
           <a href="mailto:vog@notjusthosting.com">vog@notjusthosting.com</a>
  @author  Michael Ulbrich (mul)
           <a href="mailto:mul@rentapacs.de">mul@rentapacs.de</a>
  @date    15-Jan-05, brad: split out from image.cpp
 */
#ifndef JPGIMAGE_HPP_
#define JPGIMAGE_HPP_

// *****************************************************************************
// included header files
#include "image.hpp"
#include "basicio.hpp"
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    // Supported JPEG image formats
    namespace ImageType {
        const int jpeg = 1;         //!< JPEG image type (see class JpegImage)
        const int exv  = 2;         //!< EXV image type (see class ExvImage)
    }

    /*!
      @brief Helper class, has methods to deal with %Photoshop "Information
             Resource Blocks" (IRBs).
     */
    struct EXIV2API Photoshop {
        // Todo: Public for now
        static const char     ps3Id_[]; //!< %Photoshop marker
        static const char*    irbId_[]; //!< %Photoshop IRB markers
        static const char     bimId_[]; //!< %Photoshop IRB marker (deprecated)
        static const uint16_t iptc_;    //!< %Photoshop IPTC marker
        static const uint16_t preview_; //!< %Photoshop preview marker

        /*!
          @brief Checks an IRB

          @param pPsData        Existing IRB buffer
          @param sizePsData     Size of the IRB buffer
          @return true  if the IRB marker is known and the buffer is big enough to check this;<BR>
                  false otherwise
        */
        static bool isIrb(const byte* pPsData,
                          long        sizePsData);
        /*!
          @brief Validates all IRBs

          @param pPsData        Existing IRB buffer
          @param sizePsData     Size of the IRB buffer, may be 0
          @return true  if all IRBs are valid;<BR>
                  false otherwise
        */
        static bool valid(const byte* pPsData,
                          long        sizePsData);
        /*!
          @brief Locates the data for a %Photoshop tag in a %Photoshop formated memory
              buffer. Operates on raw data to simplify reuse.
          @param pPsData Pointer to buffer containing entire payload of
              %Photoshop formated data, e.g., from APP13 Jpeg segment.
          @param sizePsData Size in bytes of pPsData.
          @param psTag %Tag number of the block to look for.
          @param record Output value that is set to the start of the
              data block within pPsData (may not be null).
          @param sizeHdr Output value that is set to the size of the header
              within the data block pointed to by record (may not be null).
          @param sizeData Output value that is set to the size of the actual
              data within the data block pointed to by record (may not be null).
          @return 0 if successful;<BR>
                  3 if no data for psTag was found in pPsData;<BR>
                 -2 if the pPsData buffer does not contain valid data.
        */
        static int locateIrb(const byte *pPsData,
                             long sizePsData,
                             uint16_t psTag,
                             const byte **record,
                             uint32_t *const sizeHdr,
                             uint32_t *const sizeData);
        /*!
          @brief Forwards to locateIrb() with \em psTag = \em iptc_
         */
        static int locateIptcIrb(const byte *pPsData,
                                 long sizePsData,
                                 const byte **record,
                                 uint32_t *const sizeHdr,
                                 uint32_t *const sizeData);
        /*!
          @brief Forwards to locatePreviewIrb() with \em psTag = \em preview_
         */
        static int locatePreviewIrb(const byte *pPsData,
                                    long sizePsData,
                                    const byte **record,
                                    uint32_t *const sizeHdr,
                                    uint32_t *const sizeData);
        /*!
          @brief Set the new IPTC IRB, keeps existing IRBs but removes the
                 IPTC block if there is no new IPTC data to write.

          @param pPsData    Existing IRB buffer
          @param sizePsData Size of the IRB buffer, may be 0
          @param iptcData   Iptc data to embed, may be empty
          @return A data buffer containing the new IRB buffer, may have 0 size
        */
        static DataBuf setIptcIrb(const byte*     pPsData,
                                  long            sizePsData,
                                  const IptcData& iptcData);

    }; // class Photoshop

    /*!
      @brief Abstract helper base class to access JPEG images.
     */
    class EXIV2API JpegBase : public Image {
    public:
        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        //@}

    protected:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing image or create
              a new image from scratch. If a new image is to be created, any
              existing data is overwritten.
          @param type Image type.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new image should be created (true).
          @param initData Data to initialize newly created images. Only used
              when \em create is true. Should contain data for the smallest
              valid image of the calling subclass.
          @param dataSize Size of initData in bytes.
         */
        JpegBase(int              type,
                 BasicIo::AutoPtr io,
                 bool             create,
                 const byte       initData[],
                 long             dataSize);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Determine if the content of the BasicIo instance is of the
              type supported by this class.

          The advance flag determines if the read position in the stream is
          moved (see below). This applies only if the type matches and the
          function returns true. If the type does not match, the stream
          position is not changed. However, if reading from the stream fails,
          the stream position is undefined. Consult the stream state to obtain
          more information in this case.

          @param iIo BasicIo instance to read from.
          @param advance Flag indicating whether the position of the io
              should be advanced by the number of characters read to
              analyse the data (true) or left at its original
              position (false). This applies only if the type matches.
          @return  true  if the data matches the type of this class;<BR>
                   false if the data does not match
         */
        virtual bool isThisType(BasicIo& iIo, bool advance) const =0;
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Writes the image header (aka signature) to the BasicIo instance.
          @param oIo BasicIo instance that the header is written to.
          @return 0 if successful;<BR>
                  4 if the output file can not be written to
         */
        virtual int writeHeader(BasicIo& oIo) const =0;
        //@}

        // Constant Data
        static const byte sos_;                 //!< JPEG SOS marker
        static const byte eoi_;                 //!< JPEG EOI marker
        static const byte app0_;                //!< JPEG APP0 marker
        static const byte app1_;                //!< JPEG APP1 marker
        static const byte app13_;               //!< JPEG APP13 marker
        static const byte com_;                 //!< JPEG Comment marker
        static const byte sof0_;                //!< JPEG Start-Of-Frame marker
        static const byte sof1_;                //!< JPEG Start-Of-Frame marker
        static const byte sof2_;                //!< JPEG Start-Of-Frame marker
        static const byte sof3_;                //!< JPEG Start-Of-Frame marker
        static const byte sof5_;                //!< JPEG Start-Of-Frame marker
        static const byte sof6_;                //!< JPEG Start-Of-Frame marker
        static const byte sof7_;                //!< JPEG Start-Of-Frame marker
        static const byte sof9_;                //!< JPEG Start-Of-Frame marker
        static const byte sof10_;               //!< JPEG Start-Of-Frame marker
        static const byte sof11_;               //!< JPEG Start-Of-Frame marker
        static const byte sof13_;               //!< JPEG Start-Of-Frame marker
        static const byte sof14_;               //!< JPEG Start-Of-Frame marker
        static const byte sof15_;               //!< JPEG Start-Of-Frame marker
        static const char exifId_[];            //!< Exif identifier
        static const char jfifId_[];            //!< JFIF identifier
        static const char xmpId_[];             //!< XMP packet identifier

    private:
        //! @name NOT implemented
        //@{
        //! Default constructor.
        JpegBase();
        //! Copy constructor
        JpegBase(const JpegBase& rhs);
        //! Assignment operator
        JpegBase& operator=(const JpegBase& rhs);
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Initialize the image with the provided data.
          @param initData Data to be written to the associated BasicIo
          @param dataSize Size in bytes of data to be written
          @return 0 if successful;<BR>
                  4 if the image can not be written to.
         */
        EXV_DLLLOCAL int initImage(const byte initData[], long dataSize);
        /*!
          @brief Provides the main implementation of writeMetadata() by
                writing all buffered metadata to the provided BasicIo.
          @param oIo BasicIo instance to write to (a temporary location).

          @return 4 if opening or writing to the associated BasicIo fails
         */
        EXV_DLLLOCAL void doWriteMetadata(BasicIo& oIo);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Advances associated io instance to one byte past the next
              Jpeg marker and returns the marker. This method should be called
              when the BasicIo instance is positioned one byte past the end of a
              Jpeg segment.
          @return the next Jpeg segment marker if successful;<BR>
                 -1 if a maker was not found before EOF
         */
        EXV_DLLLOCAL int advanceToMarker() const;
        //@}

    }; // class JpegBase

    /*!
      @brief Class to access JPEG images
     */
    class EXIV2API JpegImage : public JpegBase {
        friend EXIV2API bool isJpegType(BasicIo& iIo, bool advance);
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing Jpeg image or create
              a new image from scratch. If a new image is to be created, any
              existing data is overwritten. Since the constructor can not return
              a result, callers should check the good() method after object
              construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new file should be created (true).
         */
        JpegImage(BasicIo::AutoPtr io, bool create);
        //@}
        //! @name Accessors
        //@{
        std::string mimeType() const;
        //@}

    protected:
        //! @name Accessors
        //@{
        bool isThisType(BasicIo& iIo, bool advance) const;
        //@}
        //! @name Manipulators
        //@{
        /*!
          @brief Writes a Jpeg header (aka signature) to the BasicIo instance.
          @param oIo BasicIo instance that the header is written to.
          @return 0 if successful;<BR>
                 2 if the input image is invalid or can not be read;<BR>
                 4 if the temporary image can not be written to;<BR>
                -3 other temporary errors
         */
        int writeHeader(BasicIo& oIo) const;
        //@}

    private:
        // Constant data
        static const byte soi_;          // SOI marker
        static const byte blank_[];      // Minimal Jpeg image

        // NOT Implemented
        //! Default constructor
        JpegImage();
        //! Copy constructor
        JpegImage(const JpegImage& rhs);
        //! Assignment operator
        JpegImage& operator=(const JpegImage& rhs);

    }; // class JpegImage

    //! Helper class to access %Exiv2 files
    class EXIV2API ExvImage : public JpegBase {
        friend EXIV2API bool isExvType(BasicIo& iIo, bool advance);
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing EXV image or create
              a new image from scratch. If a new image is to be created, any
              existing data is overwritten. Since the constructor can not return
              a result, callers should check the good() method after object
              construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
                 or if a new file should be created (true).
         */
        ExvImage(BasicIo::AutoPtr io, bool create);
        //@}
        //! @name Accessors
        //@{
        std::string mimeType() const;
        //@}

    protected:
        //! @name Accessors
        //@{
        bool isThisType(BasicIo& iIo, bool advance) const;
        //@}
        //! @name Manipulators
        //@{
        int writeHeader(BasicIo& oIo) const;
        //@}

    private:
        // Constant data
        static const char exiv2Id_[];    // EXV identifier
        static const byte blank_[];      // Minimal exiv2 file

        // NOT Implemented
        //! Default constructor
        ExvImage();
        //! Copy constructor
        ExvImage(const ExvImage& rhs);
        //! Assignment operator
        ExvImage& operator=(const ExvImage& rhs);

    }; // class ExvImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new JpegImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newJpegInstance(BasicIo::AutoPtr io, bool create);
    //! Check if the file iIo is a JPEG image.
    EXIV2API bool isJpegType(BasicIo& iIo, bool advance);
    /*!
      @brief Create a new ExvImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newExvInstance(BasicIo::AutoPtr io, bool create);
    //! Check if the file iIo is an EXV file
    EXIV2API bool isExvType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef JPGIMAGE_HPP_
