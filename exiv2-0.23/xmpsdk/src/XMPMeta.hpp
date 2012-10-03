#ifndef __XMPMeta_hpp__
#define __XMPMeta_hpp__

// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:	Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"
#include "XMP_Const.h"
#include "XMPCore_Impl.hpp"
#include "XMLParserAdapter.hpp"

// -------------------------------------------------------------------------------------------------

#ifndef DumpXMLParseTree
	#define DumpXMLParseTree 0
#endif

extern XMP_VarString * xdefaultName;

class XMPIterator;
class XMPUtils;

// -------------------------------------------------------------------------------------------------

class XMPMeta {
public:

	static void
	GetVersionInfo ( XMP_VersionInfo * info );
	
	static bool
	Initialize();
	static void
	Terminate() RELEASE_NO_THROW;
	
	static void
	Unlock ( XMP_OptionBits options );

	// ---------------------------------------------------------------------------------------------

	XMPMeta();
	
	virtual ~XMPMeta() RELEASE_NO_THROW;

	// ---------------------------------------------------------------------------------------------
	
	static XMP_OptionBits
	GetGlobalOptions();
	
	static void
	SetGlobalOptions ( XMP_OptionBits options );

	// ---------------------------------------------------------------------------------------------

	static XMP_Status
	DumpNamespaces ( XMP_TextOutputProc outProc,
					 void *				refCon );
	
	static XMP_Status
	DumpAliases ( XMP_TextOutputProc outProc,
				  void *			 refCon );
	
	// ---------------------------------------------------------------------------------------------
	
	static void
	RegisterNamespace ( XMP_StringPtr	namespaceURI,
                            XMP_StringPtr	prefix );
	
	static bool
	GetNamespacePrefix ( XMP_StringPtr	 namespaceURI,
						 XMP_StringPtr * namespacePrefix,
						 XMP_StringLen * prefixSize );
	
	static bool
	GetNamespaceURI ( XMP_StringPtr	  namespacePrefix,
					  XMP_StringPtr * namespaceURI,
					  XMP_StringLen * uriSize );
	
	static void
	DeleteNamespace ( XMP_StringPtr namespaceURI );

	// ---------------------------------------------------------------------------------------------
	
	static void
	RegisterAlias ( XMP_StringPtr  aliasNS,
					XMP_StringPtr  aliasProp,
					XMP_StringPtr  actualNS,
					XMP_StringPtr  actualProp,
					XMP_OptionBits arrayForm );
	
	static bool
	ResolveAlias ( XMP_StringPtr	aliasNS,
				   XMP_StringPtr	aliasProp,
				   XMP_StringPtr *	actualNS,
				   XMP_StringLen *	nsSize,
				   XMP_StringPtr *	actualProp,
				   XMP_StringLen *	propSize,
				   XMP_OptionBits * arrayForm );
	
	static void
	DeleteAlias ( XMP_StringPtr aliasNS,
				  XMP_StringPtr aliasProp );
	
	static void
	RegisterStandardAliases ( XMP_StringPtr schemaNS );

	// ---------------------------------------------------------------------------------------------
	
	void
	UnlockObject ( XMP_OptionBits options ) const;
	
	// ---------------------------------------------------------------------------------------------
	
	bool
	GetProperty ( XMP_StringPtr	   schemaNS,
				  XMP_StringPtr	   propName,
				  XMP_StringPtr *  propValue,
				  XMP_StringLen *  valueSize,
				  XMP_OptionBits * options ) const;
	
	bool
	GetArrayItem ( XMP_StringPtr	schemaNS,
				   XMP_StringPtr	arrayName,
				   XMP_Index		itemIndex,
				   XMP_StringPtr *	itemValue,
				   XMP_StringLen *	valueSize,
				   XMP_OptionBits * options ) const;
	
	bool
	GetStructField ( XMP_StringPtr	  schemaNS,
					 XMP_StringPtr	  structName,
					 XMP_StringPtr	  fieldNS,
					 XMP_StringPtr	  fieldName,
					 XMP_StringPtr *  fieldValue,
					 XMP_StringLen *  valueSize,
					 XMP_OptionBits * options ) const;
	
	bool
	GetQualifier ( XMP_StringPtr	schemaNS,
				   XMP_StringPtr	propName,
				   XMP_StringPtr	qualNS,
				   XMP_StringPtr	qualName,
				   XMP_StringPtr *	qualValue,
				   XMP_StringLen *	valueSize,
				   XMP_OptionBits * options ) const;
	
	// ---------------------------------------------------------------------------------------------
	
	void
	SetProperty ( XMP_StringPtr	 schemaNS,
				  XMP_StringPtr	 propName,
				  XMP_StringPtr	 propValue,
				  XMP_OptionBits options );
	
	void
	SetArrayItem ( XMP_StringPtr  schemaNS,
				   XMP_StringPtr  arrayName,
				   XMP_Index	  itemIndex,
				   XMP_StringPtr  itemValue,
				   XMP_OptionBits options );
	
	void
	AppendArrayItem ( XMP_StringPtr	 schemaNS,
					  XMP_StringPtr	 arrayName,
					  XMP_OptionBits arrayOptions,
					  XMP_StringPtr	 itemValue,
					  XMP_OptionBits options );
	
	void
	SetStructField ( XMP_StringPtr	schemaNS,
					 XMP_StringPtr	structName,
					 XMP_StringPtr	fieldNS,
					 XMP_StringPtr	fieldName,
					 XMP_StringPtr	fieldValue,
					 XMP_OptionBits options );
	
	void
	SetQualifier ( XMP_StringPtr  schemaNS,
				   XMP_StringPtr  propName,
				   XMP_StringPtr  qualNS,
				   XMP_StringPtr  qualName,
				   XMP_StringPtr  qualValue,
				   XMP_OptionBits options );
	
	// ---------------------------------------------------------------------------------------------
	
	void
	DeleteProperty ( XMP_StringPtr schemaNS,
					 XMP_StringPtr propName );
	
	void
	DeleteArrayItem ( XMP_StringPtr schemaNS,
					  XMP_StringPtr arrayName,
					  XMP_Index		itemIndex );
	
	void
	DeleteStructField ( XMP_StringPtr schemaNS,
						XMP_StringPtr structName,
						XMP_StringPtr fieldNS,
						XMP_StringPtr fieldName );
	
	void
	DeleteQualifier ( XMP_StringPtr schemaNS,
					  XMP_StringPtr propName,
					  XMP_StringPtr qualNS,
					  XMP_StringPtr qualName );
	
	// ---------------------------------------------------------------------------------------------
	
	bool
	DoesPropertyExist ( XMP_StringPtr schemaNS,
						XMP_StringPtr propName ) const;
	
	bool
	DoesArrayItemExist ( XMP_StringPtr schemaNS,
						 XMP_StringPtr arrayName,
						 XMP_Index	   itemIndex ) const;
	
	bool
	DoesStructFieldExist ( XMP_StringPtr schemaNS,
						   XMP_StringPtr structName,
						   XMP_StringPtr fieldNS,
						   XMP_StringPtr fieldName ) const;
	
	bool
	DoesQualifierExist ( XMP_StringPtr schemaNS,
						 XMP_StringPtr propName,
						 XMP_StringPtr qualNS,
						 XMP_StringPtr qualName ) const;
	
	// ---------------------------------------------------------------------------------------------
	
	bool
	GetLocalizedText ( XMP_StringPtr	schemaNS,
					   XMP_StringPtr	altTextName,
					   XMP_StringPtr	genericLang,
					   XMP_StringPtr	specificLang,
					   XMP_StringPtr *	actualLang,
					   XMP_StringLen *	langSize,
					   XMP_StringPtr *	itemValue,
					   XMP_StringLen *	valueSize,
					   XMP_OptionBits * options ) const;
	
	void
	SetLocalizedText ( XMP_StringPtr  schemaNS,
					   XMP_StringPtr  altTextName,
					   XMP_StringPtr  genericLang,
					   XMP_StringPtr  specificLang,
					   XMP_StringPtr  itemValue,
					   XMP_OptionBits options );
	
	// ---------------------------------------------------------------------------------------------
	
	bool
	GetProperty_Bool ( XMP_StringPtr	schemaNS,
					   XMP_StringPtr	propName,
					   bool *			propValue,
					   XMP_OptionBits * options ) const;
	
	bool
	GetProperty_Int ( XMP_StringPtr	   schemaNS,
					  XMP_StringPtr	   propName,
					  XMP_Int32 *	   propValue,
					  XMP_OptionBits * options ) const;
	
	bool
	GetProperty_Int64 ( XMP_StringPtr	 schemaNS,
					    XMP_StringPtr	 propName,
					    XMP_Int64 *	     propValue,
					    XMP_OptionBits * options ) const;
	
	bool
	GetProperty_Float ( XMP_StringPtr	 schemaNS,
						XMP_StringPtr	 propName,
						double *		 propValue,
						XMP_OptionBits * options ) const;
	
	bool
	GetProperty_Date ( XMP_StringPtr	schemaNS,
					   XMP_StringPtr	propName,
					   XMP_DateTime *	propValue,
					   XMP_OptionBits * options ) const;
	
	// ---------------------------------------------------------------------------------------------
	
	void
	SetProperty_Bool ( XMP_StringPtr  schemaNS,
					   XMP_StringPtr  propName,
					   bool			  propValue,
					   XMP_OptionBits options );
	
	void
	SetProperty_Int ( XMP_StringPtr	 schemaNS,
					  XMP_StringPtr	 propName,
					  XMP_Int32		 propValue,
					  XMP_OptionBits options );
	
	void
	SetProperty_Int64 ( XMP_StringPtr  schemaNS,
					    XMP_StringPtr  propName,
					    XMP_Int64	   propValue,
					    XMP_OptionBits options );
	
	void
	SetProperty_Float ( XMP_StringPtr  schemaNS,
						XMP_StringPtr  propName,
						double		   propValue,
						XMP_OptionBits options );
	
	void
	SetProperty_Date ( XMP_StringPtr		schemaNS,
					   XMP_StringPtr		propName,
					   const XMP_DateTime & propValue,
					   XMP_OptionBits		options );
	
	// ---------------------------------------------------------------------------------------------
	
	void
	GetObjectName ( XMP_StringPtr * namePtr,
					XMP_StringLen * nameLen ) const;

	void
	SetObjectName ( XMP_StringPtr name );

	XMP_OptionBits
	GetObjectOptions() const;
	
	void
	SetObjectOptions ( XMP_OptionBits options );

	void
	Sort();

	void
	Erase();

	void
	Clone ( XMPMeta * clone, XMP_OptionBits options ) const;
	
	XMP_Index
	CountArrayItems ( XMP_StringPtr schemaNS,
					  XMP_StringPtr arrayName ) const;
	
	XMP_Status
	DumpObject ( XMP_TextOutputProc outProc,
				 void *				refCon ) const;
	
	// ---------------------------------------------------------------------------------------------
	
	void
	ParseFromBuffer ( XMP_StringPtr	 buffer,
					  XMP_StringLen	 bufferSize,
					  XMP_OptionBits options );
	
	void
	SerializeToBuffer ( XMP_StringPtr * rdfString,
						XMP_StringLen * rdfSize,
						XMP_OptionBits	options,
						XMP_StringLen	padding,
						XMP_StringPtr	newline,
						XMP_StringPtr	indent,
						XMP_Index		baseIndent ) const;
	
	// =============================================================================================

	// ---------------------------------------------------------------------------------------------
	// - Everything is built out of standard nodes. Each node has a name, value, option flags, a
	// vector of child nodes, and a vector of qualifier nodes.
	//
	// - The option flags are those passed to SetProperty and returned from GetProperty. They tell
	// if the node is simple, a struct or an array; whether it has qualifiers, etc.
	//
	// - The name of the node is an XML qualified name, of the form "prefix:simple-name". Since we
	// force all namespaces to be known and to have unique prefixes, this is semantically equivalent
	// to using a URI and simple name pair.
	//
	// - Although the value part is only for leaf properties and the children part is only for
	// structs and arrays, it is easier to simply have them in every node. This keeps things visible
	// so that debugging is easier
	//
	// - The top level node children are the namespaces that contain properties, the next level are
	// the top level properties, lower levels are the fields of structs or items of arrays. The name
	// of the top level nodes is just the namespace prefix, with the colon terminator. The name of
	// top level properties includes the namespace prefix.
	//
	// - Any property node, at any level, can have qualifiers. These are themselves general property
	// nodes. And could in fact themselves have qualifiers!

	// ! Expose the implementation so that file static functions can see the data.

	XMP_Int32 clientRefs;	// ! Must be signed to allow decrement from 0.
	XMP_Int32 prevTkVer;	// Previous toolkit version as MMmmuubbb (major, minor, micro, build).
	XMP_Node  tree;

	XMLParserAdapter * xmlParser;
	
	friend class XMPIterator;
	friend class XMPUtils;

private:
  
	// ! These are hidden on purpose:
	XMPMeta ( const XMPMeta & /* original */ ) : clientRefs(0), prevTkVer(0), tree(XMP_Node(0,"",0)), xmlParser(0)
		{ XMP_Throw ( "Call to hidden constructor", kXMPErr_InternalFailure ); };
	void operator= ( const XMPMeta & /* rhs */ )  
		{ XMP_Throw ( "Call to hidden operator=", kXMPErr_InternalFailure ); };

};	// class XMPMeta


// =================================================================================================

#endif	// __XMPMeta_hpp__
