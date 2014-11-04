// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"
#include "ExpatAdapter.hpp"

#include <cstring>

#if DEBUG
	#include <iostream>
#endif

using namespace std;

#if XMP_WinBuild
#   ifdef _MSC_VER
        #pragma warning ( disable : 4189 )	// local variable is initialized but not referenced
        #pragma warning ( disable : 4505 )	// unreferenced local function has been removed
#   endif
#endif

// =================================================================================================

// *** This might be faster and use less memory as a state machine. A big advantage of building an
// *** XML tree though is easy lookahead during the recursive descent processing.

// *** It would be nice to give a line number or byte offset in the exception messages.


// 7 RDF/XML Grammar (from http://www.w3.org/TR/rdf-syntax-grammar/#section-Infoset-Grammar)
//
// 7.1 Grammar summary
//
// 7.2.2 coreSyntaxTerms
//		rdf:RDF | rdf:ID | rdf:about | rdf:parseType | rdf:resource | rdf:nodeID | rdf:datatype
//
// 7.2.3 syntaxTerms
//		coreSyntaxTerms | rdf:Description | rdf:li
//
// 7.2.4 oldTerms
//		rdf:aboutEach | rdf:aboutEachPrefix | rdf:bagID
//
// 7.2.5 nodeElementURIs
//		anyURI - ( coreSyntaxTerms | rdf:li | oldTerms )
//
// 7.2.6 propertyElementURIs
//		anyURI - ( coreSyntaxTerms | rdf:Description | oldTerms )
//
// 7.2.7 propertyAttributeURIs
//		anyURI - ( coreSyntaxTerms | rdf:Description | rdf:li | oldTerms )
//
// 7.2.8 doc
//		root ( document-element == RDF, children == list ( RDF ) )
//
// 7.2.9 RDF
//		start-element ( URI == rdf:RDF, attributes == set() )
//		nodeElementList
//		end-element()
//
// 7.2.10 nodeElementList
//		ws* ( nodeElement ws* )*
//
// 7.2.11 nodeElement
//		start-element ( URI == nodeElementURIs,
//						attributes == set ( ( idAttr | nodeIdAttr | aboutAttr )?, propertyAttr* ) )
//		propertyEltList
//		end-element()
//
// 7.2.12 ws
//		A text event matching white space defined by [XML] definition White Space Rule [3] S in section Common Syntactic Constructs.
//
// 7.2.13 propertyEltList
//		ws* ( propertyElt ws* )*
//
// 7.2.14 propertyElt
//		resourcePropertyElt | literalPropertyElt | parseTypeLiteralPropertyElt |
//		parseTypeResourcePropertyElt | parseTypeCollectionPropertyElt | parseTypeOtherPropertyElt | emptyPropertyElt
//
// 7.2.15 resourcePropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr? ) )
//		ws* nodeElement ws*
//		end-element()
//
// 7.2.16 literalPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, datatypeAttr?) )
//		text()
//		end-element()
//
// 7.2.17 parseTypeLiteralPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseLiteral ) )
//		literal
//		end-element()
//
// 7.2.18 parseTypeResourcePropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseResource ) )
//		propertyEltList
//		end-element()
//
// 7.2.19 parseTypeCollectionPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseCollection ) )
//		nodeElementList
//		end-element()
//
// 7.2.20 parseTypeOtherPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseOther ) )
//		propertyEltList
//		end-element()
//
// 7.2.21 emptyPropertyElt
//		start-element ( URI == propertyElementURIs,
//						attributes == set ( idAttr?, ( resourceAttr | nodeIdAttr )?, propertyAttr* ) )
//		end-element()
//
// 7.2.22 idAttr
//		attribute ( URI == rdf:ID, string-value == rdf-id )
//
// 7.2.23 nodeIdAttr
//		attribute ( URI == rdf:nodeID, string-value == rdf-id )
//
// 7.2.24 aboutAttr
//		attribute ( URI == rdf:about, string-value == URI-reference )
//
// 7.2.25 propertyAttr
//		attribute ( URI == propertyAttributeURIs, string-value == anyString )
//
// 7.2.26 resourceAttr
//		attribute ( URI == rdf:resource, string-value == URI-reference )
//
// 7.2.27 datatypeAttr
//		attribute ( URI == rdf:datatype, string-value == URI-reference )
//
// 7.2.28 parseLiteral
//		attribute ( URI == rdf:parseType, string-value == "Literal")
//
// 7.2.29 parseResource
//		attribute ( URI == rdf:parseType, string-value == "Resource")
//
// 7.2.30 parseCollection
//		attribute ( URI == rdf:parseType, string-value == "Collection")
//
// 7.2.31 parseOther
//		attribute ( URI == rdf:parseType, string-value == anyString - ("Resource" | "Literal" | "Collection") )
//
// 7.2.32 URI-reference
//		An RDF URI Reference.
//
// 7.2.33 literal
//		Any XML element content that is allowed according to [XML] definition Content of Elements Rule [43] content
//		in section 3.1 Start-Tags, End-Tags, and Empty-Element Tags.
//
// 7.2.34 rdf-id
//		An attribute string-value matching any legal [XML-NS] token NCName.


// =================================================================================================
// Primary Parsing Functions
// =========================
//
// Each of these is responsible for recognizing an RDF syntax production and adding the appropriate
// structure to the XMP tree. They simply return for success, failures will throw an exception.

static void
RDF_RDF ( XMP_Node * xmpTree, const XML_Node & xmlNode );

static void
RDF_NodeElementList ( XMP_Node * xmpParent, const XML_Node & xmlParent, bool isTopLevel );

static void
RDF_NodeElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_NodeElementAttrs ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_PropertyElementList ( XMP_Node * xmpParent, const XML_Node & xmlParent, bool isTopLevel );
enum { kIsTopLevel = true, kNotTopLevel = false };

static void
RDF_PropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_ResourcePropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_LiteralPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_ParseTypeLiteralPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_ParseTypeResourcePropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_ParseTypeCollectionPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_ParseTypeOtherPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );

static void
RDF_EmptyPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel );


// =================================================================================================

typedef XMP_Uns8 RDFTermKind;

// *** Logic might be safer with just masks.

enum {
	kRDFTerm_Other				=  0,
	kRDFTerm_RDF				=  1,	// Start of coreSyntaxTerms.
	kRDFTerm_ID					=  2,
	kRDFTerm_about				=  3,
	kRDFTerm_parseType			=  4,
	kRDFTerm_resource			=  5,
	kRDFTerm_nodeID				=  6,
	kRDFTerm_datatype			=  7,	// End of coreSyntaxTerms.
	kRDFTerm_Description		=  8,	// Start of additions for syntaxTerms.
	kRDFTerm_li					=  9,	// End of of additions for syntaxTerms.
	kRDFTerm_aboutEach			= 10,	// Start of oldTerms.
	kRDFTerm_aboutEachPrefix	= 11,
	kRDFTerm_bagID				= 12,	// End of oldTerms.
	
	kRDFTerm_FirstCore          = kRDFTerm_RDF,
	kRDFTerm_LastCore           = kRDFTerm_datatype,
	kRDFTerm_FirstSyntax        = kRDFTerm_FirstCore,	// ! Yes, the syntax terms include the core terms.
	kRDFTerm_LastSyntax         = kRDFTerm_li,
	kRDFTerm_FirstOld           = kRDFTerm_aboutEach,
	kRDFTerm_LastOld            = kRDFTerm_bagID
};

enum {
	kRDFMask_Other				= 1 << kRDFTerm_Other,
	kRDFMask_RDF				= 1 << kRDFTerm_RDF,
	kRDFMask_ID					= 1 << kRDFTerm_ID,
	kRDFMask_about				= 1 << kRDFTerm_about,
	kRDFMask_parseType			= 1 << kRDFTerm_parseType,
	kRDFMask_resource			= 1 << kRDFTerm_resource,
	kRDFMask_nodeID				= 1 << kRDFTerm_nodeID,
	kRDFMask_datatype			= 1 << kRDFTerm_datatype,
	kRDFMask_Description		= 1 << kRDFTerm_Description,
	kRDFMask_li					= 1 << kRDFTerm_li,
	kRDFMask_aboutEach			= 1 << kRDFTerm_aboutEach,
	kRDFMask_aboutEachPrefix	= 1 << kRDFTerm_aboutEachPrefix,
	kRDFMask_bagID				= 1 << kRDFTerm_bagID
};

enum {
	kRDF_HasValueElem = 0x10000000UL	// ! Contains rdf:value child. Must fit within kXMP_ImplReservedMask!
};

// -------------------------------------------------------------------------------------------------
// GetRDFTermKind
// --------------

static RDFTermKind
GetRDFTermKind ( const XMP_VarString & name )
{
	RDFTermKind term = kRDFTerm_Other;

	// Arranged to hopefully minimize the parse time for large XMP.

	if ( (name.size() > 4) && (strncmp ( name.c_str(), "rdf:", 4 ) == 0) ) {

		if ( name == "rdf:li" ) {
			term = kRDFTerm_li;
		} else if ( name == "rdf:parseType" ) {
			term = kRDFTerm_parseType;
		} else if ( name == "rdf:Description" ) {
			term = kRDFTerm_Description;
		} else if ( name == "rdf:about" ) {
			term = kRDFTerm_about;
		} else if ( name == "rdf:resource" ) {
			term = kRDFTerm_resource;
		} else if ( name == "rdf:RDF" ) {
			term = kRDFTerm_RDF;
		} else if ( name == "rdf:ID" ) {
			term = kRDFTerm_ID;
		} else if ( name == "rdf:nodeID" ) {
			term = kRDFTerm_nodeID;
		} else if ( name == "rdf:datatype" ) {
			term = kRDFTerm_datatype;
		} else if ( name == "rdf:aboutEach" ) {
			term = kRDFTerm_aboutEach;
		} else if ( name == "rdf:aboutEachPrefix" ) {
			term = kRDFTerm_aboutEachPrefix;
		} else if ( name == "rdf:bagID" ) {
			term = kRDFTerm_bagID;
		}

	}

	return term;

}	// GetRDFTermKind


// =================================================================================================


// -------------------------------------------------------------------------------------------------
// IsCoreSyntaxTerm
// ----------------
//
// 7.2.2 coreSyntaxTerms
//		rdf:RDF | rdf:ID | rdf:about | rdf:parseType | rdf:resource | rdf:nodeID | rdf:datatype

static bool
IsCoreSyntaxTerm ( RDFTermKind term )
{

	if 	( (kRDFTerm_FirstCore <= term) && (term <= kRDFTerm_LastCore) ) return true;
	return false;

}	// IsCoreSyntaxTerm


// -------------------------------------------------------------------------------------------------
// IsOldTerm
// ---------
//
// 7.2.4 oldTerms
//		rdf:aboutEach | rdf:aboutEachPrefix | rdf:bagID

static bool
IsOldTerm ( RDFTermKind term )
{

	if 	( (kRDFTerm_FirstOld <= term) && (term <= kRDFTerm_LastOld) ) return true;
	return false;

}	// IsOldTerm

// -------------------------------------------------------------------------------------------------
// IsPropertyElementName
// ---------------------
//
// 7.2.6 propertyElementURIs
//		anyURI - ( coreSyntaxTerms | rdf:Description | oldTerms )

static bool
IsPropertyElementName ( RDFTermKind term )
{

	if 	( (term == kRDFTerm_Description) || IsOldTerm ( term ) ) return false;
	return (! IsCoreSyntaxTerm ( term ));

}	// IsPropertyElementName

// =================================================================================================
// AddChildNode
// ============

static XMP_Node *
AddChildNode ( XMP_Node * xmpParent, const XML_Node & xmlNode, const XMP_StringPtr value, bool isTopLevel )
{
	#if 0
		cout << "AddChildNode, parent = " << xmpParent->name << ", child = " << xmlNode.name;
		cout << ", value = \"" << value << '"';
		if ( isTopLevel ) cout << ", top level";
		cout << endl;
	#endif
	
	if ( xmlNode.ns.empty() ) {
		XMP_Throw ( "XML namespace required for all elements and attributes", kXMPErr_BadRDF );
	}
		
	XMP_StringPtr  childName    = xmlNode.name.c_str();
	const bool     isArrayItem  = (xmlNode.name == "rdf:li");
	const bool     isValueNode  = (xmlNode.name == "rdf:value");
	XMP_OptionBits childOptions = 0;
	
	if ( isTopLevel ) {

		// Lookup the schema node, adjust the XMP parent pointer.
		XMP_Assert ( xmpParent->parent == 0 );	// Incoming parent must be the tree root.
		XMP_Node * schemaNode = FindSchemaNode ( xmpParent, xmlNode.ns.c_str(), kXMP_CreateNodes );
		if ( schemaNode->options & kXMP_NewImplicitNode ) schemaNode->options ^= kXMP_NewImplicitNode;	// Clear the implicit node bit.
			// *** Should use "opt &= ~flag" (no conditional), need runtime check for proper 32 bit code.
		xmpParent = schemaNode;
		
		// If this is an alias set the isAlias flag in the node and the hasAliases flag in the tree.
		if ( sRegisteredAliasMap->find ( xmlNode.name ) != sRegisteredAliasMap->end() ) {
			childOptions |= kXMP_PropIsAlias;
			schemaNode->parent->options |= kXMP_PropHasAliases;
		}
		
	}

	// Make sure that this is not a duplicate of a named node.
	if ( ! (isArrayItem | isValueNode) ) {
		if ( FindChildNode ( xmpParent, childName, kXMP_ExistingOnly ) != 0 ) {
			XMP_Throw ( "Duplicate property or field node", kXMPErr_BadXMP );
		}
		
	}
	
	// Add the new child to the XMP parent node.
	XMP_Node * newChild = new XMP_Node ( xmpParent, childName, value, childOptions );
	if ( (! isValueNode) || xmpParent->children.empty() ) {
		 xmpParent->children.push_back ( newChild );
	} else {
		 xmpParent->children.insert ( xmpParent->children.begin(), newChild );
	}
	if ( isValueNode ) {
		if ( isTopLevel || (! (xmpParent->options & kXMP_PropValueIsStruct)) ) XMP_Throw ( "Misplaced rdf:value element", kXMPErr_BadRDF );
		xmpParent->options |= kRDF_HasValueElem;
	}
	
	if ( isArrayItem ) {
		if ( ! (xmpParent->options & kXMP_PropValueIsArray) ) XMP_Throw ( "Misplaced rdf:li element", kXMPErr_BadRDF );
		newChild->name = kXMP_ArrayItemName;
		#if 0	// *** XMP_DebugBuild
			newChild->_namePtr = newChild->name.c_str();
		#endif
	}
	
	return newChild;

}	// AddChildNode


// =================================================================================================
// AddQualifierNode
// ================

static XMP_Node *
AddQualifierNode ( XMP_Node * xmpParent, const XMP_VarString & name, const XMP_VarString & value )
{

	#if 0
		cout << "AddQualifierNode, parent = " << xmpParent->name << ", name = " << name;
		cout << ", value = \"" << value << '"' << endl;
	#endif
	
	const bool isLang = (name == "xml:lang");
	const bool isType = (name == "rdf:type");

	XMP_Node * newQual = 0;

		newQual = new XMP_Node ( xmpParent, name, value, kXMP_PropIsQualifier );

		if ( ! (isLang | isType) ) {
			xmpParent->qualifiers.push_back ( newQual );
		} else if ( isLang ) {
			if ( xmpParent->qualifiers.empty() ) {
				xmpParent->qualifiers.push_back ( newQual );
			} else {
				xmpParent->qualifiers.insert ( xmpParent->qualifiers.begin(), newQual );
			}
			xmpParent->options |= kXMP_PropHasLang;
		} else {
			XMP_Assert ( isType );
			if ( xmpParent->qualifiers.empty() ) {
				xmpParent->qualifiers.push_back ( newQual );
			} else {
				size_t offset = 0;
				if ( XMP_PropHasLang ( xmpParent->options ) ) offset = 1;
				xmpParent->qualifiers.insert ( xmpParent->qualifiers.begin()+offset, newQual );
			}
			xmpParent->options |= kXMP_PropHasType;
		}

		xmpParent->options |= kXMP_PropHasQualifiers;

	return newQual;

}	// AddQualifierNode


// =================================================================================================
// AddQualifierNode
// ================

static XMP_Node *
AddQualifierNode ( XMP_Node * xmpParent, const XML_Node & attr )
{
	if ( attr.ns.empty() ) {
		XMP_Throw ( "XML namespace required for all elements and attributes", kXMPErr_BadRDF );
	}
	
	return AddQualifierNode ( xmpParent, attr.name, attr.value );

}	// AddQualifierNode


// =================================================================================================
// FixupQualifiedNode
// ==================
//
// The parent is an RDF pseudo-struct containing an rdf:value field. Fix the XMP data model. The
// rdf:value node must be the first child, the other children are qualifiers. The form, value, and
// children of the rdf:value node are the real ones. The rdf:value node's qualifiers must be added
// to the others.
	
static void
FixupQualifiedNode ( XMP_Node * xmpParent )
{
	size_t qualNum, qualLim;
	size_t childNum, childLim;

	XMP_Enforce ( (xmpParent->options & kXMP_PropValueIsStruct) && (! xmpParent->children.empty()) );

	XMP_Node * valueNode = xmpParent->children[0];
	XMP_Enforce ( valueNode->name == "rdf:value" );
	
	xmpParent->qualifiers.reserve ( xmpParent->qualifiers.size() + xmpParent->children.size() + valueNode->qualifiers.size() );
	
	// Move the qualifiers on the value node to the parent. Make sure an xml:lang qualifier stays at
	// the front. Check for duplicate names between the value node's qualifiers and the parent's
	// children. The parent's children are about to become qualifiers. Check here, between the
	// groups. Intra-group duplicates are caught by AddChildNode.
	
	qualNum = 0;
	qualLim = valueNode->qualifiers.size();
	
	if ( valueNode->options & kXMP_PropHasLang ) {

		if ( xmpParent->options & kXMP_PropHasLang ) XMP_Throw ( "Redundant xml:lang for rdf:value element", kXMPErr_BadXMP );

		XMP_Node * langQual = valueNode->qualifiers[0];
		
		XMP_Assert ( langQual->name == "xml:lang" );
		langQual->parent = xmpParent;
		xmpParent->options |= kXMP_PropHasLang;

		if ( xmpParent->qualifiers.empty() ) {
			xmpParent->qualifiers.push_back ( langQual );	// *** Should use utilities to add qual & set parent.
		} else {
			xmpParent->qualifiers.insert ( xmpParent->qualifiers.begin(), langQual );
		}
		valueNode->qualifiers[0] = 0;	// We just moved it to the parent.

		qualNum = 1;	// Start the remaining copy after the xml:lang qualifier.

	}
	
	for ( ; qualNum != qualLim; ++qualNum ) {

		XMP_Node * currQual = valueNode->qualifiers[qualNum];
		if ( FindChildNode ( xmpParent, currQual->name.c_str(), kXMP_ExistingOnly ) != 0 ) {
			XMP_Throw ( "Duplicate qualifier node", kXMPErr_BadXMP );
		}

		currQual->parent = xmpParent;
		xmpParent->qualifiers.push_back ( currQual );
		valueNode->qualifiers[qualNum] = 0;	// We just moved it to the parent.

	}
	
	valueNode->qualifiers.clear();	// ! There should be nothing but null pointers.
	
	// Change the parent's other children into qualifiers. This loop starts at 1, child 0 is the
	// rdf:value node. Put xml:lang at the front, append all others.
	
	for ( childNum = 1, childLim = xmpParent->children.size(); childNum != childLim; ++childNum ) {

		XMP_Node * currQual = xmpParent->children[childNum];
	
			bool isLang = (currQual->name == "xml:lang");
			
			currQual->options |= kXMP_PropIsQualifier;
			currQual->parent = xmpParent;

			if ( isLang ) {
				if ( xmpParent->options & kXMP_PropHasLang ) XMP_Throw ( "Duplicate xml:lang qualifier", kXMPErr_BadXMP );
				xmpParent->options |= kXMP_PropHasLang;
			} else if ( currQual->name == "rdf:type" ) {
				xmpParent->options |= kXMP_PropHasType;
			}

			if ( (! isLang) || xmpParent->qualifiers.empty() ) {
				xmpParent->qualifiers.push_back ( currQual );
			} else {
				xmpParent->qualifiers.insert ( xmpParent->qualifiers.begin(), currQual );
			}
			xmpParent->children[childNum] = 0;	// We just moved it to the qualifers.
		
	}
	
	if ( ! xmpParent->qualifiers.empty() ) xmpParent->options |= kXMP_PropHasQualifiers;
	
	// Move the options and value last, other checks need the parent's original options. Move the
	// value node's children to be the parent's children. Delete the now useless value node.
	
	XMP_Assert ( xmpParent->options & (kXMP_PropValueIsStruct | kRDF_HasValueElem) );
	xmpParent->options &= ~ (kXMP_PropValueIsStruct | kRDF_HasValueElem);
	xmpParent->options |= valueNode->options;
	
	xmpParent->value.swap ( valueNode->value );
	#if 0	// *** XMP_DebugBuild
		xmpParent->_valuePtr = xmpParent->value.c_str();
	#endif

	xmpParent->children[0] = 0;	// ! Remove the value node itself before the swap.
	xmpParent->children.swap ( valueNode->children );
	
	for ( size_t childNum = 0, childLim = xmpParent->children.size(); childNum != childLim; ++childNum ) {
		XMP_Node * currChild = xmpParent->children[childNum];
		currChild->parent = xmpParent;
	}

	delete valueNode;
	
}	// FixupQualifiedNode


// =================================================================================================
// ProcessRDF
// ==========
//
// Parse the XML tree of the RDF and build the corresponding XMP tree.

// *** Throw an exception if no XMP is found? By option?
// *** Do parsing exceptions cause the partial tree to be deleted?

void ProcessRDF ( XMP_Node * xmpTree, const XML_Node & rdfNode, XMP_OptionBits options )
{
	IgnoreParam(options);
	
	RDF_RDF ( xmpTree, rdfNode );

}	// ProcessRDF


// =================================================================================================
// RDF_RDF
// =======
//
// 7.2.9 RDF
//		start-element ( URI == rdf:RDF, attributes == set() )
//		nodeElementList
//		end-element()
//
// The top level rdf:RDF node. It can only have xmlns attributes, which have already been removed
// during construction of the XML tree.

static void
RDF_RDF ( XMP_Node * xmpTree, const XML_Node & xmlNode )
{

	if ( ! xmlNode.attrs.empty() ) XMP_Throw ( "Invalid attributes of rdf:RDF element", kXMPErr_BadRDF );
	RDF_NodeElementList ( xmpTree, xmlNode, kIsTopLevel );

}	// RDF_RDF


// =================================================================================================
// RDF_NodeElementList
// ===================
//
// 7.2.10 nodeElementList
//		ws* ( nodeElement ws* )*

static void
RDF_NodeElementList ( XMP_Node * xmpParent, const XML_Node & xmlParent, bool isTopLevel )
{
	XMP_Assert ( isTopLevel );
	
	XML_cNodePos currChild = xmlParent.content.begin();	// *** Change these loops to the indexed pattern.
	XML_cNodePos endChild  = xmlParent.content.end();

	for ( ; currChild != endChild; ++currChild ) {
		if ( (*currChild)->IsWhitespaceNode() ) continue;
		RDF_NodeElement ( xmpParent, **currChild, isTopLevel );
	}

}	// RDF_NodeElementList


// =================================================================================================
// RDF_NodeElement
// ===============
//
// 7.2.5 nodeElementURIs
//		anyURI - ( coreSyntaxTerms | rdf:li | oldTerms )
//
// 7.2.11 nodeElement
//		start-element ( URI == nodeElementURIs,
//						attributes == set ( ( idAttr | nodeIdAttr | aboutAttr )?, propertyAttr* ) )
//		propertyEltList
//		end-element()
//
// A node element URI is rdf:Description or anything else that is not an RDF term.

static void
RDF_NodeElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	RDFTermKind nodeTerm = GetRDFTermKind ( xmlNode.name );
	if ( (nodeTerm != kRDFTerm_Description) && (nodeTerm != kRDFTerm_Other) ) {
		XMP_Throw ( "Node element must be rdf:Description or typedNode", kXMPErr_BadRDF );
	}

	if ( isTopLevel && (nodeTerm == kRDFTerm_Other) ) {
		XMP_Throw ( "Top level typedNode not allowed", kXMPErr_BadXMP );
	} else {
		RDF_NodeElementAttrs ( xmpParent, xmlNode, isTopLevel );
		RDF_PropertyElementList ( xmpParent, xmlNode, isTopLevel );
	}

}	// RDF_NodeElement


// =================================================================================================
// RDF_NodeElementAttrs
// ====================
//
// 7.2.7 propertyAttributeURIs
//		anyURI - ( coreSyntaxTerms | rdf:Description | rdf:li | oldTerms )
//
// 7.2.11 nodeElement
//		start-element ( URI == nodeElementURIs,
//						attributes == set ( ( idAttr | nodeIdAttr | aboutAttr )?, propertyAttr* ) )
//		propertyEltList
//		end-element()
//
// Process the attribute list for an RDF node element. A property attribute URI is anything other
// than an RDF term. The rdf:ID and rdf:nodeID attributes are simply ignored, as are rdf:about
// attributes on inner nodes.

static const XMP_OptionBits kExclusiveAttrMask = (kRDFMask_ID | kRDFMask_nodeID | kRDFMask_about);

static void
RDF_NodeElementAttrs ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	XMP_OptionBits exclusiveAttrs = 0;	// Used to detect attributes that are mutually exclusive.

	XML_cNodePos currAttr = xmlNode.attrs.begin();
	XML_cNodePos endAttr  = xmlNode.attrs.end();

	for ( ; currAttr != endAttr; ++currAttr ) {

		RDFTermKind attrTerm = GetRDFTermKind ( (*currAttr)->name );

		switch ( attrTerm ) {

			case kRDFTerm_ID     :
			case kRDFTerm_nodeID :
			case kRDFTerm_about  :

				if ( exclusiveAttrs & kExclusiveAttrMask ) XMP_Throw ( "Mutally exclusive about, ID, nodeID attributes", kXMPErr_BadRDF );
				exclusiveAttrs |= (1 << attrTerm);

				if ( isTopLevel && (attrTerm == kRDFTerm_about) ) {
					// This is the rdf:about attribute on a top level node. Set the XMP tree name if
					// it doesn't have a name yet. Make sure this name matches the XMP tree name.
					XMP_Assert ( xmpParent->parent == 0 );	// Must be the tree root node.
					if ( xmpParent->name.empty() ) {
						xmpParent->name = (*currAttr)->value;
					} else if ( ! (*currAttr)->value.empty() ) {
						if ( xmpParent->name != (*currAttr)->value ) XMP_Throw ( "Mismatched top level rdf:about values", kXMPErr_BadXMP );
					}
				}

				break;

			case kRDFTerm_Other :
				AddChildNode ( xmpParent, **currAttr, (*currAttr)->value.c_str(), isTopLevel );
				break;

			default :
				XMP_Throw ( "Invalid nodeElement attribute", kXMPErr_BadRDF );

		}

	}

}	// RDF_NodeElementAttrs


// =================================================================================================
// RDF_PropertyElementList
// =======================
//
// 7.2.13 propertyEltList
//		ws* ( propertyElt ws* )*

static void
RDF_PropertyElementList ( XMP_Node * xmpParent, const XML_Node & xmlParent, bool isTopLevel )
{
	XML_cNodePos currChild = xmlParent.content.begin();
	XML_cNodePos endChild  = xmlParent.content.end();

	for ( ; currChild != endChild; ++currChild ) {
		if ( (*currChild)->IsWhitespaceNode() ) continue;
		if ( (*currChild)->kind != kElemNode ) {
			XMP_Throw ( "Expected property element node not found", kXMPErr_BadRDF );
		}
		RDF_PropertyElement ( xmpParent, **currChild, isTopLevel );
	}

}	// RDF_PropertyElementList


// =================================================================================================
// RDF_PropertyElement
// ===================
//
// 7.2.14 propertyElt
//		resourcePropertyElt | literalPropertyElt | parseTypeLiteralPropertyElt |
//		parseTypeResourcePropertyElt | parseTypeCollectionPropertyElt | parseTypeOtherPropertyElt | emptyPropertyElt
//
// 7.2.15 resourcePropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr? ) )
//		ws* nodeElement ws*
//		end-element()
//
// 7.2.16 literalPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, datatypeAttr?) )
//		text()
//		end-element()
//
// 7.2.17 parseTypeLiteralPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseLiteral ) )
//		literal
//		end-element()
//
// 7.2.18 parseTypeResourcePropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseResource ) )
//		propertyEltList
//		end-element()
//
// 7.2.19 parseTypeCollectionPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseCollection ) )
//		nodeElementList
//		end-element()
//
// 7.2.20 parseTypeOtherPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseOther ) )
//		propertyEltList
//		end-element()
//
// 7.2.21 emptyPropertyElt
//		start-element ( URI == propertyElementURIs,
//						attributes == set ( idAttr?, ( resourceAttr | nodeIdAttr )?, propertyAttr* ) )
//		end-element()
//
// The various property element forms are not distinguished by the XML element name, but by their
// attributes for the most part. The exceptions are resourcePropertyElt and literalPropertyElt. They
// are distinguished by their XML element content.
//
// NOTE: The RDF syntax does not explicitly include the xml:lang attribute although it can appear in
// many of these. We have to allow for it in the attibute counts below.

static void
RDF_PropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	RDFTermKind nodeTerm = GetRDFTermKind ( xmlNode.name );
	if ( ! IsPropertyElementName ( nodeTerm ) ) XMP_Throw ( "Invalid property element name", kXMPErr_BadRDF );
	
	if ( xmlNode.attrs.size() > 3 ) {

		// Only an emptyPropertyElt can have more than 3 attributes.
		RDF_EmptyPropertyElement ( xmpParent, xmlNode, isTopLevel );

	} else {

		// Look through the attributes for one that isn't rdf:ID or xml:lang, it will usually tell
		// what we should be dealing with. The called routines must verify their specific syntax!

		XML_cNodePos currAttr = xmlNode.attrs.begin();
		XML_cNodePos endAttr  = xmlNode.attrs.end();
		XMP_VarString * attrName = 0;

		for ( ; currAttr != endAttr; ++currAttr ) {
			attrName = &((*currAttr)->name);
			if ( (*attrName != "xml:lang") && (*attrName != "rdf:ID") ) break;
		}

		if ( currAttr != endAttr ) {

			XMP_Assert ( attrName != 0 );
			XMP_VarString& attrValue = (*currAttr)->value;

			if ( *attrName == "rdf:datatype" ) {
				RDF_LiteralPropertyElement ( xmpParent, xmlNode, isTopLevel );
			} else if ( *attrName != "rdf:parseType" ) {
				RDF_EmptyPropertyElement ( xmpParent, xmlNode, isTopLevel );
			} else if ( attrValue == "Literal" ) {
				RDF_ParseTypeLiteralPropertyElement ( xmpParent, xmlNode, isTopLevel );
			} else if ( attrValue == "Resource" ) {
				RDF_ParseTypeResourcePropertyElement ( xmpParent, xmlNode, isTopLevel );
			} else if ( attrValue == "Collection" ) {
				RDF_ParseTypeCollectionPropertyElement ( xmpParent, xmlNode, isTopLevel );
			} else {
				RDF_ParseTypeOtherPropertyElement ( xmpParent, xmlNode, isTopLevel );
			}

		} else {

			// Only rdf:ID and xml:lang, could be a resourcePropertyElt, a literalPropertyElt, or an.
			// emptyPropertyElt. Look at the child XML nodes to decide which.

			if ( xmlNode.content.empty() ) {

				RDF_EmptyPropertyElement ( xmpParent, xmlNode, isTopLevel );

			} else {
			
				XML_cNodePos currChild = xmlNode.content.begin();
				XML_cNodePos endChild  = xmlNode.content.end();

				for ( ; currChild != endChild; ++currChild ) {
					if ( (*currChild)->kind != kCDataNode ) break;
				}
				
				if ( currChild == endChild ) {
					RDF_LiteralPropertyElement ( xmpParent, xmlNode, isTopLevel );
				} else {
					RDF_ResourcePropertyElement ( xmpParent, xmlNode, isTopLevel );
				}
			
			}

		}
		
	}

}	// RDF_PropertyElement


// =================================================================================================
// RDF_ResourcePropertyElement
// ===========================
//
// 7.2.15 resourcePropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr? ) )
//		ws* nodeElement ws*
//		end-element()
//
// This handles structs using an rdf:Description node, arrays using rdf:Bag/Seq/Alt, and typedNodes.
// It also catches and cleans up qualified properties written with rdf:Description and rdf:value.

static void
RDF_ResourcePropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	if ( isTopLevel && (xmlNode.name == "iX:changes") ) return;	// Strip old "punchcard" chaff.
	
	XMP_Node * newCompound = AddChildNode ( xmpParent, xmlNode, "", isTopLevel );
	
	XML_cNodePos currAttr = xmlNode.attrs.begin();
	XML_cNodePos endAttr  = xmlNode.attrs.end();

	for ( ; currAttr != endAttr; ++currAttr ) {
		XMP_VarString & attrName = (*currAttr)->name;
		if ( attrName == "xml:lang" ) {
			AddQualifierNode ( newCompound, **currAttr );
		} else if ( attrName == "rdf:ID" ) {
			continue;	// Ignore all rdf:ID attributes.
		} else {
			XMP_Throw ( "Invalid attribute for resource property element", kXMPErr_BadRDF );
		}
	}
	
	XML_cNodePos currChild = xmlNode.content.begin();
	XML_cNodePos endChild  = xmlNode.content.end();

	for ( ; currChild != endChild; ++currChild ) {
		if ( ! (*currChild)->IsWhitespaceNode() ) break;
	}
	if ( currChild == endChild ) XMP_Throw ( "Missing child of resource property element", kXMPErr_BadRDF );
	if ( (*currChild)->kind != kElemNode ) XMP_Throw ( "Children of resource property element must be XML elements", kXMPErr_BadRDF );

	if ( (*currChild)->name == "rdf:Bag" ) {
		newCompound->options |= kXMP_PropValueIsArray;
	} else if ( (*currChild)->name == "rdf:Seq" ) {
		newCompound->options |= kXMP_PropValueIsArray | kXMP_PropArrayIsOrdered;
	} else if ( (*currChild)->name == "rdf:Alt" ) {
		newCompound->options |= kXMP_PropValueIsArray | kXMP_PropArrayIsOrdered | kXMP_PropArrayIsAlternate;
	} else {
		newCompound->options |= kXMP_PropValueIsStruct;
		if ( (*currChild)->name != "rdf:Description" ) {
			XMP_VarString typeName ( (*currChild)->ns );
			size_t        colonPos = (*currChild)->name.find_first_of(':');
			if ( colonPos == XMP_VarString::npos ) XMP_Throw ( "All XML elements must be in a namespace", kXMPErr_BadXMP );
			typeName.append ( (*currChild)->name, colonPos, XMP_VarString::npos );
			AddQualifierNode ( newCompound, XMP_VarString("rdf:type"), typeName );
		}
	}

	RDF_NodeElement ( newCompound, **currChild, kNotTopLevel );
	if ( newCompound->options & kRDF_HasValueElem ) {
		FixupQualifiedNode ( newCompound );
	} else if ( newCompound->options & kXMP_PropArrayIsAlternate ) {
		DetectAltText ( newCompound );
	}

	for ( ++currChild; currChild != endChild; ++currChild ) {
		if ( ! (*currChild)->IsWhitespaceNode() ) XMP_Throw ( "Invalid child of resource property element", kXMPErr_BadRDF );
	}

}	// RDF_ResourcePropertyElement


// =================================================================================================
// RDF_LiteralPropertyElement
// ==========================
//
// 7.2.16 literalPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, datatypeAttr?) )
//		text()
//		end-element()
//
// Add a leaf node with the text value and qualifiers for the attributes.

static void
RDF_LiteralPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	XMP_Node * newChild = AddChildNode ( xmpParent, xmlNode, "", isTopLevel );
	
	XML_cNodePos currAttr = xmlNode.attrs.begin();
	XML_cNodePos endAttr  = xmlNode.attrs.end();

	for ( ; currAttr != endAttr; ++currAttr ) {
		XMP_VarString & attrName = (*currAttr)->name;
		if ( attrName == "xml:lang" ) {
			AddQualifierNode ( newChild, **currAttr );
		} else if ( (attrName == "rdf:ID") || (attrName == "rdf:datatype") ) {
			continue;	// Ignore all rdf:ID and rdf:datatype attributes.
		} else {
			XMP_Throw ( "Invalid attribute for literal property element", kXMPErr_BadRDF );
		}
	}
	
	XML_cNodePos currChild = xmlNode.content.begin();
	XML_cNodePos endChild  = xmlNode.content.end();
	size_t      textSize  = 0;

	for ( ; currChild != endChild; ++currChild ) {
		if ( (*currChild)->kind != kCDataNode ) XMP_Throw ( "Invalid child of literal property element", kXMPErr_BadRDF );
		textSize += (*currChild)->value.size();
	}
	
	newChild->value.reserve ( textSize );

	for ( currChild = xmlNode.content.begin(); currChild != endChild; ++currChild ) {
		newChild->value += (*currChild)->value;
	}

	#if 0	// *** XMP_DebugBuild
		newChild->_valuePtr = newChild->value.c_str();
	#endif
	
}	// RDF_LiteralPropertyElement


// =================================================================================================
// RDF_ParseTypeLiteralPropertyElement
// ===================================
//
// 7.2.17 parseTypeLiteralPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseLiteral ) )
//		literal
//		end-element()

static void
RDF_ParseTypeLiteralPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	IgnoreParam(xmpParent); IgnoreParam(xmlNode); IgnoreParam(isTopLevel); 
	
	XMP_Throw ( "ParseTypeLiteral property element not allowed", kXMPErr_BadXMP );

}	// RDF_ParseTypeLiteralPropertyElement


// =================================================================================================
// RDF_ParseTypeResourcePropertyElement
// ====================================
//
// 7.2.18 parseTypeResourcePropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseResource ) )
//		propertyEltList
//		end-element()
//
// Add a new struct node with a qualifier for the possible rdf:ID attribute. Then process the XML
// child nodes to get the struct fields.

static void
RDF_ParseTypeResourcePropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{

	XMP_Node * newStruct = AddChildNode ( xmpParent, xmlNode, "", isTopLevel );
	newStruct->options  |= kXMP_PropValueIsStruct;
	
	XML_cNodePos currAttr = xmlNode.attrs.begin();
	XML_cNodePos endAttr  = xmlNode.attrs.end();

	for ( ; currAttr != endAttr; ++currAttr ) {
		XMP_VarString & attrName = (*currAttr)->name;
		if ( attrName == "rdf:parseType" ) {
			continue;	// ! The caller ensured the value is "Resource".
		} else if ( attrName == "xml:lang" ) {
			AddQualifierNode ( newStruct, **currAttr );
		} else if ( attrName == "rdf:ID" ) {
			continue;	// Ignore all rdf:ID attributes.
		} else {
			XMP_Throw ( "Invalid attribute for ParseTypeResource property element", kXMPErr_BadRDF );
		}
	}

	RDF_PropertyElementList ( newStruct, xmlNode, kNotTopLevel );

	if ( newStruct->options & kRDF_HasValueElem ) FixupQualifiedNode ( newStruct );
	
	// *** Need to look for arrays using rdf:Description and rdf:type.

}	// RDF_ParseTypeResourcePropertyElement


// =================================================================================================
// RDF_ParseTypeCollectionPropertyElement
// ======================================
//
// 7.2.19 parseTypeCollectionPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseCollection ) )
//		nodeElementList
//		end-element()

static void
RDF_ParseTypeCollectionPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	IgnoreParam(xmpParent); IgnoreParam(xmlNode); IgnoreParam(isTopLevel); 

	XMP_Throw ( "ParseTypeCollection property element not allowed", kXMPErr_BadXMP );

}	// RDF_ParseTypeCollectionPropertyElement


// =================================================================================================
// RDF_ParseTypeOtherPropertyElement
// =================================
//
// 7.2.20 parseTypeOtherPropertyElt
//		start-element ( URI == propertyElementURIs, attributes == set ( idAttr?, parseOther ) )
//		propertyEltList
//		end-element()

static void
RDF_ParseTypeOtherPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	IgnoreParam(xmpParent); IgnoreParam(xmlNode); IgnoreParam(isTopLevel); 

	XMP_Throw ( "ParseTypeOther property element not allowed", kXMPErr_BadXMP );

}	// RDF_ParseTypeOtherPropertyElement


// =================================================================================================
// RDF_EmptyPropertyElement
// ========================
//
// 7.2.21 emptyPropertyElt
//		start-element ( URI == propertyElementURIs,
//						attributes == set ( idAttr?, ( resourceAttr | nodeIdAttr )?, propertyAttr* ) )
//		end-element()
//
//	<ns:Prop1/>  <!-- a simple property with an empty value --> 
//	<ns:Prop2 rdf:resource="http://www.adobe.com/"/> <!-- a URI value --> 
//	<ns:Prop3 rdf:value="..." ns:Qual="..."/> <!-- a simple qualified property --> 
//	<ns:Prop4 ns:Field1="..." ns:Field2="..."/> <!-- a struct with simple fields -->
//
// An emptyPropertyElt is an element with no contained content, just a possibly empty set of
// attributes. An emptyPropertyElt can represent three special cases of simple XMP properties: a
// simple property with an empty value (ns:Prop1), a simple property whose value is a URI
// (ns:Prop2), or a simple property with simple qualifiers (ns:Prop3). An emptyPropertyElt can also
// represent an XMP struct whose fields are all simple and unqualified (ns:Prop4).
//
// It is an error to use both rdf:value and rdf:resource - that can lead to invalid  RDF in the
// verbose form written using a literalPropertyElt.
//
// The XMP mapping for an emptyPropertyElt is a bit different from generic RDF, partly for 
// design reasons and partly for historical reasons. The XMP mapping rules are: 
//	1. If there is an rdf:value attribute then this is a simple property with a text value.
//		All other attributes are qualifiers.
//	2. If there is an rdf:resource attribute then this is a simple property with a URI value. 
//		All other attributes are qualifiers.
//	3. If there are no attributes other than xml:lang, rdf:ID, or rdf:nodeID then this is a simple 
//		property with an empty value. 
//	4. Otherwise this is a struct, the attributes other than xml:lang, rdf:ID, or rdf:nodeID are fields. 

static void
RDF_EmptyPropertyElement ( XMP_Node * xmpParent, const XML_Node & xmlNode, bool isTopLevel )
{
	bool hasPropertyAttrs = false;
	bool hasResourceAttr  = false;
	bool hasNodeIDAttr    = false;
	bool hasValueAttr     = false;
	
	const XML_Node * valueNode = 0;	// ! Can come from rdf:value or rdf:resource.
	
	if ( ! xmlNode.content.empty() ) XMP_Throw ( "Nested content not allowed with rdf:resource or property attributes", kXMPErr_BadRDF );
	
	// First figure out what XMP this maps to and remember the XML node for a simple value.
	
	XML_cNodePos currAttr = xmlNode.attrs.begin();
	XML_cNodePos endAttr  = xmlNode.attrs.end();

	for ( ; currAttr != endAttr; ++currAttr ) {

		RDFTermKind attrTerm = GetRDFTermKind ( (*currAttr)->name );

		switch ( attrTerm ) {

			case kRDFTerm_ID :
				// Nothing to do.
				break;

			case kRDFTerm_resource :
				if ( hasNodeIDAttr ) XMP_Throw ( "Empty property element can't have both rdf:resource and rdf:nodeID", kXMPErr_BadRDF );
				if ( hasValueAttr ) XMP_Throw ( "Empty property element can't have both rdf:value and rdf:resource", kXMPErr_BadXMP );
				hasResourceAttr = true;
				if ( ! hasValueAttr ) valueNode = *currAttr;
				break;

			case kRDFTerm_nodeID :
				if ( hasResourceAttr ) XMP_Throw ( "Empty property element can't have both rdf:resource and rdf:nodeID", kXMPErr_BadRDF );
				hasNodeIDAttr = true;
				break;

			case kRDFTerm_Other :
				if ( (*currAttr)->name == "rdf:value" ) {
					if ( hasResourceAttr ) XMP_Throw ( "Empty property element can't have both rdf:value and rdf:resource", kXMPErr_BadXMP );
					hasValueAttr = true;
					valueNode = *currAttr;
				} else if ( (*currAttr)->name != "xml:lang" ) {
					hasPropertyAttrs = true;
				}
				break;

			default :
				XMP_Throw ( "Unrecognized attribute of empty property element", kXMPErr_BadRDF );
				break;

		}

	}
	
	// Create the right kind of child node and visit the attributes again to add the fields or qualifiers.
	// ! Because of implementation vagaries, the xmpParent is the tree root for top level properties.
	// ! The schema is found, created if necessary, by AddChildNode.
	
	XMP_Node * childNode = AddChildNode ( xmpParent, xmlNode, "", isTopLevel );
	bool childIsStruct = false;
	
	if ( hasValueAttr | hasResourceAttr ) {
		childNode->value = valueNode->value;
		if ( ! hasValueAttr ) childNode->options |= kXMP_PropValueIsURI;	// ! Might have both rdf:value and rdf:resource.
	} else if ( hasPropertyAttrs ) {
		childNode->options |= kXMP_PropValueIsStruct;
		childIsStruct = true;
	}
		
	currAttr = xmlNode.attrs.begin();
	endAttr  = xmlNode.attrs.end();

	for ( ; currAttr != endAttr; ++currAttr ) {

		if ( *currAttr == valueNode ) continue;	// Skip the rdf:value or rdf:resource attribute holding the value.
		RDFTermKind attrTerm = GetRDFTermKind ( (*currAttr)->name );

		switch ( attrTerm ) {

			case kRDFTerm_ID       :
			case kRDFTerm_nodeID   :
				break;	// Ignore all rdf:ID and rdf:nodeID attributes.w
				
			case kRDFTerm_resource :
				AddQualifierNode ( childNode, **currAttr );
				break;

			case kRDFTerm_Other :
				if ( (! childIsStruct) || (*currAttr)->name == "xml:lang" ) {
					AddQualifierNode ( childNode, **currAttr );
				} else {
					AddChildNode ( childNode, **currAttr, (*currAttr)->value.c_str(), false );
				}
				break;

			default :
				XMP_Throw ( "Unrecognized attribute of empty property element", kXMPErr_BadRDF );
				break;

		}

	}

}	// RDF_EmptyPropertyElement


// =================================================================================================
