// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:	Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"

#include "XMPMeta.hpp"
#include "client-glue/WXMPMeta.hpp"

#if XMP_WinBuild
    #ifdef _MSC_VER
        #pragma warning ( disable : 4101 ) // unreferenced local variable
        #pragma warning ( disable : 4189 ) // local variable is initialized but not referenced
        #pragma warning ( disable : 4702 ) // unreachable code
        #pragma warning ( disable : 4800 ) // forcing value to bool 'true' or 'false' (performance warning)
        #if XMP_DebugBuild
            #pragma warning ( disable : 4297 ) // function assumed not to throw an exception but does
        #endif
    #endif
#endif

#if __cplusplus
extern "C" {
#endif

// =================================================================================================
// Init/Term Wrappers
// ==================

/* class static */ void
WXMPMeta_GetVersionInfo_1 ( XMP_VersionInfo * info )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPMeta_GetVersionInfo_1" )

		XMPMeta::GetVersionInfo ( info );

	XMP_EXIT_WRAPPER_NO_THROW
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_Initialize_1 ( WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPMeta_Initialize_1" )

		bool ok = XMPMeta::Initialize();
		wResult->int32Result = ok;

	XMP_EXIT_WRAPPER
}
// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_Terminate_1()
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPMeta_Terminate_1" )

		XMPMeta::Terminate();

	XMP_EXIT_WRAPPER_NO_THROW
}

// =================================================================================================
// CTor/DTor Wrappers
// ==================

void
WXMPMeta_CTor_1 ( WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_CTor_1" )

		XMPMeta * xmpObj = new XMPMeta();
		++xmpObj->clientRefs;
		XMP_Assert ( xmpObj->clientRefs == 1 );
		wResult->ptrResult = XMPMetaRef ( xmpObj );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_IncrementRefCount_1 ( XMPMetaRef xmpRef )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER ( "WXMPMeta_IncrementRefCount_1" )

		XMPMeta * thiz = (XMPMeta*)xmpRef;
		
		++thiz->clientRefs;
		XMP_Assert ( thiz->clientRefs > 0 );

	XMP_EXIT_WRAPPER_NO_THROW
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DecrementRefCount_1 ( XMPMetaRef xmpRef )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER ( "WXMPMeta_DecrementRefCount_1" )

		XMPMeta * thiz = (XMPMeta*)xmpRef;
		
		XMP_Assert ( thiz->clientRefs > 0 );
		--thiz->clientRefs;
		if ( thiz->clientRefs <= 0 ) delete ( thiz );

	XMP_EXIT_WRAPPER_NO_THROW
}

// =================================================================================================
// Class Static Wrappers
// =====================
//
// These are DLL-entry wrappers for class-static functions. They all follow a simple pattern:
//
//		try
//			acquire toolbox lock
//			validate parameters
//			call through to the implementation
//			retain toolbox lock if necessary
//		catch anything and return an appropriate XMP_Error object
//		return null (no error if we get to here)
//
// The toolbox lock is acquired through a local wrapper object that automatically unlocks when the
// try-block is exited. The lock must be retained if the function is returning a string result. The
// output string is owned by the toolkit, the client must copy the string then release the lock.
// The lock used here is the overall toolkit lock. For simplicity at this time the lock is a simple
// mutual exclusion lock, we do not allow multiple concurrent readers.
//
// The one exception to this model is UnlockToolkit. It does not acquire the toolkit lock since this
// is the function the client calls to release the lock after copying an output string!
//
// =================================================================================================

/* class static */ void
WXMPMeta_GetGlobalOptions_1 ( WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetGlobalOptions_1" )

		XMP_OptionBits options = XMPMeta::GetGlobalOptions();
		wResult->int32Result = options;

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_SetGlobalOptions_1 ( XMP_OptionBits options,
							  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetGlobalOptions_1" )

		XMPMeta::SetGlobalOptions ( options );

	XMP_EXIT_WRAPPER
}
// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_DumpNamespaces_1 ( XMP_TextOutputProc outProc,
							void *			   refCon,
							WXMP_Result *	   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DumpNamespaces_1" )

		if ( outProc == 0 ) XMP_Throw ( "Null client output routine", kXMPErr_BadParam );
		
		XMP_Status status = XMPMeta::DumpNamespaces ( outProc, refCon );
		wResult->int32Result = status;

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_DumpAliases_1 ( XMP_TextOutputProc outProc,
						 void *				refCon,
						 WXMP_Result *		wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DumpAliases_1" )

		if ( outProc == 0 ) XMP_Throw ( "Null client output routine", kXMPErr_BadParam );
		
		XMP_Status status = XMPMeta::DumpAliases ( outProc, refCon );
		wResult->int32Result = status;

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_Unlock_1 ( XMP_OptionBits options )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPMeta_Unlock_1" )

		XMPMeta::Unlock ( options );

	XMP_EXIT_WRAPPER_NO_THROW
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_RegisterNamespace_1 ( XMP_StringPtr   namespaceURI,
                               XMP_StringPtr   prefix,
                               WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_RegisterNamespace_1" )

		if ( (namespaceURI == 0) || (*namespaceURI == 0) ) XMP_Throw ( "Empty namespace URI", kXMPErr_BadSchema );
		if ( (prefix == 0) || (*prefix == 0) ) XMP_Throw ( "Empty prefix", kXMPErr_BadSchema );

		XMPMeta::RegisterNamespace ( namespaceURI, prefix );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_GetNamespacePrefix_1 ( XMP_StringPtr	namespaceURI,
								XMP_StringPtr * namespacePrefix,
								XMP_StringLen * prefixSize,
								WXMP_Result *	wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetNamespacePrefix_1" )

		if ( (namespaceURI == 0) || (*namespaceURI == 0) ) XMP_Throw ( "Empty namespace URI", kXMPErr_BadSchema );

		if ( namespacePrefix == 0 ) namespacePrefix = &voidStringPtr;
		if ( prefixSize == 0 ) prefixSize = &voidStringLen;

		bool found = XMPMeta::GetNamespacePrefix ( namespaceURI, namespacePrefix, prefixSize );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_GetNamespaceURI_1 ( XMP_StringPtr	 namespacePrefix,
							 XMP_StringPtr * namespaceURI,
							 XMP_StringLen * uriSize,
							 WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetNamespaceURI_1" )

		if ( (namespacePrefix == 0) || (*namespacePrefix == 0) ) XMP_Throw ( "Empty namespace prefix", kXMPErr_BadSchema );

		if ( namespaceURI == 0 ) namespaceURI = &voidStringPtr;
		if ( uriSize == 0 ) uriSize = &voidStringLen;
	   
		bool found = XMPMeta::GetNamespaceURI ( namespacePrefix, namespaceURI, uriSize );
		wResult->int32Result = found;

	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_DeleteNamespace_1 ( XMP_StringPtr namespaceURI,
							 WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DeleteNamespace_1" )

		if ( (namespaceURI == 0) || (*namespaceURI == 0) ) XMP_Throw ( "Empty namespace URI", kXMPErr_BadSchema );

		XMPMeta::DeleteNamespace ( namespaceURI );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_RegisterAlias_1 ( XMP_StringPtr  aliasNS,
						   XMP_StringPtr  aliasProp,
						   XMP_StringPtr  actualNS,
						   XMP_StringPtr  actualProp,
						   XMP_OptionBits arrayForm,
						   WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_RegisterAlias_1" )

		if ( (aliasNS == 0) || (*aliasNS == 0) ) XMP_Throw ( "Empty alias namespace URI", kXMPErr_BadSchema );
		if ( (aliasProp == 0) || (*aliasProp == 0) ) XMP_Throw ( "Empty alias property name", kXMPErr_BadXPath );
		if ( (actualNS == 0) || (*actualNS == 0) ) XMP_Throw ( "Empty actual namespace URI", kXMPErr_BadSchema );
		if ( (actualProp == 0) || (*actualProp == 0) ) XMP_Throw ( "Empty actual property name", kXMPErr_BadXPath );

		XMPMeta::RegisterAlias ( aliasNS, aliasProp, actualNS, actualProp, arrayForm );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_ResolveAlias_1 ( XMP_StringPtr	   aliasNS,
						  XMP_StringPtr	   aliasProp,
						  XMP_StringPtr *  actualNS,
						  XMP_StringLen *  nsSize,
						  XMP_StringPtr *  actualProp,
						  XMP_StringLen *  propSize,
						  XMP_OptionBits * arrayForm,
						  WXMP_Result *	   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_ResolveAlias_1" )
	
		if ( (aliasNS == 0) || (*aliasNS == 0) ) XMP_Throw ( "Empty alias namespace URI", kXMPErr_BadSchema );
		if ( (aliasProp == 0) || (*aliasProp == 0) ) XMP_Throw ( "Empty alias property name", kXMPErr_BadXPath );
	   
		if ( actualNS == 0 ) actualNS = &voidStringPtr;
		if ( nsSize == 0 ) nsSize = &voidStringLen;
		if ( actualProp == 0 ) actualProp = &voidStringPtr;
		if ( propSize == 0 ) propSize = &voidStringLen;
		if ( arrayForm == 0 ) arrayForm = &voidOptionBits;
		
		bool found = XMPMeta::ResolveAlias ( aliasNS, aliasProp, actualNS, nsSize, actualProp, propSize, arrayForm );
		wResult->int32Result = found;

	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_DeleteAlias_1 ( XMP_StringPtr aliasNS,
						 XMP_StringPtr aliasProp,
						 WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DeleteAlias_1" )

		if ( (aliasNS == 0) || (*aliasNS == 0) ) XMP_Throw ( "Empty alias namespace URI", kXMPErr_BadSchema );
		if ( (aliasProp == 0) || (*aliasProp == 0) ) XMP_Throw ( "Empty alias property name", kXMPErr_BadXPath );

		XMPMeta::DeleteAlias ( aliasNS, aliasProp );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

/* class static */ void
WXMPMeta_RegisterStandardAliases_1 ( XMP_StringPtr schemaNS,
									 WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_RegisterStandardAliases_1" )

		if ( schemaNS == 0 ) schemaNS = "";

		XMPMeta::RegisterStandardAliases ( schemaNS );

	XMP_EXIT_WRAPPER
}

// =================================================================================================
// Class Method Wrappers
// =====================
//
// These are DLL-entry wrappers for the methods. They all follow a simple pattern:
//
//		validate parameters
//		try
//			acquire object lock
//			call through to the implementation
//			retain object lock if necessary
//		catch anything and return an appropriate XMP_Error object
//		return null (no error if we get to here)
//
// The object lock is acquired through a local wrapper object that automatically unlocks when the
// try-block is exited. The lock must be retained if the function is returning a string result. The
// output string is owned by the object, the client must copy the string then release the lock. The
// lock used here is the per-object lock. For simplicity at this time the lock is a simple mutual
// exclusion lock, we do not allow multiple concurrent readers.
//
// The one exception to this model is UnlockObject. It does not acquire the object lock since this
// is the function the client calls to release the lock after copying an output string!
//
// =================================================================================================

void
WXMPMeta_GetProperty_1 ( XMPMetaRef		  xmpRef,
						 XMP_StringPtr	  schemaNS,
						 XMP_StringPtr	  propName,
						 XMP_StringPtr *  propValue,
						 XMP_StringLen *  valueSize,
						 XMP_OptionBits * options,
						 WXMP_Result *	  wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetProperty_1" )
	
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );
		
		if ( propValue == 0 ) propValue = &voidStringPtr;
		if ( valueSize == 0 ) valueSize = &voidStringLen;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetProperty ( schemaNS, propName, propValue, valueSize, options );
		wResult->int32Result = found;

	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetArrayItem_1 ( XMPMetaRef	   xmpRef,
						  XMP_StringPtr	   schemaNS,
						  XMP_StringPtr	   arrayName,
						  XMP_Index		   itemIndex,
						  XMP_StringPtr *  itemValue,
						  XMP_StringLen *  valueSize,
						  XMP_OptionBits * options,
						  WXMP_Result *	   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetArrayItem_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		
		if ( itemValue == 0 ) itemValue = &voidStringPtr;
		if ( valueSize == 0 ) valueSize = &voidStringLen;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetArrayItem ( schemaNS, arrayName, itemIndex, itemValue, valueSize, options );
		wResult->int32Result = found;

	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetStructField_1 ( XMPMetaRef		 xmpRef,
							XMP_StringPtr	 schemaNS,
							XMP_StringPtr	 structName,
							XMP_StringPtr	 fieldNS,
							XMP_StringPtr	 fieldName,
							XMP_StringPtr *	 fieldValue,
							XMP_StringLen *	 valueSize,
							XMP_OptionBits * options,
							WXMP_Result *	 wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetStructField_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (structName == 0) || (*structName == 0) ) XMP_Throw ( "Empty struct name", kXMPErr_BadXPath );
		if ( (fieldNS == 0) || (*fieldNS == 0) ) XMP_Throw ( "Empty field namespace URI", kXMPErr_BadSchema );
		if ( (fieldName == 0) || (*fieldName == 0) ) XMP_Throw ( "Empty field name", kXMPErr_BadXPath );
		
		if ( fieldValue == 0 ) fieldValue = &voidStringPtr;
		if ( valueSize == 0 ) valueSize = &voidStringLen;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetStructField ( schemaNS, structName, fieldNS, fieldName, fieldValue, valueSize, options );
		wResult->int32Result = found;

	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetQualifier_1 ( XMPMetaRef	   xmpRef,
						  XMP_StringPtr	   schemaNS,
						  XMP_StringPtr	   propName,
						  XMP_StringPtr	   qualNS,
						  XMP_StringPtr	   qualName,
						  XMP_StringPtr *  qualValue,
						  XMP_StringLen *  valueSize,
						  XMP_OptionBits * options,
						  WXMP_Result *	   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetQualifier_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );
		if ( (qualNS == 0) || (*qualNS == 0) ) XMP_Throw ( "Empty qualifier namespace URI", kXMPErr_BadSchema );
		if ( (qualName == 0) || (*qualName == 0) ) XMP_Throw ( "Empty qualifier name", kXMPErr_BadXPath );
		
		if ( qualValue == 0 ) qualValue = &voidStringPtr;
		if ( valueSize == 0 ) valueSize = &voidStringLen;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetQualifier ( schemaNS, propName, qualNS, qualName, qualValue, valueSize, options );
		wResult->int32Result = found;

	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetProperty_1 ( XMPMetaRef		xmpRef,
						 XMP_StringPtr	schemaNS,
						 XMP_StringPtr	propName,
						 XMP_StringPtr	propValue,
						 XMP_OptionBits options,
						 WXMP_Result *	wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetProperty_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetProperty ( schemaNS, propName, propValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetArrayItem_1 ( XMPMetaRef	 xmpRef,
						  XMP_StringPtr	 schemaNS,
						  XMP_StringPtr	 arrayName,
						  XMP_Index		 itemIndex,
						  XMP_StringPtr	 itemValue,
						  XMP_OptionBits options,
						  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetArrayItem_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetArrayItem ( schemaNS, arrayName, itemIndex, itemValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_AppendArrayItem_1 ( XMPMetaRef		xmpRef,
							 XMP_StringPtr	schemaNS,
							 XMP_StringPtr	arrayName,
							 XMP_OptionBits arrayOptions,
							 XMP_StringPtr	itemValue,
							 XMP_OptionBits options,
							 WXMP_Result *	wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_AppendArrayItem_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->AppendArrayItem ( schemaNS, arrayName, arrayOptions, itemValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetStructField_1 ( XMPMetaRef	   xmpRef,
							XMP_StringPtr  schemaNS,
							XMP_StringPtr  structName,
							XMP_StringPtr  fieldNS,
							XMP_StringPtr  fieldName,
							XMP_StringPtr  fieldValue,
							XMP_OptionBits options,
							WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetStructField_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (structName == 0) || (*structName == 0) ) XMP_Throw ( "Empty struct name", kXMPErr_BadXPath );
		if ( (fieldNS == 0) || (*fieldNS == 0) ) XMP_Throw ( "Empty field namespace URI", kXMPErr_BadSchema );
		if ( (fieldName == 0) || (*fieldName == 0) ) XMP_Throw ( "Empty field name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetStructField ( schemaNS, structName, fieldNS, fieldName, fieldValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetQualifier_1 ( XMPMetaRef	 xmpRef,
						  XMP_StringPtr	 schemaNS,
						  XMP_StringPtr	 propName,
						  XMP_StringPtr	 qualNS,
						  XMP_StringPtr	 qualName,
						  XMP_StringPtr	 qualValue,
						  XMP_OptionBits options,
						  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetQualifier_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );
		if ( (qualNS == 0) || (*qualNS == 0) ) XMP_Throw ( "Empty qualifier namespace URI", kXMPErr_BadSchema );
		if ( (qualName == 0) || (*qualName == 0) ) XMP_Throw ( "Empty qualifier name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetQualifier ( schemaNS, propName, qualNS, qualName, qualValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DeleteProperty_1 ( XMPMetaRef	  xmpRef,
							XMP_StringPtr schemaNS,
							XMP_StringPtr propName,
							WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DeleteProperty_1" )
 
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->DeleteProperty ( schemaNS, propName );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DeleteArrayItem_1 ( XMPMetaRef	   xmpRef,
							 XMP_StringPtr schemaNS,
							 XMP_StringPtr arrayName,
							 XMP_Index	   itemIndex,
							 WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DeleteArrayItem_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->DeleteArrayItem ( schemaNS, arrayName, itemIndex );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DeleteStructField_1 ( XMPMetaRef	 xmpRef,
							   XMP_StringPtr schemaNS,
							   XMP_StringPtr structName,
							   XMP_StringPtr fieldNS,
							   XMP_StringPtr fieldName,
							   WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DeleteStructField_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (structName == 0) || (*structName == 0) ) XMP_Throw ( "Empty struct name", kXMPErr_BadXPath );
		if ( (fieldNS == 0) || (*fieldNS == 0) ) XMP_Throw ( "Empty field namespace URI", kXMPErr_BadSchema );
		if ( (fieldName == 0) || (*fieldName == 0) ) XMP_Throw ( "Empty field name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->DeleteStructField ( schemaNS, structName, fieldNS, fieldName );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DeleteQualifier_1 ( XMPMetaRef	   xmpRef,
							 XMP_StringPtr schemaNS,
							 XMP_StringPtr propName,
							 XMP_StringPtr qualNS,
							 XMP_StringPtr qualName,
							 WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DeleteQualifier_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );
		if ( (qualNS == 0) || (*qualNS == 0) ) XMP_Throw ( "Empty qualifier namespace URI", kXMPErr_BadSchema );
		if ( (qualName == 0) || (*qualName == 0) ) XMP_Throw ( "Empty qualifier name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->DeleteQualifier ( schemaNS, propName, qualNS, qualName );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DoesPropertyExist_1 ( XMPMetaRef	 xmpRef,
							   XMP_StringPtr schemaNS,
							   XMP_StringPtr propName,
							   WXMP_Result * wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DoesPropertyExist_1" )
	
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.DoesPropertyExist ( schemaNS, propName );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DoesArrayItemExist_1 ( XMPMetaRef	  xmpRef,
								XMP_StringPtr schemaNS,
								XMP_StringPtr arrayName,
								XMP_Index	  itemIndex,
								WXMP_Result * wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DoesArrayItemExist_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.DoesArrayItemExist ( schemaNS, arrayName, itemIndex );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DoesStructFieldExist_1 ( XMPMetaRef	xmpRef,
								  XMP_StringPtr schemaNS,
								  XMP_StringPtr structName,
								  XMP_StringPtr fieldNS,
								  XMP_StringPtr fieldName,
								  WXMP_Result * wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DoesStructFieldExist_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (structName == 0) || (*structName == 0) ) XMP_Throw ( "Empty struct name", kXMPErr_BadXPath );
		if ( (fieldNS == 0) || (*fieldNS == 0) ) XMP_Throw ( "Empty field namespace URI", kXMPErr_BadSchema );
		if ( (fieldName == 0) || (*fieldName == 0) ) XMP_Throw ( "Empty field name", kXMPErr_BadXPath );

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.DoesStructFieldExist ( schemaNS, structName, fieldNS, fieldName );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DoesQualifierExist_1 ( XMPMetaRef	  xmpRef,
								XMP_StringPtr schemaNS,
								XMP_StringPtr propName,
								XMP_StringPtr qualNS,
								XMP_StringPtr qualName,
								WXMP_Result * wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DoesQualifierExist_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );
		if ( (qualNS == 0) || (*qualNS == 0) ) XMP_Throw ( "Empty qualifier namespace URI", kXMPErr_BadSchema );
		if ( (qualName == 0) || (*qualName == 0) ) XMP_Throw ( "Empty qualifier name", kXMPErr_BadXPath );

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.DoesQualifierExist ( schemaNS, propName, qualNS, qualName );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetLocalizedText_1 ( XMPMetaRef	   xmpRef,
							  XMP_StringPtr	   schemaNS,
							  XMP_StringPtr	   arrayName,
							  XMP_StringPtr	   genericLang,
							  XMP_StringPtr	   specificLang,
							  XMP_StringPtr *  actualLang,
							  XMP_StringLen *  langSize,
							  XMP_StringPtr *  itemValue,
							  XMP_StringLen *  valueSize,
							  XMP_OptionBits * options,
							  WXMP_Result *	   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetLocalizedText_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		if ( genericLang == 0 ) genericLang = "";
		if ( (specificLang == 0) ||(*specificLang == 0) ) XMP_Throw ( "Empty specific language", kXMPErr_BadParam );
		
		if ( actualLang == 0 ) actualLang = &voidStringPtr;
		if ( langSize == 0 ) langSize = &voidStringLen;
		if ( itemValue == 0 ) itemValue = &voidStringPtr;
		if ( valueSize == 0 ) valueSize = &voidStringLen;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetLocalizedText ( schemaNS, arrayName, genericLang, specificLang,
											 actualLang, langSize, itemValue, valueSize, options );
		wResult->int32Result = found;

	XMP_EXIT_WRAPPER_KEEP_LOCK ( found )
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetLocalizedText_1 ( XMPMetaRef	 xmpRef,
							  XMP_StringPtr	 schemaNS,
							  XMP_StringPtr	 arrayName,
							  XMP_StringPtr	 genericLang,
							  XMP_StringPtr	 specificLang,
							  XMP_StringPtr	 itemValue,
							  XMP_OptionBits options,
							  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetLocalizedText_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		if ( genericLang == 0 ) genericLang = "";
		if ( (specificLang == 0) ||(*specificLang == 0) ) XMP_Throw ( "Empty specific language", kXMPErr_BadParam );
		if ( itemValue == 0 ) itemValue = "";

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetLocalizedText ( schemaNS, arrayName, genericLang, specificLang, itemValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetProperty_Bool_1 ( XMPMetaRef	   xmpRef,
							  XMP_StringPtr	   schemaNS,
							  XMP_StringPtr	   propName,
							  XMP_Bool *	   propValue,
							  XMP_OptionBits * options,
							  WXMP_Result *	   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetProperty_Bool_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		if ( propValue == 0 ) propValue = &voidByte;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool value;
		bool found = meta.GetProperty_Bool ( schemaNS, propName, &value, options );
		if ( propValue != 0 ) *propValue = value;
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetProperty_Int_1 ( XMPMetaRef		  xmpRef,
							 XMP_StringPtr	  schemaNS,
							 XMP_StringPtr	  propName,
							 XMP_Int32 *	  propValue,
							 XMP_OptionBits * options,
							 WXMP_Result *	  wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetProperty_Int_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		if ( propValue == 0 ) propValue = &voidInt32;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetProperty_Int ( schemaNS, propName, propValue, options );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetProperty_Int64_1 ( XMPMetaRef		  xmpRef,
							   XMP_StringPtr	  schemaNS,
							   XMP_StringPtr	  propName,
							   XMP_Int64 *	  propValue,
							   XMP_OptionBits * options,
							   WXMP_Result *	  wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetProperty_Int64_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		if ( propValue == 0 ) propValue = &voidInt64;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetProperty_Int64 ( schemaNS, propName, propValue, options );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetProperty_Float_1 ( XMPMetaRef		xmpRef,
							   XMP_StringPtr	schemaNS,
							   XMP_StringPtr	propName,
							   double *			propValue,
							   XMP_OptionBits * options,
							   WXMP_Result *	wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetProperty_Float_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		if ( propValue == 0 ) propValue = &voidDouble;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetProperty_Float ( schemaNS, propName, propValue, options );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetProperty_Date_1 ( XMPMetaRef	   xmpRef,
							  XMP_StringPtr	   schemaNS,
							  XMP_StringPtr	   propName,
							  XMP_DateTime *   propValue,
							  XMP_OptionBits * options,
							  WXMP_Result *	   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetProperty_Date_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		if ( propValue == 0 ) propValue = &voidDateTime;
		if ( options == 0 ) options = &voidOptionBits;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		bool found = meta.GetProperty_Date ( schemaNS, propName, propValue, options );
		wResult->int32Result = found;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetProperty_Bool_1 ( XMPMetaRef	 xmpRef,
							  XMP_StringPtr	 schemaNS,
							  XMP_StringPtr	 propName,
							  XMP_Bool		 propValue,
							  XMP_OptionBits options,
							  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetProperty_Bool_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetProperty_Bool ( schemaNS, propName, propValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetProperty_Int_1 ( XMPMetaRef		xmpRef,
							 XMP_StringPtr	schemaNS,
							 XMP_StringPtr	propName,
							 XMP_Int32		propValue,
							 XMP_OptionBits options,
							 WXMP_Result *	wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetProperty_Int_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetProperty_Int ( schemaNS, propName, propValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetProperty_Int64_1 ( XMPMetaRef	  xmpRef,
							   XMP_StringPtr  schemaNS,
							   XMP_StringPtr  propName,
							   XMP_Int64	  propValue,
							   XMP_OptionBits options,
							   WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetProperty_Int64_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetProperty_Int64 ( schemaNS, propName, propValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetProperty_Float_1 ( XMPMetaRef	  xmpRef,
							   XMP_StringPtr  schemaNS,
							   XMP_StringPtr  propName,
							   double		  propValue,
							   XMP_OptionBits options,
							   WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetProperty_Float_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetProperty_Float ( schemaNS, propName, propValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetProperty_Date_1 ( XMPMetaRef		   xmpRef,
							  XMP_StringPtr		   schemaNS,
							  XMP_StringPtr		   propName,
							  const XMP_DateTime & propValue,
							  XMP_OptionBits	   options,
							  WXMP_Result *		   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetProperty_Date_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetProperty_Date ( schemaNS, propName, propValue, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_DumpObject_1 ( XMPMetaRef		   xmpRef,
						XMP_TextOutputProc outProc,
						void *			   refCon,
						WXMP_Result *	   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_DumpObject_1" )

		if ( outProc == 0 ) XMP_Throw ( "Null client output routine", kXMPErr_BadParam );
		
		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		XMP_Status status = meta.DumpObject ( outProc, refCon );
		wResult->int32Result = status;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_Sort_1 ( XMPMetaRef	xmpRef,
				  WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_Sort_1" )

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->Sort();
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_Erase_1 ( XMPMetaRef	xmpRef,
				   WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_Erase_1" )

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->Erase();
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_Clone_1 ( XMPMetaRef	  xmpRef,
				   XMP_OptionBits options,
				   WXMP_Result *  wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_Clone_1" )

		const XMPMeta & xOriginal = WtoXMPMeta_Ref ( xmpRef );
		XMPMeta * xClone = new XMPMeta;
		xOriginal.Clone ( xClone, options );
		XMP_Assert ( xClone->clientRefs == 0 );	// ! Gets incremented in TXMPMeta::Clone.
		wResult->ptrResult = xClone;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_CountArrayItems_1 ( XMPMetaRef	   xmpRef,
							 XMP_StringPtr schemaNS,
							 XMP_StringPtr arrayName,
							 WXMP_Result * wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_CountArrayItems_1" )
		
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		XMP_Index count = meta.CountArrayItems ( schemaNS, arrayName );
		wResult->int32Result = count;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_UnlockObject_1 ( XMPMetaRef	 xmpRef,
						  XMP_OptionBits options ) /* const */
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPMeta_UnlockObject_1" )
	
		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		meta.UnlockObject ( options );

	XMP_EXIT_WRAPPER_NO_THROW
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetObjectName_1 ( XMPMetaRef	   xmpRef,
						   XMP_StringPtr * namePtr,
						   XMP_StringLen * nameLen,
						   WXMP_Result *   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetObjectName_1" )

		if ( namePtr == 0 ) namePtr = &voidStringPtr;
		if ( nameLen == 0 ) nameLen = &voidStringLen;

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		meta.GetObjectName ( namePtr, nameLen );
		
	XMP_EXIT_WRAPPER_KEEP_LOCK ( true ) // ! Always keep the lock, a string is always returned!
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetObjectName_1 ( XMPMetaRef	 xmpRef,
						   XMP_StringPtr name,
						   WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetObjectName_1" )

		if ( name == 0 ) name = "";

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetObjectName ( name );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_GetObjectOptions_1 ( XMPMetaRef    xmpRef,
							  WXMP_Result * wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_GetObjectOptions_1" )

		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		XMP_OptionBits options = meta.GetObjectOptions();
		wResult->int32Result = options;
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SetObjectOptions_1 ( XMPMetaRef	 xmpRef,
							  XMP_OptionBits options,
							  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SetObjectOptions_1" )
	
		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->SetObjectOptions ( options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_ParseFromBuffer_1 ( XMPMetaRef		xmpRef,
							 XMP_StringPtr	buffer,
							 XMP_StringLen	bufferSize,
							 XMP_OptionBits options,
							 WXMP_Result *	wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_ParseFromBuffer_1" )

		XMPMeta * meta = WtoXMPMeta_Ptr ( xmpRef );
		meta->ParseFromBuffer ( buffer, bufferSize, options );
		
	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPMeta_SerializeToBuffer_1 ( XMPMetaRef	   xmpRef,
							   XMP_StringPtr * rdfString,
							   XMP_StringLen * rdfSize,
							   XMP_OptionBits  options,
							   XMP_StringLen   padding,
							   XMP_StringPtr   newline,
							   XMP_StringPtr   indent,
							   XMP_Index	   baseIndent,
							   WXMP_Result *   wResult ) /* const */
{
	XMP_ENTER_WRAPPER ( "WXMPMeta_SerializeToBuffer_1" )

		if ( rdfString == 0 ) rdfString = &voidStringPtr;
		if ( rdfSize == 0 ) rdfSize = &voidStringLen;
		
		if ( newline == 0 ) newline = "";
		if ( indent == 0 ) indent = "";
		
		const XMPMeta & meta = WtoXMPMeta_Ref ( xmpRef );
		meta.SerializeToBuffer ( rdfString, rdfSize, options, padding, newline, indent, baseIndent );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true ) // ! Always keep the lock, a string is always returned!
}

// =================================================================================================

#if __cplusplus
} /* extern "C" */
#endif
