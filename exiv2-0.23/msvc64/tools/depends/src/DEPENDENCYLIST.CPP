//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: DEPENDENCYLIST.CPP
//==========================================
#include <windows.h>
#include <malloc.h>
#pragma hdrstop
#include "peexe.h"
#include "dependencylist.h"

MODULE_DEPENDENCY_LIST::MODULE_DEPENDENCY_LIST( PSTR pszFileName )
{
    m_errorType = errMDL_GENERAL_FAILURE;
    m_cModules = 0;
    m_pList = 0;

    // Make a copy of the path that we can modify to get just the path portion
    PSTR pszJustPath = _strdup( pszFileName );
    if ( !pszJustPath )
        return;
    
    BOOL fHasPath = FALSE;
    PSTR pszEnd = strrchr( pszJustPath, '\\' );
    if ( pszEnd )
    {
        *pszEnd = 0;    /// Strip off the filename
        fHasPath = TRUE;
    }

    //
    // If a path was part of the input filename, save the current directory,
    // then switch to the new directory.
    //  
    char szOriginalPath[MAX_PATH];
    if ( fHasPath )
    {
        // This doesn't take into account "App_Paths"!
        GetCurrentDirectory(MAX_PATH, szOriginalPath);  // Save original dir
        SetCurrentDirectory( pszJustPath );             // Switch to app's dir
    }

    //
    // recursively build the module list
    //
    m_errorType = AddModule( pszFileName );
    
    if ( fHasPath )     // Set things back to the way they were
        SetCurrentDirectory( szOriginalPath );
    
    free( pszJustPath );    // Free the copy of the path that we allocated
}


MODULE_DEPENDENCY_LIST::~MODULE_DEPENDENCY_LIST( )
{
    PMODULE_FILE_INFO pTemp;

    // Delete each MODULE_FILE_INFO structures in the regular linked list
    pTemp = m_pList;
    while ( pTemp )
    {
        pTemp = m_pList->m_pNext;

        // Before we delete the module, delete each MODULE_FILE_INFO
        // structures in the not found list
        PMODULE_FILE_INFO pNotFound = m_pList->m_pNotFoundNext;
        while ( pNotFound )
        {
            pNotFound = m_pList->m_pNotFoundNext->m_pNotFoundNext;
            delete m_pList->m_pNotFoundNext;
            m_pList->m_pNotFoundNext = pNotFound;
        }

        // Now it's OK to delete the module
        delete m_pList;
        m_pList = pTemp;
        m_cModules--;
    }

    m_pList = 0;
}



PMODULE_FILE_INFO MODULE_DEPENDENCY_LIST::GetNextModule( PMODULE_FILE_INFO p )
{
    // Returns the next module in the linked list of MODULE_FILE_INFO's
    return p ? p->m_pNext : m_pList;
}


// Given the name of a file, find the MODULE_FILE_INFO structure that
// represents it.  The fFullName parameter specifies whether the full path
// names or just the base file names will be compared.
PMODULE_FILE_INFO MODULE_DEPENDENCY_LIST::LookupModule( PSTR pszFileName,
                                                        BOOL fFullName )
{
    PMODULE_FILE_INFO p = m_pList;  // Start at the list head

    while ( p ) // While there's still entries in the list...
    {
        PSTR pszCompName = fFullName ? p->m_szFullName : p->m_szBaseName;

        if ( 0 == lstrcmpi( pszFileName, pszCompName ) )
            return p;

        p = p->m_pNext;
    }

    return 0;
}


PSTR MODULE_DEPENDENCY_LIST::GetErrorString( void )
{
    switch ( m_errorType )
    {
        case errMDL_NO_ERROR:       return "No error";
        case errMDL_FILE_NOT_FOUND: return "File not found";
        case errMDL_NOT_PE_FILE:    return "Not a PE file";
        case errMDL_GENERAL_FAILURE:return "General failure";
        default:                    return "<Error ???>";
    }
}

    
// Adds a modules to the MODULE_FILE_INFO list.  If the module imports other
// modules, this routine recurses to add them, and check their imports.
errModuleDependencyList
MODULE_DEPENDENCY_LIST::AddModule( PSTR pszFileName )
{
    PE_EXE peFile( pszFileName );       // Get easy access to the executable
    
    if ( FALSE == peFile.IsValid() )    // A valid PE file???
        return (errModuleDependencyList)peFile.GetErrorType();

    PMODULE_FILE_INFO pNew = new MODULE_FILE_INFO( pszFileName );
    if ( !pNew )
        return errMDL_GENERAL_FAILURE;

    pNew->m_pNext = m_pList;
    m_pList = pNew;

    m_cModules++;
    
    //
    // Now see if this module imports any other modules.  If so, we need
    // to recurse and add them as well.
    //
    if (0 == peFile.GetDataDirectoryEntrySize( IMAGE_DIRECTORY_ENTRY_IMPORT ))
        return errMDL_NO_ERROR;

    // Make a pointer to the imports table
    PIMAGE_IMPORT_DESCRIPTOR pImportDir;
    pImportDir = (PIMAGE_IMPORT_DESCRIPTOR)
        peFile.GetDataDirectoryEntryPointer(IMAGE_DIRECTORY_ENTRY_IMPORT);
    if ( !pImportDir )
        return errMDL_NO_ERROR;

    // While there are still non-null IMAGE_IMPORT_DESCRIPTORs...
    while ( pImportDir->Name )
    {
        // Get a pointer to the imported module's base name
        PSTR pszBaseName;
        pszBaseName = (PSTR)peFile.GetReadablePointerFromRVA(pImportDir->Name);
        if ( !pszBaseName )
            break;

        // Check to see if it's already in our list.  Don't add again if so.
        if ( 0 == LookupModule( pszBaseName, FALSE ) )
        {
            // Search path supposedly has the same searching algorithm as
            // the the Win32 loader...
            char szPath[MAX_PATH];
            PSTR pszDontCare;
            if ( SearchPath(0, pszBaseName, 0, MAX_PATH, szPath, &pszDontCare))
                AddModule( szPath );
            else
                pNew->AddNotFoundModule( pszBaseName );
        }
        
        pImportDir++;   // Advance to next imported module
    }

    return errMDL_NO_ERROR;
}
