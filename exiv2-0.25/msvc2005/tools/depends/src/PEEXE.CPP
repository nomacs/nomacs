//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: PEEXE.CPP
//==========================================
#include <windows.h>
#include <stdlib.h>
#pragma hdrstop
#include "peexe.h"

PE_EXE::PE_EXE( PSTR pszFileName ) : EXE_FILE( pszFileName )
{
    m_pNtHdr = 0;
    
    if ( FALSE == EXE_FILE::IsValid() )
        return;

    // It's an EXE, but is it a *PE* file???  If not, set code and bail
    if ( GetExeType() != exeType_PE )
    {
        m_errorType = errEXE_FILE_INVALID_FORMAT;
        return;
    }

    m_pNtHdr = MakePtr(PIMAGE_NT_HEADERS,GetBase(),GetSecondaryHeaderOffset());
}

DWORD PE_EXE::GetDataDirectoryEntryRVA( DWORD id )
{
    // Given a IMAGE_DIRECTORY_ENTRY_XXX value (see WINNT.H), retrive the
    // RVA stored in the corresponding slot

    if ( id >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES )
        return (DWORD)-1;

    return m_pNtHdr->OptionalHeader.DataDirectory[id].VirtualAddress;
}

PVOID PE_EXE::GetDataDirectoryEntryPointer( DWORD id )
{
    // Given a IMAGE_DIRECTORY_ENTRY_XXX value (see WINNT.H), return a pointer
    // to memory that corresponds to the RVA in the specified slot.

    if ( id >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES )
        return (PVOID)-1;
    
    DWORD va = m_pNtHdr->OptionalHeader.DataDirectory[id].VirtualAddress;

    if ( !va )      // Return 0 if the RVA is 0
        return 0;

    return GetReadablePointerFromRVA( va );
}

DWORD PE_EXE::GetDataDirectoryEntrySize( DWORD id )
{
    // Given a IMAGE_DIRECTORY_ENTRY_XXX value (see WINNT.H), retrive the
    // size value stored in the corresponding slot

    if ( id >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES )
        return (DWORD)-1;

    return m_pNtHdr->OptionalHeader.DataDirectory[id].Size;
}

PVOID PE_EXE::GetReadablePointerFromRVA( DWORD rva )
{
    // Given an RVA, translate it into a pointer within our linear memory
    // mapping for the executable.

    DWORD fileOffset = RVAToFileOffset( rva );
    if ( (DWORD)-1 == fileOffset )
        return 0;
    
    return MakePtr( PVOID, GetBase(), fileOffset );
}

DWORD PE_EXE::RVAToFileOffset( DWORD rva )
{
    // Given an RVA, figure out which section encompasses it.  Next, using
    // the PointerToRawData field for the found section, return an actual
    // file offset that corresponds to the RVA

    PIMAGE_SECTION_HEADER pSectHdr = IMAGE_FIRST_SECTION( m_pNtHdr );
    
    for ( unsigned i = 0; i < GetNumberOfSections(); i++, pSectHdr++ )
    {
        DWORD cbMaxOnDisk
            = min( pSectHdr->Misc.VirtualSize, pSectHdr->SizeOfRawData );

        DWORD startSectRVA = pSectHdr->VirtualAddress;
        DWORD endSectRVA = startSectRVA + cbMaxOnDisk;
        
        if ( (rva >= startSectRVA) && (rva < endSectRVA) )
            return pSectHdr->PointerToRawData + (rva - startSectRVA);
    }
    
    return (DWORD)-1;   // RVA not found in the section table... Ooops!
}
