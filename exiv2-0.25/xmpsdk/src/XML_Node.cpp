// =================================================================================================
// Copyright 2005-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! Must be the first #include!
#include "XMLParserAdapter.hpp"

#include <map>
#include <cstring>
#include <cstdio>

// ! Can't include XMP..._Impl.hpp - used by both Core and Files.
#define XMP_LitNMatch(s,l,n)	(std::strncmp((s),(l),(n)) == 0)

#if XMP_WinBuild
#ifdef _MSC_VER
	#define snprintf _snprintf
	#pragma warning ( disable : 4996 )	// snprintf is safe
#endif
#endif

// =================================================================================================

#if 0	// Pattern for iterating over the children or attributes:
	for ( size_t xxNum = 0, xxLim = _node_->_offspring_->size(); xxNum < xxLim; ++xxNum ) {
		const XML_NodePtr _curr_ = _node_->_offspring_[xxNum];
	}
#endif

// =================================================================================================
// XML_Node::IsWhitespaceNode
//===========================

bool XML_Node::IsWhitespaceNode() const
{
	if ( this->kind != kCDataNode ) return false;

	for ( size_t i = 0; i < this->value.size(); ++i ) {
		unsigned char ch = this->value[i];
		if ( IsWhitespaceChar ( ch ) ) continue;
		// *** Add checks for other whitespace characters.
		return false;	// All the checks failed, this isn't whitespace.
	}

	return true;

}	// XML_Node::IsWhitespaceNode

// =================================================================================================
// XML_Node::IsLeafContentNode
//============================

bool XML_Node::IsLeafContentNode() const
{
	if ( this->kind != kElemNode ) return false;
	if ( this->content.size() == 0 ) return true;
	if ( this->content.size() > 1 ) return false;
	if ( this->content[0]->kind != kCDataNode ) return false;

	return true;

}	// XML_Node::IsLeafContentNode

// =================================================================================================
// XML_Node::IsEmptyLeafNode
//==========================

bool XML_Node::IsEmptyLeafNode() const
{

	if ( (this->kind != kElemNode) || (this->content.size() != 0) ) return false;
	return true;

}	// XML_Node::IsEmptyLeafNode

// =================================================================================================
// XML_Node::GetAttrValue
//=======================

XMP_StringPtr XML_Node::GetAttrValue ( XMP_StringPtr attrName ) const
{

	for ( size_t i = 0, aLim = this->attrs.size(); i < aLim; ++i ) {
		XML_Node * attrPtr = this->attrs[i];
		if ( ! attrPtr->ns.empty() ) continue;	// This form of GetAttrValue is for attrs in no namespace.
		if ( attrPtr->name == attrName ) return attrPtr->value.c_str();
	}
	
	return 0;	// Not found.

}	// XML_Node::GetAttrValue

// =================================================================================================
// XML_Node::SetAttrValue
//=======================

void XML_Node::SetAttrValue ( XMP_StringPtr attrName, XMP_StringPtr attrValue )
{
	
	for ( size_t i = 0, aLim = this->attrs.size(); i < aLim; ++i ) {
		XML_Node * attrPtr = this->attrs[i];
		if ( ! attrPtr->ns.empty() ) continue;	// This form of SetAttrValue is for attrs in no namespace.
		if ( attrPtr->name == attrName ) {
			attrPtr->value = attrValue;
			return;
		}
	}

}	// XML_Node::SetAttrValue

// =================================================================================================
// XML_Node::GetLeafContentValue
//==============================

XMP_StringPtr XML_Node::GetLeafContentValue() const
{
	if ( (! this->IsLeafContentNode()) || this->content.empty() ) return "";

	return this->content[0]->value.c_str();

}	// XML_Node::GetLeafContentValue

// =================================================================================================
// XML_Node::SetLeafContentValue
//==============================

void XML_Node::SetLeafContentValue ( XMP_StringPtr newValue )
{
	XML_Node * valueNode;
	
	if ( ! this->content.empty() ) {
		valueNode = this->content[0];
	} else {
		valueNode = new XML_Node ( this, "", kCDataNode );
		this->content.push_back ( valueNode );
	}

	valueNode->value = newValue;

}	// XML_Node::SetLeafContentValue

// =================================================================================================
// XML_Node::CountNamedElements
//=============================

size_t XML_Node::CountNamedElements ( XMP_StringPtr nsURI, XMP_StringPtr localName ) const
{
	size_t count = 0;
	
	for ( size_t i = 0, vLim = this->content.size(); i < vLim; ++i ) {
		const XML_Node & child = *this->content[i];
		if ( child.ns != nsURI ) continue;
		if ( strcmp ( localName, child.name.c_str()+child.nsPrefixLen ) != 0 ) continue;
		++count;
	}
	
	return count;
	
}	// XML_Node::CountNamedElements

// =================================================================================================
// XML_Node::GetNamedElement
//==========================

XML_NodePtr XML_Node::GetNamedElement ( XMP_StringPtr nsURI, XMP_StringPtr localName, size_t which /* = 0 */ )
{

	for ( size_t i = 0, vLim = this->content.size(); i < vLim; ++i ) {
		XML_Node * childPtr = this->content[i];
		if ( childPtr->ns != nsURI ) continue;
		if ( strcmp ( localName, childPtr->name.c_str()+childPtr->nsPrefixLen ) != 0 ) continue;
		if ( which == 0 ) return childPtr;
		--which;
	}
	
	return 0;	/// Not found.

}	// XML_Node::GetNamedElement

// =================================================================================================
// DumpNodeList
// ============

static const char * kNodeKinds[] = { "root", "elem", "attr", "cdata", "pi" };

static void DumpNodeList ( std::string * buffer, const XML_NodeVector & list, int indent )
{

	for ( size_t i = 0, limit = list.size(); i < limit; ++i ) {

		const XML_Node * node = list[i];
		
		for ( int t = indent; t > 0; --t ) *buffer += "  ";
		if ( node->IsWhitespaceNode() ) {
			*buffer += "-- whitespace --\n";
			continue;
		}

		*buffer += node->name;
		*buffer += " - ";
		*buffer += kNodeKinds[node->kind];
		if ( ! node->value.empty() ) {
			*buffer += ", value=\"";
			*buffer += node->value;
			*buffer += "\"";
		}
		if ( ! node->ns.empty() ) {
			*buffer += ", ns=\"";
			*buffer += node->ns;
			*buffer += "\"";
		}
		if ( node->nsPrefixLen != 0 ) {
			*buffer += ", prefixLen=";
			char numBuf [20];
			snprintf ( numBuf, sizeof(numBuf), "%lu", (unsigned long)node->nsPrefixLen );
			*buffer += numBuf;
		}
		*buffer += "\n";
	
		if ( ! node->attrs.empty() ) {
			for ( int t = indent+1; t > 0; --t ) *buffer += "  ";
			*buffer += "attrs:\n";
			DumpNodeList ( buffer, node->attrs, indent+2 );
		}
		
		if ( ! node->content.empty() ) {
			DumpNodeList ( buffer, node->content, indent+1 );
		}

	}

}	// DumpNodeList

// =================================================================================================
// XML_Node::Dump
//===============

void XML_Node::Dump ( std::string * buffer )
{

	*buffer = "Dump of XML_Node tree\n";

	*buffer += "Root info: name=\"";
	*buffer += this->name;
	*buffer += "\", value=\"";
	*buffer += this->value;
	*buffer += "\", ns=\"";
	*buffer += this->ns;
	*buffer += "\", kind=";
	*buffer += kNodeKinds[this->kind];
	*buffer += "\n";
	
	if ( ! this->attrs.empty() ) {
		*buffer += "  attrs:\n";
		DumpNodeList ( buffer, this->attrs, 2 );
	}
	*buffer += "\n";
	
	DumpNodeList ( buffer, this->content, 0 );

}	// XML_Node::Dump

// =================================================================================================
// SerializeOneNode
// ================

static void SerializeOneNode ( std::string * buffer, const XML_Node & node )
{
	size_t i, limit;
	XMP_StringPtr namePtr = node.name.c_str();
	if ( XMP_LitNMatch ( namePtr, "_dflt_:", 7 ) ) namePtr += 7;	// Hack for default namespaces.

	switch ( node.kind ) {
	
		case kElemNode:
			*buffer += '<';
			*buffer += namePtr;
			for ( i = 0, limit = node.attrs.size(); i < limit; ++i ) {
				SerializeOneNode ( buffer, *node.attrs[i] );
			}
			if ( node.content.empty() ) {
				*buffer += "/>";
			} else {
				*buffer += '>';
				for ( i = 0, limit = node.content.size(); i < limit; ++i ) {
					SerializeOneNode ( buffer, *node.content[i] );
				}
				*buffer += "</";
				*buffer += namePtr;
				*buffer += '>';
			}
			break;
	
		case kAttrNode:
			*buffer += ' ';
			*buffer += namePtr;
			*buffer += "=\"";
			*buffer += node.value;
			*buffer += '"';
			break;
	
		case kCDataNode:
			*buffer += node.value;
			break;
	
		case kPINode:
			*buffer += node.value;	// *** Note that we're dropping PIs during the Expat parse.
			break;
	
	}

}	// SerializeOneNode

// =================================================================================================
// CollectNamespaceDecls
// =====================

typedef std::map < std::string, std::string > NamespaceMap;

static void CollectNamespaceDecls ( NamespaceMap * nsMap, const XML_Node & node )
{
	size_t i, limit;

	if ( ! node.ns.empty() ) {
		size_t nameMid = 0;
		while ( node.name[nameMid] != ':' ) ++nameMid;
		std::string prefix = node.name.substr ( 0, nameMid );
		(*nsMap)[prefix] = node.ns;
	}
	
	if ( node.kind == kElemNode ) {

		for ( i = 0, limit = node.attrs.size(); i < limit; ++i ) {
			CollectNamespaceDecls ( nsMap, *node.attrs[i] );
		}

		for ( i = 0, limit = node.content.size(); i < limit; ++i ) {
			const XML_Node & content = *node.content[i];
			if ( content.kind == kElemNode ) CollectNamespaceDecls ( nsMap, content );
		}

	}
	
}	// CollectNamespaceDecls

// =================================================================================================
// XML_Node::Serialize
//====================

void XML_Node::Serialize ( std::string * buffer )
{
	buffer->erase();
	
	if ( this->kind != kRootNode ) {

		SerializeOneNode ( buffer, *this );

	} else {
	
		// Do the outermost level here, in order to add the XML version and namespace declarations.

		*buffer += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

		for ( size_t outer = 0, oLimit = this->content.size(); outer < oLimit; ++outer ) {

			const XML_Node & node = *this->content[outer];

			if ( node.kind != kElemNode ) {

				SerializeOneNode ( buffer, node );

			} else {

				XMP_StringPtr namePtr = node.name.c_str();
				if ( XMP_LitNMatch ( namePtr, "_dflt_:", 7 ) ) namePtr += 7;	// Hack for default namespaces.

				*buffer += '<';
				*buffer += namePtr;
				
				NamespaceMap nsMap;
				CollectNamespaceDecls ( &nsMap, node );
				NamespaceMap::iterator nsDecl = nsMap.begin();
				NamespaceMap::iterator nsEnd  = nsMap.end();
				for ( ; nsDecl != nsEnd; ++nsDecl ) {
					const std::string & prefix = nsDecl->first;
					*buffer += " xmlns";
					if ( prefix != "_dflt_" ) { *buffer += ':'; *buffer += prefix; }
					*buffer += "=\"";
					*buffer += nsDecl->second;
					*buffer += '"';
				}

				for ( size_t attr = 0, aLimit = node.attrs.size(); attr < aLimit; ++attr ) {
					SerializeOneNode ( buffer, *node.attrs[attr] );
				}

				if ( node.content.empty() ) {
					*buffer += "/>";
				} else {
					*buffer += '>';
					for ( size_t child = 0, cLimit = node.content.size(); child < cLimit; ++child ) {
						SerializeOneNode ( buffer, *node.content[child] );
					}
					*buffer += "</";
					*buffer += namePtr;
					*buffer += '>';
				}

			}

		}
		
	}
	

}	// XML_Node::Serialize

// =================================================================================================
// XML_Node::RemoveAttrs
//======================

void XML_Node::RemoveAttrs()
{

	for ( size_t i = 0, vLim = this->attrs.size(); i < vLim; ++i ) delete this->attrs[i];
	this->attrs.clear();

}	// XML_Node::RemoveAttrs

// =================================================================================================
// XML_Node::RemoveContent
//========================

void XML_Node::RemoveContent()
{

	for ( size_t i = 0, vLim = this->content.size(); i < vLim; ++i ) delete this->content[i];
	this->content.clear();

}	// XML_Node::RemoveContent

// =================================================================================================
// XML_Node::ClearNode
//====================

void XML_Node::ClearNode()
{

	this->kind = 0;
	this->ns.erase();
	this->name.erase();
	this->value.erase();

	this->RemoveAttrs();
	this->RemoveContent();

}	// XML_Node::ClearNode

// =================================================================================================
