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
  @file    utilsvideo.hpp
  @brief   An Image subclass to support RIFF video files
  @version $Rev$
           Mahesh Hegde 2014
           <b href="mailto:maheshmhegade@gmail.com">maheshmhegade@gmail.com</b>
  @date    16-Aug-14, AB: created
 */
#include "tags_int.hpp"

namespace Exiv2
{

class UtilsVideo
{
public:
    static bool compareTagValue(Exiv2::DataBuf &buf, const char *str);
    static bool compareTagValue(Exiv2::DataBuf& buf,const char arr[][5],int32_t arraysize);
    static bool simpleBytesComparison(Exiv2::DataBuf& buf ,const char* str,int32_t size);
}; // class UtilsVideo

} // namespace Exiv2
