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
  File:      utils.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   08-Dec-03, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: utils.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "utils.hpp"

// + standard includes
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
# include "getopt_win32.h"
# define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                     // for getopt(), stat()
#endif

#include <climits>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>

namespace Util {

// *****************************************************************************
// class Getopt
    Getopt::Getopt()
        : errcnt_(0)
    {
    }

    Getopt::~Getopt()
    {
    }

    int Getopt::getopt(int argc, char* const argv[], const std::string& optstring)
    {
        progname_ = Util::basename(argv[0]);

        for (;;) {
            int c = ::getopt(argc, argv, optstring.c_str());
            if (c == -1) break;
            errcnt_ += option(c, ::optarg == 0 ? "" : ::optarg, ::optopt);
        }
        for (int i = ::optind; i < argc; i++) {
            errcnt_ += nonoption(argv[i]);
        }
        return errcnt_;
    }

    int Getopt::nonoption(const std::string& /*argv*/)
    {
        return 0;
    }

// *****************************************************************************
// free functions

    std::string dirname(const std::string& path)
    {
        if (path == "") return ".";
        // Strip trailing slashes or backslashes
        std::string p = path;
        while (   p.length() > 1
               && (p[p.length()-1] == '\\' || p[p.length()-1] == '/')) {
            p = p.substr(0, p.length()-1);
        }
        if (p == "\\" || p == "/") return p;
        if (p.length() == 2 && p[1] == ':') return p; // For Windows paths
        std::string::size_type idx = p.find_last_of("\\/");
        if (idx == std::string::npos) return ".";
        if (idx == 1 && p[0] == '\\' && p[1] == '\\') return p; // For Windows paths
        p = p.substr(0, idx == 0 ? 1 : idx);
        while (   p.length() > 1
               && (p[p.length()-1] == '\\' || p[p.length()-1] == '/')) {
            p = p.substr(0, p.length()-1);
        }
        return p;
    }

    std::string basename(const std::string& path, bool delsuffix)
    {
        if (path == "") return ".";
        // Strip trailing slashes or backslashes
        std::string p = path;
        while (   p.length() > 1
               && (p[p.length()-1] == '\\' || p[p.length()-1] == '/')) {
            p = p.substr(0, p.length()-1);
        }
        if (p.length() == 2 && p[1] == ':') return ""; // For Windows paths
        std::string::size_type idx = p.find_last_of("\\/");
        if (idx == 1 && p[0] == '\\' && p[1] == '\\') return ""; // For Windows paths
        if (idx != std::string::npos) p = p.substr(idx+1);
        if (delsuffix) p = p.substr(0, p.length() - suffix(p).length());
        return p;
    }

    std::string suffix(const std::string& path)
    {
        std::string b = basename(path);
        std::string::size_type idx = b.rfind('.');
        if (idx == std::string::npos || idx == 0 || idx == b.length()-1) {
            return "";
        }
        return b.substr(idx);
    }

    bool strtol(const char* nptr, long& n)
    {
        if (!nptr || *nptr == '\0') return false;
        char* endptr = 0;
        long tmp = std::strtol(nptr, &endptr, 10);
        if (*endptr != '\0') return false;
        if (tmp == LONG_MAX || tmp == LONG_MIN) return false;
        n = tmp;
        return true;
    }

    void replace(std::string& text, const std::string& searchText, const std::string& replaceText)
    {
        std::string::size_type index = 0;
        while ((index = text.find(searchText, index)) != std::string::npos)
        {
            text.replace(index, searchText.length(), replaceText.c_str(), replaceText.length());
            index++;
        }
    }

}                                       // namespace Util
