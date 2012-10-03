#if ! __WXMPUtils_hpp__
#define __WXMPUtils_hpp__ 1

// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "client-glue/WXMP_Common.hpp"

#if __cplusplus
extern "C" {
#endif

// =================================================================================================

#define zXMPUtils_ComposeArrayItemPath_1(schemaNS,arrayName,itemIndex,fullPath,pathSize) \
    WXMPUtils_ComposeArrayItemPath_1 ( schemaNS, arrayName, itemIndex, fullPath, pathSize, &wResult );

#define zXMPUtils_ComposeStructFieldPath_1(schemaNS,structName,fieldNS,fieldName,fullPath,pathSize) \
    WXMPUtils_ComposeStructFieldPath_1 ( schemaNS, structName, fieldNS, fieldName, fullPath, pathSize, &wResult );

#define zXMPUtils_ComposeQualifierPath_1(schemaNS,propName,qualNS,qualName,fullPath,pathSize) \
    WXMPUtils_ComposeQualifierPath_1 ( schemaNS, propName, qualNS, qualName, fullPath, pathSize, &wResult );

#define zXMPUtils_ComposeLangSelector_1(schemaNS,arrayName,langName,fullPath,pathSize) \
    WXMPUtils_ComposeLangSelector_1 ( schemaNS, arrayName, langName, fullPath, pathSize, &wResult );

#define zXMPUtils_ComposeFieldSelector_1(schemaNS,arrayName,fieldNS,fieldName,fieldValue,fullPath,pathSize) \
    WXMPUtils_ComposeFieldSelector_1 ( schemaNS, arrayName, fieldNS, fieldName, fieldValue, fullPath, pathSize, &wResult );

#define zXMPUtils_ConvertFromBool_1(binValue,strValue,strSize) \
    WXMPUtils_ConvertFromBool_1 ( binValue, strValue, strSize, &wResult );

#define zXMPUtils_ConvertFromInt_1(binValue,format,strValue,strSize) \
    WXMPUtils_ConvertFromInt_1 ( binValue, format, strValue, strSize, &wResult );

#define zXMPUtils_ConvertFromInt64_1(binValue,format,strValue,strSize) \
    WXMPUtils_ConvertFromInt64_1 ( binValue, format, strValue, strSize, &wResult );

#define zXMPUtils_ConvertFromFloat_1(binValue,format,strValue,strSize) \
    WXMPUtils_ConvertFromFloat_1 ( binValue, format, strValue, strSize, &wResult );

#define zXMPUtils_ConvertFromDate_1(binValue,strValue,strSize) \
    WXMPUtils_ConvertFromDate_1 ( binValue, strValue, strSize, &wResult );

#define zXMPUtils_ConvertToBool_1(strValue) \
    WXMPUtils_ConvertToBool_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToInt_1(strValue) \
    WXMPUtils_ConvertToInt_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToInt64_1(strValue) \
    WXMPUtils_ConvertToInt64_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToFloat_1(strValue) \
    WXMPUtils_ConvertToFloat_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToDate_1(strValue,binValue) \
    WXMPUtils_ConvertToDate_1 ( strValue, binValue, &wResult );

#define zXMPUtils_CurrentDateTime_1(time) \
    WXMPUtils_CurrentDateTime_1 ( time, &wResult );

#define zXMPUtils_SetTimeZone_1(time) \
    WXMPUtils_SetTimeZone_1 ( time, &wResult );

#define zXMPUtils_ConvertToUTCTime_1(time) \
    WXMPUtils_ConvertToUTCTime_1 ( time, &wResult );

#define zXMPUtils_ConvertToLocalTime_1(time) \
    WXMPUtils_ConvertToLocalTime_1 ( time, &wResult );

#define zXMPUtils_CompareDateTime_1(left,right) \
    WXMPUtils_CompareDateTime_1 ( left, right, &wResult );

#define zXMPUtils_EncodeToBase64_1(rawStr,rawLen,encodedStr,encodedLen) \
    WXMPUtils_EncodeToBase64_1 ( rawStr, rawLen, encodedStr, encodedLen, &wResult );

#define zXMPUtils_DecodeFromBase64_1(encodedStr,encodedLen,rawStr,rawLen) \
    WXMPUtils_DecodeFromBase64_1 ( encodedStr, encodedLen, rawStr, rawLen, &wResult );

#define zXMPUtils_PackageForJPEG_1(xmpObj,stdStr,stdLen,extStr,extLen,digestStr,digestLen) \
    WXMPUtils_PackageForJPEG_1 ( xmpObj, stdStr, stdLen, extStr, extLen, digestStr, digestLen, &wResult );

#define zXMPUtils_MergeFromJPEG_1(fullXMP,extendedXMP) \
    WXMPUtils_MergeFromJPEG_1 ( fullXMP, extendedXMP, &wResult );

#define zXMPUtils_CatenateArrayItems_1(xmpObj,schemaNS,arrayName,separator,quotes,options,catedPtr,catedLen) \
    WXMPUtils_CatenateArrayItems_1 ( xmpObj, schemaNS, arrayName, separator, quotes, options, catedPtr, catedLen, &wResult );

#define zXMPUtils_SeparateArrayItems_1(xmpObj,schemaNS,arrayName,options,catedStr) \
    WXMPUtils_SeparateArrayItems_1 ( xmpObj, schemaNS, arrayName, options, catedStr, &wResult );

#define zXMPUtils_RemoveProperties_1(xmpObj,schemaNS,propName,options) \
    WXMPUtils_RemoveProperties_1 ( xmpObj, schemaNS, propName, options, &wResult );

#define zXMPUtils_AppendProperties_1(source,dest,options) \
    WXMPUtils_AppendProperties_1 ( source, dest, options, &wResult );

#define zXMPUtils_DuplicateSubtree_1(source,dest,sourceNS,sourceRoot,destNS,destRoot,options) \
    WXMPUtils_DuplicateSubtree_1 ( source, dest, sourceNS, sourceRoot, destNS, destRoot, options, &wResult );

// =================================================================================================

extern void
WXMPUtils_Unlock_1 ( XMP_OptionBits options );

// -------------------------------------------------------------------------------------------------

extern void
WXMPUtils_ComposeArrayItemPath_1 ( XMP_StringPtr   schemaNS,
                                   XMP_StringPtr   arrayName,
                                   XMP_Index       itemIndex,
                                   XMP_StringPtr * fullPath,
                                   XMP_StringLen * pathSize,
                                   WXMP_Result *   wResult );

extern void
WXMPUtils_ComposeStructFieldPath_1 ( XMP_StringPtr   schemaNS,
                                     XMP_StringPtr   structName,
                                     XMP_StringPtr   fieldNS,
                                     XMP_StringPtr   fieldName,
                                     XMP_StringPtr * fullPath,
                                     XMP_StringLen * pathSize,
                                     WXMP_Result *   wResult );

extern void
WXMPUtils_ComposeQualifierPath_1 ( XMP_StringPtr   schemaNS,
                                   XMP_StringPtr   propName,
                                   XMP_StringPtr   qualNS,
                                   XMP_StringPtr   qualName,
                                   XMP_StringPtr * fullPath,
                                   XMP_StringLen * pathSize,
                                   WXMP_Result *   wResult );

extern void
WXMPUtils_ComposeLangSelector_1 ( XMP_StringPtr   schemaNS,
                                  XMP_StringPtr   arrayName,
                                  XMP_StringPtr   langName,
                                  XMP_StringPtr * fullPath,
                                  XMP_StringLen * pathSize,
                                  WXMP_Result *   wResult );

extern void
WXMPUtils_ComposeFieldSelector_1 ( XMP_StringPtr   schemaNS,
                                   XMP_StringPtr   arrayName,
                                   XMP_StringPtr   fieldNS,
                                   XMP_StringPtr   fieldName,
                                   XMP_StringPtr   fieldValue,
                                   XMP_StringPtr * fullPath,
                                   XMP_StringLen * pathSize,
                                   WXMP_Result *   wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPUtils_ConvertFromBool_1 ( XMP_Bool        binValue,
                              XMP_StringPtr * strValue,
                              XMP_StringLen * strSize,
                              WXMP_Result *   wResult );

extern void
WXMPUtils_ConvertFromInt_1 ( XMP_Int32       binValue,
                             XMP_StringPtr   format,
                             XMP_StringPtr * strValue,
                             XMP_StringLen * strSize,
                             WXMP_Result *   wResult );

extern void
WXMPUtils_ConvertFromInt64_1 ( XMP_Int64       binValue,
                               XMP_StringPtr   format,
                               XMP_StringPtr * strValue,
                               XMP_StringLen * strSize,
                               WXMP_Result *   wResult );

extern void
WXMPUtils_ConvertFromFloat_1 ( double          binValue,
                               XMP_StringPtr   format,
                               XMP_StringPtr * strValue,
                               XMP_StringLen * strSize,
                               WXMP_Result *   wResult );

extern void
WXMPUtils_ConvertFromDate_1 ( const XMP_DateTime & binValue,
                              XMP_StringPtr *      strValue,
                              XMP_StringLen *      strSize,
                              WXMP_Result *        wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPUtils_ConvertToBool_1 ( XMP_StringPtr strValue,
                            WXMP_Result * wResult );

extern void
WXMPUtils_ConvertToInt_1 ( XMP_StringPtr strValue,
                           WXMP_Result * wResult );

extern void
WXMPUtils_ConvertToInt64_1 ( XMP_StringPtr strValue,
                             WXMP_Result * wResult );

extern void
WXMPUtils_ConvertToFloat_1 ( XMP_StringPtr strValue,
                             WXMP_Result * wResult );

extern void
WXMPUtils_ConvertToDate_1 ( XMP_StringPtr  strValue,
                            XMP_DateTime * binValue,
                            WXMP_Result *  wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPUtils_CurrentDateTime_1 ( XMP_DateTime * time,
                              WXMP_Result *  wResult );

extern void
WXMPUtils_SetTimeZone_1 ( XMP_DateTime * time,
                          WXMP_Result *  wResult );

extern void
WXMPUtils_ConvertToUTCTime_1 ( XMP_DateTime * time,
                               WXMP_Result *  wResult );

extern void
WXMPUtils_ConvertToLocalTime_1 ( XMP_DateTime * time,
                                 WXMP_Result *  wResult );

extern void
WXMPUtils_CompareDateTime_1 ( const XMP_DateTime & left,
                              const XMP_DateTime & right,
                              WXMP_Result *        wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPUtils_EncodeToBase64_1 ( XMP_StringPtr   rawStr,
                             XMP_StringLen   rawLen,
                             XMP_StringPtr * encodedStr,
                             XMP_StringLen * encodedLen,
                             WXMP_Result *   wResult );

extern void
WXMPUtils_DecodeFromBase64_1 ( XMP_StringPtr   encodedStr,
                               XMP_StringLen   encodedLen,
                               XMP_StringPtr * rawStr,
                               XMP_StringLen * rawLen,
                               WXMP_Result *   wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPUtils_PackageForJPEG_1 ( XMPMetaRef      xmpObj,
                             XMP_StringPtr * stdStr,
                             XMP_StringLen * stdLen,
                             XMP_StringPtr * extStr,
                             XMP_StringLen * extLen,
                             XMP_StringPtr * digestStr,
                             XMP_StringLen * digestLen,
                             WXMP_Result *   wResult );

extern void
WXMPUtils_MergeFromJPEG_1 ( XMPMetaRef    fullXMP,
                            XMPMetaRef    extendedXMP,
                            WXMP_Result * wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPUtils_CatenateArrayItems_1 ( XMPMetaRef      xmpObj,
                                 XMP_StringPtr   schemaNS,
                                 XMP_StringPtr   arrayName,
                                 XMP_StringPtr   separator,
                                 XMP_StringPtr   quotes,
                                 XMP_OptionBits  options,
                                 XMP_StringPtr * catedStr,
                                 XMP_StringLen * catedLen,
                                 WXMP_Result *   wResult );

extern void
WXMPUtils_SeparateArrayItems_1 ( XMPMetaRef     xmpObj,
                                 XMP_StringPtr  schemaNS,
                                 XMP_StringPtr  arrayName,
                                 XMP_OptionBits options,
                                 XMP_StringPtr  catedStr,
                                 WXMP_Result *  wResult );

extern void
WXMPUtils_RemoveProperties_1 ( XMPMetaRef     xmpObj,
                               XMP_StringPtr  schemaNS,
                               XMP_StringPtr  propName,
                               XMP_OptionBits options,
                               WXMP_Result *  wResult );

extern void
WXMPUtils_AppendProperties_1 ( XMPMetaRef     source,
                               XMPMetaRef     dest,
                               XMP_OptionBits options,
                               WXMP_Result *  wResult );

extern void
WXMPUtils_DuplicateSubtree_1 ( XMPMetaRef     source,
                               XMPMetaRef     dest,
                               XMP_StringPtr  sourceNS,
                               XMP_StringPtr  sourceRoot,
                               XMP_StringPtr  destNS,
                               XMP_StringPtr  destRoot,
                               XMP_OptionBits options,
                               WXMP_Result *  wResult );

// =================================================================================================

#if __cplusplus
} /* extern "C" */
#endif

#endif  // __WXMPUtils_hpp__
