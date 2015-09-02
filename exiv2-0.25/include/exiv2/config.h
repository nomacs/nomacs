// config.h
//
#ifdef _MSC_VER
# ifdef EXV_MSVC_CONFIGURE
#  include "exv_msvc_configure.h"
# else
#  include "exv_msvc.h"
# endif
#else
# include "exv_conf.h"
#endif

// Linux GCC 4.8 appears to be confused about strerror_r
#ifndef EXV_STRERROR_R_CHAR_P
#ifdef  __gnu_linux__
#define EXV_STRERROR_R_CHAR_P
#endif
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#ifndef __MING__
#define __MING__ 1
#endif
// Don't know why MinGW refuses to link libregex
#ifdef EXV_HAVE_REGEX
#undef EXV_HAVE_REGEX
#endif
#endif

#ifndef __CYGWIN__
#if defined(__CYGWIN32__) || defined(__CYGWIN64__)
#define __CYGWIN__ 1
#endif
#endif

#ifndef __LITTLE_ENDIAN__
#if    defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#if            __BYTE_ORDER__  ==         __ORDER_LITTLE_ENDIAN__
#define __LITTLE_ENDIAN__ 1
#endif
#endif
#endif

#ifndef __LITTLE_ENDIAN__
#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW__)
#define __LITTLE_ENDIAN__ 1
#endif
#endif

// That's all Folks!
//
