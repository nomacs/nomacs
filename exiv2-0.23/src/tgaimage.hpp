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
  @file    tgaimage.hpp
  @brief   Truevision TARGA v2 image, implemented using the following references:
           <a href="http://en.wikipedia.org/wiki/Truevision_TGA">Truevision TGA page on Wikipedia</a><br>
           <a href="http://www.gamers.org/dEngine/quake3/TGA.ps.gz">TGA(tm) File Format Specification</a>
  @version $Rev: 2681 $
  @author  Marco Piovanelli, Ovolab (marco)
           <a href="mailto:marco.piovanelli@pobox.com">marco.piovanelli@pobox.com</a>
  @date    05-Mar-2007, marco: created
 */
#ifndef TGAIMAGE_HPP_
#define TGAIMAGE_HPP_

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

    // Add TARGA to the supported image formats
    namespace ImageType {
        const int tga = 13; //!< Truevision TARGA (tga) image type (see class TgaImage)
    }

    /*!
      @brief Class to access raw TARGA images. This is just a stub - we only
          read width and height.
     */
    class EXIV2API TgaImage : public Image {
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        TgaImage(const TgaImage& rhs);
        //! Assignment operator
        TgaImage& operator=(const TgaImage& rhs);
        //@}

    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to open a Targa image. Since the
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
        TgaImage(BasicIo::AutoPtr io);
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

    }; // class TgaImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new TgaImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newTgaInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a Targa v2 image.
    EXIV2API bool isTgaType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef TGAIMAGE_HPP_
