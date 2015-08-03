@echo off
echo +++
echo +++ building %*+++
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

@echo on
cd %PROJECTDIR%..\..\..\openssl
set BUILD=0
if %ACTION% == rebuild set BUILD=build
if NOT EXIST %SolutionDir%bin\%PLATFORM%\%CONFIG%\ssleay32.lib set BUILD=build
if NOT EXIST %SolutionDir%bin\%PLATFORM%\%CONFIG%\libeay32.lib set BUILD=build
if %BUILD% == 0 GOTO POSTPROCESS


set SHARED=shared
if %CONFIG% == Debug     set SHARED=no-shared
if %CONFIG% == Release   set SHARED=no-shared

if NOT EXIST                 %SolutionDir%bin\%PLATFORM%\%CONFIG%\           mkdir %SolutionDir%bin\%PLATFORM%\%CONFIG%\

                      copy/y %ProjectDir%..\openssl\%PLATFORM%\%CONFIG%\lib\*.lib  %SolutionDir%bin\%PLATFORM%\%CONFIG%\
if %SHARED% == shared copy/y %ProjectDir%..\openssl\%PLATFORM%\%CONFIG%\bin\*.dll  %SolutionDir%bin\%PLATFORM%\%CONFIG%\

:POSTPROCESS

set ERRORLEVEL=0
echo OK
goto EOF

:ERROR
set ERRORLEVEL=911

:EOF
rem That's all Folks
rem ----------------
