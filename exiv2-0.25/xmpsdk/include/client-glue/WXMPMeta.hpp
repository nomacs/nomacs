#if ! __WXMPMeta_hpp__
#define __WXMPMeta_hpp__ 1

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
#define zXMPMeta_GetVersionInfo_1(info) \
    WXMPMeta_GetVersionInfo_1 ( info /* no wResult */ )

#define zXMPMeta_Initialize_1() \
    WXMPMeta_Initialize_1 ( &wResult )
#define zXMPMeta_Terminate_1() \
    WXMPMeta_Terminate_1 ( /* no wResult */ )

#define zXMPMeta_CTor_1() \
    WXMPMeta_CTor_1 ( &wResult )

#define zXMPMeta_GetGlobalOptions_1() \
    WXMPMeta_GetGlobalOptions_1 ( &wResult )

#define zXMPMeta_SetGlobalOptions_1(options) \
    WXMPMeta_SetGlobalOptions_1 ( options, &wResult )

#define zXMPMeta_DumpNamespaces_1(outProc,refCon) \
    WXMPMeta_DumpNamespaces_1 ( outProc, refCon, &wResult )

#define zXMPMeta_DumpAliases_1(outProc,refCon) \
    WXMPMeta_DumpAliases_1 ( outProc, refCon, &wResult )

#define zXMPMeta_RegisterNamespace_1(namespaceURI,prefix) \
    WXMPMeta_RegisterNamespace_1 ( namespaceURI, prefix, &wResult )

#define zXMPMeta_GetNamespacePrefix_1(namespaceURI,namespacePrefix,prefixSize) \
    WXMPMeta_GetNamespacePrefix_1 ( namespaceURI, namespacePrefix, prefixSize, &wResult )

#define zXMPMeta_GetNamespaceURI_1(namespacePrefix,namespaceURI,uriSize) \
    WXMPMeta_GetNamespaceURI_1 ( namespacePrefix, namespaceURI, uriSize, &wResult )

#define zXMPMeta_DeleteNamespace_1(namespaceURI) \
    WXMPMeta_DeleteNamespace_1 ( namespaceURI, &wResult )

#define zXMPMeta_RegisterAlias_1(aliasNS,aliasProp,actualNS,actualProp,arrayForm) \
    WXMPMeta_RegisterAlias_1 ( aliasNS, aliasProp, actualNS, actualProp, arrayForm, &wResult )

#define zXMPMeta_ResolveAlias_1(aliasNS,aliasProp,actualNS,nsSize,actualProp,propSize,arrayForm) \
    WXMPMeta_ResolveAlias_1 ( aliasNS, aliasProp, actualNS, nsSize, actualProp, propSize, arrayForm, &wResult )

#define zXMPMeta_DeleteAlias_1(aliasNS,aliasProp) \
    WXMPMeta_DeleteAlias_1 ( aliasNS, aliasProp, &wResult )

#define zXMPMeta_RegisterStandardAliases_1(schemaNS) \
    WXMPMeta_RegisterStandardAliases_1 ( schemaNS, &wResult )

#define zXMPMeta_GetProperty_1(schemaNS,propName,propValue,valueSize,options) \
    WXMPMeta_GetProperty_1 ( this->xmpRef, schemaNS, propName, propValue, valueSize, options, &wResult )

#define zXMPMeta_GetArrayItem_1(schemaNS,arrayName,itemIndex,itemValue,valueSize,options) \
    WXMPMeta_GetArrayItem_1 ( this->xmpRef, schemaNS, arrayName, itemIndex, itemValue, valueSize, options, &wResult )

#define zXMPMeta_GetStructField_1(schemaNS,structName,fieldNS,fieldName,fieldValue,valueSize,options) \
    WXMPMeta_GetStructField_1 ( this->xmpRef, schemaNS, structName, fieldNS, fieldName, fieldValue, valueSize, options, &wResult )

#define zXMPMeta_GetQualifier_1(schemaNS,propName,qualNS,qualName,qualValue,valueSize,options) \
    WXMPMeta_GetQualifier_1 ( this->xmpRef, schemaNS, propName, qualNS, qualName, qualValue, valueSize, options, &wResult )

#define zXMPMeta_SetProperty_1(schemaNS,propName,propValue,options) \
    WXMPMeta_SetProperty_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_SetArrayItem_1(schemaNS,arrayName,itemIndex,itemValue,options) \
    WXMPMeta_SetArrayItem_1 ( this->xmpRef, schemaNS, arrayName, itemIndex, itemValue, options, &wResult )

#define zXMPMeta_AppendArrayItem_1(schemaNS,arrayName,arrayOptions,itemValue,options) \
    WXMPMeta_AppendArrayItem_1 ( this->xmpRef, schemaNS, arrayName, arrayOptions, itemValue, options, &wResult )

#define zXMPMeta_SetStructField_1(schemaNS,structName,fieldNS,fieldName,fieldValue,options) \
    WXMPMeta_SetStructField_1 ( this->xmpRef, schemaNS, structName, fieldNS, fieldName, fieldValue, options, &wResult )

#define zXMPMeta_SetQualifier_1(schemaNS,propName,qualNS,qualName,qualValue,options) \
    WXMPMeta_SetQualifier_1 ( this->xmpRef, schemaNS, propName, qualNS, qualName, qualValue, options, &wResult )

#define zXMPMeta_DeleteProperty_1(schemaNS,propName) \
    WXMPMeta_DeleteProperty_1 ( this->xmpRef, schemaNS, propName, &wResult )

#define zXMPMeta_DeleteArrayItem_1(schemaNS,arrayName,itemIndex) \
    WXMPMeta_DeleteArrayItem_1 ( this->xmpRef, schemaNS, arrayName, itemIndex, &wResult )

#define zXMPMeta_DeleteStructField_1(schemaNS,structName,fieldNS,fieldName) \
    WXMPMeta_DeleteStructField_1 ( this->xmpRef, schemaNS, structName, fieldNS, fieldName, &wResult )

#define zXMPMeta_DeleteQualifier_1(schemaNS,propName,qualNS,qualName) \
    WXMPMeta_DeleteQualifier_1 ( this->xmpRef, schemaNS, propName, qualNS, qualName, &wResult )

#define zXMPMeta_DoesPropertyExist_1(schemaNS,propName) \
    WXMPMeta_DoesPropertyExist_1 ( this->xmpRef, schemaNS, propName, &wResult )

#define zXMPMeta_DoesArrayItemExist_1(schemaNS,arrayName,itemIndex) \
    WXMPMeta_DoesArrayItemExist_1 ( this->xmpRef, schemaNS, arrayName, itemIndex, &wResult )

#define zXMPMeta_DoesStructFieldExist_1(schemaNS,structName,fieldNS,fieldName) \
    WXMPMeta_DoesStructFieldExist_1 ( this->xmpRef, schemaNS, structName, fieldNS, fieldName, &wResult )

#define zXMPMeta_DoesQualifierExist_1(schemaNS,propName,qualNS,qualName) \
    WXMPMeta_DoesQualifierExist_1 ( this->xmpRef, schemaNS, propName, qualNS, qualName, &wResult )

#define zXMPMeta_GetLocalizedText_1(schemaNS,altTextName,genericLang,specificLang,actualLang,langSize,itemValue,valueSize,options) \
    WXMPMeta_GetLocalizedText_1 ( this->xmpRef, schemaNS, altTextName, genericLang, specificLang, actualLang, langSize, itemValue, valueSize, options, &wResult )

#define zXMPMeta_SetLocalizedText_1(schemaNS,altTextName,genericLang,specificLang,itemValue,options) \
    WXMPMeta_SetLocalizedText_1 ( this->xmpRef, schemaNS, altTextName, genericLang, specificLang, itemValue, options, &wResult )

#define zXMPMeta_GetProperty_Bool_1(schemaNS,propName,propValue,options) \
    WXMPMeta_GetProperty_Bool_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_GetProperty_Int_1(schemaNS,propName,propValue,options) \
    WXMPMeta_GetProperty_Int_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_GetProperty_Int64_1(schemaNS,propName,propValue,options) \
    WXMPMeta_GetProperty_Int64_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_GetProperty_Float_1(schemaNS,propName,propValue,options) \
    WXMPMeta_GetProperty_Float_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_GetProperty_Date_1(schemaNS,propName,propValue,options) \
    WXMPMeta_GetProperty_Date_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_SetProperty_Bool_1(schemaNS,propName,propValue,options) \
    WXMPMeta_SetProperty_Bool_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_SetProperty_Int_1(schemaNS,propName,propValue,options) \
    WXMPMeta_SetProperty_Int_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_SetProperty_Int64_1(schemaNS,propName,propValue,options) \
    WXMPMeta_SetProperty_Int64_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_SetProperty_Float_1(schemaNS,propName,propValue,options) \
    WXMPMeta_SetProperty_Float_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_SetProperty_Date_1(schemaNS,propName,propValue,options) \
    WXMPMeta_SetProperty_Date_1 ( this->xmpRef, schemaNS, propName, propValue, options, &wResult )

#define zXMPMeta_GetObjectName_1(namePtr,nameLen) \
    WXMPMeta_GetObjectName_1 ( this->xmpRef, namePtr, nameLen, &wResult )

#define zXMPMeta_SetObjectName_1(name) \
    WXMPMeta_SetObjectName_1 ( this->xmpRef, name, &wResult )

#define zXMPMeta_GetObjectOptions_1() \
    WXMPMeta_GetObjectOptions_1 ( this->xmpRef, &wResult )

#define zXMPMeta_SetObjectOptions_1(options) \
    WXMPMeta_SetObjectOptions_1 ( this->xmpRef, options, &wResult )

#define zXMPMeta_Sort_1() \
    WXMPMeta_Sort_1 ( this->xmpRef, &wResult )

#define zXMPMeta_Erase_1() \
    WXMPMeta_Erase_1 ( this->xmpRef, &wResult )

#define zXMPMeta_Clone_1(options) \
    WXMPMeta_Clone_1 ( this->xmpRef, options, &wResult )

#define zXMPMeta_CountArrayItems_1(schemaNS,arrayName) \
    WXMPMeta_CountArrayItems_1 ( this->xmpRef, schemaNS, arrayName, &wResult )

#define zXMPMeta_DumpObject_1(outProc,refCon) \
    WXMPMeta_DumpObject_1 ( this->xmpRef, outProc, refCon, &wResult )

#define zXMPMeta_ParseFromBuffer_1(buffer,bufferSize,options) \
    WXMPMeta_ParseFromBuffer_1 ( this->xmpRef, buffer, bufferSize, options, &wResult )

#define zXMPMeta_SerializeToBuffer_1(pktString,pktSize,options,padding,newline,indent,baseIndent) \
    WXMPMeta_SerializeToBuffer_1 ( this->xmpRef, pktString, pktSize, options, padding, newline, indent, baseIndent, &wResult )

// =================================================================================================

extern void
WXMPMeta_GetVersionInfo_1 ( XMP_VersionInfo * info );

extern void
WXMPMeta_Initialize_1 ( WXMP_Result * wResult );
extern void
WXMPMeta_Terminate_1();

extern void
WXMPMeta_Unlock_1 ( XMP_OptionBits options );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_CTor_1 ( WXMP_Result * wResult );

extern void
WXMPMeta_IncrementRefCount_1 ( XMPMetaRef xmpRef );

extern void
WXMPMeta_DecrementRefCount_1 ( XMPMetaRef xmpRef );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_GetGlobalOptions_1 ( WXMP_Result * wResult );

extern void
WXMPMeta_SetGlobalOptions_1 ( XMP_OptionBits options,
                              WXMP_Result *  wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_DumpNamespaces_1 ( XMP_TextOutputProc outProc,
                            void *             refCon,
                            WXMP_Result *      wResult );

extern void
WXMPMeta_DumpAliases_1 ( XMP_TextOutputProc outProc,
                         void *             refCon,
                         WXMP_Result *      wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_RegisterNamespace_1 ( XMP_StringPtr   namespaceURI,
                               XMP_StringPtr   prefix,
                               WXMP_Result *   wResult );

extern void
WXMPMeta_GetNamespacePrefix_1 ( XMP_StringPtr   namespaceURI,
                                XMP_StringPtr * namespacePrefix,
                                XMP_StringLen * prefixSize,
                                WXMP_Result *   wResult );

extern void
WXMPMeta_GetNamespaceURI_1 ( XMP_StringPtr   namespacePrefix,
                             XMP_StringPtr * namespaceURI,
                             XMP_StringLen * uriSize,
                             WXMP_Result *   wResult );

extern void
WXMPMeta_DeleteNamespace_1 ( XMP_StringPtr namespaceURI,
                             WXMP_Result * wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_RegisterAlias_1 ( XMP_StringPtr  aliasNS,
                           XMP_StringPtr  aliasProp,
                           XMP_StringPtr  actualNS,
                           XMP_StringPtr  actualProp,
                           XMP_OptionBits arrayForm,
                           WXMP_Result *  wResult );

extern void
WXMPMeta_ResolveAlias_1 ( XMP_StringPtr    aliasNS,
                          XMP_StringPtr    aliasProp,
                          XMP_StringPtr *  actualNS,
                          XMP_StringLen *  nsSize,
                          XMP_StringPtr *  actualProp,
                          XMP_StringLen *  propSize,
                          XMP_OptionBits * arrayForm,
                          WXMP_Result *    wResult );

extern void
WXMPMeta_DeleteAlias_1 ( XMP_StringPtr aliasNS,
                         XMP_StringPtr aliasProp,
                         WXMP_Result * wResult );

extern void
WXMPMeta_RegisterStandardAliases_1 ( XMP_StringPtr schemaNS,
                                     WXMP_Result * wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_UnlockObject_1 ( XMPMetaRef     xmpRef,
                          XMP_OptionBits options );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_GetProperty_1 ( XMPMetaRef       xmpRef,
                         XMP_StringPtr    schemaNS,
                         XMP_StringPtr    propName,
                         XMP_StringPtr *  propValue,
                         XMP_StringLen *  valueSize,
                         XMP_OptionBits * options,
                         WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_GetArrayItem_1 ( XMPMetaRef       xmpRef,
                          XMP_StringPtr    schemaNS,
                          XMP_StringPtr    arrayName,
                          XMP_Index        itemIndex,
                          XMP_StringPtr *  itemValue,
                          XMP_StringLen *  valueSize,
                          XMP_OptionBits * options,
                          WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_GetStructField_1 ( XMPMetaRef       xmpRef,
                            XMP_StringPtr    schemaNS,
                            XMP_StringPtr    structName,
                            XMP_StringPtr    fieldNS,
                            XMP_StringPtr    fieldName,
                            XMP_StringPtr *  fieldValue,
                            XMP_StringLen *  valueSize,
                            XMP_OptionBits * options,
                            WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_GetQualifier_1 ( XMPMetaRef       xmpRef,
                          XMP_StringPtr    schemaNS,
                          XMP_StringPtr    propName,
                          XMP_StringPtr    qualNS,
                          XMP_StringPtr    qualName,
                          XMP_StringPtr *  qualValue,
                          XMP_StringLen *  valueSize,
                          XMP_OptionBits * options,
                          WXMP_Result *    wResult ) /* const */ ;

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_SetProperty_1 ( XMPMetaRef     xmpRef,
                         XMP_StringPtr  schemaNS,
                         XMP_StringPtr  propName,
                         XMP_StringPtr  propValue,
                         XMP_OptionBits options,
                         WXMP_Result *  wResult );

extern void
WXMPMeta_SetArrayItem_1 ( XMPMetaRef     xmpRef,
                          XMP_StringPtr  schemaNS,
                          XMP_StringPtr  arrayName,
                          XMP_Index      itemIndex,
                          XMP_StringPtr  itemValue,
                          XMP_OptionBits options,
                          WXMP_Result *  wResult );

extern void
WXMPMeta_AppendArrayItem_1 ( XMPMetaRef     xmpRef,
                             XMP_StringPtr  schemaNS,
                             XMP_StringPtr  arrayName,
                             XMP_OptionBits arrayOptions,
                             XMP_StringPtr  itemValue,
                             XMP_OptionBits options,
                             WXMP_Result *  wResult );

extern void
WXMPMeta_SetStructField_1 ( XMPMetaRef     xmpRef,
                            XMP_StringPtr  schemaNS,
                            XMP_StringPtr  structName,
                            XMP_StringPtr  fieldNS,
                            XMP_StringPtr  fieldName,
                            XMP_StringPtr  fieldValue,
                            XMP_OptionBits options,
                            WXMP_Result *  wResult );

extern void
WXMPMeta_SetQualifier_1 ( XMPMetaRef     xmpRef,
                          XMP_StringPtr  schemaNS,
                          XMP_StringPtr  propName,
                          XMP_StringPtr  qualNS,
                          XMP_StringPtr  qualName,
                          XMP_StringPtr  qualValue,
                          XMP_OptionBits options,
                          WXMP_Result *  wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_DeleteProperty_1 ( XMPMetaRef    xmpRef,
                            XMP_StringPtr schemaNS,
                            XMP_StringPtr propName,
                            WXMP_Result * wResult );

extern void
WXMPMeta_DeleteArrayItem_1 ( XMPMetaRef    xmpRef,
                             XMP_StringPtr schemaNS,
                             XMP_StringPtr arrayName,
                             XMP_Index     itemIndex,
                             WXMP_Result * wResult );

extern void
WXMPMeta_DeleteStructField_1 ( XMPMetaRef    xmpRef,
                               XMP_StringPtr schemaNS,
                               XMP_StringPtr structName,
                               XMP_StringPtr fieldNS,
                               XMP_StringPtr fieldName,
                               WXMP_Result * wResult );

extern void
WXMPMeta_DeleteQualifier_1 ( XMPMetaRef    xmpRef,
                             XMP_StringPtr schemaNS,
                             XMP_StringPtr propName,
                             XMP_StringPtr qualNS,
                             XMP_StringPtr qualName,
                             WXMP_Result * wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_DoesPropertyExist_1 ( XMPMetaRef    xmpRef,
                               XMP_StringPtr schemaNS,
                               XMP_StringPtr propName,
                               WXMP_Result * wResult ) /* const */ ;

extern void
WXMPMeta_DoesArrayItemExist_1 ( XMPMetaRef    xmpRef,
                                XMP_StringPtr schemaNS,
                                XMP_StringPtr arrayName,
                                XMP_Index     itemIndex,
                                WXMP_Result * wResult ) /* const */ ;

extern void
WXMPMeta_DoesStructFieldExist_1 ( XMPMetaRef    xmpRef,
                                  XMP_StringPtr schemaNS,
                                  XMP_StringPtr structName,
                                  XMP_StringPtr fieldNS,
                                  XMP_StringPtr fieldName,
                                  WXMP_Result * wResult ) /* const */ ;

extern void
WXMPMeta_DoesQualifierExist_1 ( XMPMetaRef    xmpRef,
                                XMP_StringPtr schemaNS,
                                XMP_StringPtr propName,
                                XMP_StringPtr qualNS,
                                XMP_StringPtr qualName,
                                WXMP_Result * wResult ) /* const */ ;

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_GetLocalizedText_1 ( XMPMetaRef       xmpRef,
                              XMP_StringPtr    schemaNS,
                              XMP_StringPtr    altTextName,
                              XMP_StringPtr    genericLang,
                              XMP_StringPtr    specificLang,
                              XMP_StringPtr *  actualLang,
                              XMP_StringLen *  langSize,
                              XMP_StringPtr *  itemValue,
                              XMP_StringLen *  valueSize,
                              XMP_OptionBits * options,
                              WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_SetLocalizedText_1 ( XMPMetaRef     xmpRef,
                              XMP_StringPtr  schemaNS,
                              XMP_StringPtr  altTextName,
                              XMP_StringPtr  genericLang,
                              XMP_StringPtr  specificLang,
                              XMP_StringPtr  itemValue,
                              XMP_OptionBits options,
                              WXMP_Result *  wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_GetProperty_Bool_1 ( XMPMetaRef       xmpRef,
                              XMP_StringPtr    schemaNS,
                              XMP_StringPtr    propName,
                              XMP_Bool *       propValue,
                              XMP_OptionBits * options,
                              WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_GetProperty_Int_1 ( XMPMetaRef       xmpRef,
                             XMP_StringPtr    schemaNS,
                             XMP_StringPtr    propName,
                             XMP_Int32 *      propValue,
                             XMP_OptionBits * options,
                             WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_GetProperty_Int64_1 ( XMPMetaRef       xmpRef,
                               XMP_StringPtr    schemaNS,
                               XMP_StringPtr    propName,
                               XMP_Int64 *      propValue,
                               XMP_OptionBits * options,
                               WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_GetProperty_Float_1 ( XMPMetaRef       xmpRef,
                               XMP_StringPtr    schemaNS,
                               XMP_StringPtr    propName,
                               double *         propValue,
                               XMP_OptionBits * options,
                               WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_GetProperty_Date_1 ( XMPMetaRef       xmpRef,
                              XMP_StringPtr    schemaNS,
                              XMP_StringPtr    propName,
                              XMP_DateTime *   propValue,
                              XMP_OptionBits * options,
                              WXMP_Result *    wResult ) /* const */ ;

extern void
WXMPMeta_SetProperty_Bool_1 ( XMPMetaRef     xmpRef,
                              XMP_StringPtr  schemaNS,
                              XMP_StringPtr  propName,
                              XMP_Bool       propValue,
                              XMP_OptionBits options,
                              WXMP_Result *  wResult );

extern void
WXMPMeta_SetProperty_Int_1 ( XMPMetaRef     xmpRef,
                             XMP_StringPtr  schemaNS,
                             XMP_StringPtr  propName,
                             XMP_Int32      propValue,
                             XMP_OptionBits options,
                             WXMP_Result *  wResult );

extern void
WXMPMeta_SetProperty_Int64_1 ( XMPMetaRef     xmpRef,
                               XMP_StringPtr  schemaNS,
                               XMP_StringPtr  propName,
                               XMP_Int64      propValue,
                               XMP_OptionBits options,
                               WXMP_Result *  wResult );

extern void
WXMPMeta_SetProperty_Float_1 ( XMPMetaRef     xmpRef,
                               XMP_StringPtr  schemaNS,
                               XMP_StringPtr  propName,
                               double         propValue,
                               XMP_OptionBits options,
                               WXMP_Result *  wResult );

extern void
WXMPMeta_SetProperty_Date_1 ( XMPMetaRef           xmpRef,
                              XMP_StringPtr        schemaNS,
                              XMP_StringPtr        propName,
                              const XMP_DateTime & propValue,
                              XMP_OptionBits       options,
                              WXMP_Result *        wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_GetObjectName_1 ( XMPMetaRef      xmpRef,
                           XMP_StringPtr * namePtr,
                           XMP_StringLen * nameLen,
                           WXMP_Result *   wResult ) /* const */ ;

extern void
WXMPMeta_SetObjectName_1 ( XMPMetaRef    xmpRef,
                           XMP_StringPtr name,
                           WXMP_Result * wResult );

extern void
WXMPMeta_GetObjectOptions_1 ( XMPMetaRef    xmpRef,
                              WXMP_Result * wResult ) /* const */ ;

extern void
WXMPMeta_SetObjectOptions_1 ( XMPMetaRef     xmpRef,
                              XMP_OptionBits options,
                              WXMP_Result *  wResult );

extern void
WXMPMeta_Sort_1 ( XMPMetaRef    xmpRef,
                  WXMP_Result * wResult );

extern void
WXMPMeta_Erase_1 ( XMPMetaRef    xmpRef,
                   WXMP_Result * wResult );

extern void
WXMPMeta_Clone_1 ( XMPMetaRef     xmpRef,
                   XMP_OptionBits options,
                   WXMP_Result *  wResult ) /* const */ ;

extern void
WXMPMeta_CountArrayItems_1 ( XMPMetaRef    xmpRef,
                             XMP_StringPtr schemaNS,
                             XMP_StringPtr arrayName,
                             WXMP_Result * wResult ) /* const */ ;

extern void
WXMPMeta_DumpObject_1 ( XMPMetaRef         xmpRef,
                        XMP_TextOutputProc outProc,
                        void *             refCon,
                        WXMP_Result *      wResult ) /* const */ ;

// -------------------------------------------------------------------------------------------------

extern void
WXMPMeta_ParseFromBuffer_1 ( XMPMetaRef     xmpRef,
                             XMP_StringPtr  buffer,
                             XMP_StringLen  bufferSize,
                             XMP_OptionBits options,
                             WXMP_Result *  wResult );

extern void
WXMPMeta_SerializeToBuffer_1 ( XMPMetaRef      xmpRef,
                               XMP_StringPtr * pktString,
                               XMP_StringLen * pktSize,
                               XMP_OptionBits  options,
                               XMP_StringLen   padding,
                               XMP_StringPtr   newline,
                               XMP_StringPtr   indent,
                               XMP_Index       baseIndent,
                               WXMP_Result *   wResult ) /* const */ ;

// =================================================================================================

#if __cplusplus
} /* extern "C" */
#endif

#endif  // __WXMPMeta_hpp__
