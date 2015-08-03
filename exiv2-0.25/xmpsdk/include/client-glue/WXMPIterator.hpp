#if ! __WXMPIterator_hpp__
#define __WXMPIterator_hpp__ 1

// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
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

#define zXMPIterator_PropCTor_1(xmpRef,schemaNS,propName,options) \
    WXMPIterator_PropCTor_1 ( xmpRef, schemaNS, propName, options, &wResult );

#define zXMPIterator_TableCTor_1(schemaNS,propName,options) \
    WXMPIterator_TableCTor_1 ( schemaNS, propName, options, &wResult );
    

#define zXMPIterator_Next_1(schemaNS,nsSize,propPath,pathSize,propValue,valueSize,options) \
    WXMPIterator_Next_1 ( this->iterRef, schemaNS, nsSize, propPath, pathSize, propValue, valueSize, options, &wResult );

#define zXMPIterator_Skip_1(options) \
    WXMPIterator_Skip_1 ( this->iterRef, options, &wResult );

// -------------------------------------------------------------------------------------------------

extern void
WXMPIterator_PropCTor_1 ( XMPMetaRef     xmpRef,
                          XMP_StringPtr  schemaNS,
                          XMP_StringPtr  propName,
                          XMP_OptionBits options,
                          WXMP_Result *  wResult );

extern void
WXMPIterator_TableCTor_1 ( XMP_StringPtr  schemaNS,
                           XMP_StringPtr  propName,
                           XMP_OptionBits options,
                           WXMP_Result *  wResult );

extern void
WXMPIterator_IncrementRefCount_1 ( XMPIteratorRef iterRef );

extern void
WXMPIterator_DecrementRefCount_1 ( XMPIteratorRef iterRef );

extern void
WXMPIterator_Unlock_1 ( XMP_OptionBits options );

extern void
WXMPIterator_Next_1 ( XMPIteratorRef   iterRef,
                      XMP_StringPtr *  schemaNS,
                      XMP_StringLen *  nsSize,
                      XMP_StringPtr *  propPath,
                      XMP_StringLen *  pathSize,
                      XMP_StringPtr *  propValue,
                      XMP_StringLen *  valueSize,
                      XMP_OptionBits * options,
                      WXMP_Result *    wResult );

extern void
WXMPIterator_Skip_1 ( XMPIteratorRef iterRef,
                      XMP_OptionBits options,
                      WXMP_Result *  wResult );

extern void
WXMPUtils_UnlockIter_1 ( XMPIteratorRef iterRef,
                         XMP_OptionBits options );

// =================================================================================================

#if __cplusplus
} /* extern "C" */
#endif

#endif  // __WXMPIterator_hpp__
