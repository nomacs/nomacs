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
  @version $Rev: 3845 $
           Mahesh Hegde 2014
           <b href="mailto:maheshmhegade@gmail.com">maheshmhegade@gmail.com</b>
  @date    16-Aug-14, AB: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: utilsvideo.cpp 3845 2015-06-07 16:29:06Z ahuggel $")

#ifdef EXV_ENABLE_VIDEO
#include "utilsvideo.hpp"

#ifndef   _MSC_VER
#define stricmp strcasecmp
#endif

namespace Exiv2
{

bool UtilsVideo::compareTagValue(Exiv2::DataBuf& buf ,const char* str){
    bool  result = true;
    for(int32_t i=0; result && i<4; i++ )
        if(tolower(buf.pData_[i]) != tolower(str[i]))
            return false;
    return true;
}

bool UtilsVideo::compareTagValue(Exiv2::DataBuf& buf,const char arr[][5],int32_t arraysize){
    bool  result = false;
    for ( int32_t i=0; !result && i< arraysize; i++)
        result  = (bool)(stricmp((const char*)buf.pData_,arr[i])==0);
    return result;
}

bool UtilsVideo::simpleBytesComparison(Exiv2::DataBuf& buf ,const char* str,int32_t size){
    for(int32_t i=0; i<size; i++ )
        if(toupper(buf.pData_[i]) != str[i])
            return false;
    return true;
}

} // namespace Exiv2
#endif // EXV_ENABLE_VIDEO
