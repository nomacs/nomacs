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
/*
  File:      futils.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   08-Dec-03, ahu: created
             02-Apr-05, ahu: moved to Exiv2 namespace
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: futils.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "futils.hpp"

// + standard includes
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
# define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                     // for stat()
#endif

#include <cerrno>
#include <sstream>
#include <cstring>

#if defined EXV_HAVE_STRERROR_R && !defined EXV_HAVE_DECL_STRERROR_R
# ifdef EXV_STRERROR_R_CHAR_P
extern char *strerror_r(int errnum, char *buf, size_t n);
# else
extern int strerror_r(int errnum, char *buf, size_t n);
# endif
#endif

namespace Exiv2 {

// *****************************************************************************
// free functions

    bool fileExists(const std::string& path, bool ct)
    {
        struct stat buf;
		int ret = ::stat(path.c_str(), &buf);
        if (0 != ret)                    return false;
        if (ct && !S_ISREG(buf.st_mode)) return false;
        return true;
    } // fileExists

#ifdef EXV_UNICODE_PATH
    bool fileExists(const std::wstring& wpath, bool ct)
    {
        struct _stat buf;
        int ret = _wstat(wpath.c_str(), &buf);
        if (0 != ret)                    return false;
        if (ct && !S_ISREG(buf.st_mode)) return false;
        return true;
    } // fileExists

#endif
    std::string strError()
    {
        int error = errno;
        std::ostringstream os;
#ifdef EXV_HAVE_STRERROR_R
        const size_t n = 1024;
// _GNU_SOURCE: See Debian bug #485135
# if defined EXV_STRERROR_R_CHAR_P && defined _GNU_SOURCE
        char *buf = 0;
        char buf2[n];
        std::memset(buf2, 0x0, n);
        buf = strerror_r(error, buf2, n);
# else
        char buf[n];
        std::memset(buf, 0x0, n);
        strerror_r(error, buf, n);
# endif
        os << buf;
#else
        os << std::strerror(error);
#endif
        os << " (errno = " << error << ")";
        return os.str();
    } // strError

}                                       // namespace Exiv2
