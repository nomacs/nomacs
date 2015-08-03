// =================================================================================================
// Copyright 2004-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Const.h"

#if UnicodeTestBuild
	#include <cassert>
	#include <stdexcept>
	#define UC_Assert assert
	#define UC_Throw(m,k) throw std::logic_error ( m )
#else
	#define UC_Assert(cond) /* Nothing for now, should be XMP_Assert. */
	#define UC_Throw(msg,id)  throw XMP_Error ( id, msg )
#endif

#include "UnicodeConversions.hpp"

using namespace std;

// =================================================================================================

// *** Look into using asm inlines, e.g. count-leading bits for multi-byte UTF-8.

CodePoint_to_UTF16_Proc CodePoint_to_UTF16BE = 0;
CodePoint_to_UTF16_Proc CodePoint_to_UTF16LE = 0;

CodePoint_from_UTF16_Proc CodePoint_from_UTF16BE = 0;
CodePoint_from_UTF16_Proc CodePoint_from_UTF16LE = 0;

UTF8_to_UTF16_Proc  UTF8_to_UTF16BE = 0;
UTF8_to_UTF16_Proc  UTF8_to_UTF16LE = 0;
UTF8_to_UTF32_Proc  UTF8_to_UTF32BE = 0;
UTF8_to_UTF32_Proc  UTF8_to_UTF32LE = 0;

UTF16_to_UTF8_Proc  UTF16BE_to_UTF8 = 0;
UTF16_to_UTF8_Proc  UTF16LE_to_UTF8 = 0;
UTF32_to_UTF8_Proc  UTF32BE_to_UTF8 = 0;
UTF32_to_UTF8_Proc  UTF32LE_to_UTF8 = 0;

UTF8_to_UTF16_Proc  UTF8_to_UTF16Native = 0;
UTF8_to_UTF32_Proc  UTF8_to_UTF32Native = 0;
UTF16_to_UTF8_Proc  UTF16Native_to_UTF8 = 0;
UTF32_to_UTF8_Proc  UTF32Native_to_UTF8 = 0;

UTF16_to_UTF32_Proc UTF16BE_to_UTF32BE = 0;
UTF16_to_UTF32_Proc UTF16BE_to_UTF32LE = 0;
UTF16_to_UTF32_Proc UTF16LE_to_UTF32BE = 0;
UTF16_to_UTF32_Proc UTF16LE_to_UTF32LE = 0;

UTF32_to_UTF16_Proc UTF32BE_to_UTF16BE = 0;
UTF32_to_UTF16_Proc UTF32BE_to_UTF16LE = 0;
UTF32_to_UTF16_Proc UTF32LE_to_UTF16BE = 0;
UTF32_to_UTF16_Proc UTF32LE_to_UTF16LE = 0;

// -------------------------------------------------------------------------------------------------

static size_t swap32to16Offset = 0;	// Offset to "convert" a swapped UTF32 pointer into a swapped UTF16 pointer.

// -------------------------------------------------------------------------------------------------

static void CodePoint_to_UTF16Nat ( const UTF32Unit cpIn, UTF16Unit * utf16Out, const size_t utf16Len, size_t * utf16Written );
static void CodePoint_to_UTF16Swp ( const UTF32Unit cpIn, UTF16Unit * utf16Out, const size_t utf16Len, size_t * utf16Written );

static void CodePoint_from_UTF16Nat ( const UTF16Unit * utf16In, const size_t utf16Len, UTF32Unit * cpOut, size_t * utf16Read );
static void CodePoint_from_UTF16Swp ( const UTF16Unit * utf16In, const size_t utf16Len, UTF32Unit * cpOut, size_t * utf16Read );

// -------------------------------------------------------------------------------------------------

static void UTF8_to_UTF16Nat ( const UTF8Unit *  utf8In,    const size_t utf8Len,
				               UTF16Unit *       utf16Out,  const size_t utf16Len,
				               size_t *          utf8Read,  size_t *     utf16Written );

static void UTF8_to_UTF16Swp ( const UTF8Unit *  utf8In,    const size_t utf8Len,
				               UTF16Unit *       utf16Out,  const size_t utf16Len,
				               size_t *          utf8Read,  size_t *     utf16Written );

static void UTF8_to_UTF32Nat ( const UTF8Unit *  utf8In,    const size_t utf8Len,
				               UTF32Unit *       utf32Out,  const size_t utf32Len,
				               size_t *          utf8Read,  size_t *     utf32Written );

static void UTF8_to_UTF32Swp ( const UTF8Unit *  utf8In,    const size_t utf8Len,
				               UTF32Unit *       utf32Out,  const size_t utf32Len,
				               size_t *          utf8Read,  size_t *     utf32Written );

// -------------------------------------------------------------------------------------------------

static void UTF16Nat_to_UTF8 ( const UTF16Unit * utf16In,   const size_t utf16Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf16Read, size_t *     utf8Written );

static void UTF16Swp_to_UTF8 ( const UTF16Unit * utf16In,   const size_t utf16Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf16Read, size_t *     utf8Written );

static void UTF32Nat_to_UTF8 ( const UTF32Unit * utf32In,   const size_t utf32Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf32Read, size_t *     utf8Written );

static void UTF32Swp_to_UTF8 ( const UTF32Unit * utf32In,   const size_t utf32Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf32Read, size_t *     utf8Written );

// -------------------------------------------------------------------------------------------------

static void UTF16Nat_to_UTF32Nat ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written );

static void UTF16Nat_to_UTF32Swp ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written );

static void UTF16Swp_to_UTF32Nat ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written );

static void UTF16Swp_to_UTF32Swp ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written );

// -------------------------------------------------------------------------------------------------

static void UTF32Nat_to_UTF16Nat ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written );

static void UTF32Nat_to_UTF16Swp ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written );

static void UTF32Swp_to_UTF16Nat ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written );

static void UTF32Swp_to_UTF16Swp ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written );

// =================================================================================================

void InitializeUnicodeConversions()
{
	UC_Assert ( (sizeof(UTF8Unit) == 1) && (sizeof(UTF16Unit) == 2) && (sizeof(UTF32Unit) == 4) ); 

	UTF16Unit u16  = 0x00FF;
	bool bigEndian = (*((UTF8Unit*)&u16) == 0);

	UTF8_to_UTF16Native = UTF8_to_UTF16Nat;
	UTF8_to_UTF32Native = UTF8_to_UTF32Nat;
	UTF16Native_to_UTF8 = UTF16Nat_to_UTF8;
	UTF32Native_to_UTF8 = UTF32Nat_to_UTF8;
	
	if ( bigEndian ) {
	
		swap32to16Offset = 0;

		CodePoint_to_UTF16BE = CodePoint_to_UTF16Nat;
		CodePoint_to_UTF16LE = CodePoint_to_UTF16Swp;

		CodePoint_from_UTF16BE = CodePoint_from_UTF16Nat;
		CodePoint_from_UTF16LE = CodePoint_from_UTF16Swp;

		UTF8_to_UTF16BE = UTF8_to_UTF16Nat;
		UTF8_to_UTF16LE = UTF8_to_UTF16Swp;
		UTF8_to_UTF32BE = UTF8_to_UTF32Nat;
		UTF8_to_UTF32LE = UTF8_to_UTF32Swp;

		UTF16BE_to_UTF8 = UTF16Nat_to_UTF8;
		UTF16LE_to_UTF8 = UTF16Swp_to_UTF8;
		UTF32BE_to_UTF8 = UTF32Nat_to_UTF8;
		UTF32LE_to_UTF8 = UTF32Swp_to_UTF8;

		UTF16BE_to_UTF32BE = UTF16Nat_to_UTF32Nat;
		UTF16BE_to_UTF32LE = UTF16Nat_to_UTF32Swp;
		UTF16LE_to_UTF32BE = UTF16Swp_to_UTF32Nat;
		UTF16LE_to_UTF32LE = UTF16Swp_to_UTF32Swp;

		UTF32BE_to_UTF16BE = UTF32Nat_to_UTF16Nat;
		UTF32BE_to_UTF16LE = UTF32Nat_to_UTF16Swp;
		UTF32LE_to_UTF16BE = UTF32Swp_to_UTF16Nat;
		UTF32LE_to_UTF16LE = UTF32Swp_to_UTF16Swp;

	} else {
	
		swap32to16Offset = 1;	// ! Offset in UTF16 units!

		CodePoint_to_UTF16BE = CodePoint_to_UTF16Swp;
		CodePoint_to_UTF16LE = CodePoint_to_UTF16Nat;

		CodePoint_from_UTF16BE = CodePoint_from_UTF16Swp;
		CodePoint_from_UTF16LE = CodePoint_from_UTF16Nat;

		UTF8_to_UTF16BE = UTF8_to_UTF16Swp;
		UTF8_to_UTF16LE = UTF8_to_UTF16Nat;
		UTF8_to_UTF32BE = UTF8_to_UTF32Swp;
		UTF8_to_UTF32LE = UTF8_to_UTF32Nat;

		UTF16BE_to_UTF8 = UTF16Swp_to_UTF8;
		UTF16LE_to_UTF8 = UTF16Nat_to_UTF8;
		UTF32BE_to_UTF8 = UTF32Swp_to_UTF8;
		UTF32LE_to_UTF8 = UTF32Nat_to_UTF8;

		UTF16BE_to_UTF32BE = UTF16Swp_to_UTF32Swp;
		UTF16BE_to_UTF32LE = UTF16Swp_to_UTF32Nat;
		UTF16LE_to_UTF32BE = UTF16Nat_to_UTF32Swp;
		UTF16LE_to_UTF32LE = UTF16Nat_to_UTF32Nat;

		UTF32BE_to_UTF16BE = UTF32Swp_to_UTF16Swp;
		UTF32BE_to_UTF16LE = UTF32Swp_to_UTF16Nat;
		UTF32LE_to_UTF16BE = UTF32Nat_to_UTF16Swp;
		UTF32LE_to_UTF16LE = UTF32Nat_to_UTF16Nat;

	}

}	// InitializeUnicodeConversions

// =================================================================================================

#if XMP_MacBuild && __MWERKS__ 

	#define UTF16InSwap(inPtr)	UTF16Unit ( __lhbrx ( (void*)(inPtr), 0 ) )
	#define UTF32InSwap(inPtr)	UTF32Unit ( __lwbrx ( (void*)(inPtr), 0 ) )
	
	#define UTF16OutSwap(outPtr,value)	__sthbrx ( value, (void*)(outPtr), 0 )
	#define UTF32OutSwap(outPtr,value)	__stwbrx ( value, (void*)(outPtr), 0 )

#else

	static inline UTF16Unit UTF16InSwap ( const UTF16Unit * inPtr )
	{
		UTF16Unit inUnit = *inPtr;
		return (inUnit << 8) | (inUnit >> 8);
	}

	static inline UTF32Unit UTF32InSwap ( const UTF32Unit * inPtr )
	{
		UTF32Unit inUnit = *inPtr;
		return (inUnit << 24) | ((inUnit << 8) & 0x00FF0000) | ((inUnit >> 8) & 0x0000FF00) | (inUnit >> 24);
	}

	static inline void UTF16OutSwap ( UTF16Unit * outPtr, const UTF16Unit value )
	{
		UTF16Unit outUnit = (value << 8) | (value >> 8);
		*outPtr = outUnit;
	}

	static inline void UTF32OutSwap ( UTF32Unit * outPtr, const UTF32Unit value )
	{
		UTF32Unit outUnit = (value << 24) | ((value << 8) & 0x00FF0000) | ((value >> 8) & 0x0000FF00) | (value >> 24);
		*outPtr = outUnit;
	}

#endif

// =================================================================================================

void SwapUTF16 ( const UTF16Unit * utf16In, UTF16Unit * utf16Out, const size_t utf16Len )
{
	for ( size_t i = 0; i < utf16Len; ++i ) utf16Out[i] = UTF16InSwap(utf16In+i);
}

void SwapUTF32 ( const UTF32Unit * utf32In, UTF32Unit * utf32Out, const size_t utf32Len ) {
	for ( size_t i = 0; i < utf32Len; ++i ) utf32Out[i] = UTF32InSwap(utf32In+i);
}

// =================================================================================================

extern void ToUTF16 ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf16Str, bool bigEndian )
{
	UTF8_to_UTF16_Proc Converter = UTF8_to_UTF16LE;
	if ( bigEndian ) Converter = UTF8_to_UTF16BE;
	
	enum { kBufferSize = 8*1024 };
	UTF16Unit u16Buffer[kBufferSize];	// 16K bytes
	size_t readCount, writeCount;

	utf16Str->erase();
	utf16Str->reserve ( 2*utf8Len );	// As good a guess as any.
	
	while ( utf8Len > 0 ) {
		Converter ( utf8In, utf8Len, u16Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf16Str->append ( (const char *)u16Buffer, writeCount*2 );
		utf8In  += readCount;
		utf8Len -= readCount;
	}

}	// ToUTF16

// =================================================================================================

extern void ToUTF16Native ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf16Str )
{
	enum { kBufferSize = 8*1024 };
	UTF16Unit u16Buffer[kBufferSize];	// 16K bytes
	size_t readCount, writeCount;

	utf16Str->erase();
	utf16Str->reserve ( 2*utf8Len );	// As good a guess as any.
	
	while ( utf8Len > 0 ) {
		UTF8_to_UTF16Nat ( utf8In, utf8Len, u16Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf16Str->append ( (const char *)u16Buffer, writeCount*2 );
		utf8In  += readCount;
		utf8Len -= readCount;
	}

}	// ToUTF16Native

// =================================================================================================

extern void ToUTF32 ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf32Str, bool bigEndian )
{
	UTF8_to_UTF32_Proc Converter = UTF8_to_UTF32LE;
	if ( bigEndian ) Converter = UTF8_to_UTF32BE;
	
	enum { kBufferSize = 4*1024 };
	UTF32Unit u32Buffer[kBufferSize];	// 16K bytes
	size_t readCount, writeCount;

	utf32Str->erase();
	utf32Str->reserve ( 4*utf8Len );	// As good a guess as any.
	
	while ( utf8Len > 0 ) {
		Converter ( utf8In, utf8Len, u32Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf32Str->append ( (const char *)u32Buffer, writeCount*4 );
		utf8In  += readCount;
		utf8Len -= readCount;
	}

}	// ToUTF32

// =================================================================================================

extern void ToUTF32Native ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf32Str )
{
	enum { kBufferSize = 4*1024 };
	UTF32Unit u32Buffer[kBufferSize];	// 16K bytes
	size_t readCount, writeCount;

	utf32Str->erase();
	utf32Str->reserve ( 4*utf8Len );	// As good a guess as any.
	
	while ( utf8Len > 0 ) {
		UTF8_to_UTF32Nat ( utf8In, utf8Len, u32Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf32Str->append ( (const char *)u32Buffer, writeCount*4 );
		utf8In  += readCount;
		utf8Len -= readCount;
	}

}	// ToUTF32Native

// =================================================================================================

extern void FromUTF16 ( const UTF16Unit * utf16In, size_t utf16Len, std::string * utf8Str, bool bigEndian )
{
	UTF16_to_UTF8_Proc Converter = UTF16LE_to_UTF8;
	if ( bigEndian ) Converter = UTF16BE_to_UTF8;
	
	enum { kBufferSize = 16*1024 };
	UTF8Unit u8Buffer[kBufferSize];
	size_t readCount, writeCount;

	utf8Str->erase();
	utf8Str->reserve ( 2*utf16Len );	// As good a guess as any.
	
	while ( utf16Len > 0 ) {
		Converter ( utf16In, utf16Len, u8Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf8Str->append ( (const char *)u8Buffer, writeCount );
		utf16In  += readCount;
		utf16Len -= readCount;
	}

}	// FromUTF16

// =================================================================================================

extern void FromUTF16Native ( const UTF16Unit * utf16In, size_t utf16Len, std::string * utf8Str )
{
	enum { kBufferSize = 16*1024 };
	UTF8Unit u8Buffer[kBufferSize];
	size_t readCount, writeCount;

	utf8Str->erase();
	utf8Str->reserve ( 2*utf16Len );	// As good a guess as any.
	
	while ( utf16Len > 0 ) {
		UTF16Nat_to_UTF8 ( utf16In, utf16Len, u8Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf8Str->append ( (const char *)u8Buffer, writeCount );
		utf16In  += readCount;
		utf16Len -= readCount;
	}

}	// FromUTF16Native

// =================================================================================================

extern void FromUTF32 ( const UTF32Unit * utf32In, size_t utf32Len, std::string * utf8Str, bool bigEndian )
{
	UTF32_to_UTF8_Proc Converter = UTF32LE_to_UTF8;
	if ( bigEndian ) Converter = UTF32BE_to_UTF8;
	
	enum { kBufferSize = 16*1024 };
	UTF8Unit u8Buffer[kBufferSize];
	size_t readCount, writeCount;

	utf8Str->erase();
	utf8Str->reserve ( 2*utf32Len );	// As good a guess as any.
	
	while ( utf32Len > 0 ) {
		Converter ( utf32In, utf32Len, u8Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf8Str->append ( (const char *)u8Buffer, writeCount );
		utf32In  += readCount;
		utf32Len -= readCount;
	}

}	// FromUTF32

// =================================================================================================

extern void FromUTF32Native ( const UTF32Unit * utf32In, size_t utf32Len, std::string * utf8Str )
{
	enum { kBufferSize = 16*1024 };
	UTF8Unit u8Buffer[kBufferSize];
	size_t readCount, writeCount;

	utf8Str->erase();
	utf8Str->reserve ( 2*utf32Len );	// As good a guess as any.
	
	while ( utf32Len > 0 ) {
		UTF32Nat_to_UTF8 ( utf32In, utf32Len, u8Buffer, kBufferSize, &readCount, &writeCount );
		if ( writeCount == 0 ) UC_Throw ( "Incomplete Unicode at end of string", kXMPErr_BadXML );
		utf8Str->append ( (const char *)u8Buffer, writeCount );
		utf32In  += readCount;
		utf32Len -= readCount;
	}

}	// FromUTF32Native

// =================================================================================================

static void CodePoint_to_UTF8_Multi ( const UTF32Unit cpIn, UTF8Unit * utf8Out, const size_t utf8Len, size_t * utf8Written )
{
	size_t unitCount = 0;
	
	if ( cpIn > 0x10FFFF ) UC_Throw ( "Bad UTF-32 - out of range", kXMPErr_BadParam );
	if ( (0xD800 <= cpIn) && (cpIn <= 0xDFFF) ) UC_Throw ( "Bad UTF-32 - surrogate code point", kXMPErr_BadParam );
	
	// Compute the number of bytes using 6 data bits each. Then see if the highest order bits will
	// fit into the leading byte. Write the UTF-8 sequence if there is enough room.
	
	UTF32Unit temp, mask;
	size_t bytesNeeded = 0;
	for ( temp = cpIn; temp != 0; temp = temp >> 6 ) ++bytesNeeded;
	
	temp = cpIn >> ((bytesNeeded-1)*6);	// The highest order data bits.
	mask = (0x80 >> bytesNeeded) - 1;	// Available data bits in the leading byte.
	if ( temp > mask ) ++bytesNeeded;

	if ( bytesNeeded > utf8Len ) goto Done;	// Not enough room for the output.
	unitCount = bytesNeeded;
	
	temp = cpIn;
	for ( --bytesNeeded; bytesNeeded > 0; --bytesNeeded ) {
		utf8Out[bytesNeeded] = 0x80 | UTF8Unit ( temp & 0x3F );
		temp = temp >> 6;
	}
	
	mask = ~((1 << (8-unitCount)) - 1);
	utf8Out[0] = UTF8Unit ( mask | temp );

Done:
	*utf8Written = unitCount;
	return;
	
}	// CodePoint_to_UTF8_Multi

// =================================================================================================

void CodePoint_to_UTF8 ( const UTF32Unit cpIn, UTF8Unit * utf8Out, const size_t utf8Len, size_t * utf8Written )
{
	size_t unitCount = 0;

	UC_Assert ( (utf8Out != 0) && (utf8Written != 0) );
	if ( utf8Len == 0 ) goto Done;
	if ( cpIn > 0x7F ) goto MultiByte;	// ! Force linear execution path for ASCII.
	
	if ( utf8Len == 0 ) goto Done;
	unitCount = 1;
	*utf8Out = UTF8Unit(cpIn);

Done:
	*utf8Written = unitCount;
	return;
	
MultiByte:
	 CodePoint_to_UTF8_Multi( cpIn, utf8Out, utf8Len, utf8Written );
	 return;
	
}	// CodePoint_to_UTF8

// =================================================================================================

static void CodePoint_from_UTF8_Multi ( const UTF8Unit * utf8In, const size_t utf8Len, UTF32Unit * cpOut, size_t * utf8Read )
{
	UTF8Unit  inUnit = *utf8In;
	size_t    unitCount = 0;
	UTF32Unit cp;	// ! Avoid gcc complaints about declarations after goto's.
	const UTF8Unit * utf8Pos;

	// -------------------------------------------------------------------------------------
	// We've got a multibyte UTF-8 character. The first byte has the number of bytes and the
	// highest order data bits. The other bytes each add 6 more data bits.
	
	#if 0	// This might be a more effcient way to count the bytes.
		static XMP_Uns8 kByteCounts[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 3, 4 };
		size_t bytesNeeded = kByteCounts [ inUnit >> 4 ];
		if ( (bytesNeeded < 2) || ((bytesNeeded == 4) && ((inUnit & 0x08) != 0)) ) {
			UC_Throw ( "Invalid UTF-8 sequence length", kXMPErr_BadParam );
		}
	#endif

	size_t bytesNeeded = 0;	// Count the leading 1 bits in the first byte.
	for ( UTF8Unit temp = inUnit; temp > 0x7F; temp = temp << 1 ) ++bytesNeeded;
		// *** Consider CPU-specific assembly inline, e.g. cntlzw on PowerPC.
	
	if ( (bytesNeeded < 2) || (bytesNeeded > 4) ) UC_Throw ( "Invalid UTF-8 sequence length", kXMPErr_BadParam );
	if ( bytesNeeded > utf8Len ) goto Done;	// Not enough input in this buffer.
	unitCount = bytesNeeded;
	
	cp = inUnit & ((1 << (7-unitCount)) - 1);	// Isolate the initial data bits in the bottom of cp.
	
	utf8Pos = utf8In + 1;	// We've absorbed the first byte.
	for ( --bytesNeeded; bytesNeeded > 0; --bytesNeeded, ++utf8Pos ) {
		inUnit = *utf8Pos;
		if ( (inUnit & UTF8Unit(0xC0)) != UTF8Unit(0x80) ) UC_Throw ( "Invalid UTF-8 data byte", kXMPErr_BadParam );
		cp = (cp << 6) | (inUnit & 0x3F);
	}
	
	if ( cp >= 0xD800 ) {	// Skip the next comparisons most of the time.
		if ( (0xD800 <= cp) && (cp <= 0xDFFF) ) UC_Throw ( "Bad UTF-8 - surrogate code point", kXMPErr_BadParam );
		if ( cp > 0x10FFFF ) UC_Throw ( "Bad UTF-8 - out of range", kXMPErr_BadParam );
	}
	
	*cpOut = cp;	// ! Don't put after Done, don't write if no input.
	
Done:	
	*utf8Read = unitCount;
	return;
	
}	// CodePoint_from_UTF8_Multi

// =================================================================================================

void CodePoint_from_UTF8 ( const UTF8Unit * utf8In, const size_t utf8Len, UTF32Unit * cpOut, size_t * utf8Read )
{
	UTF8Unit inUnit;	// ! Don't read until we know there is input.
	size_t unitCount = 0;

	UC_Assert ( (utf8In != 0) && (cpOut != 0) && (utf8Read != 0) );
	if ( utf8Len == 0 ) goto Done;
	inUnit = *utf8In;
	if ( inUnit >= 0x80 ) goto MultiByte;	// ! Force linear execution path for ASCII.
	
	unitCount = 1;
	*cpOut = inUnit;	// ! Don't put after Done, don't write if no input.
	
Done:	
	*utf8Read = unitCount;
	return;

MultiByte:
	CodePoint_from_UTF8_Multi ( utf8In, utf8Len, cpOut, utf8Read );
	return;
	
}	// CodePoint_from_UTF8

// =================================================================================================

static void CodePoint_to_UTF16Nat_Surrogate ( const UTF32Unit cpIn, UTF16Unit * utf16Out, const size_t utf16Len, size_t * utf16Written )
{
	size_t    unitCount = 0;
	UTF32Unit temp;	// ! Avoid gcc complaints about declarations after goto's.

	if ( cpIn > 0x10FFFF ) UC_Throw ( "Bad UTF-32 - out of range", kXMPErr_BadParam );
	if ( utf16Len < 2 ) goto Done;	// Not enough room for the output.

	unitCount = 2;
	temp = cpIn - 0x10000;
	utf16Out[0] = 0xD800 | UTF16Unit ( temp >> 10 );
	utf16Out[1] = 0xDC00 | UTF16Unit ( temp & 0x3FF );
	
Done:
	*utf16Written = unitCount;
	return;
	
}	// CodePoint_to_UTF16Nat_Surrogate

// =================================================================================================

static void CodePoint_to_UTF16Nat ( const UTF32Unit cpIn, UTF16Unit * utf16Out, const size_t utf16Len, size_t * utf16Written )
{
	size_t unitCount = 0;

	UC_Assert ( (utf16Out != 0) && (utf16Written != 0) );	
	if ( utf16Len == 0 ) goto Done;
	if ( cpIn >= 0xD800 ) goto CheckSurrogate;	// ! Force linear execution path for the BMP.

InBMP:	
	unitCount = 1;
	*utf16Out = UTF16Unit(cpIn);
	
Done:
	*utf16Written = unitCount;
	return;

CheckSurrogate:
	if ( cpIn > 0xFFFF ) goto SurrogatePair;
	if ( cpIn > 0xDFFF ) goto InBMP;
	UC_Throw ( "Bad UTF-32 - surrogate code point", kXMPErr_BadParam );
	
SurrogatePair:
	CodePoint_to_UTF16Nat_Surrogate ( cpIn, utf16Out, utf16Len, utf16Written );
	return;
	
}	// CodePoint_to_UTF16Nat

// =================================================================================================

static void CodePoint_from_UTF16Nat_Surrogate ( const UTF16Unit * utf16In, const size_t utf16Len, UTF32Unit * cpOut, size_t * utf16Read )
{
	UTF16Unit hiUnit = *utf16In;
	size_t    unitCount = 0;
	UTF16Unit loUnit;	// ! Avoid gcc complaints about declarations after goto's.
	UTF32Unit cp;

	// ----------------------------------
	// We've got a UTF-16 surrogate pair.

	if ( hiUnit > 0xDBFF ) UC_Throw ( "Bad UTF-16 - leading low surrogate", kXMPErr_BadParam );
	if ( utf16Len < 2 ) goto Done;	// Not enough input in this buffer.
	
	loUnit  = *(utf16In+1);
	if ( (loUnit < 0xDC00) || (0xDFFF < loUnit) ) UC_Throw ( "Bad UTF-16 - missing low surrogate", kXMPErr_BadParam );
	
	unitCount = 2;
	cp = (((hiUnit & 0x3FF) << 10) | (loUnit & 0x3FF)) + 0x10000;

	*cpOut = cp;	// ! Don't put after Done, don't write if no input.
	
Done:
	*utf16Read = unitCount;
	return;
	
}	// CodePoint_from_UTF16Nat_Surrogate

// =================================================================================================

static void CodePoint_from_UTF16Nat ( const UTF16Unit * utf16In, const size_t utf16Len, UTF32Unit * cpOut, size_t * utf16Read )
{
	UTF16Unit inUnit;	// ! Don't read until we know there is input.
	size_t unitCount = 0;

	UC_Assert ( (utf16In != 0) && (cpOut != 0) && (utf16Read != 0) );
	if ( utf16Len == 0 ) goto Done;
	inUnit = *utf16In;
	if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) goto SurrogatePair;	// ! Force linear execution path for the BMP.

	unitCount = 1;
	*cpOut = inUnit;	// ! Don't put after Done, don't write if no input.
	
Done:
	*utf16Read = unitCount;
	return;

SurrogatePair:
	CodePoint_from_UTF16Nat_Surrogate ( utf16In, utf16Len, cpOut, utf16Read );
	return;
	
}	// CodePoint_from_UTF16Nat

// =================================================================================================

static void UTF8_to_UTF16Nat ( const UTF8Unit * utf8In,   const size_t utf8Len,
				               UTF16Unit *      utf16Out, const size_t utf16Len,
				               size_t *         utf8Read, size_t *     utf16Written )
{
	const UTF8Unit * utf8Pos  = utf8In;
	UTF16Unit *      utf16Pos = utf16Out;
	
	size_t utf8Left  = utf8Len;
	size_t utf16Left = utf16Len;
	
	UC_Assert ( (utf8In != 0) && (utf16Out != 0) && (utf8Read != 0) && (utf16Written != 0) );
	
	while ( (utf8Left > 0) && (utf16Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf8Left;
		if ( limit > utf16Left ) limit = utf16Left;
		for ( i = 0; i < limit; ++i ) {
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit > 0x7F ) break;
			*utf16Pos = inUnit;
			++utf8Pos;
			++utf16Pos;
		}
		utf8Left  -= i;
		utf16Left -= i;
		
		// Do a run of non-ASCII, it copies multiple input units into 1 or 2 output units.
		while ( (utf8Left > 0) && (utf16Left > 0) ) {
			UTF32Unit cp;
			size_t len8, len16;
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit <= 0x7F ) break;
			CodePoint_from_UTF8_Multi ( utf8Pos, utf8Left, &cp, &len8 );
			if ( len8 == 0 ) goto Done;		// The input buffer ends in the middle of a character.
			if ( cp <= 0xFFFF ) {
				*utf16Pos = UTF16Unit(cp);
				len16 = 1;
			} else {
				CodePoint_to_UTF16Nat_Surrogate ( cp, utf16Pos, utf16Left, &len16 );
				if ( len16 == 0 ) goto Done;	// Not enough room in the output buffer.
			}
			utf8Left  -= len8;
			utf8Pos   += len8;
			utf16Left -= len16;
			utf16Pos  += len16;
		}
	
	}

Done:	// Set the output lengths.
	*utf8Read = utf8Len - utf8Left;
	*utf16Written = utf16Len - utf16Left;
	
}	// UTF8_to_UTF16Nat

// =================================================================================================

static void UTF8_to_UTF32Nat ( const UTF8Unit *  utf8In,   const size_t utf8Len,
				               UTF32Unit *       utf32Out, const size_t utf32Len,
				               size_t *          utf8Read, size_t *     utf32Written )
{
	const UTF8Unit * utf8Pos  = utf8In;
	UTF32Unit *      utf32Pos = utf32Out;
	
	size_t utf8Left  = utf8Len;
	size_t utf32Left = utf32Len;
	
	UC_Assert ( (utf8In != 0) && (utf32Out != 0) && (utf8Read != 0) && (utf32Written != 0) );
	
	while ( (utf8Left > 0) && (utf32Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf8Left;
		if ( limit > utf32Left ) limit = utf32Left;
		for ( i = 0; i < limit; ++i ) {
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit > 0x7F ) break;
			*utf32Pos = inUnit;
			++utf8Pos;
			++utf32Pos;
		}
		utf8Left -= i;
		utf32Left -= i;
		
		// Do a run of non-ASCII, it copies variable input into 1 output unit.
		while ( (utf8Left > 0) && (utf32Left > 0) ) {
			size_t len;
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit <= 0x7F ) break;
			CodePoint_from_UTF8_Multi ( utf8Pos, utf8Left, utf32Pos, &len );
			if ( len == 0 ) goto Done;	// The input buffer ends in the middle of a character.
			utf8Left  -= len;
			utf8Pos   += len;
			utf32Left -= 1;
			utf32Pos  += 1;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf8Read = utf8Len - utf8Left;
	*utf32Written = utf32Len - utf32Left;
	
}	// UTF8_to_UTF32Nat

// =================================================================================================

static void UTF16Nat_to_UTF8 ( const UTF16Unit * utf16In,   const size_t utf16Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf16Read, size_t *     utf8Written )
{
	const UTF16Unit * utf16Pos = utf16In;
	UTF8Unit *        utf8Pos  = utf8Out;
	
	size_t utf16Left = utf16Len;
	size_t utf8Left  = utf8Len;
	
	UC_Assert ( (utf16In != 0) && (utf8Out != 0) && (utf16Read != 0) && (utf8Written != 0) );
	
	while ( (utf16Left > 0) && (utf8Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf16Left;
		if ( limit > utf8Left ) limit = utf8Left;
		for ( i = 0; i < limit; ++i ) {
			UTF16Unit inUnit = *utf16Pos;
			if ( inUnit > 0x7F ) break;
			*utf8Pos = UTF8Unit(inUnit);
			++utf16Pos;
			++utf8Pos;
		}
		utf16Left -= i;
		utf8Left  -= i;
		
		// Do a run of non-ASCII inside the BMP, it copies 1 input unit into multiple output units.
		while ( (utf16Left > 0) && (utf8Left > 0) ) {
			size_t len8;
			UTF16Unit inUnit = *utf16Pos;
			if ( inUnit <= 0x7F ) break;
			if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) break;
			CodePoint_to_UTF8_Multi ( inUnit, utf8Pos, utf8Left, &len8 );
			if ( len8 == 0 ) goto Done;		// Not enough room in the output buffer.
			utf16Left -= 1;
			utf16Pos  += 1;
			utf8Left  -= len8;
			utf8Pos   += len8;
		}
		
		// Do a run of surrogate pairs, it copies 2 input units into multiple output units.
		while ( (utf16Left > 0) && (utf8Left > 0) ) {
			UTF32Unit cp;
			size_t len16, len8;
			UTF16Unit inUnit = *utf16Pos;
			if ( (inUnit < 0xD800) || (0xDFFF < inUnit) ) break;
			CodePoint_from_UTF16Nat_Surrogate ( utf16Pos, utf16Left, &cp, &len16 );
			if ( len16 == 0 ) goto Done;	// The input buffer ends in the middle of a surrogate pair.
			UC_Assert ( len16 == 2 );
			CodePoint_to_UTF8_Multi ( cp, utf8Pos, utf8Left, &len8 );
			if ( len8 == 0 ) goto Done;		// Not enough room in the output buffer.
			utf16Left -= len16;
			utf16Pos  += len16;
			utf8Left  -= len8;
			utf8Pos   += len8;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf16Read = utf16Len - utf16Left;
	*utf8Written = utf8Len - utf8Left;
	
}	// UTF16Nat_to_UTF8

// =================================================================================================

static void UTF32Nat_to_UTF8 ( const UTF32Unit * utf32In,   const size_t utf32Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf32Read, size_t *     utf8Written )
{
	const UTF32Unit * utf32Pos = utf32In;
	UTF8Unit *        utf8Pos  = utf8Out;
	
	size_t utf32Left = utf32Len;
	size_t utf8Left  = utf8Len;
	
	UC_Assert ( (utf32In != 0) && (utf8Out != 0) && (utf32Read != 0) && (utf8Written != 0) );
	
	while ( (utf32Left > 0) && (utf8Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf32Left;
		if ( limit > utf8Left ) limit = utf8Left;
		for ( i = 0; i < limit; ++i ) {
			UTF32Unit inUnit = *utf32Pos;
			if ( inUnit > 0x7F ) break;
			*utf8Pos = UTF8Unit(inUnit);
			++utf32Pos;
			++utf8Pos;
		}
		utf32Left -= i;
		utf8Left  -= i;
		
		// Do a run of non-ASCII, it copies 1 input unit into multiple output units.
		while ( (utf32Left > 0) && (utf8Left > 0) ) {
			size_t len;
			UTF32Unit inUnit = *utf32Pos;
			if ( inUnit <= 0x7F ) break;
			CodePoint_to_UTF8_Multi ( inUnit, utf8Pos, utf8Left, &len );
			if ( len == 0 ) goto Done;	// Not enough room in the output buffer.
			utf32Left -= 1;
			utf32Pos  += 1;
			utf8Left  -= len;
			utf8Pos   += len;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf32Read = utf32Len - utf32Left;
	*utf8Written = utf8Len - utf8Left;
	
}	// UTF32Nat_to_UTF8

// =================================================================================================

static void UTF16Nat_to_UTF32Nat ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written )
{
	const UTF16Unit * utf16Pos = utf16In;
	UTF32Unit *       utf32Pos = utf32Out;
	
	size_t utf16Left = utf16Len;
	size_t utf32Left = utf32Len;
	
	UC_Assert ( (utf16In != 0) && (utf32Out != 0) && (utf16Read != 0) && (utf32Written != 0) );
	
	while ( (utf16Left > 0) && (utf32Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf16Left;
		if ( limit > utf32Left ) limit = utf32Left;
		for ( i = 0; i < limit; ++i ) {
			UTF16Unit inUnit = *utf16Pos;
			if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) break;
			*utf32Pos = inUnit;
			++utf16Pos;
			++utf32Pos;
		}
		utf16Left -= i;
		utf32Left -= i;
		
		// Do a run of surrogate pairs, it copies 2 input units into 1 output unit.
		while ( (utf16Left > 0) && (utf32Left > 0) ) {
			size_t len;
			UTF16Unit inUnit = *utf16Pos;
			if ( (inUnit < 0xD800) || (0xDFFF < inUnit) ) break;
			CodePoint_from_UTF16Nat_Surrogate ( utf16Pos, utf16Left, utf32Pos, &len );
			if ( len == 0 ) goto Done;	// The input buffer ends in the middle of a surrogate pair.
			UC_Assert ( len == 2 );
			utf16Left -= len;
			utf16Pos  += len;
			utf32Left -= 1;
			utf32Pos  += 1;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf16Read = utf16Len - utf16Left;
	*utf32Written = utf32Len - utf32Left;
	
}	// UTF16Nat_to_UTF32Nat

// =================================================================================================

static void UTF32Nat_to_UTF16Nat ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written )
{
	const UTF32Unit * utf32Pos = utf32In;
	UTF16Unit *       utf16Pos = utf16Out;
	
	size_t utf32Left = utf32Len;
	size_t utf16Left = utf16Len;
	
	UC_Assert ( (utf32In != 0) && (utf16Out != 0) && (utf32Read != 0) && (utf16Written != 0) );
	
	while ( (utf32Left > 0) && (utf16Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf32Left;
		if ( limit > utf16Left ) limit = utf16Left;
		for ( i = 0; i < limit; ++i ) {
			UTF32Unit inUnit = *utf32Pos;
			if ( inUnit > 0xFFFF ) break;
			*utf16Pos = UTF16Unit(inUnit);
			++utf32Pos;
			++utf16Pos;
		}
		utf32Left -= i;
		utf16Left -= i;
		
		// Do a run of non-BMP, it copies 1 input unit into 2 output units.
		while ( (utf32Left > 0) && (utf16Left > 0) ) {
			size_t len;
			UTF32Unit inUnit = *utf32Pos;
			if ( inUnit <= 0xFFFF ) break;
			CodePoint_to_UTF16Nat_Surrogate ( inUnit, utf16Pos, utf16Left, &len );
			if ( len == 0 ) goto Done;	// Not enough room in the output buffer.
			UC_Assert ( len == 2 );
			utf32Left -= 1;
			utf32Pos  += 1;
			utf16Left -= 2;
			utf16Pos  += 2;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf32Read = utf32Len - utf32Left;
	*utf16Written = utf16Len - utf16Left;
	
}	// UTF32Nat_to_UTF16Nat

// =================================================================================================

static void CodePoint_to_UTF16Swp_Surrogate ( const UTF32Unit cpIn, UTF16Unit * utf16Out, const size_t utf16Len, size_t * utf16Written )
{
	size_t unitCount = 0;
	UTF32Unit temp;	// ! Avoid gcc complaints about declarations after goto's.

	if ( cpIn > 0x10FFFF ) UC_Throw ( "Bad UTF-32 - out of range", kXMPErr_BadParam );
	if ( utf16Len < 2 ) goto Done;	// Not enough room for the output.

	unitCount = 2;
	temp = cpIn - 0x10000;
	UTF16OutSwap ( &utf16Out[0], (0xD800 | UTF16Unit ( temp >> 10 )) );
	UTF16OutSwap ( &utf16Out[1], (0xDC00 | UTF16Unit ( temp & 0x3FF)) );
	
Done:
	*utf16Written = unitCount;
	return;
	
}	// CodePoint_to_UTF16Swp_Surrogate

// =================================================================================================

static void CodePoint_to_UTF16Swp ( const UTF32Unit cpIn, UTF16Unit * utf16Out, const size_t utf16Len, size_t * utf16Written )
{
	size_t unitCount = 0;

	UC_Assert ( (utf16Out != 0) && (utf16Written != 0) );	
	if ( utf16Len == 0 ) goto Done;
	if ( cpIn >= 0xD800 ) goto CheckSurrogate;	// ! Force linear execution path for the BMP.

InBMP:	
	unitCount = 1;
	UTF16OutSwap ( utf16Out, UTF16Unit(cpIn) );
	
Done:
	*utf16Written = unitCount;
	return;

CheckSurrogate:
	if ( cpIn > 0xFFFF ) goto SurrogatePair;
	if ( cpIn > 0xDFFF ) goto InBMP;
	UC_Throw ( "Bad UTF-32 - surrogate code point", kXMPErr_BadParam );
	
SurrogatePair:
	CodePoint_to_UTF16Swp_Surrogate ( cpIn, utf16Out, utf16Len, utf16Written );
	return;
	
}	// CodePoint_to_UTF16Swp

// =================================================================================================

static void CodePoint_from_UTF16Swp_Surrogate ( const UTF16Unit * utf16In, const size_t utf16Len, UTF32Unit * cpOut, size_t * utf16Read )
{
	UTF16Unit hiUnit = UTF16InSwap(utf16In);
	size_t unitCount = 0;
	UTF16Unit loUnit;	// ! Avoid gcc complaints about declarations after goto's.
	UTF32Unit cp;

	// ----------------------------------
	// We've got a UTF-16 surrogate pair.

	if ( hiUnit > 0xDBFF ) UC_Throw ( "Bad UTF-16 - leading low surrogate", kXMPErr_BadParam );
	if ( utf16Len < 2 ) goto Done;	// Not enough input in this buffer.
	
	loUnit  = UTF16InSwap(utf16In+1);
	if ( (loUnit < 0xDC00) || (0xDFFF < loUnit) ) UC_Throw ( "Bad UTF-16 - missing low surrogate", kXMPErr_BadParam );
	
	unitCount = 2;
	cp = (((hiUnit & 0x3FF) << 10) | (loUnit & 0x3FF)) + 0x10000;

	*cpOut = cp;	// ! Don't put after Done, don't write if no input.
	
Done:
	*utf16Read = unitCount;
	return;
	
}	// CodePoint_from_UTF16Swp_Surrogate

// =================================================================================================

static void CodePoint_from_UTF16Swp ( const UTF16Unit * utf16In, const size_t utf16Len, UTF32Unit * cpOut, size_t * utf16Read )
{
	UTF16Unit inUnit;	// ! Don't read until we know there is input.
	size_t unitCount = 0;

	UC_Assert ( (utf16In != 0) && (cpOut != 0) && (utf16Read != 0) );
	if ( utf16Len == 0 ) goto Done;
	inUnit = UTF16InSwap(utf16In);
	if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) goto SurrogatePair;	// ! Force linear execution path for the BMP.

	unitCount = 1;
	*cpOut = inUnit;	// ! Don't put after Done, don't write if no input.
	
Done:
	*utf16Read = unitCount;
	return;

SurrogatePair:
	CodePoint_from_UTF16Swp_Surrogate ( utf16In, utf16Len, cpOut, utf16Read );
	return;
	
}	// CodePoint_from_UTF16Swp

// =================================================================================================

static void UTF8_to_UTF16Swp ( const UTF8Unit * utf8In,   const size_t utf8Len,
				               UTF16Unit *      utf16Out, const size_t utf16Len,
				               size_t *         utf8Read, size_t *     utf16Written )
{
	const UTF8Unit * utf8Pos  = utf8In;
	UTF16Unit *      utf16Pos = utf16Out;
	
	size_t utf8Left  = utf8Len;
	size_t utf16Left = utf16Len;
	
	UC_Assert ( (utf8In != 0) && (utf16Out != 0) && (utf8Read != 0) && (utf16Written != 0) );
	
	while ( (utf8Left > 0) && (utf16Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf8Left;
		if ( limit > utf16Left ) limit = utf16Left;
		for ( i = 0; i < limit; ++i ) {
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit > 0x7F ) break;
			*utf16Pos = UTF16Unit(inUnit) << 8;	// Better than: UTF16OutSwap ( utf16Pos, inUnit );
			++utf8Pos;
			++utf16Pos;
		}
		utf8Left  -= i;
		utf16Left -= i;
		
		// Do a run of non-ASCII, it copies multiple input units into 1 or 2 output units.
		while ( (utf8Left > 0) && (utf16Left > 0) ) {
			UTF32Unit cp;
			size_t len8, len16;
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit <= 0x7F ) break;
			CodePoint_from_UTF8_Multi ( utf8Pos, utf8Left, &cp, &len8 );
			if ( len8 == 0 ) goto Done;		// The input buffer ends in the middle of a character.
			if ( cp <= 0xFFFF ) {
				UTF16OutSwap ( utf16Pos, UTF16Unit(cp) );
				len16 = 1;
			} else {
				CodePoint_to_UTF16Swp_Surrogate ( cp, utf16Pos, utf16Left, &len16 );
				if ( len16 == 0 ) goto Done;	// Not enough room in the output buffer.
			}
			utf8Left  -= len8;
			utf8Pos   += len8;
			utf16Left -= len16;
			utf16Pos  += len16;
		}
	
	}

Done:	// Set the output lengths.
	*utf8Read = utf8Len - utf8Left;
	*utf16Written = utf16Len - utf16Left;
	
}	// UTF8_to_UTF16Swp

// =================================================================================================

static void UTF8_to_UTF32Swp ( const UTF8Unit *  utf8In,   const size_t utf8Len,
				               UTF32Unit *       utf32Out, const size_t utf32Len,
				               size_t *          utf8Read, size_t *     utf32Written )
{
	const UTF8Unit * utf8Pos  = utf8In;
	UTF32Unit *      utf32Pos = utf32Out;
	
	size_t utf8Left  = utf8Len;
	size_t utf32Left = utf32Len;
	
	UC_Assert ( (utf8In != 0) && (utf32Out != 0) && (utf8Read != 0) && (utf32Written != 0) );
	
	while ( (utf8Left > 0) && (utf32Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf8Left;
		if ( limit > utf32Left ) limit = utf32Left;
		for ( i = 0; i < limit; ++i ) {
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit > 0x7F ) break;
			*utf32Pos = UTF32Unit(inUnit) << 24;	// Better than: UTF32OutSwap ( utf32Pos, inUnit );
			++utf8Pos;
			++utf32Pos;
		}
		utf8Left -= i;
		utf32Left -= i;
		
		// Do a run of non-ASCII, it copies variable input into 1 output unit.
		while ( (utf8Left > 0) && (utf32Left > 0) ) {
			size_t len;
			UTF32Unit cp;
			UTF8Unit inUnit = *utf8Pos;
			if ( inUnit <= 0x7F ) break;
			CodePoint_from_UTF8_Multi ( utf8Pos, utf8Left, &cp, &len );
			if ( len == 0 ) goto Done;	// The input buffer ends in the middle of a character.
			UTF32OutSwap ( utf32Pos, cp );
			utf8Left  -= len;
			utf8Pos   += len;
			utf32Left -= 1;
			utf32Pos  += 1;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf8Read = utf8Len - utf8Left;
	*utf32Written = utf32Len - utf32Left;
	
}	// UTF8_to_UTF32Swp

// =================================================================================================

static void UTF16Swp_to_UTF8 ( const UTF16Unit * utf16In,   const size_t utf16Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf16Read, size_t *     utf8Written )
{
	const UTF16Unit * utf16Pos = utf16In;
	UTF8Unit *        utf8Pos  = utf8Out;
	
	size_t utf16Left = utf16Len;
	size_t utf8Left  = utf8Len;
	
	UC_Assert ( (utf16In != 0) && (utf8Out != 0) && (utf16Read != 0) && (utf8Written != 0) );
	
	while ( (utf16Left > 0) && (utf8Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf16Left;
		if ( limit > utf8Left ) limit = utf8Left;
		for ( i = 0; i < limit; ++i ) {
			UTF16Unit inUnit = UTF16InSwap(utf16Pos);
			if ( inUnit > 0x7F ) break;
			*utf8Pos = UTF8Unit(inUnit);
			++utf16Pos;
			++utf8Pos;
		}
		utf16Left -= i;
		utf8Left  -= i;
		
		// Do a run of non-ASCII inside the BMP, it copies 1 input unit into multiple output units.
		while ( (utf16Left > 0) && (utf8Left > 0) ) {
			size_t len8;
			UTF16Unit inUnit = UTF16InSwap(utf16Pos);
			if ( inUnit <= 0x7F ) break;
			if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) break;
			CodePoint_to_UTF8_Multi ( inUnit, utf8Pos, utf8Left, &len8 );
			if ( len8 == 0 ) goto Done;		// Not enough room in the output buffer.
			utf16Left -= 1;
			utf16Pos  += 1;
			utf8Left  -= len8;
			utf8Pos   += len8;
		}
		
		// Do a run of surrogate pairs, it copies 2 input units into multiple output units.
		while ( (utf16Left > 0) && (utf8Left > 0) ) {
			UTF32Unit cp;
			size_t len16, len8;
			UTF16Unit inUnit = UTF16InSwap(utf16Pos);
			if ( (inUnit < 0xD800) || (0xDFFF < inUnit) ) break;
			CodePoint_from_UTF16Swp_Surrogate ( utf16Pos, utf16Left, &cp, &len16 );
			if ( len16 == 0 ) goto Done;	// The input buffer ends in the middle of a surrogate pair.
			UC_Assert ( len16 == 2 );
			CodePoint_to_UTF8_Multi ( cp, utf8Pos, utf8Left, &len8 );
			if ( len8 == 0 ) goto Done;		// Not enough room in the output buffer.
			utf16Left -= len16;
			utf16Pos  += len16;
			utf8Left  -= len8;
			utf8Pos   += len8;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf16Read = utf16Len - utf16Left;
	*utf8Written = utf8Len - utf8Left;
	
}	// UTF16Swp_to_UTF8

// =================================================================================================

static void UTF32Swp_to_UTF8 ( const UTF32Unit * utf32In,   const size_t utf32Len,
				               UTF8Unit *        utf8Out,   const size_t utf8Len,
				               size_t *          utf32Read, size_t *     utf8Written )
{
	const UTF32Unit * utf32Pos = utf32In;
	UTF8Unit *        utf8Pos  = utf8Out;
	
	size_t utf32Left = utf32Len;
	size_t utf8Left  = utf8Len;
	
	UC_Assert ( (utf32In != 0) && (utf8Out != 0) && (utf32Read != 0) && (utf8Written != 0) );
	
	while ( (utf32Left > 0) && (utf8Left > 0) ) {
	
		// Do a run of ASCII, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf32Left;
		if ( limit > utf8Left ) limit = utf8Left;
		for ( i = 0; i < limit; ++i ) {
			UTF32Unit cp = UTF32InSwap(utf32Pos);
			if ( cp > 0x7F ) break;
			*utf8Pos = UTF8Unit(cp);
			++utf32Pos;
			++utf8Pos;
		}
		utf32Left -= i;
		utf8Left  -= i;
		
		// Do a run of non-ASCII, it copies 1 input unit into multiple output units.
		while ( (utf32Left > 0) && (utf8Left > 0) ) {
			size_t len;
			UTF32Unit cp = UTF32InSwap(utf32Pos);
			if ( cp <= 0x7F ) break;
			CodePoint_to_UTF8_Multi ( cp, utf8Pos, utf8Left, &len );
			if ( len == 0 ) goto Done;	// Not enough room in the output buffer.
			utf32Left -= 1;
			utf32Pos  += 1;
			utf8Left  -= len;
			utf8Pos   += len;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf32Read = utf32Len - utf32Left;
	*utf8Written = utf8Len - utf8Left;
	
}	// UTF32Swp_to_UTF8

// =================================================================================================

static void UTF16Swp_to_UTF32Swp ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written )
{
	const UTF16Unit * utf16Pos = utf16In;
	UTF32Unit *       utf32Pos = utf32Out;
	
	size_t utf16Left = utf16Len;
	size_t utf32Left = utf32Len;
	
	UC_Assert ( (utf16In != 0) && (utf32Out != 0) && (utf16Read != 0) && (utf32Written != 0) );
	
	while ( (utf16Left > 0) && (utf32Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf16Left;
		if ( limit > utf32Left ) limit = utf32Left;
		for ( i = 0; i < limit; ++i ) {
			UTF16Unit inUnit = UTF16InSwap(utf16Pos);
			if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) break;
			*utf32Pos = UTF32Unit(*utf16Pos) << 16;	// Better than: UTF32OutSwap ( utf32Pos, inUnit );
			++utf16Pos;
			++utf32Pos;
		}
		utf16Left -= i;
		utf32Left -= i;
		
		// Do a run of surrogate pairs, it copies 2 input units into 1 output unit.
		while ( (utf16Left > 0) && (utf32Left > 0) ) {
			size_t len;
			UTF32Unit cp;
			UTF16Unit inUnit = UTF16InSwap(utf16Pos);
			if ( (inUnit < 0xD800) || (0xDFFF < inUnit) ) break;
			CodePoint_from_UTF16Swp_Surrogate ( utf16Pos, utf16Left, &cp, &len );
			if ( len == 0 ) goto Done;	// The input buffer ends in the middle of a surrogate pair.
			UTF32OutSwap ( utf32Pos, cp );
			UC_Assert ( len == 2 );
			utf16Left -= len;
			utf16Pos  += len;
			utf32Left -= 1;
			utf32Pos  += 1;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf16Read = utf16Len - utf16Left;
	*utf32Written = utf32Len - utf32Left;
	
}	// UTF16Swp_to_UTF32Swp

// =================================================================================================

static void UTF32Swp_to_UTF16Swp ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written )
{
	const UTF32Unit * utf32Pos = utf32In;
	UTF16Unit *       utf16Pos = utf16Out;
	
	size_t utf32Left = utf32Len;
	size_t utf16Left = utf16Len;
	
	const size_t k32to16Offset = swap32to16Offset;	// ! Make sure compiler treats as an invariant.
	
	UC_Assert ( (utf32In != 0) && (utf16Out != 0) && (utf32Read != 0) && (utf16Written != 0) );
	
	while ( (utf32Left > 0) && (utf16Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf32Left;
		if ( limit > utf16Left ) limit = utf16Left;
		for ( i = 0; i < limit; ++i ) {
			UTF32Unit inUnit = UTF32InSwap(utf32Pos);
			if ( inUnit > 0xFFFF ) break;
			*utf16Pos = *(((UTF16Unit*)utf32Pos) + k32to16Offset);	// Better than: UTF16OutSwap ( utf16Pos, UTF16Unit(inUnit) );
			++utf32Pos;
			++utf16Pos;
		}
		utf32Left -= i;
		utf16Left -= i;
		
		// Do a run of non-BMP, it copies 1 input unit into 2 output units.
		while ( (utf32Left > 0) && (utf16Left > 0) ) {
			size_t len;
			UTF32Unit inUnit = UTF32InSwap(utf32Pos);
			if ( inUnit <= 0xFFFF ) break;
			CodePoint_to_UTF16Swp_Surrogate ( inUnit, utf16Pos, utf16Left, &len );
			if ( len == 0 ) goto Done;	// Not enough room in the output buffer.
			UC_Assert ( len == 2 );
			utf32Left -= 1;
			utf32Pos  += 1;
			utf16Left -= 2;
			utf16Pos  += 2;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf32Read = utf32Len - utf32Left;
	*utf16Written = utf16Len - utf16Left;
	
}	// UTF32Swp_to_UTF16Swp

// =================================================================================================

static void UTF16Nat_to_UTF32Swp ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written )
{
	const UTF16Unit * utf16Pos = utf16In;
	UTF32Unit *       utf32Pos = utf32Out;
	
	size_t utf16Left = utf16Len;
	size_t utf32Left = utf32Len;
	
	UC_Assert ( (utf16In != 0) && (utf32Out != 0) && (utf16Read != 0) && (utf32Written != 0) );
	
	while ( (utf16Left > 0) && (utf32Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf16Left;
		if ( limit > utf32Left ) limit = utf32Left;
		for ( i = 0; i < limit; ++i ) {
			UTF16Unit inUnit = *utf16Pos;
			if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) break;
			UTF32OutSwap ( utf32Pos, inUnit );
			++utf16Pos;
			++utf32Pos;
		}
		utf16Left -= i;
		utf32Left -= i;
		
		// Do a run of surrogate pairs, it copies 2 input units into 1 output unit.
		while ( (utf16Left > 0) && (utf32Left > 0) ) {
			size_t len;
			UTF32Unit cp;
			UTF16Unit inUnit = *utf16Pos;
			if ( (inUnit < 0xD800) || (0xDFFF < inUnit) ) break;
			CodePoint_from_UTF16Nat_Surrogate ( utf16Pos, utf16Left, &cp, &len );
			if ( len == 0 ) goto Done;	// The input buffer ends in the middle of a surrogate pair.
			UC_Assert ( len == 2 );
			UTF32OutSwap ( utf32Pos, cp );
			utf16Left -= len;
			utf16Pos  += len;
			utf32Left -= 1;
			utf32Pos  += 1;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf16Read = utf16Len - utf16Left;
	*utf32Written = utf32Len - utf32Left;
	
}	// UTF16Nat_to_UTF32Swp

// =================================================================================================

static void UTF16Swp_to_UTF32Nat ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                   UTF32Unit *       utf32Out,  const size_t utf32Len,
				                   size_t *          utf16Read, size_t *     utf32Written )
{
	const UTF16Unit * utf16Pos = utf16In;
	UTF32Unit *       utf32Pos = utf32Out;
	
	size_t utf16Left = utf16Len;
	size_t utf32Left = utf32Len;
	
	UC_Assert ( (utf16In != 0) && (utf32Out != 0) && (utf16Read != 0) && (utf32Written != 0) );
	
	while ( (utf16Left > 0) && (utf32Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf16Left;
		if ( limit > utf32Left ) limit = utf32Left;
		for ( i = 0; i < limit; ++i ) {
			UTF16Unit inUnit = UTF16InSwap(utf16Pos);
			if ( (0xD800 <= inUnit) && (inUnit <= 0xDFFF) ) break;
			*utf32Pos = inUnit;
			++utf16Pos;
			++utf32Pos;
		}
		utf16Left -= i;
		utf32Left -= i;
		
		// Do a run of surrogate pairs, it copies 2 input units into 1 output unit.
		while ( (utf16Left > 0) && (utf32Left > 0) ) {
			size_t len;
			UTF16Unit inUnit = UTF16InSwap(utf16Pos);
			if ( (inUnit < 0xD800) || (0xDFFF < inUnit) ) break;
			CodePoint_from_UTF16Swp_Surrogate ( utf16Pos, utf16Left, utf32Pos, &len );
			if ( len == 0 ) goto Done;	// The input buffer ends in the middle of a surrogate pair.
			UC_Assert ( len == 2 );
			utf16Left -= len;
			utf16Pos  += len;
			utf32Left -= 1;
			utf32Pos  += 1;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf16Read = utf16Len - utf16Left;
	*utf32Written = utf32Len - utf32Left;
	
}	// UTF16Swp_to_UTF32Nat

// =================================================================================================

static void UTF32Nat_to_UTF16Swp ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written )
{
	const UTF32Unit * utf32Pos = utf32In;
	UTF16Unit *       utf16Pos = utf16Out;
	
	size_t utf32Left = utf32Len;
	size_t utf16Left = utf16Len;
	
	UC_Assert ( (utf32In != 0) && (utf16Out != 0) && (utf32Read != 0) && (utf16Written != 0) );
	
	while ( (utf32Left > 0) && (utf16Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf32Left;
		if ( limit > utf16Left ) limit = utf16Left;
		for ( i = 0; i < limit; ++i ) {
			UTF32Unit inUnit = *utf32Pos;
			if ( inUnit > 0xFFFF ) break;
			UTF16OutSwap ( utf16Pos, UTF16Unit(inUnit) );
			++utf32Pos;
			++utf16Pos;
		}
		utf32Left -= i;
		utf16Left -= i;
		
		// Do a run of non-BMP, it copies 1 input unit into 2 output units.
		while ( (utf32Left > 0) && (utf16Left > 0) ) {
			size_t len;
			UTF32Unit inUnit = *utf32Pos;
			if ( inUnit <= 0xFFFF ) break;
			CodePoint_to_UTF16Swp_Surrogate ( inUnit, utf16Pos, utf16Left, &len );
			if ( len == 0 ) goto Done;	// Not enough room in the output buffer.
			UC_Assert ( len == 2 );
			utf32Left -= 1;
			utf32Pos  += 1;
			utf16Left -= 2;
			utf16Pos  += 2;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf32Read = utf32Len - utf32Left;
	*utf16Written = utf16Len - utf16Left;
	
}	// UTF32Nat_to_UTF16Swp

// =================================================================================================

static void UTF32Swp_to_UTF16Nat ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                   UTF16Unit *       utf16Out,  const size_t utf16Len,
				                   size_t *          utf32Read, size_t *     utf16Written )
{
	const UTF32Unit * utf32Pos = utf32In;
	UTF16Unit *       utf16Pos = utf16Out;
	
	size_t utf32Left = utf32Len;
	size_t utf16Left = utf16Len;
	
	UC_Assert ( (utf32In != 0) && (utf16Out != 0) && (utf32Read != 0) && (utf16Written != 0) );
	
	while ( (utf32Left > 0) && (utf16Left > 0) ) {
	
		// Do a run of BMP, it copies 1 input unit into 1 output unit.
		size_t i, limit = utf32Left;
		if ( limit > utf16Left ) limit = utf16Left;
		for ( i = 0; i < limit; ++i ) {
			UTF32Unit inUnit = UTF32InSwap(utf32Pos);
			if ( inUnit > 0xFFFF ) break;
			*utf16Pos = UTF16Unit(inUnit);
			++utf32Pos;
			++utf16Pos;
		}
		utf32Left -= i;
		utf16Left -= i;
		
		// Do a run of non-BMP, it copies 1 input unit into 2 output units.
		while ( (utf32Left > 0) && (utf16Left > 0) ) {
			size_t len;
			UTF32Unit inUnit = UTF32InSwap(utf32Pos);
			if ( inUnit <= 0xFFFF ) break;
			CodePoint_to_UTF16Nat_Surrogate ( inUnit, utf16Pos, utf16Left, &len );
			if ( len == 0 ) goto Done;	// Not enough room in the output buffer.
			UC_Assert ( len == 2 );
			utf32Left -= 1;
			utf32Pos  += 1;
			utf16Left -= 2;
			utf16Pos  += 2;
		}
	
	}
	
Done:	// Set the output lengths.
	*utf32Read = utf32Len - utf32Left;
	*utf16Written = utf16Len - utf16Left;
	
}	// UTF32Swp_to_UTF16Nat

// =================================================================================================
