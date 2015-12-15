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
 File:      helpers.cpp
 Version:   $Rev: 1762 $
 Author(s): Brad Schick (brad) <schickb@gmail.com>
 History:   19-Jan-09, brad: created
*/
// *****************************************************************************

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <exiv2/image.hpp>
#include <exiv2/easyaccess.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/iptc.hpp>
#include <exiv2/tags.hpp>
//#include <exiv2/xmp.hpp>
#include <cassert>
#include <sstream>
#include <ctime>
#include "helpers.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED

namespace fs = boost::filesystem;
typedef Exiv2::ExifData::const_iterator (*EasyAccessFct)(const Exiv2::ExifData& ed);


std::string scrub(const std::string &dirty, bool strip_space = false)
{
    std::string scrub = boost::trim_copy(dirty);
    if(strip_space) {
        boost::regex space("\\s");
        scrub = boost::regex_replace(scrub, space, "");
    }
    boost::regex dash("[:/\\\\|<>]");
    boost::regex under("[\"'\\[\\]\\{\\}#=%\\$\\?,\\+\\*]");
    scrub = boost::regex_replace(scrub, dash, "-");

    return boost::regex_replace(scrub, under, "_");
}

bool exif_data(const Exiv2::Image *image, const char *key, Exiv2::ExifData::const_iterator &md) 
{
    assert(image && key);
    bool ok = false;
    try {
        const Exiv2::ExifData &exifData = image->exifData();
        Exiv2::ExifKey exifKey(key);
        md = exifData.findKey(exifKey);
        if(md != exifData.end() && md->typeId() != Exiv2::undefined)
            ok = true;
    } 
    catch(const Exiv2::AnyError&) {
    }
    return ok;
}

bool exif_data_easy(const Exiv2::Image *image, EasyAccessFct easy, Exiv2::ExifData::const_iterator &md) 
{
    assert(image && easy);
    bool ok = false;
    try {
        const Exiv2::ExifData &exifData = image->exifData();
        md = easy(exifData);
        if(md != exifData.end() && md->typeId() != Exiv2::undefined)
            ok = true;
    } 
    catch(const Exiv2::AnyError&) {
    }
    return ok;
}


bool iptc_data(const Exiv2::Image *image, const char *key, Exiv2::IptcData::const_iterator &md) 
{
    bool ok = false;
    assert(image && key);
    try {
        const Exiv2::IptcData &iptcData = image->iptcData();
        Exiv2::IptcKey iptcKey(key);
        md = iptcData.findKey(iptcKey);
        if(md != iptcData.end() && md->typeId() != Exiv2::undefined)
            ok = true;
    } 
    catch(const Exiv2::AnyError&) {
    }
    return ok;
}

std::string exif_date(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.DateTimeDigitized", md);
    if(!done)
        done = exif_data(image, "Exif.Photo.DateTimeOriginal", md);
    if(!done)
        return "";

    std::string date = scrub(md->print().substr(0,10));
    // Some files have zeros for dates, just fail in that case
    if(boost::lexical_cast<int>(date.substr(0,4))==0)
        return "";

    return date;
}

std::string exif_year(const Exiv2::Image *image, const fs::path &path) 
{
    std::string date = exif_date(image, path);
    if(date.length())
        return date.substr(0,4);
    else
        return date;
}

std::string exif_month(const Exiv2::Image *image, const fs::path &path) 
{
    std::string date = exif_date(image, path);
    if(date.length())
        return date.substr(5,2);
    else
        return date;
}

std::string exif_day(const Exiv2::Image *image, const fs::path &path) 
{
    std::string date = exif_date(image, path);
    if(date.length())
        return date.substr(8,2);
    else
        return date;
}

bool iptc_get_date(const Exiv2::Image *image, Exiv2::DateValue::Date &date) 
{
    Exiv2::IptcData::const_iterator md;
    bool done = iptc_data(image, "Iptc.Application2.DigitizationDate", md);
    if(!done)
        done = iptc_data(image, "Iptc.Application2.DateCreated", md);
    if(!done)
        return false;
    date = ((Exiv2::DateValue*)md->getValue().get())->getDate();
    return date.year > 0;
}

std::string iptc_date(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::DateValue::Date date;
    if(iptc_get_date(image, date))
        return str(boost::format("%4d-%02d-%02d") % date.year % date.month % date.day);
    else
        return "";
}

std::string iptc_year(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::DateValue::Date date;
    if(iptc_get_date(image, date))
        return str(boost::format("%4d") % date.year);
    else
        return "";
}

std::string iptc_month(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::DateValue::Date date;
    if(iptc_get_date(image, date))
        return str(boost::format("%02d") % date.month);
    else
        return "";
}

std::string iptc_day(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::DateValue::Date date;
    if(iptc_get_date(image, date))
        return str(boost::format("%02d") % date.day);
    else
        return "";
}

bool file_get_tm(const fs::path &path, std::tm &tm)
{
    std::time_t timer = fs::last_write_time(path);
    if(time > 0) {
        tm = *localtime(&timer);
        return true;
    } 
    else {
        return false;
    }
}

std::string file_date(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%4d-%02d-%02d") % (tm.tm_year + 1900) % (tm.tm_mon + 1) % tm.tm_mday);
    else
        return "";
}

std::string file_year(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%4d") % (tm.tm_year + 1900));
    else
        return "";
}

std::string file_month(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%02d") % (tm.tm_mon + 1));
    else
        return "";
}

std::string file_day(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%02d") % tm.tm_mday);
    else
        return "";
}

/*
std::string xmp_date(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}

std::string xmp_year(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}

std::string xmp_month(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}

std::string xmp_day(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_time(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.DateTimeDigitized", md);
    if(!done)
        done = exif_data(image, "Exif.Photo.DateTimeOriginal", md);
    if(!done)
        return "";

    std::string datetime = md->print();
    // Some files have zeros for dates, just fail in that case
    if(boost::lexical_cast<int>(datetime.substr(0,4)) == 0)
        return "";

    return scrub(datetime.substr(11));
}

std::string exif_hour(const Exiv2::Image *image, const fs::path &path) 
{
    std::string time = exif_time(image, path);
    if(time.length())
        return time.substr(0,2);
    else
        return time;
}

std::string exif_minute(const Exiv2::Image *image, const fs::path &path) 
{
    std::string time = exif_time(image, path);
    if(time.length())
        return time.substr(3,2);
    else
        return time;
}

std::string exif_second(const Exiv2::Image *image, const fs::path &path) 
{
    std::string time = exif_time(image, path);
    if(time.length())
        return time.substr(6,2);
    else
        return time;
}

bool iptc_get_time(const Exiv2::Image *image, Exiv2::TimeValue::Time &time) 
{
    Exiv2::IptcData::const_iterator md;
    bool done = iptc_data(image, "Iptc.Application2.DigitizationTime", md);
    if(!done)
        done = iptc_data(image, "Iptc.Application2.TimeCreated", md);
    if(!done)
        return false;
    time = ((Exiv2::TimeValue*)md->getValue().get())->getTime();
    // Zero is a valid time, so this one is hard to check.
    return true;
}

std::string iptc_time(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::TimeValue::Time time;
    if(iptc_get_time(image, time))
        return str(boost::format("%02d-%02d-%02d") % time.hour % time.minute % time.second);
    else
        return "";
}

std::string iptc_hour(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::TimeValue::Time time;
    if(iptc_get_time(image, time))
        return str(boost::format("%02d") % time.hour);
    else
        return "";
}

std::string iptc_minute(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::TimeValue::Time time;
    if(iptc_get_time(image, time))
        return str(boost::format("%02d") % time.minute);
    else
        return "";
}

std::string iptc_second(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::TimeValue::Time time;
    if(iptc_get_time(image, time))
        return str(boost::format("%02d") % time.second);
    else
        return "";
}

std::string file_time(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%02d-%02d-%02d") % tm.tm_hour % tm.tm_min % tm.tm_sec);
    else
        return "";
}

std::string file_hour(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%02d") % tm.tm_hour);
    else
        return "";
}

std::string file_minute(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%02d") % tm.tm_min);
    else
        return "";
}

std::string file_second(const Exiv2::Image *, const fs::path &path) 
{
    std::tm tm;
    if(file_get_tm(path, tm))
        return str(boost::format("%02d") % tm.tm_sec);
    else
        return "";
}

/*std::string xmp_time(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}

std::string xmp_hour(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}

std::string xmp_minute(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}

std::string xmp_second(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_dimension(const Exiv2::Image *image, const fs::path &path) 
{
    return exif_width(image, path) + "-" + exif_height(image, path);
}

std::string exif_width(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.PixelXDimension", md);
    if(!done)
        return "";
    return scrub(md->print());
}

std::string exif_height(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.PixelYDimension", md);
    if(!done)
        return "";
    return scrub(md->print());
}

std::string file_dimension(const Exiv2::Image *image, const fs::path &path) 
{
    if(image)
        return file_width(image, path) + "-" + file_height(image, path);
    else
        return "";
}

std::string file_width(const Exiv2::Image *image, const fs::path &) 
{
    if(image)
        return str(boost::format("%02d") % image->pixelWidth());
    else
        return "";
}

std::string file_height(const Exiv2::Image *image, const fs::path &) 
{
    if(image)
        return str(boost::format("%02d") % image->pixelHeight());
    else
        return "";
}

/*
std::string xmp_dimension(const Exiv2::Image *image, const fs::path &) 
{
    return ""
}

std::string xmp_width(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}

std::string xmp_height(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_model(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Image.Model", md);
    if(!done)
        return "";
    return scrub(md->print());
}

std::string exif_make(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Image.Make", md);
    if(!done)
        return "";
    return scrub(md->print());
}

/*std::string xmp_model(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_speed(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.ShutterSpeedValue", md);
    if(!done)
        done = exif_data(image, "Exif.Photo.ExposureTime", md);
    if(!done)
        return "";
    return scrub(md->print());
}

/*std::string xmp_speed(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_aperture(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.ApertureValue", md);
    if(!done)
        done = exif_data(image, "Exif.Photo.FNumber", md);
    if(!done)
        return "";
    return scrub(md->print());
}

/*std::string xmp_aperture(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_focal(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.FocalLength", md);
    if(!done)
        return "";
    return scrub(md->print());
}

/*std::string xmp_focal(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_distance(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.SubjectDistance", md);
    if(!done)
        return "";
    return scrub(md->print());
}

/*std::string xmp_distance(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_meter(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.MeteringMode", md);
    if(!done)
        return "";
    return scrub(md->print());
}

std::string exif_macro(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data_easy(image, Exiv2::macroMode, md);
    if(!done)
        return "";
    return scrub(md->print());
}

std::string exif_orientation(const Exiv2::Image *image, const fs::path &)
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data_easy(image, Exiv2::orientation, md);
    if(!done)
        return "";
    return scrub(md->print(), true);
}

std::string exif_lens(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data_easy(image, Exiv2::lensName, md);
    if(!done)
        return "";
    return scrub(md->print());
}


std::string exif_iso(const Exiv2::Image *image, const fs::path &)
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data_easy(image, Exiv2::isoSpeed, md);
    if(!done)
        return "";
    return scrub(md->print());
}

/*std::string xmp_meter(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

std::string exif_keyword(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::ExifData::const_iterator md;
    bool done = exif_data(image, "Exif.Photo.UserComment", md);
    if(!done)
        return "";
    return scrub(md->print());
}

std::string iptc_keyword(const Exiv2::Image *image, const fs::path &) 
{
    Exiv2::IptcData::const_iterator md;
    bool done = iptc_data(image, "Iptc.Application2.Keywords", md);
    if(!done)
        return "";
    return scrub(md->print());
}

/*std::string xmp_keyword(const Exiv2::Image *image, const fs::path &) 
{
    return "";
}*/

