//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: MEMORYMPAPPEDFILE.CPP
//==========================================
#include <windows.h>
#pragma hdrstop
#include "memorymappedfile.h"

MEMORY_MAPPED_FILE::MEMORY_MAPPED_FILE( PSTR pszFileName )
{
    //
    // Given a filename, the constructor opens a file handle, creates a file
    // mapping, and maps the entire file into memory.
    //
    m_hFile = INVALID_HANDLE_VALUE;
    m_hFileMapping = 0;
    m_pMemoryMappedFileBase = 0;
    m_cbFile = 0;
    m_errCode = errMMF_FileOpen;    // Initial error code: not found

    // First get a file handle      
    m_hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
                    
    if ( m_hFile == INVALID_HANDLE_VALUE )
    {
        m_errCode = errMMF_FileOpen;
        return;
    }

    m_cbFile = ::GetFileSize( m_hFile, 0 );

    // Now, create a file mapping   
    m_hFileMapping = CreateFileMapping(m_hFile,NULL, PAGE_READONLY, 0, 0,NULL);
    if ( m_hFileMapping == 0 )
    {
        // Oops.  Something went wrong.  Clean up.
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_errCode = errMMF_FileMapping;
        return;
    }

    m_pMemoryMappedFileBase = (PCHAR)MapViewOfFile( m_hFileMapping,
                                                    FILE_MAP_READ, 0, 0, 0);

    if ( m_pMemoryMappedFileBase == 0 )
    {
        // Oops.  Something went wrong.  Clean up.
        CloseHandle(m_hFileMapping);
        m_hFileMapping = 0;
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_errCode = errMMF_MapView;
        return;
    }

    m_errCode = errMMF_NoError;
}

MEMORY_MAPPED_FILE::~MEMORY_MAPPED_FILE(void)
{
    // Clean up everything that was created by the constructor
    if ( m_pMemoryMappedFileBase )
        UnmapViewOfFile( m_pMemoryMappedFileBase );

    if ( m_hFileMapping )
        CloseHandle( m_hFileMapping );

    if ( m_hFile != INVALID_HANDLE_VALUE )
        CloseHandle( m_hFile ); 

    m_errCode = errMMF_FileOpen;
}
