#ifndef __TXMPMeta_hpp__
#define __TXMPMeta_hpp__    1

#if ( ! __XMP_hpp__ )
    #error "Do not directly include, use XMPSDK.hpp"
#endif

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

// =================================================================================================
/// \file TXMPMeta.hpp
/// \brief API for access to the XMP Toolkit core services.
///
/// \c TXMPMeta is the template class providing the core services of the XMP Toolkit. It must be
/// instantiated with a string class such as \c std::string. Read the Toolkit Overview for
/// information about the overall architecture of the XMP API, and the documentation for \c XMPSDK.hpp
/// for specific instantiation instructions.
///
/// Access these functions through the concrete class, \c SXMPMeta.
// =================================================================================================

// =================================================================================================
/// \class TXMPMeta TXMPMeta.hpp
/// \brief API for access to the XMP Toolkit core services.
///
/// \c TXMPMeta is the template class providing the core services of the XMP Toolkit. It should be
/// instantiated with a string class such as \c std::string. Read the Toolkit Overview for
/// information about the overall architecture of the XMP API, and the documentation for \c XMPSDK.hpp
/// for specific instantiation instructions.
///
/// Access these functions through the concrete class, \c SXMPMeta.
///
/// You can create \c TXMPMeta objects (also called XMP objects) from metadata that you construct,
/// or that you obtain from files using the XMP Toolkit's XMPFiles component; see \c TXMPFiles.hpp.
// =================================================================================================

template <class tStringObj> class TXMPIterator;
template <class tStringObj> class TXMPUtils;

// -------------------------------------------------------------------------------------------------

template <class tStringObj> class TXMPMeta {

public:

    // =============================================================================================
    // Initialization and termination
    // ==============================

    // ---------------------------------------------------------------------------------------------
    /// \name Initialization and termination
    ///
    /// @{

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetVersionInfo() retrieves runtime version information.
    ///
    /// The header \c XMPVersion.hpp defines a static version number for the XMP Toolkit, which
    /// describes the version of the API used at client compile time. It is not necessarily the same
    /// as the runtime version. Do not base runtime decisions on the static version alone; you can,
    /// however, compare the runtime and static versions.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta). The
    /// function can be called before calling \c TXMPMeta::Initialize().
    ///
    /// @param info [out] A buffer in which to return the version information.

    static void GetVersionInfo ( XMP_VersionInfo * info );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c Initialize() explicitly initializes the XMP Toolkit before use. */

    /// Initializes the XMP Toolkit.
    ///
    /// Call this function before making any other calls to the \c TXMPMeta functions, except
    /// \c TXMPMeta::GetVersionInfo().
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @return True on success. */
    static bool Initialize();
    // ---------------------------------------------------------------------------------------------
    /// @brief \c Terminate() explicitly terminates usage of the XMP Toolkit.
    ///
    /// Frees structures created on initialization.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).

    static void Terminate();

    /// @}

    // =============================================================================================
    // Constuctors and destructor
    // =========================

    // ---------------------------------------------------------------------------------------------
    /// \name Constructors and destructor
    /// @{

    // ---------------------------------------------------------------------------------------------
    /// @brief Default constructor, creates an empty object.
    ///
    /// The default constructor creates a new empty \c TXMPMeta object.
    ///
    /// @return The new object. */
    TXMPMeta();

    // ---------------------------------------------------------------------------------------------
    /// @brief Copy constructor, creates a client object refering to the same internal object.
    ///
    /// The copy constructor creates a new \c TXMPMeta object that refers to the same internal XMP
    /// object. as an existing \c TXMPMeta object.
    ///
    /// @param original The object to copy.
    ///
    /// @return The new object. */

    TXMPMeta ( const TXMPMeta<tStringObj> & original );

    // ---------------------------------------------------------------------------------------------
    /// @brief Assignment operator, assigns the internal reference and increments the reference count.
    ///
    /// The assignment operator assigns the internal ref from the rhs object and increments the
    /// reference count on the underlying internal XMP object.

    void operator= ( const TXMPMeta<tStringObj> & rhs );

    // ---------------------------------------------------------------------------------------------
    /// @brief Reconstructs an XMP object from an internal reference.
    ///
    /// This constructor creates a new \c TXMPMeta object that refers to the underlying reference object
    /// of an existing \c TXMPMeta object. Use to safely pass XMP objects across DLL boundaries.
    ///
    /// @param xmpRef The underlying reference object, obtained from some other XMP object with
    /// \c TXMPMeta::GetInternalRef().
    ///
    /// @return The new object.

    TXMPMeta ( XMPMetaRef xmpRef );

    // ---------------------------------------------------------------------------------------------
    /// @brief Constructs an object and parse one buffer of RDF into it.
    ///
    /// This constructor creates a new \c TXMPMeta object and populates it with metadata from a
    /// buffer containing serialized RDF. This buffer must be a complete RDF parse stream.
    ///
    /// The result of passing serialized data to this function is identical to creating an empty
    /// object then calling \c TXMPMeta::ParseFromBuffer(). To use the constructor, however, the RDF
    /// must be complete. If you need to parse data from multiple buffers, create an empty object
    /// and use  \c TXMPMeta::ParseFromBuffer().
    ///
    /// @param buffer  A pointer to the buffer of RDF to be parsed. Can be null if the length is 0;
    /// in this case, the function creates an empty object.
    ///
    /// @param xmpSize  The length in bytes of the buffer.
    ///
    /// @return The new object.

    TXMPMeta ( XMP_StringPtr buffer,
               XMP_StringLen xmpSize );

    // ---------------------------------------------------------------------------------------------
    /// @brief Destructor, typical virtual destructor. */
    virtual ~TXMPMeta() throw();

    /// @}

    // =============================================================================================
    // Global state functions
    // ======================

    // ---------------------------------------------------------------------------------------------
    /// \name Global option flags
    /// @{
    /// Global option flags affect the overall behavior of the XMP Toolkit. The available options
    /// will be declared in \c XMP_Const.h. There are none in this version of the Toolkit.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetGlobalOptions() retrieves the set of global option flags. There are none in
    /// this version of the Toolkit.
    ///
    /// This function is static; you can make the call from the class without instantiating it.
    ///
    /// @return A logical OR of global option bit-flag constants.

     static XMP_OptionBits GetGlobalOptions();

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetGlobalOptions() updates the set of global option flags. There are none in this
    /// version of the Toolkit.
    ///
    /// The entire set is replaced with the new values. If only one flag is to be modified, use
    /// \c TXMPMeta::GetGlobalOptions() to obtain the current set, modify the desired flag, then use
    /// this function to reset the value.
    ///
    /// This function is static; you can make the call from the class without instantiating it.
    ///
    /// @param options A logical OR of global option bit-flag constants.

    static void SetGlobalOptions ( XMP_OptionBits options );

    /// @}

    // ---------------------------------------------------------------------------------------------
    /// \name Internal data structure dump utilities
    /// @{
    ///
    /// These are debugging utilities that dump internal data structures, to be handled by
    /// client-defined callback described in \c XMP_Const.h.
	///
	/// @see Member function \c TXMPMeta::DumpObject()

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DumpNamespaces() sends the list of registered namespace URIs and prefixes to a handler.
    ///
    /// For debugging. Invokes a client-defined callback for each line of output.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param outProc The client-defined procedure to handle each line of output.
    ///
    /// @param clientData A pointer to client-defined data to pass to the handler.
    ///
    /// @return	A success-fail status value, returned from the handler. Zero is success, failure
    /// values are client-defined.

    static XMP_Status DumpNamespaces ( XMP_TextOutputProc outProc,
                     				   void *             clientData );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DumpAliases() sends the list of registered aliases and corresponding actuals to a handler.
    ///
    /// For debugging. Invokes a client-defined callback for each line of output.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param outProc The client-defined procedure to handle each line of output.
    ///
    /// @param clientData A pointer to client-defined data to pass to the handler.
    ///
    /// @return	A success-fail status value, returned from the handler. Zero is success, failure
    /// values are client-defined.

    static XMP_Status DumpAliases ( XMP_TextOutputProc outProc,
                  					void *             clientData );

    /// @}

    // ---------------------------------------------------------------------------------------------
    /// \name Namespace Functions
    /// @{
    ///
    /// Namespaces must be registered before use in namespace URI parameters or path expressions.
    /// Within the XMP Toolkit the registered namespace URIs and prefixes must be unique. Additional
    /// namespaces encountered when parsing RDF are automatically registered.
    ///
    /// The namespace URI should always end in an XML name separator such as '/' or '#'. This is
    /// because some forms of RDF shorthand catenate a namespace URI with an element name to form a
    /// new URI.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c RegisterNamespace() registers a namespace URI with a prefix.
    ///
    /// If the the prefix is in use, the URI of the existing prefix is overwritten.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param namespaceURI The URI for the namespace. Must be a valid XML URI.
    ///
    /// @param prefix The prefix to be used. Must be a valid XML name.
    ///
    /// @note No checking is done on either the URI or the prefix.  */

    static void RegisterNamespace ( XMP_StringPtr namespaceURI,
                                    XMP_StringPtr prefix );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetNamespacePrefix() obtains the prefix for a registered namespace URI, and
    /// reports whether the URI is registered.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param namespaceURI The URI for the namespace. Must not be null or the empty string. It is
    /// not an error if the namespace URI is not registered.
    ///
    /// @param namespacePrefix [out] A string object in which to return the prefix registered for
    /// this URI, with a terminating colon character, ':'. If the namespace is not registered, this
    /// string is not modified.
    ///
    /// @return True if the namespace URI is registered.

    static bool GetNamespacePrefix ( XMP_StringPtr namespaceURI,
                         			 tStringObj *  namespacePrefix );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetNamespaceURI() obtains the URI for a registered namespace prefix, and reports
    /// whether the prefix is registered.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param namespacePrefix The prefix for the namespace. Must not be null or the empty string.
    /// It is not an error if the namespace prefix is not registered.
    ///
    /// @param namespaceURI [out] A string object in which to return the URI registered for this
    /// prefix. If the prefix is not registered, this string is not modified.
    ///
    /// @return True if the namespace prefix is registered.

    static bool GetNamespaceURI ( XMP_StringPtr namespacePrefix,
                      			  tStringObj *  namespaceURI );

    // ---------------------------------------------------------------------------------------------
    /// @brief Not implemented.
    ///
    /// Deletes a namespace from the registry. Does nothing if the URI is not registered, or if the
    /// parameter is null or the empty string.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param namespaceURI The URI for the namespace.

    static void DeleteNamespace ( XMP_StringPtr namespaceURI );

    /// @}

    // ---------------------------------------------------------------------------------------------
    /// \name Alias functions
    /// @{
    ///
    /// Aliases in XMP serve the same purpose as Windows file shortcuts, Mac OS file aliases, or
    /// UNIX file symbolic links. The aliases are multiple names for the same property. One
    /// distinction of XMP aliases is that they are ordered. An alias name points to an actual name;
    /// the primary significance of the actual name is that it is the preferred name for output,
    /// generally the most widely recognized name.
    ///
    /// XMP restricts the names that can be aliased. The alias must be a top-level property name,
    /// not a field within a structure or an element within an array. The actual can be a top-level
    /// property name, the first element within a top-level array, or the default element in an
    /// alt-text array. This does not mean the alias can only be a simple property; you can alias a
    /// top-level structure or array to an identical top-level structure or array, or to the first
    /// item of an array of structures.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c RegisterAlias() associates an alias name with an actual name.
    ///
    /// Defines an alias mapping from one namespace/property to another. Both property names must be
    /// simple names. An alias can be a direct mapping, where the alias and actual have the same
    /// data type. It is also possible to map a simple alias to an item in an array. This can either
    /// be to the first item in the array, or to the 'x-default' item in an alt-text array. Multiple
    /// alias names can map to the same actual, as long as the forms match. It is a no-op to
    /// reregister an alias in an identical fashion.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param aliasNS The namespace URI for the alias. Must not be null or the empty string.
    ///
    /// @param aliasProp The name of the alias. Must be a simple name, not null or the empty string
    /// and not a general path expression.
    ///
    /// @param actualNS The namespace URI for the actual. Must not be null or the empty string.
    ///
    /// @param actualProp The name of the actual. Must be a simple name, not null or the empty string
    /// and not a general path expression.
    ///
    /// @param arrayForm Provides the array form for simple aliases to an array item. This is needed
    /// to know what kind of array to create if set for the first time via the simple alias. Pass
    /// \c #kXMP_NoOptions, the default value, for all direct aliases regardless of whether the actual
    /// data type is an array or not. One of these constants:
    ///
    ///   \li \c #kXMP_NoOptions - This is a direct mapping. The actual data type does not matter.
    ///   \li \c #kXMP_PropValueIsArray - The actual is an unordered array, the alias is to the
    ///   first element of the array.
    ///   \li \c #kXMP_PropArrayIsOrdered - The actual is an ordered array, the alias is to the
    ///   first element of the array.
    ///   \li \c #kXMP_PropArrayIsAlternate - The actual is an alternate array, the alias is to the
    ///   first element of the array.
    ///   \li \c #kXMP_PropArrayIsAltText - The actual is an alternate text array, the alias is to
    ///   the 'x-default' element of the array.  */

    static void RegisterAlias ( XMP_StringPtr  aliasNS,
                    			XMP_StringPtr  aliasProp,
                    			XMP_StringPtr  actualNS,
                    			XMP_StringPtr  actualProp,
                    			XMP_OptionBits arrayForm = kXMP_NoOptions );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ResolveAlias() reports whether a name is an alias, and what it is aliased to.
    ///
    /// Output strings are not written until return, so you can use this to
    /// "reduce" a path to the base form as follows:
    /// <pre>
    ///   isAlias = SXMPMeta::ResolveAlias ( ns.c_str(), path.c_str(), &ns, &path, 0 );
    /// </pre>
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param aliasNS The namespace URI for the alias. Must not be null or the empty string.
    ///
    /// @param aliasProp The name of the alias. Can be an arbitrary path expression path, must not
    /// null or the empty string.
    ///
    /// @param actualNS  [out] A string object in which to return the namespace URI for the actual.
    /// Not modified if the given name is not an alias. Can be null if the namespace URI is not wanted.
    ///
    /// @param actualProp  [out] A string object in which to return the path of the actual.
    /// Not modified if the given name is not an alias. Can be null if the actual's path is not wanted.
    ///
    /// @param arrayForm [out] A string object in which to return the array form of the actual. This
    /// is 0 (\c #kXMP_NoOptions) if the alias and actual forms match, otherwise it is the options
    /// passed to \c TXMPMeta::RegisterAlias(). Not modified if the given name is not an alias. Can
    /// be null if the actual's array form is not wanted.
    ///
    /// @return True if the provided name is an alias.

    static bool ResolveAlias ( XMP_StringPtr    aliasNS,
                   			   XMP_StringPtr    aliasProp,
                  			   tStringObj *     actualNS,
                 			   tStringObj *     actualProp,
                 			   XMP_OptionBits * arrayForm );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DeleteAlias() deletes an alias.
    ///
    /// This deletes only the registration of the alias, it does not delete the actual property.
    /// It deletes any view of the property through the alias name.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param aliasNS The namespace URI for the alias. Must not be null or the empty string.
    ///
    /// @param aliasProp The name of the alias. Must be a simple name, not null or the empty string
    /// and not a general path expression. It is not an error to provide
    /// a name that has not been registered as an alias.

    static void DeleteAlias ( XMP_StringPtr aliasNS,
                  			  XMP_StringPtr aliasProp );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c RegisterStandardAliases() registers all of the built-in aliases for a standard namespace.
    ///
    /// The built-in aliases are documented in the XMP Specification. This function registers the
    /// aliases in the given namespace; that is, it creates the aliases from this namespace to
    /// actuals in other namespaces.
    ///
    /// This function is static; make the call directly from the concrete class (\c SXMPMeta).
    ///
    /// @param schemaNS The namespace URI for the aliases. Must not be null or the empty string.

    static void RegisterStandardAliases ( XMP_StringPtr schemaNS );

    /// @}

    // =============================================================================================
    // Basic property manipulation functions
    // =====================================

    // *** Should add discussion of schemaNS and propName prefix usage.

    // ---------------------------------------------------------------------------------------------
    /// \name Accessing property values
    /// @{
    ///
    /// The property value accessors all take a property specification; the top level namespace URI
    ///	(the "schema" namespace) and the basic name of the property being referenced. See the
    ///	introductory discussion of path expression usage for more information.
	///
    /// The accessor functions return true if the specified property exists. If it does, output
    /// parameters return the value (if any) and option flags describing the property. The option
    /// bit-flag constants that describe properties are \c kXMP_PropXx and
	/// \c kXMP_ArrayIsXx. See \c #kXMP_PropValueIsURI and following, and macros \c #XMP_PropIsSimple
	/// and following in \c XMP_Const.h. If the property exists and has a value, it is returned as a
	/// Unicode string in UTF-8 encoding. Arrays and the non-leaf levels of structs do not have
	/// values.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetProperty() reports whether a property exists, and retrieves its value.
    ///
    /// This is the simplest property accessor. Use this to retrieve the values of top-level simple
    /// properties, or after using the path composition functions in \c TXMPUtils.
    ///
    /// When specifying a namespace and path (in this and all other accessors):
    ///   \li If a namespace URI is specified, it must be for a registered namespace.
    ///   \li If the namespace is specified only by a prefix in the property name path,
    /// it must be a registered prefix.
    ///   \li If both a URI and path prefix are present, they must be corresponding
    /// parts of a registered namespace.
    ///
    /// @param schemaNS The namespace URI for the property. The URI must be for a registered
    /// namespace. Can be null or the empty string if the first component of the \c propName path
    /// contains a namespace prefix.
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string. The first component can be a namespace prefix; if present without a
    /// \c schemaNS value, the prefix specifies the namespace. The prefix must be for a registered
    /// namespace, and if a namespace URI is specified, must match the registered prefix for that
    /// namespace.
    ///
    /// @param propValue [out] A string object in which to return the value of the property, if the
    /// property exists and has a value. Arrays and non-leaf levels of structs do not have values.
    /// Can be null if the value is not wanted.
    ///
    /// @param options A buffer in which to return option flags describing the property. Can be null
    /// if the flags are not wanted.
    ///
    /// @return True if the property exists.

    bool GetProperty ( XMP_StringPtr    schemaNS,
                 	   XMP_StringPtr    propName,
                       tStringObj *     propValue,
                  	   XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetArrayItem() provides access to items within an array.
    ///
    /// Reports whether the item exists; if it does, and if it has a value, the function retrieves
    /// the value. Items are accessed by an integer index, where the first item has index 1.
    ///
    /// @param schemaNS The namespace URI for the array; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param itemIndex The 1-based index of the desired item. Use the macro \c #kXMP_ArrayLastItem
    /// to specify the last existing array item.
    ///
    /// @param itemValue [out] A string object in which to return the value of the array item, if it
    /// has a value. Arrays and non-leaf levels of structs do not have values. Can be null if the
    /// value is not wanted.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the array item.
    /// Can be null if the flags are not wanted.
    ///
    /// @return True if the array item exists.

    bool GetArrayItem ( XMP_StringPtr    schemaNS,
                   		XMP_StringPtr    arrayName,
                   		XMP_Index        itemIndex,
                   		tStringObj *     itemValue,
                   		XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetStructField() provides access to fields within a nested structure.
    ///
    /// Reports whether the field exists; if it does, and if it has a value, the function retrieves
    /// the value.
    ///
    /// @param schemaNS The namespace URI for the struct; see \c GetProperty().
    ///
    /// @param structName The name of the struct. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param fieldNS The namespace URI for the field. Same URI and prefix usage as the \c schemaNS
    /// and \c structName parameters.
    ///
    /// @param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Same URI and prefix usage as the \c schemaNS and \c structName parameters.
    ///
    /// @param fieldValue [out] A string object in which to return the value of the field, if the
    /// field has a value. Arrays and non-leaf levels of structs do not have values. Can be null if
    /// the value is not wanted.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the field. Can
    /// be null if the flags are not wanted.
    ///
    /// @return True if the field exists.

    bool GetStructField ( XMP_StringPtr    schemaNS,
                     	  XMP_StringPtr    structName,
                     	  XMP_StringPtr    fieldNS,
                     	  XMP_StringPtr    fieldName,
                     	  tStringObj *     fieldValue,
                     	  XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetQualifier() provides access to a qualifier attached to a property.
    ///
    /// @note In this version of the Toolkit, qualifiers are supported only for simple leaf properties.
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
    /// @param qualName The name of the qualifier. Must be a single XML name, must not be null or
    /// the empty string. Same URI and prefix usage as the \c schemaNS and \c propName parameters.
    ///
    /// @param qualValue [out] A string object in which to return the value of the qualifier, if the
    /// qualifier has a value. Arrays and non-leaf levels of structs do not have values. Can be null
    /// if the value is not wanted.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the qualifier.
    /// Can be null if the flags are not wanted.
    ///
    /// @return True if the qualifier exists.

    bool GetQualifier ( XMP_StringPtr    schemaNS,
                   		XMP_StringPtr    propName,
                  		XMP_StringPtr    qualNS,
                  		XMP_StringPtr    qualName,
                   		tStringObj *     qualValue,
                   		XMP_OptionBits * options ) const;

    /// @}

    // =============================================================================================

    // ---------------------------------------------------------------------------------------------
    /// \name Creating properties and setting their values
 	/// @{
 	///
    /// These functions all take a property specification; the top level namespace URI (the "schema"
    /// namespace) and the basic name of the property being referenced. See the introductory
    /// discussion of path expression usage for more information.
	///
    /// All of the functions take a UTF-8 encoded Unicode string for the property value. Arrays and
    /// non-leaf levels of structs do not have values. The value can be passed as an
    /// \c #XMP_StringPtr (a pointer to a null-terminated string), or as a string object
    /// (\c tStringObj).

    /// Each function takes an options flag that describes the property. You can use these functions
    /// to create empty arrays and structs by setting appropriate option flags. When you assign a
    /// value, all levels of a struct that are implicit in the assignment are created if necessary.
    /// \c TXMPMeta::AppendArrayItem() implicitly creates the named array if necessary.
    ///
    /// The allowed option bit-flags include:
    ///   \li \c #kXMP_PropValueIsStruct - Can be used to create an empty struct.
    ///		A struct is implicitly created when the first field is set.
    ///   \li \c #kXMP_PropValueIsArray - By default, a general unordered array (bag).
    ///   \li \c #kXMP_PropArrayIsOrdered - An ordered array.
   	///   \li \c #kXMP_PropArrayIsAlternate - An alternative array.
   	///   \li \c #kXMP_PropArrayIsAltText - An alt-text array. Each array element must
    /// 	be a simple property with an \c xml:lang attribute.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetProperty() creates or sets a property value.
    ///
    /// This is the simplest property setter. Use it for top-level simple properties, or after using
    /// the path composition functions in \c TXMPUtils.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue The new value, a pointer to a null terminated UTF-8 string. Must be null
    /// for arrays and non-leaf levels of structs that do not have values.
    ///
    /// @param options Option flags describing the property; a logical OR of allowed bit-flag
    /// constants; see \c #kXMP_PropValueIsStruct and following. Must match the type of a property
    /// that already exists.

    void SetProperty ( XMP_StringPtr  schemaNS,
					   XMP_StringPtr  propName,
					   XMP_StringPtr  propValue,
					   XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetProperty() creates or sets a property value using a string object.
	///
    /// Overloads the basic form of the function, allowing you to pass a string object
	/// for the item value. It is otherwise identical; see details in the canonical form.

    void SetProperty ( XMP_StringPtr      schemaNS,
					   XMP_StringPtr      propName,
					   const tStringObj & propValue,
					   XMP_OptionBits     options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetArrayItem() creates or sets the value of an item within an array.
    ///
    /// Items are accessed by an integer index, where the first item has index 1. This function
    /// creates the item if necessary, but the array itself must already exist Use
    /// \c AppendArrayItem() to create arrays. A new item is automatically appended if the index is the
    /// array size plus 1. To insert a new item before or after an existing item, use option flags.
    ///
    /// Use \c TXMPUtils::ComposeArrayItemPath() to create a complex path.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param itemIndex The 1-based index of the desired item. Use the macro \c #kXMP_ArrayLastItem
    /// to specify the last existing array item.
    ///
    /// @param itemValue The new item value, a null-terminated UTF-8 string, if the array item has a
    /// value.
    ///
    /// @param options Option flags describing the array type and insertion location for a new item;
    /// a logical OR of allowed bit-flag constants. The type, if specified, must match the existing
    /// array type, \c #kXMP_PropArrayIsOrdered, \c #kXMP_PropArrayIsAlternate, or
    /// \c #kXMP_PropArrayIsAltText. Default (0 or \c #kXMP_NoOptions) matches the  existing array type.
    ///
    /// To insert a new item before or after the specified index, set flag \c #kXMP_InsertBeforeItem
    /// or \c #kXMP_InsertAfterItem.

    void SetArrayItem ( XMP_StringPtr  schemaNS,
					    XMP_StringPtr  arrayName,
					    XMP_Index      itemIndex,
					    XMP_StringPtr  itemValue,
					    XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetArrayItem() creates or sets the value of an item within an array using a string object.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object in which to
	/// return the item value. It is otherwise identical; see details in the canonical form.

    void SetArrayItem ( XMP_StringPtr      schemaNS,
					    XMP_StringPtr      arrayName,
					    XMP_Index          itemIndex,
					    const tStringObj & itemValue,
					    XMP_OptionBits     options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c AppendArrayItem() adds an item to an array, creating the array if necessary.
    ///
    /// This function simplifies construction of an array by not requiring that you pre-create an
    /// empty array. The array that is assigned is created automatically if it does not yet exist.
    /// If the array exists, it must have the form specified by the options. Each call appends a new
    /// item to the array.
    ///
    /// Use \c TXMPUtils::ComposeArrayItemPath() to create a complex path.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param arrayOptions Option flags describing the array type to create; a logical OR of
    /// allowed bit-flag constants, \c #kXMP_PropArrayIsOrdered, \c #kXMP_PropArrayIsAlternate, or
    /// \c #kXMP_PropArrayIsAltText. If the array exists, must match the existing array type or be
    /// null (0 or \c #kXMP_NoOptions).
    ///
    /// @param itemValue The new item value, a null-terminated UTF-8 string, if the array item has a
    /// value.
    ///
    /// @param itemOptions Option flags describing the item type to create; one of the bit-flag
    /// constants \c #kXMP_PropValueIsArray or \c #kXMP_PropValueIsStruct to create a complex array
    /// item.

    void AppendArrayItem ( XMP_StringPtr  schemaNS,
						   XMP_StringPtr  arrayName,
						   XMP_OptionBits arrayOptions,
						   XMP_StringPtr  itemValue,
						   XMP_OptionBits itemOptions = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c AppendArrayItem() adds an item to an array using a string object value, creating
    /// the array if necessary.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object in which to
	/// return the item value. It is otherwise identical; see details in the canonical form.

    void AppendArrayItem ( XMP_StringPtr      schemaNS,
						   XMP_StringPtr      arrayName,
						   XMP_OptionBits     arrayOptions,
						   const tStringObj & itemValue,
						   XMP_OptionBits     itemOptions = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetStructField() creates or sets the value of a field within a nested structure.
    ///
    /// Use this to  set a value within an existing structure, create a new field within an existing
    /// structure, or create an empty structure of any depth. If you set a field in a structure that
    /// does not exist, the structure is automatically created.
    ///
    /// Use \c TXMPUtils::ComposeStructFieldPath() to create a complex path.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param structName The name of the struct. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param fieldNS The namespace URI for the field. Same namespace and prefix usage as
    /// \c GetProperty().
    ///
    /// @param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Same namespace and prefix usage as \c GetProperty().
    ///
    /// @param fieldValue The new value, a null-terminated UTF-8 string, if the field has a value.
    /// Null to create a new, empty struct or empty field in an existing struct.
    ///
    /// @param options Option flags describing the property, in which the bit-flag
    /// \c #kXMP_PropValueIsStruct must be set to create a struct.

    void SetStructField ( XMP_StringPtr   schemaNS,
						  XMP_StringPtr   structName,
						  XMP_StringPtr   fieldNS,
						  XMP_StringPtr   fieldName,
						  XMP_StringPtr   fieldValue,
						  XMP_OptionBits  options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetStructField() creates or sets the value of a field within a nested structure,
    /// using a string object.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object in which to
	/// return the field value. It is otherwise identical; see details in the canonical form.

    void SetStructField ( XMP_StringPtr      schemaNS,
						  XMP_StringPtr      structName,
						  XMP_StringPtr      fieldNS,
						  XMP_StringPtr      fieldName,
						  const tStringObj & fieldValue,
						  XMP_OptionBits     options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetQualifier() creates or sets a qualifier attached to a property.
    ///
    /// Use this to  set a value for an existing qualifier, or create a new qualifier. <<how do
    /// options work? macro vs bit-flag? interaction w/XMP_PropHasQualifier?>> Use
    /// \c TXMPUtils::ComposeQualifierPath() to create a complex path.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property to which the qualifier is attached. Can be a
    /// general path expression, must not be null or the empty string; see \c GetProperty() for
    /// namespace prefix usage.
    ///
    /// @param qualNS The namespace URI for the qualifier. Same namespace and prefix usage as
    /// \c GetProperty().
    ///
    /// @param qualName The name of the qualifier. Must be a single XML name, must not be null or
    /// the empty string. Same namespace and prefix usage as \c GetProperty().
    ///
    /// @param qualValue The new value, a null-terminated UTF-8 string, if the qualifier has a
    /// value. Null to create a new, empty qualifier.
    ///
    /// @param options Option flags describing the <<qualified property? qualifier?>>, a logical OR
    /// of property-type bit-flag constants. Use the macro \c #XMP_PropIsQualifier to create a
    /// qualifier.	 <<??>>

    void SetQualifier ( XMP_StringPtr  schemaNS,
					    XMP_StringPtr  propName,
					    XMP_StringPtr  qualNS,
					    XMP_StringPtr  qualName,
					    XMP_StringPtr  qualValue,
					    XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetQualifier() creates or sets a qualifier attached to a property using a string object.
    ///
    /// Overloads the basic form of the function, allowing you to pass a string object
	/// for the qualifier value. It is otherwise identical; see details in the canonical form.

    void SetQualifier ( XMP_StringPtr      schemaNS,
					    XMP_StringPtr      propName,
					    XMP_StringPtr      qualNS,
					    XMP_StringPtr      qualName,
					    const tStringObj & qualValue,
					    XMP_OptionBits     options = 0 );

    /// @}

    // =============================================================================================

    // ---------------------------------------------------------------------------------------------
    /// \name Detecting and deleting properties.
    /// @{
    ///
    /// The namespace URI and prefix usage for property specifiers in these functions is the same as
    /// for \c TXMPMeta::GetProperty().

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DeleteProperty() deletes an XMP subtree rooted at a given property.
    ///
    /// It is not an error if the property does not exist.
    ///
    /// @param schemaNS The namespace URI for the property; see \c GetProperty().
    ///
    /// @param propName The name of the property; see \c GetProperty().

    void DeleteProperty ( XMP_StringPtr schemaNS,
                     	  XMP_StringPtr propName );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DeleteArrayItem() deletes an XMP subtree rooted at a given array item.
    ///
    /// It is not an error if the array item does not exist. Use
    /// \c TXMPUtils::ComposeArrayItemPath() to create a complex path.
    ///
    /// @param schemaNS The namespace URI for the array; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param itemIndex The 1-based index of the desired item. Use the macro \c #kXMP_ArrayLastItem
    /// to specify the last existing array item.

    void DeleteArrayItem ( XMP_StringPtr schemaNS,
						   XMP_StringPtr arrayName,
						   XMP_Index     itemIndex );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DeleteStructField() deletes an XMP subtree rooted at a given struct field.
    ///
    /// It is not an error if the field does not exist.
    ///
    /// @param schemaNS The namespace URI for the struct; see \c GetProperty().
    ///
    /// @param structName The name of the struct. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param fieldNS The namespace URI for the field. Same namespace and prefix usage as
    /// \c GetProperty().
    ///
    /// @param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Same namespace and prefix usage as \c GetProperty().

    void DeleteStructField ( XMP_StringPtr schemaNS,
							 XMP_StringPtr structName,
							 XMP_StringPtr fieldNS,
							 XMP_StringPtr fieldName );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DeleteQualifier() deletes an XMP subtree rooted at a given qualifier.
    ///
    /// It is not an error if the qualifier does not exist.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property to which the qualifier is attached. Can be a
    /// general path expression, must not be null or the empty string; see \c GetProperty() for
    /// namespace prefix usage.
    ///
    /// @param qualNS The namespace URI for the qualifier. Same namespace and prefix usage as
    /// \c GetProperty().
    ///
    /// @param qualName The name of the qualifier. Must be a single XML name, must not be null or
    /// the empty string. Same namespace and prefix usage as \c GetProperty().

    void DeleteQualifier ( XMP_StringPtr schemaNS,
						   XMP_StringPtr propName,
						   XMP_StringPtr qualNS,
						   XMP_StringPtr qualName );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DoesPropertyExist() reports whether a property currently exists.
    ///
    /// @param schemaNS The namespace URI for the property; see \c GetProperty().
    ///
    /// @param propName The name of the property; see \c GetProperty().
    ///
    /// @return True if the property exists.

    bool DoesPropertyExist ( XMP_StringPtr schemaNS,
                        	 XMP_StringPtr propName ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DoesArrayItemExist() reports whether an array item currently exists.
    ///
    /// Use \c TXMPUtils::ComposeArrayItemPath() to create a complex path.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param itemIndex The 1-based index of the desired item. Use the macro \c #kXMP_ArrayLastItem
    /// to specify the last existing array item.
    ///
    /// @return True if the array item exists.

    bool DoesArrayItemExist ( XMP_StringPtr schemaNS,
							  XMP_StringPtr arrayName,
							  XMP_Index     itemIndex ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DoesStructFieldExist() reports whether a struct field currently exists.
    ///
    /// Use \c TXMPUtils::ComposeStructFieldPath() to create a complex path.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param structName The name of the struct. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param fieldNS The namespace URI for the field. Same namespace and prefix usage as
    /// \c GetProperty().
    ///
    /// @param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Same namespace and prefix usage as \c GetProperty().
    ///
    /// @return True if the field exists.

    bool DoesStructFieldExist ( XMP_StringPtr schemaNS,
							    XMP_StringPtr structName,
							    XMP_StringPtr fieldNS,
							    XMP_StringPtr fieldName ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DoesQualifierExist() reports whether a qualifier currently exists.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property to which the qualifier is attached. Can be a
    /// general path expression, must not be null or the empty string; see \c GetProperty() for
    /// namespace prefix usage.
    ///
    /// @param qualNS The namespace URI for the qualifier. Same namespace and prefix usage as
    /// \c GetProperty().
    ///
    /// @param qualName The name of the qualifier. Must be a single XML name, must not be null or
    /// the empty string. Same namespace and prefix usage as \c GetProperty().
    ///
    /// @return True if the qualifier exists.

    bool DoesQualifierExist ( XMP_StringPtr schemaNS,
							  XMP_StringPtr propName,
							  XMP_StringPtr qualNS,
							  XMP_StringPtr qualName ) const;

    /// @}

    // =============================================================================================
    // Specialized Get and Set functions
    // =============================================================================================

    // ---------------------------------------------------------------------------------------------
    /// \name Accessing properties as binary values.
    /// @{
    ///
	/// These are very similar to \c TXMPMeta::GetProperty() and \c TXMPMeta::SetProperty(), except
	/// that the value is returned or provided in binary form instead of as a UTF-8 string.
	/// \c TXMPUtils provides functions for converting between binary and string values.
    /// Use the path composition functions in  \c TXMPUtils	to compose complex path expressions
    /// for fields or items in nested structures or arrays, or for qualifiers.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetProperty_Bool() retrieves the value of a Boolean property as a C++ bool.
    ///
    /// Reports whether a property exists, and retrieves its binary value and property type information.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue [out] A buffer in which to return the binary value. Can be null if the
    /// value is not wanted. Must be null for arrays and non-leaf levels of structs that do not have
    /// values.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the property, a
    /// logical OR of allowed bit-flag constants; see \c #kXMP_PropValueIsStruct and following. Can
    /// be null if flags are not wanted.
    ///
    /// @return True if the property exists.

    bool GetProperty_Bool ( XMP_StringPtr    schemaNS,
						    XMP_StringPtr    propName,
						    bool *           propValue,
						    XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetProperty_Int() retrieves the value of an integer property as a C long integer.
    ///
    /// Reports whether a property exists, and retrieves its binary value and property type information.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue [out] A buffer in which to return the binary value. Can be null if the
    /// value is not wanted. Must be null for arrays and non-leaf levels of structs that do not have
    /// values.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the property, a
    /// logical OR of allowed bit-flag constants; see \c #kXMP_PropValueIsStruct and following. Can
    /// be null if flags are not wanted.
    ///
    /// @return True if the property exists.

    bool GetProperty_Int ( XMP_StringPtr    schemaNS,
						   XMP_StringPtr    propName,
						   long *           propValue,
						   XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetProperty_Int64() retrieves the value of an integer property as a C long long integer.
    ///
    /// Reports whether a property exists, and retrieves its binary value and property type information.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue [out] A buffer in which to return the binary value. Can be null if the
    /// value is not wanted. Must be null for arrays and non-leaf levels of structs that do not have
    /// values.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the property, a
    /// logical OR of allowed bit-flag constants; see \c #kXMP_PropValueIsStruct and following. Can
    /// be null if flags are not wanted.
    ///
    /// @return True if the property exists.

    bool GetProperty_Int64 ( XMP_StringPtr    schemaNS,
							 XMP_StringPtr    propName,
							 long long *      propValue,
							 XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetProperty_Float() retrieves the value of a floating-point property as a C double float.
    ///
    /// Reports whether a property exists, and retrieves its binary value and property type information.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue [out] A buffer in which to return the binary value. Can be null if the
    /// value is not wanted. Must be null for arrays and non-leaf levels of structs that do not have
    /// values.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the property, a
    /// logical OR of allowed bit-flag constants; see \c #kXMP_PropValueIsStruct and following. Can
    /// be null if flags are not wanted.
    ///
    /// @return True if the property exists.

    bool GetProperty_Float ( XMP_StringPtr    schemaNS,
							 XMP_StringPtr    propName,
							 double *         propValue,
							 XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetProperty_Date() retrieves the value of a date-time property as an \c #XMP_DateTime structure.
    ///
    /// Reports whether a property exists, and retrieves its binary value and property type information.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue [out] A buffer in which to return the binary value. Can be null if the
    /// value is not wanted. Must be null for arrays and non-leaf levels of structs that do not have
    /// values.
    ///
    /// @param options [out] A buffer in which to return the option flags describing the property, a
    /// logical OR of allowed bit-flag constants; see \c #kXMP_PropValueIsStruct and following. Can
    /// be null if flags are not wanted.
    ///
    /// @return True if the property exists.

    bool GetProperty_Date ( XMP_StringPtr    schemaNS,
						    XMP_StringPtr    propName,
						    XMP_DateTime *   propValue,
						    XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetProperty_Bool() sets the value of a Boolean property using a C++ bool.
    ///
    /// Sets a property with a binary value, creating it if necessary.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue The new binary value. Can be null if creating the property. Must be null
    /// for arrays and non-leaf levels of structs that do not have values.
    ///
    /// @param options Option flags describing the property; a logical OR of allowed bit-flag
    /// constants; see \c #kXMP_PropValueIsStruct and following. Must match the type of a property
    /// that already exists.

    void SetProperty_Bool ( XMP_StringPtr  schemaNS,
						    XMP_StringPtr  propName,
						    bool           propValue,
						    XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetProperty_Int() sets the value of an integer property using a C long integer.
    ///
    /// Sets a property with a binary value, creating it if necessary.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue The new binary value. Can be null if creating the property. Must be null
    /// for arrays and non-leaf levels of structs that do not have values.
    ///
    /// @param options Option flags describing the property; a logical OR of allowed bit-flag
    /// constants; see \c #kXMP_PropValueIsStruct and following. Must match the type of a property
    /// that already exists.

    void SetProperty_Int ( XMP_StringPtr  schemaNS,
						   XMP_StringPtr  propName,
						   long           propValue,
						   XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetProperty_Int64() sets the value of an integer property using a C long long integer.
    ///
    /// Sets a property with a binary value, creating it if necessary.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue The new binary value. Can be null if creating the property. Must be null
    /// for arrays and non-leaf levels of structs that do not have values.
    ///
    /// @param options Option flags describing the property; a logical OR of allowed bit-flag
    /// constants; see \c #kXMP_PropValueIsStruct and following. Must match the type of a property
    /// that already exists.

    void SetProperty_Int64 ( XMP_StringPtr  schemaNS,
							 XMP_StringPtr  propName,
							 long long      propValue,
							 XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetProperty_Float() sets the value of a floating-point property using a C double float.
    ///
    /// Sets a property with a binary value, creating it if necessary.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue The new binary value. Can be null if creating the property. Must be null
    /// for arrays and non-leaf levels of structs that do not have values.
    ///
    /// @param options Option flags describing the property; a logical OR of allowed bit-flag
    /// constants; see \c #kXMP_PropValueIsStruct and following. Must match the type of a property
    /// that already exists.

    void SetProperty_Float ( XMP_StringPtr  schemaNS,
							 XMP_StringPtr  propName,
							 double         propValue,
							 XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetProperty_Date() sets the value of a date/time property using an \c #XMP_DateTime structure.
    ///
    /// Sets a property with a binary value, creating it if necessary.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param propName The name of the property. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param propValue The new binary value. Can be null if creating the property. Must be null
    /// for arrays and non-leaf levels of structs that do not have values.
    ///
    /// @param options Option flags describing the property; a logical OR of allowed bit-flag
    /// constants; see \c #kXMP_PropValueIsStruct and following. Must match the type of a property
    /// that already exists.

    void SetProperty_Date ( XMP_StringPtr         schemaNS,
						    XMP_StringPtr         propName,
						    const XMP_DateTime &  propValue,
						    XMP_OptionBits        options = 0 );

    /// @}
    // =============================================================================================
    /// \name Accessing localized text (alt-text) properties.
    /// @{
    ///
	/// Localized text properties are stored in alt-text arrays. They allow multiple concurrent
	/// localizations of a property value, for example a document title or copyright in several
	/// languages.
    ///
    /// These functions provide convenient support for localized text properties, including a
    /// number of special and obscure aspects. The most important aspect of these functions is that
    /// they select an appropriate array item based on one or two RFC 3066 language tags. One of
    /// these languages, the "specific" language, is preferred and selected if there is an exact
    /// match. For many languages it is also possible to define a "generic" language that can be
    /// used if there is no specific language match. The generic language must be a valid RFC 3066
    /// primary subtag, or the empty string.
    ///
    /// For example, a specific language of "en-US" should be used in the US, and a specific
    /// language of "en-UK" should be used in England. It is also appropriate to use "en" as the
    /// generic language in each case. If a US document goes to England, the "en-US" title is
    /// selected by using the "en" generic language and the "en-UK" specific language.
    ///
    /// It is considered poor practice, but allowed, to pass a specific language that is just an
    /// RFC 3066 primary tag. For example "en" is not a good specific language, it should only be
    /// used as a generic language. Passing "i" or "x" as the generic language is also considered
    /// poor practice but allowed.
    ///
    /// Advice from the W3C about the use of RFC 3066 language tags can be found at:
    ///     \li http://www.w3.org/International/articles/language-tags/
    ///
    /// \note RFC 3066 language tags must be treated in a case insensitive manner. The XMP toolkit
    /// does this by normalizing their capitalization:
 	/// 	\li The primary subtag is lower case, the suggested practice of ISO 639.
	/// 	\li All 2 letter secondary subtags are upper case, the suggested practice of ISO 3166.
	/// 	\li All other subtags are lower case.
    ///
    /// The XMP specification defines an artificial language, "x-default", that is used to
    /// explicitly denote a default item in an alt-text array. The XMP toolkit normalizes alt-text
    /// arrays such that the x-default item is the first item. The \c SetLocalizedText() function
    /// has several special features related to the x-default item, see its description for details.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetLocalizedText() retrieves information about a selected item in an alt-text array.
    ///
    /// The array item is selected according to these rules:
    ///   \li Look for an exact match with the specific language.
    ///   \li If a generic language is given, look for a partial match.
    ///   \li Look for an x-default item.
    ///   \li Choose the first item.
    ///
    /// A partial match with the generic language is where the start of the item's language matches
    /// the generic string and the next character is '-'. An exact match is also recognized as a
    /// degenerate case.
    ///
    /// You can pass "x-default" as the specific language. In this case, selection of an
    /// \c x-default item is an exact match by the first rule, not a selection by the 3rd rule. The
    /// last 2 rules are fallbacks used when the specific and generic languages fail to produce a
    /// match.
    ///
    /// The return value reports whether a match was successfully made.
    ///
    /// @param schemaNS The namespace URI for the alt-text array; see \c GetProperty().
    ///
    /// @param altTextName The name of the alt-text array. Can be a general path expression, must
    /// not be null or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param genericLang The name of the generic language as an RFC 3066 primary subtag. Can be
    /// null or the empty string if no generic language is wanted.
    ///
    /// @param specificLang The name of the specific language as an RFC 3066 tag, or "x-default".
    /// Must not be null or the empty string.
    ///
    /// @param actualLang [out] A string object in which to return the language of the selected
    /// array item, if an appropriate array item is found. Can be null if the language is not wanted.
    ///
    /// @param itemValue [out] A string object in which to return the value of the array item, if an
    /// appropriate array item is found. Can be null if the value is not wanted.
    ///
    /// @param options A buffer in which to return the option flags that describe the array item, if
    /// an appropriate array item is found. Can be null if the flags are not wanted.
    ///
    /// @return True if an appropriate array item exists.

    bool GetLocalizedText ( XMP_StringPtr    schemaNS,
						    XMP_StringPtr    altTextName,
						    XMP_StringPtr    genericLang,
						    XMP_StringPtr    specificLang,
						    tStringObj *     actualLang,
						    tStringObj *     itemValue,
						    XMP_OptionBits * options ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetLocalizedText() modifies the value of a selected item in an alt-text array.
    ///
    /// Creates an appropriate array item if necessary, and handles special cases for the x-default
    /// item.
    ///
    /// The array item is selected according to these rules:
    ///   \li Look for an exact match with the specific language.
    ///   \li If a generic language is given, look for a partial match.
    ///   \li Look for an x-default item.
    ///   \li Choose the first item.
    ///
    /// A partial match with the generic language is where the start of the item's language matches
    /// the generic string and the next character is '-'. An exact match is also recognized as a
    /// degenerate case.
    ///
    /// You can pass "x-default" as the specific language. In this case, selection of an
    /// \c x-default item is an exact match by the first rule, not a selection by the 3rd rule. The
    /// last 2 rules are fallbacks used when the specific and generic languages fail to produce a
    /// match.
    ///
    /// Item values are modified according to these rules:
    ///
    ///   \li If the selected item is from a match with the specific language, the value of that
    ///   item is modified. If the existing value of that item matches the existing value of the
    ///   x-default item, the x-default item is also modified. If the array only has 1 existing item
    ///   (which is not x-default), an x-default item is added with the given value.
    ///
    ///   \li If the selected item is from a match with the generic language and there are no other
    ///   generic matches, the value of that item is modified. If the existing value of that item
    ///   matches the existing value of the x-default item, the x-default item is also modified. If
    ///   the array only has 1 existing item (which is not x-default), an x-default item is added
    ///   with the given value.
    ///
    ///   \li If the selected item is from a partial match with the generic language and there are
    ///   other partial matches, a new item is created for the specific language. The x-default item
    ///   is not modified.
    ///
    ///   \li If the selected item is from the last 2 rules then a new item is created for the
    ///   specific language. If the array only had an x-default item, the x-default item is also
    ///   modified. If the array was empty, items are created for the specific language and
    ///   x-default.
    ///
    /// @param schemaNS The namespace URI for the alt-text array; see \c GetProperty().
    ///
    /// @param altTextName The name of the alt-text array. Can be a general path expression, must
    /// not be null or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param genericLang The name of the generic language as an RFC 3066 primary subtag. Can be
    /// null or the empty string if no generic language is wanted.
    ///
    /// @param specificLang The name of the specific language as an RFC 3066 tag, or "x-default".
    /// Must not be null or the empty string.
    ///
    /// @param itemValue The new value for the matching array item, specified as a null-terminated
    /// UTF-8 string.
    ///
    /// @param options Option flags, none currently defined.

    void SetLocalizedText ( XMP_StringPtr  schemaNS,
						    XMP_StringPtr  altTextName,
						    XMP_StringPtr  genericLang,
						    XMP_StringPtr  specificLang,
						    XMP_StringPtr  itemValue,
						    XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetLocalizedText() modifies the value of a selected item in an alt-text array using
    /// a string object.
    ///
    /// Creates an appropriate array item if necessary, and handles special cases for the x-default
    /// item.
    ///
    /// The array item is selected according to these rules:
    ///   \li Look for an exact match with the specific language.
    ///   \li If a generic language is given, look for a partial match.
    ///   \li Look for an x-default item.
    ///   \li Choose the first item.
    ///
    /// A partial match with the generic language is where the start of the item's language matches
    /// the generic string and the next character is '-'. An exact match is also recognized as a
    /// degenerate case.
    ///
    /// You can pass "x-default" as the specific language. In this case, selection of an \c x-default
    /// item is an exact match by the first rule, not a selection by the 3rd rule. The last 2 rules
    /// are fallbacks used when the specific and generic languages fail to produce a match.
    ///
    /// Item values are modified according to these rules:
    ///
    ///   \li If the selected item is from a match with the specific language, the value of that
    ///   item is modified. If the existing value of that item matches the existing value of the
    ///   x-default item, the x-default item is also modified. If the array only has 1 existing item
    ///   (which is not x-default), an x-default item is added with the given value.
    ///
    ///   \li If the selected item is from a match with the generic language and there are no other
    ///   generic matches, the value of that item is modified. If the existing value of that item
    ///   matches the existing value of the x-default item, the x-default item is also modified. If
    ///   the array only has 1 existing item (which is not x-default), an x-default item is added
    ///   with the given value.
    ///
    ///   \li If the selected item is from a partial match with the generic language and there are
    ///   other partial matches, a new item is created for the specific language. The x-default item
    ///   is not modified.
    ///
    ///   \li If the selected item is from the last 2 rules then a new item is created for the
    ///   specific language. If the array only had an x-default item, the x-default item is also
    ///   modified. If the array was empty, items are created for the specific language and
    ///   x-default.
    ///
    /// @param schemaNS The namespace URI for the alt-text array; see \c GetProperty().
    ///
    /// @param altTextName The name of the alt-text array. Can be a general path expression, must
    /// not be null or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @param genericLang The name of the generic language as an RFC 3066 primary subtag. Can be
    /// null or the empty string if no generic language is wanted.
    ///
    /// @param specificLang The name of the specific language as an RFC 3066 tag, or "x-default".
    /// Must not be null or the empty string.
    ///
    /// @param itemValue The new value for the matching array item, specified as a string object.
    ///
    /// @param options Option flags, none currently defined.

    void SetLocalizedText ( XMP_StringPtr      schemaNS,
						    XMP_StringPtr      altTextName,
						    XMP_StringPtr      genericLang,
						    XMP_StringPtr      specificLang,
						    const tStringObj & itemValue,
						    XMP_OptionBits     options = 0 );

    /// @}

   	// =============================================================================================
    /// \name Creating and reading serialized RDF.
    /// @{
    ///
	/// The metadata contained in an XMP object must be serialized as RDF for storage in an XMP
	/// packet and output to a file. Similarly, metadata in the form of serialized RDF (such as
	/// metadata read from a file using \c TXMPFiles) must be parsed into an XMP object for
	/// manipulation with the XMP Toolkit.
	///
	/// These functions support parsing serialized RDF into an XMP object, and serializing an XMP
    /// object into RDF. The input for parsing can be any valid Unicode encoding. ISO Latin-1 is
    /// also recognized, but its use is strongly discouraged. Serialization is always as UTF-8.

    // ---------------------------------------------------------------------------------------------
    /// @brief \c ParseFromBuffer() parses RDF from a series of input buffers into this XMP object.
    ///
    /// Use this to convert metadata from serialized RDF form (as, for example, read from an XMP
    /// packet embedded in a file) into an XMP object that you can manipulate with the XMP Toolkit.
    /// If this XMP object is empty and the input buffer contains a complete XMP packet, this is the
    /// same as creating a new XMP object from that buffer with the constructor.
    ///
    /// You can use this function to combine multiple buffers into a single metadata tree. To
    /// terminate an input loop conveniently, pass  the option \c #kXMP_ParseMoreBuffers for all
    /// real input, then make a final call with a zero length and \c #kXMP_NoOptions. The buffers
    /// can be any length. The buffer boundaries need not respect XML tokens or even Unicode
    /// characters.
    ///
    /// @param buffer A pointer to a buffer of input. Can be null if \c bufferSize is 0.
    ///
    /// @param bufferSize The length of the input buffer in bytes. Zero is a valid value.
    ///
    /// @param options An options flag that controls how the parse operation is performed. A logical
    /// OR of these bit-flag constants:
    ///   \li \c #kXMP_ParseMoreBuffers - This is not the last buffer of input, more calls follow.
    ///   \li \c #kXMP_RequireXMPMeta - The \c x:xmpmeta XML element is required around \c rdf:RDF.
    ///
    /// @see \c TXMPFiles::GetXMP()

    void ParseFromBuffer ( XMP_StringPtr  buffer,
						   XMP_StringLen  bufferSize,
						   XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SerializeToBuffer() serializes metadata in this XMP object into a string as RDF.
    ///
    /// Use this to prepare metadata for storage as an XMP packet embedded in a file. See \c TXMPFiles::PutXMP().
    ///
    /// @param rdfString [out] A string object in which to return the serialized RDF. Must not be null.
    ///
    /// @param options An options flag that controls how the serialization operation is performed.
    /// The specified options must be logically consistent; an exception is thrown if they are not.
    /// A logical OR of these bit-flag constants:
    ///   \li \c kXMP_OmitPacketWrapper - Do not include an XML packet wrapper. This cannot be
    ///   specified together with \c #kXMP_ReadOnlyPacket, \c #kXMP_IncludeThumbnailPad, or
    ///   \c #kXMP_ExactPacketLength.
    ///   \li \c kXMP_ReadOnlyPacket - Create a read-only XML packet wapper. Cannot be specified
    ///   together with \c kXMP_OmitPacketWrapper.
    ///   \li \c kXMP_UseCompactFormat - Use a highly compact RDF syntax and layout.
    ///   \li \c kXMP_WriteAliasComments - Include XML comments for aliases.
    ///   \li \c kXMP_IncludeThumbnailPad - Include typical space for a JPEG thumbnail in the
    ///   padding if no \c xmp:Thumbnails property is present. Cannot be specified together with
    ///   \c kXMP_OmitPacketWrapper.
    ///   \li \c kXMP_ExactPacketLength - The padding parameter provides the overall packet length.
    ///   The actual amount of padding is computed. An exception is thrown if the packet exceeds
    ///   this length with no padding.	Cannot be specified together with
    ///   \c kXMP_OmitPacketWrapper.
    ///
    /// In addition to the above options, you can include one of the following encoding options:
    ///   \li \c #kXMP_EncodeUTF8 - Encode as UTF-8, the default.
    ///   \li \c #kXMP_EncodeUTF16Big - Encode as big-endian UTF-16.
    ///   \li \c #kXMP_EncodeUTF16Little - Encode as little-endian UTF-16.
    ///   \li \c #kXMP_EncodeUTF32Big - Encode as big-endian UTF-32.
    ///   \li \c #kXMP_EncodeUTF32Little - Encode as little-endian UTF-32.
    ///
    /// @param padding The amount of padding to be added if a writeable XML packet is created. If
    /// zero (the default) an appropriate amount of padding is computed.
    ///
    /// @param newline The string to be used as a line terminator. If empty, defaults to linefeed,
    /// U+000A, the standard XML newline.
    ///
    /// @param indent The string to be used for each level of indentation in the serialized RDF. If
    /// empty, defaults to two ASCII spaces, U+0020.
    ///
    /// @param baseIndent The number of levels of indentation to be used for the outermost XML
    /// element in the serialized RDF. This is convenient when embedding the RDF in other text.

    void SerializeToBuffer ( tStringObj *   rdfString,
							 XMP_OptionBits options,
							 XMP_StringLen  padding,
							 XMP_StringPtr  newline,
							 XMP_StringPtr  indent = "",
							 XMP_Index      baseIndent = 0 ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SerializeToBuffer() serializes metadata in this XMP object into a string as RDF.
    ///
    /// This simpler form of the function uses default values for the \c newline, \c indent, and
    /// \c baseIndent parameters.
    ///
    /// @param rdfString [out] A string object in which to return the serialized RDF. Must not be null.
    ///
    /// @param options An options flag that controls how the serialization operation is performed.
    /// The specified options must be logically consistent; an exception is thrown if they are not.
    /// A logical OR of these bit-flag constants:
    ///   \li \c kXMP_OmitPacketWrapper - Do not include an XML packet wrapper. This cannot be
    ///   specified together with \c #kXMP_ReadOnlyPacket, \c #kXMP_IncludeThumbnailPad, or
    ///   \c #kXMP_ExactPacketLength.
    ///   \li \c kXMP_ReadOnlyPacket - Create a read-only XML packet wapper. Cannot be specified
    ///   together with \c kXMP_OmitPacketWrapper.
    ///   \li \c kXMP_UseCompactFormat - Use a highly compact RDF syntax and layout.
    ///   \li \c kXMP_WriteAliasComments - Include XML comments for aliases.
    ///   \li \c kXMP_IncludeThumbnailPad - Include typical space for a JPEG thumbnail in the
    ///   padding if no \c xmp:Thumbnails property is present. Cannot be specified together with
    ///   \c kXMP_OmitPacketWrapper.
    ///   \li \c kXMP_ExactPacketLength - The padding parameter provides the overall packet length.
    ///   The actual amount of padding is computed. An exception is thrown if the packet exceeds
    ///   this length with no padding.	Cannot be specified together with
    ///   \c kXMP_OmitPacketWrapper.
    ///
    /// In addition to the above options, you can include one of the following encoding options:
    ///   \li \c #kXMP_EncodeUTF8 - Encode as UTF-8, the default.
    ///   \li \c #kXMP_EncodeUTF16Big - Encode as big-endian UTF-16.
    ///   \li \c #kXMP_EncodeUTF16Little - Encode as little-endian UTF-16.
    ///   \li \c #kXMP_EncodeUTF32Big - Encode as big-endian UTF-32.
    ///   \li \c #kXMP_EncodeUTF32Little - Encode as little-endian UTF-32.
    ///
    /// @param padding The amount of padding to be added if a writeable XML packet is created.
    /// If zero (the default) an appropriate amount of padding is computed.

    void SerializeToBuffer ( tStringObj *   rdfString,
							 XMP_OptionBits options = 0,
							 XMP_StringLen  padding = 0 ) const;

    /// @}
    // =============================================================================================
    // Miscellaneous Member Functions
    // ==============================

    // ---------------------------------------------------------------------------------------------
    /// \name Helper functions.
    /// @{

    // ---------------------------------------------------------------------------------------------
    /// @brief Retrieves an internal reference that can be safely passed across DLL boundaries and
    /// reconstructed.
    ///
    /// The \c TXMPMeta class is a normal C++ template, it is instantiated and local to each client
    /// executable, as are the other \c TXMP* classes. Different clients might not use the same
    /// string type to instantiate \c TXMPMeta.
    ///
    /// Because of this you should not pass \c SXMPMeta objects, or pointers to \c SXMPMeta objects,
    /// across DLL boundaries. Use this function to obtain a safe internal reference that you can
    /// pass, then construct a local object on the callee side. This construction does not create a
    /// cloned XMP tree, it is the same underlying XMP object safely wrapped in each client's
    /// \c SXMPMeta object.
    ///
    /// Use this function and the associated constructor like this:
    ///   \li The callee's header contains:
    /// <pre>
    /// CalleeMethod ( XMPMetaRef xmpRef );
    /// </pre>
    ///
    ///   \li The caller's code contains:
    /// <pre>
    /// SXMPMeta callerXMP;
    /// CalleeMethod ( callerXMP.GetInternalRef() );
    /// </pre>
    ///
    ///   \li The callee's code contains:
    /// <pre>
    /// SXMPMeta calleeXMP ( xmpRef );
    /// </pre>
    ///
    /// @return The reference object.

    XMPMetaRef GetInternalRef() const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c GetObjectName() retrieves the client-assigned name of this XMP object.
    ///
    /// Assign this name with \c SetObjectName().
    ///
    /// @param name [out] A string object in which to return the name.

    void GetObjectName ( tStringObj * name ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetObjectName() assigns a name to this XMP object.
    ///
    /// Retrieve this client-assigned name with \c GetObjectName().
    ///
    /// @param name The name as a null-terminated UTF-8 string.

    void SetObjectName ( XMP_StringPtr name );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c SetObjectName() assigns a name to this XMP object.
    ///
    /// Retrieve this client-assigned name with \c GetObjectName().
    ///
    /// @param name The name as a string object.

    void SetObjectName ( tStringObj name );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c Sort() sorts the data model tree of an XMP object.
    ///
    /// Use this function to sort the data model of an XMP object into a canonical order. This can
    /// be convenient when comparing data models, (e.g. by text comparison of DumpObject output).
    ///
    /// At the top level the namespaces are sorted by their prefixes. Within a namespace, the top
    /// level properties are sorted by name. Within a struct, the fields are sorted by their
    /// qualified name, i.e. their XML prefix:local form. Unordered arrays of simple items are
    /// sorted by value. Language Alternative arrays are sorted by the xml:lang qualifiers, with
    /// the "x-default" item placed first.
    
    void Sort();

    // ---------------------------------------------------------------------------------------------
    /// @brief \c Erase() restores the object to a "just constructed" state.
    
    void Erase();
    
    // ---------------------------------------------------------------------------------------------
    /// @brief \c Clone() creates a deep copy of an XMP object.
    ///
    /// Use this function to copy an entire XMP metadata tree. Assignment and copy constructors only
    /// increment a reference count, they do not do a deep copy. This function returns an object,
    /// not a pointer. The following shows correct usage:
    ///
    /// <pre>
    /// SXMPMeta * clone1 = new SXMPMeta ( sourceXMP.Clone() );  // This works.
    /// SXMPMeta   clone2 ( sourceXMP.Clone );  	// This works also. (Not a pointer.)
    /// </pre>
    /// The \c clone2 example does not use an explicit pointer.
    /// This is good for local usage, protecting against memory leaks.
    ///
    /// This is an example of incorrect usage:
    /// <pre>
    /// SXMPMeta * clone3 = &sourceXMP.Clone();		// ! This does not work!
    /// </pre>
    /// The assignment to \c clone3 creates a temporary object, initializes it with the clone,
    /// assigns the address of the temporary to \c clone3, then deletes the temporary.
    ///
    /// @param options Option flags, not currently defined..
    ///
    /// @return An XMP object cloned from the original.

    TXMPMeta Clone ( XMP_OptionBits options = 0 ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c CountArrayItems() reports the number of items currently defined in an array.
    ///
    /// @param schemaNS The namespace URI; see \c GetProperty().
    ///
    /// @param arrayName The name of the array. Can be a general path expression, must not be null
    /// or the empty string; see \c GetProperty() for namespace prefix usage.
    ///
    /// @return The number of items.

    XMP_Index CountArrayItems ( XMP_StringPtr schemaNS,
                      			XMP_StringPtr arrayName ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief \c DumpObject() outputs the content of an XMP object to a callback handler for debugging.
    ///
    /// Invokes a client-defined callback for each line of output.
    ///
    /// @param outProc The client-defined procedure to handle each line of output.
    ///
    /// @param clientData A pointer to client-defined data to pass to the handler.
    ///
    /// @return	A success-fail status value, returned from the handler. Zero is success, failure
    /// values are client-defined.
    ///
    /// @see Static functions \c DumpNamespaces() and \c DumpAliases()

    XMP_Status DumpObject ( XMP_TextOutputProc outProc,
                 			void *	           clientData ) const;

    // ---------------------------------------------------------------------------------------------
    /// @brief Not implemented
    XMP_OptionBits GetObjectOptions() const;

    // ---------------------------------------------------------------------------------------------
    /// \brief Not implemented
    void SetObjectOptions ( XMP_OptionBits options );

    /// @}

    // =============================================================================================

    XMPMetaRef xmpRef;  // *** Should be private, see below.

private:

#if 0	// *** VS.Net and gcc seem to not handle the friend declarations properly.
    friend class TXMPIterator <class tStringObj>;
    friend class TXMPUtils <class tStringObj>;
#endif

};  // class TXMPMeta

#endif  // __TXMPMeta_hpp__
