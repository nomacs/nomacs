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
  @file    quicktimevideo.hpp
  @brief   An Image subclass to support Quick Time video files
  @version $Rev$
  @author  Abhinav Badola for GSoC 2012
           <a href="mailto:mail.abu.to@gmail.com">mail.abu.to@gmail.com</a>
  @date    28-Jun-12, AB: created
 */
#ifndef QUICKTIMEVIDEO_HPP
#define QUICKTIMEVIDEO_HPP

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

    // Add qtime to the supported image formats
    namespace ImageType {
        const int qtime = 22; //!< Treating qtime as an image type>
    }

    /*!
      @brief Class to access QuickTime video files.
     */
    class EXIV2API QuickTimeVideo:public Image
    {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for a QuickTime video. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
         */
        QuickTimeVideo(BasicIo::AutoPtr io);
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
        void tagDecoder(Exiv2::DataBuf & buf, unsigned long size);

    private:
        /*!
          @brief Interpret file type of the video, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void fileTypeDecoder(unsigned long size);
        /*!
          @brief Interpret Media Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void mediaHeaderDecoder(unsigned long size);
        /*!
          @brief Interpret Video Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void videoHeaderDecoder(unsigned long size);
        /*!
          @brief Interpret Movie Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void movieHeaderDecoder(unsigned long size);
        /*!
          @brief Interpret Track Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void trackHeaderDecoder(unsigned long size);
        /*!
          @brief Interpret Handler Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void handlerDecoder(unsigned long size);
        /*!
          @brief Interpret Tag which contain other sub-tags,
              and save it in the respective XMP container.
         */
        void multipleEntriesDecoder();
        /*!
          @brief Interpret Sample Description Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void sampleDesc(unsigned long size);
        /*!
          @brief Interpret Image Description Tag, and save it
              in the respective XMP container.
         */
        void imageDescDecoder();
        /*!
          @brief Interpret User Data Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void userDataDecoder(unsigned long size);
        /*!
          @brief Interpret Preview Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void previewTagDecoder(unsigned long size);
        /*!
          @brief Interpret Meta Keys Tags, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void keysTagDecoder(unsigned long size);
        /*!
          @brief Interpret Track Aperture Tags, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void trackApertureTagDecoder(unsigned long size);
        /*!
          @brief Interpret Nikon Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void NikonTagsDecoder(unsigned long size);
        /*!
          @brief Interpret Tags from Different Camera make, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
        void CameraTagsDecoder(unsigned long size);
        /*!
          @brief Interpret Audio Description Tag, and save it
              in the respective XMP container.
         */
        void audioDescDecoder();
        /*!
          @brief Helps to calculate Frame Rate from timeToSample chunk,
              and save it in the respective XMP container.
         */
        void timeToSampleDecoder();
        /*!
          @brief Recognizes which stream is currently under processing,
              and save its information in currentStream_ .
         */
        void setMediaStream();
        /*!
          @brief Used to discard a tag along with its data. The Tag will
              be skipped and not decoded.
          @param size Size of the data block that is to skipped.
         */
        void discard(unsigned long size);
        /*!
          @brief Calculates Aspect Ratio of a video, and stores it in the
              respective XMP container.
         */
        void aspectRatio();

    private:
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        QuickTimeVideo(const QuickTimeVideo& rhs);
        //! Assignment operator
        QuickTimeVideo& operator=(const QuickTimeVideo& rhs);
        //@}

    private:
        //! Variable which stores Time Scale unit, used to calculate time.
        uint64_t timeScale_;
        //! Variable which stores current stream being processsed.
        int currentStream_;
        //! Variable to check the end of metadata traversing.
        bool continueTraversing_;
        //! Variable to store height and width of a video frame.
        uint64_t height_, width_;

    }; //QuickTimeVideo End

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new QuicktimeVideo instance and return an auto-pointer to it.
          Caller owns the returned object and the auto-pointer ensures that
          it will be deleted.
     */
    EXIV2API Image::AutoPtr newQTimeInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a Quick Time Video.
    EXIV2API bool isQTimeType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // QUICKTIMEVIDEO_HPP
