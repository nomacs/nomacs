#ifndef __XMPCore_Impl_hpp__
#define __XMPCore_Impl_hpp__

// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! Must be the first #include!
#include "XMP_Const.h"
#include "XMP_BuildInfo.h"

#include "client-glue/WXMPMeta.hpp"

#include <vector>
#include <string>
#include <map>

#include <cassert>
#include <cstring>

#if XMP_WinBuild
	#include <windows.h>
#else
	// Use pthread for both Mac and generic UNIX.
	#include <pthread.h>
#endif

#if XMP_WinBuild
#   ifdef _MSC_VER
        #pragma warning ( disable : 4244 )	// possible loss of data (temporary for 64 bit builds)
        #pragma warning ( disable : 4267 )	// possible loss of data (temporary for 64 bit builds)
#   endif
#endif

// =================================================================================================
// Primary internal types

class XMP_Node;
class XML_Node;
class XPathStepInfo;

typedef XMP_Node *	XMP_NodePtr;

typedef std::vector<XMP_Node*>		XMP_NodeOffspring;
typedef XMP_NodeOffspring::iterator	XMP_NodePtrPos;

typedef std::string	XMP_VarString;
typedef XMP_VarString::iterator			XMP_VarStringPos;
typedef XMP_VarString::const_iterator	XMP_cVarStringPos;

typedef std::pair < XMP_VarString, XMP_VarString >	XMP_StringPair;

typedef std::map < XMP_VarString, XMP_VarString > XMP_StringMap;
typedef XMP_StringMap::iterator			XMP_StringMapPos;
typedef XMP_StringMap::const_iterator	XMP_cStringMapPos;

typedef std::vector < XPathStepInfo >		XMP_ExpandedXPath;
typedef XMP_ExpandedXPath::iterator			XMP_ExpandedXPathPos;
typedef XMP_ExpandedXPath::const_iterator	XMP_cExpandedXPathPos;

typedef std::map < XMP_VarString, XMP_ExpandedXPath > XMP_AliasMap;	// Alias name to actual path.
typedef XMP_AliasMap::iterator			XMP_AliasMapPos;
typedef XMP_AliasMap::const_iterator	XMP_cAliasMapPos;

// =================================================================================================
// General global variables and macros

extern XMP_Int32 sXMP_InitCount;

extern XMP_AliasMap *	sRegisteredAliasMap;

extern XMP_StringMap *	sNamespaceURIToPrefixMap;
extern XMP_StringMap *	sNamespacePrefixToURIMap;

extern XMP_VarString *	sOutputNS;
extern XMP_VarString *	sOutputStr;

extern void *			voidVoidPtr;	// Used to backfill null output parameters.
extern XMP_StringPtr	voidStringPtr;
extern XMP_StringLen	voidStringLen;
extern XMP_OptionBits	voidOptionBits;
extern XMP_Bool			voidByte;
extern bool				voidBool;
extern XMP_Int32		voidInt32;
extern XMP_Int64		voidInt64;
extern double			voidDouble;
extern XMP_DateTime		voidDateTime;
extern WXMP_Result		void_wResult;

#define kHexDigits "0123456789ABCDEF"

#define XMP_LitMatch(s,l)		(std::strcmp((s),(l)) == 0)
#define XMP_LitNMatch(s,l,n)	(std::strncmp((s),(l),(n)) == 0)
	// *** Use the above macros!

#define kTab ((char)0x09)
#define kLF ((char)0x0A)
#define kCR ((char)0x0D)

#if XMP_WinBuild
	#define snprintf _snprintf
#endif

#define WtoXMPMeta_Ref(xmpRef)	*((const XMPMeta *)(xmpRef))
#define WtoXMPMeta_Ptr(xmpRef)	(((xmpRef) == 0) ? 0 : (XMPMeta *)(xmpRef))

#define WtoXMPIterator_Ref(iterRef)	*((const XMPIterator *)(iterRef))
#define WtoXMPIterator_Ptr(iterRef)	(((iterRef) == 0) ? 0 : (XMPIterator *)(iterRef))

#define WtoXMPDocOps_Ref(docRef)	*((const XMPDocOps *)(docRef))
#define WtoXMPDocOps_Ptr(docRef)	(((docRef) == 0) ? 0 : (XMPDocOps *)(docRef))

#define IgnoreParam(p)	voidVoidPtr = (void*)&p

// =================================================================================================
// Version info

#if XMP_DebugBuild
	#define kXMPCore_DebugFlag 1
#else
	#define kXMPCore_DebugFlag 0
#endif

#define kXMPCore_VersionNumber	( (kXMPCore_DebugFlag << 31)    |	\
                                  (XMP_API_VERSION_MAJOR << 24) |	\
						          (XMP_API_VERSION_MINOR << 16) |	\
						          (XMP_API_VERSION_MICRO << 8) )

	#define kXMPCoreName "XMP Core"
	#define kXMPCore_VersionMessage	kXMPCoreName " " XMP_API_VERSION_STRING
// =================================================================================================
// Support for asserts

#define _MakeStr(p)			#p
#define _NotifyMsg(n,c,f,l)	#n " failed: " #c " in " f " at line " _MakeStr(l)

#if ! XMP_DebugBuild
	#define XMP_Assert(c)	((void) 0)
#else
		#define XMP_Assert(c)	assert ( c )
#endif

	#define XMP_Enforce(c)																			\
		if ( ! (c) ) {																				\
			const char * assert_msg = _NotifyMsg ( XMP_Enforce, (c), __FILE__, __LINE__ );			\
			XMP_Throw ( assert_msg , kXMPErr_EnforceFailure );										\
		}
// =================================================================================================
// Support for exceptions and thread locking

#ifndef TraceXMPCalls
	#define TraceXMPCalls	0
#endif

#if ! TraceXMPCalls

	#define AnnounceThrow(msg)		/* Do nothing. */
	#define AnnounceCatch(msg)		/* Do nothing. */

	#define AnnounceEntry(proc)		/* Do nothing. */
	#define AnnounceNoLock(proc)	/* Do nothing. */
	#define AnnounceExit()			/* Do nothing. */

	#define ReportLock()			++sLockCount
	#define ReportUnlock()			--sLockCount
	#define ReportKeepLock()		/* Do nothing. */

#else

	extern FILE * xmpCoreOut;

	#define AnnounceThrow(msg)	\
		fprintf ( xmpCoreOut, "XMP_Throw: %s\n", msg ); fflush ( xmpOut )
	#define AnnounceCatch(msg)	\
		fprintf ( xmpCoreOut, "Catch in %s: %s\n", procName, msg ); fflush ( xmpOut )

	#define AnnounceEntry(proc)			\
		const char * procName = proc;	\
		fprintf ( xmpCoreOut, "Entering %s\n", procName ); fflush ( xmpOut )
	#define AnnounceNoLock(proc)		\
		const char * procName = proc;	\
		fprintf ( xmpCoreOut, "Entering %s (no lock)\n", procName ); fflush ( xmpOut )
	#define AnnounceExit()	\
		fprintf ( xmpCoreOut, "Exiting %s\n", procName ); fflush ( xmpOut )

	#define ReportLock()	\
		++sLockCount; fprintf ( xmpCoreOut, "  Auto lock, count = %d\n", sLockCount ); fflush ( xmpOut )
	#define ReportUnlock()	\
		--sLockCount; fprintf ( xmpCoreOut, "  Auto unlock, count = %d\n", sLockCount ); fflush ( xmpOut )
	#define ReportKeepLock()	\
		fprintf ( xmpCoreOut, "  Keeping lock, count = %d\n", sLockCount ); fflush ( xmpOut )

#endif

#define XMP_Throw(msg,id)	{ AnnounceThrow ( msg ); throw XMP_Error ( id, msg ); }

// -------------------------------------------------------------------------------------------------

#if XMP_WinBuild
	typedef CRITICAL_SECTION XMP_Mutex;
#else
	// Use pthread for both Mac and generic UNIX.
	typedef pthread_mutex_t XMP_Mutex;
#endif

extern XMP_Mutex sXMPCoreLock;
extern int	sLockCount;	// Keep signed to catch unlock errors.
extern XMP_VarString * sExceptionMessage;

extern bool XMP_InitMutex ( XMP_Mutex * mutex );
extern void XMP_TermMutex ( XMP_Mutex & mutex );

extern void XMP_EnterCriticalRegion ( XMP_Mutex & mutex );
extern void XMP_ExitCriticalRegion ( XMP_Mutex & mutex );

class XMP_AutoMutex {
public:
	XMP_AutoMutex() : mutex(&sXMPCoreLock) { XMP_EnterCriticalRegion ( *mutex ); ReportLock(); };
	~XMP_AutoMutex() { if ( mutex != 0 ) { ReportUnlock(); XMP_ExitCriticalRegion ( *mutex ); mutex = 0; } };
	void KeepLock() { ReportKeepLock(); mutex = 0; };
private:
	XMP_Mutex * mutex;
};

// *** Switch to XMPEnterObjectWrapper & XMPEnterStaticWrapper, to allow for per-object locks.

// ! Don't do the initialization check (sXMP_InitCount > 0) for the no-lock case. That macro is used
// ! by WXMPMeta_Initialize_1.

#define XMP_ENTER_WRAPPER_NO_LOCK(proc)						\
	AnnounceNoLock ( proc );								\
	XMP_Assert ( (0 <= sLockCount) && (sLockCount <= 1) );	\
	try {													\
		wResult->errMessage = 0;

#define XMP_ENTER_WRAPPER(proc)								\
	AnnounceEntry ( proc );									\
	XMP_Assert ( sXMP_InitCount > 0 );	                    \
	XMP_Assert ( (0 <= sLockCount) && (sLockCount <= 1) );	\
	try {													\
		XMP_AutoMutex mutex;								\
		wResult->errMessage = 0;

#define XMP_EXIT_WRAPPER	\
	XMP_CATCH_EXCEPTIONS	\
	AnnounceExit();

#define XMP_EXIT_WRAPPER_KEEP_LOCK(keep)	\
		if ( keep ) mutex.KeepLock();		\
	XMP_CATCH_EXCEPTIONS					\
	AnnounceExit();

#define XMP_EXIT_WRAPPER_NO_THROW				\
	} catch ( ... )	{							\
		AnnounceCatch ( "no-throw catch-all" );	\
		/* Do nothing. */						\
	}											\
	AnnounceExit();

#define XMP_CATCH_EXCEPTIONS										\
	} catch ( XMP_Error & xmpErr ) {								\
		wResult->int32Result = xmpErr.GetID(); 						\
		wResult->ptrResult   = (void*)"XMP";						\
		wResult->errMessage  = xmpErr.GetErrMsg();					\
		if ( wResult->errMessage == 0 ) wResult->errMessage = "";	\
		AnnounceCatch ( wResult->errMessage );						\
	} catch ( std::exception & stdErr ) {							\
		wResult->int32Result = kXMPErr_StdException; 				\
		wResult->errMessage  = stdErr.what(); 						\
		if ( wResult->errMessage == 0 ) wResult->errMessage = "";	\
		AnnounceCatch ( wResult->errMessage );						\
	} catch ( ... ) {												\
		wResult->int32Result = kXMPErr_UnknownException; 			\
		wResult->errMessage  = "Caught unknown exception";			\
		AnnounceCatch ( wResult->errMessage );						\
	}

#if XMP_DebugBuild
	#define RELEASE_NO_THROW	/* empty */
#else
	#define RELEASE_NO_THROW	throw()
#endif

// =================================================================================================
// ExpandXPath, FindNode, and related support

// *** Normalize the use of "const xx &" for input params

#define kXMP_ArrayItemName	"[]"

#define kXMP_CreateNodes	true
#define kXMP_ExistingOnly	false

#define FindConstSchema(t,u)	FindSchemaNode ( const_cast<XMP_Node*>(t), u, kXMP_ExistingOnly, 0 )
#define FindConstChild(p,c)		FindChildNode ( const_cast<XMP_Node*>(p), c, kXMP_ExistingOnly, 0 )
#define FindConstQualifier(p,c)	FindQualifierNode ( const_cast<XMP_Node*>(p), c, kXMP_ExistingOnly, 0 )
#define FindConstNode(t,p)		FindNode ( const_cast<XMP_Node*>(t), p, kXMP_ExistingOnly, 0 )

extern XMP_OptionBits
VerifySetOptions ( XMP_OptionBits options, XMP_StringPtr propValue );

extern void
ComposeXPath ( const XMP_ExpandedXPath & expandedXPath,
			   XMP_VarString * stringXPath );

extern void
ExpandXPath	( XMP_StringPtr			schemaNS,
			  XMP_StringPtr			propPath,
			  XMP_ExpandedXPath *	expandedXPath );

extern XMP_Node *
FindSchemaNode ( XMP_Node *		  xmpTree,
				 XMP_StringPtr	  nsURI,
				 bool			  createNodes,
				 XMP_NodePtrPos * ptrPos = 0 );

extern XMP_Node *
FindChildNode ( XMP_Node *		 parent,
				XMP_StringPtr	 childName,
				bool			 createNodes,
				XMP_NodePtrPos * ptrPos = 0 );

extern XMP_Node *
FindQualifierNode ( XMP_Node *		 parent,
					XMP_StringPtr	 qualName,
					bool			 createNodes,
					XMP_NodePtrPos * ptrPos = 0 );

extern XMP_Node *
FindNode ( XMP_Node *		xmpTree,
		   const XMP_ExpandedXPath & expandedXPath,
		   bool				createNodes,
		   XMP_OptionBits	leafOptions = 0,
		   XMP_NodePtrPos * ptrPos = 0 );

extern XMP_Index
LookupLangItem ( const XMP_Node * arrayNode, XMP_VarString & lang );	// ! Lang must be normalized!

extern XMP_Index
LookupFieldSelector ( const XMP_Node * arrayNode, XMP_StringPtr fieldName, XMP_StringPtr fieldValue );

extern void
CloneOffspring ( const XMP_Node * origParent, XMP_Node * cloneParent );

extern XMP_Node *
CloneSubtree ( const XMP_Node * origRoot, XMP_Node * cloneParent );

extern bool
CompareSubtrees ( const XMP_Node & leftNode, const XMP_Node & rightNode );

extern void
DeleteEmptySchema ( XMP_Node * schemaNode );

extern void
NormalizeLangValue ( XMP_VarString * value );

extern void
NormalizeLangArray ( XMP_Node * array );

extern void
DetectAltText ( XMP_Node * xmpParent );

extern void
SortNamedNodes ( XMP_NodeOffspring & nodeVector );

static inline bool
IsPathPrefix ( XMP_StringPtr fullPath, XMP_StringPtr prefix )
{
	bool isPrefix = false;
	XMP_StringLen prefixLen = std::strlen(prefix);
	if ( XMP_LitNMatch ( prefix, fullPath, prefixLen ) ) {
		char separator = fullPath[prefixLen];
		if ( (separator == 0) || (separator == '/') ||
			 (separator == '[') || (separator == '*') ) isPrefix = true;
	}
	return isPrefix;
}

// -------------------------------------------------------------------------------------------------

class XPathStepInfo {
public:
	XMP_VarString	step;
	XMP_OptionBits	options;
	XPathStepInfo ( XMP_StringPtr _step, XMP_OptionBits _options ) : step(_step), options(_options) {};
	XPathStepInfo ( XMP_VarString _step, XMP_OptionBits _options ) : step(_step), options(_options) {};
private:
	XPathStepInfo() : options(0) {};	// ! Hide the default constructor.
};

enum { kSchemaStep = 0, kRootPropStep = 1, kAliasIndexStep = 2 };

enum {	// Bits for XPathStepInfo options.	// *** Add mask check to init code.
	kXMP_StepKindMask		= 0x0F,	// ! The step kinds are mutually exclusive numbers.
	kXMP_StructFieldStep	= 0x01,	// Also for top level nodes (schema "fields").
	kXMP_QualifierStep		= 0x02,	// ! Order is significant to separate struct/qual from array kinds!
	kXMP_ArrayIndexStep		= 0x03,	// ! The kinds must not overlay array form bits!
	kXMP_ArrayLastStep		= 0x04,
	kXMP_QualSelectorStep	= 0x05,
	kXMP_FieldSelectorStep	= 0x06,
	kXMP_StepIsAlias        = 0x10
};

#define GetStepKind(f)	((f) & kXMP_StepKindMask)

#define kXMP_NewImplicitNode	kXMP_InsertAfterItem

// =================================================================================================
// XMP_Node details

#if 0	// Pattern for iterating over the children or qualifiers:
	for ( size_t xxNum = 0, xxLim = _node_->_offspring_.size(); xxNum < xxLim; ++xxNum ) {
		const XMP_Node * _curr_ = _node_->_offspring_[xxNum];
	}
#endif

class XMP_Node {
public:

	XMP_OptionBits		options;
	XMP_VarString		name, value;
	XMP_Node *			parent;
	XMP_NodeOffspring	children;
	XMP_NodeOffspring	qualifiers;
	#if XMP_DebugBuild
		// *** XMP_StringPtr	_namePtr, _valuePtr;	// *** Not working, need operator=?
	#endif

	XMP_Node ( XMP_Node * _parent, XMP_StringPtr _name, XMP_OptionBits _options )
		: options(_options), name(_name), parent(_parent)
	{
		#if XMP_DebugBuild
			XMP_Assert ( (name.find ( ':' ) != XMP_VarString::npos) || (name == kXMP_ArrayItemName) ||
			             (options & kXMP_SchemaNode) || (parent == 0) );
			// *** _namePtr  = name.c_str();
			// *** _valuePtr = value.c_str();
		#endif
	};

	XMP_Node ( XMP_Node * _parent, const XMP_VarString & _name, XMP_OptionBits _options )
		: options(_options), name(_name), parent(_parent)
	{
		#if XMP_DebugBuild
			XMP_Assert ( (name.find ( ':' ) != XMP_VarString::npos) || (name == kXMP_ArrayItemName) ||
			             (options & kXMP_SchemaNode) || (parent == 0) );
			// *** _namePtr  = name.c_str();
			// *** _valuePtr = value.c_str();
		#endif
	};
	
	XMP_Node ( XMP_Node * _parent, XMP_StringPtr _name, XMP_StringPtr _value, XMP_OptionBits _options )
		: options(_options), name(_name), value(_value), parent(_parent)
	{
		#if XMP_DebugBuild
			XMP_Assert ( (name.find ( ':' ) != XMP_VarString::npos) || (name == kXMP_ArrayItemName) ||
			             (options & kXMP_SchemaNode) || (parent == 0) );
			// *** _namePtr  = name.c_str();
			// *** _valuePtr = value.c_str();
		#endif
	};

	XMP_Node ( XMP_Node * _parent, const XMP_VarString & _name, const XMP_VarString & _value, XMP_OptionBits _options )
		: options(_options), name(_name), value(_value), parent(_parent)
	{
		#if XMP_DebugBuild
			XMP_Assert ( (name.find ( ':' ) != XMP_VarString::npos) || (name == kXMP_ArrayItemName) ||
			             (options & kXMP_SchemaNode) || (parent == 0) );
			// *** _namePtr  = name.c_str();
			// *** _valuePtr = value.c_str();
		#endif
	};
	
	void RemoveChildren()
	{
		for ( size_t i = 0, vLim = children.size(); i < vLim; ++i ) {
			if ( children[i] != 0 ) delete children[i];
		}
		children.clear();
	}
	
	void RemoveQualifiers()
	{
		for ( size_t i = 0, vLim = qualifiers.size(); i < vLim; ++i ) {
			if ( qualifiers[i] != 0 ) delete qualifiers[i];
		}
		qualifiers.clear();
	}
	
	void ClearNode()
	{
		options = 0;
		name.erase();
		value.erase();
		this->RemoveChildren();
		this->RemoveQualifiers();
	}

	virtual ~XMP_Node() { RemoveChildren(); RemoveQualifiers(); };

private:
	XMP_Node() : options(0), parent(0)	// ! Make sure parent pointer is always set.
	{
		#if XMP_DebugBuild
			// *** _namePtr  = name.c_str();
			// *** _valuePtr = value.c_str();
		#endif
	};

};

class XMP_AutoNode {	// Used to hold a child during subtree construction.
public:
	XMP_Node * nodePtr;
	XMP_AutoNode() : nodePtr(0) {};
	~XMP_AutoNode() { if ( nodePtr != 0 ) delete ( nodePtr ); nodePtr = 0; };
	XMP_AutoNode ( XMP_Node * _parent, XMP_StringPtr _name, XMP_OptionBits _options )
		: nodePtr ( new XMP_Node ( _parent, _name, _options ) ) {};
	XMP_AutoNode ( XMP_Node * _parent, const XMP_VarString & _name, XMP_OptionBits _options )
		: nodePtr ( new XMP_Node ( _parent, _name, _options ) ) {};
	XMP_AutoNode ( XMP_Node * _parent, XMP_StringPtr _name, XMP_StringPtr _value, XMP_OptionBits _options )
		: nodePtr ( new XMP_Node ( _parent, _name, _value, _options ) ) {};
	XMP_AutoNode ( XMP_Node * _parent, const XMP_VarString & _name, const XMP_VarString & _value, XMP_OptionBits _options )
		: nodePtr ( new XMP_Node ( _parent, _name, _value, _options ) ) {};
};

extern void ProcessRDF ( XMP_Node * xmpTree, const XML_Node & xmlTree, XMP_OptionBits options );

// =================================================================================================

#endif	// __XMPCore_Impl_hpp__
