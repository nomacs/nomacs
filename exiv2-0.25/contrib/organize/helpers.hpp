// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2009 Brad Schick <schickb@gmail.com>
 *
 * This file is part of the organize tool.
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
/*
 File:      helpers.hpp
 Version:   $Rev: 1762 $
 Author(s): Brad Schick (brad) <schickb@gmail.com>
 History:   19-Jan-09, brad: created
*/
// *****************************************************************************


#ifndef HELPERS_HPP_
#define HELPERS_HPP_

#include <boost/filesystem.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED
namespace fs = boost::filesystem;


typedef std::string (*pfunc)(const Exiv2::Image *image, const fs::path &path);

// This would be a lot smaller if Exiv2 had support 
// for unified metadata

std::string exif_date(const Exiv2::Image *image, const fs::path &path);
std::string exif_year(const Exiv2::Image *image, const fs::path &path);
std::string exif_month(const Exiv2::Image *image, const fs::path &path);
std::string exif_day(const Exiv2::Image *image, const fs::path &path);
std::string iptc_date(const Exiv2::Image *image, const fs::path &path);
std::string iptc_year(const Exiv2::Image *image, const fs::path &path);
std::string iptc_month(const Exiv2::Image *image, const fs::path &path);
std::string iptc_day(const Exiv2::Image *image, const fs::path &path);
std::string file_date(const Exiv2::Image *image, const fs::path &path);
std::string file_year(const Exiv2::Image *image, const fs::path &path);
std::string file_month(const Exiv2::Image *image, const fs::path &path);
std::string file_day(const Exiv2::Image *image, const fs::path &path);
/*std::string xmp_date(const Exiv2::Image *image, const fs::path &path);
std::string xmp_year(const Exiv2::Image *image, const fs::path &path);
std::string xmp_month(const Exiv2::Image *image, const fs::path &path);
std::string xmp_day(const Exiv2::Image *image, const fs::path &path);*/
std::string exif_time(const Exiv2::Image *image, const fs::path &path);
std::string exif_hour(const Exiv2::Image *image, const fs::path &path);
std::string exif_minute(const Exiv2::Image *image, const fs::path &path);
std::string exif_second(const Exiv2::Image *image, const fs::path &path);
std::string iptc_time(const Exiv2::Image *image, const fs::path &path);
std::string iptc_hour(const Exiv2::Image *image, const fs::path &path);
std::string iptc_minute(const Exiv2::Image *image, const fs::path &path);
std::string iptc_second(const Exiv2::Image *image, const fs::path &path);
std::string file_time(const Exiv2::Image *image, const fs::path &path);
std::string file_hour(const Exiv2::Image *image, const fs::path &path);
std::string file_minute(const Exiv2::Image *image, const fs::path &path);
std::string file_second(const Exiv2::Image *image, const fs::path &path);
/*std::string xmp_time(const Exiv2::Image *image, const fs::path &path);
std::string xmp_hour(const Exiv2::Image *image, const fs::path &path);
std::string xmp_minute(const Exiv2::Image *image, const fs::path &path);
std::string xmp_second(const Exiv2::Image *image, const fs::path &path);*/
std::string exif_dimension(const Exiv2::Image *image, const fs::path &path);
std::string exif_width(const Exiv2::Image *image, const fs::path &path);
std::string exif_height(const Exiv2::Image *image, const fs::path &path);
std::string file_dimension(const Exiv2::Image *image, const fs::path &path);
std::string file_width(const Exiv2::Image *image, const fs::path &path);
std::string file_height(const Exiv2::Image *image, const fs::path &path);
/*std::string xmp_dimension(const Exiv2::Image *image, const fs::path &path);
std::string xmp_width(const Exiv2::Image *image, const fs::path &path);
std::string xmp_height(const Exiv2::Image *image, const fs::path &path);*/
std::string exif_model(const Exiv2::Image *image, const fs::path &path);
std::string exif_make(const Exiv2::Image *image, const fs::path &path);
/*std::string xmp_model(const Exiv2::Image *image, const fs::path &path);
std::string xmp_make(const Exiv2::Image *image, const fs::path &path);*/
std::string exif_speed(const Exiv2::Image *image, const fs::path &path);
//std::string xmp_speed(const Exiv2::Image *image, const fs::path &path);
std::string exif_aperture(const Exiv2::Image *image, const fs::path &path);
//std::string xmp_aperture(const Exiv2::Image *image, const fs::path &path);
std::string exif_focal(const Exiv2::Image *image, const fs::path &path);
//std::string xmp_focal(const Exiv2::Image *image, const fs::path &path);
std::string exif_distance(const Exiv2::Image *image, const fs::path &path);
//std::string xmp_distance(const Exiv2::Image *image, const fs::path &path);
std::string exif_meter(const Exiv2::Image *image, const fs::path &path);
//std::string xmp_meter(const Exiv2::Image *image, const fs::path &path);
std::string exif_macro(const Exiv2::Image *image, const fs::path &path);
std::string exif_orientation(const Exiv2::Image *image, const fs::path &path);
std::string exif_lens(const Exiv2::Image *image, const fs::path &path);
std::string exif_keyword(const Exiv2::Image *image, const fs::path &path);
std::string iptc_keyword(const Exiv2::Image *image, const fs::path &path);
//std::string xmp_keyword(const Exiv2::Image *image, const fs::path &path);
std::string exif_iso(const Exiv2::Image *image, const fs::path &path);

#endif //HELPERS_HPP_

