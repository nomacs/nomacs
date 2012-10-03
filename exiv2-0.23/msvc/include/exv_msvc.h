/* ***************************************************************** -*- C -*- */
/*!
  @file    exv_msvc.h
  @brief   Configuration settings for MSVC
  @version $Rev: 2685 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    07-Feb-04, ahu: created
           26-Feb-05, ahu: renamed and moved to src directory
 */

/* Todo: The PACKAGE_* defines should be generated */

#ifdef _MSC_VER

#if _MSC_VER >= 1600
// stdint.h was introduced with DevStudio 2010
#define EXV_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <process.h> header file. */
#define EXV_HAVE_PROCESS_H 1

/* Define to the address where bug reports for this package should be sent. */
#define EXV_PACKAGE_BUGREPORT "ahuggel@gmx.net"

/* Define to the full name of this package. */
#define EXV_PACKAGE_NAME "exiv2"

/* Define to the full name and version of this package. */
#define EXV_PACKAGE_STRING "exiv2 0.23"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.23"

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

/* File path seperator */
#define EXV_SEPERATOR_STR "\\"
#define EXV_SEPERATOR_CHR '\\'

/* Windows unicode path support */
#define EXV_UNICODE_PATH

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

#endif /* _MSC_VER */
