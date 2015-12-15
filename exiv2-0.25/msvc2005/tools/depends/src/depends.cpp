//==========================================
// Matt Pietrek
// Microsoft Systems Journal, Feb 1997
// FILE: DEPENDS.CPP
//==========================================
#include <windows.h>
#include <stdio.h>
#include "dependencylist.h"
#include "peexe.h"

//============================== Variables ===============================
char g_szHelpSyntax[] =
"DEPENDS - Matt Pietrek, 1997, for MSJ\n"
"Syntax: DEPENDS [args] <executable filename>\n"
"  /v  show version information\n"
"  /t  show time & date information\n"
"  /p  show full path\n"
"  /q  quiet (don't report some MS dlls)\n"
"  /l  show link time & date information\n\n";

char * g_pszPrimaryFile = 0;

BOOL g_fShowDateTime = FALSE;
BOOL g_fShowLinkDateTime = FALSE;
BOOL g_fShowVersion = FALSE;
BOOL g_fShowFullPath = FALSE;
BOOL g_fQuiet = FALSE;

//============================== Prototypes ===============================
void DisplayFileInformation( PMODULE_FILE_INFO pModInfo,BOOL bQuiet );
void ShowVersionInfo( PSTR pszFileName );
BOOL TimeDateStampToFileTime( DWORD timeDateStamp, LPFILETIME pFileTime );
BOOL GetFileDateAsString( LPFILETIME pFt, char * pszDate, unsigned cbIn );
BOOL GetFileTimeAsString( LPFILETIME pFt, char * pszTime, unsigned cbIn,
                          BOOL fSeconds );

//=================================== Code ================================

BOOL ProcessCommandLine( int argc, char * argv[] )
{
    BOOL fSawFileName = FALSE;

    if ( argc < 2 )
        return FALSE;

    for ( int i = 1; i < argc; i++ )
    {
        PSTR pArg = argv[i];

        if ( (*pArg == '/') || (*pArg == '-') ) // Is it a switch char?
        {
            pArg++; // Point past switch char

            if ( 0 == lstrcmpi( pArg, "v" ) )
                g_fShowVersion = TRUE;                              
            else if ( 0 == lstrcmpi( pArg, "t" ) )
                g_fShowDateTime = TRUE;
            else if ( 0 == lstrcmpi( pArg, "l" ) )
                g_fShowLinkDateTime = TRUE;
            else if ( 0 == lstrcmpi( pArg, "p" ) )
                g_fShowFullPath = TRUE;
            else if ( 0 == lstrcmpi( pArg, "q" ) )
                g_fQuiet = TRUE;
            else
            {
                printf( "Unrecognized option: \"%s\"\n", pArg );
                return FALSE;
            }
        }
        else
        {
            if ( fSawFileName )
                return FALSE;

            g_pszPrimaryFile = pArg;
            fSawFileName = TRUE;
        }
    }

    return fSawFileName;
}

LPCTSTR getModuleBase(PMODULE_FILE_INFO pModule,BOOL bQuiet)
{
	LPCTSTR base = pModule->GetBaseName() ;
    LPCTSTR result = base ;
    if ( bQuiet ) {
        // keep quiet about these guys - they're build/compiler dependent
	    if (   _strnicmp(base,"MSVCR",5) == 0 ) result = NULL ;
	    if (   _strnicmp(base,"MSVCP",5) == 0 ) result = NULL ;
	    if (   _strnicmp(base,"API-MS-Win",10) == 0 ) result = NULL ;
    }
    return result ;
}

int main( int argc, char * argv[] )
{
    if ( !ProcessCommandLine( argc, argv ) )
    {
        printf( "%s %d bit build\n%s",argv[0],8*sizeof(void*),g_szHelpSyntax );
        return 1;
    }

    MODULE_DEPENDENCY_LIST depends( g_pszPrimaryFile );

    if ( !depends.IsValid() )
    {
        printf( "Error: %s %s\n", g_pszPrimaryFile, depends.GetErrorString() );
        return 1;
    }

    PMODULE_FILE_INFO pModInfo = 0;

    while ( pModInfo = depends.GetNextModule( pModInfo ) )
    {
        DisplayFileInformation( pModInfo,g_fQuiet );

        PMODULE_FILE_INFO pNotFound = 0;

        while ( pNotFound = pModInfo->GetNextNotFoundModule(pNotFound) )
        {
			LPCTSTR base = getModuleBase(pNotFound,g_fQuiet) ;
            if ( base ) printf( "  Not found: %s\n", base );
        }
    }

    return 0;
}

void DisplayFileInformation( PMODULE_FILE_INFO pModInfo, BOOL bQuiet )
{
    LPCTSTR base = getModuleBase(pModInfo,bQuiet);
	if ( !base ) return ;
	printf( "%-14s", base) ; // ->GetBaseName() );

    PSTR pszFullName = pModInfo->GetFullName();

    if ( g_fShowDateTime )
    {
        HFILE hFile = _lopen( pszFullName, OF_READ );
        if ( HFILE_ERROR != hFile )
        {
            FILETIME ft;

            if ( GetFileTime( (HANDLE)hFile, 0, 0, &ft ) )
            {
                char szFileDate[32] = { 0 };
                char szFileTime[32] = { 0 };

                GetFileDateAsString(&ft, szFileDate, sizeof(szFileDate) );
                GetFileTimeAsString(&ft, szFileTime, sizeof(szFileTime),
                                    TRUE);

                printf( "%s %s  ", szFileDate, szFileTime );
            }

            _lclose( hFile );
        }
    }

    if ( g_fShowLinkDateTime )
    {
        FILETIME ft;
        char szFileDate[32] = { 0 };
        char szFileTime[32] = { 0 };

        PE_EXE exe( pszFullName );

        TimeDateStampToFileTime( exe.GetTimeDateStamp(), &ft );

        GetFileDateAsString(&ft, szFileDate, sizeof(szFileDate) );
        GetFileTimeAsString(&ft, szFileTime, sizeof(szFileTime),
                            TRUE);

        printf( "%s %s  ", szFileDate, szFileTime );
    }

    if ( g_fShowFullPath )
        printf( "(%s)", pszFullName );

    printf( "\n" );

    if ( g_fShowVersion )
        ShowVersionInfo( pszFullName );
}

void ShowVersionInfo( PSTR pszFileName )
{
    DWORD cbVerInfo, dummy;

    // How big is the version info?
    cbVerInfo = GetFileVersionInfoSize( pszFileName, &dummy );
    if ( !cbVerInfo )
        return;
    
    // Allocate space to hold the info
    PBYTE pVerInfo = new BYTE[cbVerInfo];
    if ( !pVerInfo )
        return;

    _try
    {
        if ( !GetFileVersionInfo(pszFileName, 0, cbVerInfo, pVerInfo) )
            _leave;

        char * predefResStrings[] = 
        {
            "CompanyName",
            "FileDescription",
            "FileVersion",
            "InternalName",
            "LegalCopyright",
            "OriginalFilename",
            "ProductName",
            "ProductVersion",
            0
        };

        for ( unsigned i=0; predefResStrings[i]; i++ )
        {
            char szQueryStr[ 0x100 ];
            char szQueryStr2[0x100 ];

            // Format the string with the 1200 codepage (Unicode)
            wsprintf( szQueryStr, "\\StringFileInfo\\%04X%04X\\%s",
                        GetUserDefaultLangID(), 1200,
                        predefResStrings[i] );

            // Format the string with the 1252 codepage (Windows Multilingual)
            wsprintf( szQueryStr2, "\\StringFileInfo\\%04X%04X\\%s",
                        GetUserDefaultLangID(), 1252,
                        predefResStrings[i] );
            // We may want to format a string with the "0000" codepage

            PSTR pszVerRetVal;
            UINT cbReturn;
            BOOL fFound;

            // Try first with the 1252 codepage
            fFound = VerQueryValue( pVerInfo, szQueryStr,
                                    (LPVOID *)&pszVerRetVal, &cbReturn );
            if ( !fFound )
            {
                // Hmm... 1252 wasn't found.  Try the 1200 codepage
                fFound = VerQueryValue( pVerInfo, szQueryStr2,
                                        (LPVOID *)&pszVerRetVal, &cbReturn );
            }

            if ( fFound )
                printf( "  %s %s\n", predefResStrings[i], pszVerRetVal );
        }
    }
    _finally
    {
        delete []pVerInfo;
    }
}


// Convert a TimeDateStamp (i.e., # of seconds since 1/1/1970) into a FILETIME

BOOL TimeDateStampToFileTime( DWORD timeDateStamp, LPFILETIME pFileTime )
{
    __int64 t1970 = 0x019DB1DED53E8000; // Magic... GMT...  Don't ask....

    __int64 timeStampIn100nsIncr = (__int64)timeDateStamp * 10000000;

    __int64 finalValue = t1970 + timeStampIn100nsIncr;

    memcpy( pFileTime, &finalValue, sizeof( finalValue ) );

    return TRUE;
}

BOOL GetFileDateAsString( LPFILETIME pFt, char * pszDate, unsigned cbIn )
{
    FILETIME ftLocal;
    SYSTEMTIME st;

    if ( !FileTimeToLocalFileTime( pFt, &ftLocal ) )
        return FALSE;

    if ( !FileTimeToSystemTime( &ftLocal, &st ) )
        return FALSE;

    char szTemp[12];

    wsprintf(   szTemp, "%02u/%02u/%04u",
                st.wMonth, st.wDay, st.wYear );
    lstrcpyn( pszDate, szTemp, cbIn );

    return TRUE;
}

BOOL GetFileTimeAsString(   LPFILETIME pFt, char * pszTime, unsigned cbIn,
                            BOOL fSeconds )
{
    FILETIME ftLocal;
    SYSTEMTIME st;

    if ( !FileTimeToLocalFileTime( pFt, &ftLocal ) )
        return FALSE;

    if ( !FileTimeToSystemTime( &ftLocal, &st ) )
        return FALSE;

    char szTemp[12];

    if ( fSeconds ) // Want seconds???
    {
        wsprintf( szTemp, "%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond );
    }
    else    // No thanks..  Just hours and minutes
    {
        wsprintf( szTemp, "%02u:%02u", st.wHour, st.wMinute );
    }

    lstrcpyn( pszTime, szTemp, cbIn );

    return TRUE;
}
