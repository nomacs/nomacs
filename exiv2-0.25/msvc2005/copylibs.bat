@echo off
rem copylibs.bat
rem get the support libraries
rem
setlocal
set EXIV2LIBS=c:\exiv2libs

xcopy/yesihq  %EXIV2LIBS%\expat-2.1.0     ..\..\expat
xcopy/yesihq  %EXIV2LIBS%\zlib-1.2.7      ..\..\zlib
xcopy/yesihq  %EXIV2LIBS%\openssl-1.0.1j  ..\..\openssl 
xcopy/yesihq  %EXIV2LIBS%\libssh-0.5.5    ..\..\libssh
xcopy/yesihq  %EXIV2LIBS%\curl-7.39.0     ..\..\curl


rem That's all Folks!
rem -----------------