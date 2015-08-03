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
  @file    riffvideo.hpp
  @brief   An Image subclass to support RIFF video files
  @version $Rev$
  @author  Abhinav Badola for GSoC 2012
           <a href="mailto:mail.abu.to@gmail.com">mail.abu.to@gmail.com</a>
  @date    18-Jun-12, AB: created
 */
#ifndef RIFFVIDEO_HPP
#define RIFFVIDEO_HPP

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

    // Add RIFF to the supported image formats
    namespace ImageType {
        const int riff = 20; //!< Treating riff as an image type>
    }

    /*!
      @brief Class to access RIFF video files.
     */
    class EXIV2API RiffVideo:public Image
    {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for a Riff video. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
         */
        RiffVideo(BasicIo::AutoPtr io);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const;
        const char* printAudioEncoding(uint64_t i);
        //@}

    protected:
        /*!
          @brief Check for a valid tag and decode the block at the current IO
          position. Calls tagDecoder() or skips to next tag, if required.
         */
        void decodeBlock();
        /*!
          @brief Interpret tag information, and call the respective function
              to save it in the respective XMP container. Decodes a Tag
              Information and saves it in the respective XMP container, if
              the block size is small.
          @param buf Data buffer which cotains tag ID.
          @param size Size of the data block used to store Tag Information.
         */
        void tagDecoder(Exiv2::DataBuf& buf, unsigned long size);
        /*!
          @brief Interpret Junk tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void junkHandler(long size);
        /*!
          @brief Interpret Stream tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void streamHandler(long size);
        /*!
          @brief Interpret Stream Format tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void streamFormatHandler(long size);
        /*!
          @brief Interpret Riff Header tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void aviHeaderTagsHandler(long size);
        /*!
          @brief Interpret Riff List tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void listHandler(long size);
        /*!
          @brief Interpret Riff Stream Data tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void streamDataTagHandler(long size);
        /*!
          @brief Interpret INFO tag information, and save it
              in the respective XMP container.
         */
        void infoTagsHandler();
        /*!
          @brief Interpret Nikon Tags related to Video information, and
              save it in the respective XMP container.
         */
        void nikonTagsHandler();
        /*!
          @brief Interpret OpenDML tag information, and save it
              in the respective XMP container.
         */
        void odmlTagsHandler();
        //! @brief Skips Particular Blocks of Metadata List.
        void skipListData();
        /*!
          @brief Interprets DateTimeOriginal tag or stream name tag
              information, and save it in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
          @param i parameter used to overload function
         */
        void dateTimeOriginal(long size, int i = 0);
        /*!
          @brief Calculates Sample Rate of a particular stream.
          @param buf Data buffer with the dividend.
          @param divisor The Divisor required to calculate sample rate.
          @return Return the sample rate of the stream.
         */
        double returnSampleRate(Exiv2::DataBuf& buf, long divisor = 1);
        /*!
          @brief Calculates Aspect Ratio of a video, and stores it in the
              respective XMP container.
          @param width Width of the video.
          @param height Height of the video.
         */
        void fillAspectRatio(long width = 1,long height = 1);
        /*!
          @brief Calculates Duration of a video, and stores it in the
              respective XMP container.
          @param frame_rate Frame rate of the video.
          @param frame_count Total number of frames present in the video.
         */
        void fillDuration(double frame_rate, long frame_count);

    private:
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        RiffVideo(const RiffVideo& rhs);
        //! Assignment operator
        RiffVideo& operator=(const RiffVideo& rhs);
        //@}

    private:
        //! Variable to check the end of metadata traversing.
        bool continueTraversing_;
        //! Variable which stores current stream being processsed.
        int streamType_;

    }; //Class RiffVideo

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new RiffVideo instance and return an auto-pointer to it.
          Caller owns the returned object and the auto-pointer ensures that
          it will be deleted.
     */
    EXIV2API Image::AutoPtr newRiffInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a Riff Video.
    EXIV2API bool isRiffType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // RIFFVIDEO_HPP
