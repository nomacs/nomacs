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
  @file    minoltamn_int.hpp
  @brief   Minolta MakerNote implemented using the following references:<br>
           <a href="http://www.dalibor.cz/minolta/makernote.htm">Minolta Makernote Format Specification</a> by Dalibor Jelinek,<br>
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Minolta.html">Minolta Makernote list</a> by Phil Harvey<br>
           <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/minolta_mn.html">Minolta Makernote list from PHP JPEG Metadata Toolkit</a><br>
           Email communication with <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a><br>
           Some Minolta camera settings have been decoded by <a href="mailto:xraynaud@gmail.com">Xavier Raynaud</a> from digiKam project and added by Gilles Caulier.
  @version $Rev: 2701 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Gilles Caulier (cgilles)
           <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
  @date    06-May-06, gc: submitted
 */
#ifndef MINOLTAMN_INT_HPP_
#define MINOLTAMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// + standard includes
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Minolta cameras
    class MinoltaMakerNote {
    public:
        //! Return read-only list of built-in Minolta tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in Minolta Standard Camera Settings tags
        static const TagInfo* tagListCsStd();
        //! Return read-only list of built-in Minolta 7D Camera Settings tags
        static const TagInfo* tagListCs7D();
        //! Return read-only list of built-in Minolta 5D Camera Settings tags
        static const TagInfo* tagListCs5D();
        //! Return read-only list of built-in Sony A100 Camera Settings tags
        static const TagInfo* tagListCsA100();

        //! @name Print functions for Minolta %MakerNote tags
        //@{
        //! Print Exposure Speed setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureSpeedStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print Exposure Time setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureTimeStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print F Number setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaFNumberStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print Exposure Compensation setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureCompensationStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print Focal Length setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaFocalLengthStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print Minolta Date from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaDateStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print Minolta Time from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaTimeStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print Flash Exposure Compensation setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaFlashExposureCompStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print White Balance setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaWhiteBalanceStd(std::ostream& os, const Value& value, const ExifData*);
        //! Print Brightness setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaBrightnessStd(std::ostream& os, const Value& value, const ExifData*);

        //! Print Exposure Manual Bias setting from 5D Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureManualBias5D(std::ostream& os, const Value& value, const ExifData*);
        //! Print Exposure Compensation setting from 5D Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureCompensation5D(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Tag information
        static const TagInfo tagInfo_[];
        static const TagInfo tagInfoCsA100_[];
        static const TagInfo tagInfoCs5D_[];
        static const TagInfo tagInfoCs7D_[];
        static const TagInfo tagInfoCsStd_[];

    }; // class MinoltaMakerNote

    // -- Minolta and Sony MakerNote Common Values ---------------------------------------

    //! Print Minolta/Sony Lens id values to readable labels.
    std::ostream& printMinoltaSonyLensID(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Color Mode values to readable labels.
    std::ostream& printMinoltaSonyColorMode(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony bool function values to readable labels.
    std::ostream& printMinoltaSonyBoolValue(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony bool inverse function values to readable labels.
    std::ostream& printMinoltaSonyBoolInverseValue(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony AF Area Mode values to readable labels.
    std::ostream& printMinoltaSonyAFAreaMode(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Local AF Area Point values to readable labels.
    std::ostream& printMinoltaSonyLocalAFAreaPoint(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony dynamic range optimizer mode values to readable labels.
    std::ostream& printMinoltaSonyDynamicRangeOptimizerMode(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony priority setup shutter release values to readable labels.
    std::ostream& printMinoltaSonyPrioritySetupShutterRelease(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Quality values to readable labels.
    std::ostream& printMinoltaSonyQualityCs(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Rotation values to readable labels.
    std::ostream& printMinoltaSonyRotation(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Scene Mode values to readable labels.
    std::ostream& printMinoltaSonySceneMode(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Image Quality values to readable labels.
    std::ostream& printMinoltaSonyImageQuality(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Teleconverter Model values to readable labels.
    std::ostream& printMinoltaSonyTeleconverterModel(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony White Balance Std values to readable labels.
    std::ostream& printMinoltaSonyWhiteBalanceStd(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony ZoneMatching values to readable labels.
    std::ostream& printMinoltaSonyZoneMatching(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony FlashExposureComp values to readable labels.
    std::ostream& printMinoltaSonyFlashExposureComp(std::ostream&, const Value&, const ExifData*);

    // TODO: Added shared methods here.

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef MINOLTAMN_INT_HPP_
