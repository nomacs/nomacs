// ********************************************************* -*- C++ -*-
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
  @file    futils.hpp
  @brief   Basic file utility functions required by Exiv2
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    12-Dec-03, ahu: created<BR>
           02-Apr-05, ahu: moved to Exiv2 namespace
 */
#ifndef FUTILS_HPP_
#define FUTILS_HPP_

// *********************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

// + standard includes
#include <string>

// *********************************************************************
// namespace extensions
namespace Exiv2 {

// *********************************************************************
// free functions

    /*!
      @brief Test if a file exists.

      @param  path Name of file to verify.
      @param  ct   Flag to check if <i>path</i> is a regular file.
      @return true if <i>path</i> exists and, if <i>ct</i> is set,
      is a regular file, else false.

      @note The function calls <b>stat()</b> test for <i>path</i>
      and its type, see stat(2). <b>errno</b> is left unchanged
      in case of an error.
     */
    EXIV2API bool fileExists(const std::string& path, bool ct =false);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like fileExists() but accepts a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API bool fileExists(const std::wstring& wpath, bool ct =false);
#endif
    /*!
      @brief Return a system error message and the error code (errno).
             See %strerror(3).
     */
    EXIV2API std::string strError();

}                                       // namespace Exiv2

#endif                                  // #ifndef FUTILS_HPP_
