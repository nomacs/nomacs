#ifndef __XMP_Const_h__
#define __XMP_Const_h__ 1

// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"

   #include <stddef.h>

#if XMP_MacBuild	// ! No stdint.h on Windows and some UNIXes.
    #include <stdint.h>
#endif

#if __cplusplus
extern "C" {
#endif

// =================================================================================================
/// \file XMP_Const.h
/// \brief Common C/C++ types and constants for the XMP toolkit.
// =================================================================================================

// =================================================================================================
// Basic types and constants
// =========================

// The XMP_... types are used on the off chance that the ..._t types present a problem. In that
// case only the declarations of the XMP_... types needs to change, not all of the uses. These
// types are used where fixed sizes are required in order to have a known ABI for a DLL build.

#if XMP_MacBuild

    typedef int8_t   XMP_Int8;
    typedef int16_t  XMP_Int16;
    typedef int32_t  XMP_Int32;
    typedef int64_t  XMP_Int64;

    typedef uint8_t  XMP_Uns8;
    typedef uint16_t XMP_Uns16;
    typedef uint32_t XMP_Uns32;
    typedef uint64_t XMP_Uns64;

#else

    typedef signed char XMP_Int8;
    typedef signed short XMP_Int16;
    typedef signed long XMP_Int32;
    typedef signed long long XMP_Int64;

    typedef unsigned char XMP_Uns8;
    typedef unsigned short XMP_Uns16;
    typedef unsigned long XMP_Uns32;
    typedef unsigned long long XMP_Uns64;

#endif

typedef XMP_Uns8 XMP_Bool;

/// An "ABI safe" pointer to the internal part of an XMP object. Use to pass an XMP object across
/// client DLL boundaries. See \c TXMPMeta::GetInternalRef().
typedef struct __XMPMeta__ *        XMPMetaRef;

/// An "ABI safe" pointer to the internal part of an XMP iteration object. Use to pass an XMP
/// iteration object across client DLL boundaries. See \c TXMPIterator.
typedef struct __XMPIterator__ *    XMPIteratorRef;

/// An "ABI safe" pointer to the internal part of an XMP document operations object. Use to pass an
/// XMP document operations object across client DLL boundaries. See \c TXMPDocOps.
typedef struct __XMPDocOps__ *    XMPDocOpsRef;

/// An "ABI safe" pointer to the internal part of an XMP file-handling object. Use to pass an XMP
/// file-handling object across  client DLL boundaries. See \c TXMPFiles.
typedef struct __XMPFiles__ *       XMPFilesRef;

// =================================================================================================

/// \name General scalar types and constants
/// @{

/// \typedef XMP_StringPtr
/// \brief The type for input string parameters. A <tt>const char *</tt>, a null-terminated UTF-8
/// string.

/// \typedef XMP_StringLen
/// \brief The type for string length parameters. A 32-bit unsigned integer, as big as will be
/// practically needed.

/// \typedef XMP_Index
/// \brief The type for offsets and indices. A 32-bit signed integer. It is signed to allow -1 for
/// loop termination.

/// \typedef XMP_OptionBits
/// \brief The type for a collection of 32 flag bits. Individual flags are defined as enum value bit
/// masks; see \c #kXMP_PropValueIsURI and following. A number of macros provide common set or set
/// operations, such as \c XMP_PropIsSimple. For other tests use an expression like <code>options &
/// kXMP_<theOption></code>. When passing multiple option flags use the bitwise-OR operator. '|',
/// not the arithmatic plus, '+'.

typedef const char * XMP_StringPtr;  // Points to a null terminated UTF-8 string.
typedef XMP_Uns32    XMP_StringLen;
typedef XMP_Int32    XMP_Index;      // Signed, sometimes -1 is handy.
typedef XMP_Uns32    XMP_OptionBits; // Used as 32 individual bits.

/// \def kXMP_TrueStr
/// \brief The canonical true string value for Booleans in serialized XMP.
///
/// Code that converts from string to bool should be case insensitive, and also allow "1".

/// \def kXMP_FalseStr
/// \brief The canonical false string value for Booleans in serialized XMP.
///
/// Code that converts	from string to bool should be case insensitive, and also allow "0".

#define kXMP_TrueStr  "True"  // Serialized XMP spellings, not for the type bool.
#define kXMP_FalseStr "False"

/// Type for yes/no/maybe answers. The values are picked to allow Boolean-like usage. The yes and
/// values are true (non-zero), the no value is false (zero).
enum {
	/// The part or parts have definitely changed.
	kXMPTS_Yes = 1,
	/// The part or parts have definitely not changed.
	kXMPTS_No = 0,
	/// The part or parts might, or might not, have changed.
	kXMPTS_Maybe = -1
};
typedef XMP_Int8 XMP_TriState;

/// @}

// =================================================================================================

/// \struct XMP_DateTime
/// \brief The expanded type for a date and time.
///
/// Dates and time in the serialized XMP are ISO 8601 strings. The \c XMP_DateTime struct allows
/// easy conversion with other formats.
///
/// All of the fields are 32 bit, even though most could be 8 bit. This avoids overflow when doing
/// carries for arithmetic or normalization. All fields have signed values for the same reasons.
///
/// Date-time values are occasionally used with only a date or only a time component. A date without
/// a time has zeros in the \c XMP_DateTime struct for all time fields. A time without a date has
/// zeros for all date fields (year, month, and day).
///
/// \c TXMPUtils provides utility functions for manipulating date-time values.
///
/// @see \c TXMPUtils::ConvertToDate(), \c TXMPUtils::ConvertFromDate(),
/// \c TXMPUtils::CompareDateTime(), \c TXMPUtils::ConvertToLocalTime(),
/// \c TXMPUtils::ConvertToUTCTime(), \c TXMPUtils::CurrentDateTime(),
/// \c TXMPUtils::SetTimeZone()

struct XMP_DateTime {

	/// The year, can be negative.
    XMP_Int32 year;

	/// The month in the range 1..12.
    XMP_Int32 month;

	/// The day of the month in the range 1..31.
    XMP_Int32 day;

	/// The hour in the range 0..23.
    XMP_Int32 hour;

	/// The minute in the range 0..59.
    XMP_Int32 minute;

	/// The second in the range 0..59.
    XMP_Int32 second;

	/// The "sign" of the time zone, \c #kXMP_TimeIsUTC (0) means UTC, \c #kXMP_TimeWestOfUTC (-1)
	/// is west, \c #kXMP_TimeEastOfUTC (+1) is east.
    XMP_Int32 tzSign;

	/// The time zone hour in the range 0..23.
    XMP_Int32 tzHour;

	/// The time zone minute in the range 0..59.
    XMP_Int32 tzMinute;

	/// Nanoseconds within a second, often left as zero.
    XMP_Int32 nanoSecond;

};

/// Constant values for \c XMP_DateTime::tzSign field.
enum {
	/// Time zone is west of UTC.
    kXMP_TimeWestOfUTC = -1,
	/// UTC time.
    kXMP_TimeIsUTC     =  0,
	/// Time zone is east of UTC.
    kXMP_TimeEastOfUTC = +1
};

// =================================================================================================
// Standard namespace URI constants
// ================================

/// \name XML namespace constants for standard XMP schema.
/// @{
///
/// \def kXMP_NS_XMP
/// \brief The XML namespace for the XMP "basic" schema.
///
/// \def kXMP_NS_XMP_Rights
/// \brief The XML namespace for the XMP copyright schema.
///
/// \def kXMP_NS_XMP_MM
/// \brief The XML namespace for the XMP digital asset management schema.
///
/// \def kXMP_NS_XMP_BJ
/// \brief The XML namespace for the job management schema.
///
/// \def kXMP_NS_XMP_T
/// \brief The XML namespace for the XMP text document schema.
///
/// \def kXMP_NS_XMP_T_PG
/// \brief The XML namespace for the XMP paged document schema.
///
/// \def kXMP_NS_PDF
/// \brief The XML namespace for the PDF schema.
///
/// \def kXMP_NS_Photoshop
/// \brief The XML namespace for the Photoshop custom schema.
///
/// \def kXMP_NS_EXIF
/// \brief The XML namespace for Adobe's EXIF schema.
///
/// \def kXMP_NS_TIFF
/// \brief The XML namespace for Adobe's TIFF schema.
///
/// @}

#define kXMP_NS_XMP        "http://ns.adobe.com/xap/1.0/"

#define kXMP_NS_XMP_Rights "http://ns.adobe.com/xap/1.0/rights/"
#define kXMP_NS_XMP_MM     "http://ns.adobe.com/xap/1.0/mm/"
#define kXMP_NS_XMP_BJ     "http://ns.adobe.com/xap/1.0/bj/"

#define kXMP_NS_PDF        "http://ns.adobe.com/pdf/1.3/"
#define kXMP_NS_Photoshop  "http://ns.adobe.com/photoshop/1.0/"
#define kXMP_NS_PSAlbum    "http://ns.adobe.com/album/1.0/"
#define kXMP_NS_EXIF       "http://ns.adobe.com/exif/1.0/"
#define kXMP_NS_EXIF_Aux   "http://ns.adobe.com/exif/1.0/aux/"
#define kXMP_NS_TIFF       "http://ns.adobe.com/tiff/1.0/"
#define kXMP_NS_PNG        "http://ns.adobe.com/png/1.0/"
#define kXMP_NS_SWF        "http://ns.adobe.com/swf/1.0/"
#define kXMP_NS_JPEG       "http://ns.adobe.com/jpeg/1.0/"
#define kXMP_NS_JP2K       "http://ns.adobe.com/jp2k/1.0/"
#define kXMP_NS_CameraRaw  "http://ns.adobe.com/camera-raw-settings/1.0/"
#define kXMP_NS_DM         "http://ns.adobe.com/xmp/1.0/DynamicMedia/"
#define kXMP_NS_ASF        "http://ns.adobe.com/asf/1.0/"
#define kXMP_NS_WAV        "http://ns.adobe.com/xmp/wav/1.0/"

#define kXMP_NS_XMP_Note   "http://ns.adobe.com/xmp/note/"

#define kXMP_NS_AdobeStockPhoto "http://ns.adobe.com/StockPhoto/1.0/"
#define kXMP_NS_CreatorAtom "http://ns.adobe.com/creatorAtom/1.0/"

/// \name XML namespace constants for qualifiers and structured property fields.
/// @{
///
/// \def kXMP_NS_XMP_IdentifierQual
/// \brief The XML namespace for qualifiers of the xmp:Identifier property.
///
/// \def kXMP_NS_XMP_Dimensions
/// \brief The XML namespace for fields of the Dimensions type.
///
/// \def kXMP_NS_XMP_Image
/// \brief The XML namespace for fields of a graphical image. Used for the Thumbnail type.
///
/// \def kXMP_NS_XMP_ResourceEvent
/// \brief The XML namespace for fields of the ResourceEvent type.
///
/// \def kXMP_NS_XMP_ResourceRef
/// \brief The XML namespace for fields of the ResourceRef type.
///
/// \def kXMP_NS_XMP_ST_Version
/// \brief The XML namespace for fields of the Version type.
///
/// \def kXMP_NS_XMP_ST_Job
/// \brief The XML namespace for fields of the JobRef type.
///
/// @}

#define kXMP_NS_XMP_IdentifierQual "http://ns.adobe.com/xmp/Identifier/qual/1.0/"
#define kXMP_NS_XMP_Dimensions     "http://ns.adobe.com/xap/1.0/sType/Dimensions#"
#define kXMP_NS_XMP_Text           "http://ns.adobe.com/xap/1.0/t/"
#define kXMP_NS_XMP_PagedFile      "http://ns.adobe.com/xap/1.0/t/pg/"
#define kXMP_NS_XMP_Graphics       "http://ns.adobe.com/xap/1.0/g/"
#define kXMP_NS_XMP_Image          "http://ns.adobe.com/xap/1.0/g/img/"
#define kXMP_NS_XMP_Font           "http://ns.adobe.com/xap/1.0/sType/Font#"
#define kXMP_NS_XMP_ResourceEvent  "http://ns.adobe.com/xap/1.0/sType/ResourceEvent#"
#define kXMP_NS_XMP_ResourceRef    "http://ns.adobe.com/xap/1.0/sType/ResourceRef#"
#define kXMP_NS_XMP_ST_Version     "http://ns.adobe.com/xap/1.0/sType/Version#"
#define kXMP_NS_XMP_ST_Job         "http://ns.adobe.com/xap/1.0/sType/Job#"
#define kXMP_NS_XMP_ManifestItem   "http://ns.adobe.com/xap/1.0/sType/ManifestItem#"

// Deprecated XML namespace constants
#define kXMP_NS_XMP_T     "http://ns.adobe.com/xap/1.0/t/"
#define kXMP_NS_XMP_T_PG  "http://ns.adobe.com/xap/1.0/t/pg/"
#define kXMP_NS_XMP_G_IMG "http://ns.adobe.com/xap/1.0/g/img/"

/// \name XML namespace constants from outside Adobe.
/// @{
///
/// \def kXMP_NS_DC
/// \brief The XML namespace for the Dublin Core schema.
///
/// \def kXMP_NS_IPTCCore
/// \brief The XML namespace for the IPTC Core schema.
///
/// \def kXMP_NS_RDF
/// \brief The XML namespace for RDF.
///
/// \def kXMP_NS_XML
/// \brief The XML namespace for XML.
///
/// @}

#define kXMP_NS_DC              "http://purl.org/dc/elements/1.1/"

#define kXMP_NS_IPTCCore       "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/"

#define kXMP_NS_DICOM          "http://ns.adobe.com/DICOM/"

#define kXMP_NS_PDFA_Schema    "http://www.aiim.org/pdfa/ns/schema#"
#define kXMP_NS_PDFA_Property  "http://www.aiim.org/pdfa/ns/property#"
#define kXMP_NS_PDFA_Type      "http://www.aiim.org/pdfa/ns/type#"
#define kXMP_NS_PDFA_Field     "http://www.aiim.org/pdfa/ns/field#"
#define kXMP_NS_PDFA_ID        "http://www.aiim.org/pdfa/ns/id/"
#define kXMP_NS_PDFA_Extension "http://www.aiim.org/pdfa/ns/extension/"

#define kXMP_NS_PDFX           "http://ns.adobe.com/pdfx/1.3/"
#define	kXMP_NS_PDFX_ID        "http://www.npes.org/pdfx/ns/id/"

#define kXMP_NS_RDF            "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define kXMP_NS_XML            "http://www.w3.org/XML/1998/namespace"

// =================================================================================================
// Enums and macros used for option bits
// =====================================

/// \name Macros for standard option selections.
/// @{
///
/// \def kXMP_ArrayLastItem
/// \brief Options macro accesses last array item.
///
/// \def kXMP_UseNullTermination
/// \brief Options macro sets string style.
///
/// \def kXMP_NoOptions
/// \brief Options macro clears all property-type bits.
///
/// @}

#define kXMP_ArrayLastItem      ((XMP_Index)(-1L))
#define kXMP_UseNullTermination ((XMP_StringLen)(~0UL))
#define kXMP_NoOptions          ((XMP_OptionBits)0UL)

/// \name Macros for setting and testing general option bits.
/// @{
///
/// \def XMP_SetOption
/// \brief Macro sets an option flag bit.
///	\param var A variable storing an options flag.
/// \param opt The bit-flag constant to set.
///
/// \def XMP_ClearOption
/// \brief Macro clears an option flag bit.
///	\param var A variable storing an options flag.
/// \param opt The bit-flag constant to clear.
///
/// \def XMP_TestOption
/// \brief Macro reports whether an option flag bit is set.
///	\param var A variable storing an options flag.
/// \param opt The bit-flag constant to test.
/// \return True if the bit is set.
///
/// \def XMP_OptionIsSet
/// \brief Macro reports whether an option flag bit is set.
///	\param var A variable storing an options flag.
/// \param opt The bit-flag constant to test.
/// \return True if the bit is set.
///
/// \def XMP_OptionIsClear
/// \brief Macro reports whether an option flag bit is clear.
///	\param var A variable storing an options flag.
/// \param opt The bit-flag constant to test.
/// \return True if the bit is clear.
///
/// @}

#define XMP_SetOption(var,opt)      var |= (opt)
#define XMP_ClearOption(var,opt)    var &= ~(opt)
#define XMP_TestOption(var,opt)     (((var) & (opt)) != 0)
#define XMP_OptionIsSet(var,opt)    (((var) & (opt)) != 0)
#define XMP_OptionIsClear(var,opt)  (((var) & (opt)) == 0)

/// \name Macros for setting and testing specific option bits.
/// @{
///
/// \def XMP_PropIsSimple
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_PropIsStruct
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_PropIsArray
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_ArrayIsUnordered
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_ArrayIsOrdered
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_ArrayIsAlternate
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_ArrayIsAltText
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_PropHasQualifiers
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_PropIsQualifier
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_PropHasLang
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_NodeIsSchema
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// \def XMP_PropIsAlias
/// \brief Macro reports the property type specified by an options flag.
/// \param opt The options flag to check.
///
/// @}

#define XMP_PropIsSimple(opt)       (((opt) & kXMP_PropCompositeMask) == 0)
#define XMP_PropIsStruct(opt)       (((opt) & kXMP_PropValueIsStruct) != 0)
#define XMP_PropIsArray(opt)        (((opt) & kXMP_PropValueIsArray) != 0)

#define XMP_ArrayIsUnordered(opt)   (((opt) & kXMP_PropArrayIsOrdered) == 0)
#define XMP_ArrayIsOrdered(opt)     (((opt) & kXMP_PropArrayIsOrdered) != 0)
#define XMP_ArrayIsAlternate(opt)   (((opt) & kXMP_PropArrayIsAlternate) != 0)
#define XMP_ArrayIsAltText(opt)     (((opt) & kXMP_PropArrayIsAltText) != 0)

#define XMP_PropHasQualifiers(opt)  (((opt) & kXMP_PropHasQualifiers) != 0)
#define XMP_PropIsQualifier(opt)    (((opt) & kXMP_PropIsQualifier) != 0)
#define XMP_PropHasLang(opt)        (((opt) & kXMP_PropHasLang) != 0)

#define XMP_NodeIsSchema(opt)       (((opt) & kXMP_SchemaNode) != 0)
#define XMP_PropIsAlias(opt)        (((opt) & kXMP_PropIsAlias) != 0)

// -------------------------------------------------------------------------------------------------

/// Option bit flags for the \c TXMPMeta property accessor functions.
enum {

	/// The XML string form of the property value is a URI, use rdf:resource attribute. DISCOURAGED
    kXMP_PropValueIsURI       = 0x00000002UL,

	// ------------------------------------------------------
    // Options relating to qualifiers attached to a property.

	/// The property has qualifiers, includes \c rdf:type and \c xml:lang.
    kXMP_PropHasQualifiers    = 0x00000010UL,

	/// This is a qualifier for some other property, includes \c rdf:type and \c xml:lang.
	/// Qualifiers can have arbitrary structure, and can themselves have qualifiers. If the
	/// qualifier itself has a structured value, this flag is only set for the top node of the
	/// qualifier's subtree.
    kXMP_PropIsQualifier      = 0x00000020UL,

	/// Implies \c #kXMP_PropHasQualifiers, property has \c xml:lang.
    kXMP_PropHasLang          = 0x00000040UL,

	/// Implies \c #kXMP_PropHasQualifiers, property has \c rdf:type.
    kXMP_PropHasType          = 0x00000080UL,

	// --------------------------------------------
    // Options relating to the data structure form.

	/// The value is a structure with nested fields.
    kXMP_PropValueIsStruct    = 0x00000100UL,

	/// The value is an array (RDF alt/bag/seq). The "ArrayIs..." flags identify specific types
	/// of array; default is a general unordered array, serialized using an \c rdf:Bag container.
    kXMP_PropValueIsArray     = 0x00000200UL,

	/// The item order does not matter.
    kXMP_PropArrayIsUnordered = kXMP_PropValueIsArray,

	/// Implies \c #kXMP_PropValueIsArray, item order matters. It is serialized using an \c rdf:Seq container.
    kXMP_PropArrayIsOrdered   = 0x00000400UL,

	/// Implies \c #kXMP_PropArrayIsOrdered, items are alternates. It is serialized using an \c rdf:Alt container.
    kXMP_PropArrayIsAlternate = 0x00000800UL,

	// ------------------------------------
    // Additional struct and array options.

	/// Implies \c #kXMP_PropArrayIsAlternate, items are localized text.  Each array element is a
	/// simple property with an \c xml:lang attribute.
    kXMP_PropArrayIsAltText   = 0x00001000UL,

    // kXMP_InsertBeforeItem  = 0x00004000UL,  ! Used by SetXyz functions.
    // kXMP_InsertAfterItem   = 0x00008000UL,  ! Used by SetXyz functions.

	// ----------------------------
    // Other miscellaneous options.

	/// This property is an alias name for another property.  This is only returned by
	/// \c TXMPMeta::GetProperty() and then only if the property name is simple, not an path expression.
    kXMP_PropIsAlias          = 0x00010000UL,

	/// This property is the base value (actual) for a set of aliases.This is only returned by
	/// \c TXMPMeta::GetProperty() and then only if the property name is simple, not an path expression.
    kXMP_PropHasAliases       = 0x00020000UL,

	/// The value of this property is "owned" by the application, and should not generally be editable in a UI.
    kXMP_PropIsInternal       = 0x00040000UL,

	/// The value of this property is not derived from the document content.
    kXMP_PropIsStable         = 0x00100000UL,

	/// The value of this property is derived from the document content.
    kXMP_PropIsDerived        = 0x00200000UL,

    // kXMPUtil_AllowCommas   = 0x10000000UL,  ! Used by TXMPUtils::CatenateArrayItems and ::SeparateArrayItems.
    // kXMP_DeleteExisting    = 0x20000000UL,  ! Used by TXMPMeta::SetXyz functions to delete any pre-existing property.
    // kXMP_SchemaNode        = 0x80000000UL,  ! Returned by iterators - #define to avoid warnings

	// ------------------------------
    // Masks that are multiple flags.

	/// Property type bit-flag mask for all array types
    kXMP_PropArrayFormMask    = kXMP_PropValueIsArray | kXMP_PropArrayIsOrdered | kXMP_PropArrayIsAlternate | kXMP_PropArrayIsAltText,

	/// Property type bit-flag mask for composite types (array and struct)
    kXMP_PropCompositeMask    = kXMP_PropValueIsStruct | kXMP_PropArrayFormMask,

	/// Mask for bits that are reserved for transient use by the implementation.
    kXMP_ImplReservedMask     = 0x70000000L

};

#define kXMP_SchemaNode ((XMP_OptionBits)0x80000000UL)

/// Option bit flags for the \c TXMPMeta property setting functions. These option bits are shared
/// with the accessor functions:
///   \li \c #kXMP_PropValueIsURI
///   \li \c #kXMP_PropValueIsStruct
///   \li \c #kXMP_PropValueIsArray
///   \li \c #kXMP_PropArrayIsOrdered
///   \li \c #kXMP_PropArrayIsAlternate
///   \li \c #kXMP_PropArrayIsAltText
enum {

    /// Option for array item location: Insert a new item before the given index.
    kXMP_InsertBeforeItem      = 0x00004000UL,

    /// Option for array item location: Insert a new item after the given index.
    kXMP_InsertAfterItem       = 0x00008000UL,

    /// Delete any pre-existing property.
    kXMP_DeleteExisting        = 0x20000000UL,

	/// Bit-flag mask for property-value option bits
    kXMP_PropValueOptionsMask  = kXMP_PropValueIsURI,

	/// Bit-flag mask for array-item location bits
    kXMP_PropArrayLocationMask = kXMP_InsertBeforeItem | kXMP_InsertAfterItem

};

// -------------------------------------------------------------------------------------------------

/// Option bit flags for \c TXMPMeta::ParseFromBuffer().
enum {

	/// Require a surrounding \c x:xmpmeta element.
    kXMP_RequireXMPMeta   = 0x0001UL,

	/// This is the not last input buffer for this parse stream.
    kXMP_ParseMoreBuffers = 0x0002UL,

	/// Do not reconcile alias differences, throw an exception.
    kXMP_StrictAliasing   = 0x0004UL

};

/// Option bit flags for \c TXMPMeta::SerializeToBuffer().
enum {

    // *** Option to remove empty struct/array, or leaf with empty value?

	/// Omit the XML packet wrapper.
    kXMP_OmitPacketWrapper   = 0x0010UL,

	/// Default is a writeable packet.
    kXMP_ReadOnlyPacket      = 0x0020UL,

	/// Use a compact form of RDF.
    kXMP_UseCompactFormat    = 0x0040UL,

	/// Include a padding allowance for a thumbnail image.
    kXMP_IncludeThumbnailPad = 0x0100UL,

	/// The padding parameter is the overall packet length.
    kXMP_ExactPacketLength   = 0x0200UL,

	/// Show aliases as XML comments.
    kXMP_WriteAliasComments  = 0x0400UL,

	/// Omit all formatting whitespace.
    kXMP_OmitAllFormatting   = 0x0800UL,
    
    /// Omit the x:xmpmeta element surrounding the rdf:RDF element.
    kXMP_OmitXMPMetaElement  = 0x1000UL,

    _XMP_LittleEndian_Bit    = 0x0001UL,  // ! Don't use directly, see the combined values below!
    _XMP_UTF16_Bit           = 0x0002UL,
    _XMP_UTF32_Bit           = 0x0004UL,

	/// Bit-flag mask for encoding-type bits
    kXMP_EncodingMask        = 0x0007UL,

	/// Use UTF8 encoding
    kXMP_EncodeUTF8          = 0UL,

	/// Use UTF16 big-endian encoding
    kXMP_EncodeUTF16Big      = _XMP_UTF16_Bit,

	/// Use UTF16 little-endian encoding
    kXMP_EncodeUTF16Little   = _XMP_UTF16_Bit | _XMP_LittleEndian_Bit,

	/// Use UTF32 big-endian encoding
    kXMP_EncodeUTF32Big      = _XMP_UTF32_Bit,

	/// Use UTF13 little-endian encoding
    kXMP_EncodeUTF32Little   = _XMP_UTF32_Bit | _XMP_LittleEndian_Bit

};

// -------------------------------------------------------------------------------------------------

/// Option bit flags for \c TXMPIterator construction.
enum {

	/// The low 8 bits are an enum of what data structure to iterate.
    kXMP_IterClassMask      = 0x00FFUL,

    /// Iterate the property tree of a TXMPMeta object.
    kXMP_IterProperties     = 0x0000UL,

	/// Iterate the global alias table.
    kXMP_IterAliases        = 0x0001UL,

	/// Iterate the global namespace table.
    kXMP_IterNamespaces     = 0x0002UL,

	/// Just do the immediate children of the root, default is subtree.
    kXMP_IterJustChildren   = 0x0100UL,

	/// Just do the leaf nodes, default is all nodes in the subtree.
    kXMP_IterJustLeafNodes  = 0x0200UL,

	/// Return just the leaf part of the path, default is the full path.
    kXMP_IterJustLeafName   = 0x0400UL,

	 /// Include aliases, default is just actual properties.
    kXMP_IterIncludeAliases = 0x0800UL,

	 /// Omit all qualifiers.
    kXMP_IterOmitQualifiers = 0x1000UL

};

/// Option bit flags for \c TXMPIterator::Skip().
enum {

	/// Skip the subtree below the current node.
    kXMP_IterSkipSubtree    = 0x0001UL,

	/// Skip the subtree below and remaining siblings of the current node.
    kXMP_IterSkipSiblings   = 0x0002UL

};

// -------------------------------------------------------------------------------------------------
/// Option bit flags for \c TXMPUtils::CatenateArrayItems() and \c TXMPUtils::SeparateArrayItems().
/// These option bits are shared with the accessor functions:
///   \li \c #kXMP_PropValueIsArray,
///   \li \c #kXMP_PropArrayIsOrdered, 
///   \li \c #kXMP_PropArrayIsAlternate,
///   \li \c #kXMP_PropArrayIsAltText
enum {

	/// Allow commas in item values, default is separator.
    kXMPUtil_AllowCommas      = 0x10000000UL

};

/// Option bit flags for \c TXMPUtils::RemoveProperties() and \c TXMPUtils::AppendProperties().
enum {

	 /// Do all properties, default is just external properties.
    kXMPUtil_DoAllProperties   = 0x0001UL,

	/// Replace existing values, default is to leave them.
    kXMPUtil_ReplaceOldValues  = 0x0002UL,

	/// Delete properties if the new value is empty.
    kXMPUtil_DeleteEmptyValues = 0x0004UL,

	/// Include aliases, default is just actual properties.
    kXMPUtil_IncludeAliases    = 0x0800UL

};

// =================================================================================================
// Types and Constants for XMPFiles
// ================================

/// File format constants for use with XMPFiles.
enum {
    
    // ! Hex used to avoid gcc warnings. Leave the constants so the text reads big endian. There
    // ! seems to be no decent way on UNIX to determine the target endianness at compile time.
    // ! Forcing it on the client isn't acceptable.

	// --------------------
    // Public file formats.

	/// Public file format constant: 'PDF '
    kXMP_PDFFile             = 0x50444620UL,
	/// Public file format constant: 'PS  ', general PostScript following DSC conventions
    kXMP_PostScriptFile      = 0x50532020UL,
	/// Public file format constant: 'EPS ', encapsulated PostScript
    kXMP_EPSFile             = 0x45505320UL,

	/// Public file format constant: 'JPEG'
    kXMP_JPEGFile            = 0x4A504547UL,
	/// Public file format constant: 'JPX ', JPEG 2000, ISO 15444-1
    kXMP_JPEG2KFile          = 0x4A505820UL,
	/// Public file format constant: 'TIFF'
    kXMP_TIFFFile            = 0x54494646UL,
	/// Public file format constant: 'GIF '
    kXMP_GIFFile             = 0x47494620UL,
	/// Public file format constant: 'PNG '
    kXMP_PNGFile             = 0x504E4720UL,

	/// Public file format constant: 'SWF '
    kXMP_SWFFile             = 0x53574620UL,
	/// Public file format constant: 'FLA '
    kXMP_FLAFile             = 0x464C4120UL,
	/// Public file format constant: 'FLV '
    kXMP_FLVFile             = 0x464C5620UL,

	/// Public file format constant: 'MOV ', Quicktime
    kXMP_MOVFile             = 0x4D4F5620UL,
	/// Public file format constant: 'AVI '
    kXMP_AVIFile             = 0x41564920UL,
	/// Public file format constant: 'CIN ', Cineon
    kXMP_CINFile             = 0x43494E20UL,
 	/// Public file format constant: 'WAV '
    kXMP_WAVFile             = 0x57415620UL,
	/// Public file format constant: 'MP3 '
    kXMP_MP3File             = 0x4D503320UL,
	/// Public file format constant: 'SES ', Audition session
    kXMP_SESFile             = 0x53455320UL,
	/// Public file format constant: 'CEL ', Audition loop
    kXMP_CELFile             = 0x43454C20UL,
	/// Public file format constant: 'MPEG'
    kXMP_MPEGFile            = 0x4D504547UL,
	/// Public file format constant: 'MP2 '
    kXMP_MPEG2File           = 0x4D503220UL,
	/// Public file format constant: 'MP4 ', ISO 14494-12 and -14
    kXMP_MPEG4File           = 0x4D503420UL,
	/// Public file format constant: 'WMAV', Windows Media Audio and Video
    kXMP_WMAVFile            = 0x574D4156UL,
	/// Public file format constant:  'AIFF'
    kXMP_AIFFFile            = 0x41494646UL,
	/// Public file format constant:  'P2  ', a collection not really a single file
    kXMP_P2File              = 0x50322020UL,
	/// Public file format constant:  'XDCF', a collection not really a single file
    kXMP_XDCAM_FAMFile       = 0x58444346UL,
	/// Public file format constant:  'XDCS', a collection not really a single file
    kXMP_XDCAM_SAMFile       = 0x58444353UL,
	/// Public file format constant:  'XDCX', a collection not really a single file
    kXMP_XDCAM_EXFile        = 0x58444358UL,
	/// Public file format constant:  'AVHD', a collection not really a single file
    kXMP_AVCHDFile           = 0x41564844UL,
	/// Public file format constant:  'SHDV', a collection not really a single file
    kXMP_SonyHDVFile         = 0x53484456UL,

	/// Public file format constant: 'HTML'
    kXMP_HTMLFile            = 0x48544D4CUL,
	/// Public file format constant: 'XML '
    kXMP_XMLFile             = 0x584D4C20UL,
	/// Public file format constant:  'text'
    kXMP_TextFile            = 0x74657874UL,

	// -------------------------------
    // Adobe application file formats.

	/// Adobe application file format constant: 'PSD '
    kXMP_PhotoshopFile       = 0x50534420UL,
	/// Adobe application file format constant: 'AI  '
    kXMP_IllustratorFile     = 0x41492020UL,
	/// Adobe application file format constant: 'INDD'
    kXMP_InDesignFile        = 0x494E4444UL,
	/// Adobe application file format constant: 'AEP '
    kXMP_AEProjectFile       = 0x41455020UL,
	/// Adobe application file format constant: 'AET ', After Effects Project Template
    kXMP_AEProjTemplateFile  = 0x41455420UL,
	/// Adobe application file format constant: 'FFX '
    kXMP_AEFilterPresetFile  = 0x46465820UL,
	/// Adobe application file format constant: 'NCOR'
    kXMP_EncoreProjectFile   = 0x4E434F52UL,
	/// Adobe application file format constant: 'PRPJ'
    kXMP_PremiereProjectFile = 0x5052504AUL,
	/// Adobe application file format constant: 'PRTL'
    kXMP_PremiereTitleFile   = 0x5052544CUL,
	/// Adobe application file format constant: 'UCF ', Universal Container Format
	kXMP_UCFFile             = 0x55434620UL,

	// -------
    // Others.

	/// Unknown file format constant: '    '
    kXMP_UnknownFile         = 0x20202020UL

};

/// Type for file format identification constants. See \c #kXMP_PDFFile and following.
typedef XMP_Uns32 XMP_FileFormat;

// -------------------------------------------------------------------------------------------------

/// Byte-order masks, do not use directly
enum {
    kXMP_CharLittleEndianMask = 1,
    kXMP_Char16BitMask        = 2,
    kXMP_Char32BitMask        = 4
};

/// Constants to allow easy testing for 16/32 bit and big/little endian.
enum {
	/// 8-bit
    kXMP_Char8Bit        = 0,
	/// 16-bit big-endian
    kXMP_Char16BitBig    = kXMP_Char16BitMask,
	/// 16-bit little-endian
    kXMP_Char16BitLittle = kXMP_Char16BitMask | kXMP_CharLittleEndianMask,
	/// 32-bit big-endian
    kXMP_Char32BitBig    = kXMP_Char32BitMask,
	/// 32-bit little-endian
    kXMP_Char32BitLittle = kXMP_Char32BitMask | kXMP_CharLittleEndianMask,
	/// Variable or not-yet-known cases
    kXMP_CharUnknown     = 1
};

/// \name Macros to test components of the character form mask
/// @{
///
/// \def XMP_CharFormIs16Bit
/// \brief Macro reports the encoding of a character.
/// \param f The character to check.
///
/// \def XMP_CharFormIs32Bit
/// \brief Macro reports the encoding of a character.
/// \param f The character to check.
///
/// \def XMP_CharFormIsBigEndian
/// \brief Macro reports the byte-order of a character.
/// \param f The character to check.
///
/// \def XMP_CharFormIsLittleEndian
/// \brief Macro reports the byte-order of a character.
/// \param f The character to check.
///
/// \def XMP_GetCharSize
/// \brief Macro reports the byte-size of a character.
/// \param f The character to check.
///
/// \def XMP_CharToSerializeForm
/// \brief Macro converts \c XMP_Uns8 to \c XMP_OptionBits.
/// \param cf The character to convert.
///
/// \def XMP_CharFromSerializeForm
/// \brief Macro converts \c XMP_OptionBits to \c XMP_Uns8.
/// \param sf The character to convert.
///
/// @}

#define XMP_CharFormIs16Bit(f)         ( ((int)(f) & kXMP_Char16BitMask) != 0 )
#define XMP_CharFormIs32Bit(f)         ( ((int)(f) & kXMP_Char32BitMask) != 0 )
#define XMP_CharFormIsBigEndian(f)     ( ((int)(f) & kXMP_CharLittleEndianMask) == 0 )
#define XMP_CharFormIsLittleEndian(f)  ( ((int)(f) & kXMP_CharLittleEndianMask) != 0 )
#define XMP_GetCharSize(f)             ( ((int)(f)&6) == 0 ? 1 : (int)(f)&6 )
#define XMP_CharToSerializeForm(cf)    ( (XMP_OptionBits)(cf) )
#define XMP_CharFromSerializeForm(sf)  ( (XMP_Uns8)(sf) )

/// \def kXMPFiles_UnknownOffset
/// \brief Constant for an unknown packet offset within a file.
#define kXMPFiles_UnknownOffset	((XMP_Int64)-1)

/// \def kXMPFiles_UnknownLength
/// \brief Constant for an unknown packet length within a file.
#define kXMPFiles_UnknownLength	((XMP_Int32)-1)

/// XMP packet description
struct XMP_PacketInfo {

	/// Packet offset in the file in bytes, -1 if unknown.
    XMP_Int64 offset;
	/// Packet length in the file in bytes, -1 if unknown.
    XMP_Int32 length;
	/// Packet padding size in bytes, zero if unknown.
    XMP_Int32 padSize;   // Zero if unknown.

	/// Character format using the values \c kXMP_Char8Bit, \c kXMP_Char16BitBig, etc.
    XMP_Uns8  charForm;
	/// True if there is a packet wrapper and the trailer says writeable by dumb packet scanners.
    XMP_Bool  writeable;
    /// True if there is a packet wrapper, the "<?xpacket...>" XML processing instructions.
    XMP_Bool  hasWrapper;

	/// Padding to make the struct's size be a multiple 4.
    XMP_Uns8  pad;

	/// Default constructor.
	XMP_PacketInfo() : offset(kXMPFiles_UnknownOffset), length(kXMPFiles_UnknownLength),
					   padSize(0), charForm(0), writeable(0), hasWrapper(0), pad(0) {};

};

/// Version of the XMP_PacketInfo type
enum {
	/// Version of the XMP_PacketInfo type
	kXMP_PacketInfoVersion = 3
};

// -------------------------------------------------------------------------------------------------

/// Values for \c XMP_ThumbnailInfo::tnailFormat.
enum {
	/// The thumbnail data has an unknown format.
    kXMP_UnknownTNail = 0,
	/// The thumbnail data is a JPEG stream, presumably compressed.
    kXMP_JPEGTNail    = 1,
	/// The thumbnail data is a TIFF stream, presumably uncompressed.
    kXMP_TIFFTNail    = 2,
	/// The thumbnail data is in the format of Photoshop Image Resource 1036.
    kXMP_PShopTNail   = 3
};

/// Thumbnail descriptor
struct XMP_ThumbnailInfo {

	/// The format of the containing file.
    XMP_FileFormat   fileFormat;
	/// Full image size in pixels.
    XMP_Uns32        fullWidth, fullHeight;
	/// Thumbnail image size in pixels.
    XMP_Uns32        tnailWidth, tnailHeight;
	/// Orientation of full image and thumbnail, as defined by Exif for tag 274.

    XMP_Uns16        fullOrientation, tnailOrientation;
	/// Raw image data from the host file, valid for life of the owning \c XMPFiles object. Do not modify!
    const XMP_Uns8 * tnailImage;
	/// The size in bytes of the thumbnail image data.
    XMP_Uns32        tnailSize;
	/// The format of the thumbnail image data.
    XMP_Uns8         tnailFormat;

	/// Padding to make the struct's size be a multiple 4.
    XMP_Uns8         pad1, pad2, pad3;

	/// Default constructor.
	XMP_ThumbnailInfo() : fileFormat(kXMP_UnknownFile), fullWidth(0), fullHeight(0),
						  tnailWidth(0), tnailHeight(0), fullOrientation(0), tnailOrientation(0),
						  tnailImage(0), tnailSize(0), tnailFormat(kXMP_UnknownTNail) {};

};

/// Version of the XMP_ThumbnailInfo type
enum {
	/// Version of the XMP_ThumbnailInfo type
	kXMP_ThumbnailInfoVersion = 1
};

// -------------------------------------------------------------------------------------------------

/// Option bit flags for \c TXMPFiles::Initialize().
enum {
	/// Do not initialize QuickTime, the client will.
    kXMPFiles_NoQuickTimeInit = 0x0001
};

/// Option bit flags for \c TXMPFiles::GetFormatInfo().
enum {

	/// Can inject first-time XMP into an existing file.
    kXMPFiles_CanInjectXMP        = 0x00000001,

	/// Can expand XMP or other metadata in an existing file.
    kXMPFiles_CanExpand           = 0x00000002,

	/// Can copy one file to another, writing new metadata.
    kXMPFiles_CanRewrite          = 0x00000004,

	 /// Can expand, but prefers in-place update.
    kXMPFiles_PrefersInPlace      = 0x00000008,

	/// Supports reconciliation between XMP and other forms.
    kXMPFiles_CanReconcile        = 0x00000010,

	 /// Allows access to just the XMP, ignoring other forms.
    kXMPFiles_AllowsOnlyXMP       = 0x00000020,

	/// File handler returns raw XMP packet information.
    kXMPFiles_ReturnsRawPacket    = 0x00000040,

	 /// File handler returns native thumbnail.
    kXMPFiles_ReturnsTNail        = 0x00000080,

	/// The file handler does the file open and close.
    kXMPFiles_HandlerOwnsFile     = 0x00000100,

	/// The file handler allows crash-safe file updates.
    kXMPFiles_AllowsSafeUpdate    = 0x00000200,

	/// The file format needs the XMP packet to be read-only.
    kXMPFiles_NeedsReadOnlyPacket = 0x00000400,

	/// The file handler uses a "sidecar" file for the XMP.
    kXMPFiles_UsesSidecarXMP      = 0x00000800,

	/// The format is folder oriented, for example the P2 video format.
    kXMPFiles_FolderBasedFormat   = 0x00001000

};

/// Option bit flags for \c TXMPFiles::OpenFile().
enum {

	/// Open for read-only access.
    kXMPFiles_OpenForRead           = 0x00000001,

	/// Open for reading and writing.
    kXMPFiles_OpenForUpdate         = 0x00000002,

	/// Only the XMP is wanted, allows space/time optimizations.
    kXMPFiles_OpenOnlyXMP           = 0x00000004,

	/// Cache thumbnail if possible, \c TXMPFiles::GetThumbnail() will be called.
    kXMPFiles_OpenCacheTNail        = 0x00000008,

	/// Be strict about locating XMP and reconciling with other forms.
    kXMPFiles_OpenStrictly          = 0x00000010,

	/// Require the use of a smart handler.
    kXMPFiles_OpenUseSmartHandler   = 0x00000020,

	/// Force packet scanning, do not use a smart handler.
    kXMPFiles_OpenUsePacketScanning = 0x00000040,

	/// Only packet scan files "known" to need scanning.
    kXMPFiles_OpenLimitedScanning   = 0x00000080,
    
    /// Attempt to repair a file opened for update, default is to not open (throw an exception).
    kXMPFiles_OpenRepairFile        = 0x00000100,

	 /// Set if calling from background thread.
    kXMPFiles_OpenInBackground      = 0x10000000

};

// A note about kXMPFiles_OpenInBackground. The XMPFiles handler for .mov files currently uses
// QuickTime. On Macintosh, calls to Enter/ExitMovies versus Enter/ExitMoviesOnThread must be made.
// This option is used to signal background use so that the .mov handler can behave appropriately.

/// Option bit flags for \c TXMPFiles::CloseFile().
enum {
	/// Write into a temporary file and swap for crash safety.
    kXMPFiles_UpdateSafely = 0x0001
};

// =================================================================================================
// Exception codes
// ===============

/// \name Errors Exception handling
/// @{
///
/// XMP Tookit errors result in throwing an \c XMP_Error exception. Any exception thrown within the
/// XMP Toolkit is caught in the toolkit and rethrown as an \c XMP_Error.
///
/// The \c XMP_Error class contains a numeric code and an English explanation. New numeric codes may
/// be added at any time. There are typically many possible explanations for each numeric code. The
/// explanations try to be precise about the specific circumstances causing the error.
///
/// \note The explanation string is for debugging use only. It must not be shown to users in a
/// final product. It is written for developers not users, and never localized.
///

/// XMP Toolkit error, associates an error code with a descriptive error string.
class XMP_Error {
public:

	/// @brief Constructor for an XMP_Error.
	///
	/// @param _id The numeric code.
	///
	/// @param _errMsg The descriptive string, for debugging use only. It must not be shown to users
	/// in a final product. It is written for developers, not users, and never localized.
	XMP_Error ( XMP_Int32 _id, XMP_StringPtr _errMsg ) : id(_id), errMsg(_errMsg) {};

	/// Retrieves the numeric code from an XMP_Error.
	inline XMP_Int32     GetID() const     { return id; };

	/// Retrieves the descriptive string from an XMP_Error.
	inline XMP_StringPtr GetErrMsg() const { return errMsg; };

private:
	/// Exception code. See constants \c #kXMPErr_Unknown and following.
	XMP_Int32     id;
	/// Descriptive string, for debugging use only. It must not be shown to users in a final
	/// product. It is written for developers, not users, and never localized.
	XMP_StringPtr errMsg;
};

/// Exception code constants
enum {

	// --------------------
    // Generic error codes.
   
	/// Generic unknown error
    kXMPErr_Unknown          =   0,
	/// Generic undefined error
    kXMPErr_TBD              =   1,
	/// Generic unavailable error
    kXMPErr_Unavailable      =   2,
	/// Generic bad object error
    kXMPErr_BadObject        =   3,
	/// Generic bad parameter error
    kXMPErr_BadParam         =   4,
	/// Generic bad value error
    kXMPErr_BadValue         =   5,
	/// Generic assertion failure
    kXMPErr_AssertFailure    =   6,
	/// Generic enforcement failure
    kXMPErr_EnforceFailure   =   7,
	/// Generic unimplemented error
    kXMPErr_Unimplemented    =   8,
	/// Generic internal failure
    kXMPErr_InternalFailure  =   9,
	/// Generic deprecated error
    kXMPErr_Deprecated       =  10,
	/// Generic external failure
    kXMPErr_ExternalFailure  =  11,
	/// Generic user abort error
    kXMPErr_UserAbort        =  12,
	/// Generic standard exception
    kXMPErr_StdException     =  13,
	/// Generic unknown exception
    kXMPErr_UnknownException =  14,
	/// Generic out-of-memory error
    kXMPErr_NoMemory         =  15,

	// ------------------------------------
    // More specific parameter error codes.

	/// Bad schema parameter
    kXMPErr_BadSchema        = 101,
	/// Bad XPath parameter
    kXMPErr_BadXPath         = 102,
	/// Bad options parameter
    kXMPErr_BadOptions       = 103,
	/// Bad index parameter
    kXMPErr_BadIndex         = 104,
	/// Bad iteration position
    kXMPErr_BadIterPosition  = 105,
	/// XML parsing error
    kXMPErr_BadParse         = 106,
	/// Serialization error
    kXMPErr_BadSerialize     = 107,
	/// File format error
    kXMPErr_BadFileFormat    = 108,
	/// No file handler found for format
    kXMPErr_NoFileHandler    = 109,
	/// Data too large for JPEG file format
    kXMPErr_TooLargeForJPEG  = 110,

	// -----------------------------------------------
    // File format and internal structure error codes.

	/// XML format error
    kXMPErr_BadXML           = 201,
	/// RDF format error
    kXMPErr_BadRDF           = 202,
	/// XMP format error
    kXMPErr_BadXMP           = 203,
	/// Empty iterator
    kXMPErr_EmptyIterator    = 204,
	/// Unicode error
    kXMPErr_BadUnicode       = 205,
	/// TIFF format error
    kXMPErr_BadTIFF          = 206,
	/// JPEG format error
    kXMPErr_BadJPEG          = 207,
	/// PSD format error
    kXMPErr_BadPSD           = 208,
	/// PSIR format error
    kXMPErr_BadPSIR          = 209,
	/// IPTC format error
    kXMPErr_BadIPTC          = 210,
	/// MPEG format error
    kXMPErr_BadMPEG          = 211

};

/// @}

// =================================================================================================
// Client callbacks
// ================

// -------------------------------------------------------------------------------------------------
/// \name Special purpose callback functions
/// @{

/// A signed 32-bit integer used as a status result for the output callback routine,
/// \c XMP_TextOutputProc. Zero means no error, all other values except -1 are private to the callback.
/// The callback is wrapped to prevent exceptions being thrown across DLL boundaries. Any exceptions
/// thrown out of the callback cause a return status of -1.

typedef XMP_Int32 XMP_Status;

// -------------------------------------------------------------------------------------------------
/// The signature of a client-defined callback for text output from XMP Toolkit debugging
/// operations. The callback is invoked one or more times for each line of output. The end of a line
/// is signaled by a '\\n' character at the end of the buffer. Formatting newlines are never present
/// in the middle of a buffer, but values of properties might contain any UTF-8 characters.
///
/// @param refCon A pointer to client-defined data passed to the TextOutputProc.
///
/// @param buffer  A string containing one line of output.
///
/// @param bufferSize The number of characters in the output buffer.
///
/// @return A success/fail status value. Any failure result aborts the output.
///
/// @see \c TXMPMeta::DumpObject()

typedef XMP_Status (* XMP_TextOutputProc) ( void *        refCon,
                                            XMP_StringPtr buffer,
                                            XMP_StringLen bufferSize );

// -------------------------------------------------------------------------------------------------
/// The signature of a client-defined callback to check for a user request to abort a time-consuming
/// operation within XMPFiles.
///
/// @param arg A pointer to caller-defined data passed from the registration call.
///
/// @return True to abort the current operation, which results in an exception being thrown.
///
/// @see \c TXMPFiles::SetAbortProc()

typedef bool (* XMP_AbortProc) ( void * arg );	// Used by .

/// @}

// =================================================================================================
// Stuff with no better place to be
// ================================

/// XMP Toolkit version information
typedef struct XMP_VersionInfo {
	/// The primary release number, the "1" in version "1.2.3".
    XMP_Uns8      major;
	/// The secondary release number, the "2" in version "1.2.3".
    XMP_Uns8      minor;
	/// The tertiary release number, the "3" in version "1.2.3".
    XMP_Uns8      micro;
	 /// A 0/1 boolean value, true if this is a debug build.
    XMP_Bool      isDebug;
	 /// A rolling build number, monotonically increasing in a release.
    XMP_Uns32     build;
	 /// Individual feature implementation flags.
    XMP_Uns32     flags;
	 /// A comprehensive version information string.
    XMP_StringPtr message;
} XMP_VersionInfo;

// =================================================================================================

#if __cplusplus
} // extern "C"
#endif

#endif  // __XMP_Const_h__
