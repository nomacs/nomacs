@echo off
echo +++
echo +++ building %* +++
echo +++

setlocal ENABLEEXTENSIONS

set PLATFORM=%1
set CONFIG=%2
set OutDir=%3
set VCInstallDir=%4
set ProjectDir=%5
set SolutionDir=%6
set Action=%7

echo +++ PLATFORM     = %PLATFORM%
echo +++ CONFIG       = %CONFIG%
echo +++ OutDir       = %OutDir%
echo +++ VCInstallDir = %VCInstallDir%
echo +++ ProjectDir   = %ProjectDir%
echo +++ SolutionDir  = %SolutionDir%
echo +++ Action       = %Action%
echo +++
echo.

perl --version > nul 2> nul
if ERRORLEVEL 1 (
  echo perl is not available
  goto ERROR
)

if NOT EXIST %PROJECTDIR%\..\..\..\openssl (
  echo %PROJECTDIR%\..\..\..\openssl does not exist
  goto ERROR
)

@echo on
cd %PROJECTDIR%..\..\..\openssl
set BUILD=0
if %ACTION% == rebuild set BUILD=build
if NOT EXIST %SolutionDir%bin\%PLATFORM%\%CONFIG%\ssleay32.lib set BUILD=build
if NOT EXIST %SolutionDir%bin\%PLATFORM%\%CONFIG%\libeay32.lib set BUILD=build
if %BUILD% == 0 GOTO POSTPROCESS

set VCVARS=vcvars32.bat
set VCCONFIG=VC-WIN32
set DO=ms\do_ms.bat
if %PLATFORM% == x64 (
    set VCCONFIG=VC-WIN64A
    set VCVARS=x86_amd64\vcvarsx86_amd64.bat
    SET DO=ms\do_win64a.bat
)
call %VCInstallDir%\bin\%VCVARS%

set SHARED=shared
if %CONFIG% == Debug     set SHARED=no-shared
if %CONFIG% == Release   set SHARED=no-shared

set MAKEFILE=ms\ntdll.mak
if %SHARED% == no-shared set MAKEFILE=ms\nt.mak
perl Configure %VCCONFIG% no-asm %SHARED% --prefix=%ProjectDir%%PLATFORM%\%CONFIG%
call     %DO%
nmake -f %MAKEFILE% 
nmake -f %MAKEFILE% install
nmake -f %MAKEFILE% clean

if NOT EXIST                    %SolutionDir%bin\%PLATFORM%\%CONFIG%\ mkdir %SolutionDir%bin\%PLATFORM%\%CONFIG%\
                         copy/y %ProjectDir%%PLATFORM%\%CONFIG%\lib\*.lib   %SolutionDir%bin\%PLATFORM%\%CONFIG%\
if %SHARED% == shared    copy/y %ProjectDir%%PLATFORM%\%CONFIG%\bin\*.dll   %SolutionDir%bin\%PLATFORM%\%CONFIG%\

:POSTPROCESS
set ERRORLEVEL=0
goto EOF

:ERROR
set ERRORLEVEL=911

:EOF
rem That's all Folks
rem ----------------
