// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
//
// Adobe patent application tracking #P435, entitled 'Unique markers to simplify embedding data of
// one format in a file with a different format', inventors: Sean Parent, Greg Gilley.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"

#include "XMPMeta.hpp"
#include "XMPIterator.hpp"
#include "XMPUtils.hpp"

#include "XMP_Version.h"
#include "UnicodeInlines.incl_cpp"
#include "UnicodeConversions.hpp"
#include "ExpatAdapter.hpp"

#if XMP_DebugBuild
	#include <iostream>
#endif

using namespace std;

#if XMP_WinBuild
    #ifdef _MSC_VER
        #pragma warning ( disable : 4533 )	// initialization of '...' is skipped by 'goto ...'
        #pragma warning ( disable : 4702 )	// unreachable code
        #pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
    #endif
#endif


// *** Use the XMP_PropIsXyz (Schema, Simple, Struct, Array, ...) macros
// *** Add debug codegen checks, e.g. that typical masking operations really work
// *** Change all uses of strcmp and strncmp to XMP_LitMatch and XMP_LitNMatch


// =================================================================================================
// Local Types and Constants
// =========================

typedef unsigned char XMP_CLTMatch;

enum {	// Values for XMP_CLTMatch.
	kXMP_CLT_NoValues,
	kXMP_CLT_SpecificMatch,
	kXMP_CLT_SingleGeneric,
	kXMP_CLT_MultipleGeneric,
	kXMP_CLT_XDefault,
	kXMP_CLT_FirstItem
};


// =================================================================================================
// Static Variables
// ================


// =================================================================================================
// Local Utilities
// ===============


// -------------------------------------------------------------------------------------------------
// SetNodeValue
// ------------

static inline void
SetNodeValue ( XMP_Node * node, XMP_StringPtr value )
{

	#if XMP_DebugBuild	// ! Hack to force an assert.
		if ( (node->name == "xmp:TestAssertNotify") && XMP_LitMatch ( value, "DoIt!" ) ) {
			XMP_Assert ( node->name != "xmp:TestAssertNotify" );
		}
	#endif
	
	node->value = value;
	
	XMP_Uns8* chPtr = (XMP_Uns8*) node->value.c_str();	// Check for valid UTF-8, replace ASCII controls with a space.
	while ( *chPtr != 0 ) {
		while ( (*chPtr != 0) && (*chPtr < 0x80) ) {
			if ( *chPtr < 0x20 ) {
				if ( (*chPtr != kTab) && (*chPtr != kLF) && (*chPtr != kCR) ) *chPtr = 0x20;
			} else if (*chPtr == 0x7F ) {
				*chPtr = 0x20;
			}
			++chPtr;
		}
		XMP_Assert ( (*chPtr == 0) || (*chPtr >= 0x80) );
		if ( *chPtr != 0 ) (void) GetCodePoint ( (const XMP_Uns8 **) &chPtr );	// Throws for bad UTF-8.
	}

	if ( XMP_PropIsQualifier(node->options) && (node->name == "xml:lang") ) NormalizeLangValue ( &node->value );

	#if 0	// *** XMP_DebugBuild
		node->_valuePtr = node->value.c_str();
	#endif
	
}	// SetNodeValue


// -------------------------------------------------------------------------------------------------
// SetNode
// -------
//
// The internals for SetProperty and related calls, used after the node is found or created.

static void
SetNode	( XMP_Node * node, XMP_StringPtr value, XMP_OptionBits options )
{
	if ( options & kXMP_DeleteExisting ) {
		XMP_ClearOption ( options, kXMP_DeleteExisting );
		node->options = options;
		node->value.erase();
		node->RemoveChildren();
		node->RemoveQualifiers();
	}
	
	node->options |= options;	// Keep options set by FindNode when creating a new node.

	if ( value != 0 ) {
	
		// This is setting the value of a leaf node.
		if ( node->options & kXMP_PropCompositeMask ) XMP_Throw ( "Composite nodes can't have values", kXMPErr_BadXPath );
		XMP_Assert ( node->children.empty() );
		SetNodeValue ( node, value );
	
	} else {
	
		// This is setting up an array or struct.
		if ( ! node->value.empty() ) XMP_Throw ( "Composite nodes can't have values", kXMPErr_BadXPath );
		if ( node->options & kXMP_PropCompositeMask ) {	// Can't change an array to a struct, or vice versa.
			if ( (options & kXMP_PropCompositeMask) != (node->options & kXMP_PropCompositeMask) ) {
				XMP_Throw ( "Requested and existing composite form mismatch", kXMPErr_BadXPath );
			}
		}
		node->RemoveChildren();
	
	}
	
}	// SetNode


// -------------------------------------------------------------------------------------------------
// DoSetArrayItem
// --------------

static void
DoSetArrayItem ( XMP_Node *		arrayNode,
				 XMP_Index		itemIndex,
				 XMP_StringPtr	itemValue,
				 XMP_OptionBits options )
{
	XMP_OptionBits itemLoc = options & kXMP_PropArrayLocationMask;
	XMP_Index      arraySize = arrayNode->children.size();
	
	options &= ~kXMP_PropArrayLocationMask;
	options = VerifySetOptions ( options, itemValue );
	
	// Now locate or create the item node and set the value. Note the index parameter is one-based!
	// The index can be in the range [0..size+1] or "last", normalize it and check the insert flags.
	// The order of the normalization checks is important. If the array is empty we end up with an
	// index and location to set item size+1.
	
	XMP_Node * itemNode = 0;
	
	if ( itemIndex == kXMP_ArrayLastItem ) itemIndex = arraySize;
	if ( (itemIndex == 0) && (itemLoc == kXMP_InsertAfterItem) ) {
		itemIndex = 1;
		itemLoc = kXMP_InsertBeforeItem;
	}
	if ( (itemIndex == arraySize) && (itemLoc == kXMP_InsertAfterItem) ) {
		itemIndex += 1;
		itemLoc = 0;
	}
	if ( (itemIndex == arraySize+1) && (itemLoc == kXMP_InsertBeforeItem) ) itemLoc = 0;
	
	if ( itemIndex == arraySize+1 ) {

		if ( itemLoc != 0 ) XMP_Throw ( "Can't insert before or after implicit new item", kXMPErr_BadIndex );
		itemNode = new XMP_Node ( arrayNode, kXMP_ArrayItemName, 0 );
		arrayNode->children.push_back ( itemNode );

	} else {

		if ( (itemIndex < 1) || (itemIndex > arraySize) ) XMP_Throw ( "Array index out of bounds", kXMPErr_BadIndex );
		--itemIndex;	// ! Convert the index to a C zero-based value!
		if ( itemLoc == 0 ) {
			itemNode = arrayNode->children[itemIndex];
		} else {
			XMP_NodePtrPos itemPos = arrayNode->children.begin() + itemIndex;
			if ( itemLoc == kXMP_InsertAfterItem ) ++itemPos;
			itemNode = new XMP_Node ( arrayNode, kXMP_ArrayItemName, 0 );
			itemPos = arrayNode->children.insert ( itemPos, itemNode );
		}

	}
	
	SetNode ( itemNode, itemValue, options );
	
}	// DoSetArrayItem


// -------------------------------------------------------------------------------------------------
// ChooseLocalizedText
// -------------------
//
// 1. Look for an exact match with the specific language.
// 2. If a generic language is given, look for partial matches.
// 3. Look for an "x-default" item.
// 4. Choose the first item.

static XMP_CLTMatch
ChooseLocalizedText ( const XMP_Node *	 arrayNode,
					  XMP_StringPtr		 genericLang,
					  XMP_StringPtr		 specificLang,
					  const XMP_Node * * itemNode )
{
	const XMP_Node * currItem = 0;
	const size_t itemLim = arrayNode->children.size();
	size_t itemNum;
	
	// See if the array has the right form. Allow empty alt arrays, that is what parsing returns.
	// *** Should check alt-text bit when that is reliably maintained.

	if ( ! ( XMP_ArrayIsAltText(arrayNode->options) ||
	         (arrayNode->children.empty() && XMP_ArrayIsAlternate(arrayNode->options)) ) ) {
		XMP_Throw ( "Localized text array is not alt-text", kXMPErr_BadXPath );
	}
	if ( arrayNode->children.empty() ) {
		*itemNode = 0;
		return kXMP_CLT_NoValues;
	}

	for ( itemNum = 0; itemNum < itemLim; ++itemNum ) {
		currItem = arrayNode->children[itemNum];
		if ( currItem->options & kXMP_PropCompositeMask ) {
			XMP_Throw ( "Alt-text array item is not simple", kXMPErr_BadXPath );
		}
		if ( currItem->qualifiers.empty() || (currItem->qualifiers[0]->name != "xml:lang") ) {
			XMP_Throw ( "Alt-text array item has no language qualifier", kXMPErr_BadXPath );
		}
	}

	// Look for an exact match with the specific language.
	for ( itemNum = 0; itemNum < itemLim; ++itemNum ) {
		currItem = arrayNode->children[itemNum];
		if ( currItem->qualifiers[0]->value == specificLang ) {
			*itemNode = currItem;
			return kXMP_CLT_SpecificMatch;
		}
	}
	
	if ( *genericLang != 0 ) {

		// Look for the first partial match with the generic language.
		const size_t genericLen = strlen ( genericLang );
		for ( itemNum = 0; itemNum < itemLim; ++itemNum ) {
			currItem = arrayNode->children[itemNum];
			XMP_StringPtr currLang = currItem->qualifiers[0]->value.c_str();
			const size_t currLangSize = currItem->qualifiers[0]->value.size();
			if ( (currLangSize >= genericLen) &&
				 XMP_LitNMatch ( currLang, genericLang, genericLen ) &&
				 ((currLangSize == genericLen) || (currLang[genericLen] == '-')) ) {
				*itemNode = currItem;
				break;	// ! Don't return, need to look for other matches.
			}
		}

		if ( itemNum < itemLim ) {
			
			// Look for a second partial match with the generic language.
			for ( ++itemNum; itemNum < itemLim; ++itemNum ) {
				currItem = arrayNode->children[itemNum];
				XMP_StringPtr currLang = currItem->qualifiers[0]->value.c_str();
				const size_t currLangSize = currItem->qualifiers[0]->value.size();
				if ( (currLangSize >= genericLen) &&
					 XMP_LitNMatch ( currLang, genericLang, genericLen ) &&
					 ((currLangSize == genericLen) || (currLang[genericLen] == '-')) ) {
					return kXMP_CLT_MultipleGeneric;	// ! Leave itemNode with the first partial match.
				}
			}
			return kXMP_CLT_SingleGeneric;	// No second partial match was found.

		}
		
	}
	
	// Look for an 'x-default' item.
	for ( itemNum = 0; itemNum < itemLim; ++itemNum ) {
		currItem = arrayNode->children[itemNum];
		if ( currItem->qualifiers[0]->value == "x-default" ) {
			*itemNode = currItem;
			return kXMP_CLT_XDefault;
		}
	}
	
	// Everything failed, choose the first item.
	*itemNode = arrayNode->children[0];
	return kXMP_CLT_FirstItem;
	
}	// ChooseLocalizedText


// -------------------------------------------------------------------------------------------------
// AppendLangItem
// --------------

static void
AppendLangItem ( XMP_Node * arrayNode, XMP_StringPtr itemLang, XMP_StringPtr itemValue )
{
	XMP_Node * newItem  = new XMP_Node ( arrayNode, kXMP_ArrayItemName, itemValue, (kXMP_PropHasQualifiers | kXMP_PropHasLang) );
	XMP_Node * langQual = new XMP_Node ( newItem, "xml:lang", itemLang, kXMP_PropIsQualifier );
	newItem->qualifiers.push_back ( langQual );

	if ( (arrayNode->children.empty()) || (langQual->value != "x-default") ) {
		arrayNode->children.push_back ( newItem );
	} else {
		arrayNode->children.insert ( arrayNode->children.begin(), newItem );
	}

}	// AppendLangItem


// =================================================================================================
// Class Methods
// =============
//
//
// =================================================================================================


// -------------------------------------------------------------------------------------------------
// GetProperty
// -----------

bool
XMPMeta::GetProperty ( XMP_StringPtr	schemaNS,
					   XMP_StringPtr	propName,
					   XMP_StringPtr *	propValue,
					   XMP_StringLen *	valueSize,
					   XMP_OptionBits *	options ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (propValue != 0) && (valueSize != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_ExpandedXPath expPath;
	ExpandXPath ( schemaNS, propName, &expPath );
	
	XMP_Node * propNode = FindConstNode ( &tree, expPath );
	if ( propNode == 0 ) return false;
	
	*propValue = propNode->value.c_str();
	*valueSize = propNode->value.size();
	*options   = propNode->options;
	
	return true;
	
}	// GetProperty


// -------------------------------------------------------------------------------------------------
// GetArrayItem
// ------------

bool
XMPMeta::GetArrayItem ( XMP_StringPtr	 schemaNS,
						XMP_StringPtr	 arrayName,
						XMP_Index		 itemIndex,
						XMP_StringPtr *	 itemValue,
						XMP_StringLen *	 valueSize,
						XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (itemValue != 0) && (valueSize != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr itemPath;
	XMP_StringLen pathLen;

	XMPUtils::ComposeArrayItemPath ( schemaNS, arrayName, itemIndex, &itemPath, &pathLen );
	return GetProperty ( schemaNS, itemPath, itemValue, valueSize, options );

}	// GetArrayItem


// -------------------------------------------------------------------------------------------------
// GetStructField
// --------------

bool
XMPMeta::GetStructField	( XMP_StringPtr	   schemaNS,
						  XMP_StringPtr	   structName,
						  XMP_StringPtr	   fieldNS,
						  XMP_StringPtr	   fieldName,
						  XMP_StringPtr *  fieldValue,
						  XMP_StringLen *  valueSize,
						  XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (structName != 0) && (fieldNS != 0) && (fieldName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (fieldValue != 0) && (valueSize != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr fieldPath;
	XMP_StringLen pathLen;

	XMPUtils::ComposeStructFieldPath ( schemaNS, structName, fieldNS, fieldName, &fieldPath, &pathLen );
	return GetProperty ( schemaNS, fieldPath, fieldValue, valueSize, options );

}	// GetStructField


// -------------------------------------------------------------------------------------------------
// GetQualifier
// ------------

bool
XMPMeta::GetQualifier ( XMP_StringPtr	 schemaNS,
						XMP_StringPtr	 propName,
						XMP_StringPtr	 qualNS,
						XMP_StringPtr	 qualName,
						XMP_StringPtr *	 qualValue,
						XMP_StringLen *	 valueSize,
						XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) && (qualNS != 0) && (qualName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (qualValue != 0) && (valueSize != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr qualPath;
	XMP_StringLen pathLen;

	XMPUtils::ComposeQualifierPath ( schemaNS, propName, qualNS, qualName, &qualPath, &pathLen );
	return GetProperty ( schemaNS, qualPath, qualValue, valueSize, options );

}	// GetQualifier


// -------------------------------------------------------------------------------------------------
// SetProperty
// -----------

// *** Should handle array items specially, calling SetArrayItem.

void
XMPMeta::SetProperty ( XMP_StringPtr  schemaNS,
					   XMP_StringPtr  propName,
					   XMP_StringPtr  propValue,
					   XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	options = VerifySetOptions ( options, propValue );

	XMP_ExpandedXPath expPath;
	ExpandXPath ( schemaNS, propName, &expPath );

	XMP_Node * propNode = FindNode ( &tree, expPath, kXMP_CreateNodes, options );
	if ( propNode == 0 ) XMP_Throw ( "Specified property does not exist", kXMPErr_BadXPath );
	
	SetNode ( propNode, propValue, options );
	
}	// SetProperty


// -------------------------------------------------------------------------------------------------
// SetArrayItem
// ------------

void
XMPMeta::SetArrayItem ( XMP_StringPtr  schemaNS,
						XMP_StringPtr  arrayName,
						XMP_Index	   itemIndex,
						XMP_StringPtr  itemValue,
						XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) );	// Enforced by wrapper.

	XMP_ExpandedXPath arrayPath;
	ExpandXPath ( schemaNS, arrayName, &arrayPath );
	XMP_Node * arrayNode = FindNode ( &tree, arrayPath, kXMP_ExistingOnly );	// Just lookup, don't try to create.
	if ( arrayNode == 0 ) XMP_Throw ( "Specified array does not exist", kXMPErr_BadXPath );
	
	DoSetArrayItem ( arrayNode, itemIndex, itemValue, options );
	
}	// SetArrayItem


// -------------------------------------------------------------------------------------------------
// AppendArrayItem
// ---------------

void
XMPMeta::AppendArrayItem ( XMP_StringPtr  schemaNS,
						   XMP_StringPtr  arrayName,
						   XMP_OptionBits arrayOptions,
						   XMP_StringPtr  itemValue,
						   XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) );	// Enforced by wrapper.

	arrayOptions = VerifySetOptions ( arrayOptions, 0 );
	if ( (arrayOptions & ~kXMP_PropArrayFormMask) != 0 ) {
		XMP_Throw ( "Only array form flags allowed for arrayOptions", kXMPErr_BadOptions );
	}
	
	// Locate or create the array. If it already exists, make sure the array form from the options
	// parameter is compatible with the current state.
	
	XMP_ExpandedXPath arrayPath;
	ExpandXPath ( schemaNS, arrayName, &arrayPath );
	XMP_Node * arrayNode = FindNode ( &tree, arrayPath, kXMP_ExistingOnly );	// Just lookup, don't try to create.
	
	if ( arrayNode != 0 ) {
		// The array exists, make sure the form is compatible. Zero arrayForm means take what exists.
		if ( ! (arrayNode->options & kXMP_PropValueIsArray) ) {
			XMP_Throw ( "The named property is not an array", kXMPErr_BadXPath );
		}
		#if 0
			// *** Disable for now. Need to do some general rethinking of semantic checks.
			if ( (arrayOptions != 0) && (arrayOptions != (arrayNode->options & kXMP_PropArrayFormMask)) ) {
				XMP_Throw ( "Mismatch of existing and specified array form", kXMPErr_BadOptions );
			}
		#endif
	} else {
		// The array does not exist, try to create it.
		if ( arrayOptions == 0 ) XMP_Throw ( "Explicit arrayOptions required to create new array", kXMPErr_BadOptions );
		arrayNode = FindNode ( &tree, arrayPath, kXMP_CreateNodes, arrayOptions );
		if ( arrayNode == 0 ) XMP_Throw ( "Failure creating array node", kXMPErr_BadXPath );
	}
	
	DoSetArrayItem ( arrayNode, kXMP_ArrayLastItem, itemValue, (options | kXMP_InsertAfterItem) );
	
}	// AppendArrayItem


// -------------------------------------------------------------------------------------------------
// SetStructField
// --------------

void
XMPMeta::SetStructField	( XMP_StringPtr	 schemaNS,
						  XMP_StringPtr	 structName,
						  XMP_StringPtr	 fieldNS,
						  XMP_StringPtr	 fieldName,
						  XMP_StringPtr	 fieldValue,
						  XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (structName != 0) && (fieldNS != 0) && (fieldName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	fieldPath;
	XMP_StringLen	pathLen;

	XMPUtils::ComposeStructFieldPath ( schemaNS, structName, fieldNS, fieldName, &fieldPath, &pathLen );
	SetProperty ( schemaNS, fieldPath, fieldValue, options );

}	// SetStructField


// -------------------------------------------------------------------------------------------------
// SetQualifier
// ------------

void
XMPMeta::SetQualifier ( XMP_StringPtr  schemaNS,
						XMP_StringPtr  propName,
						XMP_StringPtr  qualNS,
						XMP_StringPtr  qualName,
						XMP_StringPtr  qualValue,
						XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) && (qualNS != 0) && (qualName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	qualPath;
	XMP_StringLen	pathLen;

	XMP_ExpandedXPath expPath;
	ExpandXPath ( schemaNS, propName, &expPath );
	XMP_Node * propNode = FindNode ( &tree, expPath, kXMP_ExistingOnly );
	if ( propNode == 0 ) XMP_Throw ( "Specified property does not exist", kXMPErr_BadXPath );

	XMPUtils::ComposeQualifierPath ( schemaNS, propName, qualNS, qualName, &qualPath, &pathLen );
	SetProperty ( schemaNS, qualPath, qualValue, options );

}	// SetQualifier


// -------------------------------------------------------------------------------------------------
// DeleteProperty
// --------------

void
XMPMeta::DeleteProperty	( XMP_StringPtr	schemaNS,
						  XMP_StringPtr	propName )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	XMP_ExpandedXPath	expPath;
	ExpandXPath ( schemaNS, propName, &expPath );
	
	XMP_NodePtrPos ptrPos;
	XMP_Node * propNode = FindNode ( &tree, expPath, kXMP_ExistingOnly, kXMP_NoOptions, &ptrPos );
	if ( propNode == 0 ) return;
	XMP_Node * parentNode = propNode->parent;
	
	// Erase the pointer from the parent's vector, then delete the node and all below it.
	
	if ( ! (propNode->options & kXMP_PropIsQualifier) ) {

		parentNode->children.erase ( ptrPos );
		DeleteEmptySchema ( parentNode );

	} else {

		if ( propNode->name == "xml:lang" ) {
			XMP_Assert ( parentNode->options & kXMP_PropHasLang );	// *** &= ~flag would be safer
			parentNode->options ^= kXMP_PropHasLang;
		} else if ( propNode->name == "rdf:type" ) {
			XMP_Assert ( parentNode->options & kXMP_PropHasType );
			parentNode->options ^= kXMP_PropHasType;
		}

		parentNode->qualifiers.erase ( ptrPos );
		XMP_Assert ( parentNode->options & kXMP_PropHasQualifiers );
		if ( parentNode->qualifiers.empty() ) parentNode->options ^= kXMP_PropHasQualifiers;

	}
	
	delete propNode;	// ! The destructor takes care of the whole subtree.
	
}	// DeleteProperty


// -------------------------------------------------------------------------------------------------
// DeleteArrayItem
// ---------------

void
XMPMeta::DeleteArrayItem ( XMP_StringPtr schemaNS,
						   XMP_StringPtr arrayName,
						   XMP_Index	 itemIndex )
{
	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	itemPath;
	XMP_StringLen	pathLen;

	XMPUtils::ComposeArrayItemPath ( schemaNS, arrayName, itemIndex, &itemPath, &pathLen );
	DeleteProperty ( schemaNS, itemPath );

}	// DeleteArrayItem


// -------------------------------------------------------------------------------------------------
// DeleteStructField
// -----------------

void
XMPMeta::DeleteStructField ( XMP_StringPtr schemaNS,
							 XMP_StringPtr structName,
							 XMP_StringPtr fieldNS,
							 XMP_StringPtr fieldName )
{
	XMP_Assert ( (schemaNS != 0) && (structName != 0) && (fieldNS != 0) && (fieldName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	fieldPath;
	XMP_StringLen	pathLen;

	XMPUtils::ComposeStructFieldPath ( schemaNS, structName, fieldNS, fieldName, &fieldPath, &pathLen );
	DeleteProperty ( schemaNS, fieldPath );

}	// DeleteStructField


// -------------------------------------------------------------------------------------------------
// DeleteQualifier
// ---------------

void
XMPMeta::DeleteQualifier ( XMP_StringPtr schemaNS,
						   XMP_StringPtr propName,
						   XMP_StringPtr qualNS,
						   XMP_StringPtr qualName )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) && (qualNS != 0) && (qualName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	qualPath;
	XMP_StringLen	pathLen;

	XMPUtils::ComposeQualifierPath ( schemaNS, propName, qualNS, qualName, &qualPath, &pathLen );
	DeleteProperty ( schemaNS, qualPath );

}	// DeleteQualifier


// -------------------------------------------------------------------------------------------------
// DoesPropertyExist
// -----------------

bool
XMPMeta::DoesPropertyExist ( XMP_StringPtr schemaNS,
							 XMP_StringPtr propName ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	XMP_ExpandedXPath	expPath;
	ExpandXPath ( schemaNS, propName, &expPath );

	XMP_Node * propNode = FindConstNode ( &tree, expPath );
	return (propNode != 0);
	
}	// DoesPropertyExist


// -------------------------------------------------------------------------------------------------
// DoesArrayItemExist
// ------------------

bool
XMPMeta::DoesArrayItemExist	( XMP_StringPtr	schemaNS,
							  XMP_StringPtr	arrayName,
							  XMP_Index		itemIndex ) const
{
	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	itemPath;
	XMP_StringLen	pathLen;

	XMPUtils::ComposeArrayItemPath ( schemaNS, arrayName, itemIndex, &itemPath, &pathLen );
	return DoesPropertyExist ( schemaNS, itemPath );

}	// DoesArrayItemExist


// -------------------------------------------------------------------------------------------------
// DoesStructFieldExist
// --------------------

bool
XMPMeta::DoesStructFieldExist ( XMP_StringPtr schemaNS,
								XMP_StringPtr structName,
								XMP_StringPtr fieldNS,
								XMP_StringPtr fieldName ) const
{
	XMP_Assert ( (schemaNS != 0) && (structName != 0) && (fieldNS != 0) && (fieldName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	fieldPath;
	XMP_StringLen	pathLen;

	XMPUtils::ComposeStructFieldPath ( schemaNS, structName, fieldNS, fieldName, &fieldPath, &pathLen );
	return DoesPropertyExist ( schemaNS, fieldPath );

}	// DoesStructFieldExist


// -------------------------------------------------------------------------------------------------
// DoesQualifierExist
// ------------------

bool
XMPMeta::DoesQualifierExist	( XMP_StringPtr	schemaNS,
							  XMP_StringPtr	propName,
							  XMP_StringPtr	qualNS,
							  XMP_StringPtr	qualName ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) && (qualNS != 0) && (qualName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	qualPath;
	XMP_StringLen	pathLen;

	XMPUtils::ComposeQualifierPath ( schemaNS, propName, qualNS, qualName, &qualPath, &pathLen );
	return DoesPropertyExist ( schemaNS, qualPath );

}	// DoesQualifierExist


// -------------------------------------------------------------------------------------------------
// GetLocalizedText
// ----------------

bool
XMPMeta::GetLocalizedText ( XMP_StringPtr	 schemaNS,
							XMP_StringPtr	 arrayName,
							XMP_StringPtr	 _genericLang,
							XMP_StringPtr	 _specificLang,
							XMP_StringPtr *	 actualLang,
							XMP_StringLen *	 langSize,
							XMP_StringPtr *	 itemValue,
							XMP_StringLen *	 valueSize,
							XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) && (_genericLang != 0) && (_specificLang != 0) );	// Enforced by wrapper.
	XMP_Assert ( (actualLang != 0) && (langSize != 0) );	// Enforced by wrapper.
	XMP_Assert ( (itemValue != 0) && (valueSize != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_VarString zGenericLang  ( _genericLang );
	XMP_VarString zSpecificLang ( _specificLang );
	NormalizeLangValue ( &zGenericLang );
	NormalizeLangValue ( &zSpecificLang );
	
	XMP_StringPtr genericLang  = zGenericLang.c_str();
	XMP_StringPtr specificLang = zSpecificLang.c_str();
	
	XMP_ExpandedXPath arrayPath;
	ExpandXPath ( schemaNS, arrayName, &arrayPath );
	
	const XMP_Node * arrayNode = FindConstNode ( &tree, arrayPath );	// *** This expand/find idiom is used in 3 Getters.
	if ( arrayNode == 0 ) return false;			// *** Should extract it into a local utility.
	
	XMP_CLTMatch match;
	const XMP_Node * itemNode;
	
	match = ChooseLocalizedText ( arrayNode, genericLang, specificLang, &itemNode );
	if ( match == kXMP_CLT_NoValues ) return false;
	
	*actualLang = itemNode->qualifiers[0]->value.c_str();
	*langSize   = itemNode->qualifiers[0]->value.size();
	*itemValue  = itemNode->value.c_str();
	*valueSize  = itemNode->value.size();
	*options    = itemNode->options;

	return true;
	
}	// GetLocalizedText


// -------------------------------------------------------------------------------------------------
// SetLocalizedText
// ----------------

void
XMPMeta::SetLocalizedText ( XMP_StringPtr  schemaNS,
							XMP_StringPtr  arrayName,
							XMP_StringPtr  _genericLang,
							XMP_StringPtr  _specificLang,
							XMP_StringPtr  itemValue,
							XMP_OptionBits options )
{
	options = options;	// Avoid unused parameter warning.

	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) && (_genericLang != 0) && (_specificLang != 0) );	// Enforced by wrapper.

	XMP_VarString zGenericLang  ( _genericLang );
	XMP_VarString zSpecificLang ( _specificLang );
	NormalizeLangValue ( &zGenericLang );
	NormalizeLangValue ( &zSpecificLang );
	
	XMP_StringPtr genericLang  = zGenericLang.c_str();
	XMP_StringPtr specificLang = zSpecificLang.c_str();
	
	XMP_ExpandedXPath arrayPath;
	ExpandXPath ( schemaNS, arrayName, &arrayPath );
	
	// Find the array node and set the options if it was just created.
	XMP_Node * arrayNode = FindNode ( &tree, arrayPath, kXMP_CreateNodes,
									  (kXMP_PropValueIsArray | kXMP_PropArrayIsOrdered | kXMP_PropArrayIsAlternate) );
	if ( arrayNode == 0 ) XMP_Throw ( "Failed to find or create array node", kXMPErr_BadXPath );
	if ( ! XMP_ArrayIsAltText(arrayNode->options) ) {
		if ( arrayNode->children.empty() && XMP_ArrayIsAlternate(arrayNode->options) ) {
			arrayNode->options |= kXMP_PropArrayIsAltText;
		} else {
			XMP_Throw ( "Localized text array is not alt-text", kXMPErr_BadXPath );
		}
	}
	
	// Make sure the x-default item, if any, is first.
	
	size_t itemNum, itemLim;
	XMP_Node * xdItem = 0;
	bool haveXDefault = false;
	
	for ( itemNum = 0, itemLim = arrayNode->children.size(); itemNum < itemLim; ++itemNum ) {
		XMP_Node * currItem = arrayNode->children[itemNum];
		XMP_Assert ( XMP_PropHasLang(currItem->options) );
		if ( currItem->qualifiers.empty() || (currItem->qualifiers[0]->name != "xml:lang") ) {
			XMP_Throw ( "Language qualifier must be first", kXMPErr_BadXPath );
		}
		if ( currItem->qualifiers[0]->value == "x-default" ) {
			xdItem = currItem;
			haveXDefault = true;
			break;
		}
	}
	
	if ( haveXDefault && (itemNum != 0) ) {
		XMP_Assert ( arrayNode->children[itemNum]->qualifiers[0]->value == "x-default" );
		XMP_Node * temp = arrayNode->children[0];
		arrayNode->children[0] = arrayNode->children[itemNum];
		arrayNode->children[itemNum] = temp;
	}
	
	// Find the appropriate item. ChooseLocalizedText will make sure the array is a language alternative.
		
	const XMP_Node * cItemNode;	// ! ChooseLocalizedText returns a pointer to a const node.
	XMP_CLTMatch match = ChooseLocalizedText ( arrayNode, genericLang, specificLang, &cItemNode );
	XMP_Node * itemNode = const_cast<XMP_Node*> ( cItemNode );

	const bool specificXDefault = XMP_LitMatch ( specificLang, "x-default" );
	
	switch ( match ) {

		case kXMP_CLT_NoValues :

			// Create the array items for the specificLang and x-default, with x-default first.
			AppendLangItem ( arrayNode, "x-default", itemValue );
			haveXDefault = true;
			if ( ! specificXDefault ) AppendLangItem ( arrayNode, specificLang, itemValue );
			break;
			
		case kXMP_CLT_SpecificMatch :
		
			if ( ! specificXDefault ) {
				// Update the specific item, update x-default if it matches the old value.
				if ( haveXDefault && (xdItem != itemNode) && (xdItem->value == itemNode->value) ) {
					SetNodeValue ( xdItem, itemValue );
				}
				SetNodeValue ( itemNode, itemValue );	// ! Do this after the x-default check!
			} else {
				// Update all items whose values match the old x-default value.
				XMP_Assert ( haveXDefault && (xdItem == itemNode) );
				for ( itemNum = 0, itemLim = arrayNode->children.size(); itemNum < itemLim; ++itemNum ) {
					XMP_Node * currItem = arrayNode->children[itemNum];
					if ( (currItem == xdItem) || (currItem->value != xdItem->value) ) continue;
					SetNodeValue ( currItem, itemValue );
				}
				SetNodeValue ( xdItem, itemValue );	// And finally do the x-default item.
			}
			break;

		case kXMP_CLT_SingleGeneric :
		
			// Update the generic item, update x-default if it matches the old value.
			if ( haveXDefault && (xdItem != itemNode) && (xdItem->value == itemNode->value) ) {
				SetNodeValue ( xdItem, itemValue );
			}
			SetNodeValue ( itemNode, itemValue );	// ! Do this after the x-default check!
			break;

		case kXMP_CLT_MultipleGeneric :
		
			// Create the specific language, ignore x-default.
			AppendLangItem ( arrayNode, specificLang, itemValue );
			if ( specificXDefault ) haveXDefault = true;
			break;
			
		case kXMP_CLT_XDefault :

			// Create the specific language, update x-default if it was the only item.
			if ( arrayNode->children.size() == 1 ) SetNodeValue ( xdItem, itemValue );
			AppendLangItem ( arrayNode, specificLang, itemValue );
			break;

		case kXMP_CLT_FirstItem	:

			// Create the specific language, don't add an x-default item.
			AppendLangItem ( arrayNode, specificLang, itemValue );
			if ( specificXDefault ) haveXDefault = true;
			break;
			
		default :
			XMP_Throw ( "Unexpected result from ChooseLocalizedText", kXMPErr_InternalFailure );

	}

	// Add an x-default at the front if needed.
	if ( (! haveXDefault) && (arrayNode->children.size() == 1) ) {
		AppendLangItem ( arrayNode, "x-default", itemValue );
	}

}	// SetLocalizedText


// -------------------------------------------------------------------------------------------------
// GetProperty_Bool
// ----------------

bool
XMPMeta::GetProperty_Bool ( XMP_StringPtr	 schemaNS,
							XMP_StringPtr	 propName,
							bool *			 propValue,
							XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (propValue != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	bool found = GetProperty ( schemaNS, propName, &valueStr, &valueLen, options );
	if ( found ) {
		if ( ! XMP_PropIsSimple ( *options ) ) XMP_Throw ( "Property must be simple", kXMPErr_BadXPath );
		*propValue = XMPUtils::ConvertToBool ( valueStr );
	}
	return found;
	
}	// GetProperty_Bool


// -------------------------------------------------------------------------------------------------
// GetProperty_Int
// ---------------

bool
XMPMeta::GetProperty_Int ( XMP_StringPtr	schemaNS,
						   XMP_StringPtr	propName,
						   XMP_Int32 *		propValue,
						   XMP_OptionBits *	options ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (propValue != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	bool found = GetProperty ( schemaNS, propName, &valueStr, &valueLen, options );
	if ( found ) {
		if ( ! XMP_PropIsSimple ( *options ) ) XMP_Throw ( "Property must be simple", kXMPErr_BadXPath );
		*propValue = XMPUtils::ConvertToInt ( valueStr );
	}
	return found;
	
}	// GetProperty_Int


// -------------------------------------------------------------------------------------------------
// GetProperty_Int64
// -----------------

bool
XMPMeta::GetProperty_Int64 ( XMP_StringPtr	  schemaNS,
						     XMP_StringPtr	  propName,
						     XMP_Int64 *	  propValue,
						     XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (propValue != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	bool found = GetProperty ( schemaNS, propName, &valueStr, &valueLen, options );
	if ( found ) {
		if ( ! XMP_PropIsSimple ( *options ) ) XMP_Throw ( "Property must be simple", kXMPErr_BadXPath );
		*propValue = XMPUtils::ConvertToInt64 ( valueStr );
	}
	return found;
	
}	// GetProperty_Int64


// -------------------------------------------------------------------------------------------------
// GetProperty_Float
// -----------------

bool
XMPMeta::GetProperty_Float ( XMP_StringPtr	  schemaNS,
							 XMP_StringPtr	  propName,
							 double *		  propValue,
							 XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (propValue != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	bool found = GetProperty ( schemaNS, propName, &valueStr, &valueLen, options );
	if ( found ) {
		if ( ! XMP_PropIsSimple ( *options ) ) XMP_Throw ( "Property must be simple", kXMPErr_BadXPath );
		*propValue = XMPUtils::ConvertToFloat ( valueStr );
	}
	return found;
	
}	// GetProperty_Float


// -------------------------------------------------------------------------------------------------
// GetProperty_Date
// ----------------

bool
XMPMeta::GetProperty_Date ( XMP_StringPtr	 schemaNS,
							XMP_StringPtr	 propName,
							XMP_DateTime *	 propValue,
							XMP_OptionBits * options ) const
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.
	XMP_Assert ( (propValue != 0) && (options != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	bool found = GetProperty ( schemaNS, propName, &valueStr, &valueLen, options );
	if ( found )  {
		if ( ! XMP_PropIsSimple ( *options ) ) XMP_Throw ( "Property must be simple", kXMPErr_BadXPath );
		XMPUtils::ConvertToDate ( valueStr, propValue );
	}
	return found;
	
}	// GetProperty_Date


// -------------------------------------------------------------------------------------------------
// SetProperty_Bool
// ----------------

void
XMPMeta::SetProperty_Bool ( XMP_StringPtr  schemaNS,
							XMP_StringPtr  propName,
							bool		   propValue,
							XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	XMPUtils::ConvertFromBool ( propValue, &valueStr, &valueLen );
	SetProperty ( schemaNS, propName, valueStr, options );
	
}	// SetProperty_Bool


// -------------------------------------------------------------------------------------------------
// SetProperty_Int
// ---------------

void
XMPMeta::SetProperty_Int ( XMP_StringPtr  schemaNS,
						   XMP_StringPtr  propName,
						   XMP_Int32	  propValue,
						   XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	XMPUtils::ConvertFromInt ( propValue, "", &valueStr, &valueLen );
	SetProperty ( schemaNS, propName, valueStr, options );
	
}	// SetProperty_Int


// -------------------------------------------------------------------------------------------------
// SetProperty_Int64
// -----------------

void
XMPMeta::SetProperty_Int64 ( XMP_StringPtr  schemaNS,
						     XMP_StringPtr  propName,
						     XMP_Int64	    propValue,
						     XMP_OptionBits options )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	XMPUtils::ConvertFromInt64 ( propValue, "", &valueStr, &valueLen );
	SetProperty ( schemaNS, propName, valueStr, options );
	
}	// SetProperty_Int64


// -------------------------------------------------------------------------------------------------
// SetProperty_Float
// -----------------

void
XMPMeta::SetProperty_Float ( XMP_StringPtr	schemaNS,
							 XMP_StringPtr	propName,
							 double			propValue,
							 XMP_OptionBits	options )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	XMPUtils::ConvertFromFloat ( propValue, "", &valueStr, &valueLen );
	SetProperty ( schemaNS, propName, valueStr, options );
	
}	// SetProperty_Float


// -------------------------------------------------------------------------------------------------
// SetProperty_Date
// ----------------

void
XMPMeta::SetProperty_Date ( XMP_StringPtr		   schemaNS,
							XMP_StringPtr		   propName,
							const	XMP_DateTime & propValue,
							XMP_OptionBits		   options )
{
	XMP_Assert ( (schemaNS != 0) && (propName != 0) );	// Enforced by wrapper.

	XMP_StringPtr	valueStr;
	XMP_StringLen	valueLen;
	
	XMPUtils::ConvertFromDate ( propValue, &valueStr, &valueLen );
	SetProperty ( schemaNS, propName, valueStr, options );
	
}	// SetProperty_Date

// =================================================================================================

