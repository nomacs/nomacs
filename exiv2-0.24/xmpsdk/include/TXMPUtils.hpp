#ifndef __TXMPUtils_hpp__
#define __TXMPUtils_hpp__ 1

#if ( ! __XMP_hpp__ )
    #error "Do not directly include, use XMPSDK.hpp"
#endif

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

// =================================================================================================
/// \file TXMPUtils.hpp
/// \brief API for access to the XMP Toolkit utility services.
///
/// \c TXMPUtils is the template class providing utility services for the XMP Toolkit. It must be
/// instantiated with a string class such as \c std::string. See the instructions in XMPSDK.hpp, and
/// the Overview for a discussion of the overall architecture of the XMP API.
// =================================================================================================

// =================================================================================================
/// \class TXMPUtils TXMPUtils.hpp
/// @brief API for access to the XMP Toolkit utility services.
///
/// \c TXMPUtils is a template class which must be instantiated with a string class such as
/// \c std::string. See the instructions in XMPSDK.hpp, and the Overview for a discussion of the overall
/// architecture of the XMP API.
///
/// This class defines helper functions that support the basic metadata manipulation provided by
/// \c TXMPMeta. All of the functions are static; that is, you call them directly from the concrete
/// class (\c SXMPUtils), which is never itself instantiated.
///
/// General categories of utilities include:
///
///   \li Composing complex path expressions, which you can then pass to the property access
///   functions in \c TXMPMeta
///   \li Converting between binary and string forms of property values
///   \li Manipulating date/time values
///   \li Encoding and decoding base-64 strings
///   \li JPEG file handling
///   \li Editing aids for creating a user interface for the XMP Toolkit
// =================================================================================================

template <class tStringObj> class TXMPUtils {

public:

    // =============================================================================================
    // No constructors or destructor declared or needed
    // ================================================

    //  ============================================================================================
    /// \name Path composition
    /// @{
    ///
    /// These functions provide support for composing path expressions to deeply nested properties.
    /// The functions in \c TXMPMeta such as \c TXMPMeta::GetProperty(),
    /// \c TXMPMeta::GetArrayItem(), and \c TXMPMeta::GetStructField() provide easy access to top level
    /// simple properties, items in top level arrays, and fields of top level structs.	They are
    /// not as convenient for more complex things, such as fields several levels deep in a complex
    /// struct, or fields within an array of structs, or items of an array that is a field of a
    /// struct. You can use these utility functions to compose these paths, which you can then pass
    /// to the property access functions. You can also compose	paths to top-level array items or
	/// struct fields so that you can use the binary accessors such as
	/// \c TXMPMeta::GetProperty_Int().
    ///
    /// You can use these functions is to compose a complete path expression, or all but the last
    /// component. For example, suppose you have a property that is an array of integers within a
    /// struct. You can access one of the array items like this:
    ///
    /// <pre>
    ///   SXMPUtils::ComposeStructFieldPath ( schemaNS, "Struct", fieldNS, "Array", &path );
    ///   SXMPUtils::ComposeArrayItemPath ( schemaNS, path, index, &path );
    ///   exists = xmpObj.GetProperty_Int ( schemaNS, path, &value, &options );
    /// </pre>
    ///
    /// You could also use this code if you want the string form of the integer:
    ///
    /// <pre>
    ///   SXMPUtils::ComposeStructFieldPath ( schemaNS, "Struct", fieldNS, "Array", &path );
    ///   xmpObj.GetArrayItem ( schemaNS, path, index, &value, &options );
    /// </pre>
    ///
    /// \note It might look confusing that the \c schemaNS is passed in all of the calls above. This
    /// is because the XMP Toolkit keeps the top-level "schema" namespace separate from the rest of
    /// the path expression.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ComposeArrayItemPath() composes the path expression for an item in an array.
    ///
    /// The returned string is in the form <tt>ns:arrayName[i]</tt>, where "ns" is the prefix for
    /// the specified namespace, and "i" is the decimal representation of specified item index.
    /// If the last item was specified, the path is <tt>ns:arrayName[last()]</tt>.
    ///
    /// @param schemaNS The namespace URI for the array; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param itemIndex The 1-based index of the desired item. Use the macro
    /// \c #kXMP_ArrayLastItem to specify the last existing array item.
    ///
    /// @param fullPath [out] A string in which to return the composed path.

    static void ComposeArrayItemPath ( XMP_StringPtr schemaNS,
									   XMP_StringPtr arrayName,
									   XMP_Index     itemIndex,
									   tStringObj *  fullPath );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ComposeStructFieldPath() composes the path expression for a field in a struct.
    ///
    /// The returned string is in the form <tt>ns:structName/fNS:fieldName</tt>, where "ns" is the
    /// prefix for the schema namespace, and "fNS" is the prefix for field namespace.
    ///
    /// @param schemaNS The namespace URI for the struct; see \c GetProperty().
    ///
    /// @param structName The name of the struct. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param fieldNS The namespace URI for the field. Same URI and prefix usage as the
    /// \c schemaNS and \c structName parameters.
    ///
    /// @param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Same URI and prefix usage as the \c schemaNS and \c structName parameters.
    ///
    /// @param fullPath [out] A string in which to return the composed path.

    static void ComposeStructFieldPath ( XMP_StringPtr schemaNS,
										 XMP_StringPtr structName,
										 XMP_StringPtr fieldNS,
										 XMP_StringPtr fieldName,
										 tStringObj *  fullPath );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ComposeQualifierPath() composes the path expression for a qualifier.
    ///
    /// The returned string is in the form <tt>ns:propName/?qNS:qualName</tt>, where "ns" is the
    /// prefix for the schema namespace, and "qNS" is the prefix for the qualifier namespace.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property to which the qualifier is attached. Can be a
    /// general path expression, must not be null or the empty string; see \c GetProperty() for
    /// namespace prefix usage.
    ///
    /// @param qualNS The namespace URI for the qualifier. Same URI and prefix usage as the
    /// \c schemaNS and \c propName parameters.
    ///
    /// @param qualName The name of the qualifier. Must be a single XML name, must not be null or the
    /// empty string. Same URI and prefix usage as the \c schemaNS and \c propName parameters.
    ///
    /// @param fullPath [out] A string in which to return the composed path.

    static void ComposeQualifierPath ( XMP_StringPtr schemaNS,
									   XMP_StringPtr propName,
									   XMP_StringPtr qualNS,
									   XMP_StringPtr qualName,
									   tStringObj *  fullPath );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ComposeLangSelector() composes the path expression to select an alternate item by language.
    ///
    /// Path syntax allows two forms of "content addressing" to select an item in an array of
    /// alternatives. The form used in this function lets you select an item in an alt-text array
    /// based on the value of its \c xml:lang qualifier. The other form of content addressing is
    /// shown in \c ComposeFieldSelector().
    ///
    /// The returned string is in the form <tt>ns:arrayName[\@xml:lang='langName']</tt>, where
    /// "ns" is the prefix for the schema namespace
    ///
    /// This function provides a path expression that is explicitly and only for a specific
    /// language. In most cases, \c TXMPMeta::SetLocalizedText() and \c TXMPMeta::GetLocalizedText()
    /// are preferred, because they provide extra logic to choose the appropriate language and
    /// maintain consistency with the 'x-default' value.
    ///
    /// @param schemaNS The namespace URI for the array; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param langName The RFC 3066 code for the desired language, as a null-terminated UTF-8 string.
    ///
    /// @param fullPath [out] A string in which to return the composed path.

    static void ComposeLangSelector ( XMP_StringPtr schemaNS,
									  XMP_StringPtr arrayName,
									  XMP_StringPtr langName,
									  tStringObj *  fullPath );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ComposeLangSelector() composes a path expression to select an alternate item by language.
    ///
    /// Path syntax allows two forms of "content addressing" to select an item in an array of
    /// alternatives. The form used in this function lets you select an item in an alt-text array
    /// based on the value of its \c xml:lang qualifier. The other form of content addressing is
    /// shown in \c ComposeFieldSelector().
    ///
    /// The returned string is in the form <tt>ns:arrayName[\@xml:lang='langName']</tt>, where
    /// "ns" is the prefix for the schema namespace
    ///
    /// This function provides a path expression that is explicitly and only for a specific
    /// language. In most cases, \c TXMPMeta::SetLocalizedText() and \c TXMPMeta::GetLocalizedText()
    /// are preferred, because they provide extra logic to choose the appropriate language and
    /// maintain consistency with the 'x-default' value.
    ///
    /// @param schemaNS The namespace URI for the array; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param langName The RFC 3066 code for the desired language, as a string object.
    ///
    /// @param fullPath [out] A string in which to return the composed path.

    static void ComposeLangSelector ( XMP_StringPtr      schemaNS,
									  XMP_StringPtr      arrayName,
									  const tStringObj & langName,
									  tStringObj *       fullPath );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ComposeFieldSelector() composes a path expression to select an alternate item by a field's value.
    ///
    /// Path syntax allows two forms of "content addressing" to select an item in an array of
    /// alternatives. The form used in this function lets you select an item in an array of structs
    /// based on the value of one of the fields in the structs. The other form of content addressing
    /// is shown in \c ComposeLangSelector().
    ///
    /// For example, consider a simple struct that has two fields, the name of a city and the URI of
    /// an FTP site in that city. Use this to create an array of download alternatives. You can show
    /// the user a popup built from the values of the city fields, then get the corresponding URI as
    /// follows:
    /// <pre>
    ///   ComposeFieldSelector ( schemaNS, "Downloads", fieldNS, "City", chosenCity, &path );
    ///   exists = GetStructField ( schemaNS, path, fieldNS, "URI", &uri );
    /// </pre>
    ///
    /// The returned string is in the form <tt>ns:arrayName[fNS:fieldName='fieldValue']</tt>, where
    /// "ns" is the prefix for the schema namespace and "fNS" is the prefix for the field namespace.
    ///
    /// @param schemaNS The namespace URI for the array; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param fieldNS The namespace URI for the field used as the selector. Same URI and prefix
    /// usage as the \c schemaNS and \c arrayName parameters.
    ///
    /// @param fieldName The name of the field used as the selector. Must be a single XML name, must
    /// not be null or the empty string. It must be the name of a field that is itself simple.
    ///
    /// @param fieldValue The desired value of the field, specified as a null-terminated UTF-8 string.
    ///
    /// @param fullPath [out] A string in which to return the composed path.

    static void ComposeFieldSelector ( XMP_StringPtr schemaNS,
									   XMP_StringPtr arrayName,
									   XMP_StringPtr fieldNS,
									   XMP_StringPtr fieldName,
									   XMP_StringPtr fieldValue,
									   tStringObj *  fullPath );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ComposeFieldSelector() composes a path expression to select an alternate item by a field's value.
    ///
    /// Path syntax allows two forms of "content addressing" to select an item in an array of
    /// alternatives. The form used in this function lets you select an item in an array of structs
    /// based on the value of one of the fields in the structs. The other form of content addressing
    /// is shown in \c ComposeLangSelector().
    ///
    /// For example, consider a simple struct that has two fields, the name of a city and the URI of
    /// an FTP site in that city. Use this to create an array of download alternatives. You can show
    /// the user a popup built from the values of the city fields, then get the corresponding URI as
    /// follows:
    /// <pre>
    ///   ComposeFieldSelector ( schemaNS, "Downloads", fieldNS, "City", chosenCity, &path );
    ///   exists = GetStructField ( schemaNS, path, fieldNS, "URI", &uri );
    /// </pre>
    ///
    /// The returned string is in the form <tt>ns:arrayName[fNS:fieldName='fieldValue']</tt>, where
    /// "ns" is the prefix for the schema namespace and "fNS" is the prefix for the field namespace.
    ///
    /// @param schemaNS The namespace URI for the array; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param fieldNS The namespace URI for the field used as the selector. Same URI and prefix
    /// usage as the \c schemaNS and \c arrayName parameters.
    ///
    /// @param fieldName The name of the field used as the selector. Must be a single XML name, must
    /// not be null or the empty string. It must be the name of a field that is itself simple.
    ///
    /// @param fieldValue The desired value of the field, specified as a string object.
    ///
    /// @param fullPath [out] A string in which to return the composed path.

    static void ComposeFieldSelector ( XMP_StringPtr      schemaNS,
									   XMP_StringPtr      arrayName,
									   XMP_StringPtr      fieldNS,
									   XMP_StringPtr      fieldName,
									   const tStringObj & fieldValue,
									   tStringObj *       fullPath );

    /// @}

    // =============================================================================================
    /// \name Conversion between binary types and strings
    /// @{
    ///
	///	The main accessors in \c TXMPMeta set and retrieve property values as strings. additional
	///	functions, such as \c TXMPMeta::SetPropertyInt(), set and retrieve property values as
	///	explicit binary data types. Use these functions to convert between binary and string
	///	values.
	///
	///	Strings can be specified as null-terminated UTF-8 (\c #XMP_StringPtr), or as string
	///	objects (\c tStringObj) of the type declared when instantiating the XMP classes; see
	///	\c XMPSDK.hpp. Alternate forms of each conversion function allow either type of string.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertFromBool() converts a Boolean value to a string.
    ///
    /// The string values of Booleans are returned by the macros \c #kXMP_TrueStr and
    /// \c #kXMP_FalseStr in \c XMP_Const.h.
    ///
    /// @param binValue The Boolean value to be converted.
    ///
    /// @param strValue [out] A buffer in which to return the string representation of the value.

    static void ConvertFromBool ( bool	       binValue,
								  tStringObj * strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertFromInt() converts a 32-bit integer value to a string.
    ///
    /// @param binValue The integer value to be converted.
    ///
    /// @param format Optional. A C \c sprintf format for the conversion. Default is "%d".
    ///
    /// @param strValue [out] A buffer in which to return the string representation of the value.

    static void ConvertFromInt ( long	       binValue,
								 XMP_StringPtr format,
								 tStringObj *  strValue );
    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertFromInt64() converts a 64-bit integer value to a string.
    ///
    /// @param binValue The integer value to be converted.
    ///
    /// @param format Optional. A C \c sprintf format for the conversion. Default is "%d".
    ///
    /// @param strValue [out] A buffer in which to return the string representation of the value.

    static void ConvertFromInt64 ( long long	 binValue,
								   XMP_StringPtr format,
								   tStringObj *  strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertFromFloat() converts a floating-point value to a string.
    ///
    /// @param binValue The floating-point value to be converted.
    ///
    /// @param format Optional. A C \c sprintf format for the conversion. Default is "%d".
    ///
    /// @param strValue [out] A buffer in which to return the string representation of the value.

    static void ConvertFromFloat ( double	     binValue,
								   XMP_StringPtr format,
								   tStringObj *	 strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertFromDate() converts a date/time value to a string.
    ///
    /// Formats a date according to the ISO 8601 profile in http://www.w3.org/TR/NOTE-datetime:
    /// <pre>
    ///   YYYY
    ///   YYYY-MM
    ///   YYYY-MM-DD
    ///   YYYY-MM-DDThh:mmTZD
    ///   YYYY-MM-DDThh:mm:ssTZD
    ///   YYYY-MM-DDThh:mm:ss.sTZD
    /// </pre>
    ///
    ///  \c YYYY = four-digit year, formatted as "%.4d" <br>
    ///  \c MM	 = two-digit month (01=January)	<br>
    ///  \c DD	 = two-digit day of month (01 through 31) <br>
    ///  \c hh	 = two digits of hour (00 through 23) <br>
    ///  \c mm	 = two digits of minute (00 through 59)	<br>
    ///  \c ss	 = two digits of second (00 through 59)	<br>
    ///  \c s	 = one or more digits representing a decimal fraction of a second <br>
    ///  \c TZD	 = time zone designator (Z or +hh:mm or -hh:mm)
    ///
    /// Time-only input is allowed where the year, month, and day are all zero. This is output as
    /// "0000-00-00...".
    ///
    /// @note ISO 8601 does not allow years less than 1000 or greater than 9999. This API allows
    /// any year, even negative ones.
    ///
    /// @param binValue The date/time value to be converted.
    ///
    /// @param strValue [out] A buffer in which to return the ISO 8601 string representation of the date/time.

    static void ConvertFromDate ( const XMP_DateTime & binValue,
								  tStringObj *	       strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToBool() converts a string to a Boolean value.
    ///
    /// The preferred strings are those returned by the macros \c #kXMP_TrueStr and \c #kXMP_FalseStr.
    /// If these do not match, the function does a case insensitive comparison, then simply 't' or 'f',
    /// and finally non-zero and zero integer representations.
    ///
    /// @param strValue The string representation of the value, specified as a null-terminated UTF-8 string.
    ///
    /// @return The appropriate C++ bool value for the string.

    static bool ConvertToBool ( XMP_StringPtr strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToBool() converts a string to a Boolean value.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object,
	/// rather than a <tt>const * char</tt>. It is otherwise identical; see details in the canonical form.
    ///
    /// @param strValue The string representation of the value, specified as a string object.
    ///
    /// @return The appropriate C++ bool value for the string.

    static bool ConvertToBool ( const tStringObj & strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToInt() converts a string to a 32-bit integer value.
    ///
    /// @param strValue The string representation of the value, specified as a null-terminated UTF-8 string.
    ///
    /// @return The 32-bit integer value.

    static long ConvertToInt ( XMP_StringPtr strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToInt() converts a string to a 32-bit integer value.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object,
	/// rather than a <tt>const * char</tt>. It is otherwise identical.
	///
    /// @param strValue The string representation of the value, specified as a string object.
    ///
    /// @return The 32-bit integer value.

    static long ConvertToInt ( const tStringObj & strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToInt64() converts a string to a 64-bit integer value.
    ///
    /// @param strValue The string representation of the value, specified as a null-terminated UTF-8 string.
    ///
    /// @return The 64-bit integer value.

    static long long ConvertToInt64 ( XMP_StringPtr strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToInt64() converts a string to a 64-bit integer value.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object,
	/// rather than a <tt>const * char</tt>. It is otherwise identical.
	///
    /// @param strValue The string representation of the value, specified as a string object.
    ///
    /// @return The 64-bit integer value.

    static long long ConvertToInt64 ( const tStringObj & strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToFloat() converts a string to a floating-point value.
    ///
    /// @param strValue The string representation of the value, specified as a null-terminated UTF-8 string.
    ///
    /// @return The floating-point value.

    static double ConvertToFloat ( XMP_StringPtr strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToFloat() converts a string to a floating-point value.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object,
	/// rather than a <tt>const * char</tt>. It is otherwise identical.
	///
    /// @param strValue The string representation of the value, specified as a string object.
    ///
    /// @return The floating-point value.

    static double ConvertToFloat ( const tStringObj & strValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToDate() converts a string to a date/time value.
    ///
    /// Parses a date according to the ISO 8601 profile in http://www.w3.org/TR/NOTE-datetime:
    /// <pre>
    ///   YYYY
    ///   YYYY-MM
    ///   YYYY-MM-DD
    ///   YYYY-MM-DDThh:mmTZD
    ///   YYYY-MM-DDThh:mm:ssTZD
    ///   YYYY-MM-DDThh:mm:ss.sTZD
    /// </pre>
    ///
    ///  \c YYYY = four-digit year, formatted as "%.4d" <br>
    ///  \c MM	 = two-digit month (01=January)	<br>
    ///  \c DD	 = two-digit day of month (01 through 31) <br>
    ///  \c hh	 = two digits of hour (00 through 23) <br>
    ///  \c mm	 = two digits of minute (00 through 59)	<br>
    ///  \c ss	 = two digits of second (00 through 59)	<br>
    ///  \c s	 = one or more digits representing a decimal fraction of a second <br>
    ///  \c TZD	 = time zone designator (Z or +hh:mm or -hh:mm)
    ///
    /// A missing date portion or missing TZD are tolerated. A missing date value can begin with
    /// "Thh:" or "hh:"; the year, month, and day are all set to zero in the \c #XMP_DateTime value.
    /// A missing TZD is assumed to be UTC.
    ///
    /// @note ISO 8601 does not allow years less than 1000 or greater than 9999. This API allows
    /// any year, even negative ones.
    ///
    /// @param strValue The ISO 8601 string representation of the date/time, specified as a
    /// null-terminated UTF-8 string.
    ///
    /// @param binValue [out] A buffer in which to return the binary date/time value.

    static void ConvertToDate ( XMP_StringPtr  strValue,
								XMP_DateTime * binValue );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToDate() converts a string to a date/time value.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object,
	/// rather than a <tt>const * char</tt>. It is otherwise identical.
	/// See details for the canonical form.
	///
    ///
    /// @param strValue The ISO 8601 string representation of the date/time, specified as a string
    /// object.
    ///
    /// @param binValue [out] A buffer in which to return the binary date/time value.

    static void ConvertToDate ( const tStringObj & strValue,
								XMP_DateTime *     binValue );

    /// @}

    // =============================================================================================
    /// \name Date-time manipulation
    /// @{
    ///
	///	In addition to the type-conversion functions that convert between strings and binary
	///	date-time values, these functions create, manipulate, and compare date-time values.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c CurrentDateTime() obtains the current date and time.
    ///
    /// Creates and returns a binary \c #XMP_DateTime value. The returned time is UTC, properly
    /// adjusted for the local time zone. The resolution of the time is not guaranteed to be finer
    /// than seconds.
    ///
    /// @param time	[out] A buffer in which to return the date/time value.

    static void CurrentDateTime ( XMP_DateTime * time );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetTimeZone() sets the time zone in a date/time value to the local time zone.
    ///
    /// Any existing time zone value is replaced. The other date/time fields are not adjusted in any way.
    ///
    /// @param time	A pointer to the date-time value, which is modified in place.

    static void SetTimeZone ( XMP_DateTime * time );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToUTCTime() ensures that a time is UTC.
    ///
    /// If the time zone is not UTC, the time is adjusted and the time zone set to be UTC. If the
    /// time zone is already UTC, the value is not modified.
    ///
    /// @param time	A pointer to the date-time value, which is modified in place.

    static void ConvertToUTCTime ( XMP_DateTime * time );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ConvertToLocalTime() ensures that a time is local.
    ///
    /// If the time zone is not the local zone, the time is adjusted and the time zone set to be local.
    /// If the time zone is already the local zone, the value is not modified.
    ///
    /// @param time	A pointer to the date-time value, which is modified in place.

    static void ConvertToLocalTime ( XMP_DateTime * time );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c CompareDateTime() compares the order of two date/time values.
    ///
    /// @param left The left-side date/time value.
    ///
    /// @param right The right-side date/time value.
    ///
    /// @return An integer indicating the order:
    ///   \li -1 if left is earlier than right
    ///   \li 0 if left matches right
    ///   \li +1 if left is later than right

    static int CompareDateTime ( const XMP_DateTime & left,
								 const XMP_DateTime & right );

    /// @}

    // =============================================================================================
    /// \name Base64 encoding and decoding
    /// @{
    ///
	/// These functions convert between raw data values and Base64-encoded strings.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c EncodeToBase64() converts a raw data value to a Base64-encoded string.
    ///
    /// @param rawStr An \c #XMP_StringPtr (char *) string containing the raw data to be converted.
    ///
    /// @param rawLen The number of characters of raw data to be converted.
    ///
    /// @param encodedStr [out] A string object in which to return the encoded string.

    static void EncodeToBase64 ( XMP_StringPtr rawStr,
								 XMP_StringLen rawLen,
								 tStringObj *  encodedStr );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c EncodeToBase64() converts a raw data value passed in a string object to a Base64-encoded string.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object as input.
	/// It is otherwise identical.
 	///
    /// @param rawStr A string object containing the raw data to be converted.
    ///
    /// @param encodedStr [out] A string object in which to return the encoded string.

    static void  EncodeToBase64 ( const tStringObj & rawStr,
								  tStringObj *       encodedStr );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c  DecodeFromBase64() Decodes a Base64-encoded string to raw data.
    ///
    /// @param encodedStr An \c #XMP_StringPtr (char *) string containing the encoded data to be converted.
    ///
    /// @param encodedLen The number of characters of raw data to be converted.
    ///
    /// @param rawStr [out] A string object in which to return the decoded data.

    static void DecodeFromBase64 ( XMP_StringPtr encodedStr,
								   XMP_StringLen encodedLen,
								   tStringObj *  rawStr );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c  DecodeFromBase64() Decodes a Base64-encoded string, passed as a string object, to raw data.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object as input.
	/// It is otherwise identical.
 	///
    /// @param encodedStr An string object containing the encoded data to be converted.
    ///
    /// @param rawStr [out] A string object in which to return the decoded data.

    static void DecodeFromBase64 ( const tStringObj & encodedStr,
								   tStringObj *       rawStr );

    /// @}

    // =============================================================================================
    // =============================================================================================
    /// \name JPEG file handling
    /// @{
    ///
    /// These functions support the partitioning of XMP in JPEG files into standard and extended
    /// portions in order to work around the 64KB size limit of JPEG marker segments.
    ///
	/// @note (Doc note) Add detail about how to write out and read back extended data

    // ---------------------------------------------------------------------------------------------
    /// @brief \c PackageForJPEG() creates XMP serializations appropriate for a JPEG file.
    ///
    /// The standard XMP in a JPEG file is limited to 64K bytes. This function serializes the XMP
    /// metadata in an XMP object into a string of RDF (see \c TXMPMeta::SerializeToBuffer()). If
    /// the data does not fit into the 64K byte limit, it creates a second packet string with the
    /// extended data.
    ///
    /// @param xmpObj The XMP object containing the metadata.
    ///
    /// @param standardXMP [out] A string object in which to return the full standard XMP packet.
    ///
    /// @param extendedXMP [out] A string object in which to return the serialized extended XMP,
    /// empty if not needed.
    ///
    /// @param extendedDigest [out] A string object in which to return an MD5 digest of the serialized
    /// extended XMP, empty if not needed.
    ///
    /// @see \c MergeFromJPEG()

    static void PackageForJPEG ( const TXMPMeta<tStringObj> & xmpObj,
								 tStringObj *                 standardXMP,
								 tStringObj *                 extendedXMP,
								 tStringObj *                 extendedDigest );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c MergeFromJPEG() merges standard and extended XMP retrieved from a JPEG file.
    ///
    /// When an extended partition stores properties that do not fit into the JPEG file limitation
    /// of 64K bytes, this function integrates those properties back into the same XMP object with
    /// those from the standard XMP packet.
    ///
    /// @param fullXMP [in, out] An XMP object which the caller has initialized from the standard
    /// XMP packet in a JPEG file. The extended XMP is added to this object.
    ///
    /// @param extendedXMP An XMP object which the caller has initialized from the extended XMP
    /// packet in a JPEG file.
    ///
    /// @see \c PackageForJPEG()

    static void MergeFromJPEG ( TXMPMeta<tStringObj> *       fullXMP,
								const TXMPMeta<tStringObj> & extendedXMP );

    /// @}

    // =============================================================================================
    /// \name Editing utilities
    /// @{
    ///
    /// These functions are useful in implementing a user interface for editing XMP. They
	/// convert sets of property values to and from displayable and manipulable strings, and perform
	/// operations on sets of metadata, such as those available from the File Info dialog box.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c CatenateArrayItems() creates a single edit string from a set of array item values.
    ///
    /// Collects the values of all items in an array into a single string, using a specified
    /// separation string. Each item in the specified array must be a simple string value.
    ///
    /// @param xmpObj The XMP object containing the array to be catenated.
    ///
    /// @param schemaNS The schema namespace URI for the array. Must not be null or the empty string.
    ///
    /// @param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string.
    ///
    /// @param separator The string with which to separate the items in the catenated string.
    /// Defaults to "; ", ASCII semicolon and space (U+003B, U+0020).
    ///
    /// @param quotes The character or characters to use as quotes around array items that contain a
    /// separator. Defaults to the double-quote character ("), ASCII quote (U+0022).
    ///
    /// @param options Option flags to control the catenation. <<what options?>>
    ///
    /// @param catedStr [out] A string object in which to return the catenated array items.
    ///
    /// @see \c SeparateArrayItems()

    static void CatenateArrayItems ( const TXMPMeta<tStringObj> & xmpObj,
									 XMP_StringPtr                schemaNS,
									 XMP_StringPtr                arrayName,
									 XMP_StringPtr                separator,
									 XMP_StringPtr                quotes,
									 XMP_OptionBits               options,
									 tStringObj *                 catedStr );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SeparateArrayItems() updates an array from a concatenated edit string of values.
    ///
    /// This reverses the action of \c CatenateArrayItems(), separating out individual array items
    /// from the edit string and updating the array with the new values. Each item in the array must
    /// be a simple string value.
    ///
    /// @param xmpObj The XMP object containing the array to be updated.
    ///
    /// @param schemaNS The schema namespace URI for the array. Must not be null or the empty string.
    ///
    /// @param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string.
    ///
    /// @param options Option flags to control the separation. <<what options?>>
    ///
    /// @param catedStr The concatenated array items, as created by \c CatenateArrayItems(),
    /// specified as a null-terminated UTF-8 string.

    static void SeparateArrayItems ( TXMPMeta<tStringObj> * xmpObj,
									 XMP_StringPtr          schemaNS,
									 XMP_StringPtr          arrayName,
									 XMP_OptionBits         options,
									 XMP_StringPtr          catedStr );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SeparateArrayItems() updates an array from a concatenated edit string of values.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object in which
    ///	to return the concatenated string. It is otherwise identical; see details for the canonical form.
 	///

    static void SeparateArrayItems ( TXMPMeta<tStringObj> * xmpObj,
									 XMP_StringPtr          schemaNS,
									 XMP_StringPtr          arrayName,
									 XMP_OptionBits         options,
									 const tStringObj &     catedStr );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c  RemoveProperties() removes multiple properties from an XMP object.
    ///
    /// The operation depends on how the namespace and property are specified:
    ///
    ///   \li Non-empty \c schemaNS and \c propName - The named property is removed if it is an
    ///   external property, or if the \c #kXMPUtil_DoAllProperties option flag is set. It does not
    ///   matter whether the named property is an actual property or an alias.
    ///
    ///   \li Non-empty \c schemaNS and empty \c propName - All external properties in the named
    ///   schema are removed. Internal properties are also removed if the
    ///   \c #kXMPUtil_DoAllProperties option flag is set. In addition, aliases from the named schema
    ///   are removed if the \c #kXMPUtil_IncludeAliases option flag is set.
    ///
    ///   \li Empty \c schemaNS and empty \c propName - All external properties in all schemas are
    ///   removed. Internal properties are also removed if the \c #kXMPUtil_DoAllProperties option
    ///   flag is set. Aliases are handled implicitly, because the associated actuals are removed or
    ///   not.
    ///
    ///   \li It is an error to pass an empty \c schemaNS and non-empty \c propName.
    ///
    /// @param xmpObj The XMP object containing the properties to be removed.
    ///
    /// @param schemaNS Optional schema namespace URI for the properties to be removed.
    ///
    /// @param propName Optional path expression for the property to be removed.
    ///
    /// @param options Option flags to control the deletion operation. A logical OR of these
    /// bit-flag constants:
    ///   \li \c #kXMPUtil_DoAllProperties - Delete internal properties in addition to external properties.
    ///   \li \c #kXMPUtil_IncludeAliases - Include aliases if the schema is explicitly specified.

    static void RemoveProperties ( TXMPMeta<tStringObj> * xmpObj,
								   XMP_StringPtr          schemaNS = 0,
								   XMP_StringPtr          propName = 0,
								   XMP_OptionBits         options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c AppendProperties() adds or moves properties from one XMP object to another.
    ///
    /// The default operation is to append only external properties that do not already exist in the
    /// destination. Option flags allow you to	add internal properties, and to merge values of
    /// properties that exist in both the source and destination.
    ///
    ///   \li \c #kXMPUtil_DoAllProperties: Operate on all top-level properties, external and
    ///   internal. You can use this flag together with \c #kXMPUtil_ReplaceOldValues to replace the
    ///   values of existing top-level properties.
    ///
    ///   \li \c #kXMPUtil_ReplaceOldValues: Propogate all top-level properties from the source to
    ///   the destination, replacing any existing values. The values of properties in the
    ///   destination that are not in the source are not modified.<br>
    ///   The keep-or-replace-old notion also applies within structs and arrays. Top-level
    ///   properties are added to the destination if they do not already exist. If they do exist but
    ///   differ in form (simple/struct/array) then the destination is not modified. If the forms
    ///   match, simple properties are left unchanged, while structs and arrays are merged.<br>
    ///   Do not use this option when the processing is more complicated. <<than what??>>
    ///
    ///   \li \c #kXMPUtil_DeleteEmptyValues: An empty value in the source XMP causes the
    ///   corresponding destination property to be deleted. By default, empty values are treated in
    ///   the same way as non-empty values. An empty value is a simple empty string, an array with
    ///   no items,or a struct with no fields. Qualifiers are ignored.
    ///
    /// The detailed behavior is defined by the following pseudo-code:
    ///
    /// <pre>
    /// AppendProperties ( sourceXMP, destXMP, options ):
    ///    doAll = options & kXMPUtil_DoAllProperties
    ///    replaceOld = options & kXMPUtil_ReplaceOldValues
    ///    deleteEmpty = options & kXMPUtil_DeleteEmptyValues
    ///    for all source schema (top level namespaces):
    ///    for all top level properties in sourceSchema:
    ///    if doAll or prop is external:
    ///       AppendSubtree ( sourceNode, destSchema, replaceOld, deleteEmpty )
    ///
    /// AppendSubtree ( sourceNode, destParent, replaceOld, deleteEmpty ):
    ///    if deleteEmpty and source value is empty:
    ///       delete the corresponding child from destParent
    ///    else if sourceNode not in destParent (by name):
    ///       copy sourceNode's subtree to destParent
    ///    else if replaceOld:
    ///       delete subtree from destParent
    ///       copy sourceNode's subtree to destParent
    ///    else: // (Already exists in dest and not replacing, merge structs and arrays)
    ///       if sourceNode and destNode forms differ:
    ///          return, leave the destNode alone
    ///       else if form is a struct:
    ///          for each field in sourceNode:
    ///             AppendSubtree ( sourceNode.field, destNode, replaceOld )
    ///       else if form is an alt-text array:
    ///          copy new items by xml:lang value into the destination
    ///       else if form is an array:
    ///          copy new items by value into the destination, ignoring order and duplicates
    /// </pre>
    ///
    /// Array item checking is n-squared; this can be time-intensive if the replace-old options is
    /// not specified. Each source item is checked to see if it already exists in the destination,
    /// without regard to order or duplicates. Simple items are compared by value and \c xml:lang
    /// qualifier; other qualifiers are ignored. Structs are recursively compared by field names,
    /// without regard to field order. Arrays are compared by recursively comparing all items.
    ///
    /// @param source The source XMP object.
    ///
    /// @param dest The destination XMP object.
    ///
    /// @param options Option flags to control the copying.	A logical OR of these bit-flag constants:
    ///   \li \c kXMPUtil_DoAllProperties - Operate on internal properties in addition to external properties.
    ///   \li \c kXMPUtil_ReplaceOldValues - Replace the values of existing properties.
    ///   \li \c kXMPUtil_DeleteEmptyValues - Delete properties if the new value is empty.

    static void AppendProperties ( const TXMPMeta<tStringObj> & source,
								   TXMPMeta<tStringObj> *       dest,
								   XMP_OptionBits               options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DuplicateSubtree() replicates a subtree from one XMP object into another.
    ///
    /// The destination can be a different namespace and root location in the same object, or the
    /// same or a different location in another XMP object.
    ///
    /// @param source The source XMP object.
    ///
    /// @param dest The destination XMP object.
    ///
    /// @param sourceNS The schema namespace URI for the source subtree.
    ///
    /// @param sourceRoot The root location for the source subtree. Can be a general path expression,
    /// must not be null or the empty string.
    ///
    /// @param destNS The schema namespace URI for the destination. Defaults to the source namespace.
    ///
    /// @param destRoot The root location for the destination. Can be a general path expression.
    /// Defaults to the source location.
    ///
    /// @param options Option flags to control the operation. <<options?>>

    static void DuplicateSubtree ( const TXMPMeta<tStringObj> & source,
								   TXMPMeta<tStringObj> *       dest,
								   XMP_StringPtr                sourceNS,
								   XMP_StringPtr                sourceRoot,
								   XMP_StringPtr                destNS = 0,
								   XMP_StringPtr                destRoot = 0,
								   XMP_OptionBits               options = 0 );

    /// @}

    // =============================================================================================

    // =============================================================================================

};  // class TXMPUtils

// =================================================================================================

#endif // __TXMPUtils_hpp__
