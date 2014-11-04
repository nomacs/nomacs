// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2013 Andreas Huggel <ahuggel@gmx.net>
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
  File:      version.cpp
  Version:   $Rev: 3201 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-Mar-07, ahu: created

 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: version.cpp 3201 2013-12-01 12:13:42Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
# ifndef  __MINGW__
#  define __MINGW__
# endif
#endif

#if defined(__CYGWIN__) || defined(__MINGW__)
#include <windows.h>
#endif

#include "version.hpp"

// + standard includes
#include <iomanip>
#include <sstream>

namespace Exiv2 {
    int versionNumber()
    {
        return EXIV2_MAKE_VERSION(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION);
    }

    std::string versionString()
    {
        std::ostringstream os;
        os << EXIV2_MAJOR_VERSION << '.' << EXIV2_MINOR_VERSION << '.' << EXIV2_PATCH_VERSION;
        return os.str();

    }

    std::string versionNumberHexString()
    {
        std::ostringstream os;
        os << std::hex << std::setw(6) << std::setfill('0') << Exiv2::versionNumber();
        return os.str();
    }

    const char* version()
    {
        return EXV_PACKAGE_VERSION;
    }

    bool testVersion(int major, int minor, int patch)
    {
        return versionNumber() >= EXIV2_MAKE_VERSION(major,minor,patch);
    }
}                                       // namespace Exiv2

#include <string>
#include <vector>
#include <stdio.h>
using namespace std;
typedef vector<string>      string_v;
typedef string_v::iterator  string_i;

#ifndef lengthof
#define lengthof(x) sizeof(x)/sizeof(x[0])
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 512
#endif


// platform specific support for dumpLibraryInfo
#if defined(WIN32)
# include <windows.h>
# include <psapi.h>

// tell MSVC to link psapi.
#ifdef  _MSC_VER
#pragma comment( lib, "psapi" )
#endif

#elif defined(__APPLE__)
# include <mach-o/dyld.h>

#elif defined(__linux__)
# include <unistd.h>
// http://syprog.blogspot.com/2011/12/listing-loaded-shared-objects-in-linux.html
# include "link.h"
# include <dlfcn.h>
  struct something
  {
    void*  pointers[3];
    struct something* ptr;
  };
  struct lmap
  {
    void*    base_address;   /* Base address of the shared object */
    char*    path;           /* Absolute file name (path) of the shared object */
    void*    not_needed1;    /* Pointer to the dynamic section of the shared object */
    struct lmap *next, *prev;/* chain of loaded objects */
  };
#elif defined(__MINGW32__) || defined(__MINGW64__)
#ifndef __MINGW__
#define __MINGW__
#endif
#endif

EXIV2API void dumpLibraryInfo(std::ostream& os)
{
      string_v libs; // libs[0] == executable

      int      bits = 8*sizeof(void*);
#if   defined(_DEBUG) || defined(DEBUG)
      int debug=1;
#else
      int debug=0;
#endif

#if   defined(EXV_HAVE_DLL)
      int dll=1;
#else
      int dll=0;
#endif

      const char* compiler =
#if   defined(_MSC_VER)
      "MSVC"    ;

#ifndef __VERSION__
      char version[20];
      sprintf(version,"%d.%02d",(_MSC_VER-600)/100,_MSC_VER%100);
#define __VERSION__ version
#endif

#elif defined(__clang__)
      "Clang"   ;
#elif defined(__GNUG__)
      "G++"     ;
#elif defined(__GNUC__)
      "GCC"     ;
#elif defined(__SUNPRO_CC)
      "CC (oracle)";
#elif defined (__SUNPRO_C)
      "cc (oracle)";
#else
      "unknown" ;
#endif

#if defined(__SUNPRO_CC) || defined (__SUNPRO_C)
#define     __oracle__      
#endif
      
      
#ifndef __VERSION__
#ifdef  __clang__version__
#define __VERSION__ __clang__version__
#else
#define __VERSION__ "unknown"
#endif
#endif

      const char* platform =
#if defined(__CYGWIN__)
      "cygwin";
#elif defined(_MSC_VER)
      "windows";
#elif defined(__APPLE__)
      "apple";
#elif defined(__MINGW__)
      "mingw";
#elif defined(__linux__)
      "linux";
#else
      "unknown";
#endif

#if defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW__)
    // enumerate loaded libraries and determine path to executable
    HMODULE handles[200];
    DWORD   cbNeeded;
    if ( EnumProcessModules(GetCurrentProcess(),handles,lengthof(handles),&cbNeeded)) {
        char szFilename[_MAX_PATH];
        for ( DWORD h = 0 ; h < cbNeeded/sizeof(handles[0]) ; h++ ) {
            GetModuleFileNameA(handles[h],szFilename,lengthof(szFilename)) ;
            libs.push_back(szFilename);
        }
    }
#elif defined(__APPLE__)
    // man 3 dyld
    uint32_t count = _dyld_image_count();
    for (uint32_t image = 0 ; image < count ; image++ ) {
        const char* image_path = _dyld_get_image_name(image);
        libs.push_back(image_path);
    }
#elif defined(__linux__)
    // http://stackoverflow.com/questions/606041/how-do-i-get-the-path-of-a-process-in-unix-linux
    char proc[100];
    char path[500];
    sprintf(proc,"/proc/%d/exe", getpid());
    int l = readlink (proc, path,sizeof(path)-1);
    if (l>0) {
        path[l]=0;
        libs.push_back(path);
    } else {
        libs.push_back("unknown");
    }

    // http://syprog.blogspot.com/2011/12/listing-loaded-shared-objects-in-linux.html
    struct lmap* pl;
    void* ph = dlopen(NULL, RTLD_NOW);
    struct something* p = (struct something*)ph;
    p  = p->ptr;
    pl = (struct lmap*)p->ptr;

    while ( pl )
    {
        libs.push_back(pl->path);
        pl = pl->next;
    }
#endif

    os << "exiv2="    << Exiv2::versionString() << endl;
    os << "platform=" << platform               << endl;
    os << "compiler=" << compiler               << endl;
    os << "bits="     << bits                   << endl;
    os << "dll="      << dll                    << endl;
    os << "debug="    << debug                  << endl;
    os << "version="  << __VERSION__            << endl;
    os << "date="     << __DATE__               << endl;
    os << "time="     << __TIME__               << endl;

    if ( libs.begin() != libs.end() ) {
        os << "executable=" << *libs.begin() << endl;
        for ( string_i lib = libs.begin()+1 ; lib != libs.end() ; lib++ )
            os << "library=" << *lib << endl;
    }
}
