// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"

#include "XMPIterator.hpp"
#include "client-glue/WXMPIterator.hpp"

#if XMP_WinBuild
#   ifdef _MSC_VER
        #pragma warning ( disable : 4101 ) // unreferenced local variable
        #pragma warning ( disable : 4189 ) // local variable is initialized but not referenced
        #pragma warning ( disable : 4800 ) // forcing value to bool 'true' or 'false' (performance warning)
        #if XMP_DebugBuild
            #pragma warning ( disable : 4297 ) // function assumed not to throw an exception but does
#       endif
#   endif
#endif

#if __cplusplus
extern "C" {
#endif

// =================================================================================================
// CTor/DTor Wrappers
// ==================

void
WXMPIterator_PropCTor_1 ( XMPMetaRef     xmpRef,
                          XMP_StringPtr  schemaNS,
                          XMP_StringPtr  propName,
                          XMP_OptionBits options,
                          WXMP_Result *  wResult )
{
    XMP_ENTER_WRAPPER ( "WXMPIterator_PropCTor_1" )

		if ( schemaNS == 0 ) schemaNS = "";
		if ( propName == 0 ) propName = "";

		const XMPMeta & xmpObj = WtoXMPMeta_Ref ( xmpRef );
		XMPIterator *   iter   = new XMPIterator ( xmpObj, schemaNS, propName, options );
		++iter->clientRefs;
		XMP_Assert ( iter->clientRefs == 1 );
		wResult->ptrResult = XMPIteratorRef ( iter );

    XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPIterator_TableCTor_1 ( XMP_StringPtr  schemaNS,
                           XMP_StringPtr  propName,
                           XMP_OptionBits options,
                           WXMP_Result *  wResult )
{
    XMP_ENTER_WRAPPER ( "WXMPIterator_TableCTor_1" )

		if ( schemaNS == 0 ) schemaNS = "";
		if ( propName == 0 ) propName = "";

		XMPIterator * iter = new XMPIterator ( schemaNS, propName, options );
		++iter->clientRefs;
		XMP_Assert ( iter->clientRefs == 1 );
		wResult->ptrResult = XMPIteratorRef ( iter );

    XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPIterator_IncrementRefCount_1 ( XMPIteratorRef iterRef )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER ( "WXMPIterator_IncrementRefCount_1" )

		XMPIterator * thiz = (XMPIterator*)iterRef;
		
		++thiz->clientRefs;
		XMP_Assert ( thiz->clientRefs > 1 );

	XMP_EXIT_WRAPPER_NO_THROW
}

// -------------------------------------------------------------------------------------------------

void
WXMPIterator_DecrementRefCount_1 ( XMPIteratorRef iterRef )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER ( "WXMPIterator_DecrementRefCount_1" )

		XMPIterator * thiz = (XMPIterator*)iterRef;
		
		XMP_Assert ( thiz->clientRefs > 0 );
		--thiz->clientRefs;
		if ( thiz->clientRefs <= 0 ) delete ( thiz );

	XMP_EXIT_WRAPPER_NO_THROW
}

// -------------------------------------------------------------------------------------------------

void
WXMPIterator_Unlock_1 ( XMP_OptionBits options )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
    XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPIterator_Unlock_1" )

		XMPIterator::Unlock ( options );

    XMP_EXIT_WRAPPER_NO_THROW
}

// =================================================================================================
// Class Method Wrappers
// =====================

void
WXMPIterator_Next_1 ( XMPIteratorRef   iterRef,
                      XMP_StringPtr *  schemaNS,
                      XMP_StringLen *  nsSize,
                      XMP_StringPtr *  propPath,
                      XMP_StringLen *  pathSize,
                      XMP_StringPtr *  propValue,
                      XMP_StringLen *  valueSize,
                      XMP_OptionBits * propOptions,
                      WXMP_Result *    wResult )
{
    XMP_ENTER_WRAPPER ( "WXMPIterator_Next_1" )

		if ( schemaNS == 0 ) schemaNS = &voidStringPtr;
		if ( nsSize == 0 ) nsSize = &voidStringLen;
		if ( propPath == 0 ) propPath = &voidStringPtr;
		if ( pathSize == 0 ) pathSize = &voidStringLen;
		if ( propValue == 0 ) propValue = &voidStringPtr;
		if ( valueSize == 0 ) valueSize = &voidStringLen;
		if ( propOptions == 0 ) propOptions = &voidOptionBits;

		XMPIterator * iter = WtoXMPIterator_Ptr ( iterRef );
		XMP_Bool found = iter->Next ( schemaNS, nsSize, propPath, pathSize, propValue, valueSize, propOptions );
		wResult->int32Result = found;

    XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

void
WXMPIterator_Skip_1 ( XMPIteratorRef iterRef,
                      XMP_OptionBits options,
                      WXMP_Result *  wResult )
{
    XMP_ENTER_WRAPPER ( "WXMPIterator_Skip_1" )

		XMPIterator * iter = WtoXMPIterator_Ptr ( iterRef );
		iter->Skip ( options );

    XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_UnlockIter_1 ( XMPIteratorRef iterRef,
                         XMP_OptionBits options )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
    XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_UnlockIter_1" )

		XMPIterator * iter = WtoXMPIterator_Ptr ( iterRef );
		iter->UnlockIter ( options );

    XMP_EXIT_WRAPPER_NO_THROW
}

// =================================================================================================

#if __cplusplus
} /* extern "C" */
#endif
