/* ***************************************************************** -*- C -*- */
/*!
  @file    exv_msvc.h
  @brief   Configuration settings for MSVC
  @version $Rev: 2320 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    07-Feb-04, ahu: created
 */

#ifdef  _MSC_VER
#ifndef _EXV_MSVC_H_
#define _EXV_MSVC_H_

#if _MSC_VER >= 1600
// stdint.h was introduced with DevStudio 2010
#define EXV_HAVE_STDINT_H 1
#endif
#if    _MSC_VER >= 1900
#define MSDEV_2014    1
#elif  _MSC_VER >= 1800
#define MSDEV_2013    1
#elif  _MSC_VER >= 1700
#define MSDEV_2012    1
#elif  _MSC_VER >= 1600
#define MSDEV_2010    1
#elif  _MSC_VER >= 1500
#define MSDEV_2008    1
#elif  _MSC_VER >= 1400
#define MSDEV_2005    1
#else
#define MSDEV_2003    1
#endif

#if MSDEV_2012
#define HAVE_NTOHLL 1
#endif

/* Define to 1 if you have the <regex.h> header file. */
// #define EXV_HAVE_REGEX 1

/* Define to 1 if you have the <process.h> header file. */
#define EXV_HAVE_PROCESS_H 1

/* Define to the address where bug reports for this package should be sent. */
#define EXV_PACKAGE_BUGREPORT "ahuggel@gmx.net"

/* Define to the full name of this package. */
#define EXV_PACKAGE_NAME "exiv2"

/* Define to the full name and version of this package. */
#define EXV_PACKAGE_STRING "exiv2 0.25"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.25"

/* Define to `int' if <sys/types.h> does not define pid_t. */
typedef int pid_t;

#ifndef EXV_COMMERCIAL_VERSION

/* Define to 1 to enable translation of Nikon lens names. */
# define EXV_HAVE_LENSDATA 1

/* Define to 1 if translation of program messages to the user's
   native language is requested. */
# undef EXV_ENABLE_NLS

#endif /* !EXV_COMMERCIAL_VERSION */

/* Define to 1 if you have the `iconv' function. */
# undef EXV_HAVE_ICONV

#ifdef  EXV_HAVE_ICONV
#define EXV_ICONV_CONST const
#endif

/* Define as 1 if you have the `zlib' library. (0 to omit zlib) [png support] */
#define HAVE_LIBZ 1

#if HAVE_LIBZ
  #define EXV_HAVE_LIBZ
  // assist VC7.1 to compile vsnprintf
  #if (_MSC_VER < 1400) && !defined(vsnprintf)
  #define vsnprintf _vsnprintf
  #endif
#endif

/* Define to 1 if you have the Adobe XMP Toolkit. */
#define EXV_HAVE_XMP_TOOLKIT 1

/* Define to 1 to build video support into the library */
#undef  EXV_ENABLE_VIDEO

/* Define to 1 to build webready support into the library */
#undef  EXV_ENABLE_WEBREADY

/* Define as 1 if you want to use curl to perform http I/O */
#ifndef EXV_USE_CURL
#define EXV_USE_CURL 0
#endif

/* Define as 1 if you want to use libssh (for ssh:// and sftp:// support */
#ifndef EXV_USE_SSH
#define EXV_USE_SSH 0
#endif

/* Help out our buddy curl */
#ifndef EXV_HAVE_DLL
#define CURL_STATICLIB
#endif

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")

/* File path separator */
#define EXV_SEPARATOR_STR "\\"
#define EXV_SEPARATOR_CHR '\\'

/* Windows unicode path support */
// #define EXV_UNICODE_PATH

/* Define to 1 if you have the `mmap' function. */
/* #undef EXV_HAVE_MMAP */

/* Define to 1 if you have the `munmap' function. */
/* #undef EXV_HAVE_MUNMAP */

/* Shared library support */
#ifdef  EXV_HAVE_DLL
#define EXV_IMPORT __declspec(dllimport)
#define EXV_EXPORT __declspec(dllexport)
#define EXV_DLLLOCAL
#define EXV_DLLPUBLIC
#else
#define EXV_IMPORT
#define EXV_EXPORT
#define EXV_DLLLOCAL
#define EXV_DLLPUBLIC
#define EXIV2API
#endif

/* Define EXIV2API for DLL builds */
#ifdef   EXV_HAVE_DLL
#  ifdef EXV_BUILDING_LIB
#    define EXIV2API EXV_EXPORT
#  else
#    define EXIV2API EXV_IMPORT
#  endif /* ! EXV_BUILDING_LIB */
#else
#  define EXIV2API
#endif /* ! EXV_HAVE_DLL */


/*
  Disable warning 4251.  This is warning from std templates about exporting interfaces
*/
#ifdef  EXV_HAVE_DLL
#pragma warning( disable : 4251 )
#endif
/*
  Visual Studio C++ 2005 (8.0)
  Disable warnings about 'deprecated' standard functions
  See, eg. http://www.informit.com/guides/content.asp?g=cplusplus&seqNum=259
*/
#if _MSC_VER >= 1400
# pragma warning(disable : 4996)
#endif

#endif /* _EXV_MSVC_H_ */
#endif /* _MSC_VER     */
