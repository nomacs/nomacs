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
/*
  File:      version.cpp
  Version:   $Rev: 3800 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-Mar-07, ahu: created

 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: version.cpp 3800 2015-05-08 22:26:36Z robinwmills $")

// *****************************************************************************

#include "config.h"

#ifndef EXV_USE_SSH
#define EXV_USE_SSH 0
#endif

#ifndef EXV_USE_CURL
#define EXV_USE_CURL 0
#endif

#if EXV_USE_CURL == 1
#include <curl/curl.h>
#endif

#if defined(__CYGWIN__) || defined(__MINGW__)
#include <windows.h>
#endif

#include "http.hpp"
#include "svn_version.h"
#include "version.hpp"

// + standard includes
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>

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
#endif

static void output(std::ostream& os,const exv_grep_keys_t& greps,const char* name,const std::string& value)
{
    bool bPrint = greps.empty();
    for( exv_grep_keys_t::const_iterator g = greps.begin();
        !bPrint && g != greps.end() ; ++g
    ) {
#if EXV_HAVE_REGEX
        bPrint = (  0 == regexec( &(*g), name         , 0, NULL, 0)
                 || 0 == regexec( &(*g), value.c_str(), 0, NULL, 0)
                 );
#else
        bPrint = std::string(name).find(*g) != std::string::npos || value.find(*g) != std::string::npos;
#endif
    }
    if ( bPrint ) os << name << "=" << value << endl;
}

static void output(std::ostream& os,const exv_grep_keys_t& greps,const char* name,int value)
{
    std::ostringstream stringStream;
    stringStream << value;
    output(os,greps,name,stringStream.str());
}

void Exiv2::dumpLibraryInfo(std::ostream& os,const exv_grep_keys_t& keys)
{
    string_v libs; // libs[0] == executable

    int      bits = 8*sizeof(void*);
#if defined(_DEBUG) || defined(DEBUG)
    int debug=1;
#else
    int debug=0;
#endif

#if defined(DLL_EXPORT) || defined(EXV_HAVE_DLL)
    int dll=1;
#else
    int dll=0;
#endif

    const char* compiler =
#if defined(_MSC_VER)
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
#define __oracle__
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
#elif defined(__MINGW64__)
    "mingw64";
#elif defined(__MINGW32__)
    "mingw32";
#elif defined(__linux__)
    "linux";
#else
    "unknown";
#endif

    int have_regex       =0;
    int have_gmtime_r    =0;
    int have_inttypes    =0;
    int have_libintl     =0;
    int have_lensdata    =0;
    int have_iconv       =0;
    int have_memory      =0;
    int have_memset      =0;
    int have_lstat       =0;
    int have_stdbool     =0;
    int have_stdint      =0;
    int have_stdlib      =0;
    int have_strlib      =0;
    int have_strchr      =0;
    int have_strerror    =0;
    int have_strerror_r  =0;
    int have_strings_h   =0;
    int have_strtol      =0;
    int have_mmap        =0;
    int have_munmap      =0;
    int have_sys_stat    =0;
    int have_timegm      =0;
    int have_unistd_h    =0;
    int have_sys_mman    =0;
    int have_libz        =0;
    int have_xmptoolkit  =0;
    int have_bool        =0;
    int have_strings     =0;
    int have_sys_types   =0;
    int have_unistd      =0;
    int have_unicode_path=0;

    int enable_video     =0;
    int enable_webready  =0;

#if EXV_HAVE_DECL_STRERROR_R
    have_strerror_r=1;
#endif

#if EXV_HAVE_GMTIME_R
    have_gmtime_r=1;
#endif

#if EXV_HAVE_INTTYPES_H
    have_inttypes=1;
#endif

#if EXV_HAVE_LIBINTL_H
    have_libintl=1;
#endif

#if EXV_HAVE_LENSDATA
    have_lensdata=1;
#endif

#if EXV_HAVE_ICONV
    have_iconv=1;
#endif

#if EXV_HAVE_LIBINTL_H
    have_libintl=1;
#endif

#if EXV_HAVE_REGEX
    have_regex=1;
#endif

#if EXV_HAVE_MEMORY_H
    have_memory=1;
#endif

#if EXV_HAVE_MEMSET
    have_memset=1;
#endif

#if EXV_HAVE_LSTAT
    have_lstat=1;
#endif

#if EXV_HAVE_STDBOOL_H
    have_stdbool=1;
#endif

#if EXV_HAVE_STDINT_H
    have_stdint=1;
#endif

#if EXV_HAVE_STDLIB_H
    have_stdlib=1;
#endif

#if EXV_HAVE_STRCHR
    have_strchr=1;
#endif

#if EXV_HAVE_STRERROR
    have_strerror=1;
#endif

#if EXV_HAVE_STRERROR_R
    have_strerror_r=1;
#endif

#if EXV_HAVE_STRINGS_H
    have_strings=1;
#endif

#if EXV_HAVE_STRTOL
    have_strtol=1;
#endif

#if EXV_HAVE_MMAP
    have_mmap=1;
#endif

#if EXV_HAVE_MUNMAP
    have_munmap=1;
#endif

#if EXV_HAVE_SYS_STAT_H
    have_sys_stat=1;
#endif

#if EXV_HAVE_SYS_TYPES_H
    have_sys_types=1;
#endif

#if EXV_HAVE_TIMEGM
    have_timegm=1;
#endif

#if EXV_HAVE_UNISTD_H
    have_unistd=1;
#endif

#if EXV_HAVE_SYS_MMAN_H
    have_sys_mman=1;
#endif

#if HAVE_LIBZ
    have_libz=1;
#endif

#if EXV_HAVE_XMP_TOOLKIT
    have_xmptoolkit=1;
#endif

#if EXV_HAVE__BOOL
    have_bool=1;
#endif

#if  EXV_HAVE_STRINGS
     have_strings=1;
#endif

#if  EXV_SYS_TYPES
     have_sys_types=1;
#endif

#if  EXV_HAVE_UNISTD
     have_unistd=1;
#endif

#if  EXV_UNICODE_PATH
     have_unicode_path=1;
#endif

#if  EXV_ENABLE_VIDEO
     enable_video=1;
#endif

#if  EXV_ENABLE_WEBREADY
     enable_webready=1;
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
    struct lmap*      pl;
    void*             ph = dlopen(NULL, RTLD_NOW);
    struct something* p  = (struct something*) ph;

    p  = p->ptr;
    pl = (struct lmap*)p->ptr;

    while ( pl )
    {
        libs.push_back(pl->path);
        pl = pl->next;
    }
#endif
    output(os,keys,"exiv2",Exiv2::versionString());
    output(os,keys,"platform"       , platform   );
    output(os,keys,"compiler"       , compiler   );
    output(os,keys,"bits"           , bits       );
    output(os,keys,"dll"            , dll        );
    output(os,keys,"debug"          , debug      );
    output(os,keys,"version"        , __VERSION__);
    output(os,keys,"date"           , __DATE__   );
    output(os,keys,"time"           , __TIME__   );
    output(os,keys,"svn"            , SVN_VERSION);
    output(os,keys,"ssh"            , EXV_USE_SSH);
#if EXV_USE_CURL == 1
    std::string curl_protocols;
    curl_version_info_data* vinfo = curl_version_info(CURLVERSION_NOW);
    for (int i = 0; vinfo->protocols[i]; i++) {
        curl_protocols += vinfo->protocols[i];
        curl_protocols += " " ;
    }
    output(os,keys,"curlprotocols" ,curl_protocols);
#else
    output(os,keys,"curl"          , EXV_USE_CURL);
#endif
    output(os,keys,"id"        , "$Id: version.cpp 3800 2015-05-08 22:26:36Z robinwmills $");
    if ( libs.begin() != libs.end() ) {
        output(os,keys,"executable" ,*libs.begin());
        for ( string_i lib = libs.begin()+1 ; lib != libs.end() ; lib++ )
            output(os,keys,"library",*lib);
    }

    output(os,keys,"have_regex"        ,have_regex       );
    output(os,keys,"have_strerror_r"   ,have_strerror_r  );
    output(os,keys,"have_gmtime_r"     ,have_gmtime_r    );
    output(os,keys,"have_inttypes"     ,have_inttypes    );
    output(os,keys,"have_libintl"      ,have_libintl     );
    output(os,keys,"have_lensdata"     ,have_lensdata    );
    output(os,keys,"have_iconv"        ,have_iconv       );
    output(os,keys,"have_memory"       ,have_memory      );
    output(os,keys,"have_memset"       ,have_memset      );
    output(os,keys,"have_lstat"        ,have_lstat       );
    output(os,keys,"have_stdbool"      ,have_stdbool     );
    output(os,keys,"have_stdint"       ,have_stdint      );
    output(os,keys,"have_stdlib"       ,have_stdlib      );
    output(os,keys,"have_strlib"       ,have_strlib      );
    output(os,keys,"have_strchr"       ,have_strchr      );
    output(os,keys,"have_strerror"     ,have_strerror    );
    output(os,keys,"have_strerror_r"   ,have_strerror_r  );
    output(os,keys,"have_strings_h"    ,have_strings_h   );
    output(os,keys,"have_strtol"       ,have_strtol      );
    output(os,keys,"have_mmap"         ,have_mmap        );
    output(os,keys,"have_munmap"       ,have_munmap      );
    output(os,keys,"have_sys_stat"     ,have_sys_stat    );
    output(os,keys,"have_timegm"       ,have_timegm      );
    output(os,keys,"have_unistd_h"     ,have_unistd_h    );
    output(os,keys,"have_sys_mman"     ,have_sys_mman    );
    output(os,keys,"have_libz"         ,have_libz        );
    output(os,keys,"have_xmptoolkit"   ,have_xmptoolkit  );
    output(os,keys,"have_bool"         ,have_bool        );
    output(os,keys,"have_strings"      ,have_strings     );
    output(os,keys,"have_sys_types"    ,have_sys_types   );
    output(os,keys,"have_unistd"       ,have_unistd      );
    output(os,keys,"have_unicode_path" ,have_unicode_path);
    output(os,keys,"enable_video"      ,enable_video     );
    output(os,keys,"enable_webready"   ,enable_webready  );

#if defined(__linux__)
    dlclose(ph);
    ph=NULL;
#endif

}
