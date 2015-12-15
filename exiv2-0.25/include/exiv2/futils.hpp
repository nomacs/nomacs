// ********************************************************* -*- C++ -*-
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
  @file    futils.hpp
  @brief   Basic file utility functions required by Exiv2
  @version $Rev: 3091 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    12-Dec-03, ahu: created<BR>
           02-Apr-05, ahu: moved to Exiv2 namespace
 */
#ifndef FUTILS_HPP_
#define FUTILS_HPP_

// *********************************************************************
// included header files
#include "config.h"
#include "types.hpp"

#ifdef EXV_HAVE_STDINT_H
# include <stdint.h>                     // for uint8_t
#endif

// + standard includes
#include <string>
#include <map>
#include <limits.h>						 // for UINT_MAX

// *********************************************************************
// namespace extensions
namespace Exiv2 {
    //! the name of environmental variables.
    enum    EnVar       { envHTTPPOST = 0, envTIMEOUT = 1};
    //! the collection of protocols.
    enum    Protocol    { pFile = 0, pHttp, pFtp, pHttps, pSftp, pSsh, pFileUri, pDataUri, pStdin};
// *********************************************************************
// free functions
    /*!
      @brief  Return the value of environmental variable.
      @param  var The name of environmental variable.
      @return the value of environmental variable. If it's empty, the default value is returned.
     */
    EXIV2API std::string getEnv(EnVar var);
    /*!
      @brief Convert an integer value to its hex character.
      @param code The integer value.
      @return the input's hex character.
     */
    EXIV2API char to_hex(char code);
    /*!
      @brief Convert a hex character to its integer value.
      @param ch The hex character.
      @return the input's integer value.
     */
    EXIV2API char from_hex(char ch);
    /*!
      @brief Encode the input url.
      @param str The url needs encoding.
      @return the url-encoded version of str.

      @note Be sure to free() the returned string after use
            Source: http://www.geekhideout.com/urlcode.shtml
     */
    EXIV2API char* urlencode(char* str);
    /*!
      @brief Decode the input url.
      @param str The url needs decoding.
      @return the url-decoded version of str.

      @note Be sure to free() the returned string after use
            Source: http://www.geekhideout.com/urlcode.shtml
     */
    EXIV2API char* urldecode(const char* str);
    /*!
      @brief Like urlencode(char* str) but accept the input url in the std::string and modify it.
     */
    EXIV2API void urldecode(std::string& str);
    /*!
      @brief Encode in base64 the data in data_buf and put the resulting string in result.
      @param data_buf The data need to encode
      @param dataLength Size in bytes of the in buffer
      @param result The container for the result, NULL if it fails
      @param resultSize Size in bytes of the out string, it should be at least ((dataLength + 2) / 3) * 4 + 1
      @return 1 indicate success

      @note Source: http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
      */
    EXIV2API int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize);
    /*!
      @brief Decode base64 data and put the resulting string in out.
      @param in The data need to decode.
      @param out The container for the result, it should be large enough to contain the result.
      @param out_size The size of out in bytes.
      @return the size of the resulting string. If it fails, return -1.

      @note Source: https://github.com/davidgaleano/libwebsockets/blob/master/lib/base64-decode.c
      */
    EXIV2API long base64decode(const char *in, char *out, size_t out_size);
    /*!
      @brief Return the protocol of the path.
      @param path The path of file to extract the protocol.
      @return the protocol of the path.
     */
    EXIV2API Protocol fileProtocol(const std::string& path);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like fileProtocol() but accept a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API Protocol fileProtocol(const std::wstring& wpath);
#endif
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
      @brief Like fileExists(const std::string& path, bool ct =false) but
            accepts a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API bool fileExists(const std::wstring& wpath, bool ct =false);
#endif
    /*!
      @brief Get the path of file URL.

      @param  url The file URL in the format file:///<path> or file://<host>/<path>.
      @return the path of file URL.
     */
    EXIV2API std::string pathOfFileUrl(const std::string& url);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like pathOfFileUrl(const std::string& url) but accepts a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API std::wstring pathOfFileUrl(const std::wstring& wurl);
#endif
    /*!
      @brief Return a system error message and the error code (errno).
             See %strerror(3).
     */
    EXIV2API std::string strError();

    /*!
      @brief A container for URL components. It also provides the method to parse a
            URL to get the protocol, host, path, port, querystring, username, password.

      Source: http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform
     */
    class Uri
    {
    public:
        // DATA
        std::string QueryString;    //!< URL query string
        std::string Path;           //!< URL file path
        std::string Protocol;       //!< URL protocol
        std::string Host;           //!< URL host
        std::string Port;           //!< URL port
        std::string Username;       //!< URL username
        std::string Password;       //!< URL password

        /*!
          @brief Parse the input URL to the protocol, host, path, username, password
         */
        static Uri EXIV2API Parse(const std::string &uri);

        /*!
          @brief Decode the url components.
         */
        static void EXIV2API Decode(Uri& uri);
    }; // class Uri

} // namespace Exiv2

#endif                                  // #ifndef FUTILS_HPP_
