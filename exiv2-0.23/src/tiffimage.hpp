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
  @file    tiffimage.hpp
  @brief   Class TiffImage
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Mar-06, ahu: created
 */
#ifndef TIFFIMAGE_HPP_
#define TIFFIMAGE_HPP_

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

    // Add TIFF to the supported image formats
    namespace ImageType {
        const int tiff = 4;          //!< TIFF image type (see class TiffImage)
        const int dng = 4;           //!< DNG image type (see class TiffImage)
        const int nef = 4;           //!< NEF image type (see class TiffImage)
        const int pef = 4;           //!< PEF image type (see class TiffImage)
        const int arw = 4;           //!< ARW image type (see class TiffImage)
        const int sr2 = 4;           //!< SR2 image type (see class TiffImage)
        const int srw = 4;           //!< SRW image type (see class TiffImage)
    }

    /*!
      @brief Class to access TIFF images. Exif metadata is
          supported directly, IPTC is read from the Exif data, if present.
     */
    class EXIV2API TiffImage : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing TIFF image or create
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
        TiffImage(BasicIo::AutoPtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        /*!
          @brief Not supported. TIFF format does not contain a comment.
              Calling this function will throw an Error(32).
         */
        void setComment(const std::string& comment);
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
        TiffImage(const TiffImage& rhs);
        //! Assignment operator
        TiffImage& operator=(const TiffImage& rhs);
        //@}

        //! @name Accessors
        //@{
        //! Return the group name of the group with the primary image.
        std::string primaryGroup() const;
        //@}

    private:
        // DATA
        mutable std::string primaryGroup_;     //!< The primary group
        mutable std::string mimeType_;         //!< The MIME type
        mutable int pixelWidth_;               //!< Width of the primary image in pixels 
        mutable int pixelHeight_;              //!< Height of the primary image in pixels 

    }; // class TiffImage

    /*!
      @brief Stateless parser class for data in TIFF format. Images use this
             class to decode and encode TIFF data. It is a wrapper of the
             internal class Internal::TiffParserWorker.
     */
    class EXIV2API TiffParser {
    public:
        /*!
          @brief Decode metadata from a buffer \em pData of length \em size
                 with data in TIFF format to the provided metadata containers.

          @param exifData Exif metadata container.
          @param iptcData IPTC metadata container.
          @param xmpData  XMP metadata container.
          @param pData    Pointer to the data buffer. Must point to data in TIFF
                          format; no checks are performed.
          @param size     Length of the data buffer.

          @return Byte order in which the data is encoded.
        */
        static ByteOrder decode(
                  ExifData& exifData,
                  IptcData& iptcData,
                  XmpData&  xmpData,
            const byte*     pData,
                  uint32_t  size
        );
        /*!
          @brief Encode metadata from the provided metadata to TIFF format.

          The original binary image in the memory block \em pData, \em size is
          parsed and updated in-place if possible ("non-intrusive" writing).
          If that is not possible (e.g., if new tags were added), the entire
          TIFF structure is re-written to the \em io instance ("intrusive" writing).<br>
          The return value indicates which write method was used. If it is
          \c wmNonIntrusive, the original memory \em pData, \em size contains
          the result and nothing is written to \em io. If the return value is
          \c wmIntrusive, a new TIFF structure was created and written to
          \em io. The memory block \em pData, \em size may be partly updated
          in this case and should not be used anymore.

          @note If there is no metadata to encode, i.e., all metadata
                containers are empty, then the return value is \c wmIntrusive
                and nothing is written to \em io, i.e., no TIFF header is written.

          @param io        IO instance to write the binary image to in case of
                           "intrusive" writing. Nothing is written to \em io in
                           the case of "non-intrusive" writing.
          @param pData     Pointer to the binary image data buffer. Must
                           point to data in TIFF format; no checks are
                           performed. Will be modified if "non-intrusive"
                           writing is possible.
          @param size      Length of the data buffer.
          @param byteOrder Byte order to use.
          @param exifData  Exif metadata container.
          @param iptcData  IPTC metadata container.
          @param xmpData   XMP metadata container.

          @return Write method used.
        */
        static WriteMethod encode(
                  BasicIo&  io,
            const byte*     pData,
                  uint32_t  size,
                  ByteOrder byteOrder,
            const ExifData& exifData,
            const IptcData& iptcData,
            const XmpData&  xmpData
        );

    }; // class TiffParser

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new TiffImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newTiffInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a TIFF image.
    EXIV2API bool isTiffType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFIMAGE_HPP_
