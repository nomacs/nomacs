@echo off
setlocal

rem ----------------------------
rem Work in progress - not ready for production use
rem ----------------------------

rem ----------------------------
rem cm - wrapper script for CMake
rem http://www.dostips.com/

goto:afterSyntax
rem -------------------------------
rem Functions
:Syntax
echo useage: cm [ clean ^| unix ^| 2003 ^| 2005 ^| 2008 ^| 2010 [64]] "option"+
goto:eof
:afterSyntax

rem ----------------------------
rem test command args
set "G=%1"
shift

rem [option: 32|Win32|x64|64]
set  Q=
set "P=%1"
if     DEFINED P shift
IF NOT DEFINED P set P=x
if (%P%)==(x64)   set P=64
if (%P%)==(64)    set Q=64
if (%P%)==(Win32) set P=32
if (%P%)==(32)    set Q=32
if (%P%)==(x)     set P=
if DEFINED Q      set P=

rem ----------------------------
rem figure out the generator (G)
if NOT DEFINED G (
	call:Syntax
	goto:jail
)

if "%G%"=="7"     set G=2003
if "%G%"=="8"     set G=2005
if "%G%"=="9"     set G=2008
if "%G%"=="10"    set G=2010

if "%G%"=="2003"  set "G=Visual Studio 7 .NET 2003"
if "%G%"=="2005"  set "G=Visual Studio 8 2005"
if "%G%"=="2008"  set "G=Visual Studio 9 2008"
if "%G%"=="2010"  set "G=Visual Studio 10"

if (%Q%)==(64)    set "G=%G% Win64"

rem ----------------------------
rem Cleanup from last time
set C=CMakeCache.txt
set D=CMakeFiles
set X=xmdpsk
set Y=cmake_install.cmake
if EXIST %C%     del       %C%
if EXIST %D%     rmdir/s/q %D% 
if EXIST %X%\%C% del       %X%\%C%
if EXIST %X%\%D% rmdir/s/q %X%\%D%
if EXIST %Y%     del/s     cmake_*.cmake >NUL

rem ----------------------------
rem really execute cmake
if NOT "%G%"=="clean" (
    rem echo.---------------------------------
    echo cmake CMakeLists.txt -G "%G%" %P% %1 %2 %3 %4 %5
         cmake CMakeLists.txt -G "%G%" %P% %1 %2 %3 %4 %5
)

rem ----------------------------
rem cleanup and finish
:jail
endlocal
goto:eof

rem That's All Folks!
rem
