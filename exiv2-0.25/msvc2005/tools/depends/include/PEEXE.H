//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: PEEXE.H
//==========================================
#ifndef __PEEXE_H__
#define __PEEXE_H__

#ifndef __EXEFILE_H__
#include "exefile.h"
#endif

class PE_EXE : public EXE_FILE
{
    public:

    PE_EXE( PSTR pszFileName );
    ~PE_EXE( ){ }

    BOOL IsValid() { return m_pNtHdr ? TRUE : FALSE; }

    // For those who want at the data directly
    PIMAGE_NT_HEADERS GetIMAGE_NT_HEADERS( void ) { return m_pNtHdr; }

    // IMAGE_FILE_HEADER fields
    WORD    GetMachine( void )
                { return m_pNtHdr->FileHeader.Machine; }
    WORD    GetNumberOfSections( void )
                { return m_pNtHdr->FileHeader.NumberOfSections; }
    DWORD   GetTimeDateStamp(void)
                { return m_pNtHdr->FileHeader.TimeDateStamp; }
    DWORD   GetCharacteristics( void )
                { return m_pNtHdr->FileHeader.Characteristics; }

    // IMAGE_OPTIONAL_HEADER fields
    DWORD   GetSizeOfCode( void )
            { return m_pNtHdr->OptionalHeader.SizeOfCode; }
    DWORD   GetSizeOfInitializedData( void )
            { return m_pNtHdr->OptionalHeader.SizeOfInitializedData; }
    DWORD   GetSizeOfUninitializedData( void )
            { return m_pNtHdr->OptionalHeader.SizeOfUninitializedData; }
    DWORD   GetAddressOfEntryPoint( void )
            { return m_pNtHdr->OptionalHeader.AddressOfEntryPoint; }
    DWORD   GetBaseOfCode( void )
            { return m_pNtHdr->OptionalHeader.BaseOfCode; }
/*
    DWORD   GetBaseOfData( void )
            { return m_pNtHdr->OptionalHeader.BaseOfData; }
*/
    DWORD   GetImageBase( void )
            { return m_pNtHdr->OptionalHeader.ImageBase; }
    DWORD   GetSectionAlignment( void )
            { return m_pNtHdr->OptionalHeader.SectionAlignment; }
    DWORD   GetFileAlignment( void )
            { return m_pNtHdr->OptionalHeader.FileAlignment; }
    WORD    GetMajorOperatingSystemVersion( void )
            { return m_pNtHdr->OptionalHeader.MajorOperatingSystemVersion; }
    WORD    GetMinorOperatingSystemVersion( void )
            { return m_pNtHdr->OptionalHeader.MinorOperatingSystemVersion; }
    WORD    GetMajorImageVersion( void )
            { return m_pNtHdr->OptionalHeader.MajorImageVersion; }
    WORD    GetMinorImageVersion( void )
            { return m_pNtHdr->OptionalHeader.MinorImageVersion; }
    WORD    GetMajorSubsystemVersion( void )
            { return m_pNtHdr->OptionalHeader.MajorSubsystemVersion; }
    WORD    GetMinorSubsystemVersion( void )
            { return m_pNtHdr->OptionalHeader.MinorSubsystemVersion; }
//    DWORD   GetWin32VersionValue( void )
//          { return m_pNtHdr->OptionalHeader.Win32VersionValue; }
    DWORD   GetSizeOfImage( void )
            { return m_pNtHdr->OptionalHeader.SizeOfImage; }
    DWORD   GetSizeOfHeaders( void )
            { return m_pNtHdr->OptionalHeader.SizeOfHeaders; }
    WORD    GetSubsystem( void )
            { return m_pNtHdr->OptionalHeader.Subsystem; }
    DWORD   GetSizeOfStackReserve( void )
            { return m_pNtHdr->OptionalHeader.SizeOfStackReserve; }
    DWORD   GetSizeOfStackCommit( void )
            { return m_pNtHdr->OptionalHeader.SizeOfStackCommit; }
    DWORD   GetSizeOfHeapReserve( void )
            { return m_pNtHdr->OptionalHeader.SizeOfHeapReserve; }
    DWORD   GetSizeOfHeapCommit( void )
            { return m_pNtHdr->OptionalHeader.SizeOfHeapCommit; }

    DWORD   GetDataDirectoryEntryRVA( DWORD id );
    PVOID   GetDataDirectoryEntryPointer( DWORD id );
    DWORD   GetDataDirectoryEntrySize( DWORD id );

    PVOID   GetReadablePointerFromRVA( DWORD rva );
    
    protected:

    DWORD   RVAToFileOffset( DWORD rva );

    PIMAGE_NT_HEADERS m_pNtHdr;
};

#endif
