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
  @file    matroskavideo.hpp
  @brief   An Image subclass to support Matroska video files
  @version $Rev$
  @author  Abhinav Badola for GSoC 2012
           <a href="mailto:mail.abu.to@gmail.com">mail.abu.to@gmail.com</a>
  @date    18-Jun-12, AB: created
 */
#ifndef MATROSKAVIDEO_HPP_
#define MATROSKAVIDEO_HPP_

// *****************************************************************************
// included header files
#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    // Add MKV to the supported image formats
    namespace ImageType {
        const int mkv = 21; //!< Treating mkv as an image type>
    }

    // Todo: Should be hidden
    /*!
      @brief Helper structure for the Matroska tags lookup table.
     */
    struct MatroskaTags {
        uint64_t val_;                          //!< Tag value
        const char* label_;                     //!< Translation of the tag value

        //! Comparison operator for use with the find template
       bool operator==(uint64_t key) const { return val_ == key; }
    }; // struct TagDetails

    /*!
      @brief Class to access Matroska video files.
     */
    class EXIV2API MatroskaVideo : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for a Matroska video. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
         */
        MatroskaVideo(BasicIo::AutoPtr io);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const;
        //@}

    protected:
        /*!
          @brief Function used to calulate the size of a block.
              This information is only stored in one byte.
              The size of the block is calculated by counting
              the number of leading zeros in the binary code of the byte.
              Size = (No. of leading zeros + 1) bytes
          @param b The byte, which stores the information to calculate the size
          @return Return the size of the block.
         */
        uint32_t findBlockSize(byte b);
        /*!
          @brief Check for a valid tag and decode the block at the current IO position.
              Calls contentManagement() or skips to next tag, if required.
         */
        void decodeBlock();
        /*!
          @brief Interpret tag information, and save it in the respective XMP container.
          @param mt Pointer to current tag,
          @param buf Pointer to the memory area with the tag information.
          @param size Size of \em buf.
         */
        void contentManagement(const MatroskaTags* mt, const byte* buf, long size);
        /*!
          @brief Calculates Aspect Ratio of a video, and stores it in the
              respective XMP container.
         */
        void aspectRatio();

    private:
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        MatroskaVideo(const MatroskaVideo& rhs);
        //! Assignment operator
        MatroskaVideo& operator=(const MatroskaVideo& rhs);
        //@}

    private:
        //! Variable to check the end of metadata traversing.
        bool continueTraversing_;
        //! Variable to store height and width of a video frame.
        uint64_t height_, width_;

    }; // class MatroskaVideo

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new MatroskaVideo instance and return an auto-pointer to it.
          Caller owns the returned object and the auto-pointer ensures that
          it will be deleted.
     */
    EXIV2API Image::AutoPtr newMkvInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a Matroska Video.
    EXIV2API bool isMkvType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef MATROSKAVIDEO_HPP_
