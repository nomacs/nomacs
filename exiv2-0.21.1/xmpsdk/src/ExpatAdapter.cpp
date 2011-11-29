// =================================================================================================
// Copyright 2005-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! Must be the first #include!
#include "XMPCore_Impl.hpp"

#include "ExpatAdapter.hpp"
#include "XMPMeta.hpp"

#include "expat.h"

#include <string.h>

using namespace std;

#if XMP_WinBuild
#   ifdef _MSC_VER
        #pragma warning ( disable : 4996 )	// '...' was declared deprecated
#   endif
#endif

// *** Set memory handlers.

#ifndef DumpXMLParseEvents
	#define DumpXMLParseEvents	0
#endif

#define FullNameSeparator	'@'

// =================================================================================================

static void StartNamespaceDeclHandler    ( void * userData, XMP_StringPtr prefix, XMP_StringPtr uri );
static void EndNamespaceDeclHandler      ( void * userData, XMP_StringPtr prefix );

static void StartElementHandler          ( void * userData, XMP_StringPtr name, XMP_StringPtr* attrs );
static void EndElementHandler            ( void * userData, XMP_StringPtr name );

static void CharacterDataHandler         ( void * userData, XMP_StringPtr cData, int len );
static void StartCdataSectionHandler     ( void * userData );
static void EndCdataSectionHandler       ( void * userData );

static void ProcessingInstructionHandler ( void * userData, XMP_StringPtr target, XMP_StringPtr data );
static void CommentHandler               ( void * userData, XMP_StringPtr comment );

#if BanAllEntityUsage

	// For now we do this by banning DOCTYPE entirely. This is easy and consistent with what is
	// available in recent Java XML parsers. Another, somewhat less drastic, approach would be to 
	// ban all entity declarations. We can't allow declarations and ban references, Expat does not
	// call the SkippedEntityHandler for references in attribute values.
	
	// ! Standard entities (&amp;, &lt;, &gt;, &quot;, &apos;, and numeric character references) are
	// ! not banned. Expat handles them transparently no matter what.

	static void StartDoctypeDeclHandler ( void * userData, XMP_StringPtr doctypeName,
										  XMP_StringPtr sysid, XMP_StringPtr pubid, int has_internal_subset );

#endif

// =================================================================================================

extern "C" ExpatAdapter * XMP_NewExpatAdapter()
{
	return new ExpatAdapter;
}	// XMP_NewExpatAdapter

// =================================================================================================

ExpatAdapter::ExpatAdapter() : parser(0)
{

	#if XMP_DebugBuild
		this->elemNesting = 0;
		#if DumpXMLParseEvents
			if ( this->parseLog == 0 ) this->parseLog = stdout;
		#endif
	#endif

	this->parser = XML_ParserCreateNS ( 0, FullNameSeparator );
	if ( this->parser == 0 ) XMP_Throw ( "Failure creating Expat parser", kXMPErr_ExternalFailure );
	
	XML_SetUserData ( this->parser, this );
	
	XML_SetNamespaceDeclHandler ( this->parser, StartNamespaceDeclHandler, EndNamespaceDeclHandler );
	XML_SetElementHandler ( this->parser, StartElementHandler, EndElementHandler );

	XML_SetCharacterDataHandler ( this->parser, CharacterDataHandler );
	XML_SetCdataSectionHandler ( this->parser, StartCdataSectionHandler, EndCdataSectionHandler );

	XML_SetProcessingInstructionHandler ( this->parser, ProcessingInstructionHandler );
	XML_SetCommentHandler ( this->parser, CommentHandler );

	#if BanAllEntityUsage
		XML_SetStartDoctypeDeclHandler ( this->parser, StartDoctypeDeclHandler );
		isAborted = false;
	#endif

	this->parseStack.push_back ( &this->tree );	// Push the XML root node.

}	// ExpatAdapter::ExpatAdapter

// =================================================================================================

ExpatAdapter::~ExpatAdapter()
{

	if ( this->parser != 0 ) XML_ParserFree ( this->parser );
	this->parser = 0;

}	// ExpatAdapter::~ExpatAdapter

// =================================================================================================

#if XMP_DebugBuild
	static XMP_VarString sExpatMessage;
#endif

static const char * kOneSpace = " ";

void ExpatAdapter::ParseBuffer ( const void * buffer, size_t length, bool last /* = true */ )
{
	enum XML_Status status;
	
	if ( length == 0 ) {	// Expat does not like empty buffers.
		if ( ! last ) return;
		buffer = kOneSpace;
		length = 1;
	}
	
	status = XML_Parse ( this->parser, (const char *)buffer, length, last );
	
	#if BanAllEntityUsage
		if ( this->isAborted ) XMP_Throw ( "DOCTYPE is not allowed", kXMPErr_BadXML );
	#endif

	if ( status != XML_STATUS_OK ) {
	
		XMP_StringPtr errMsg = "XML parsing failure";

		#if 0	// XMP_DebugBuild	// Disable for now to make test output uniform. Restore later with thread safety.
		
			// *** This is a good candidate for a callback error notification mechanism.
			// *** This code is not thread safe, the sExpatMessage isn't locked. But that's OK for debug usage.

			enum XML_Error expatErr = XML_GetErrorCode ( this->parser );
			const char *   expatMsg = XML_ErrorString ( expatErr );
			int errLine = XML_GetCurrentLineNumber ( this->parser );
		
			char msgBuffer[1000];
			// AUDIT: Use of sizeof(msgBuffer) for snprintf length is safe.
			snprintf ( msgBuffer, sizeof(msgBuffer), "# Expat error %d at line %d, \"%s\"", expatErr, errLine, expatMsg );
			sExpatMessage = msgBuffer;
			errMsg = sExpatMessage.c_str();

			#if  DumpXMLParseEvents
				if ( this->parseLog != 0 ) fprintf ( this->parseLog, "%s\n", errMsg, expatErr, errLine, expatMsg );
			#endif

		#endif

		XMP_Throw ( errMsg, kXMPErr_BadXML );

	}
	
}	// ExpatAdapter::ParseBuffer

// =================================================================================================
// =================================================================================================

#if XMP_DebugBuild & DumpXMLParseEvents

	static inline void PrintIndent ( FILE * file, size_t count )
	{
		for ( ; count > 0; --count ) fprintf ( file, "  " );
	}

#endif

// =================================================================================================

static void SetQualName ( XMP_StringPtr fullName, XML_Node * node )
{
	// Expat delivers the full name as a catenation of namespace URI, separator, and local name.

	// As a compatibility hack, an "about" or "ID" attribute of an rdf:Description element is
	// changed to "rdf:about" or rdf:ID. Easier done here than in the RDF recognizer.
	
	// As a bug fix hack, change a URI of "http://purl.org/dc/1.1/" to ""http://purl.org/dc/elements/1.1/.
	// Early versions of Flash that put XMP in SWF used a bad URI for the dc: namespace.

	// ! This code presumes the RDF namespace prefix is "rdf".

	size_t sepPos = strlen(fullName);
	for ( --sepPos; sepPos > 0; --sepPos ) {
		if ( fullName[sepPos] == FullNameSeparator ) break;
	}

	if ( fullName[sepPos] == FullNameSeparator ) {

		XMP_StringPtr prefix;
		XMP_StringLen prefixLen;
		XMP_StringPtr localPart = fullName + sepPos + 1;

		node->ns.assign ( fullName, sepPos );
		if ( node->ns == "http://purl.org/dc/1.1/" ) node->ns = "http://purl.org/dc/elements/1.1/";

		bool found = XMPMeta::GetNamespacePrefix ( node->ns.c_str(), &prefix, &prefixLen );
		if ( ! found ) XMP_Throw ( "Unknown URI in Expat full name", kXMPErr_ExternalFailure );
		node->nsPrefixLen = prefixLen;	// ! Includes the ':'.
		
		node->name = prefix;
		node->name += localPart;

	} else {

		node->name = fullName;	// The name is not in a namespace.
	
		if ( node->parent->name == "rdf:Description" ) {
			if ( node->name == "about" ) {
				node->ns   = kXMP_NS_RDF;
				node->name = "rdf:about";
				node->nsPrefixLen = 4;	// ! Include the ':'.
			} else if ( node->name == "ID" ) {
				node->ns   = kXMP_NS_RDF;
				node->name = "rdf:ID";
				node->nsPrefixLen = 4;	// ! Include the ':'.
			}
		}
		
	}

}	// SetQualName

// =================================================================================================

static void StartNamespaceDeclHandler ( void * userData, XMP_StringPtr prefix, XMP_StringPtr uri )
{
	IgnoreParam(userData);
	
	// As a bug fix hack, change a URI of "http://purl.org/dc/1.1/" to ""http://purl.org/dc/elements/1.1/.
	// Early versions of Flash that put XMP in SWF used a bad URI for the dc: namespace.
	
	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif

	if ( prefix == 0 ) prefix = "_dflt_";	// Have default namespace.
	if ( uri == 0 ) return;	// Ignore, have xmlns:pre="", no URI to register.
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "StartNamespace: %s - \"%s\"\n", prefix, uri );
		}
	#endif
	
	if ( XMP_LitMatch ( uri, "http://purl.org/dc/1.1/" ) ) uri = "http://purl.org/dc/elements/1.1/";
	XMPMeta::RegisterNamespace ( uri, prefix );

}	// StartNamespaceDeclHandler

// =================================================================================================

static void EndNamespaceDeclHandler ( void * userData, XMP_StringPtr prefix )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif

	if ( prefix == 0 ) prefix = "_dflt_";	// Have default namespace.
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "EndNamespace: %s\n", prefix );
		}
	#endif
	
	// ! Nothing to do, Expat has done all of the XML processing.

}	// EndNamespaceDeclHandler

// =================================================================================================

static void StartElementHandler ( void * userData, XMP_StringPtr name, XMP_StringPtr* attrs )
{
	XMP_Assert ( attrs != 0 );
	ExpatAdapter * thiz = (ExpatAdapter*)userData;
	
	size_t attrCount = 0;
	for ( XMP_StringPtr* a = attrs; *a != 0; ++a ) ++attrCount;
	if ( (attrCount & 1) != 0 )	XMP_Throw ( "Expat attribute info has odd length", kXMPErr_ExternalFailure );
	attrCount = attrCount/2;	// They are name/value pairs.
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "StartElement: %s, %d attrs", name, attrCount );
			for ( XMP_StringPtr* attr = attrs; *attr != 0; attr += 2 ) {
				XMP_StringPtr attrName = *attr;
				XMP_StringPtr attrValue = *(attr+1);
				fprintf ( thiz->parseLog, ", %s = \"%s\"", attrName, attrValue );
			}
			fprintf ( thiz->parseLog, "\n" );
		}
	#endif

	XML_Node * parentNode = thiz->parseStack.back();
	XML_Node * elemNode   = new XML_Node ( parentNode, "", kElemNode );
	
	SetQualName ( name, elemNode );
	
	for ( XMP_StringPtr* attr = attrs; *attr != 0; attr += 2 ) {

		XMP_StringPtr attrName = *attr;
		XMP_StringPtr attrValue = *(attr+1);
		XML_Node * attrNode = new XML_Node ( elemNode, "", kAttrNode );

		SetQualName ( attrName, attrNode );
		attrNode->value = attrValue;
		if ( attrNode->name == "xml:lang" ) NormalizeLangValue ( &attrNode->value );
		elemNode->attrs.push_back ( attrNode );

	}
	
	parentNode->content.push_back ( elemNode );
	thiz->parseStack.push_back ( elemNode );
	
	if ( elemNode->name == "rdf:RDF" ) {
		thiz->rootNode = elemNode;
		++thiz->rootCount;
	}
	#if XMP_DebugBuild
		++thiz->elemNesting;
	#endif

}	// StartElementHandler

// =================================================================================================

static void EndElementHandler ( void * userData, XMP_StringPtr name )
{
	IgnoreParam(name);
	
	ExpatAdapter * thiz = (ExpatAdapter*)userData;

	#if XMP_DebugBuild
		--thiz->elemNesting;
	#endif
	(void) thiz->parseStack.pop_back();
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "EndElement: %s\n", name );
		}
	#endif

}	// EndElementHandler

// =================================================================================================

static void CharacterDataHandler ( void * userData, XMP_StringPtr cData, int len )
{
	ExpatAdapter * thiz = (ExpatAdapter*)userData;
	
	if ( (cData == 0) || (len == 0) ) { cData = ""; len = 0; }
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "CharContent: \"" );
			for ( int i = 0; i < len; ++i ) fprintf ( thiz->parseLog, "%c", cData[i] );
			fprintf ( thiz->parseLog, "\"\n" );
		}
	#endif
	
	XML_Node * parentNode = thiz->parseStack.back();
	XML_Node * cDataNode  = new XML_Node ( parentNode, "", kCDataNode );
	
	cDataNode->value.assign ( cData, len );
	parentNode->content.push_back ( cDataNode );
	
}	// CharacterDataHandler

// =================================================================================================

static void StartCdataSectionHandler ( void * userData )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "StartCDATA\n" );
		}
	#endif
	
	// *** Since markup isn't recognized inside CDATA, this affects XMP's double escaping.
	
}	// StartCdataSectionHandler

// =================================================================================================

static void EndCdataSectionHandler ( void * userData )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "EndCDATA\n" );
		}
	#endif	

}	// EndCdataSectionHandler

// =================================================================================================

static void ProcessingInstructionHandler ( void * userData, XMP_StringPtr target, XMP_StringPtr data )
{
	XMP_Assert ( target != 0 );
	ExpatAdapter * thiz = (ExpatAdapter*)userData;

	if ( ! XMP_LitMatch ( target, "xpacket" ) ) return;	// Ignore all PIs except the XMP packet wrapper.
	if ( data == 0 ) data = "";
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "PI: %s - \"%s\"\n", target, data );
		}
	#endif
	
	XML_Node * parentNode = thiz->parseStack.back();
	XML_Node * piNode  = new XML_Node ( parentNode, target, kPINode );
	
	piNode->value.assign ( data );
	parentNode->content.push_back ( piNode );
	
}	// ProcessingInstructionHandler

// =================================================================================================

static void CommentHandler ( void * userData, XMP_StringPtr comment )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif

	if ( comment == 0 ) comment = "";
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "Comment: \"%s\"\n", comment );
		}
	#endif
	
	// ! Comments are ignored.
	
}	// CommentHandler

// =================================================================================================

#if BanAllEntityUsage
static void StartDoctypeDeclHandler ( void * userData, XMP_StringPtr doctypeName,
									  XMP_StringPtr sysid, XMP_StringPtr pubid, int has_internal_subset )
{
	IgnoreParam(userData);

	ExpatAdapter * thiz = (ExpatAdapter*)userData;

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->elemNesting );
			fprintf ( thiz->parseLog, "DocType: \"%s\"\n", doctypeName );
		}
	#endif
	
	thiz->isAborted = true;	// ! Can't throw an exception across the plain C Expat frames.
	(void) XML_StopParser ( thiz->parser, XML_FALSE /* not resumable */ );

}	// StartDoctypeDeclHandler
#endif

// =================================================================================================
