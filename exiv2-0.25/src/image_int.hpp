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
  @file    image_int.hpp
  @brief   Internal image helpers
  @version $Rev: 3777 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    10-May-15, ahu: created
 */
#ifndef IMAGE_INT_HPP_
#define IMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    /*!
      @brief format a string in the pattern of \em sprintf \em .
     */
    std::string stringFormat(const char* format, ...);

    /*!
      @brief format binary for display in \em printStructure() \em .
     */
    std::string binaryToString(DataBuf& buf, size_t size, size_t start =0);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef IMAGE_INT_HPP_
