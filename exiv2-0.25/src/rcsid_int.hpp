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
  @file    rcsid_int.hpp
  @brief   Define an RCS id string in every object file compiled from a source
           file that includes rcsid_int.hpp.

  This is a simplified version of the ACE_RCSID macro that is used in the
  <a href="http://www.cs.wustl.edu/~schmidt/ACE.html">ACE(TM)</a> distribution.

  @version $Rev: 3777 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    02-Feb-04, ahu: created
 */
#ifndef RCSID_INT_HPP_
#define RCSID_INT_HPP_

#if !defined (EXIV2_RCSID)
/*!
  @brief Macro to store version information in each object file.

         Use this macro by including the following two lines at the beginning of
         each *.cpp file.  See the ident(1) manual pages for more information.

         @code
         #include "rcsid_int.hpp"
         EXIV2_RCSID("@(#) $Id$");
         @endcode

         The macro hack itself has the following purposes:
         -# To define the RCS id string variable in the local namespace, so
            that there won't be any duplicate extern symbols at link time.
         -# To avoid warnings of the type "variable declared and never used".

 */

#if (defined(__GNUG__) || defined(__GNUC__) || defined (_MSC_VER)) && ! defined(__clang__)
#define EXIV2_RCSID(id) \
    namespace { \
        inline const char* getRcsId(const char*) { return id ; } \
        const char* rcsId = getRcsId(rcsId); \
    }
#else
#define EXIV2_RCSID(id)
#endif

#endif // #if !defined (EXIV2_RCSID)
#endif // #ifndef RCSID_INT_HPP_
