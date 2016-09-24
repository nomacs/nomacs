// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:	Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"

#include "XMPUtils.hpp"

#include "MD5.h"

#include <map>

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>

#include <stdio.h>	// For snprintf.

#if XMP_WinBuild
#ifdef _MSC_VER
	#pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
	#pragma warning ( disable : 4996 )	// '...' was declared deprecated
#endif
#endif

// =================================================================================================
// Local Types and Constants
// =========================

static const char * sBase64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// =================================================================================================
// Static Variables
// ================

XMP_VarString * sComposedPath = 0;		// *** Only really need 1 string. Shrink periodically?
XMP_VarString * sConvertedValue = 0;
XMP_VarString * sBase64Str = 0;
XMP_VarString * sCatenatedItems = 0;
XMP_VarString * sStandardXMP = 0;
XMP_VarString * sExtendedXMP = 0;
XMP_VarString * sExtendedDigest = 0;

// =================================================================================================
// Local Utilities
// ===============


// -------------------------------------------------------------------------------------------------
// ANSI Time Functions
// -------------------
//
// A bit of hackery to use the best available time functions. Mac and UNIX have thread safe versions
// of gmtime and localtime. On Mac the CodeWarrior functions are buggy, use Apple's.

#if XMP_UNIXBuild

	typedef time_t			ansi_tt;
	typedef struct tm		ansi_tm;

	#define ansi_time		time
	#define ansi_mktime		mktime
	#define ansi_difftime	difftime

	#define ansi_gmtime		gmtime_r
	#define ansi_localtime	localtime_r

#elif XMP_WinBuild

	// ! VS.Net 2003 (VC7) does not provide thread safe versions of gmtime and localtime.
	// ! VS.Net 2005 (VC8) inverts the parameters for the safe versions of gmtime and localtime.

	typedef time_t			ansi_tt;
	typedef struct tm		ansi_tm;

	#define ansi_time		time
	#define ansi_mktime		mktime
	#define ansi_difftime	difftime

	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#define ansi_gmtime(tt,tm)		gmtime_s ( tm, tt )
		#define ansi_localtime(tt,tm)	localtime_s ( tm, tt )
	#else
		static inline void ansi_gmtime ( const ansi_tt * ttTime, ansi_tm * tmTime )
		{
			ansi_tm * tmx = gmtime ( ttTime );	// ! Hope that there is no race!
			if ( tmx == 0 ) XMP_Throw ( "Failure from ANSI C gmtime function", kXMPErr_ExternalFailure );
			*tmTime = *tmx;
		}
		static inline void ansi_localtime ( const ansi_tt * ttTime, ansi_tm * tmTime )
		{
			ansi_tm * tmx = localtime ( ttTime );	// ! Hope that there is no race!
			if ( tmx == 0 ) XMP_Throw ( "Failure from ANSI C localtime function", kXMPErr_ExternalFailure );
			*tmTime = *tmx;
		}
	#endif

#elif XMP_MacBuild

	#if ! __MWERKS__

		typedef time_t			ansi_tt;
		typedef struct tm		ansi_tm;

		#define ansi_time		time
		#define ansi_mktime		mktime
		#define ansi_difftime	difftime

		#define ansi_gmtime		gmtime_r
		#define ansi_localtime	localtime_r

	#else

		// ! The CW versions are buggy. Use Apple's code, time_t, and "struct tm".

		#include <mach-o/dyld.h>

		typedef _BSD_TIME_T_	ansi_tt;

		typedef struct apple_tm {
			int tm_sec;		/* seconds after the minute [0-60] */
			int tm_min;		/* minutes after the hour [0-59] */
			int tm_hour;	/* hours since midnight [0-23] */
			int tm_mday;	/* day of the month [1-31] */
			int tm_mon;		/* months since January [0-11] */
			int tm_year;	/* years since 1900 */
			int tm_wday;	/* days since Sunday [0-6] */
			int tm_yday;	/* days since January 1 [0-365] */
			int tm_isdst;	/* Daylight Savings Time flag */
			long	tm_gmtoff;	/* offset from CUT in seconds */
			char	*tm_zone;	/* timezone abbreviation */
		} ansi_tm;


		typedef ansi_tt (* GetTimeProc)	 ( ansi_tt * ttTime );
		typedef ansi_tt (* MakeTimeProc) ( ansi_tm * tmTime );
		typedef double	(* DiffTimeProc) ( ansi_tt t1, ansi_tt t0 );

		typedef void (* ConvertTimeProc) ( const ansi_tt * ttTime, ansi_tm * tmTime );

		static GetTimeProc ansi_time = 0;
		static MakeTimeProc ansi_mktime = 0;
		static DiffTimeProc ansi_difftime = 0;

		static ConvertTimeProc ansi_gmtime = 0;
		static ConvertTimeProc ansi_localtime = 0;

		static void LookupTimeProcs()
		{
			_dyld_lookup_and_bind_with_hint ( "_time", "libSystem", (XMP_Uns32*)&ansi_time, 0 );
			_dyld_lookup_and_bind_with_hint ( "_mktime", "libSystem", (XMP_Uns32*)&ansi_mktime, 0 );
			_dyld_lookup_and_bind_with_hint ( "_difftime", "libSystem", (XMP_Uns32*)&ansi_difftime, 0 );
			_dyld_lookup_and_bind_with_hint ( "_gmtime_r", "libSystem", (XMP_Uns32*)&ansi_gmtime, 0 );
			_dyld_lookup_and_bind_with_hint ( "_localtime_r", "libSystem", (XMP_Uns32*)&ansi_localtime, 0 );
		}

	#endif

#endif


// -------------------------------------------------------------------------------------------------
// IsLeapYear
// ----------

static bool
IsLeapYear ( long year )
{

	if ( year < 0 ) year = -year + 1;		// Fold the negative years, assuming there is a year 0.

	if ( (year % 4) != 0 ) return false;	// Not a multiple of 4.
	if ( (year % 100) != 0 ) return true;	// A multiple of 4 but not a multiple of 100.
	if ( (year % 400) == 0 ) return true;	// A multiple of 400.

	return false;							// A multiple of 100 but not a multiple of 400.

}	// IsLeapYear


// -------------------------------------------------------------------------------------------------
// DaysInMonth
// -----------

static int
DaysInMonth ( XMP_Int32 year, XMP_Int32 month )
{

	static short	daysInMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
									   // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec

	int days = daysInMonth [ month ];
	if ( (month == 2) && IsLeapYear ( year ) ) days += 1;

	return days;

}	// DaysInMonth


// -------------------------------------------------------------------------------------------------
// AdjustTimeOverflow
// ------------------

static void
AdjustTimeOverflow ( XMP_DateTime * time )
{
	enum { kBillion = 1000*1000*1000L };

	// ----------------------------------------------------------------------------------------------
	// To be safe against pathalogical overflow we first adjust from month to second, then from
	// nanosecond back up to month. This leaves each value closer to zero before propagating into it.
	// For example if the hour and minute are both near max, adjusting minutes first can cause the
	// hour to overflow.

	// ! Photoshop 8 creates "time only" values with zeros for year, month, and day.

	if ( (time->year != 0) || (time->month != 0) || (time->day != 0) ) {

		while ( time->month < 1 ) {
			time->year -= 1;
			time->month += 12;
		}

		while ( time->month > 12 ) {
			time->year += 1;
			time->month -= 12;
		}

		while ( time->day < 1 ) {
			time->month -= 1;
			if ( time->month < 1 ) {	// ! Keep the months in range for indexing daysInMonth!
				time->year -= 1;
				time->month += 12;
			}
			time->day += DaysInMonth ( time->year, time->month );	// ! Decrement month before so index here is right!
		}

		while ( time->day > DaysInMonth ( time->year, time->month ) ) {
			time->day -= DaysInMonth ( time->year, time->month );	// ! Increment month after so index here is right!
			time->month += 1;
			if ( time->month > 12 ) {
				time->year += 1;
				time->month -= 12;
			}
		}

	}

	while ( time->hour < 0 ) {
		time->day -= 1;
		time->hour += 24;
	}

	while ( time->hour >= 24 ) {
		time->day += 1;
		time->hour -= 24;
	}

	while ( time->minute < 0 ) {
		time->hour -= 1;
		time->minute += 60;
	}

	while ( time->minute >= 60 ) {
		time->hour += 1;
		time->minute -= 60;
	}

	while ( time->second < 0 ) {
		time->minute -= 1;
		time->second += 60;
	}

	while ( time->second >= 60 ) {
		time->minute += 1;
		time->second -= 60;
	}

	while ( time->nanoSecond < 0 ) {
		time->second -= 1;
		time->nanoSecond += kBillion;
	}

	while ( time->nanoSecond >= kBillion ) {
		time->second += 1;
		time->nanoSecond -= kBillion;
	}

	while ( time->second < 0 ) {
		time->minute -= 1;
		time->second += 60;
	}

	while ( time->second >= 60 ) {
		time->minute += 1;
		time->second -= 60;
	}

	while ( time->minute < 0 ) {
		time->hour -= 1;
		time->minute += 60;
	}

	while ( time->minute >= 60 ) {
		time->hour += 1;
		time->minute -= 60;
	}

	while ( time->hour < 0 ) {
		time->day -= 1;
		time->hour += 24;
	}

	while ( time->hour >= 24 ) {
		time->day += 1;
		time->hour -= 24;
	}

	if ( (time->year != 0) || (time->month != 0) || (time->day != 0) ) {

		while ( time->month < 1 ) { // Make sure the months are OK first, for DaysInMonth.
			time->year -= 1;
			time->month += 12;
		}

		while ( time->month > 12 ) {
			time->year += 1;
			time->month -= 12;
		}

		while ( time->day < 1 ) {
			time->month -= 1;
			if ( time->month < 1 ) {
				time->year -= 1;
				time->month += 12;
			}
			time->day += DaysInMonth ( time->year, time->month );
		}

		while ( time->day > DaysInMonth ( time->year, time->month ) ) {
			time->day -= DaysInMonth ( time->year, time->month );
			time->month += 1;
			if ( time->month > 12 ) {
				time->year += 1;
				time->month -= 12;
			}
		}

	}

}	// AdjustTimeOverflow


// -------------------------------------------------------------------------------------------------
// GatherInt
// ---------

static XMP_Int32
GatherInt ( XMP_StringPtr strValue, size_t * _pos, const char * errMsg )
{
	size_t	 pos   = *_pos;
	XMP_Int32 value = 0;

	for ( char ch = strValue[pos]; ('0' <= ch) && (ch <= '9'); ++pos, ch = strValue[pos] ) {
		value = (value * 10) + (ch - '0');
	}

	if ( pos == *_pos ) XMP_Throw ( errMsg, kXMPErr_BadParam );
	*_pos = pos;
	return value;

}	// GatherInt


// -------------------------------------------------------------------------------------------------

static void FormatFullDateTime ( XMP_DateTime & tempDate, char * buffer, size_t bufferLen )
{

	AdjustTimeOverflow ( &tempDate );	// Make sure all time parts are in range.

	if ( (tempDate.second == 0) && (tempDate.nanoSecond == 0) ) {

		// Output YYYY-MM-DDThh:mmTZD.
		snprintf ( buffer, bufferLen, "%.4d-%02d-%02dT%02d:%02d",	// AUDIT: Callers pass sizeof(buffer).
                           static_cast<int>(tempDate.year), static_cast<int>(tempDate.month), static_cast<int>(tempDate.day), static_cast<int>(tempDate.hour), static_cast<int>(tempDate.minute) );

	} else if ( tempDate.nanoSecond == 0  ) {

		// Output YYYY-MM-DDThh:mm:ssTZD.
		snprintf ( buffer, bufferLen, "%.4d-%02d-%02dT%02d:%02d:%02d",	// AUDIT: Callers pass sizeof(buffer).
                           static_cast<int>(tempDate.year), static_cast<int>(tempDate.month), static_cast<int>(tempDate.day),
                           static_cast<int>(tempDate.hour), static_cast<int>(tempDate.minute), static_cast<int>(tempDate.second) );

	} else {

		// Output YYYY-MM-DDThh:mm:ss.sTZD.
		snprintf ( buffer, bufferLen, "%.4d-%02d-%02dT%02d:%02d:%02d.%09d", // AUDIT: Callers pass sizeof(buffer).
                           static_cast<int>(tempDate.year), static_cast<int>(tempDate.month), static_cast<int>(tempDate.day),
                           static_cast<int>(tempDate.hour), static_cast<int>(tempDate.minute), static_cast<int>(tempDate.second), static_cast<int>(tempDate.nanoSecond) );
		for ( size_t i = strlen(buffer)-1; buffer[i] == '0'; --i ) buffer[i] = 0;	// Trim excess digits.

	}

}	// FormatFullDateTime


// -------------------------------------------------------------------------------------------------
// DecodeBase64Char
// ----------------

// The decode mapping:
//
//	encoded		encoded			raw
//	char		value			value
//	-------		-------			-----
//	A .. Z		0x41 .. 0x5A	 0 .. 25
//	a .. z		0x61 .. 0x7A	26 .. 51
//	0 .. 9		0x30 .. 0x39	52 .. 61
//	+			0x2B			62
//	/			0x2F			63

static unsigned char
DecodeBase64Char ( XMP_Uns8 ch )
{

	if ( ('A' <= ch) && (ch <= 'Z') ) {
		ch = ch - 'A';
	} else if ( ('a' <= ch) && (ch <= 'z') ) {
		ch = ch - 'a' + 26;
	} else if ( ('0' <= ch) && (ch <= '9') ) {
		ch = ch - '0' + 52;
	} else if ( ch == '+' ) {
		ch = 62;
	} else if ( ch == '/' ) {
		ch = 63;
	} else if ( (ch == ' ') || (ch == kTab) || (ch == kLF) || (ch == kCR) ) {
		ch = 0xFF;	// Will be ignored by the caller.
	} else {
		XMP_Throw ( "Invalid base-64 encoded character", kXMPErr_BadParam );
	}

	return ch;

}	// DecodeBase64Char ();


// -------------------------------------------------------------------------------------------------
// EstimateSizeForJPEG
// -------------------
//
// Estimate the serialized size for the subtree of an XMP_Node. Support for PackageForJPEG.

static size_t
EstimateSizeForJPEG ( const XMP_Node * xmpNode )
{

	size_t estSize = 0;
	size_t nameSize = xmpNode->name.size();
	bool   includeName = (! XMP_PropIsArray ( xmpNode->parent->options ));

	if ( XMP_PropIsSimple ( xmpNode->options ) ) {

		if ( includeName ) estSize += (nameSize + 3);	// Assume attribute form.
		estSize += xmpNode->value.size();

	} else if ( XMP_PropIsArray ( xmpNode->options ) ) {

		// The form of the value portion is: <rdf:Xyz><rdf:li>...</rdf:li>...</rdf:Xyx>
		if ( includeName ) estSize += (2*nameSize + 5);
		size_t arraySize = xmpNode->children.size();
		estSize += 9 + 10;	// The rdf:Xyz tags.
		estSize += arraySize * (8 + 9);	// The rdf:li tags.
		for ( size_t i = 0; i < arraySize; ++i ) {
			estSize += EstimateSizeForJPEG ( xmpNode->children[i] );
		}

	} else {

		// The form is: <headTag rdf:parseType="Resource">...fields...</tailTag>
		if ( includeName ) estSize += (2*nameSize + 5);
		estSize += 25;	// The rdf:parseType="Resource" attribute.
		size_t fieldCount = xmpNode->children.size();
		for ( size_t i = 0; i < fieldCount; ++i ) {
			estSize += EstimateSizeForJPEG ( xmpNode->children[i] );
		}

	}

	return estSize;

}	// EstimateSizeForJPEG


// -------------------------------------------------------------------------------------------------
// MoveOneProperty
// ---------------

static bool MoveOneProperty ( XMPMeta & stdXMP, XMPMeta * extXMP,
							  XMP_StringPtr schemaURI, XMP_StringPtr propName )
{

	XMP_Node * propNode = 0;
	XMP_NodePtrPos stdPropPos;

	XMP_Node * stdSchema = FindSchemaNode ( &stdXMP.tree, schemaURI, kXMP_ExistingOnly, 0 );
	if ( stdSchema != 0 ) {
		propNode = FindChildNode ( stdSchema, propName, kXMP_ExistingOnly, &stdPropPos );
	}
	if ( propNode == 0 ) return false;

	XMP_Node * extSchema = FindSchemaNode ( &extXMP->tree, schemaURI, kXMP_CreateNodes );

	propNode->parent = extSchema;

	extSchema->options &= ~kXMP_NewImplicitNode;
	extSchema->children.push_back ( propNode );

	stdSchema->children.erase ( stdPropPos );
	DeleteEmptySchema ( stdSchema );

	return true;

}	// MoveOneProperty


// -------------------------------------------------------------------------------------------------
// CreateEstimatedSizeMap
// ----------------------

#ifndef Trace_PackageForJPEG
	#define Trace_PackageForJPEG 0
#endif

typedef std::pair < XMP_VarString*, XMP_VarString* > StringPtrPair;
typedef std::multimap < size_t, StringPtrPair > PropSizeMap;

static void CreateEstimatedSizeMap ( XMPMeta & stdXMP, PropSizeMap * propSizes )
{
	#if Trace_PackageForJPEG
		printf ( "  Creating top level property map:\n" );
	#endif

	for ( size_t s = stdXMP.tree.children.size(); s > 0; --s ) {

		XMP_Node * stdSchema = stdXMP.tree.children[s-1];

		for ( size_t p = stdSchema->children.size(); p > 0; --p ) {

			XMP_Node * stdProp = stdSchema->children[p-1];
			if ( (stdSchema->name == kXMP_NS_XMP_Note) &&
				 (stdProp->name == "xmpNote:HasExtendedXMP") ) continue;	// ! Don't move xmpNote:HasExtendedXMP.

			size_t propSize = EstimateSizeForJPEG ( stdProp );
			StringPtrPair namePair ( &stdSchema->name, &stdProp->name );
			PropSizeMap::value_type mapValue ( propSize, namePair );

			(void) propSizes->insert ( propSizes->upper_bound ( propSize ), mapValue );
			#if Trace_PackageForJPEG
				printf ( "    %d bytes, %s in %s\n", propSize, stdProp->name.c_str(), stdSchema->name.c_str() );
			#endif

		}

	}

}	// CreateEstimatedSizeMap


// -------------------------------------------------------------------------------------------------
// MoveLargestProperty
// -------------------

static size_t MoveLargestProperty ( XMPMeta & stdXMP, XMPMeta * extXMP, PropSizeMap & propSizes )
{
	XMP_Assert ( ! propSizes.empty() );

	#if 0
		// *** Xcode 2.3 on Mac OS X 10.4.7 seems to have a bug where this does not pick the last
		// *** item in the map. We'll just avoid it on all platforms until thoroughly tested.
		PropSizeMap::iterator lastPos = propSizes.end();
		--lastPos;	// Move to the actual last item.
	#else
		PropSizeMap::iterator lastPos = propSizes.begin();
		PropSizeMap::iterator nextPos = lastPos;
		for ( ++nextPos; nextPos != propSizes.end(); ++nextPos ) lastPos = nextPos;
	#endif

	size_t propSize = lastPos->first;
	const char * schemaURI = lastPos->second.first->c_str();
	const char * propName  = lastPos->second.second->c_str();

	#if Trace_PackageForJPEG
		printf ( "  Move %s, %d bytes\n", propName, propSize );
	#endif

    bool moved = MoveOneProperty ( stdXMP, extXMP, schemaURI, propName );
	XMP_Assert ( moved );
	UNUSED(moved);

	propSizes.erase ( lastPos );
	return propSize;

}	// MoveLargestProperty


// =================================================================================================
// Class Static Functions
// ======================


// -------------------------------------------------------------------------------------------------
// Initialize
// ----------

/* class static */ bool
XMPUtils::Initialize()
{
	sComposedPath	= new XMP_VarString();
	sConvertedValue = new XMP_VarString();
	sBase64Str		= new XMP_VarString();
	sCatenatedItems = new XMP_VarString();
	sStandardXMP    = new XMP_VarString();
	sExtendedXMP    = new XMP_VarString();
	sExtendedDigest = new XMP_VarString();

	#if XMP_MacBuild && __MWERKS__
		LookupTimeProcs();
	#endif

	return true;

}	// Initialize


// -------------------------------------------------------------------------------------------------
// Terminate
// ---------

#define EliminateGlobal(g) delete ( g ); g = 0

/* class static */ void
XMPUtils::Terminate() RELEASE_NO_THROW
{
	EliminateGlobal ( sComposedPath );
	EliminateGlobal ( sConvertedValue );
	EliminateGlobal ( sBase64Str );
	EliminateGlobal ( sCatenatedItems );
	EliminateGlobal ( sStandardXMP );
	EliminateGlobal ( sExtendedXMP );
	EliminateGlobal ( sExtendedDigest );

	return;

}	// Terminate


// -------------------------------------------------------------------------------------------------
// Unlock
// ------

/* class static */ void
XMPUtils::Unlock ( XMP_OptionBits options )
{
	UNUSED(options);

	XMPMeta::Unlock ( 0 );

}	// Unlock

// -------------------------------------------------------------------------------------------------
// ComposeArrayItemPath
// --------------------
//
// Return "arrayName[index]".

/* class static */ void
XMPUtils::ComposeArrayItemPath ( XMP_StringPtr	 schemaNS,
								 XMP_StringPtr	 arrayName,
								 XMP_Index		 itemIndex,
								 XMP_StringPtr * fullPath,
								 XMP_StringLen * pathSize )
{
	XMP_Assert ( schemaNS != 0 );	// Enforced by wrapper.
	XMP_Assert ( *arrayName != 0 ); // Enforced by wrapper.
	XMP_Assert ( (fullPath != 0) && (pathSize != 0) );	// Enforced by wrapper.

	XMP_ExpandedXPath expPath;	// Just for side effects to check namespace and basic path.
	ExpandXPath ( schemaNS, arrayName, &expPath );

	if ( (itemIndex < 0) && (itemIndex != kXMP_ArrayLastItem) ) XMP_Throw ( "Array index out of bounds", kXMPErr_BadParam );

	XMP_StringLen reserveLen = strlen(arrayName) + 2 + 32;	// Room plus padding.

	sComposedPath->erase();
	sComposedPath->reserve ( reserveLen );
	sComposedPath->append ( reserveLen, ' ' );

	if ( itemIndex != kXMP_ArrayLastItem ) {
		// AUDIT: Using string->size() for the snprintf length is safe.
                snprintf ( const_cast<char*>(sComposedPath->c_str()), sComposedPath->size(), "%s[%d]", arrayName, static_cast<int>(itemIndex) );
	} else {
		*sComposedPath = arrayName;
		*sComposedPath += "[last()] ";
		(*sComposedPath)[sComposedPath->size()-1] = 0;	// ! Final null is for the strlen at exit.
	}

	*fullPath = sComposedPath->c_str();
	*pathSize = strlen ( *fullPath );	// ! Don't use sComposedPath->size()!

	XMP_Enforce ( *pathSize < sComposedPath->size() );	// Rather late, but complain about buffer overflow.

}	// ComposeArrayItemPath


// -------------------------------------------------------------------------------------------------
// ComposeStructFieldPath
// ----------------------
//
// Return "structName/ns:fieldName".

/* class static */ void
XMPUtils::ComposeStructFieldPath ( XMP_StringPtr   schemaNS,
								   XMP_StringPtr   structName,
								   XMP_StringPtr   fieldNS,
								   XMP_StringPtr   fieldName,
								   XMP_StringPtr * fullPath,
								   XMP_StringLen * pathSize )
{
	XMP_Assert ( (schemaNS != 0) && (fieldNS != 0) );		// Enforced by wrapper.
	XMP_Assert ( (*structName != 0) && (*fieldName != 0) ); // Enforced by wrapper.
	XMP_Assert ( (fullPath != 0) && (pathSize != 0) );		// Enforced by wrapper.

	XMP_ExpandedXPath expPath;	// Just for side effects to check namespace and basic path.
	ExpandXPath ( schemaNS, structName, &expPath );

	XMP_ExpandedXPath fieldPath;
	ExpandXPath ( fieldNS, fieldName, &fieldPath );
	if ( fieldPath.size() != 2 ) XMP_Throw ( "The fieldName must be simple", kXMPErr_BadXPath );

	XMP_StringLen reserveLen = strlen(structName) + fieldPath[kRootPropStep].step.size() + 1;

	sComposedPath->erase();
	sComposedPath->reserve ( reserveLen );
	*sComposedPath = structName;
	*sComposedPath += '/';
	*sComposedPath += fieldPath[kRootPropStep].step;

	*fullPath = sComposedPath->c_str();
	*pathSize = sComposedPath->size();

}	// ComposeStructFieldPath


// -------------------------------------------------------------------------------------------------
// ComposeQualifierPath
// --------------------
//
// Return "propName/?ns:qualName".

/* class static */ void
XMPUtils::ComposeQualifierPath ( XMP_StringPtr	 schemaNS,
								 XMP_StringPtr	 propName,
								 XMP_StringPtr	 qualNS,
								 XMP_StringPtr	 qualName,
								 XMP_StringPtr * fullPath,
								 XMP_StringLen * pathSize )
{
	XMP_Assert ( (schemaNS != 0) && (qualNS != 0) );		// Enforced by wrapper.
	XMP_Assert ( (*propName != 0) && (*qualName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (fullPath != 0) && (pathSize != 0) );		// Enforced by wrapper.

	XMP_ExpandedXPath expPath;	// Just for side effects to check namespace and basic path.
	ExpandXPath ( schemaNS, propName, &expPath );

	XMP_ExpandedXPath qualPath;
	ExpandXPath ( qualNS, qualName, &qualPath );
	if ( qualPath.size() != 2 ) XMP_Throw ( "The qualifier name must be simple", kXMPErr_BadXPath );

	XMP_StringLen reserveLen = strlen(propName) + qualPath[kRootPropStep].step.size() + 2;

	sComposedPath->erase();
	sComposedPath->reserve ( reserveLen );
	*sComposedPath = propName;
	*sComposedPath += "/?";
	*sComposedPath += qualPath[kRootPropStep].step;

	*fullPath = sComposedPath->c_str();
	*pathSize = sComposedPath->size();

}	// ComposeQualifierPath


// -------------------------------------------------------------------------------------------------
// ComposeLangSelector
// -------------------
//
// Return "arrayName[?xml:lang="lang"]".

// *** #error "handle quotes in the lang - or verify format"

/* class static */ void
XMPUtils::ComposeLangSelector ( XMP_StringPtr	schemaNS,
								XMP_StringPtr	arrayName,
								XMP_StringPtr	_langName,
								XMP_StringPtr * fullPath,
								XMP_StringLen * pathSize )
{
	XMP_Assert ( schemaNS != 0 );	// Enforced by wrapper.
	XMP_Assert ( (*arrayName != 0) && (*_langName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (fullPath != 0) && (pathSize != 0) );		// Enforced by wrapper.

	XMP_ExpandedXPath expPath;	// Just for side effects to check namespace and basic path.
	ExpandXPath ( schemaNS, arrayName, &expPath );

	XMP_VarString langName ( _langName );
	NormalizeLangValue ( &langName );

	XMP_StringLen reserveLen = strlen(arrayName) + langName.size() + 14;

	sComposedPath->erase();
	sComposedPath->reserve ( reserveLen );
	*sComposedPath = arrayName;
	*sComposedPath += "[?xml:lang=\"";
	*sComposedPath += langName;
	*sComposedPath += "\"]";

	*fullPath = sComposedPath->c_str();
	*pathSize = sComposedPath->size();

}	// ComposeLangSelector


// -------------------------------------------------------------------------------------------------
// ComposeFieldSelector
// --------------------
//
// Return "arrayName[ns:fieldName="fieldValue"]".

// *** #error "handle quotes in the value"

/* class static */ void
XMPUtils::ComposeFieldSelector ( XMP_StringPtr	 schemaNS,
								 XMP_StringPtr	 arrayName,
								 XMP_StringPtr	 fieldNS,
								 XMP_StringPtr	 fieldName,
								 XMP_StringPtr	 fieldValue,
								 XMP_StringPtr * fullPath,
								 XMP_StringLen * pathSize )
{
	XMP_Assert ( (schemaNS != 0) && (fieldNS != 0) && (fieldValue != 0) );	// Enforced by wrapper.
	XMP_Assert ( (*arrayName != 0) && (*fieldName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (fullPath != 0) && (pathSize != 0) );		// Enforced by wrapper.

	XMP_ExpandedXPath expPath;	// Just for side effects to check namespace and basic path.
	ExpandXPath ( schemaNS, arrayName, &expPath );

	XMP_ExpandedXPath fieldPath;
	ExpandXPath ( fieldNS, fieldName, &fieldPath );
	if ( fieldPath.size() != 2 ) XMP_Throw ( "The fieldName must be simple", kXMPErr_BadXPath );

	XMP_StringLen reserveLen = strlen(arrayName) + fieldPath[kRootPropStep].step.size() + strlen(fieldValue) + 5;

	sComposedPath->erase();
	sComposedPath->reserve ( reserveLen );
	*sComposedPath = arrayName;
	*sComposedPath += '[';
	*sComposedPath += fieldPath[kRootPropStep].step;
	*sComposedPath += "=\"";
	*sComposedPath += fieldValue;
	*sComposedPath += "\"]";

	*fullPath = sComposedPath->c_str();
	*pathSize = sComposedPath->size();

}	// ComposeFieldSelector


// -------------------------------------------------------------------------------------------------
// ConvertFromBool
// ---------------

/* class static */ void
XMPUtils::ConvertFromBool ( bool			binValue,
							XMP_StringPtr * strValue,
							XMP_StringLen * strSize )
{
	XMP_Assert ( (strValue != 0) && (strSize != 0) );	// Enforced by wrapper.

	if ( binValue ) {
		*strValue = kXMP_TrueStr;
		*strSize  = strlen ( kXMP_TrueStr );
	} else {
		*strValue = kXMP_FalseStr;
		*strSize  = strlen ( kXMP_FalseStr );
	}

}	// ConvertFromBool


// -------------------------------------------------------------------------------------------------
// ConvertFromInt
// --------------

/* class static */ void
XMPUtils::ConvertFromInt ( XMP_Int32	   binValue,
						   XMP_StringPtr   format,
						   XMP_StringPtr * strValue,
						   XMP_StringLen * strSize )
{
	XMP_Assert ( (format != 0) && (strValue != 0) && (strSize != 0) );	// Enforced by wrapper.

	if ( *format == 0 ) format = "%d";

	sConvertedValue->erase();
	sConvertedValue->reserve ( 100 );		// More than enough for any reasonable format and value.
	sConvertedValue->append ( 100, ' ' );

	// AUDIT: Using string->size() for the snprintf length is safe.
	snprintf ( const_cast<char*>(sConvertedValue->c_str()), sConvertedValue->size(), format, binValue );

	*strValue = sConvertedValue->c_str();
	*strSize  = strlen ( *strValue );	// ! Don't use sConvertedValue->size()!

	XMP_Enforce ( *strSize < sConvertedValue->size() ); // Rather late, but complain about buffer overflow.

}	// ConvertFromInt


// -------------------------------------------------------------------------------------------------
// ConvertFromInt64
// ----------------

/* class static */ void
XMPUtils::ConvertFromInt64 ( XMP_Int64	     binValue,
						     XMP_StringPtr   format,
						     XMP_StringPtr * strValue,
						     XMP_StringLen * strSize )
{
	XMP_Assert ( (format != 0) && (strValue != 0) && (strSize != 0) );	// Enforced by wrapper.

	if ( *format == 0 ) format = "%lld";

	sConvertedValue->erase();
	sConvertedValue->reserve ( 100 );		// More than enough for any reasonable format and value.
	sConvertedValue->append ( 100, ' ' );

	// AUDIT: Using string->size() for the snprintf length is safe.
	snprintf ( const_cast<char*>(sConvertedValue->c_str()), sConvertedValue->size(), format, binValue );

	*strValue = sConvertedValue->c_str();
	*strSize  = strlen ( *strValue );	// ! Don't use sConvertedValue->size()!

	XMP_Enforce ( *strSize < sConvertedValue->size() ); // Rather late, but complain about buffer overflow.

}	// ConvertFromInt64


// -------------------------------------------------------------------------------------------------
// ConvertFromFloat
// ----------------

/* class static */ void
XMPUtils::ConvertFromFloat ( double			 binValue,
							 XMP_StringPtr	 format,
							 XMP_StringPtr * strValue,
							 XMP_StringLen * strSize )
{
	XMP_Assert ( (format != 0) && (strValue != 0) && (strSize != 0) );	// Enforced by wrapper.

	if ( *format == 0 ) format = "%f";

	sConvertedValue->erase();
	sConvertedValue->reserve ( 1000 );		// More than enough for any reasonable format and value.
	sConvertedValue->append ( 1000, ' ' );

	// AUDIT: Using string->size() for the snprintf length is safe.
	snprintf ( const_cast<char*>(sConvertedValue->c_str()), sConvertedValue->size(), format, binValue );

	*strValue = sConvertedValue->c_str();
	*strSize  = strlen ( *strValue );	// ! Don't use sConvertedValue->size()!

	XMP_Enforce ( *strSize < sConvertedValue->size() ); // Rather late, but complain about buffer overflow.

}	// ConvertFromFloat


// -------------------------------------------------------------------------------------------------
// ConvertFromDate
// ---------------
//
// Format a date according to ISO 8601 and http://www.w3.org/TR/NOTE-datetime:
//	YYYY
//	YYYY-MM
//	YYYY-MM-DD
//	YYYY-MM-DDThh:mmTZD
//	YYYY-MM-DDThh:mm:ssTZD
//	YYYY-MM-DDThh:mm:ss.sTZD
//
//	YYYY = four-digit year
//	MM	 = two-digit month (01=January, etc.)
//	DD	 = two-digit day of month (01 through 31)
//	hh	 = two digits of hour (00 through 23)
//	mm	 = two digits of minute (00 through 59)
//	ss	 = two digits of second (00 through 59)
//	s	 = one or more digits representing a decimal fraction of a second
//	TZD	 = time zone designator (Z or +hh:mm or -hh:mm)
//
// Note that ISO 8601 does not seem to allow years less than 1000 or greater than 9999. We allow
// any year, even negative ones. The year is formatted as "%.4d".

// *** Need to check backward compatibility for partial forms!

/* class static */ void
XMPUtils::ConvertFromDate ( const XMP_DateTime & binValue,
							XMP_StringPtr *		 strValue,
							XMP_StringLen *		 strSize )
{
	XMP_Assert ( (strValue != 0) && (strSize != 0) );	// Enforced by wrapper.

	bool addTimeZone = false;
	char buffer [100];	// Plenty long enough.

	// Pick the format, use snprintf to format into a local buffer, assign to static output string.
	// Don't use AdjustTimeOverflow at the start, that will wipe out zero month or day values.

	// ! Photoshop 8 creates "time only" values with zeros for year, month, and day.

	XMP_DateTime tempDate = binValue;

	// Temporary fix for bug 1269463, silently fix out of range month or day.

	bool haveDay  = (tempDate.day != 0);
	bool haveTime = ( (tempDate.hour != 0)   || (tempDate.minute != 0) ||
			          (tempDate.second != 0) || (tempDate.nanoSecond != 0) ||
			          (tempDate.tzSign != 0) || (tempDate.tzHour != 0) || (tempDate.tzMinute != 0) );

	if ( tempDate.month == 0 ) {
		if ( haveDay || haveTime ) tempDate.month = 1;
	} else {
		if ( tempDate.month < 1 ) tempDate.month = 1;
		if ( tempDate.month > 12 ) tempDate.month = 12;
	}

	if ( tempDate.day == 0 ) {
		if ( haveTime ) tempDate.day = 1;
	} else {
		if ( tempDate.day < 1 ) tempDate.day = 1;
		if ( tempDate.day > 31 ) tempDate.day = 31;
	}

	// Now carry on with the original logic.

	if ( tempDate.month == 0 ) {

		// Output YYYY if all else is zero, otherwise output a full string for the quasi-bogus
		// "time only" values from Photoshop CS.
		if ( (tempDate.day == 0) && (tempDate.hour == 0) && (tempDate.minute == 0) &&
			 (tempDate.second == 0) && (tempDate.nanoSecond == 0) &&
			 (tempDate.tzSign == 0) && (tempDate.tzHour == 0) && (tempDate.tzMinute == 0) ) {
                    snprintf ( buffer, sizeof(buffer), "%.4d", static_cast<int>(tempDate.year) ); // AUDIT: Using sizeof for snprintf length is safe.
		} else if ( (tempDate.year == 0) && (tempDate.day == 0) ) {
			FormatFullDateTime ( tempDate, buffer, sizeof(buffer) );
			addTimeZone = true;
		} else {
			XMP_Throw ( "Invalid partial date", kXMPErr_BadParam);
		}

	} else if ( tempDate.day == 0 ) {

		// Output YYYY-MM.
		if ( (tempDate.month < 1) || (tempDate.month > 12) ) XMP_Throw ( "Month is out of range", kXMPErr_BadParam);
		if ( (tempDate.hour != 0) || (tempDate.minute != 0) ||
			 (tempDate.second != 0) || (tempDate.nanoSecond != 0) ||
			 (tempDate.tzSign != 0) || (tempDate.tzHour != 0) || (tempDate.tzMinute != 0) ) {
			XMP_Throw ( "Invalid partial date, non-zeros after zero month and day", kXMPErr_BadParam);
		}
		snprintf ( buffer, sizeof(buffer), "%.4d-%02d", static_cast<int>(tempDate.year), static_cast<int>(tempDate.month) );	// AUDIT: Using sizeof for snprintf length is safe.

	} else if ( (tempDate.hour == 0) && (tempDate.minute == 0) &&
				(tempDate.second == 0) && (tempDate.nanoSecond == 0) &&
				(tempDate.tzSign == 0) && (tempDate.tzHour == 0) && (tempDate.tzMinute == 0) ) {

		// Output YYYY-MM-DD.
		if ( (tempDate.month < 1) || (tempDate.month > 12) ) XMP_Throw ( "Month is out of range", kXMPErr_BadParam);
		if ( (tempDate.day < 1) || (tempDate.day > 31) ) XMP_Throw ( "Day is out of range", kXMPErr_BadParam);
		snprintf ( buffer, sizeof(buffer), "%.4d-%02d-%02d", static_cast<int>(tempDate.year), static_cast<int>(tempDate.month), static_cast<int>(tempDate.day) ); // AUDIT: Using sizeof for snprintf length is safe.

	} else {

		FormatFullDateTime ( tempDate, buffer, sizeof(buffer) );
		addTimeZone = true;

	}

	sConvertedValue->assign ( buffer );

	if ( addTimeZone ) {

		if ( (tempDate.tzHour < 0) || (tempDate.tzHour > 23) ||
			 (tempDate.tzMinute < 0 ) || (tempDate.tzMinute > 59) ||
			 (tempDate.tzSign < -1) || (tempDate.tzSign > +1) ||
			 ((tempDate.tzSign != 0) && (tempDate.tzHour == 0) && (tempDate.tzMinute == 0)) ||
			 ((tempDate.tzSign == 0) && ((tempDate.tzHour != 0) || (tempDate.tzMinute != 0))) ) {
			XMP_Throw ( "Invalid time zone values", kXMPErr_BadParam );
		}

		if ( tempDate.tzSign == 0 ) {
			*sConvertedValue += 'Z';
		} else {
                    snprintf ( buffer, sizeof(buffer), "+%02d:%02d", static_cast<int>(tempDate.tzHour), static_cast<int>(tempDate.tzMinute) );	// AUDIT: Using sizeof for snprintf length is safe.
			if ( tempDate.tzSign < 0 ) buffer[0] = '-';
			*sConvertedValue += buffer;
		}

	}

	*strValue = sConvertedValue->c_str();
	*strSize  = sConvertedValue->size();

}	// ConvertFromDate


// -------------------------------------------------------------------------------------------------
// ConvertToBool
// -------------
//
// Formally the string value should be "True" or "False", but we should be more flexible here. Map
// the string to lower case. Allow any of "true", "false", "t", "f", "1", or "0".

/* class static */ bool
XMPUtils::ConvertToBool ( XMP_StringPtr strValue )
{
	if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty convert-from string", kXMPErr_BadValue );

	bool result = false;
	XMP_VarString strObj ( strValue );

	for ( XMP_VarStringPos ch = strObj.begin(); ch != strObj.end(); ++ch ) {
		if ( ('A' <= *ch) && (*ch <= 'Z') ) *ch += 0x20;
	}

	if ( (strObj == "true") || (strObj == "t") || (strObj == "1") ) {
		result = true;
	} else if ( (strObj == "false") || (strObj == "f") || (strObj == "0") ) {
		result = false;
	} else {
		XMP_Throw ( "Invalid Boolean string", kXMPErr_BadParam );
	}

	return result;

}	// ConvertToBool


// -------------------------------------------------------------------------------------------------
// ConvertToInt
// ------------

/* class static */ XMP_Int32
XMPUtils::ConvertToInt ( XMP_StringPtr strValue )
{
	if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty convert-from string", kXMPErr_BadValue );

	int count;
	char nextCh;
	XMP_Int32 result;

	if ( ! XMP_LitNMatch ( strValue, "0x", 2 ) ) {
            count = sscanf ( strValue, "%d%c", (int*)&result, &nextCh );
	} else {
            count = sscanf ( strValue, "%x%c", (unsigned int*)&result, &nextCh );
	}

	if ( count != 1 ) XMP_Throw ( "Invalid integer string", kXMPErr_BadParam );

	return result;

}	// ConvertToInt


// -------------------------------------------------------------------------------------------------
// ConvertToInt64
// --------------

/* class static */ XMP_Int64
XMPUtils::ConvertToInt64 ( XMP_StringPtr strValue )
{
#if defined(__MINGW32__)// || defined(__MINGW64__)
    return ConvertToInt(strValue);
#else
	if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty convert-from string", kXMPErr_BadValue );

	int count;
	char nextCh;
	XMP_Int64 result;

	if ( ! XMP_LitNMatch ( strValue, "0x", 2 ) ) {
		count = sscanf ( strValue, "%lld%c", &result, &nextCh );
	} else {
		count = sscanf ( strValue, "%llx%c", &result, &nextCh );
	}

	if ( count != 1 ) XMP_Throw ( "Invalid integer string", kXMPErr_BadParam );

	return result;
#endif
}	// ConvertToInt64


// -------------------------------------------------------------------------------------------------
// ConvertToFloat
// --------------

/* class static */ double
XMPUtils::ConvertToFloat ( XMP_StringPtr strValue )
{
	if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty convert-from string", kXMPErr_BadValue );

	XMP_VarString oldLocale;	// Try to make sure number conversion uses '.' as the decimal point.
	XMP_StringPtr oldLocalePtr = setlocale ( LC_ALL, 0 );
	if ( oldLocalePtr != 0 ) {
		oldLocale.assign ( oldLocalePtr );
		setlocale ( LC_ALL, "C" );
	}

	errno = 0;
	char * numEnd;
	double result = strtod ( strValue, &numEnd );

	if ( oldLocalePtr != 0 ) setlocale ( LC_ALL, oldLocalePtr );	// ! Reset locale before possible throw!
	if ( (errno != 0) || (*numEnd != 0) ) XMP_Throw ( "Invalid float string", kXMPErr_BadParam );

	return result;

}	// ConvertToFloat


// -------------------------------------------------------------------------------------------------
// ConvertToDate
// -------------
//
// Parse a date according to ISO 8601 and http://www.w3.org/TR/NOTE-datetime:
//	YYYY
//	YYYY-MM
//	YYYY-MM-DD
//	YYYY-MM-DDThh:mmTZD
//	YYYY-MM-DDThh:mm:ssTZD
//	YYYY-MM-DDThh:mm:ss.sTZD
//
//	YYYY = four-digit year
//	MM	 = two-digit month (01=January, etc.)
//	DD	 = two-digit day of month (01 through 31)
//	hh	 = two digits of hour (00 through 23)
//	mm	 = two digits of minute (00 through 59)
//	ss	 = two digits of second (00 through 59)
//	s	 = one or more digits representing a decimal fraction of a second
//	TZD	 = time zone designator (Z or +hh:mm or -hh:mm)
//
// Note that ISO 8601 does not seem to allow years less than 1000 or greater than 9999. We allow
// any year, even negative ones. The year is formatted as "%.4d".

// ! Tolerate missing TZD, assume the time is in local time
// ! Tolerate missing date portion, in case someone foolishly writes a time-only value that way.

// *** Put the ISO format comments in the header documentation.

/* class static */ void
XMPUtils::ConvertToDate ( XMP_StringPtr	 strValue,
						  XMP_DateTime * binValue )
{
	if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty convert-from string", kXMPErr_BadValue);

	size_t	 pos = 0;
	XMP_Int32 temp;

	XMP_Assert ( sizeof(*binValue) == sizeof(XMP_DateTime) );
	(void) memset ( binValue, 0, sizeof(*binValue) );	// AUDIT: Safe, using sizeof destination.

	bool timeOnly = ( (strValue[0] == 'T') ||
					  ((strlen(strValue) >= 2) && (strValue[1] == ':')) ||
					  ((strlen(strValue) >= 3) && (strValue[2] == ':')) );

	if ( ! timeOnly ) {

		if ( strValue[0] == '-' ) pos = 1;

		temp = GatherInt ( strValue, &pos, "Invalid year in date string" ); // Extract the year.
		if ( (strValue[pos] != 0) && (strValue[pos] != '-') ) XMP_Throw ( "Invalid date string, after year", kXMPErr_BadParam );
		if ( strValue[0] == '-' ) temp = -temp;
		binValue->year = temp;
		if ( strValue[pos] == 0 ) return;

		++pos;
		temp = GatherInt ( strValue, &pos, "Invalid month in date string" );	// Extract the month.
		if ( (strValue[pos] != 0) && (strValue[pos] != '-') ) XMP_Throw ( "Invalid date string, after month", kXMPErr_BadParam );
		binValue->month = temp;
		if ( strValue[pos] == 0 ) return;

		++pos;
		temp = GatherInt ( strValue, &pos, "Invalid day in date string" );	// Extract the day.
		if ( (strValue[pos] != 0) && (strValue[pos] != 'T') ) XMP_Throw ( "Invalid date string, after day", kXMPErr_BadParam );
		binValue->day = temp;
		if ( strValue[pos] == 0 ) return;

		// Allow year, month, and day to all be zero; implies the date portion is missing.
		if ( (binValue->year != 0) || (binValue->month != 0) || (binValue->day != 0) ) {
			// Temporary fix for bug 1269463, silently fix out of range month or day.
			// if ( (binValue->month < 1) || (binValue->month > 12) ) XMP_Throw ( "Month is out of range", kXMPErr_BadParam );
			// if ( (binValue->day < 1) || (binValue->day > 31) ) XMP_Throw ( "Day is out of range", kXMPErr_BadParam );
			if ( binValue->month < 1 ) binValue->month = 1;
			if ( binValue->month > 12 ) binValue->month = 12;
			if ( binValue->day < 1 ) binValue->day = 1;
			if ( binValue->day > 31 ) binValue->day = 31;
		}

	}

	if ( strValue[pos] == 'T' ) {
		++pos;
	} else if ( ! timeOnly ) {
		XMP_Throw ( "Invalid date string, missing 'T' after date", kXMPErr_BadParam );
	}

	temp = GatherInt ( strValue, &pos, "Invalid hour in date string" ); // Extract the hour.
	if ( strValue[pos] != ':' ) XMP_Throw ( "Invalid date string, after hour", kXMPErr_BadParam );
	if ( temp > 23 ) temp = 23;	// *** 1269463: XMP_Throw ( "Hour is out of range", kXMPErr_BadParam );
	binValue->hour = temp;
	// Don't check for done, we have to work up to the time zone.

	++pos;
	temp = GatherInt ( strValue, &pos, "Invalid minute in date string" );	// And the minute.
	if ( (strValue[pos] != ':') && (strValue[pos] != 'Z') &&
		 (strValue[pos] != '+') && (strValue[pos] != '-') && (strValue[pos] != 0) ) XMP_Throw ( "Invalid date string, after minute", kXMPErr_BadParam );
	if ( temp > 59 ) temp = 59;	// *** 1269463: XMP_Throw ( "Minute is out of range", kXMPErr_BadParam );
	binValue->minute = temp;
	// Don't check for done, we have to work up to the time zone.

	if ( strValue[pos] == ':' ) {

		++pos;
		temp = GatherInt ( strValue, &pos, "Invalid whole seconds in date string" );	// Extract the whole seconds.
		if ( (strValue[pos] != '.') && (strValue[pos] != 'Z') &&
			 (strValue[pos] != '+') && (strValue[pos] != '-') && (strValue[pos] != 0) ) {
			XMP_Throw ( "Invalid date string, after whole seconds", kXMPErr_BadParam );
		}
		if ( temp > 59 ) temp = 59;	// *** 1269463: XMP_Throw ( "Whole second is out of range", kXMPErr_BadParam );
		binValue->second = temp;
		// Don't check for done, we have to work up to the time zone.

		if ( strValue[pos] == '.' ) {

			++pos;
			size_t digits = pos;	// Will be the number of digits later.

			temp = GatherInt ( strValue, &pos, "Invalid fractional seconds in date string" );	// Extract the fractional seconds.
			if ( (strValue[pos] != 'Z') && (strValue[pos] != '+') && (strValue[pos] != '-') && (strValue[pos] != 0) ) {
				XMP_Throw ( "Invalid date string, after fractional second", kXMPErr_BadParam );
			}

			digits = pos - digits;
			for ( ; digits > 9; --digits ) temp = temp / 10;
			for ( ; digits < 9; ++digits ) temp = temp * 10;

			if ( temp >= 1000*1000*1000 ) XMP_Throw ( "Fractional second is out of range", kXMPErr_BadParam );
			binValue->nanoSecond = temp;
			// Don't check for done, we have to work up to the time zone.

		}

	}

	if ( strValue[pos] == 'Z' ) {

		++pos;

	} else if ( strValue[pos] != 0 ) {

		if ( strValue[pos] == '+' ) {
			binValue->tzSign = kXMP_TimeEastOfUTC;
		} else if ( strValue[pos] == '-' ) {
			binValue->tzSign = kXMP_TimeWestOfUTC;
		} else {
			XMP_Throw ( "Time zone must begin with 'Z', '+', or '-'", kXMPErr_BadParam );
		}

		++pos;
		temp = GatherInt ( strValue, &pos, "Invalid time zone hour in date string" );	// Extract the time zone hour.
		if ( strValue[pos] != ':' ) XMP_Throw ( "Invalid date string, after time zone hour", kXMPErr_BadParam );
		if ( temp > 23 ) XMP_Throw ( "Time zone hour is out of range", kXMPErr_BadParam );
		binValue->tzHour = temp;

		++pos;
		temp = GatherInt ( strValue, &pos, "Invalid time zone minute in date string" ); // Extract the time zone minute.
		if ( temp > 59 ) XMP_Throw ( "Time zone minute is out of range", kXMPErr_BadParam );
		binValue->tzMinute = temp;

	} else {

		XMPUtils::SetTimeZone( binValue );

	}

	if ( strValue[pos] != 0 ) XMP_Throw ( "Invalid date string, extra chars at end", kXMPErr_BadParam );

}	// ConvertToDate


// -------------------------------------------------------------------------------------------------
// EncodeToBase64
// --------------
//
// Encode a string of raw data bytes in base 64 according to RFC 2045. For the encoding definition
// see section 6.8 in <http://www.ietf.org/rfc/rfc2045.txt>. Although it isn't needed for RDF, we
// do insert a linefeed character as a newline for every 76 characters of encoded output.

/* class static */ void
XMPUtils::EncodeToBase64 ( XMP_StringPtr   rawStr,
						   XMP_StringLen   rawLen,
						   XMP_StringPtr * encodedStr,
						   XMP_StringLen * encodedLen )
{
	if ( (rawStr == 0) && (rawLen != 0) ) XMP_Throw ( "Null raw data buffer", kXMPErr_BadParam );
	if ( rawLen == 0 ) {
		*encodedStr = 0;
		*encodedLen = 0;
		return;
	}

	char	encChunk[4];

	unsigned long	in, out;
	unsigned char	c1, c2, c3;
	unsigned long	merge;

	const size_t	outputSize	= (rawLen / 3) * 4; // Approximate, might be  small.

	sBase64Str->erase();
	sBase64Str->reserve ( outputSize );

	// ----------------------------------------------------------------------------------------
	// Each 6 bits of input produces 8 bits of output, so 3 input bytes become 4 output bytes.
	// Process the whole chunks of 3 bytes first, then deal with any remainder. Be careful with
	// the loop comparison, size-2 could be negative!

	for ( in = 0, out = 0; (in+2) < rawLen; in += 3, out += 4 ) {

		c1	= rawStr[in];
		c2	= rawStr[in+1];
		c3	= rawStr[in+2];

		merge	= (c1 << 16) + (c2 << 8) + c3;

		encChunk[0] = sBase64Chars [ merge >> 18 ];
		encChunk[1] = sBase64Chars [ (merge >> 12) & 0x3F ];
		encChunk[2] = sBase64Chars [ (merge >> 6) & 0x3F ];
		encChunk[3] = sBase64Chars [ merge & 0x3F ];

		if ( out >= 76 ) {
			sBase64Str->append ( 1, kLF );
			out = 0;
		}
		sBase64Str->append ( encChunk, 4 );

	}

	// ------------------------------------------------------------------------------------------
	// The output must always be a multiple of 4 bytes. If there is a 1 or 2 byte input remainder
	// we need to create another chunk. Zero pad with bits to a 6 bit multiple, then add one or
	// two '=' characters to pad out to 4 bytes.

	switch ( rawLen - in ) {

		case 0:		// Done, no remainder.
			break;

		case 1:		// One input byte remains.

			c1	= rawStr[in];
			merge	= c1 << 16;

			encChunk[0] = sBase64Chars [ merge >> 18 ];
			encChunk[1] = sBase64Chars [ (merge >> 12) & 0x3F ];
			encChunk[2] = encChunk[3] = '=';

			if ( out >= 76 ) sBase64Str->append ( 1, kLF );
			sBase64Str->append ( encChunk, 4 );
			break;

		case 2:		// Two input bytes remain.

			c1	= rawStr[in];
			c2	= rawStr[in+1];
			merge	= (c1 << 16) + (c2 << 8);

			encChunk[0] = sBase64Chars [ merge >> 18 ];
			encChunk[1] = sBase64Chars [ (merge >> 12) & 0x3F ];
			encChunk[2] = sBase64Chars [ (merge >> 6) & 0x3F ];
			encChunk[3] = '=';

			if ( out >= 76 ) sBase64Str->append ( 1, kLF );
			sBase64Str->append ( encChunk, 4 );
			break;

	}

	// -------------------------
	// Assign the output values.

	*encodedStr = sBase64Str->c_str();
	*encodedLen = sBase64Str->size();

}	// EncodeToBase64


// -------------------------------------------------------------------------------------------------
// DecodeFromBase64
// ----------------
//
// Decode a string of raw data bytes from base 64 according to RFC 2045. For the encoding definition
// see section 6.8 in <http://www.ietf.org/rfc/rfc2045.txt>. RFC 2045 talks about ignoring all "bad"
// input but warning about non-whitespace. For XMP use we ignore space, tab, LF, and CR. Any other
// bad input is rejected.

/* class static */ void
XMPUtils::DecodeFromBase64 ( XMP_StringPtr	 encodedStr,
							 XMP_StringLen	 encodedLen,
							 XMP_StringPtr * rawStr,
							 XMP_StringLen * rawLen )
{
	if ( (encodedStr == 0) && (encodedLen != 0) ) XMP_Throw ( "Null encoded data buffer", kXMPErr_BadParam );
	if ( encodedLen == 0 ) {
		*rawStr = 0;
		*rawLen = 0;
		return;
	}

	unsigned char	ch, rawChunk[3];
	unsigned long	inStr, inChunk, inLimit, merge, padding;

	XMP_StringLen	outputSize	= (encodedLen / 4) * 3; // Only a close approximation.

	sBase64Str->erase();
	sBase64Str->reserve ( outputSize );


	// ----------------------------------------------------------------------------------------
	// Each 8 bits of input produces 6 bits of output, so 4 input bytes become 3 output bytes.
	// Process all but the last 4 data bytes first, then deal with the final chunk. Whitespace
	// in the input must be ignored. The first loop finds where the last 4 data bytes start and
	// counts the number of padding equal signs.

	padding = 0;
	for ( inStr = 0, inLimit = encodedLen; (inStr < 4) && (inLimit > 0); ) {
		inLimit -= 1;	// ! Don't do in the loop control, the decr/test order is wrong.
		ch = encodedStr[inLimit];
		if ( ch == '=' ) {
			padding += 1;	// The equal sign padding is a data byte.
		} else if ( DecodeBase64Char(ch) == 0xFF ) {
			continue;	// Ignore whitespace, don't increment inStr.
		} else {
			inStr += 1;
		}
	}

	// ! Be careful to count whitespace that is immediately before the final data. Otherwise
	// ! middle portion will absorb the final data and mess up the final chunk processing.

	while ( (inLimit > 0) && (DecodeBase64Char(encodedStr[inLimit-1]) == 0xFF) ) --inLimit;

	if ( inStr == 0 ) return;	// Nothing but whitespace.
	if ( padding > 2 ) XMP_Throw ( "Invalid encoded string", kXMPErr_BadParam );

	// -------------------------------------------------------------------------------------------
	// Now process all but the last chunk. The limit ensures that we have at least 4 data bytes
	// left when entering the output loop, so the inner loop will succeed without overrunning the
	// end of the data. At the end of the outer loop we might be past inLimit though.

	inStr = 0;
	while ( inStr < inLimit ) {

		merge = 0;
		for ( inChunk = 0; inChunk < 4; ++inStr ) { // ! Yes, increment inStr on each pass.
			ch = DecodeBase64Char ( encodedStr [inStr] );
			if ( ch == 0xFF ) continue; // Ignore whitespace.
			merge = (merge << 6) + ch;
			inChunk += 1;
		}

		rawChunk[0] = (unsigned char) (merge >> 16);
		rawChunk[1] = (unsigned char) ((merge >> 8) & 0xFF);
		rawChunk[2] = (unsigned char) (merge & 0xFF);

		sBase64Str->append ( (char*)rawChunk, 3 );

	}

	// -------------------------------------------------------------------------------------------
	// Process the final, possibly partial, chunk of data. The input is always a multiple 4 bytes,
	// but the raw data can be any length. The number of padding '=' characters determines if the
	// final chunk has 1, 2, or 3 raw data bytes.

	XMP_Assert ( inStr < encodedLen );

	merge = 0;
	for ( inChunk = 0; inChunk < 4-padding; ++inStr ) { // ! Yes, increment inStr on each pass.
		ch = DecodeBase64Char ( encodedStr[inStr] );
		if ( ch == 0xFF ) continue; // Ignore whitespace.
		merge = (merge << 6) + ch;
		inChunk += 1;
	}

	if ( padding == 2 ) {

		rawChunk[0] = (unsigned char) (merge >> 4);
		sBase64Str->append ( (char*)rawChunk, 1 );

	} else if ( padding == 1 ) {

		rawChunk[0] = (unsigned char) (merge >> 10);
		rawChunk[1] = (unsigned char) ((merge >> 2) & 0xFF);
		sBase64Str->append ( (char*)rawChunk, 2 );

	} else {

		rawChunk[0] = (unsigned char) (merge >> 16);
		rawChunk[1] = (unsigned char) ((merge >> 8) & 0xFF);
		rawChunk[2] = (unsigned char) (merge & 0xFF);
		sBase64Str->append ( (char*)rawChunk, 3 );

	}

	// -------------------------
	// Assign the output values.

	*rawStr = sBase64Str->c_str();
	*rawLen = sBase64Str->size();

}	// DecodeFromBase64


// -------------------------------------------------------------------------------------------------
// PackageForJPEG
// --------------

/* class static */ void
XMPUtils::PackageForJPEG ( const XMPMeta & origXMP,
						   XMP_StringPtr * stdStr,
						   XMP_StringLen * stdLen,
						   XMP_StringPtr * extStr,
						   XMP_StringLen * extLen,
						   XMP_StringPtr * digestStr,
						   XMP_StringLen * digestLen )
{
	enum { kStdXMPLimit = 65000 };
	static const char * kPacketTrailer = "<?xpacket end=\"w\"?>";
	static size_t kTrailerLen = strlen ( kPacketTrailer );

	XMP_StringPtr tempStr;
	XMP_StringLen tempLen;

	XMPMeta stdXMP, extXMP;

	sStandardXMP->clear();	// Clear the static strings that get returned to the client.
	sExtendedXMP->clear();
	sExtendedDigest->clear();

	XMP_OptionBits keepItSmall = kXMP_UseCompactFormat | kXMP_OmitAllFormatting;

	// Try to serialize everything. Note that we're making internal calls to SerializeToBuffer, so
	// we'll be getting back the pointer and length for its internal string.

	origXMP.SerializeToBuffer ( &tempStr, &tempLen, keepItSmall, 1, "", "", 0 );
	#if Trace_PackageForJPEG
		printf ( "\nXMPUtils::PackageForJPEG - Full serialize %d bytes\n", tempLen );
	#endif

	if ( tempLen > kStdXMPLimit ) {

		// Couldn't fit everything, make a copy of the input XMP and make sure there is no xmp:Thumbnails property.

		stdXMP.tree.options = origXMP.tree.options;
		stdXMP.tree.name    = origXMP.tree.name;
		stdXMP.tree.value   = origXMP.tree.value;
		CloneOffspring ( &origXMP.tree, &stdXMP.tree );

		if ( stdXMP.DoesPropertyExist ( kXMP_NS_XMP, "Thumbnails" ) ) {
			stdXMP.DeleteProperty ( kXMP_NS_XMP, "Thumbnails" );
			stdXMP.SerializeToBuffer ( &tempStr, &tempLen, keepItSmall, 1, "", "", 0 );
			#if Trace_PackageForJPEG
				printf ( "  Delete xmp:Thumbnails, %d bytes left\n", tempLen );
			#endif
		}

	}

	if ( tempLen > kStdXMPLimit ) {

		// Still doesn't fit, move all of the Camera Raw namespace. Add a dummy value for xmpNote:HasExtendedXMP.

		stdXMP.SetProperty ( kXMP_NS_XMP_Note, "HasExtendedXMP", "123456789-123456789-123456789-12", 0 );

		XMP_NodePtrPos crSchemaPos;
		XMP_Node * crSchema = FindSchemaNode ( &stdXMP.tree, kXMP_NS_CameraRaw, kXMP_ExistingOnly, &crSchemaPos );

		if ( crSchema != 0 ) {
			crSchema->parent = &extXMP.tree;
			extXMP.tree.children.push_back ( crSchema );
			stdXMP.tree.children.erase ( crSchemaPos );
			stdXMP.SerializeToBuffer ( &tempStr, &tempLen, keepItSmall, 1, "", "", 0 );
			#if Trace_PackageForJPEG
				printf ( "  Move Camera Raw schema, %d bytes left\n", tempLen );
			#endif
		}

	}

	if ( tempLen > kStdXMPLimit ) {

		// Still doesn't fit, move photoshop:History.

		bool moved = MoveOneProperty ( stdXMP, &extXMP, kXMP_NS_Photoshop, "photoshop:History" );

		if ( moved ) {
			stdXMP.SerializeToBuffer ( &tempStr, &tempLen, keepItSmall, 1, "", "", 0 );
			#if Trace_PackageForJPEG
				printf ( "  Move photoshop:History, %d bytes left\n", tempLen );
			#endif
		}

	}

	if ( tempLen > kStdXMPLimit ) {

		// Still doesn't fit, move top level properties in order of estimated size. This is done by
		// creating a multi-map that maps the serialized size to the string pair for the schema URI
		// and top level property name. Since maps are inherently ordered, a reverse iteration of
		// the map can be done to move the largest things first. We use a double loop to keep going
		// until the serialization actually fits, in case the estimates are off.

		PropSizeMap propSizes;
		CreateEstimatedSizeMap ( stdXMP, &propSizes );

		#if Trace_PackageForJPEG
		if ( ! propSizes.empty() ) {
			printf ( "  Top level property map, smallest to largest:\n" );
			PropSizeMap::iterator mapPos = propSizes.begin();
			PropSizeMap::iterator mapEnd = propSizes.end();
			for ( ; mapPos != mapEnd; ++mapPos ) {
				size_t propSize = mapPos->first;
				const char * schemaName = mapPos->second.first->c_str();
				const char * propName   = mapPos->second.second->c_str();
				printf ( "    %d bytes, %s in %s\n", propSize, propName, schemaName );
			}
		}
		#endif

		#if 0	// Trace_PackageForJPEG		*** Xcode 2.3 on 10.4.7 has bugs in backwards iteration
		if ( ! propSizes.empty() ) {
			printf ( "  Top level property map, largest to smallest:\n" );
			PropSizeMap::iterator mapPos   = propSizes.end();
			PropSizeMap::iterator mapBegin = propSizes.begin();
			for ( --mapPos; true; --mapPos ) {
				size_t propSize = mapPos->first;
				const char * schemaName = mapPos->second.first->c_str();
				const char * propName   = mapPos->second.second->c_str();
				printf ( "    %d bytes, %s in %s\n", propSize, propName, schemaName );
				if ( mapPos == mapBegin ) break;
			}
		}
		#endif

		// Outer loop to make sure enough is actually moved.

		while ( (tempLen > kStdXMPLimit) && (! propSizes.empty()) ) {

			// Inner loop, move what seems to be enough according to the estimates.

			while ( (tempLen > kStdXMPLimit) && (! propSizes.empty()) ) {

				size_t propSize = MoveLargestProperty ( stdXMP, &extXMP, propSizes );
				XMP_Assert ( propSize > 0 );

				if ( propSize > tempLen ) propSize = tempLen;	// ! Don't go negative.
				tempLen -= propSize;

			}

			// Reserialize the remaining standard XMP.

			stdXMP.SerializeToBuffer ( &tempStr, &tempLen, keepItSmall, 1, "", "", 0 );

		}

	}

	if ( tempLen > kStdXMPLimit ) {
		// Still doesn't fit, throw an exception and let the client decide what to do.
		// ! This should never happen with the policy of moving any and all top level properties.
		XMP_Throw ( "Can't reduce XMP enough for JPEG file", kXMPErr_TooLargeForJPEG );
	}

	// Set the static output strings.

	if ( extXMP.tree.children.empty() ) {

		// Just have the standard XMP.
		sStandardXMP->assign ( tempStr, tempLen );

	} else {

		// Have extended XMP. Serialize it, compute the digest, reset xmpNote:HasExtendedXMP, and
		// reserialize the standard XMP.

		extXMP.SerializeToBuffer ( &tempStr, &tempLen, (keepItSmall | kXMP_OmitPacketWrapper), 0, "", "", 0 );
		sExtendedXMP->assign ( tempStr, tempLen );

		MD5_CTX  context;
		XMP_Uns8 digest [16];
		MD5Init ( &context );
		MD5Update ( &context, (XMP_Uns8*)tempStr, tempLen );
		MD5Final ( digest, &context );

		sExtendedDigest->reserve ( 32 );
		for ( size_t i = 0; i < 16; ++i ) {
			XMP_Uns8 byte = digest[i];
			sExtendedDigest->push_back ( kHexDigits [ byte>>4 ] );
			sExtendedDigest->push_back ( kHexDigits [ byte&0xF ] );
		}

		stdXMP.SetProperty ( kXMP_NS_XMP_Note, "HasExtendedXMP", sExtendedDigest->c_str(), 0 );
		stdXMP.SerializeToBuffer ( &tempStr, &tempLen, keepItSmall, 1, "", "", 0 );
		sStandardXMP->assign ( tempStr, tempLen );

	}

	// Adjust the standard XMP padding to be up to 2KB.

	XMP_Assert ( (sStandardXMP->size() > kTrailerLen) && (sStandardXMP->size() <= kStdXMPLimit) );
	const char * packetEnd = 0;
    packetEnd = sStandardXMP->c_str() + sStandardXMP->size() - kTrailerLen;
	XMP_Assert ( XMP_LitMatch ( packetEnd, kPacketTrailer ) );
	UNUSED(packetEnd);

	size_t extraPadding = kStdXMPLimit - sStandardXMP->size();	// ! Do this before erasing the trailer.
	if ( extraPadding > 2047 ) extraPadding = 2047;
	sStandardXMP->erase ( sStandardXMP->size() - kTrailerLen );
	sStandardXMP->append ( extraPadding, ' ' );
	sStandardXMP->append ( kPacketTrailer );

	// Assign the output pointer and sizes.

	*stdStr = sStandardXMP->c_str();
	*stdLen = sStandardXMP->size();
	*extStr = sExtendedXMP->c_str();
	*extLen = sExtendedXMP->size();
	*digestStr = sExtendedDigest->c_str();
	*digestLen = sExtendedDigest->size();

}	// PackageForJPEG


// -------------------------------------------------------------------------------------------------
// MergeFromJPEG
// -------------
//
// Copy all of the top level properties from extendedXMP to fullXMP, replacing any duplicates.
// Delete the xmpNote:HasExtendedXMP property from fullXMP.

/* class static */ void
XMPUtils::MergeFromJPEG ( XMPMeta *       fullXMP,
                          const XMPMeta & extendedXMP )
{

	XMPUtils::AppendProperties ( extendedXMP, fullXMP, kXMPUtil_DoAllProperties );
	fullXMP->DeleteProperty ( kXMP_NS_XMP_Note, "HasExtendedXMP" );

}	// MergeFromJPEG


// -------------------------------------------------------------------------------------------------
// CurrentDateTime
// ---------------

/* class static */ void
XMPUtils::CurrentDateTime ( XMP_DateTime * xmpTime )
{
	XMP_Assert ( xmpTime != 0 );	// ! Enforced by wrapper.

	ansi_tt binTime = ansi_time(0);
	if ( binTime == -1 ) XMP_Throw ( "Failure from ANSI C time function", kXMPErr_ExternalFailure );
	ansi_tm currTime;
	ansi_localtime ( &binTime, &currTime );

	xmpTime->year = currTime.tm_year + 1900;
	xmpTime->month = currTime.tm_mon + 1;
	xmpTime->day = currTime.tm_mday;
	xmpTime->hour = currTime.tm_hour;
	xmpTime->minute = currTime.tm_min;
	xmpTime->second = currTime.tm_sec;

	xmpTime->nanoSecond = 0;
	xmpTime->tzSign = 0;
	xmpTime->tzHour = 0;
	xmpTime->tzMinute = 0;

	XMPUtils::SetTimeZone ( xmpTime );

}	// CurrentDateTime


// -------------------------------------------------------------------------------------------------
// SetTimeZone
// -----------
//
// Sets just the time zone part of the time.  Useful for determining the local time zone or for
// converting a "zone-less" time to a proper local time. The ANSI C time functions are smart enough
// to do all the right stuff, as long as we call them properly!

/* class static */ void
XMPUtils::SetTimeZone ( XMP_DateTime * xmpTime )
{
	XMP_Assert ( xmpTime != 0 );	// ! Enforced by wrapper.

	if ( (xmpTime->tzSign != 0) || (xmpTime->tzHour != 0) || (xmpTime->tzMinute != 0) ) {
		XMP_Throw ( "SetTimeZone can only be used on \"zoneless\" times", kXMPErr_BadParam );
	}

	// Create ansi_tt form of the input time. Need the ansi_tm form to make the ansi_tt form.

	ansi_tt ttTime;
	ansi_tm tmLocal, tmUTC;

	if ( (xmpTime->year == 0) && (xmpTime->month == 0) && (xmpTime->day == 0) ) {
		ansi_tt now = ansi_time(0);
		if ( now == -1 ) XMP_Throw ( "Failure from ANSI C time function", kXMPErr_ExternalFailure );
		ansi_localtime ( &now, &tmLocal );
	} else {
		tmLocal.tm_year = xmpTime->year - 1900;
		while ( tmLocal.tm_year < 70 ) tmLocal.tm_year += 4;	// ! Some versions of mktime barf on years before 1970.
		tmLocal.tm_mon	 = xmpTime->month - 1;
		tmLocal.tm_mday	 = xmpTime->day;
	}

	tmLocal.tm_hour = xmpTime->hour;
	tmLocal.tm_min = xmpTime->minute;
	tmLocal.tm_sec = xmpTime->second;
	tmLocal.tm_isdst = -1;	// Don't know if daylight time is in effect.

	ttTime = ansi_mktime ( &tmLocal );
	if ( ttTime == -1 ) XMP_Throw ( "Failure from ANSI C mktime function", kXMPErr_ExternalFailure );

	// Convert back to a localized ansi_tm time and get the corresponding UTC ansi_tm time.

	ansi_localtime ( &ttTime, &tmLocal );
	ansi_gmtime ( &ttTime, &tmUTC );

	// Get the offset direction and amount.

	ansi_tm tmx = tmLocal;	// ! Note that mktime updates the ansi_tm parameter, messing up difftime!
	ansi_tm tmy = tmUTC;
	tmx.tm_isdst = tmy.tm_isdst = 0;
	ansi_tt ttx = ansi_mktime ( &tmx );
	ansi_tt tty = ansi_mktime ( &tmy );
	double diffSecs;

	if ( (ttx != -1) && (tty != -1) ) {
		diffSecs = ansi_difftime ( ttx, tty );
	} else {
		#if XMP_MacBuild
			// Looks like Apple's mktime is buggy - see W1140533. But the offset is visible.
			diffSecs = tmLocal.tm_gmtoff;
		#else
			// Win and UNIX don't have a visible offset. Make sure we know about the failure,
			// then try using the current date/time as a close fallback.
			ttTime = ansi_time(0);
			if ( ttTime == -1 ) XMP_Throw ( "Failure from ANSI C time function", kXMPErr_ExternalFailure );
			ansi_localtime ( &ttTime, &tmx );
			ansi_gmtime ( &ttTime, &tmy );
			tmx.tm_isdst = tmy.tm_isdst = 0;
			ttx = ansi_mktime ( &tmx );
			tty = ansi_mktime ( &tmy );
			if ( (ttx == -1) || (tty == -1) ) XMP_Throw ( "Failure from ANSI C mktime function", kXMPErr_ExternalFailure );
			diffSecs = ansi_difftime ( ttx, tty );
		#endif
	}

	if ( diffSecs > 0.0 ) {
		xmpTime->tzSign = kXMP_TimeEastOfUTC;
	} else if ( diffSecs == 0.0 ) {
		xmpTime->tzSign = kXMP_TimeIsUTC;
	} else {
		xmpTime->tzSign = kXMP_TimeWestOfUTC;
		diffSecs = -diffSecs;
	}
	xmpTime->tzHour = XMP_Int32 ( diffSecs / 3600.0 );
	xmpTime->tzMinute = XMP_Int32 ( (diffSecs / 60.0) - (xmpTime->tzHour * 60.0) );

	// *** Save the tm_isdst flag in a qualifier?

	XMP_Assert ( (0 <= xmpTime->tzHour) && (xmpTime->tzHour <= 23) );
	XMP_Assert ( (0 <= xmpTime->tzMinute) && (xmpTime->tzMinute <= 59) );
	XMP_Assert ( (-1 <= xmpTime->tzSign) && (xmpTime->tzSign <= +1) );
	XMP_Assert ( (xmpTime->tzSign == 0) ? ((xmpTime->tzHour == 0) && (xmpTime->tzMinute == 0)) :
										  ((xmpTime->tzHour != 0) || (xmpTime->tzMinute != 0)) );

}	// SetTimeZone


// -------------------------------------------------------------------------------------------------
// ConvertToUTCTime
// ----------------

/* class static */ void
XMPUtils::ConvertToUTCTime ( XMP_DateTime * time )
{
	XMP_Assert ( time != 0 );	// ! Enforced by wrapper.

	XMP_Assert ( (0 <= time->tzHour) && (time->tzHour <= 23) );
	XMP_Assert ( (0 <= time->tzMinute) && (time->tzMinute <= 59) );
	XMP_Assert ( (-1 <= time->tzSign) && (time->tzSign <= +1) );
	XMP_Assert ( (time->tzSign == 0) ? ((time->tzHour == 0) && (time->tzMinute == 0)) :
									   ((time->tzHour != 0) || (time->tzMinute != 0)) );

	if ( time->tzSign == kXMP_TimeEastOfUTC ) {
		// We are before (east of) GMT, subtract the offset from the time.
		time->hour -= time->tzHour;
		time->minute -= time->tzMinute;
	} else if ( time->tzSign == kXMP_TimeWestOfUTC ) {
		// We are behind (west of) GMT, add the offset to the time.
		time->hour += time->tzHour;
		time->minute += time->tzMinute;
	}

	AdjustTimeOverflow ( time );
	time->tzSign = time->tzHour = time->tzMinute = 0;

}	// ConvertToUTCTime


// -------------------------------------------------------------------------------------------------
// ConvertToLocalTime
// ------------------

/* class static */ void
XMPUtils::ConvertToLocalTime ( XMP_DateTime * time )
{
	XMP_Assert ( time != 0 );	// ! Enforced by wrapper.

	XMP_Assert ( (0 <= time->tzHour) && (time->tzHour <= 23) );
	XMP_Assert ( (0 <= time->tzMinute) && (time->tzMinute <= 59) );
	XMP_Assert ( (-1 <= time->tzSign) && (time->tzSign <= +1) );
	XMP_Assert ( (time->tzSign == 0) ? ((time->tzHour == 0) && (time->tzMinute == 0)) :
									   ((time->tzHour != 0) || (time->tzMinute != 0)) );

	ConvertToUTCTime ( time );	// The existing time zone might not be the local one.
	SetTimeZone ( time );		// Fill in the local timezone offset, then adjust the time.

	if ( time->tzSign > 0 ) {
		// We are before (east of) GMT, add the offset to the time.
		time->hour += time->tzHour;
		time->minute += time->tzMinute;
	} else if ( time->tzSign < 0 ) {
		// We are behind (west of) GMT, subtract the offset from the time.
		time->hour -= time->tzHour;
		time->minute -= time->tzMinute;
	}

	AdjustTimeOverflow ( time );

}	// ConvertToLocalTime


// -------------------------------------------------------------------------------------------------
// CompareDateTime
// ---------------

/* class static */ int
XMPUtils::CompareDateTime ( const XMP_DateTime & _in_left,
							const XMP_DateTime & _in_right )
{
	int result;

	XMP_DateTime left  = _in_left;
	XMP_DateTime right = _in_right;

	ConvertToUTCTime ( &left );
	ConvertToUTCTime ( &right );

	// *** We could use memcmp if the XMP_DateTime stuct has no holes.

	if ( left.year < right.year ) {
		result = -1;
	} else if ( left.year > right.year ) {
		result = +1;
	} else if ( left.month < right.month ) {
		result = -1;
	} else if ( left.month > right.month ) {
		result = +1;
	} else if ( left.day < right.day ) {
		result = -1;
	} else if ( left.day > right.day ) {
		result = +1;
	} else if ( left.hour < right.hour ) {
		result = -1;
	} else if ( left.hour > right.hour ) {
		result = +1;
	} else if ( left.minute < right.minute ) {
		result = -1;
	} else if ( left.minute > right.minute ) {
		result = +1;
	} else if ( left.second < right.second ) {
		result = -1;
	} else if ( left.second > right.second ) {
		result = +1;
	} else if ( left.nanoSecond < right.nanoSecond ) {
		result = -1;
	} else if ( left.nanoSecond > right.nanoSecond ) {
		result = +1;
	} else {
		result = 0;
	}

	return result;

}	// CompareDateTime

// =================================================================================================
