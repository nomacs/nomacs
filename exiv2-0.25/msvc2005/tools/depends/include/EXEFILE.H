//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: EXEFILE.H
//==========================================
#ifndef __EXEFILE_H__
#define __EXEFILE_H__

#ifndef __MEMMAPFL_H__
#include "memorymappedfile.h"
#endif

// MakePtr is a macro that allows you to easily add to values (including
// pointers) together without dealing with C's pointer arithmetic.  It
// essentially treats the last two parameters as DWORDs.  The first
// parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (DWORD)(addValue))

enum EXE_TYPE { exeType_Invalid, exeType_DOS, exeType_NE, exeType_VXD,
                exeType_LX, exeType_PE };

enum errEXE_FILE {  errEXE_FILE_NO_ERROR,
                    errEXE_FILE_FILE_NOT_FOUND,
                    errEXE_FILE_INVALID_FORMAT };

class EXE_FILE : public MEMORY_MAPPED_FILE
{
    public:
        
    EXE_FILE( PSTR pszFileName );
    ~EXE_FILE( ){ ; }
    
    BOOL    IsValid( void ){ return errMMF_NoError == m_errorType; }
    
    errEXE_FILE GetErrorType( void ){ return m_errorType; }

    DWORD   GetSecondaryHeaderOffset( void ){ return m_secondaryHeaderOffset; }

    EXE_TYPE GetExeType( void ){ return m_exeType; }

    PSTR    GetFileTypeDescription( void );

    protected:

    errEXE_FILE m_errorType;
            
    private:

    LONG        m_secondaryHeaderOffset;
    EXE_TYPE    m_exeType;

};

#endif
