//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: DEPENDENCYLIST.H
//==========================================
#ifndef __DEPLIST_H__
#define __DEPLIST_H__

#ifndef __MODULEFILEINFO_H__
#include "modulefileinfo.h"
#endif

enum errModuleDependencyList {  errMDL_NO_ERROR,
                                errMDL_FILE_NOT_FOUND,
                                errMDL_NOT_PE_FILE,
                                errMDL_GENERAL_FAILURE };

//
// The MODULE_DEPENDENCY_LIST class creates a linked list of MODULE_FILE_INFO
// structures.  In theory, this list will represent every executable file
// loaded by the Win32 loader when the executable is loaded.  The class creates
// the list by starting with the file passed to the constructor, and recursing
// through all the import tables.
//
class MODULE_DEPENDENCY_LIST
{
    public:
        
    MODULE_DEPENDENCY_LIST( PSTR pszFileName );

    ~MODULE_DEPENDENCY_LIST( );
    
    BOOL IsValid( void ){ return (BOOL)(m_errorType == errMDL_NO_ERROR); }

    errModuleDependencyList GetErrorType( void ){ return m_errorType; }

    PSTR GetErrorString( void );

    PMODULE_FILE_INFO GetNextModule( PMODULE_FILE_INFO p );
        
    PMODULE_FILE_INFO LookupModule( PSTR pszFileName, BOOL fFullName );

    unsigned GetNumberOfModules( void ){ return m_cModules; }

    protected:
        
    unsigned            m_cModules;         // Number of modules in list

    PMODULE_FILE_INFO   m_pList;            // Pointer to head of linked list

    // Recursively adds modules to the list 
    errModuleDependencyList AddModule( PSTR pszFullName );

    errModuleDependencyList m_errorType;    // Error type
};

#endif
