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
  @file    epsimage.hpp
  @brief   EPS image.
           <br>References:
           <br>[1] <a href="http://partners.adobe.com/public/developer/en/ps/5001.DSC_Spec.pdf">Adobe PostScript Language Document Structuring Conventions Specification, Version 3.0</a>, September 1992
           <br>[2] <a href="http://partners.adobe.com/public/developer/en/ps/5002.EPSF_Spec.pdf">Adobe Encapsulated PostScript File Format Specification, Version 3.0</a>, May 1992
           <br>[3] <a href="http://www.adobe.com/content/dam/Adobe/en/devnet/xmp/pdfs/XMPSpecificationPart3.pdf">Adobe XMP Specification Part 3: Storage in Files</a>, July 2010
           <br>[4] <a href="http://groups.google.com/group/adobe.illustrator.windows/msg/0a9d7b1244b59062">Re: Thumbnail data format in ai file</a>, Dec 2003
  @version $Rev: $
  @author  Michael Ulbrich (mul)
           <a href="mailto:mul@rentapacs.de">mul@rentapacs.de</a>
  @author  Volker Grabsch (vog)
           <a href="mailto:vog@notjusthosting.com">vog@notjusthosting.com</a>
  @date    7-Mar-2011, vog: created
 */
#ifndef EPSIMAGE_HPP_
#define EPSIMAGE_HPP_

// *****************************************************************************
// included header files
#include "image.hpp"
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2
{

// *****************************************************************************
// class definitions

    // Add EPS to the supported image formats
    namespace ImageType {
        const int eps = 18;                     //!< EPS image type
    }

    /*!
      @brief Class to access EPS images.
     */
    class EXIV2API EpsImage : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to open a EPS image. Since the
              constructor can't return a result, callers should check the
              good() method after object construction to determine success
              or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new file should be created (true).
         */
        EpsImage(BasicIo::AutoPtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        /*!
          @brief Not supported.
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
        EpsImage(const EpsImage& rhs);
        //! Assignment operator
        EpsImage& operator=(const EpsImage& rhs);
        //@}

    }; // class EpsImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new EpsImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newEpsInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a EPS image.
    EXIV2API bool isEpsType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef EPSIMAGE_HPP_
