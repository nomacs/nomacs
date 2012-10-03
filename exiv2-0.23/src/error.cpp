// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      error.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   02-Apr-05, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: error.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "error.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <iostream>
#include <string>
#include <cassert>

// *****************************************************************************
namespace {

    //! Helper structure defining an error message.
    struct ErrMsg {
        //! Comparison operator
        bool operator==(int code) const { return code_ == code; }

        int code_;                              //!< Error code
        const char* message_;                   //!< Error message
    };

    //! Complete list of Exiv2 exception error messages
    const ErrMsg errList[] = {
        { -1, N_("Error %0: arg2=%2, arg3=%3, arg1=%1.") },
        {  0, N_("Success") },
        {  1, "%1" }, // %1=error message
        {  2, "%1: Call to `%3' failed: %2" }, // %1=path, %2=strerror, %3=function that failed
        {  3, N_("This does not look like a %1 image") }, // %1=Image type
        {  4, N_("Invalid dataset name `%1'") }, // %1=dataset name
        {  5, N_("Invalid record name `%1'") }, // %1=record name
        {  6, N_("Invalid key `%1'") }, // %1=key
        {  7, N_("Invalid tag name or ifdId `%1', ifdId %2") }, // %1=tag name, %2=ifdId
        {  8, N_("Value not set") },
        {  9, N_("%1: Failed to open the data source: %2") }, // %1=path, %2=strerror
        { 10, N_("%1: Failed to open file (%2): %3") }, // %1=path, %2=mode, %3=strerror
        { 11, N_("%1: The file contains data of an unknown image type") }, // %1=path
        { 12, N_("The memory contains data of an unknown image type") },
        { 13, N_("Image type %1 is not supported") }, // %1=image type
        { 14, N_("Failed to read image data") },
        { 15, N_("This does not look like a JPEG image") },
        { 16, N_("%1: Failed to map file for reading and writing: %2") }, // %1=path, %2=strerror
        { 17, N_("%1: Failed to rename file to %2: %3") }, // %1=old path, %2=new path, %3=strerror
        { 18, N_("%1: Transfer failed: %2") }, // %1=path, %2=strerror
        { 19, N_("Memory transfer failed: %1") }, // %1=strerror
        { 20, N_("Failed to read input data") },
        { 21, N_("Failed to write image") },
        { 22, N_("Input data does not contain a valid image") },
        { 23, N_("Invalid ifdId %1") }, // %1=ifdId
        { 24, N_("Entry::setValue: Value too large (tag=%1, size=%2, requested=%3)") }, // %1=tag, %2=dataSize, %3=required size
        { 25, N_("Entry::setDataArea: Value too large (tag=%1, size=%2, requested=%3)") }, // %1=tag, %2=dataAreaSize, %3=required size
        { 26, N_("Offset out of range") },
        { 27, N_("Unsupported data area offset type") },
        { 28, N_("Invalid charset: `%1'") }, // %1=charset name
        { 29, N_("Unsupported date format") },
        { 30, N_("Unsupported time format") },
        { 31, N_("Writing to %1 images is not supported") }, // %1=image format
        { 32, N_("Setting %1 in %2 images is not supported") }, // %1=metadata type, %2=image format
        { 33, N_("This does not look like a CRW image") },
        { 34, N_("%1: Not supported") }, // %1=function
        { 35, N_("No namespace info available for XMP prefix `%1'") }, // %1=prefix
        { 36, N_("No prefix registered for namespace `%2', needed for property path `%1'") }, // %1=namespace
        { 37, N_("Size of %1 JPEG segment is larger than 65535 bytes") }, // %1=type of metadata (Exif, IPTC, JPEG comment)
        { 38, N_("Unhandled Xmpdatum %1 of type %2") }, // %1=key, %2=value type
        { 39, N_("Unhandled XMP node %1 with opt=%2") }, // %1=key, %2=XMP Toolkit option flags
        { 40, N_("XMP Toolkit error %1: %2") }, // %1=XMP_Error::GetID(), %2=XMP_Error::GetErrMsg()
        { 41, N_("Failed to decode Lang Alt property %1 with opt=%2") }, // %1=property path, %3=XMP Toolkit option flags
        { 42, N_("Failed to decode Lang Alt qualifier %1 with opt=%2") }, // %1=qualifier path, %3=XMP Toolkit option flags
        { 43, N_("Failed to encode Lang Alt property %1") }, // %1=key
        { 44, N_("Failed to determine property name from path %1, namespace %2") }, // %1=property path, %2=namespace
        { 45, N_("Schema namespace %1 is not registered with the XMP Toolkit") }, // %1=namespace
        { 46, N_("No namespace registered for prefix `%1'") }, // %1=prefix
        { 47, N_("Aliases are not supported. Please send this XMP packet to ahuggel@gmx.net `%1', `%2', `%3'") }, // %1=namespace, %2=property path, %3=value
        { 48, N_("Invalid XmpText type `%1'") }, // %1=type
        { 49, N_("TIFF directory %1 has too many entries") }, // %1=TIFF directory name
        { 50, N_("Multiple TIFF array element tags %1 in one directory") }, // %1=tag number
        { 51, N_("TIFF array element tag %1 has wrong type") }, // %1=tag number
        { 52, N_("%1 has invalid XMP value type `%2'") } // %1=key, %2=value type
    };

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    LogMsg::Level LogMsg::level_ = LogMsg::warn; // Default output level
    LogMsg::Handler LogMsg::handler_ = LogMsg::defaultHandler;

    void LogMsg::defaultHandler(int level, const char* s)
    {
        switch (static_cast<LogMsg::Level>(level)) {
        case LogMsg::debug: std::cerr << "Debug: "; break;
        case LogMsg::info:  std::cerr << "Info: "; break;
        case LogMsg::warn:  std::cerr << "Warning: "; break;
        case LogMsg::error: std::cerr << "Error: "; break;
        case LogMsg::mute:  assert(false);
        }
        std::cerr << s;
    }

    AnyError::~AnyError() throw()
    {
    }

    //! @cond IGNORE
    template<>
    void BasicError<char>::setMsg()
    {
        std::string msg = _(errMsg(code_));
        std::string::size_type pos;
        pos = msg.find("%0");
        if (pos != std::string::npos) {
            msg.replace(pos, 2, toString(code_));
        }
        if (count_ > 0) {
            pos = msg.find("%1");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg1_);
            }
        }
        if (count_ > 1) {
            pos = msg.find("%2");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg2_);
            }
        }
        if (count_ > 2) {
            pos = msg.find("%3");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg3_);
            }
        }
        msg_ = msg;
#ifdef EXV_UNICODE_PATH
        wmsg_ = s2ws(msg);
#endif
    }
    //! @endcond

#ifdef EXV_UNICODE_PATH
    template<>
    void BasicError<wchar_t>::setMsg()
    {
        std::string s = _(errMsg(code_));
        std::wstring wmsg(s.begin(), s.end());
        std::wstring::size_type pos;
        pos = wmsg.find(L"%0");
        if (pos != std::wstring::npos) {
            wmsg.replace(pos, 2, toBasicString<wchar_t>(code_));
        }
        if (count_ > 0) {
            pos = wmsg.find(L"%1");
            if (pos != std::wstring::npos) {
                wmsg.replace(pos, 2, arg1_);
            }
        }
        if (count_ > 1) {
            pos = wmsg.find(L"%2");
            if (pos != std::wstring::npos) {
                wmsg.replace(pos, 2, arg2_);
            }
        }
        if (count_ > 2) {
            pos = wmsg.find(L"%3");
            if (pos != std::wstring::npos) {
                wmsg.replace(pos, 2, arg3_);
            }
        }
        wmsg_ = wmsg;
        msg_ = ws2s(wmsg);
    }
#endif

    const char* errMsg(int code)
    {
        const ErrMsg* em = find(errList, code);
        return em ? em->message_ : "";
    }

}                                       // namespace Exiv2
