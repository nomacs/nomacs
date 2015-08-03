#ifndef __XMP_hpp__
#define __XMP_hpp__ 1

// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

//  ================================================================================================
/// \file XMPSDK.hpp
/// \brief Overall header file for the XMP Toolkit
///
/// This is an overall header file, the only one that C++ clients should include.
///
/// The full client API is in the \c TXMPMeta.hpp, \c TXMPIterator.hpp, \c TXMPUtils.hpp headers.
/// Read these for information, but do not include them directly. The \c TXMP... classes are C++
/// template classes that must be instantiated with a string class such as \c std::string. The
/// string class is used to return text strings for property values, serialized XMP, and so on.
/// Clients must also compile \c XMP.incl_cpp to ensure that all client-side glue code is generated.
/// This should be done by including it in exactly one client source file.
///
/// There are two C preprocessor macros that simplify use of the templates:
///
/// \li \c TXMP_STRING_TYPE - Define this as the string class to use with the template. You will get
/// the template headers included and typedefs (\c SXMPMeta, and so on) to use in your code.
///
/// \li \c TXMP_EXPAND_INLINE - Define this as 1 if you want to have the template functions expanded
/// inline in your code. Leave it undefined, or defined as 0, to use out-of-line instantiations of
/// the template functions. Compiling \c XMP.incl_cpp generates explicit out-of-line
/// instantiations if \c TXMP_EXPAND_INLINE is off.
///
/// The template parameter, class \c tStringObj, must have the following member functions (which
/// match those for \c std::string):
///
/// <pre>
///  tStringObj& assign ( const char * str, size_t len )
///  size_t size() const
///  const char * c_str() const
/// </pre>
///
/// The string class must be suitable for at least UTF-8. This is the encoding used for all general
/// values, and is the default encoding for serialized XMP. The string type must also be suitable
/// for UTF-16 or UTF-32 if those serialization encodings are used. This mainly means tolerating
/// embedded 0 bytes, which \c std::string does.
//  ================================================================================================

/// /c XMP_Environment.h must be the first included header.
#include "XMP_Environment.h"

#include "XMP_Version.h"
#include "XMP_Const.h"

#ifdef _MSC_VER
    #if XMP_DebugBuild
        #pragma warning ( push, 4 )
    #else
        #pragma warning ( push, 3 )
    #endif
    #pragma warning ( disable : 4702 ) // unreachable code
    #pragma warning ( disable : 4800 ) // forcing value to bool 'true' or 'false' (performance warning)
#endif

#if defined ( TXMP_STRING_TYPE )

    #include "TXMPMeta.hpp"
    #include "TXMPIterator.hpp"
    #include "TXMPUtils.hpp"
    typedef class TXMPMeta <TXMP_STRING_TYPE>     SXMPMeta;       // For client convenience.
    typedef class TXMPIterator <TXMP_STRING_TYPE> SXMPIterator;
    typedef class TXMPUtils <TXMP_STRING_TYPE>    SXMPUtils;
    #if TXMP_EXPAND_INLINE
    	#error "TXMP_EXPAND_INLINE is not working at present. Please don't use it."
        #include "client-glue/TXMPMeta.incl_cpp"
        #include "client-glue/TXMPIterator.incl_cpp"
        #include "client-glue/TXMPUtils.incl_cpp"
    #endif

#endif  // TXMP_STRING_TYPE

#ifdef _MSC_VER
    #pragma warning ( pop )
#endif

// =================================================================================================

#endif  // __XMP_hpp__
