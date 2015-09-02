#ifndef __XMP_Environment_h__
#define __XMP_Environment_h__ 1

// =================================================================================================
// XMP_Environment.h - Build environment flags for the XMP toolkit.
// ================================================================
//
// This header is just C preprocessor macro definitions to set up the XMP toolkit build environment.
// It must be the first #include in any chain since it might affect things in other #includes.
//
// =================================================================================================

// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

// =================================================================================================
// Determine the Platform
// ======================

// One of MAC_ENV, WIN_ENV, or UNIX_ENV must be defined by the client. Since some other code
// requires these to be defined without values, they are only used here to define XMP-specific
// macros with 0 or 1 values.

/* 20-Oct-07, ahu: Determine the platform, set the above defines accordingly.                     */

#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif

#if defined __CYGWIN32__ && !defined __CYGWIN__
   /* For backwards compatibility with Cygwin b19 and
      earlier, we define __CYGWIN__ here, so that
      we can rely on checking just for that macro. */
# define __CYGWIN__  __CYGWIN32__
#endif
#if defined WIN32 && !defined __CYGWIN__
# define WIN_ENV 1
/* Todo: How to correctly recognize a Mac platform? */
#elif defined macintosh || defined MACOS_CLASSIC || defined MACOS_X_UNIX || defined MACOS_X || defined MACOS
# define MAC_ENV 1
#else 
# define UNIX_ENV 1
#endif

// ! Tempting though it might be to have a standard macro for big or little endian, there seems to
// ! be no decent way to do that on our own in UNIX. Forcing it on the client isn't acceptable.

#if defined ( MAC_ENV )

	#if 0	// ! maybe someday - ! MAC_ENV
		#error "MAC_ENV must be defined so that \"#if MAC_ENV\" is true"
	#endif
	
    #if defined ( WIN_ENV ) || defined ( UNIX_ENV )
        #error "XMP environment error - must define only one of MAC_ENV, WIN_ENV, or UNIX_ENV"
    #endif

    #define XMP_MacBuild  1
    #define XMP_WinBuild  0
    #define XMP_UNIXBuild 0

#elif defined ( WIN_ENV )

	#if 0	// ! maybe someday - ! WIN_ENV
		#error "WIN_ENV must be defined so that \"#if WIN_ENV\" is true"
	#endif
	
    #if defined ( UNIX_ENV )
        #error "XMP environment error - must define only one of MAC_ENV, WIN_ENV, or UNIX_ENV"
    #endif

    #define XMP_MacBuild  0
    #define XMP_WinBuild  1
    #define XMP_UNIXBuild 0

#elif defined ( UNIX_ENV )

	#if 0	// ! maybe someday - ! UNIX_ENV
		#error "UNIX_ENV must be defined so that \"#if UNIX_ENV\" is true"
	#endif
	
    #define XMP_MacBuild  0
    #define XMP_WinBuild  0
    #define XMP_UNIXBuild 1

#else

    #error "XMP environment error - must define one of MAC_ENV, WIN_ENV, or UNIX_ENV"

#endif

// =================================================================================================
// Common Macros
// =============

#if defined ( DEBUG )
    #if defined ( NDEBUG )
		#undef NDEBUG
		#warning
		#warning "XMP environment - DEBUG and NDEBUG defined.  NDEBUG has been undefined"
		#warning
    #endif
    #define XMP_DebugBuild 1
#endif

#if defined ( NDEBUG )
    #define XMP_DebugBuild 0
#endif

#ifndef XMP_DebugBuild
    #define XMP_DebugBuild 0
#endif

#if XMP_DebugBuild
    #include <stdio.h>  // The assert macro needs printf.
#endif

#ifndef XMP_64
	#if _WIN64
		#define XMP_64 1
	#else
		#define XMP_64 0
	#endif
#endif

#define UNUSED(x) (void)(x)

// =================================================================================================
// Macintosh Specific Settings
// ===========================

// =================================================================================================
// Windows Specific Settings
// =========================

// =================================================================================================
// UNIX Specific Settings
// ======================

// =================================================================================================

#endif  // __XMP_Environment_h__

/*
  If you're using Solaris and the Solaris Studio compiler, then you really
  do need to use -library=stdcxx4 along with these inclusions below
*/
#if defined(OS_SOLARIS)
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#if defined(__cplusplus)
#include <ios>
#include <fstream>
#endif
#endif

