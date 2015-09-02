#ifndef __TXMPIterator_hpp__
#define __TXMPIterator_hpp__ 1

#if ( ! __XMP_hpp__ )
    #error "Do not directly include, use XMPSDK.hpp"
#endif

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

// =================================================================================================
/// \file TXMPIterator.hpp
/// \brief API for access to the XMP Toolkit iteration services.
///
/// \c TXMPIterator is the template class providing iteration services for the XMP Toolkit. It must
/// be instantiated with a string class such as \c std::string. See the instructions in XMPSDK.hpp, and
/// the Overview for a discussion of the overall architecture of the XMP API.
// =================================================================================================

// =================================================================================================
/// \class TXMPIterator TXMPIterator.hpp
/// @brief API for access to the XMP Toolkit iteration services.
///
/// \c TXMPIterator provides a uniform means to iterate over the schema and properties within an XMP
/// object. \c TXMPIterator is a template class which must be instantiated with a string class such
/// as \c std::string. See the instructions in XMPSDK.hpp, and the Overview for a discussion of the
/// overall architecture of the XMP API. Access these functions through the concrete class,
/// \c SXMPIterator.
///
/// @note Only XMP object iteration is currently available. Future development may include iteration
/// over global tables, such as registered namespaces.
///
/// To understand how iteration works, you should have a thorough understanding of the XMP data
/// tree, as described in the XMP Specification Part 1. You might also find it helpful to create
/// some complex XMP and examine the output of \c TXMPMeta::DumpObject().
///
///   \li The top of the XMP data tree is a single root node. This does not explicitly appear in the
///   dump and is never visited by an iterator; that is, it is never returned from
///   \c TXMPIterator::Next().
///
///   \li Beneath the root are schema nodes; these collect the top-level properties in the same
///   namespace. They are created and destroyed implicitly.
///
///   \li Beneath the schema nodes are the property nodes. The nodes below a property node depend on
///   its type (simple, struct, or array) and whether it has qualifiers.
///
/// A \c TXMPIterator constructor defines a starting point for the iteration, and options that
/// control how it proceeds. By default, iteration starts at the root and visits all nodes beneath
/// it in a depth-first manner. The root node iteself is not visited; the first visited node is a
/// schema node. You can provide a schema name or property path to select a different starting node.
/// By default, this visits the named root node first then all nodes beneath it in a depth-first
/// manner.
///
/// The function \c TXMPIterator::Next() delivers the schema URI, path, and option flags for the
/// node being visited. If the node is simple, it also delivers the value. Qualifiers for this node
/// are visited next. The fields of a struct or items of an array are visited after the qualifiers
/// of the parent.
///
/// You can specify options when constructing the iteration object to control how the iteration is
/// performed.
///
///   \li \c #kXMP_IterJustChildren - Visit just the immediate children of the root. Skip the root
///   itself and all nodes below the immediate children. This omits the qualifiers of the immediate
///   children, the qualifier nodes being below what they qualify.
///   \li \c #kXMP_IterJustLeafNodes - Visit just the leaf property nodes and their qualifiers.
///   \li \c #kXMP_IterJustLeafName - Return just the leaf component of the node names. The default
///   is to return the full path name.
///   \li \c #kXMP_IterIncludeAliases - Include aliases as part of the iteration. Since aliases are
///   not actual nodes the default iteration does not visit them.
///   \li \c #kXMP_IterOmitQualifiers - Do not visit the qualifiers of a node.
// =================================================================================================

#include "client-glue/WXMPIterator.hpp"

template <class tStringObj> class TXMPIterator {

public:

    // ---------------------------------------------------------------------------------------------
    /// @brief Assignment operator, assigns the internal ref and increments the ref count.
    ///
    /// Assigns the internal reference from an existing object and increments the reference count on
    /// the underlying internal XMP iterator.
    ///
    /// @param rhs An existing iteration object.

    void operator= ( const TXMPIterator<tStringObj> & rhs );

    // ---------------------------------------------------------------------------------------------
    /// @brief Copy constructor, creates a client object refering to the same internal object.
    ///
    /// Creates a new client iterator that refers to the same underlying iterator as an existing object.
    ///
    /// @param original An existing iteration object to copy.

    TXMPIterator ( const TXMPIterator<tStringObj> & original );

    // ---------------------------------------------------------------------------------------------
    /// @brief Constructs an iterator for properties within a schema in an XMP object.
    ///
    /// See the class description for the general operation of an  XMP object iterator.
    /// Overloaded forms are provided to iterate the entire data tree,
    /// a subtree rooted at a specific node, or properties within a specific schema.
    ///
    /// @param xmpObj The XMP object over which to iterate.
    ///
    /// @param schemaNS Optional schema namespace URI to restrict the iteration. To visit all of the
    /// schema, pass 0 or the empty string "".
    ///
    /// @param propName Optional property name to restrict the iteration. May be an arbitrary path
    /// expression. If provided, a schema URI must also be provided. To visit all properties, pass 0
    /// or the empty string "".
    ///
    /// @param options Option flags to control the iteration. A logical OR of these bit flag constants:
    ///   \li \c #kXMP_IterJustChildren - Visit only the immediate children of the root; default visits subtrees.
    ///   \li \c #kXMP_IterJustLeafNodes - Visit only the leaf nodes; default visits all nodes.
    ///   \li \c #kXMP_IterJustLeafName - Return just the leaf part of the path; default returns the full path.
    ///   \li \c #kXMP_IterOmitQualifiers - Omit all qualifiers.
    ///
    /// @return The new TXMPIterator object.

    TXMPIterator ( const TXMPMeta<tStringObj> & xmpObj,
                   XMP_StringPtr  schemaNS,
                   XMP_StringPtr  propName,
                   XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief Constructs an iterator for a subtree of properties within an XMP object.
    ///
    /// See the class description for the general operation of an  XMP object iterator. Overloaded
    /// forms are provided to iterate the entire data tree, a subtree rooted at a specific node, or
    /// properties within a specific schema.
    ///
    /// @param xmpObj The XMP object over which to iterate.
    ///
    /// @param schemaNS Optional schema namespace URI to restrict the iteration. To visit all of the
    /// schema, pass 0 or the empty string "".
    ///
    /// @param options Option flags to control the iteration. A logical OR of these bit flag constants:
    ///   \li \c #kXMP_IterJustChildren - Visit only the immediate children of the root; default visits subtrees.
    ///   \li \c #kXMP_IterJustLeafNodes - Visit only the leaf nodes; default visits all nodes.
    ///   \li \c #kXMP_IterJustLeafName - Return just the leaf part of the path; default returns the full path.
    ///   \li \c #kXMP_IterOmitQualifiers - Omit all qualifiers.
    ///
    /// @return The new TXMPIterator object.

    TXMPIterator ( const TXMPMeta<tStringObj> & xmpObj,
                   XMP_StringPtr  schemaNS,
                   XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief Constructs an iterator for the entire data tree within an XMP object.
    ///
    /// See the class description for the general operation of an  XMP object iterator. Overloaded
    /// forms are provided to iterate the entire data tree, a subtree rooted at a specific node, or
    /// properties within a specific schema.
    ///
    /// @param xmpObj The XMP object over which to iterate.
    ///
    /// @param options Option flags to control the iteration. A logical OR of these bit flag constants:
    ///   \li \c #kXMP_IterJustChildren - Visit only the immediate children of the root; default visits subtrees.
    ///   \li \c #kXMP_IterJustLeafNodes - Visit only the leaf nodes; default visits all nodes.
    ///   \li \c #kXMP_IterJustLeafName - Return just the leaf part of the path; default returns the full path.
    ///   \li \c #kXMP_IterOmitQualifiers - Omit all qualifiers.
    ///
    /// @return The new \c TXMPIterator object.

    TXMPIterator ( const TXMPMeta<tStringObj> & xmpObj,
                   XMP_OptionBits options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief Constructs an iterator for the global tables of the XMP toolkit.	Not implemented.

    TXMPIterator ( XMP_StringPtr  schemaNS,
                   XMP_StringPtr  propName,
                   XMP_OptionBits options );

    // ---------------------------------------------------------------------------------------------
    /// @brief Destructor, typical virtual destructor.

    virtual ~TXMPIterator() throw();

    // ---------------------------------------------------------------------------------------------
    /// @brief \c Next() visits the next node in the iteration.
    ///
    /// Proceeds to the next node according to the options specified on creation of this object, and
    /// delivers the schema URI, path, and option flags for the node being visited. If the node is
    /// simple, it also delivers the value.
    ///
    /// @param schemaNS [out] A string object in which to return the assigned the schema namespace
    /// URI of the current property. Can be null if the value is not wanted.
    ///
    /// @param propPath [out]  A string object in which to return the XPath name of the current
    /// property. Can be null if the value is not wanted.
    ///
    /// @param propValue  [out] A string object in which to return the value of the current
    /// property. Can be null if the value is not wanted.
    ///
    /// @param options  [out] A buffer in which to return the flags describing the current property,
    /// which are a logical OR of \c #XMP_OptionBits bit-flag constants.
    ///
    /// @return True if there was another node to visit, false if the iteration is complete.

    bool Next ( tStringObj *     schemaNS = 0,
          		tStringObj *     propPath = 0,
           		tStringObj *     propValue = 0,
           		XMP_OptionBits * options = 0 );

    // ---------------------------------------------------------------------------------------------
    /// @brief \c Skip() skips some portion of the remaining iterations.
    ///
    /// @param options Option flags to control the iteration, a logical OR of these bit-flag
    /// constants:
    ///   \li \c #kXMP_IterSkipSubtree -  Skip the subtree below the current node.
    ///   \li \c #kXMP_IterSkipSiblings - Skip the subtree below and remaining siblings of the current node.

    void Skip ( XMP_OptionBits options );

private:

    XMPIteratorRef  iterRef;

    TXMPIterator();	// ! Hidden, must choose property or table iteration.

};  // class TXMPIterator

// =================================================================================================

#endif // __TXMPIterator_hpp__
