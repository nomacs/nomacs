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
  @file    crwimage.hpp
  @brief   Class CrwImage to access Canon CRW images.<BR>
           References:<BR>
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/canon_raw.html">The Canon RAW (CRW) File Format</a> by Phil Harvey
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    28-Aug-05, ahu: created
 */
#ifndef CRWIMAGE_HPP_
#define CRWIMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "image.hpp"
#include "basicio.hpp"

// + standard includes
#include <iosfwd>
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;
    class IptcData;

// *****************************************************************************
// class definitions

    // Add CRW to the supported image formats
    namespace ImageType {
        const int crw = 3;          //!< CRW image type (see class CrwImage)
    }

    /*!
      @brief Class to access raw Canon CRW images. Only Exif metadata and a
             comment are supported. CRW format does not contain IPTC metadata.
     */
    class EXIV2API CrwImage : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing CRW image or create
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
        CrwImage(BasicIo::AutoPtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        /*!
          @brief Not supported. CRW format does not contain IPTC metadata.
              Calling this function will throw an Error(32).
         */
        void setIptcData(const IptcData& iptcData);
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const;
        int pixelWidth() const;
        int pixelHeight() const;
        //@}

    private:
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        CrwImage(const CrwImage& rhs);
        //! Assignment operator
        CrwImage& operator=(const CrwImage& rhs);
        //@}

    }; // class CrwImage

    /*!
      Stateless parser class for Canon CRW images (Ciff format).
    */
    class EXIV2API CrwParser {
    public:
        /*!
          @brief Decode metadata from a Canon CRW image in data buffer \em pData
                 of length \em size into \em crwImage.

          This is the entry point to access image data in Ciff format. The
          parser uses classes CiffHeader, CiffEntry, CiffDirectory.

          @param pCrwImage Pointer to the %Exiv2 CRW image to hold the metadata
                           read from the buffer.
          @param pData     Pointer to the data buffer. Must point to the data of
                           a CRW image; no checks are performed.
          @param size      Length of the data buffer.

          @throw Error If the data buffer cannot be parsed.
        */
        static void decode(CrwImage* pCrwImage, const byte* pData, uint32_t size);
        /*!
          @brief Encode metadata from the CRW image into a data buffer (the
                 binary CRW image).

          @param blob      Data buffer for the binary image (target).
          @param pData     Pointer to the binary image data buffer. Must
                           point to data in CRW format; no checks are
                           performed.
          @param size      Length of the data buffer.
          @param pCrwImage Pointer to the %Exiv2 CRW image with the metadata to
                           encode.

          @throw Error If the metadata from the CRW image cannot be encoded.
         */
        static void encode(
                  Blob&     blob,
            const byte*     pData,
                  uint32_t  size,
            const CrwImage* pCrwImage
        );

    }; // class CrwParser

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new CrwImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newCrwInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a CRW image.
    EXIV2API bool isCrwType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef CRWIMAGE_HPP_
