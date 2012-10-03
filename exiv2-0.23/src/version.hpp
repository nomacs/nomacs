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
  @file    version.hpp
  @brief   Precompiler define and a function to test the %Exiv2 version.
           References: Similar versioning defines are used in KDE, GTK and other
           libraries. See http://apr.apache.org/versioning.html for accompanying
           guidelines.
  @version $Rev: 2685 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    31-May-06, ahu: created
 */
#ifndef VERSION_HPP_
#define VERSION_HPP_

// *****************************************************************************
// included header files
// + standard includes
#include <string>

/*!
  @brief %Exiv2 MAJOR version number of the library used at compile-time.
 */
#define EXIV2_MAJOR_VERSION  (0)
/*!
  @brief %Exiv2 MINOR version number of the library used at compile-time.
 */
#define EXIV2_MINOR_VERSION (23)
/*!
  @brief %Exiv2 PATCH version number of the library used at compile-time.
 */
#define EXIV2_PATCH_VERSION  (0)
/*!
  @brief Make an integer version number for comparison from a major, minor and
         a patch version number.
 */
#define EXIV2_MAKE_VERSION(major,minor,patch) \
    (((major) << 16) | ((minor) << 8) | (patch))
/*!
  @brief The %Exiv2 version number of the library used at compile-time as
         an integer number for easy comparison.
 */
#define EXIV2_VERSION \
    EXIV2_MAKE_VERSION(EXIV2_MAJOR_VERSION,EXIV2_MINOR_VERSION,EXIV2_PATCH_VERSION)
/*!
  @brief Deprecated version check macro. Do not use.

  This macro has flaws and only remains for backward compatibility.
  Use EXIV2_TEST_VERSION and testVersion() instead.
 */
#define EXIV2_CHECK_VERSION(major,minor,patch) \
    ( Exiv2::versionNumber() >= EXIV2_MAKE_VERSION(major,minor,patch) )
/*!
  @brief Macro to test the version of the available %Exiv2 library at compile-time.
         Return true if it is the same as or newer than the passed-in version.

  Versions are denoted using a triplet of integers: \em MAJOR.MINOR.PATCH .

  @code
  // Don't include the <exiv2/version.hpp> file directly, it is included by
  // <exiv2/types.hpp>. Early Exiv2 versions didn't have version.hpp and the macros.

  #include <exiv2/types.hpp>

  // Make sure an EXIV2_TEST_VERSION macro exists:

  #ifdef EXIV2_VERSION
  # ifndef EXIV2_TEST_VERSION
  # define EXIV2_TEST_VERSION(major,minor,patch) \
      ( EXIV2_VERSION >= EXIV2_MAKE_VERSION(major,minor,patch) )
  # endif
  #else
  # define EXIV2_TEST_VERSION(major,minor,patch) (false)
  #endif

  std::cout << "Compiled with Exiv2 version " << EXV_PACKAGE_VERSION << "\n"
            << "Runtime Exiv2 version is    " << Exiv2::version()    << "\n";

  // Test the Exiv2 version available at runtime but compile the if-clause only if
  // the compile-time version is at least 0.15. Earlier versions didn't have a
  // testVersion() function:

  #if EXIV2_TEST_VERSION(0,15,0)
  if (Exiv2::testVersion(0,13,0)) {
      std::cout << "Available Exiv2 version is equal to or greater than 0.13\n";
  }
  else {
      std::cout << "Installed Exiv2 version is less than 0.13\n";
  }
  #else
  std::cout << "Compile-time Exiv2 version doesn't have Exiv2::testVersion()\n";
  #endif
  @endcode
 */
#define EXIV2_TEST_VERSION(major,minor,patch) \
    ( EXIV2_VERSION >= EXIV2_MAKE_VERSION(major,minor,patch) )

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    /*!
      @brief Return the version of %Exiv2 available at runtime as an integer.
    */
    EXIV2API int versionNumber();
    /*!
      @brief Return the version of %Exiv2 as hex string of fixed length 6.
    */
    EXIV2API std::string versionNumberHexString();
    /*!
      @brief Return the version of %Exiv2 available at runtime as a string.
    */
    EXIV2API const char* version();
    /*!
      @brief Test the version of the available %Exiv2 library at runtime. Return
             true if it is the same as or newer than the passed-in version.

      Versions are denoted using a triplet of integers: \em major.minor.patch .

      @code
      // Don't include the <exiv2/version.hpp> file directly, it is included by
      // <exiv2/types.hpp>. Early Exiv2 versions didn't have version.hpp and the macros.

      #include <exiv2/types.hpp>

      // Make sure an EXIV2_TEST_VERSION macro exists:

      #ifdef EXIV2_VERSION
      # ifndef EXIV2_TEST_VERSION
      # define EXIV2_TEST_VERSION(major,minor,patch) \
          ( EXIV2_VERSION >= EXIV2_MAKE_VERSION(major,minor,patch) )
      # endif
      #else
      # define EXIV2_TEST_VERSION(major,minor,patch) (false)
      #endif

      std::cout << "Compiled with Exiv2 version " << EXV_PACKAGE_VERSION << "\n"
                << "Runtime Exiv2 version is    " << Exiv2::version()    << "\n";

      // Test the Exiv2 version available at runtime but compile the if-clause only if
      // the compile-time version is at least 0.15. Earlier versions didn't have a
      // testVersion() function:

      #if EXIV2_TEST_VERSION(0,15,0)
      if (Exiv2::testVersion(0,13,0)) {
          std::cout << "Available Exiv2 version is equal to or greater than 0.13\n";
      }
      else {
          std::cout << "Installed Exiv2 version is less than 0.13\n";
      }
      #else
      std::cout << "Compile-time Exiv2 version doesn't have Exiv2::testVersion()\n";
      #endif
      @endcode
     */
    EXIV2API bool testVersion(int major, int minor, int patch);
}                                       // namespace Exiv2
#endif                                  // VERSION_HPP_
