//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: MEMORYMAPPEDFILE.H
//==========================================
#ifndef __MEMMAPFL_H__
#define __MEMMAPFL_H__

enum errMMF {   errMMF_NoError, errMMF_FileOpen,
                errMMF_FileMapping, errMMF_MapView };

class MEMORY_MAPPED_FILE
{
    public:
    
    MEMORY_MAPPED_FILE( PSTR pszFileName );
    ~MEMORY_MAPPED_FILE(void);
    
    PVOID   GetBase( void ){ return m_pMemoryMappedFileBase; }
    DWORD   GetFileSize( void ){ return m_cbFile; }
    BOOL    IsValid( void ) { return errMMF_NoError == m_errCode; } 
    errMMF  GetErrorType(){ return m_errCode; }

    private:

    HANDLE      m_hFile;
    HANDLE      m_hFileMapping; // Handle of memory mapped file
    PVOID       m_pMemoryMappedFileBase;
    DWORD       m_cbFile;
    errMMF      m_errCode;  
};

typedef MEMORY_MAPPED_FILE *PMEMORY_MAPPED_FILE;

#endif
