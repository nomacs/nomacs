@echo off
rem copylibs.bat
rem get the support libraries
rem
setlocal
set EXIV2LIBS=c:\exiv2libs

xcopy/yesihq  %EXIV2LIBS%\expat-2.0.1     ..\..\expat-2.0.1
xcopy/yesihq  %EXIV2LIBS%\zlib-1.2.3      ..\..\zlib-1.2.3


rem That's all Folks!
rem -----------------