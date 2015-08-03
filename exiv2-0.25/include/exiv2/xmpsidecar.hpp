// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
  @file    xmpsidecar.hpp
  @brief   An Image subclass to support XMP sidecar files
  @version $Rev: 3090 $
  @author  Andreas Huggel
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    07-Mar-08, ahu: created
 */
#ifndef XMPSIDECAR_HPP_
#define XMPSIDECAR_HPP_

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

    // Add XMP to the supported image formats
    namespace ImageType {
        const int xmp = 10;          //!< XMP sidecar files (see class XmpSidecar)
    }

    /*!
      @brief Class to access XMP sidecar files. They contain only XMP metadata.
     */
    class EXIV2API XmpSidecar : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for an XMP sidecar file. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new image should be created (true).
         */
        XmpSidecar(BasicIo::AutoPtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        /*!
          @brief Not supported. XMP sidecar files do not contain a comment.
              Calling this function will throw an instance of Error(32).
         */
        void setComment(const std::string& comment);
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const;
        //@}

    private:
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        XmpSidecar(const XmpSidecar& rhs);
        //! Assignment operator
        XmpSidecar& operator=(const XmpSidecar& rhs);
        //@}

    }; // class XmpSidecar

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new XmpSidecar instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newXmpInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is an XMP sidecar file.
    EXIV2API bool isXmpType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef XMPSIDECAR_HPP_
