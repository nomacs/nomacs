//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: MODULEFILEINFO.H
//==========================================
#ifndef __MODULEFILEINFO_H__
#define __MODULEFILEINFO_H__

class MODULE_DEPENDENCY_LIST;

//
// This structure represents one executable file in a module dependency list.
// Both the base filename and the complete path are stored.
//
class MODULE_FILE_INFO
{
    public:

    MODULE_FILE_INFO( PSTR pszFileName );
    ~MODULE_FILE_INFO( void ){}

    PSTR GetBaseName( void ){ return m_szBaseName; }
    PSTR GetFullName( void ){ return m_szFullName; }

    // For enumerating through the unlocatable imported modules
    MODULE_FILE_INFO * GetNextNotFoundModule( MODULE_FILE_INFO * );

    private:

    MODULE_FILE_INFO * m_pNext;
    MODULE_FILE_INFO * m_pNotFoundNext;

    char    m_szBaseName[MAX_PATH];
    char    m_szFullName[MAX_PATH];

    // And an unlocatable module to the "not found" list
    void AddNotFoundModule( PSTR pszFileName );

    friend class MODULE_DEPENDENCY_LIST;
};

typedef MODULE_FILE_INFO * PMODULE_FILE_INFO;

#endif
