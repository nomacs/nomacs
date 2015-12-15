// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"

#include "XMPIterator.hpp"

#include <string>
#include <stdio.h>	// For snprintf.

#if XMP_WinBuild
    #ifdef _MSC_VER
        #pragma warning ( disable : 4702 )	// unreachable code
        #pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
        #pragma warning ( disable : 4996 )	// '...' was declared deprecated
    #endif
#endif

// =================================================================================================
// Support Routines
// =================================================================================================


#ifndef TraceIterators
	#define TraceIterators 0
#endif

#if TraceIterators
	static const char * sStageNames[] = { "before", "self", "qualifiers", "children" };
#endif

static XMP_Node * sDummySchema = 0;	// ! Used for some ugliness with aliases.

// -------------------------------------------------------------------------------------------------
// AddSchemaProps
// --------------
//
// Add the top level properties to the IterNode for a schema.

static void
AddSchemaProps ( IterInfo & info, IterNode & iterSchema, const XMP_Node * xmpSchema )
{
	UNUSED(info);
	#if TraceIterators
		printf ( "    Adding properties of %s\n", xmpSchema->name.c_str() );
	#endif

	for ( size_t propNum = 0, propLim = xmpSchema->children.size(); propNum != propLim; ++propNum ) {
		const XMP_Node * xmpProp = xmpSchema->children[propNum];
		// *** set the has-aliases bit when appropriate
		iterSchema.children.push_back ( IterNode ( xmpProp->options, xmpProp->name, 0 ) );
		#if TraceIterators
			printf ( "        %s\n", xmpProp->name.c_str() );
		#endif
	}

}	// AddSchemaProps

// -------------------------------------------------------------------------------------------------
// AddSchemaAliases
// ----------------
//
// Add the aliases to the IterNode for a schema, if the corresponding actual exists.

static void
AddSchemaAliases ( IterInfo & info, IterNode & iterSchema, XMP_StringPtr schemaURI )
{
	
	// We're showing the aliases also. Look them up by their namespace prefix. Yes, the alias map is
	// sorted so we could process just that portion. But that takes more code and the extra speed
	// isn't worth it. (Plus this way we avoid a dependence on the map implementation.) Lookup the
	// XMP node from the alias, to make sure the actual exists.
	
	#if TraceIterators
		printf ( "    Adding aliases\n", schemaURI );
	#endif

	XMP_StringPtr nsPrefix;
	XMP_StringLen nsLen;
	bool found = XMPMeta::GetNamespacePrefix ( schemaURI, &nsPrefix, &nsLen );
	if ( ! found ) XMP_Throw ( "Unknown iteration namespace", kXMPErr_BadSchema );
	
	XMP_AliasMapPos currAlias = sRegisteredAliasMap->begin();
	XMP_AliasMapPos endAlias  = sRegisteredAliasMap->end();
	
	for ( ; currAlias != endAlias; ++currAlias ) {
		if ( XMP_LitNMatch ( currAlias->first.c_str(), nsPrefix, nsLen ) ) {
			const XMP_Node * actualProp = FindConstNode ( &info.xmpObj->tree, currAlias->second );
			if ( actualProp != 0 ) {
				iterSchema.children.push_back ( IterNode ( (actualProp->options | kXMP_PropIsAlias), currAlias->first, 0 ) );
				#if TraceIterators
					printf ( "        %s  =>  %s\n", currAlias->first.c_str(), actualProp->name.c_str() );
				#endif
			}
		}
	}

}	// AddSchemaAliases

// -------------------------------------------------------------------------------------------------
// AddNodeOffspring
// ----------------
//
// Add the immediate children and qualifiers to an IterNode.

static void
AddNodeOffspring ( IterInfo & info, IterNode & iterParent, const XMP_Node * xmpParent )
{
	XMP_VarString currPath ( iterParent.fullPath );
	size_t        leafOffset = iterParent.fullPath.size();
	
	if ( (! xmpParent->qualifiers.empty()) && (! (info.options & kXMP_IterOmitQualifiers)) ) {

		#if TraceIterators
			printf ( "    Adding qualifiers of %s\n", currPath.c_str() );
		#endif

		currPath += "/?";	// All qualifiers are named and use paths like "Prop/?Qual".
		leafOffset += 2;
		
		for ( size_t qualNum = 0, qualLim = xmpParent->qualifiers.size(); qualNum != qualLim; ++qualNum ) {
			const XMP_Node * xmpQual = xmpParent->qualifiers[qualNum];
			currPath += xmpQual->name;
			iterParent.qualifiers.push_back ( IterNode ( xmpQual->options, currPath, leafOffset ) );
			currPath.erase ( leafOffset );
			#if TraceIterators
				printf ( "        %s\n", xmpQual->name.c_str() );
			#endif
		}
		
		leafOffset -= 2;
		currPath.erase ( leafOffset );

	}

	if ( ! xmpParent->children.empty() ) {
	
		#if TraceIterators
			printf ( "    Adding children of %s\n", currPath.c_str() );
		#endif

		XMP_Assert ( xmpParent->options & kXMP_PropCompositeMask );
		
		if ( xmpParent->options & kXMP_PropValueIsStruct ) {
			currPath += '/';
			leafOffset += 1;
		}
		
		for ( size_t childNum = 0, childLim = xmpParent->children.size(); childNum != childLim; ++childNum ) {
			const XMP_Node * xmpChild = xmpParent->children[childNum];
			if ( ! (xmpParent->options & kXMP_PropValueIsArray) ) {
				currPath += xmpChild->name;
			} else {
				char buffer [32];	// AUDIT: Using sizeof(buffer) below for snprintf length is safe.
				snprintf ( buffer, sizeof(buffer), "[%lu]", static_cast<unsigned long>(childNum+1) );	// ! XPath indices are one-based.
				currPath += buffer;
			}
			iterParent.children.push_back ( IterNode ( xmpChild->options, currPath, leafOffset ) );
			currPath.erase ( leafOffset );
			#if TraceIterators
				printf ( "        %s\n", (iterParent.children.back().fullPath.c_str() + leafOffset) );
			#endif
		}
	
	}

}	// AddNodeOffspring

// -------------------------------------------------------------------------------------------------
// SetCurrSchema
// -------------

static inline void
SetCurrSchema ( IterInfo & info, XMP_StringPtr schemaName )
{

	info.currSchema = schemaName;
	#if 0	// *** XMP_DebugBuild
		info._schemaPtr = info.currSchema.c_str();
	#endif

}	// SetCurrSchema

static inline void
SetCurrSchema ( IterInfo & info, XMP_VarString & schemaName )
{

	info.currSchema = schemaName;
	#if 0	// *** XMP_DebugBuild
		info._schemaPtr = info.currSchema.c_str();
	#endif

}	// SetCurrSchema

// -------------------------------------------------------------------------------------------------
// AdvanceIterPos
// --------------
//
// Adjust currPos and possibly endPos for the next step in a pre-order depth-first traversal. The
// current node has just been visited, move on to its qualifiers, children, then siblings, or back
// up to an ancestor. AdvanceIterPos either moves to a property or qualifier node that can be
// visited, or to the end of the entire iteration.

static void
AdvanceIterPos ( IterInfo & info )
{
	// -------------------------------------------------------------------------------------------
	// Keep looking until we find a node to visit or the end of everything. The first time through
	// the current node will exist, we just visited it. But we have to keep looking if the current
	// node was the last of its siblings or is an empty schema.
	
	// ! It is possible that info.currPos == info.endPos on entry. Don't dereference info.currPos yet!

	while ( true ) {
	
		if ( info.currPos == info.endPos ) {
		
			// ------------------------------------------------------------------------------------
			// At the end of a set of siblings, move up to an ancestor. We've either just finished
			// the qualifiers and will move to the children, or have just finished the children and
			// will move on to the next sibling.
			
			if ( info.ancestors.empty() ) break;	// We're at the end of the schema list.

			IterPosPair & parent = info.ancestors.back();
			info.currPos = parent.first;
			info.endPos  = parent.second;
			info.ancestors.pop_back();
			
			#if TraceIterators
				printf ( "    Moved up to %s, stage = %s\n",
				         info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage] );
			#endif
		
		} else {
			
			// -------------------------------------------------------------------------------------------
			// Decide what to do with this iteration node based on its state. Don't use a switch statement,
			// some of the cases want to break from the loop. A break in a switch just exits the case.
			
			#if TraceIterators
				printf ( "    Moving from %s, stage = %s\n",
				         info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage] );
			#endif
			
			if ( info.currPos->visitStage == kIter_BeforeVisit ) {		// Visit this node now.
				if ( info.currPos->options & kXMP_SchemaNode ) SetCurrSchema ( info, info.currPos->fullPath );
				break;
			}

			if ( info.currPos->visitStage == kIter_VisitSelf ) {		// Just finished visiting the value portion.
				info.currPos->visitStage = kIter_VisitQualifiers;		// Start visiting the qualifiers.
				if ( ! info.currPos->qualifiers.empty() ) {
					info.ancestors.push_back ( IterPosPair ( info.currPos, info.endPos ) );
					info.endPos  = info.currPos->qualifiers.end();		// ! Set the parent's endPos before changing currPos!
					info.currPos = info.currPos->qualifiers.begin();
					break;
				}
			}

			if ( info.currPos->visitStage == kIter_VisitQualifiers ) {	// Just finished visiting the qualifiers.
				info.currPos->qualifiers.clear();
				info.currPos->visitStage = kIter_VisitChildren;			// Start visiting the children.
				if ( ! info.currPos->children.empty() ) {
					info.ancestors.push_back ( IterPosPair ( info.currPos, info.endPos ) );
					info.endPos  = info.currPos->children.end();		// ! Set the parent's endPos before changing currPos!
					info.currPos = info.currPos->children.begin();
					break;
				}
			}

			if ( info.currPos->visitStage == kIter_VisitChildren ) {	// Just finished visiting the children.
				info.currPos->children.clear();
				++info.currPos;											// Move to the next sibling.
				continue;
			}
			
			#if TraceIterators
				if ( info.currPos != info.endPos ) {
					printf ( "    Moved to %s, stage = %s\n",
					         info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage] );
				}
			#endif
			
		}

	}	// Loop to find the next node.
	
	XMP_Assert ( (info.currPos == info.endPos) || (info.currPos->visitStage == kIter_BeforeVisit) );

}	// AdvanceIterPos

// -------------------------------------------------------------------------------------------------
// GetNextXMPNode
// --------------
//
// Used by XMPIterator::Next to obtain the next XMP node, ignoring the kXMP_IterJustLeafNodes flag.
// This isolates some messy code, allowing a clean loop in Next if kXMP_IterJustLeafNodes is set.

static const XMP_Node *
GetNextXMPNode ( IterInfo & info )
{
	const XMP_Node * xmpNode = 0;

	// ----------------------------------------------------------------------------------------------
	// On entry currPos points to an iteration node whose state is either before-visit or visit-self.
	// If it is before-visit then we will return that node's value part now. If it is visit-self it
	// means the previous iteration returned the value portion of that node, so we can advance to the
	// next node in the iteration tree. Then we find the corresponding XMP node, allowing for the XMP
	// tree to have been modified since that part of the iteration tree was constructed.
	
	// ! NOTE: Supporting aliases throws in some nastiness with schemas. There might not be any XMP
	// ! node for the schema, but we still have to visit it because of possible aliases. The static
	// ! sDummySchema is returned if there is no real schema node.

	if ( info.currPos->visitStage != kIter_BeforeVisit ) AdvanceIterPos ( info );
	
	bool isSchemaNode = false;
	XMP_ExpandedXPath expPath;	// Keep outside the loop to avoid constant construct/destruct.
	
	while ( info.currPos != info.endPos ) {

		isSchemaNode = XMP_NodeIsSchema ( info.currPos->options );
		if ( isSchemaNode ) {
			SetCurrSchema ( info, info.currPos->fullPath );
			xmpNode = FindConstSchema ( &info.xmpObj->tree, info.currPos->fullPath.c_str() );
			if ( xmpNode == 0 ) xmpNode = sDummySchema;
		} else {
			ExpandXPath ( info.currSchema.c_str(), info.currPos->fullPath.c_str(), &expPath );
			xmpNode = FindConstNode ( &info.xmpObj->tree, expPath );
		}
		if ( xmpNode != 0 ) break;	// Exit the loop, we found a live XMP node.

		info.currPos->visitStage = kIter_VisitChildren;	// Make AdvanceIterPos move to the next sibling.
		info.currPos->children.clear();
		info.currPos->qualifiers.clear();
		AdvanceIterPos ( info );

	}

	if ( info.currPos == info.endPos ) return 0;
	
	// -------------------------------------------------------------------------------------------
	// Now we've got the iteration node and corresponding XMP node. Add the iteration children for
	// structs and arrays. The children of schema were added when the iterator was constructed.

	XMP_Assert ( info.currPos->visitStage == kIter_BeforeVisit );

	if ( info.currPos->visitStage == kIter_BeforeVisit ) {
		if ( (! isSchemaNode) && (! (info.options & kXMP_IterJustChildren)) ) {
			AddNodeOffspring ( info, *info.currPos, xmpNode );
		}
		info.currPos->visitStage = kIter_VisitSelf;
	}
	
	return xmpNode;

}	// GetNextXMPNode

// =================================================================================================
// Init/Term
// =================================================================================================

// -------------------------------------------------------------------------------------------------
// Initialize
// ----------

/* class static */ bool
XMPIterator::Initialize()
{
	sDummySchema = new XMP_Node ( 0, "dummy:schema/", kXMP_SchemaNode);
	return true;
	
}	// Initialize

// -------------------------------------------------------------------------------------------------
// Terminate
// ----------

/* class static */ void
XMPIterator::Terminate() RELEASE_NO_THROW
{
	delete ( sDummySchema );
	sDummySchema = 0;
	return;
	
}	// Terminate

// -------------------------------------------------------------------------------------------------
// Unlock
// ------

void
XMPIterator::Unlock	( XMP_OptionBits options )
{
	UNUSED(options);

	XMPMeta::Unlock ( 0 );
	
}	// Unlock

// =================================================================================================
// Constructors
// =================================================================================================

// -------------------------------------------------------------------------------------------------
// XMPIterator
// -----------
//
// Constructor for iterations over the nodes in an XMPMeta object. This builds a tree of iteration
// nodes that caches the existing node names of the XMPMeta object. The iteration tree is a partial
// replica of the XMPMeta tree. The initial iteration tree normally has just the root node, all of
// the schema nodes for a full object iteration. Lower level nodes (children and qualifiers) are 
// added when the parent is visited. If the kXMP_IterJustChildren option is passed then the initial
// iterator includes the children and the parent is marked as done. The iteration tree nodes are
// pruned when they are no longer needed. 

XMPIterator::XMPIterator ( const XMPMeta & xmpObj,
						   XMP_StringPtr   schemaNS,
						   XMP_StringPtr   propName,
						   XMP_OptionBits  options ) : clientRefs(0), info(IterInfo(options,&xmpObj))
{
	if ( (options & kXMP_IterClassMask) != kXMP_IterProperties ) {
		XMP_Throw ( "Unsupported iteration kind", kXMPErr_BadOptions );
	}
	
	// *** Lock the XMPMeta object if we ever stop using a full DLL lock.

	if ( *propName != 0 ) {

		// An iterator rooted at a specific node.

		#if TraceIterators
			printf ( "\nNew XMP property iterator for \"%s\", options = %X\n    Schema = %s, root = %s\n",
			         xmpObj.tree.name.c_str(), options, schemaNS, propName );
		#endif
		
		XMP_ExpandedXPath propPath;
		ExpandXPath ( schemaNS, propName, &propPath );
		XMP_Node * propNode = FindConstNode ( &xmpObj.tree, propPath );	// If not found get empty iteration.
		
		if ( propNode != 0 ) {

			XMP_VarString rootName ( propPath[1].step );	// The schema is [0].
			for ( size_t i = 2; i < propPath.size(); ++i ) {
				XMP_OptionBits stepKind = GetStepKind ( propPath[i].options );
				if ( stepKind <= kXMP_QualifierStep ) rootName += '/';
				rootName += propPath[i].step;
			}

			propName = rootName.c_str();
			size_t leafOffset = rootName.size();
			while ( (leafOffset > 0) && (propName[leafOffset] != '/') && (propName[leafOffset] != '[') ) --leafOffset;
			if ( propName[leafOffset] == '/' ) ++leafOffset;

			info.tree.children.push_back ( IterNode ( propNode->options, propName, leafOffset ) );
			SetCurrSchema ( info, propPath[kSchemaStep].step.c_str() );
			if ( info.options & kXMP_IterJustChildren ) {
				AddNodeOffspring ( info, info.tree.children.back(), propNode );
			}

		}
	
	} else if ( *schemaNS != 0 ) {

		// An iterator for all properties in one schema.
		
		#if TraceIterators
			printf ( "\nNew XMP schema iterator for \"%s\", options = %X\n    Schema = %s\n",
			         xmpObj.tree.name.c_str(), options, schemaNS );
		#endif
		
		info.tree.children.push_back ( IterNode ( kXMP_SchemaNode, schemaNS, 0 ) );
		IterNode & iterSchema = info.tree.children.back();
		
		XMP_Node * xmpSchema = FindConstSchema ( &xmpObj.tree, schemaNS );
		if ( xmpSchema != 0 ) AddSchemaProps ( info, iterSchema, xmpSchema );
		
		if ( info.options & kXMP_IterIncludeAliases ) AddSchemaAliases ( info, iterSchema, schemaNS );
		
		if ( iterSchema.children.empty() ) {
			info.tree.children.pop_back();	// No properties, remove the schema node.
		} else {
			SetCurrSchema ( info, schemaNS );
		}
	
	} else {

		// An iterator for all properties in all schema. First add schema that exist (have children),
		// adding aliases from them if appropriate. Then add schema that have no actual properties
		// but do have aliases to existing properties, if we're including aliases in the iteration.
		
		#if TraceIterators
			printf ( "\nNew XMP tree iterator for \"%s\", options = %X\n",
			         xmpObj.tree.name.c_str(), options );
		#endif
		
		// First pick up the schema that exist.
		
		for ( size_t schemaNum = 0, schemaLim = xmpObj.tree.children.size(); schemaNum != schemaLim; ++schemaNum ) {

			const XMP_Node * xmpSchema = xmpObj.tree.children[schemaNum];
			info.tree.children.push_back ( IterNode ( kXMP_SchemaNode, xmpSchema->name, 0 ) );
			IterNode & iterSchema = info.tree.children.back();

			if ( ! (info.options & kXMP_IterJustChildren) ) {
				AddSchemaProps ( info, iterSchema, xmpSchema );
				if ( info.options & kXMP_IterIncludeAliases ) AddSchemaAliases ( info, iterSchema, xmpSchema->name.c_str() );
				if ( iterSchema.children.empty() ) info.tree.children.pop_back();	// No properties, remove the schema node.
			}

		}
		
		if ( info.options & kXMP_IterIncludeAliases ) {

			// Add the schema that only have aliases. The most convenient, and safest way, is to go
			// through the registered namespaces, see if it exists, and let AddSchemaAliases do its
			// thing if not. Don't combine with the above loop, it is nicer to have the "real" stuff
			// be in storage order (not subject to the namespace map order).
			
			// ! We don't do the kXMP_IterJustChildren handing in the same way here as above. The
			// ! existing schema (presumably) have actual children. We need to call AddSchemaAliases
			// ! here to determine if the namespace has any aliases to existing properties. We then
			// ! strip the children if necessary.

			XMP_cStringMapPos currNS = sNamespaceURIToPrefixMap->begin();
			XMP_cStringMapPos endNS  = sNamespaceURIToPrefixMap->end();
			for ( ; currNS != endNS; ++currNS ) {
				XMP_StringPtr schemaName = currNS->first.c_str();
				if ( FindConstSchema ( &xmpObj.tree, schemaName ) != 0 ) continue;
				info.tree.children.push_back ( IterNode ( kXMP_SchemaNode, schemaName, 0 ) );
				IterNode & iterSchema = info.tree.children.back();
				AddSchemaAliases ( info, iterSchema, schemaName );
				if ( iterSchema.children.empty() ) {
					info.tree.children.pop_back();	// No aliases, remove the schema node.
				} else if ( info.options & kXMP_IterJustChildren ) {
					iterSchema.children.clear();	// Get rid of the children.
				}
			}

		}

	}
	
	// Set the current iteration position to the first node to be visited.
	
	info.currPos = info.tree.children.begin();
	info.endPos  = info.tree.children.end();
	
	if ( (info.options & kXMP_IterJustChildren) && (info.currPos != info.endPos) && (*schemaNS != 0) ) {
		info.currPos->visitStage = kIter_VisitSelf;
	}

	#if TraceIterators
		if ( info.currPos == info.endPos ) {
			printf ( "    ** Empty iteration **\n" );
		} else {
			printf ( "    Initial node %s, stage = %s, iterator @ %.8X\n",
			         info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage], this );
		}
	#endif
	
}	// XMPIterator for XMPMeta objects

// -------------------------------------------------------------------------------------------------
// XMPIterator
// -----------
//
// Constructor for iterations over global tables such as registered namespaces or aliases.

XMPIterator::XMPIterator ( XMP_StringPtr  /*schemaNS*/,
                           XMP_StringPtr  /*propName*/,
                           XMP_OptionBits options ) : clientRefs(0), info(IterInfo(options,0))
{

	XMP_Throw ( "Unimplemented XMPIterator constructor for global tables", kXMPErr_Unimplemented );

}	// XMPIterator for global tables

// -------------------------------------------------------------------------------------------------
// ~XMPIterator
// -----------

XMPIterator::~XMPIterator() RELEASE_NO_THROW
{
	XMP_Assert ( this->clientRefs <= 0 );
	// Let everything else default.
	
}	// ~XMPIterator

// =================================================================================================
// Iteration Methods
// =================================================================================================

// -------------------------------------------------------------------------------------------------
// Next
// ----
//
// Do a preorder traversal of the cached nodes.

// *** Need to document the relationships between currPos, endPos, and visitStage.

bool
XMPIterator::Next ( XMP_StringPtr *	 schemaNS,
					XMP_StringLen *	 nsSize,
					XMP_StringPtr *	 propPath,
					XMP_StringLen *	 pathSize,
					XMP_StringPtr *	 propValue,
					XMP_StringLen *	 valueSize,
					XMP_OptionBits * propOptions )
{
	// *** Lock the XMPMeta object if we ever stop using a full DLL lock.
	
	// ! NOTE: Supporting aliases throws in some nastiness with schemas. There might not be any XMP
	// ! node for the schema, but we still have to visit it because of possible aliases.
	
	if ( info.currPos == info.endPos ) return false;	// Happens at the start of an empty iteration.
	
	#if TraceIterators
		printf ( "Next iteration from %s, stage = %s, iterator @ %.8X\n",
			     info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage], this );
	#endif
	
	const XMP_Node * xmpNode = GetNextXMPNode ( info );
	if ( xmpNode == 0 ) return false;
	bool isSchemaNode = XMP_NodeIsSchema ( info.currPos->options );
	
	if ( info.options & kXMP_IterJustLeafNodes ) {
		while ( isSchemaNode || (! xmpNode->children.empty()) ) {
			info.currPos->visitStage = kIter_VisitQualifiers;	// Skip to this node's children.
			xmpNode = GetNextXMPNode ( info );
			if ( xmpNode == 0 ) return false;
			isSchemaNode = XMP_NodeIsSchema ( info.currPos->options );
		}
	}
	
	*schemaNS = info.currSchema.c_str();
	*nsSize   = info.currSchema.size();

	*propOptions = info.currPos->options;

	*propPath  = "";
	*pathSize  = 0;
	*propValue = "";
	*valueSize = 0;
	
	if ( ! (*propOptions & kXMP_SchemaNode) ) {

		*propPath = info.currPos->fullPath.c_str();
		*pathSize = info.currPos->fullPath.size();
		if ( info.options & kXMP_IterJustLeafName ) {
			*propPath += info.currPos->leafOffset;
			*pathSize -= info.currPos->leafOffset;
		}
		
		if ( ! (*propOptions & kXMP_PropCompositeMask) ) {
			*propValue = xmpNode->value.c_str();
			*valueSize = xmpNode->value.size();
		}

	}
	
	#if TraceIterators
		printf ( "    Next node %s, stage = %s\n",
			     info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage] );
	#endif
	
	return true;

}	// Next

// -------------------------------------------------------------------------------------------------
// Skip
// ----
//
// Skip some portion of the traversal related to the last visited node. We skip either that node's
// children, or those children and the previous node's siblings. The implementation might look a bit
// awkward because info.currNode always points to the next node to be visited. We might already have
// moved past the things to skip, e.g. if the previous node was simple and the last of its siblings.

enum {
	kXMP_ValidIterSkipOptions	= kXMP_IterSkipSubtree | kXMP_IterSkipSiblings
};

void
XMPIterator::Skip ( XMP_OptionBits iterOptions )
{
//	if ( (info.currPos == kIter_NullPos) )  XMP_Throw ( "No prior postion to skip from", kXMPErr_BadIterPosition );
	if ( iterOptions == 0 ) XMP_Throw ( "Must specify what to skip", kXMPErr_BadOptions );
	if ( (iterOptions & ~kXMP_ValidIterSkipOptions) != 0 ) XMP_Throw ( "Undefined options", kXMPErr_BadOptions );

	#if TraceIterators
		printf ( "Skipping from %s, stage = %s, iterator @ %.8X",
			     info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage], this );
	#endif
	
	if ( iterOptions & kXMP_IterSkipSubtree ) {
		#if TraceIterators
			printf ( ", mode = subtree\n" );
		#endif
		info.currPos->visitStage = kIter_VisitChildren;
	} else if ( iterOptions & kXMP_IterSkipSiblings ) {
		#if TraceIterators
			printf ( ", mode = siblings\n" );
		#endif
		info.currPos = info.endPos;
		AdvanceIterPos ( info );
	}
	#if TraceIterators
		printf ( "    Skipped to %s, stage = %s\n",
			     info.currPos->fullPath.c_str(), sStageNames[info.currPos->visitStage] );
	#endif
	

}	// Skip

// -------------------------------------------------------------------------------------------------
// UnlockIter
// ----------

void
XMPIterator::UnlockIter	( XMP_OptionBits options )
{
	UNUSED(options);

	XMPMeta::Unlock ( 0 );
	
}	// UnlockIter

// =================================================================================================
