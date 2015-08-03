// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:	Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

// *** Should change "type * inParam" to "type & inParam"

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"

#include "XMPUtils.hpp"
#include "client-glue/WXMPUtils.hpp"

#if XMP_WinBuild
    #ifdef _MSC_VER
        #pragma warning ( disable : 4101 ) // unreferenced local variable
        #pragma warning ( disable : 4189 ) // local variable is initialized but not referenced
        #pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
        #if XMP_DebugBuild
            #pragma warning ( disable : 4297 ) // function assumed not to throw an exception but does
        #endif
    #endif
#endif

#if __cplusplus
extern "C" {
#endif

// =================================================================================================
// Class Static Wrappers
// =====================

void
WXMPUtils_Unlock_1 ( XMP_OptionBits options )
{
	WXMP_Result * wResult = &void_wResult;	// ! Needed to "fool" the EnterWrapper macro.
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_Unlock_1" )

		XMPUtils::Unlock ( options );

	XMP_EXIT_WRAPPER_NO_THROW
}

// =================================================================================================

void
WXMPUtils_ComposeArrayItemPath_1 ( XMP_StringPtr   schemaNS,
								   XMP_StringPtr   arrayName,
								   XMP_Index	   itemIndex,
								   XMP_StringPtr * fullPath,
								   XMP_StringLen * pathSize,
								   WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ComposeArrayItemPath_1" )
	
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		
		if ( fullPath == 0 ) fullPath = &voidStringPtr;
		if ( pathSize == 0 ) pathSize = &voidStringLen;

		XMPUtils::ComposeArrayItemPath ( schemaNS, arrayName, itemIndex, fullPath, pathSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ComposeStructFieldPath_1 ( XMP_StringPtr	 schemaNS,
									 XMP_StringPtr	 structName,
									 XMP_StringPtr	 fieldNS,
									 XMP_StringPtr	 fieldName,
									 XMP_StringPtr * fullPath,
									 XMP_StringLen * pathSize,
									 WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ComposeStructFieldPath_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (structName == 0) || (*structName == 0) ) XMP_Throw ( "Empty struct name", kXMPErr_BadXPath );
		if ( (fieldNS == 0) || (*fieldNS == 0) ) XMP_Throw ( "Empty field namespace URI", kXMPErr_BadSchema );
		if ( (fieldName == 0) || (*fieldName == 0) ) XMP_Throw ( "Empty field name", kXMPErr_BadXPath );
		
		if ( fullPath == 0 ) fullPath = &voidStringPtr;
		if ( pathSize == 0 ) pathSize = &voidStringLen;

		XMPUtils::ComposeStructFieldPath ( schemaNS, structName, fieldNS, fieldName, fullPath, pathSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ComposeQualifierPath_1 ( XMP_StringPtr   schemaNS,
								   XMP_StringPtr   propName,
								   XMP_StringPtr   qualNS,
								   XMP_StringPtr   qualName,
								   XMP_StringPtr * fullPath,
								   XMP_StringLen * pathSize,
								   WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ComposeQualifierPath_1" )
	
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (propName == 0) || (*propName == 0) ) XMP_Throw ( "Empty property name", kXMPErr_BadXPath );
		if ( (qualNS == 0) || (*qualNS == 0) ) XMP_Throw ( "Empty qualifier namespace URI", kXMPErr_BadSchema );
		if ( (qualName == 0) || (*qualName == 0) ) XMP_Throw ( "Empty qualifier name", kXMPErr_BadXPath );
		
		if ( fullPath == 0 ) fullPath = &voidStringPtr;
		if ( pathSize == 0 ) pathSize = &voidStringLen;

		XMPUtils::ComposeQualifierPath ( schemaNS, propName, qualNS, qualName, fullPath, pathSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ComposeLangSelector_1 ( XMP_StringPtr	  schemaNS,
								  XMP_StringPtr	  arrayName,
								  XMP_StringPtr	  langName,
								  XMP_StringPtr * fullPath,
								  XMP_StringLen * pathSize,
								  WXMP_Result *	  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ComposeLangSelector_1" )
	
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		if ( (langName == 0) || (*langName == 0) ) XMP_Throw ( "Empty language name", kXMPErr_BadParam );
		
		if ( fullPath == 0 ) fullPath = &voidStringPtr;
		if ( pathSize == 0 ) pathSize = &voidStringLen;

		XMPUtils::ComposeLangSelector ( schemaNS, arrayName, langName, fullPath, pathSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ComposeFieldSelector_1 ( XMP_StringPtr   schemaNS,
								   XMP_StringPtr   arrayName,
								   XMP_StringPtr   fieldNS,
								   XMP_StringPtr   fieldName,
								   XMP_StringPtr   fieldValue,
								   XMP_StringPtr * fullPath,
								   XMP_StringLen * pathSize,
								   WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ComposeFieldSelector_1" )
	
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		if ( (fieldNS == 0) || (*fieldNS == 0) ) XMP_Throw ( "Empty field namespace URI", kXMPErr_BadSchema );
		if ( (fieldName == 0) || (*fieldName == 0) ) XMP_Throw ( "Empty field name", kXMPErr_BadXPath );
		if ( fieldValue == 0 ) fieldValue = "";
		
		if ( fullPath == 0 ) fullPath = &voidStringPtr;
		if ( pathSize == 0 ) pathSize = &voidStringLen;

		XMPUtils::ComposeFieldSelector ( schemaNS, arrayName, fieldNS, fieldName, fieldValue, fullPath, pathSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// =================================================================================================

void
WXMPUtils_ConvertFromBool_1 ( XMP_Bool		  binValue,
							  XMP_StringPtr * strValue,
							  XMP_StringLen * strSize,
							  WXMP_Result *	  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ConvertFromBool_1" )

		if ( strValue == 0 ) strValue = &voidStringPtr;
		if ( strSize == 0 ) strSize = &voidStringLen;

		XMPUtils::ConvertFromBool ( binValue, strValue, strSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertFromInt_1 ( XMP_Int32		 binValue,
							 XMP_StringPtr	 format,
							 XMP_StringPtr * strValue,
							 XMP_StringLen * strSize,
							 WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ConvertFromInt_1" )

		if ( format == 0 ) format = "";
		
		if ( strValue == 0 ) strValue = &voidStringPtr;
		if ( strSize == 0 ) strSize = &voidStringLen;

		XMPUtils::ConvertFromInt ( binValue, format, strValue, strSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertFromInt64_1 ( XMP_Int64	   binValue,
							   XMP_StringPtr   format,
							   XMP_StringPtr * strValue,
							   XMP_StringLen * strSize,
							   WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ConvertFromInt64_1" )

		if ( format == 0 ) format = "";
		
		if ( strValue == 0 ) strValue = &voidStringPtr;
		if ( strSize == 0 ) strSize = &voidStringLen;

		XMPUtils::ConvertFromInt64 ( binValue, format, strValue, strSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertFromFloat_1 ( double		   binValue,
							   XMP_StringPtr   format,
							   XMP_StringPtr * strValue,
							   XMP_StringLen * strSize,
							   WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ConvertFromFloat_1" )

		if ( format == 0 ) format = "";
		
		if ( strValue == 0 ) strValue = &voidStringPtr;
		if ( strSize == 0 ) strSize = &voidStringLen;

		XMPUtils::ConvertFromFloat ( binValue, format, strValue, strSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertFromDate_1 ( const XMP_DateTime & binValue,
							  XMP_StringPtr *	   strValue,
							  XMP_StringLen *	   strSize,
							  WXMP_Result *		   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_ConvertFromDate_1" )

		if ( strValue == 0 ) strValue = &voidStringPtr;
		if ( strSize == 0 ) strSize = &voidStringLen;

		XMPUtils::ConvertFromDate( binValue, strValue, strSize );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// =================================================================================================

void
WXMPUtils_ConvertToBool_1 ( XMP_StringPtr strValue,
							WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_ConvertToBool_1" )

		if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty string value", kXMPErr_BadParam);
		XMP_Bool result = XMPUtils::ConvertToBool ( strValue );
		wResult->int32Result = result;

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertToInt_1 ( XMP_StringPtr strValue,
						   WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_ConvertToInt_1" )

		if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty string value", kXMPErr_BadParam);
		XMP_Int32 result = XMPUtils::ConvertToInt ( strValue );
		wResult->int32Result = result;

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertToInt64_1 ( XMP_StringPtr strValue,
						     WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_ConvertToInt64_1" )

		if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty string value", kXMPErr_BadParam);
		XMP_Int64 result = XMPUtils::ConvertToInt64 ( strValue );
		wResult->int64Result = result;

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertToFloat_1 ( XMP_StringPtr strValue,
							 WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_ConvertToFloat_1")

		if ( (strValue == 0) || (*strValue == 0) ) XMP_Throw ( "Empty string value", kXMPErr_BadParam);
		double result = XMPUtils::ConvertToFloat ( strValue );
		wResult->floatResult = result;

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertToDate_1 ( XMP_StringPtr  strValue,
							XMP_DateTime * binValue,
							WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_ConvertToDate_1" )

		if ( binValue == 0 ) XMP_Throw ( "Null output date", kXMPErr_BadParam); // ! Pointer is from the client.
		XMPUtils::ConvertToDate ( strValue, binValue );

	XMP_EXIT_WRAPPER
}

// =================================================================================================

void
WXMPUtils_CurrentDateTime_1 ( XMP_DateTime * time,
							  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_CurrentDateTime_1" )
	
		if ( time == 0 ) XMP_Throw ( "Null output date", kXMPErr_BadParam);
		XMPUtils::CurrentDateTime ( time );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_SetTimeZone_1 ( XMP_DateTime * time,
						  WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_SetTimeZone_1" )

		if ( time == 0 ) XMP_Throw ( "Null output date", kXMPErr_BadParam);
		XMPUtils::SetTimeZone ( time );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertToUTCTime_1 ( XMP_DateTime * time,
							   WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_ConvertToUTCTime_1" )

		if ( time == 0 ) XMP_Throw ( "Null output date", kXMPErr_BadParam);
		XMPUtils::ConvertToUTCTime ( time );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_ConvertToLocalTime_1 ( XMP_DateTime * time,
								 WXMP_Result *	wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_ConvertToLocalTime_1" )

		if ( time == 0 ) XMP_Throw ( "Null output date", kXMPErr_BadParam);
		XMPUtils::ConvertToLocalTime ( time );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_CompareDateTime_1 ( const XMP_DateTime & left,
							  const XMP_DateTime & right,
							  WXMP_Result *		   wResult )
{
	XMP_ENTER_WRAPPER_NO_LOCK ( "WXMPUtils_CompareDateTime_1" )

		int result = XMPUtils::CompareDateTime ( left, right );
		wResult->int32Result = result;

	XMP_EXIT_WRAPPER
}

// =================================================================================================

void
WXMPUtils_EncodeToBase64_1 ( XMP_StringPtr	 rawStr,
							 XMP_StringLen	 rawLen,
							 XMP_StringPtr * encodedStr,
							 XMP_StringLen * encodedLen,
							 WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_EncodeToBase64_1" )
	
		if ( encodedStr == 0 ) encodedStr = &voidStringPtr;
		if ( encodedLen == 0 ) encodedLen = &voidStringLen;

		XMPUtils::EncodeToBase64 ( rawStr, rawLen, encodedStr, encodedLen );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_DecodeFromBase64_1 ( XMP_StringPtr   encodedStr,
							   XMP_StringLen   encodedLen,
							   XMP_StringPtr * rawStr,
							   XMP_StringLen * rawLen,
							   WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_DecodeFromBase64_1" )

		if ( rawStr == 0 ) rawStr = &voidStringPtr;
		if ( rawLen == 0 ) rawLen = &voidStringLen;

		XMPUtils::DecodeFromBase64 ( encodedStr, encodedLen, rawStr, rawLen );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// =================================================================================================

void
WXMPUtils_PackageForJPEG_1 ( XMPMetaRef      wxmpObj,
                             XMP_StringPtr * stdStr,
                             XMP_StringLen * stdLen,
                             XMP_StringPtr * extStr,
                             XMP_StringLen * extLen,
                             XMP_StringPtr * digestStr,
                             XMP_StringLen * digestLen,
                             WXMP_Result *   wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_PackageForJPEG_1" )

		if ( stdStr == 0 ) stdStr = &voidStringPtr;
		if ( stdLen == 0 ) stdLen = &voidStringLen;
		if ( extStr == 0 ) extStr = &voidStringPtr;
		if ( extLen == 0 ) extLen = &voidStringLen;
		if ( digestStr == 0 ) digestStr = &voidStringPtr;
		if ( digestLen == 0 ) digestLen = &voidStringLen;

		const XMPMeta & xmpObj = WtoXMPMeta_Ref ( wxmpObj );
		XMPUtils::PackageForJPEG ( xmpObj, stdStr, stdLen, extStr, extLen, digestStr, digestLen );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_MergeFromJPEG_1 ( XMPMetaRef    wfullXMP,
                            XMPMetaRef    wextendedXMP,
                            WXMP_Result * wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_MergeFromJPEG_1" )

		if ( wfullXMP == 0 ) XMP_Throw ( "Output XMP pointer is null", kXMPErr_BadParam );

		XMPMeta * fullXMP = WtoXMPMeta_Ptr ( wfullXMP );
		const XMPMeta & extendedXMP = WtoXMPMeta_Ref ( wextendedXMP );
		XMPUtils::MergeFromJPEG ( fullXMP, extendedXMP );

	XMP_EXIT_WRAPPER
}

// =================================================================================================

void
WXMPUtils_CatenateArrayItems_1 ( XMPMetaRef 	 wxmpObj,
								 XMP_StringPtr	 schemaNS,
								 XMP_StringPtr	 arrayName,
								 XMP_StringPtr	 separator,
								 XMP_StringPtr	 quotes,
								 XMP_OptionBits	 options,
								 XMP_StringPtr * catedStr,
								 XMP_StringLen * catedLen,
								 WXMP_Result *	 wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_CatenateArrayItems_1" )

		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		
		if ( separator == 0 ) separator = "; ";
		if ( quotes == 0 ) quotes = "\"";
		
		if ( catedStr == 0 ) catedStr = &voidStringPtr;
		if ( catedLen == 0 ) catedLen = &voidStringLen;

		const XMPMeta & xmpObj = WtoXMPMeta_Ref ( wxmpObj );
		XMPUtils::CatenateArrayItems ( xmpObj, schemaNS, arrayName, separator, quotes, options, catedStr, catedLen );

	XMP_EXIT_WRAPPER_KEEP_LOCK ( true )
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_SeparateArrayItems_1 ( XMPMetaRef 	wxmpObj,
								 XMP_StringPtr	schemaNS,
								 XMP_StringPtr	arrayName,
								 XMP_OptionBits options,
								 XMP_StringPtr	catedStr,
								 WXMP_Result *	wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_SeparateArrayItems_1" )

		if ( wxmpObj == 0 ) XMP_Throw ( "Output XMP pointer is null", kXMPErr_BadParam );
		if ( (schemaNS == 0) || (*schemaNS == 0) ) XMP_Throw ( "Empty schema namespace URI", kXMPErr_BadSchema );
		if ( (arrayName == 0) || (*arrayName == 0) ) XMP_Throw ( "Empty array name", kXMPErr_BadXPath );
		if ( catedStr == 0 ) catedStr = "";
		
		XMPMeta * xmpObj = WtoXMPMeta_Ptr ( wxmpObj );
		XMPUtils::SeparateArrayItems ( xmpObj, schemaNS, arrayName, options, catedStr );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_RemoveProperties_1 ( XMPMetaRef 	  wxmpObj,
							   XMP_StringPtr  schemaNS,
							   XMP_StringPtr  propName,
							   XMP_OptionBits options,
							   WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_RemoveProperties_1" )

		if ( wxmpObj == 0 ) XMP_Throw ( "Output XMP pointer is null", kXMPErr_BadParam );
		if ( schemaNS == 0 ) schemaNS = "";
		if ( propName == 0 ) propName = "";
		
		XMPMeta * xmpObj = WtoXMPMeta_Ptr ( wxmpObj );
		XMPUtils::RemoveProperties ( xmpObj, schemaNS, propName, options );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_AppendProperties_1 ( XMPMetaRef     wSource,
							   XMPMetaRef 	  wDest,
							   XMP_OptionBits options,
							   WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_AppendProperties_1" )

		if ( wDest == 0 ) XMP_Throw ( "Output XMP pointer is null", kXMPErr_BadParam );

		const XMPMeta & source = WtoXMPMeta_Ref ( wSource );
		XMPMeta * dest = WtoXMPMeta_Ptr ( wDest );
		XMPUtils::AppendProperties ( source, dest, options );

	XMP_EXIT_WRAPPER
}

// -------------------------------------------------------------------------------------------------

void
WXMPUtils_DuplicateSubtree_1 ( XMPMetaRef     wSource,
							   XMPMetaRef 	  wDest,
							   XMP_StringPtr  sourceNS,
							   XMP_StringPtr  sourceRoot,
							   XMP_StringPtr  destNS,
							   XMP_StringPtr  destRoot,
							   XMP_OptionBits options,
							   WXMP_Result *  wResult )
{
	XMP_ENTER_WRAPPER ( "WXMPUtils_DuplicateSubtree_1" )
	
		if ( wDest == 0 ) XMP_Throw ( "Output XMP pointer is null", kXMPErr_BadParam );
		if ( (sourceNS == 0) || (*sourceNS == 0) ) XMP_Throw ( "Empty source schema URI", kXMPErr_BadSchema );
		if ( (sourceRoot == 0) || (*sourceRoot == 0) ) XMP_Throw ( "Empty source root name", kXMPErr_BadXPath );
		if ( destNS == 0 ) destNS = sourceNS;
		if ( destRoot == 0 ) destRoot = sourceRoot;

		const XMPMeta & source = WtoXMPMeta_Ref ( wSource );
		XMPMeta * dest = WtoXMPMeta_Ptr ( wDest );
		XMPUtils::DuplicateSubtree ( source, dest, sourceNS, sourceRoot, destNS, destRoot, options );

	XMP_EXIT_WRAPPER
}

// =================================================================================================

#if __cplusplus
} /* extern "C" */
#endif
