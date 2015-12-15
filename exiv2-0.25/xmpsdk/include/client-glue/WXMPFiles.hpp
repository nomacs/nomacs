#ifndef __WXMPFiles_hpp__
#define __WXMPFiles_hpp__	1

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "client-glue/WXMP_Common.hpp"

#if __cplusplus
extern "C" {
#endif

// =================================================================================================
/// \file WXMPFiles.h
/// \brief High level support to access metadata in files of interest to Adobe applications.
///
/// This header ...
///
// =================================================================================================

// =================================================================================================

#define WrapCheckXMPFilesRef(result,WCallProto) \
    WXMP_Result wResult;                        \
    WCallProto;                                 \
    PropagateException ( wResult );             \
    XMPFilesRef result = XMPFilesRef(wResult.ptrResult)

// =================================================================================================

#define zXMPFiles_GetVersionInfo_1(versionInfo) \
	WXMPFiles_GetVersionInfo_1 ( versionInfo /* no wResult */ )

#define zXMPFiles_Initialize_1() \
	WXMPFiles_Initialize_1 ( &wResult )

#define zXMPFiles_Initialize_2(options) \
	WXMPFiles_Initialize_2 ( options, &wResult )

#define zXMPFiles_Terminate_1() \
	WXMPFiles_Terminate_1 ( /* no wResult */ )

#define zXMPFiles_CTor_1() \
	WXMPFiles_CTor_1 ( &wResult )

#define zXMPFiles_GetFormatInfo_1(format,flags) \
	WXMPFiles_GetFormatInfo_1 ( format, flags, &wResult )

#define zXMPFiles_CheckFileFormat_1(filePath) \
	WXMPFiles_CheckFileFormat_1 ( filePath, &wResult )

#define zXMPFiles_CheckPackageFormat_1(folderPath) \
	WXMPFiles_CheckPackageFormat_1 ( folderPath, &wResult )

#define zXMPFiles_OpenFile_1(filePath,format,openFlags) \
	WXMPFiles_OpenFile_1 ( this->xmpFilesRef, filePath, format, openFlags, &wResult )
    
#define zXMPFiles_CloseFile_1(closeFlags) \
	WXMPFiles_CloseFile_1 ( this->xmpFilesRef, closeFlags, &wResult )
	
#define zXMPFiles_GetFileInfo_1(filePath,filePathLen,openFlags,format,handlerFlags) \
	WXMPFiles_GetFileInfo_1 ( this->xmpFilesRef, filePath, filePathLen, openFlags, format, handlerFlags, &wResult )
    
#define zXMPFiles_SetAbortProc_1(abortProc,abortArg) \
	WXMPFiles_SetAbortProc_1 ( this->xmpFilesRef, abortProc, abortArg, &wResult )
    
#define zXMPFiles_GetXMP_1(xmpRef,xmpPacket,xmpPacketLen,packetInfo) \
	WXMPFiles_GetXMP_1 ( this->xmpFilesRef, xmpRef, xmpPacket, xmpPacketLen, packetInfo, &wResult )
    
#define zXMPFiles_GetThumbnail_1(tnailInfo) \
	WXMPFiles_GetThumbnail_1 ( this->xmpFilesRef, tnailInfo, &wResult )
    
#define zXMPFiles_PutXMP_1(xmpRef,xmpPacket,xmpPacketLen) \
	WXMPFiles_PutXMP_1 ( this->xmpFilesRef, xmpRef, xmpPacket, xmpPacketLen, &wResult )
    
#define zXMPFiles_CanPutXMP_1(xmpRef,xmpPacket,xmpPacketLen) \
	WXMPFiles_CanPutXMP_1 ( this->xmpFilesRef, xmpRef, xmpPacket, xmpPacketLen, &wResult )

// =================================================================================================

extern void WXMPFiles_GetVersionInfo_1 ( XMP_VersionInfo * versionInfo );

extern void WXMPFiles_Initialize_1 ( WXMP_Result * result );

extern void WXMPFiles_Initialize_2 ( XMP_OptionBits options, WXMP_Result * result );

extern void WXMPFiles_Terminate_1();

extern void WXMPFiles_CTor_1 ( WXMP_Result * result );

extern void WXMPFiles_UnlockLib_1();

extern void WXMPFiles_UnlockObj_1 ( XMPFilesRef xmpFilesRef );

extern void WXMPFiles_IncrementRefCount_1 ( XMPFilesRef xmpFilesRef );

extern void WXMPFiles_DecrementRefCount_1 ( XMPFilesRef xmpFilesRef );

extern void WXMPFiles_GetFormatInfo_1 ( XMP_FileFormat   format,
                                        XMP_OptionBits * flags,	// ! Can be null.
                                        WXMP_Result *    result );

extern void WXMPFiles_CheckFileFormat_1 ( XMP_StringPtr filePath,
                               			  WXMP_Result * result );

extern void WXMPFiles_CheckPackageFormat_1 ( XMP_StringPtr folderPath,
                      						 WXMP_Result * result );

extern void WXMPFiles_OpenFile_1 ( XMPFilesRef    xmpFilesRef,
                                   XMP_StringPtr  filePath,
					               XMP_FileFormat format,
					               XMP_OptionBits openFlags,
                                   WXMP_Result *  result );
    
extern void WXMPFiles_CloseFile_1 ( XMPFilesRef    xmpFilesRef,
                                    XMP_OptionBits closeFlags,
                                    WXMP_Result *  result );
	
extern void WXMPFiles_GetFileInfo_1 ( XMPFilesRef      xmpFilesRef,
                                      XMP_StringPtr *  filePath,
                                      XMP_StringLen *  filePathLen,
					                  XMP_OptionBits * openFlags,		// ! Can be null.
					                  XMP_FileFormat * format,		// ! Can be null.
					                  XMP_OptionBits * handlerFlags,	// ! Can be null.
                                      WXMP_Result *    result );
    
extern void WXMPFiles_SetAbortProc_1 ( XMPFilesRef   xmpFilesRef,
                                       XMP_AbortProc abortProc,
									   void *        abortArg,
                                       WXMP_Result * result );
    
extern void WXMPFiles_GetXMP_1 ( XMPFilesRef      xmpFilesRef,
                                 XMPMetaRef       xmpRef,		// ! Can be null.
    			                 XMP_StringPtr *  xmpPacket,
    			                 XMP_StringLen *  xmpPacketLen,
    			                 XMP_PacketInfo * packetInfo,	// ! Can be null.
                                 WXMP_Result *    result );
    
extern void WXMPFiles_GetThumbnail_1 ( XMPFilesRef         xmpFilesRef,
    			                       XMP_ThumbnailInfo * tnailInfo,	// ! Can be null.
                                       WXMP_Result *       result );
    
extern void WXMPFiles_PutXMP_1 ( XMPFilesRef   xmpFilesRef,
                                 XMPMetaRef    xmpRef,		// ! Only one of the XMP object or packet are passed.
                                 XMP_StringPtr xmpPacket,
                                 XMP_StringLen xmpPacketLen,
                                 WXMP_Result * result );
    
extern void WXMPFiles_CanPutXMP_1 ( XMPFilesRef   xmpFilesRef,
                                    XMPMetaRef	  xmpRef,		// ! Only one of the XMP object or packet are passed.
                                    XMP_StringPtr xmpPacket,
                                    XMP_StringLen xmpPacketLen,
                                    WXMP_Result * result );

// =================================================================================================

#if __cplusplus
}
#endif

#endif // __WXMPFiles_hpp__
