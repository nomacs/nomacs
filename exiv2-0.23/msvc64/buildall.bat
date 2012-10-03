@echo off

rem ##
rem buildall - wee script for building from the command line

setlocal ENABLEEXTENSIONS 
set "SYNTAX=buildall [ /build ^| /rebuild ^| /clean ^| /upgrade ]"

rem ##
rem test arguments
set "ACTION=%1%"
if NOT DEFINED ACTION (
	echo %SYNTAX%
	goto jail
)

rem ##
rem execute /upgrade
if %ACTION%==/upgrade (
	devenv /upgrade exiv2.sln
	goto jail
)

rem ##
rem cleanup the bin if necessary
set DELBIN=0
if %ACTION%==/rebuild set DELBIN=1
if %ACTION%==/clean   set DELBIN=1
if %DELBIN%==1 (
	if EXIST bin rmdir/s/q bin
	del/s *.pdb *.ild *.ncb *.bsc *.idb *.ilk *.pch *.tlog > NUL
	rmdir/s/q exiv2lib\win32 rmdir/s/q exiv2lib\x64 rmdir/s/q zlib\win32 zlib\x64 > NUL
)

rem ##
rem the main build activity
devenv exiv2.sln %ACTION% "Debug|Win32"
devenv exiv2.sln %ACTION% "DebugDLL|Win32" 
devenv exiv2.sln %ACTION% "Release|Win32" 
devenv exiv2.sln %ACTION% "ReleaseDLL|Win32" 
devenv exiv2.sln %ACTION% "Debug|x64" 
devenv exiv2.sln %ACTION% "DebugDLL|x64" 
devenv exiv2.sln %ACTION% "Release|x64" 
devenv exiv2.sln %ACTION% "ReleaseDLL|x64" 

rem ##
rem cleanup and leave
:jail
endlocal

rem That's all Folks!
rem ##
