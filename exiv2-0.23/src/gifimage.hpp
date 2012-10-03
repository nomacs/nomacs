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
  @file    gifimage.hpp
  @brief   GIF image, implemented using the following references:
           <a href="http://www.w3.org/Graphics/GIF/spec-gif89a.txt">GIF89 specification</a> by W3C<br>
  @version $Rev: 2681 $
  @author  Marco Piovanelli, Ovolab (marco)
           <a href="mailto:marco.piovanelli@pobox.com">marco.piovanelli@pobox.com</a>
  @date    26-Feb-2007, marco: created
 */
#ifndef GIFIMAGE_HPP_
#define GIFIMAGE_HPP_

// *****************************************************************************
// included header files
#include "exif.hpp"
#include "iptc.hpp"
#include "image.hpp"
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    // Add GIF to the supported image formats
    namespace ImageType {
        const int gif = 11;          //!< GIF image type (see class GifImage)
    }

    /*!
      @brief Class to access raw GIF images. Exif/IPTC metadata are supported
             directly.
     */
    class EXIV2API GifImage : public Image {
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        GifImage(const GifImage& rhs);
        //! Assignment operator
        GifImage& operator=(const GifImage& rhs);
        //@}

    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to open a GIF image. Since the
              constructor can not return a result, callers should check the
              good() method after object construction to determine success
              or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
         */
        GifImage(BasicIo::AutoPtr io);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        /*!
          @brief Todo: Write metadata back to the image. This method is not
              yet(?) implemented. Calling it will throw an Error(31).
         */
        void writeMetadata();
        /*!
          @brief Todo: Not supported yet(?). Calling this function will throw
              an instance of Error(32).
         */
        void setExifData(const ExifData& exifData);
        /*!
          @brief Todo: Not supported yet(?). Calling this function will throw
              an instance of Error(32).
         */
        void setIptcData(const IptcData& iptcData);
        /*!
          @brief Not supported. Calling this function will throw an instance
              of Error(32).
         */
        void setComment(const std::string& comment);
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const;
        //@}

    }; // class GifImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new GifImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newGifInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a GIF image.
    EXIV2API bool isGifType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef GIFIMAGE_HPP_
