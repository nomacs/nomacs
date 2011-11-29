// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMP_Version.h"
#include "XMPCore_Impl.hpp"
#include "XMPMeta.hpp"	// *** For use of GetNamespacePrefix in FindSchemaNode.

#include "UnicodeInlines.incl_cpp"

#include <algorithm>

using namespace std;

#if XMP_WinBuild
    #ifdef _MSC_VER
        #pragma warning ( disable : 4290 )	// C++ exception specification ignored except ... not __declspec(nothrow)
        #pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
    #endif
#endif

// *** Add debug codegen checks, e.g. that typical masking operations really work
// *** Make option constants 0x...UL.

// Internal code should be using #if with XMP_MacBuild, XMP_WinBuild, or XMP_UNIXBuild.
// This is a sanity check in case of accidental use of *_ENV. Some clients use the poor
// practice of defining the *_ENV macro with an empty value.
#if defined ( MAC_ENV )
	#if ! MAC_ENV
		#error "MAC_ENV must be defined so that \"#if MAC_ENV\" is true"
	#endif
#elif defined ( WIN_ENV )
	#if ! WIN_ENV
		#error "WIN_ENV must be defined so that \"#if WIN_ENV\" is true"
	#endif
#elif defined ( UNIX_ENV )
	#if ! UNIX_ENV
		#error "UNIX_ENV must be defined so that \"#if UNIX_ENV\" is true"
	#endif
#endif

// =================================================================================================
// Static Variables
// ================

XMP_Int32 sXMP_InitCount = 0;

XMP_StringMap *	sNamespaceURIToPrefixMap = 0;
XMP_StringMap *	sNamespacePrefixToURIMap = 0;

XMP_AliasMap *	sRegisteredAliasMap = 0;	// Needed by XMPIterator.

XMP_VarString *	sOutputNS  = 0;
XMP_VarString *	sOutputStr = 0;
XMP_VarString * sExceptionMessage = 0;

XMP_Mutex sXMPCoreLock;
int sLockCount = 0;

#if TraceXMPCalls
	FILE * xmpOut = stderr;
#endif

void *              voidVoidPtr    = 0;	// Used to backfill null output parameters.
XMP_StringPtr		voidStringPtr  = 0;
XMP_StringLen		voidStringLen  = 0;
XMP_OptionBits		voidOptionBits = 0;
XMP_Uns8			voidByte       = 0;
bool				voidBool       = 0;
XMP_Int32			voidInt32      = 0;
XMP_Int64			voidInt64      = 0;
double				voidDouble     = 0.0;
XMP_DateTime		voidDateTime;
WXMP_Result 		void_wResult;

// =================================================================================================
// Mutex Utilities
// ===============

// ! Note that the mutex need not be "recursive", allowing the same thread to acquire it multiple
// ! times. There is a single XMP lock which is acquired in the wrapper classes. Internal calls
// ! never go back out to the wrappers.

#if XMP_WinBuild

	bool XMP_InitMutex ( XMP_Mutex * mutex ) {
		InitializeCriticalSection ( mutex );
		return true;
	}
	
	void XMP_TermMutex ( XMP_Mutex & mutex ) {
		DeleteCriticalSection ( &mutex );
	}

	void XMP_EnterCriticalRegion ( XMP_Mutex & mutex ) {
		EnterCriticalSection ( &mutex );
	}
	
	void XMP_ExitCriticalRegion ( XMP_Mutex & mutex ) {
		LeaveCriticalSection ( &mutex );
	}

#else

	// Use pthread for both Mac and generic UNIX.
	// ! Would be nice to specify PTHREAD_MUTEX_ERRORCHECK, but the POSIX documentation is useless.
	// ! Would be OK but overkill to specify PTHREAD_MUTEX_RECURSIVE.

	bool XMP_InitMutex ( XMP_Mutex * mutex ) {
		int err = pthread_mutex_init ( mutex, 0 );
		return (err == 0 );
	}
	
	void XMP_TermMutex ( XMP_Mutex & mutex ) {
		(void) pthread_mutex_destroy ( &mutex );
	}

	void XMP_EnterCriticalRegion ( XMP_Mutex & mutex ) {
		int err = pthread_mutex_lock ( &mutex );
		if ( err != 0 ) XMP_Throw ( "XMP_EnterCriticalRegion - pthread_mutex_lock failure", kXMPErr_ExternalFailure );
	}
	
	void XMP_ExitCriticalRegion ( XMP_Mutex & mutex ) {
		int err = pthread_mutex_unlock ( &mutex );
		if ( err != 0 ) XMP_Throw ( "XMP_ExitCriticalRegion - pthread_mutex_unlock failure", kXMPErr_ExternalFailure );
	}

#endif

// =================================================================================================
// Local Utilities
// ===============

// -------------------------------------------------------------------------------------------------
// VerifyXPathRoot
// ---------------
//
// Set up the first 2 components of the expanded XPath. Normalizes the various cases of using the
// full schema URI and/or a qualified root property name. Returns true for normal processing. If
// allowUnknownSchemaNS is true and the schema namespace is not registered, false is returned. If
// allowUnknownSchemaNS is false and the schema namespace is not registered, an exception is thrown.

// *** Should someday check the full syntax.

static void
VerifyXPathRoot	( XMP_StringPtr			schemaURI,
				  XMP_StringPtr			propName,
				  XMP_ExpandedXPath *	expandedXPath )
{
	// Do some basic checks on the URI and name. Try to lookup the URI. See if the name is qualified.
	
	XMP_Assert ( (schemaURI != 0) && (propName != 0) && (*propName != 0) );
	XMP_Assert ( (expandedXPath != 0) && (expandedXPath->empty()) );

	if ( *schemaURI == 0 ) XMP_Throw ( "Schema namespace URI is required", kXMPErr_BadSchema );

	if ( (*propName == '?') || (*propName == '@') ) {
		XMP_Throw ( "Top level name must not be a qualifier", kXMPErr_BadXPath );
	}
	for ( XMP_StringPtr ch = propName; *ch != 0; ++ch ) {
		if ( (*ch == '/') || (*ch == '[') ) {
			XMP_Throw ( "Top level name must be simple", kXMPErr_BadXPath );
		}
	}

	XMP_StringMapPos uriPos = sNamespaceURIToPrefixMap->find ( XMP_VarString ( schemaURI ) );
	if ( uriPos == sNamespaceURIToPrefixMap->end() ) {
		XMP_Throw ( "Unregistered schema namespace URI", kXMPErr_BadSchema );
	}

	XMP_StringPtr colonPos = propName;
	while ( (*colonPos != 0) && (*colonPos != ':') ) ++colonPos;
	VerifySimpleXMLName ( propName, colonPos );	// Verify the part before any colon.

	// Verify the various URI and prefix combinations. Initialize the expanded XPath.
	
	if ( *colonPos == 0 ) {
	
		// The propName is unqualified, use the schemaURI and associated prefix.
		
		expandedXPath->push_back ( XPathStepInfo ( schemaURI, kXMP_SchemaNode ) );
		expandedXPath->push_back ( XPathStepInfo ( uriPos->second, 0 ) );
		(*expandedXPath)[kRootPropStep].step += propName;
	
	} else {

		// The propName is qualified. Make sure the prefix is legit. Use the associated URI and qualified name.

		size_t prefixLen = colonPos - propName + 1;	// ! Include the colon.
		VerifySimpleXMLName ( colonPos+1, colonPos+strlen(colonPos) );

		XMP_VarString prefix ( propName, prefixLen );
		XMP_StringMapPos prefixPos = sNamespacePrefixToURIMap->find ( prefix );
		if ( prefixPos == sNamespacePrefixToURIMap->end() ) {
			XMP_Throw ( "Unknown schema namespace prefix", kXMPErr_BadSchema );
		}
		if ( prefix != uriPos->second ) {
			XMP_Throw ( "Schema namespace URI and prefix mismatch", kXMPErr_BadSchema );
		}

		expandedXPath->push_back ( XPathStepInfo ( schemaURI, kXMP_SchemaNode ) );
		expandedXPath->push_back ( XPathStepInfo ( propName, 0 ) );
	
	}

}	// VerifyXPathRoot

// -------------------------------------------------------------------------------------------------
// VerifyQualName
// --------------

static void
VerifyQualName ( XMP_StringPtr qualName, XMP_StringPtr nameEnd )
{
	if ( qualName >= nameEnd ) XMP_Throw ( "Empty qualified name", kXMPErr_BadXPath );

	XMP_StringPtr colonPos = qualName;
	while ( (colonPos < nameEnd) && (*colonPos != ':') ) ++colonPos;
	if ( (colonPos == qualName) || (colonPos >= nameEnd) ) XMP_Throw ( "Ill-formed qualified name", kXMPErr_BadXPath );

	VerifySimpleXMLName ( qualName, colonPos );
	VerifySimpleXMLName ( colonPos+1, nameEnd );

	size_t prefixLen = colonPos - qualName + 1;	// ! Include the colon.
	XMP_VarString prefix ( qualName, prefixLen );
	XMP_StringMapPos prefixPos = sNamespacePrefixToURIMap->find ( prefix );
	if ( prefixPos == sNamespacePrefixToURIMap->end() ) {
		XMP_Throw ( "Unknown namespace prefix for qualified name", kXMPErr_BadXPath );
	}

}	// VerifyQualName

// -------------------------------------------------------------------------------------------------
// FindIndexedItem
// ---------------
//
//	[index]	An element of an array.
//
// Support the implicit creation of a new last item.

static XMP_Index
FindIndexedItem ( XMP_Node * arrayNode, const XMP_VarString & indexStep, bool createNodes )
{
	XMP_Index index = 0;
	size_t    chLim = indexStep.size() - 1;

	XMP_Assert ( (chLim >= 2) && (indexStep[0] == '[') && (indexStep[chLim] == ']') );
	
	for ( size_t chNum = 1; chNum != chLim; ++chNum ) {
		XMP_Assert ( ('0' <= indexStep[chNum]) && (indexStep[chNum] <= '9') );
		index = (index * 10) + (indexStep[chNum] - '0');
		if ( index < 0 ) {
			XMP_Throw ( "Array index overflow", kXMPErr_BadXPath );	// ! Overflow, not truly negative.
		}
	}

	--index;	// Change to a C-style, zero based index.
	if ( index < 0 ) XMP_Throw ( "Array index must be larger than zero", kXMPErr_BadXPath );

	if ( (index == (XMP_Index)arrayNode->children.size()) && createNodes ) {	// Append a new last+1 node.
		XMP_Node * newItem = new XMP_Node ( arrayNode, kXMP_ArrayItemName, kXMP_NewImplicitNode );
		arrayNode->children.push_back ( newItem );
	}

	// ! Don't throw here for a too large index. SetProperty will throw, GetProperty will not.
	if ( index >= (XMP_Index)arrayNode->children.size() ) index = -1;
	return index;
	
}	// FindIndexedItem

// -------------------------------------------------------------------------------------------------
// SplitNameAndValue
// -----------------
//
// Split the name and value parts for field and qualifier selectors:
//
//	[qualName="value"]	An element in an array of structs, chosen by a field value.
//	[?qualName="value"]	An element in an array, chosen by a qualifier value.
//
// The value portion is a string quoted by ''' or '"'. The value may contain any character including
// a doubled quoting character. The value may be empty.

static void
SplitNameAndValue ( const XMP_VarString & selStep, XMP_VarString * nameStr, XMP_VarString * valueStr )
{
	XMP_StringPtr partBegin = selStep.c_str();
	XMP_StringPtr partEnd;
	
	const XMP_StringPtr valueEnd = partBegin + (selStep.size() - 2);
	const char          quote    = *valueEnd;
	
	XMP_Assert ( (*partBegin == '[') && (*(valueEnd+1) == ']') );
	XMP_Assert ( (selStep.size() >= 6) && ((quote == '"') || (quote == '\'')) );

	// Extract the name part.
	
	++partBegin;	// Skip the opening '['.
	if ( *partBegin == '?' ) ++partBegin;
	for ( partEnd = partBegin+1; *partEnd != '='; ++partEnd ) {};
	
	nameStr->assign ( partBegin, (partEnd - partBegin) );
	
	// Extract the value part, reducing doubled quotes.
	
	XMP_Assert ( *(partEnd+1) == quote );
	
	partBegin = partEnd + 2;
	valueStr->erase();
	valueStr->reserve ( valueEnd - partBegin );	// Maximum length, don't optimize doubled quotes.
	
	for ( partEnd = partBegin; partEnd < valueEnd; ++partEnd ) {
		if ( (*partEnd == quote) && (*(partEnd+1) == quote) ) {
			++partEnd;
			valueStr->append ( partBegin, (partEnd - partBegin) );
			partBegin = partEnd+1;	// ! Loop will increment partEnd again.
		}
	}

	valueStr->append ( partBegin, (partEnd - partBegin) );	// ! The loop does not add the last part.

}	// SplitNameAndValue

// -------------------------------------------------------------------------------------------------
// LookupQualSelector
// ------------------
//
//	[?qualName="value"]	An element in an array, chosen by a qualifier value.
//
// Note that we don't create implicit nodes for qualifier selectors, so no CreateNodes parameter.

static XMP_Index
LookupQualSelector ( XMP_Node * arrayNode, const XMP_VarString & qualName, XMP_VarString & qualValue )
{
	XMP_Index index;
		
	if ( qualName == "xml:lang" ) {
	
		// *** Should check that the value is legit RFC 1766/3066.
		NormalizeLangValue ( &qualValue );
		index = LookupLangItem ( arrayNode, qualValue ) ;
	
	} else {

		XMP_Index itemLim;
		for ( index = 0, itemLim = arrayNode->children.size(); index != itemLim; ++index ) {

			const XMP_Node * currItem = arrayNode->children[index];
			XMP_Assert ( currItem->parent == arrayNode );
			
			size_t q, qualLim;
			for ( q = 0, qualLim = currItem->qualifiers.size(); q != qualLim; ++q ) {
				const XMP_Node * currQual = currItem->qualifiers[q];
				XMP_Assert ( currQual->parent == currItem );
				if ( currQual->name != qualName ) continue;
				if ( currQual->value == qualValue ) break;	// Exit qual loop.
			}
			if ( q != qualLim ) break;	// Exit child loop, found an item with a matching qualifier.

		}
		if ( index == itemLim ) index = -1;

	}
	
	return index;
	
}	// LookupQualSelector

// -------------------------------------------------------------------------------------------------
// FollowXPathStep
// ---------------
//
// After processing by ExpandXPath, a step can be of these forms:
//	qualName			A top level property or struct field.
//	[index]				An element of an array.
//	[last()]			The last element of an array.
//	[qualName="value"]	An element in an array of structs, chosen by a field value.
//	[?qualName="value"]	An element in an array, chosen by a qualifier value.
//	?qualName			A general qualifier.
//
// Find the appropriate child node, resolving aliases, and optionally creating nodes.

static XMP_Node *
FollowXPathStep	( XMP_Node *	   parentNode,
				  const XMP_ExpandedXPath & fullPath,
				  size_t		   stepNum,
				  bool			   createNodes,
				  XMP_NodePtrPos * ptrPos,
				  bool			   aliasedArrayItem = false )
{
	XMP_Node * nextNode = 0;
	const XPathStepInfo & nextStep = fullPath[stepNum];
	XMP_Index      index    = 0;
	XMP_OptionBits stepKind = nextStep.options & kXMP_StepKindMask;
	
	XMP_Assert ( (kXMP_StructFieldStep <= stepKind) && (stepKind <= kXMP_FieldSelectorStep) );

	if ( stepKind == kXMP_StructFieldStep ) {

		nextNode = FindChildNode ( parentNode, nextStep.step.c_str(), createNodes, ptrPos );

	} else if ( stepKind == kXMP_QualifierStep ) {
	
		XMP_StringPtr qualStep = nextStep.step.c_str();
		XMP_Assert ( *qualStep == '?' );
		++qualStep;
		nextNode = FindQualifierNode ( parentNode, qualStep, createNodes, ptrPos );

	} else {
	
		// This is an array indexing step. First get the index, then get the node.

		if ( ! (parentNode->options & kXMP_PropValueIsArray) ) {
			XMP_Throw ( "Indexing applied to non-array", kXMPErr_BadXPath );
		}
		
		if ( stepKind == kXMP_ArrayIndexStep ) {
			index = FindIndexedItem ( parentNode, nextStep.step, createNodes );
		} else if ( stepKind == kXMP_ArrayLastStep ) {
			index = parentNode->children.size() - 1;
		} else if ( stepKind == kXMP_FieldSelectorStep ) {
			XMP_VarString fieldName, fieldValue;
			SplitNameAndValue ( nextStep.step, &fieldName, &fieldValue );
			index = LookupFieldSelector ( parentNode, fieldName.c_str(), fieldValue.c_str() );
		} else if ( stepKind == kXMP_QualSelectorStep ) {
			XMP_VarString qualName, qualValue;
			SplitNameAndValue ( nextStep.step, &qualName, &qualValue );
			index = LookupQualSelector ( parentNode, qualName, qualValue );
		} else {
			XMP_Throw ( "Unknown array indexing step in FollowXPathStep", kXMPErr_InternalFailure );
		}
		
		if ( (0 <= index) && (index <= (XMP_Index)parentNode->children.size()) ) nextNode = parentNode->children[index];

		if ( (index == -1) && createNodes && aliasedArrayItem && (stepKind == kXMP_QualSelectorStep) ) {
		
			// An ugly special case without an obvious better place to be. We have an alias to the
			// x-default item of an alt-text array. A simple reference via SetProperty must create
			// the x-default item if it does not yet exist.
			
			XMP_Assert ( parentNode->options & kXMP_PropArrayIsAltText );
			XMP_Assert ( (stepNum == 2) && (nextStep.step == "[?xml:lang=\"x-default\"]") );

			nextNode = new XMP_Node ( parentNode, kXMP_ArrayItemName,
									  (kXMP_PropHasQualifiers | kXMP_PropHasLang | kXMP_NewImplicitNode) );

			XMP_Node * langQual = new XMP_Node ( nextNode, "xml:lang", "x-default", kXMP_PropIsQualifier );
			nextNode->qualifiers.push_back ( langQual );

			if ( parentNode->children.empty() ) {
				parentNode->children.push_back ( nextNode );
			} else {
				parentNode->children.insert ( parentNode->children.begin(), nextNode );
			}

			index = 0;	// ! C-style index! The x-default item is always first.

		}
		
		if ( (nextNode != 0) && (ptrPos != 0) ) *ptrPos = parentNode->children.begin() + index;
	
	}

	if ( (nextNode != 0) && (nextNode->options & kXMP_NewImplicitNode) ) {
		nextNode->options |= (nextStep.options & kXMP_PropArrayFormMask);
	}
	
	XMP_Assert ( (ptrPos == 0) || (nextNode == 0) || (nextNode == **ptrPos) );
	XMP_Assert ( (nextNode != 0) || (! createNodes) );
	return nextNode;
	
}	// FollowXPathStep

// -------------------------------------------------------------------------------------------------
// CheckImplicitStruct
// -------------------

static inline void
CheckImplicitStruct	( XMP_Node * node,
					  const XMP_ExpandedXPath &	expandedXPath,
					  size_t	 stepNum,
					  size_t	 stepLim )
{

	if ( (stepNum < stepLim) &&
		 ((node->options & kXMP_PropCompositeMask) == 0) &&
		 (GetStepKind ( expandedXPath[stepNum].options ) == kXMP_StructFieldStep) ) {

		node->options |= kXMP_PropValueIsStruct;

	}

}	// CheckImplicitStruct

// -------------------------------------------------------------------------------------------------
// DeleteSubtree
// -------------

// *** Might be useful elsewhere?

static void
DeleteSubtree ( XMP_NodePtrPos rootNodePos )
{
	XMP_Node * rootNode   = *rootNodePos;
	XMP_Node * rootParent = rootNode->parent;

	if ( ! (rootNode->options & kXMP_PropIsQualifier) ) {

		rootParent->children.erase ( rootNodePos );

	} else {

		rootParent->qualifiers.erase ( rootNodePos );

		XMP_Assert ( rootParent->options & kXMP_PropHasQualifiers);
		if ( rootParent->qualifiers.empty() ) rootParent->options ^= kXMP_PropHasQualifiers;

		if ( rootNode->name == "xml:lang" ) {
			XMP_Assert ( rootParent->options & kXMP_PropHasLang);
			rootParent->options ^= kXMP_PropHasLang;
		} else if ( rootNode->name == "rdf:type" ) {
			XMP_Assert ( rootParent->options & kXMP_PropHasType);
			rootParent->options ^= kXMP_PropHasType;
		}

	}

	delete rootNode;

}	// DeleteSubtree

// =================================================================================================
// =================================================================================================

// =================================================================================================
// VerifySetOptions
// ================
//
// Normalize and verify the option flags for SetProperty and similar functions. The allowed options
// here are just those that apply to the property, that would be kept in the XMP_Node. Others that
// affect the selection of the node or other processing must be removed by now. These are:
//	kXMP_InsertBeforeItem
//	kXMP_InsertAfterItem
//	kXMP_KeepQualifiers
//	kXMPUtil_AllowCommas

enum {
	kXMP_AllSetOptionsMask	= (kXMP_PropValueIsURI       |
							   kXMP_PropValueIsStruct    |
							   kXMP_PropValueIsArray     |
							   kXMP_PropArrayIsOrdered   |
							   kXMP_PropArrayIsAlternate |
							   kXMP_PropArrayIsAltText   |
							   kXMP_DeleteExisting)
};

XMP_OptionBits
VerifySetOptions ( XMP_OptionBits options, XMP_StringPtr propValue )
{

	if ( options & kXMP_PropArrayIsAltText )   options |= kXMP_PropArrayIsAlternate;
	if ( options & kXMP_PropArrayIsAlternate ) options |= kXMP_PropArrayIsOrdered;
	if ( options & kXMP_PropArrayIsOrdered )   options |= kXMP_PropValueIsArray;
	
	if ( options & ~kXMP_AllSetOptionsMask ) {
		XMP_Throw ( "Unrecognized option flags", kXMPErr_BadOptions );
	}
	
	if ( (options & kXMP_PropValueIsStruct) && (options & kXMP_PropValueIsArray) ) {
		XMP_Throw ( "IsStruct and IsArray options are mutually exclusive", kXMPErr_BadOptions );
	}
	
	if ( (options & kXMP_PropValueOptionsMask) && (options & kXMP_PropCompositeMask) ) {
		XMP_Throw ( "Structs and arrays can't have \"value\" options", kXMPErr_BadOptions );
	}
	
	if ( (propValue != 0) && (options & kXMP_PropCompositeMask) ) {
		XMP_Throw ( "Structs and arrays can't have string values", kXMPErr_BadOptions );
	}

	return options;

}	// VerifySetOptions

// =================================================================================================
// ComposeXPath
// ============
//
// Compose the canonical string form of an expanded XPath expression.

extern void
ComposeXPath ( const XMP_ExpandedXPath & expandedXPath,
			   XMP_VarString * stringXPath )
{
	*stringXPath = expandedXPath[kRootPropStep].step;

	for ( size_t index = kRootPropStep+1; index < expandedXPath.size(); ++index ) {
		const XPathStepInfo & currStep = expandedXPath[index];

		switch ( currStep.options & kXMP_StepKindMask ) {

			case kXMP_StructFieldStep :
			case kXMP_QualifierStep :
				*stringXPath += '/';
				*stringXPath += currStep.step;
				break;

			case kXMP_ArrayIndexStep :
			case kXMP_ArrayLastStep :
			case kXMP_QualSelectorStep :
			case kXMP_FieldSelectorStep :
				*stringXPath += currStep.step;
				break;

			default:
				XMP_Throw ( "Unexpected", kXMPErr_InternalFailure );

		}

	}

}	// ComposeXPath

// =================================================================================================
// ExpandXPath
// ===========
//
// Split an XPath expression apart at the conceptual steps, adding the root namespace prefix to the
// first property component. The schema URI is put in the first (0th) slot in the expanded XPath.
// Check if the top level component is an alias, but don't resolve it.
//
// In the most verbose case steps are separated by '/', and each step can be of these forms:
//
//	qualName				A top level property or struct field.
//	*[index]				An element of an array.
//	*[last()]				The last element of an array.
//	*[fieldName="value"]	An element in an array of structs, chosen by a field value.
//	*[@xml:lang="value"]	An element in an alt-text array, chosen by the xml:lang qualifier.
//	*[?qualName="value"]	An element in an array, chosen by a qualifier value.
//	@xml:lang				An xml:lang qualifier.
//	?qualName				A general qualifier.
//
// The logic is complicated though by shorthand for arrays, the separating '/' and leading '*'
// are optional. These are all equivalent:  array/*[2]  array/[2]  array*[2]  array[2]
// All of these are broken into the 2 steps "array" and "[2]".
//
// The value portion in the array selector forms is a string quoted by ''' or '"'. The value
// may contain any character including a doubled quoting character. The value may be empty.
//
// The syntax isn't checked, but an XML name begins with a letter or '_', and contains letters,
// digits, '.', '-', '_', and a bunch of special non-ASCII Unicode characters. An XML qualified
// name is a pair of names separated by a colon.

void
ExpandXPath	( XMP_StringPtr			schemaNS,
			  XMP_StringPtr			propPath,
			  XMP_ExpandedXPath *	expandedXPath )
{
	XMP_Assert ( (schemaNS != 0) && (propPath != 0) && (*propPath != 0) && (expandedXPath != 0) );
	
	XMP_StringPtr	stepBegin, stepEnd;
	XMP_StringPtr	qualName, nameEnd;
	XMP_VarString	currStep;
		
	qualName = nameEnd = NULL;
	size_t resCount = 2;	// Guess at the number of steps. At least 2, plus 1 for each '/' or '['.
	for ( stepEnd = propPath; *stepEnd != 0; ++stepEnd ) {
		if ( (*stepEnd == '/') || (*stepEnd == '[') ) ++resCount;
	}
	
	expandedXPath->clear();
	expandedXPath->reserve ( resCount );
	
	// -------------------------------------------------------------------------------------------
	// Pull out the first component and do some special processing on it: add the schema namespace
	// prefix and see if it is an alias. The start must be a qualName.
	
	stepBegin = propPath;
	stepEnd = stepBegin;
	while ( (*stepEnd != 0) && (*stepEnd != '/') && (*stepEnd != '[') && (*stepEnd != '*') ) ++stepEnd;
	if ( stepEnd == stepBegin ) XMP_Throw ( "Empty initial XPath step", kXMPErr_BadXPath );
	currStep.assign ( stepBegin, (stepEnd - stepBegin) );
	
	VerifyXPathRoot ( schemaNS, currStep.c_str(), expandedXPath );

	XMP_OptionBits stepFlags = kXMP_StructFieldStep;	
	if ( sRegisteredAliasMap->find ( (*expandedXPath)[kRootPropStep].step ) != sRegisteredAliasMap->end() ) {
		stepFlags |= kXMP_StepIsAlias;
	}
	(*expandedXPath)[kRootPropStep].options |= stepFlags;
		
	// -----------------------------------------------------
	// Now continue to process the rest of the XPath string.

	while ( *stepEnd != 0 ) {

		stepBegin = stepEnd;
		if ( *stepBegin == '/' ) ++stepBegin;
		if ( *stepBegin == '*' ) {
			++stepBegin;
			if ( *stepBegin != '[' ) XMP_Throw ( "Missing '[' after '*'", kXMPErr_BadXPath );
		}
		stepEnd = stepBegin;

		if ( *stepBegin != '[' ) {
		
			// A struct field or qualifier.
			qualName = stepBegin;
			while ( (*stepEnd != 0) && (*stepEnd != '/') && (*stepEnd != '[') && (*stepEnd != '*') ) ++stepEnd;
			nameEnd = stepEnd;
			stepFlags = kXMP_StructFieldStep;	// ! Touch up later, also changing '@' to '?'.
			
		} else {
		
			// One of the array forms.
		
			++stepEnd;	// Look at the character after the leading '['.
			
			if ( ('0' <= *stepEnd) && (*stepEnd <= '9') ) {

				// A numeric (decimal integer) array index.
				while ( (*stepEnd != 0) && ('0' <= *stepEnd) && (*stepEnd <= '9') ) ++stepEnd;
				if ( *stepEnd != ']' ) XMP_Throw ( "Missing ']' for integer array index", kXMPErr_BadXPath );
				stepFlags = kXMP_ArrayIndexStep;

			} else {

				// Could be "[last()]" or one of the selector forms. Find the ']' or '='.
				
				while ( (*stepEnd != 0) && (*stepEnd != ']') && (*stepEnd != '=') ) ++stepEnd;
				if ( *stepEnd == 0 ) XMP_Throw ( "Missing ']' or '=' for array index", kXMPErr_BadXPath );

				if ( *stepEnd == ']' ) {

					if ( strncmp ( "[last()", stepBegin, (stepEnd - stepBegin) ) != 0 ) {
						XMP_Throw ( "Invalid non-numeric array index", kXMPErr_BadXPath );
					}
					stepFlags = kXMP_ArrayLastStep;

				} else {

					qualName = stepBegin+1;
					nameEnd = stepEnd;
					++stepEnd;	// Absorb the '=', remember the quote.
					const char quote = *stepEnd;
					if ( (quote != '\'') && (quote != '"') ) {
						XMP_Throw ( "Invalid quote in array selector", kXMPErr_BadXPath );
					}

					++stepEnd;	// Absorb the leading quote.
					while ( *stepEnd != 0 ) {
						if ( *stepEnd == quote ) {
							if ( *(stepEnd+1) != quote ) break;
							++stepEnd;
						}
						++stepEnd;
					}
					if ( *stepEnd == 0 ) {
						XMP_Throw ( "No terminating quote for array selector", kXMPErr_BadXPath );
					}
					++stepEnd;	// Absorb the trailing quote.
					
					stepFlags = kXMP_FieldSelectorStep;	// ! Touch up later, also changing '@' to '?'.

				}

			}

			if ( *stepEnd != ']' ) XMP_Throw ( "Missing ']' for array index", kXMPErr_BadXPath );
			++stepEnd;
			
		}

		if ( stepEnd == stepBegin ) XMP_Throw ( "Empty XPath step", kXMPErr_BadXPath );
		currStep.assign ( stepBegin, (stepEnd - stepBegin) );

		if ( GetStepKind ( stepFlags ) == kXMP_StructFieldStep ) {

			if ( currStep[0] == '@' ) {
				currStep[0] = '?';
				if ( currStep != "?xml:lang" ) XMP_Throw ( "Only xml:lang allowed with '@'", kXMPErr_BadXPath );
			}
			if ( currStep[0] == '?' ) {
				++qualName;
				stepFlags = kXMP_QualifierStep;
			}
			VerifyQualName ( qualName, nameEnd );

		} else if ( GetStepKind ( stepFlags ) == kXMP_FieldSelectorStep ) {

			if ( currStep[1] == '@' ) {
				currStep[1] = '?';
				if ( strncmp ( currStep.c_str(), "[?xml:lang=", 11 ) != 0 ) {
					XMP_Throw ( "Only xml:lang allowed with '@'", kXMPErr_BadXPath );
				}
			}
			if ( currStep[1] == '?' ) {
				++qualName;
				stepFlags = kXMP_QualSelectorStep;
			}
			VerifyQualName ( qualName, nameEnd );

		}

		expandedXPath->push_back ( XPathStepInfo ( currStep, stepFlags ) );

	}

}	// ExpandXPath

// =================================================================================================
// FindSchemaNode
// ==============
//
// Find or create a schema node. Returns a pointer to the node, and optionally an iterator for the
// node's position in the top level vector of schema nodes. The iterator is unchanged if no schema
// node (null) is returned.

XMP_Node *
FindSchemaNode	( XMP_Node *		xmpTree,
				  XMP_StringPtr		nsURI,
				  bool				createNodes,
				  XMP_NodePtrPos *	ptrPos /* = 0 */ )
{
	XMP_Node * schemaNode = 0;
	
	XMP_Assert ( xmpTree->parent == 0 );
	
	for ( size_t schemaNum = 0, schemaLim = xmpTree->children.size(); schemaNum != schemaLim; ++schemaNum ) {
		XMP_Node * currSchema = xmpTree->children[schemaNum];
		XMP_Assert ( currSchema->parent == xmpTree );
		if ( currSchema->name == nsURI ) {
			schemaNode = currSchema;
			if ( ptrPos != 0 ) *ptrPos = xmpTree->children.begin() + schemaNum;
			break;
		}
	}
	
	if ( (schemaNode == 0) && createNodes ) {

		schemaNode = new XMP_Node ( xmpTree, nsURI, (kXMP_SchemaNode | kXMP_NewImplicitNode) );
		XMP_StringPtr prefixPtr;
		XMP_StringLen prefixLen;
        bool found = false;
        found = XMPMeta::GetNamespacePrefix ( nsURI, &prefixPtr, &prefixLen );	// *** Use map directly?
		XMP_Assert ( found );

		schemaNode->value.assign ( prefixPtr, prefixLen );
		xmpTree->children.push_back ( schemaNode );
		if ( ptrPos != 0 ) *ptrPos = xmpTree->children.end() - 1;

		#if 0	// *** XMP_DebugBuild
			schemaNode->_valuePtr = schemaNode->value.c_str();
		#endif

	}
	
	XMP_Assert ( (ptrPos == 0) || (schemaNode == 0) || (schemaNode == **ptrPos) );
	XMP_Assert ( (schemaNode != 0) || (! createNodes) );
	return schemaNode;
	
}	// FindSchemaNode

// =================================================================================================
// FindChildNode
// =============
//
// Find or create a child node under a given parent node. Returns a pointer to the child node, and
// optionally an iterator for the node's position in the parent's vector of children. The iterator
// is unchanged if no child node (null) is returned.

XMP_Node *
FindChildNode	( XMP_Node *		parent,
				  XMP_StringPtr		childName,
				  bool				createNodes,
				  XMP_NodePtrPos *	ptrPos /* = 0 */ )
{
	XMP_Node * childNode = 0;

	if ( ! (parent->options & (kXMP_SchemaNode | kXMP_PropValueIsStruct)) ) {
		if ( ! (parent->options & kXMP_NewImplicitNode) ) {
			XMP_Throw ( "Named children only allowed for schemas and structs", kXMPErr_BadXPath );
		}
		if ( parent->options & kXMP_PropValueIsArray ) {
			XMP_Throw ( "Named children not allowed for arrays", kXMPErr_BadXPath );
		}
		if ( ! createNodes ) {	// *** Should be assert? If !createNodes, why is the parent a new implicit node?
			XMP_Throw ( "Parent is new implicit node, but createNodes is false", kXMPErr_InternalFailure );
		}
		parent->options |= kXMP_PropValueIsStruct;
	}
	
	for ( size_t childNum = 0, childLim = parent->children.size(); childNum != childLim; ++childNum ) {
		XMP_Node * currChild = parent->children[childNum];
		XMP_Assert ( currChild->parent == parent );
		if ( currChild->name == childName ) {
			childNode = currChild;
			if ( ptrPos != 0 ) *ptrPos = parent->children.begin() + childNum;
			break;
		}
	}
	
	if ( (childNode == 0) && createNodes ) {
		childNode = new XMP_Node ( parent, childName, kXMP_NewImplicitNode );
		parent->children.push_back ( childNode );
		if ( ptrPos != 0 ) *ptrPos = parent->children.end() - 1;
	}
	
	XMP_Assert ( (ptrPos == 0) || (childNode == 0) || (childNode == **ptrPos) );
	XMP_Assert ( (childNode != 0) || (! createNodes) );
	return childNode;
	
}	// FindChildNode

// =================================================================================================
// FindQualifierNode
// =================
//
// Find or create a qualifier node under a given parent node. Returns a pointer to the qualifier node,
// and optionally an iterator for the node's position in the parent's vector of qualifiers. The iterator
// is unchanged if no qualifier node (null) is returned.
//
// ! On entry, the qualName parameter must not have the leading '?' from the XPath step.

XMP_Node *
FindQualifierNode	( XMP_Node *		parent,
					  XMP_StringPtr		qualName,
					  bool				createNodes,
					  XMP_NodePtrPos *	ptrPos /* = 0 */ )	// *** Require ptrPos internally & remove checks?
{
	XMP_Node * qualNode = 0;
	
	XMP_Assert ( *qualName != '?' );
	
	for ( size_t qualNum = 0, qualLim = parent->qualifiers.size(); qualNum != qualLim; ++qualNum ) {
		XMP_Node * currQual = parent->qualifiers[qualNum];
		XMP_Assert ( currQual->parent == parent );
		if ( currQual->name == qualName ) {
			qualNode = currQual;
			if ( ptrPos != 0 ) *ptrPos = parent->qualifiers.begin() + qualNum;
			break;
		}
	}
	
	if ( (qualNode == 0) && createNodes ) {

		qualNode = new XMP_Node ( parent, qualName, (kXMP_PropIsQualifier | kXMP_NewImplicitNode) );
		parent->options |= kXMP_PropHasQualifiers;

		const bool isLang 	 = XMP_LitMatch ( qualName, "xml:lang" );
		const bool isType 	 = XMP_LitMatch ( qualName, "rdf:type" );
		const bool isSpecial = isLang | isType;

		if ( isLang ) {
			parent->options |= kXMP_PropHasLang;
		} else if ( isType ) {
			parent->options |= kXMP_PropHasType;
		}
		
		if ( parent->qualifiers.empty() || (! isSpecial) ) {
			parent->qualifiers.push_back ( qualNode );
			if ( ptrPos != 0 ) *ptrPos = parent->qualifiers.end() - 1;
		} else {
			XMP_NodePtrPos insertPos = parent->qualifiers.begin();	// ! Lang goes first, type after.
			if ( isType && (parent->options & kXMP_PropHasLang) ) ++insertPos;	// *** Does insert at end() work?
			insertPos = parent->qualifiers.insert ( insertPos, qualNode );
			if ( ptrPos != 0 ) *ptrPos = insertPos;
		}

	}
	
	XMP_Assert ( (ptrPos == 0) || (qualNode == 0) || (qualNode == **ptrPos) );
	XMP_Assert ( (qualNode != 0) || (! createNodes) );
	return qualNode;
	
}	// FindQualifierNode

// =================================================================================================
// LookupFieldSelector
// ===================
//
//	[fieldName="value"]	An element in an array of structs, chosen by a field value.
//
// Note that we don't create implicit nodes for field selectors, so no CreateNodes parameter.

XMP_Index
LookupFieldSelector ( const XMP_Node * arrayNode, XMP_StringPtr fieldName, XMP_StringPtr fieldValue )
{
	XMP_Index index, itemLim;
	
	for ( index = 0, itemLim = arrayNode->children.size(); index != itemLim; ++index ) {

		const XMP_Node * currItem = arrayNode->children[index];
		XMP_Assert ( currItem->parent == arrayNode );

		if ( ! (currItem->options & kXMP_PropValueIsStruct) ) {
			XMP_Throw ( "Field selector must be used on array of struct", kXMPErr_BadXPath );
		}

		size_t f, fieldLim;
		for ( f = 0, fieldLim = currItem->children.size(); f != fieldLim; ++f ) {
			const XMP_Node * currField = currItem->children[f];
			XMP_Assert ( currField->parent == currItem );
			if ( currField->name != fieldName ) continue;
			if ( currField->value == fieldValue ) break;	// Exit qual loop.
		}
		if ( f != fieldLim ) break;	// Exit child loop, found an item with a matching qualifier.

	}
	
	if ( index == itemLim ) index = -1;
	return index;
	
}	// LookupFieldSelector

// =================================================================================================
// LookupLangItem
// ==============
//
// ! Assumes that the language value is already normalized.

XMP_Index
LookupLangItem ( const XMP_Node * arrayNode, XMP_VarString & lang )
{
	if ( ! (arrayNode->options & kXMP_PropValueIsArray) ) {	// *** Check for alt-text?
		XMP_Throw ( "Language item must be used on array", kXMPErr_BadXPath );
	}

	XMP_Index index   = 0;
	XMP_Index itemLim = arrayNode->children.size();
	
	for ( ; index != itemLim; ++index ) {
		const XMP_Node * currItem = arrayNode->children[index];
		XMP_Assert ( currItem->parent == arrayNode );
		if ( currItem->qualifiers.empty() || (currItem->qualifiers[0]->name != "xml:lang") ) continue;
		if ( currItem->qualifiers[0]->value == lang ) break;
	}
	
	if ( index == itemLim ) index = -1;
	return index;
	
}	// LookupLangItem

// =================================================================================================
// FindNode
// ========
//
// Follow an expanded path expression to find or create a node. Returns a pointer to the node, and
// optionally an iterator for the node's position in the parent's vector of children or qualifiers.
// The iterator is unchanged if no child node (null) is returned.

XMP_Node *
FindNode ( XMP_Node *		xmpTree,
		   const XMP_ExpandedXPath & expandedXPath,
		   bool				createNodes,
		   XMP_OptionBits	leafOptions /* = 0 */,
	 	   XMP_NodePtrPos * ptrPos /* = 0 */ )
{
	XMP_Node *     currNode = 0;
	XMP_NodePtrPos currPos;
	XMP_NodePtrPos newSubPos;	// Root of implicitly created subtree. Valid only if leaf is new.
	bool           leafIsNew = false;
	
	XMP_Assert ( (leafOptions == 0) || createNodes );

	if ( expandedXPath.empty() ) XMP_Throw ( "Empty XPath", kXMPErr_BadXPath );
	
	size_t stepNum = 1;	// By default start calling FollowXPathStep for the top level property step.
	size_t stepLim = expandedXPath.size();
	
	// The start of processing deals with the schema node and top level alias. If the top level step
	// is not an alias, lookup the expanded path's schema URI. Otherwise, lookup the expanded path
	// for the actual. While tempting, don't substitute the actual's path into the local one, don't
	// risk messing with the caller's use of that. Also don't call FindNode recursively, we need to
	// keep track of the root of the implicitly created subtree as we move down the path.
	
	if ( ! (expandedXPath[kRootPropStep].options & kXMP_StepIsAlias) ) {
		
		currNode = FindSchemaNode ( xmpTree, expandedXPath[kSchemaStep].step.c_str(), createNodes, &currPos );
		if ( currNode == 0 ) return 0;

		if ( currNode->options & kXMP_NewImplicitNode ) {
			currNode->options ^= kXMP_NewImplicitNode;	// Clear the implicit node bit.
			if ( ! leafIsNew ) newSubPos = currPos;	// Save the top most implicit node.
			leafIsNew = true;	// If any parent is new, the leaf will be new also.
		}

	} else {

		stepNum = 2;	// ! Continue processing the original path at the second level step.

		XMP_AliasMapPos aliasPos = sRegisteredAliasMap->find ( expandedXPath[kRootPropStep].step );
		XMP_Assert ( aliasPos != sRegisteredAliasMap->end() );
		
		currNode = FindSchemaNode ( xmpTree, aliasPos->second[kSchemaStep].step.c_str(), createNodes, &currPos );
		if ( currNode == 0 ) goto EXIT;
		if ( currNode->options & kXMP_NewImplicitNode ) {
			currNode->options ^= kXMP_NewImplicitNode;	// Clear the implicit node bit.
			if ( ! leafIsNew ) newSubPos = currPos;	// Save the top most implicit node.
			leafIsNew = true;	// If any parent is new, the leaf will be new also.
		}

		currNode = FollowXPathStep ( currNode, aliasPos->second, 1, createNodes, &currPos );
		if ( currNode == 0 ) goto EXIT;
		if ( currNode->options & kXMP_NewImplicitNode ) {
			currNode->options ^= kXMP_NewImplicitNode;	// Clear the implicit node bit.
			CheckImplicitStruct ( currNode, expandedXPath, 2, stepLim );
			if ( ! leafIsNew ) newSubPos = currPos;	// Save the top most implicit node.
			leafIsNew = true;	// If any parent is new, the leaf will be new also.
		}
		
		XMP_OptionBits arrayForm = aliasPos->second[kRootPropStep].options & kXMP_PropArrayFormMask;
		XMP_Assert ( (arrayForm == 0) || (arrayForm & kXMP_PropValueIsArray) );
		XMP_Assert ( (arrayForm == 0) ? (aliasPos->second.size() == 2) : (aliasPos->second.size() == 3) );
		
		if ( arrayForm != 0 ) { 
			currNode = FollowXPathStep ( currNode, aliasPos->second, 2, createNodes, &currPos, true );
			if ( currNode == 0 ) goto EXIT;
			if ( currNode->options & kXMP_NewImplicitNode ) {
				currNode->options ^= kXMP_NewImplicitNode;	// Clear the implicit node bit.
				CheckImplicitStruct ( currNode, expandedXPath, 2, stepLim );
				if ( ! leafIsNew ) newSubPos = currPos;	// Save the top most implicit node.
				leafIsNew = true;	// If any parent is new, the leaf will be new also.
			}
		}
		
	}
	
	// Now follow the remaining steps of the original XPath.
	
	// *** ??? Change all the num/lim loops back to num<lim? Probably safer.
	
	try {
		for ( ; stepNum < stepLim; ++stepNum ) {
			currNode = FollowXPathStep ( currNode, expandedXPath, stepNum, createNodes, &currPos );
			if ( currNode == 0 ) goto EXIT;
			if ( currNode->options & kXMP_NewImplicitNode ) {
				currNode->options ^= kXMP_NewImplicitNode;	// Clear the implicit node bit.
				CheckImplicitStruct ( currNode, expandedXPath, stepNum+1, stepLim );
				if ( ! leafIsNew ) newSubPos = currPos;	// Save the top most implicit node.
				leafIsNew = true;	// If any parent is new, the leaf will be new also.
			}
		}
	} catch ( ... ) {
		if ( leafIsNew ) DeleteSubtree ( newSubPos );
		throw;
	}
	
	// Done. Delete the implicitly created subtree if the eventual node was not found.

EXIT:

	XMP_Assert ( (currNode == 0) || (currNode == *currPos) );
	XMP_Assert ( (currNode != 0) || (! createNodes) );

	if ( leafIsNew ) {
		if ( currNode != 0 ) {
			currNode->options |= leafOptions;
		} else {
			DeleteSubtree ( newSubPos );
		}
	}

	if ( (currNode != 0) && (ptrPos != 0) ) *ptrPos = currPos;
	return currNode;
	
}	// FindNode

// =================================================================================================
// CloneOffspring
// ==============

void
CloneOffspring ( const XMP_Node * origParent, XMP_Node * cloneParent )
{
	size_t qualCount  = origParent->qualifiers.size();
	size_t childCount = origParent->children.size();

	if ( qualCount > 0 ) {

		cloneParent->qualifiers.reserve ( qualCount );

		for ( size_t qualNum = 0, qualLim = qualCount; qualNum != qualLim; ++qualNum ) {
			const XMP_Node * origQual  = origParent->qualifiers[qualNum];
			XMP_Node *       cloneQual = new XMP_Node ( cloneParent, origQual->name, origQual->value, origQual->options );
			CloneOffspring ( origQual, cloneQual );
			cloneParent->qualifiers.push_back ( cloneQual );
		}

	}
	
	if ( childCount > 0 ) {

		cloneParent->children.reserve ( childCount );

		for ( size_t childNum = 0, childLim = childCount; childNum != childLim; ++childNum ) {
			const XMP_Node * origChild  = origParent->children[childNum];
			XMP_Node *       cloneChild = new XMP_Node ( cloneParent, origChild->name, origChild->value, origChild->options );
			CloneOffspring ( origChild, cloneChild );
			cloneParent->children.push_back ( cloneChild );
		}

	}
	
}	// CloneOffspring

// =================================================================================================
// CloneSubtree
// ============

XMP_Node *
CloneSubtree ( const XMP_Node * origRoot, XMP_Node * cloneParent )
{
	#if XMP_DebugBuild
		if ( cloneParent->parent == 0 ) {
			XMP_Assert ( origRoot->options & kXMP_SchemaNode );
			XMP_Assert ( FindConstSchema ( cloneParent, origRoot->name.c_str() ) == 0 );
		} else {
			XMP_Assert ( ! (origRoot->options & kXMP_SchemaNode) );
			if ( cloneParent->options & kXMP_PropValueIsStruct ) {	// Might be an array.
				XMP_Assert ( FindConstChild ( cloneParent, origRoot->name.c_str() ) == 0 );
			}
		}
	#endif
	
	XMP_Node * cloneRoot = new XMP_Node ( cloneParent, origRoot->name, origRoot->value, origRoot->options );
	CloneOffspring ( origRoot, cloneRoot ) ;
	cloneParent->children.push_back ( cloneRoot );
	
	return cloneRoot;
	
}	// CloneSubtree

// =================================================================================================
// CompareSubtrees
// ===============
//
// Compare 2 subtrees for semantic equality. The comparison includes value, qualifiers, and form.
// Schemas, top level properties, struct fields, and qualifiers are allowed to have differing order,
// the appropriate right node is found from the left node's name. Alt-text arrays are allowed to be
// in differing language order, other arrays are compared in order.

// *** Might someday consider sorting unordered arrays.
// *** Should expose this through XMPUtils.

bool
CompareSubtrees ( const XMP_Node & leftNode, const XMP_Node & rightNode )
{
	// Don't compare the names here, we want to allow the outermost roots to have different names.
	if ( (leftNode.value != rightNode.value) ||
	     (leftNode.options != rightNode.options) ||
	     (leftNode.children.size() != rightNode.children.size()) ||
	     (leftNode.qualifiers.size() != rightNode.qualifiers.size()) ) return false;
	
	// Compare the qualifiers, allowing them to be out of order.
	for ( size_t qualNum = 0, qualLim = leftNode.qualifiers.size(); qualNum != qualLim; ++qualNum ) {
		const XMP_Node * leftQual  = leftNode.qualifiers[qualNum];
		const XMP_Node * rightQual = FindConstQualifier ( &rightNode, leftQual->name.c_str() );
		if ( (rightQual == 0) || (! CompareSubtrees ( *leftQual, *rightQual )) ) return false;
	}
	
	if ( (leftNode.parent == 0) || (leftNode.options & (kXMP_SchemaNode | kXMP_PropValueIsStruct)) ) {

		// The parent node is a tree root, a schema, or a struct.
		for ( size_t childNum = 0, childLim = leftNode.children.size(); childNum != childLim; ++childNum ) {
			const XMP_Node * leftChild  = leftNode.children[childNum];
			const XMP_Node * rightChild = FindConstChild ( &rightNode, leftChild->name.c_str() );
			if ( (rightChild == 0) || (! CompareSubtrees ( *leftChild, *rightChild )) ) return false;
		}

	} else if ( leftNode.options & kXMP_PropArrayIsAltText ) {

		// The parent node is an alt-text array.
		for ( size_t childNum = 0, childLim = leftNode.children.size(); childNum != childLim; ++childNum ) {
			const XMP_Node * leftChild = leftNode.children[childNum];
			XMP_Assert ( (! leftChild->qualifiers.empty()) && (leftChild->qualifiers[0]->name == "xml:lang") );
			XMP_Index rightIndex = LookupLangItem ( &rightNode, leftChild->qualifiers[0]->value );
			if ( rightIndex == -1 ) return false;
			const XMP_Node * rightChild = rightNode.children[rightIndex];
			if ( ! CompareSubtrees ( *leftChild, *rightChild ) ) return false;
		}

	} else {

		// The parent must be simple or some other (not alt-text) kind of array.
		XMP_Assert ( (! (leftNode.options & kXMP_PropCompositeMask)) || (leftNode.options & kXMP_PropValueIsArray) );
		for ( size_t childNum = 0, childLim = leftNode.children.size(); childNum != childLim; ++childNum ) {
			const XMP_Node * leftChild  = leftNode.children[childNum];
			const XMP_Node * rightChild = rightNode.children[childNum];
			if ( ! CompareSubtrees ( *leftChild, *rightChild ) ) return false;
		}

	}
	
	return true;
	
}	// CompareSubtrees

// =================================================================================================
// DeleteEmptySchema
// =================

void
DeleteEmptySchema ( XMP_Node * schemaNode )
{

	if ( XMP_NodeIsSchema ( schemaNode->options ) && schemaNode->children.empty() ) {

		XMP_Node * xmpTree = schemaNode->parent;

		size_t schemaNum = 0;
		size_t schemaLim = xmpTree->children.size();
		while ( (schemaNum < schemaLim) && (xmpTree->children[schemaNum] != schemaNode) ) ++schemaNum;
		XMP_Assert ( schemaNum < schemaLim );

		XMP_NodePtrPos schemaPos = xmpTree->children.begin() + schemaNum;
		XMP_Assert ( *schemaPos == schemaNode );

		xmpTree->children.erase ( schemaPos );
		delete schemaNode;

	}

}	// DeleteEmptySchema

// =================================================================================================
// NormalizeLangValue
// ==================
//
// Normalize an xml:lang value so that comparisons are effectively case insensitive as required by
// RFC 3066 (which superceeds RFC 1766). The normalization rules:
//
//	- The primary subtag is lower case, the suggested practice of ISO 639.
//	- All 2 letter secondary subtags are upper case, the suggested practice of ISO 3166.
//	- All other subtags are lower case.

void
NormalizeLangValue ( XMP_VarString * value )
{
	char * tagStart;
	char * tagEnd;

	// Find and process the primary subtag.
	
	tagStart = (char*) value->c_str();
	for ( tagEnd = tagStart; (*tagEnd != 0) && (*tagEnd != '-'); ++tagEnd ) {
		if ( ('A' <= *tagEnd) && (*tagEnd <= 'Z') ) *tagEnd += 0x20;
	}
	
	// Find and process the secondary subtag.
	
	tagStart = tagEnd;
	if ( *tagStart == '-' ) ++tagStart;
	for ( tagEnd = tagStart; (*tagEnd != 0) && (*tagEnd != '-'); ++tagEnd ) {
		if ( ('A' <= *tagEnd) && (*tagEnd <= 'Z') ) *tagEnd += 0x20;
	}
	if ( tagEnd == tagStart+2 ) {
		if ( ('a' <= *tagStart) && (*tagStart <= 'z') ) *tagStart -= 0x20;
		++tagStart;
		if ( ('a' <= *tagStart) && (*tagStart <= 'z') ) *tagStart -= 0x20;
	}
	
	// Find and process the remaining subtags.
	
	while ( true ) {
		tagStart = tagEnd;
		if ( *tagStart == '-' ) ++tagStart;
		if ( *tagStart == 0 ) break;
		for ( tagEnd = tagStart; (*tagEnd != 0) && (*tagEnd != '-'); ++tagEnd ) {
			if ( ('A' <= *tagEnd) && (*tagEnd <= 'Z') ) *tagEnd += 0x20;
		}
	}
	
}	// NormalizeLangValue

// =================================================================================================
// NormalizeLangArray
// ==================
//
// Make sure the x-default item is first. Touch up "single value" arrays that have a default plus
// one real language. This case should have the same value for both items. Older Adobe apps were
// hardwired to only use the 'x-default' item, so we copy that value to the other item.

void
NormalizeLangArray ( XMP_Node * array )
{
	XMP_Assert ( XMP_ArrayIsAltText(array->options) );
	
	size_t itemNum;
	size_t itemLim = array->children.size();
	bool   hasDefault = false;
	
	for ( itemNum = 0; itemNum < itemLim; ++itemNum ) {
	
		if ( array->children[itemNum]->qualifiers.empty() ||
			 (array->children[itemNum]->qualifiers[0]->name != "xml:lang") ) {
			XMP_Throw ( "AltText array items must have an xml:lang qualifier", kXMPErr_BadXMP );
		}

		if ( array->children[itemNum]->qualifiers[0]->value == "x-default" ) {
			hasDefault = true;
			break;
		}

	}

	if ( hasDefault ) {

		if ( itemNum != 0 ) {
			XMP_Node * temp = array->children[0];
			array->children[0] = array->children[itemNum];
			array->children[itemNum] = temp;
		}

// 09-Oct-07, ahu: disabled to avoid unexpected behaviour
//		if ( itemLim == 2 ) array->children[1]->value = array->children[0]->value;

	}
	
}	// NormalizeLangArray

// =================================================================================================
// DetectAltText
// =============
//
// See if an array is an alt-text array. If so, make sure the x-default item is first.

void
DetectAltText ( XMP_Node * xmpParent )
{
	XMP_Assert ( XMP_ArrayIsAlternate(xmpParent->options) );

	size_t itemNum, itemLim;
	
	for ( itemNum = 0, itemLim = xmpParent->children.size(); itemNum < itemLim; ++itemNum ) {
		XMP_OptionBits currOptions = xmpParent->children[itemNum]->options;
		if ( (currOptions & kXMP_PropCompositeMask) || (! (currOptions & kXMP_PropHasLang)) ) break;
	}
	
	if ( (itemLim != 0) && (itemNum == itemLim) ) {
		xmpParent->options |= kXMP_PropArrayIsAltText;
		NormalizeLangArray ( xmpParent );
	}

}	// DetectAltText

// =================================================================================================
// SortNamedNodes
// ==============
//
// Sort the pointers in an XMP_NodeOffspring vector by name.

static inline bool Compare ( const XMP_Node * left, const XMP_Node * right )
{
	return (left->name < right->name);
}

void
SortNamedNodes ( XMP_NodeOffspring & nodeVector )
{
	sort ( nodeVector.begin(), nodeVector.end(), Compare );
}	// SortNamedNodes

// =================================================================================================
