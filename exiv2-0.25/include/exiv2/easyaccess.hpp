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
  @file    easyaccess.hpp
  @brief   Provides easy (high-level) access to some Exif meta data.
  @version $Rev: 3091 $
  @author  Carsten Pfeiffer <pfeiffer@kde.org>
  @date    28-Feb-09, gis: created
 */
#ifndef EASYACCESS_HPP_
#define EASYACCESS_HPP_

// *****************************************************************************
// included header files
#include "exif.hpp"

namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;

    //! Return the orientation of the image
    EXIV2API ExifData::const_iterator orientation(const ExifData& ed);
    //! Return the ISO speed used to shoot the image
    EXIV2API ExifData::const_iterator isoSpeed(const ExifData& ed);
    //! Return the flash bias value
    EXIV2API ExifData::const_iterator flashBias(const ExifData& ed);
    //! Return the exposure mode setting
    EXIV2API ExifData::const_iterator exposureMode(const ExifData& ed);
    //! Return the scene mode setting
    EXIV2API ExifData::const_iterator sceneMode(const ExifData& ed);
    //! Return the macro mode setting
    EXIV2API ExifData::const_iterator macroMode(const ExifData& ed);
    //! Return the image quality setting
    EXIV2API ExifData::const_iterator imageQuality(const ExifData& ed);
    //! Return the white balance setting
    EXIV2API ExifData::const_iterator whiteBalance(const ExifData& ed);
    //! Return the name of the lens used
    EXIV2API ExifData::const_iterator lensName(const ExifData& ed);
    //! Return the saturation level
    EXIV2API ExifData::const_iterator saturation(const ExifData& ed);
    //! Return the sharpness level
    EXIV2API ExifData::const_iterator sharpness(const ExifData& ed);
    //! Return the contrast level
    EXIV2API ExifData::const_iterator contrast(const ExifData& ed);
    //! Return the scene capture type
    EXIV2API ExifData::const_iterator sceneCaptureType(const ExifData& ed);
    //! Return the metering mode setting
    EXIV2API ExifData::const_iterator meteringMode(const ExifData& ed);
    //! Return the camera make
    EXIV2API ExifData::const_iterator make(const ExifData& ed);
    //! Return the camera model
    EXIV2API ExifData::const_iterator model(const ExifData& ed);
    //! Return the exposure time
    EXIV2API ExifData::const_iterator exposureTime(const ExifData& ed);
    //! Return the F number
    EXIV2API ExifData::const_iterator fNumber(const ExifData& ed);
    //! Return the subject distance
    EXIV2API ExifData::const_iterator subjectDistance(const ExifData& ed);
    //! Return the camera serial number
    EXIV2API ExifData::const_iterator serialNumber(const ExifData& ed);
    //! Return the focal length setting
    EXIV2API ExifData::const_iterator focalLength(const ExifData& ed);
    //! Return the AF point
    EXIV2API ExifData::const_iterator afPoint(const ExifData& ed);

} // namespace Exiv2

#endif // EASYACCESS_HPP_
