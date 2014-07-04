/*************************************************************************************************
Copyright Nikon Electronic Imaging Department - All rights reserved
*************************************************************************************************/

#ifndef	_NKTYPES_
#define	_NKTYPES_

#include <limits.h>

#ifndef _WIN32
	#include <string.h>
	#include <stdio.h>
#else
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#include <stdlib.h>
	#include <windows.h>
	#include <tchar.h>
#endif

typedef double	DOUB_P;
typedef	void*	LPVPTR;
typedef	unsigned short	UWORD;
typedef	short	SWORD;

#ifdef _WIN32
//	_TEXT(x) macro already defined
// _T(x) macro already defined
// LPCSTR typedef already defined
// LPCTSTR typedef already defined
// BYTE typedef already defined
// FAR macro already defined
	typedef	_TUCHAR	UCHAR;
	typedef	_TSCHAR	SCHAR;
	typedef	_TCHAR	CHAR;
#else
	#if !defined( FAR )
		#define  FAR
	#endif
	#define	_TEXT(x)		((char*)x)
//	#define	_T(x)			((char*)x)
	typedef	unsigned char	UCHAR;
	typedef	signed char		SCHAR;
	typedef	char			CHAR;
	typedef	unsigned char	BYTE;
	typedef	const UCHAR*	LPCSTR;
	typedef	const char*		LPCTSTR;
#endif


// Changed this from #ifndef BOOL to #ifndef WIN32 because of a bug in Visual C++ 2.2
// Changed it to a short integer type to retain compatibility with present Mac implementation
#ifndef __OBJC__ // TIK 2006.09.11 add for MD3Tester for mac.
    #ifndef _WIN32
        typedef	SWORD	BOOL;
    #endif
#endif

#ifndef ULONG
	typedef	unsigned long	ULONG;
#endif
	
#ifndef SLONG
	typedef	long	SLONG;
#endif

#ifndef NKPARAM
	typedef	ULONG	NKPARAM;
#endif

#ifndef LPVOID
	typedef	void FAR*	LPVOID;
#endif

#ifndef NKREF
	typedef	LPVOID	NKREF;
#endif

#ifndef __cplusplus // c++ quick and dirty fix

#ifndef bool
	typedef BOOL bool;
#endif

#ifndef false
	#define	false 0
#endif

#ifndef true
	#define	true !false
#endif

#ifndef FALSE
	#define	FALSE false
#endif

#ifndef TRUE
	#define	TRUE true
#endif

#endif

#define	kMaxULONG ULONG_MAX	// The maximum value of a ULONG type

// @func void * | NK_DEREF |
// This is how we do the first dereference of a handle.  Always use this macro if you want to
// retain cross-platform capability.
#ifdef _WIN32
	#define NK_DEREF(a)	((LPVPTR)a)	// Windows actually is a single dereference
#else
	// Macs use a double dereference
	#define NK_DEREF(a)	(StripAddress(*((Ptr*)(a))))
#endif

// make sure we know what NULL and nil mean
#ifndef NULL
	#define NULL	0
#endif

#ifndef nil
	#define nil		NULL
#endif

typedef SLONG	NKERROR;

#ifndef _WIN32
	typedef	struct FSSpec	NkFileID;
#else
	#ifdef __cplusplus
		class CNkString;
		typedef	CNkString	NkFileID;
	#else
		typedef	UCHAR*		NkFileID;
	#endif
#endif

#ifdef _WIN32
	#define _SPRINTF	_stprintf
	#define _STRCPY	_tcscpy
	#define _STRNCPY	_tcsncpy
	#define _STRLEN	_tcslen
	#define _STRCAT	_tcscat
#else
	#define _SPRINTF	sprintf
	#define _STRCPY	strcpy
	#define _STRNCPY	strncpy
	#define _STRLEN	strlen
	#define _STRCAT	strcat
#endif

// these macros are used in various places

#ifndef CLEARMEM
	#define CLEARMEM( dst, sz ) ::memset( (dst), 0, (sz) )
#endif

#ifndef MOVEMEM
	#define MOVEMEM( dst, src, sz ) ::memmove( (dst), (src), (sz) )
#endif

#ifndef NEW_INSTANCE
	#if defined( _WIN32 ) && defined( _DEBUG )
		// Add some debugging information to the new operator
		#ifndef _CRTDBG_MAP_ALLOC
			#define	_CRTDBG_MAP_ALLOC
		#endif
		#include <crtdbg.h>
		#define	NEW_INSTANCE		new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#else
		#define	NEW_INSTANCE		new
	#endif
#endif

#ifndef DELETE_INSTANCE
	#define DELETE_INSTANCE delete
#endif

#ifdef _WIN32
	// these are the members of the POINT structure
	#define POINTX x
	#define POINTY y
#else
	// these are the members of the POINT structure
	#define POINTX h
	#define POINTY v
#endif

#endif

//================================================================================================
