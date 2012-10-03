//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: MODULEFILEINFO.CPP
//==========================================
#include <windows.h>
#include "modulefileinfo.h"

MODULE_FILE_INFO::MODULE_FILE_INFO( PSTR pszFileName )
{
    m_pNext = 0;
    m_pNotFoundNext = 0;

    // Find the last '\\' to obtain a pointer to just the base filename part
    PSTR pszBaseName = strrchr( pszFileName, '\\' );
    if ( pszBaseName )  // We found a path, so advance to the base filename
        pszBaseName++;
    else
        pszBaseName = pszFileName;  // No path.  Use the same name for both
    
    // Initialize the new MODULE_FILE_INFO, and stick it at the head
    // of the list.
    lstrcpyn( m_szFullName, pszFileName, sizeof(m_szFullName) );
    lstrcpyn( m_szBaseName, pszBaseName, sizeof(m_szBaseName) );

}

void MODULE_FILE_INFO::AddNotFoundModule( PSTR pszFileName )
{
    PMODULE_FILE_INFO pNew = new MODULE_FILE_INFO( pszFileName );

    pNew->m_pNotFoundNext = m_pNotFoundNext;
    m_pNotFoundNext = pNew;
}

MODULE_FILE_INFO * MODULE_FILE_INFO::GetNextNotFoundModule( PMODULE_FILE_INFO p)
{
    PMODULE_FILE_INFO pNext = p ? p->m_pNotFoundNext : m_pNotFoundNext;
    
    return pNext;   
}

